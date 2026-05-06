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

#include "window_manager.h"

#include <algorithm>
#include <cinttypes>
#include <shared_mutex>

#include "input_manager.h"
#include "marshalling_helper.h"
#include "parameters.h"
#include "window_adapter.h"
#include "window_display_change_adapter.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "ws_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManager"};
}
std::unordered_map<int32_t, sptr<WindowManager>> WindowManager::windowManagerMap_ = {};
std::shared_mutex WindowManager::windowManagerMapMutex_;

class WindowManager::Impl {
public:
    explicit Impl(std::recursive_mutex& mutex) : mutex_(mutex) {}
    void NotifyWMSConnected(int32_t userId, int32_t screenId, int32_t pid = INVALID_PID);
    void NotifyWMSDisconnected(int32_t userId, int32_t screenId, int32_t pid = INVALID_PID);
    void NotifyFocused(uint32_t windowId, const sptr<IRemoteObject>& abilityToken,
        WindowType windowType, DisplayId displayId);
    void NotifyUnfocused(uint32_t windowId, const sptr<IRemoteObject>& abilityToken,
        WindowType windowType, DisplayId displayId);
    void NotifyFocused(const sptr<FocusChangeInfo>& focusChangeInfo);
    void NotifyApplicationFocusChangedResult(bool isFocused);
    void NotifyWindowModeChange(WindowModeType type);
    void NotifyUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo);
    void NotifySystemBarChanged(DisplayId displayId, const SystemBarRegionTints& tints);
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos, WindowUpdateType type);
    void NotifyWindowVisibilityInfoChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos);
    void NotifyWindowVisibilityStateChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos);
    void PackWindowChangeInfo(const std::unordered_set<WindowInfoKey>& interestInfo,
        const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, WindowInfoList& windowChangeInfos);
    void NotifyWindowDrawingContentInfoChanged(const std::vector<sptr<WindowDrawingContentInfo>>&
        windowDrawingContentInfos);
    void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing);
    void NotifyWaterMarkFlagChangedResult(bool showWaterMark);
    void NotifyVisibleWindowNumChanged(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo);
    void NotifyGestureNavigationEnabledResult(bool enable);
    void NotifyDisplayInfoChanged(const sptr<IRemoteObject>& token, DisplayId displayId,
        float density, DisplayOrientation orientation);
    void NotifyDisplayIdChange(const WindowInfoList& windowInfoList);
    void NotifyWindowModeChangeForPropertyChange(const WindowInfoList& windowInfoList);
    void NotifyFloatingScaleChange(const WindowInfoList& windowInfoList);
    void NotifyMidSceneStatusChange(const WindowInfoList& windowInfoList);
    WindowInfoList GetWindowInfoListByInterestWindowIds(
        const sptr<IWindowInfoChangedListener>& listener, const WindowInfoList& windowInfoList);
    void NotifyWindowStyleChange(WindowStyleType type);
    void NotifyWindowSystemBarPropertyChange(WindowType type, const SystemBarProperty& systemBarProperty);
    void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info);
    void NotifyWindowRectChange(const WindowInfoList& windowInfoList);
    void NotifyWindowGlobalRectChange(const WindowInfoList& windowInfoList);
    void NotifyWMSWindowDestroyed(const WindowLifeCycleInfo& lifeCycleInfo, void* jsWindowNapiValue);
    void NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo);

    static inline SingletonDelegator<WindowManager> delegator_;
    template<typename T>
    using ListenerSet = std::unordered_set<sptr<T>, SptrHash<T>>;

    // Attribute
    std::recursive_mutex& mutex_;
    sptr<WindowManagerAgent> WindowDisplayIdChangeListenerAgent_; // can not be deleted or CI will fail
    sptr<WindowManagerAgent> windowPropertyChangeAgent_;
    ListenerSet<IWindowInfoChangedListener> windowDisplayIdChangeListeners_;
    ListenerSet<IWindowInfoChangedListener> windowRectChangeListeners_;
    ListenerSet<IWindowInfoChangedListener> windowGlobalRectChangeListeners_;
    ListenerSet<IWindowInfoChangedListener> windowModeChangeListeners_;
    ListenerSet<IWindowInfoChangedListener> floatingScaleChangeListeners_;
    ListenerSet<IWindowInfoChangedListener> midSceneStatusChangeListeners_;
    // Window support rotation
    sptr<WindowManagerAgent> windowSupportRotationListenerAgent_;
    ListenerSet<IWindowSupportRotationListener> windowSupportRotationListeners_;
    // Above locked by mutex_

    ListenerSet<IApplicationFocusChangedListener> applicationFocusChangeListeners_;

    std::map<sptr<IRemoteObject>, std::vector<sptr<WindowDisplayChangeAdapter>>> displayInfoChangedListeners_;

    // Life cycle listener
    sptr<IWindowLifeCycleListener> windowLifeCycleListener_ = nullptr;

    // WMS Connection listener
    sptr<IWMSConnectionChangedListener> wmsConnectionChangedListener_ = nullptr;
    std::mutex wmsConnectionChangedMutex_;

    // --------------- Listeners and need to register agent -----------------
    // Focus
    ListenerSet<IFocusChangedListener> focusChangedListeners_;
    sptr<WindowManagerAgent> focusChangedListenerAgent_ = nullptr;
    std::recursive_mutex focusChangedMutex_;

    // Window mode
    ListenerSet<IWindowModeChangedListener> windowModeListeners_;
    sptr<WindowManagerAgent> windowModeListenerAgent_ = nullptr;

    // System bar
    ListenerSet<ISystemBarChangedListener> systemBarChangedListeners_;
    sptr<WindowManagerAgent> systemBarChangedListenerAgent_ = nullptr;

    // Window update
    ListenerSet<IWindowUpdateListener> windowUpdateListeners_;
    sptr<WindowManagerAgent> windowUpdateListenerAgent_ = nullptr;

    // Window visbility
    ListenerSet<IVisibilityChangedListener> windowVisibilityListeners_;
    sptr<WindowManagerAgent> windowVisibilityListenerAgent_ = nullptr;

    // Window visibility state
    ListenerSet<IWindowInfoChangedListener> windowVisibilityStateListeners_;
    sptr<WindowManagerAgent> windowVisibilityStateListenerAgent_ = nullptr;

    // Window drawing content
    ListenerSet<IDrawingContentChangedListener> windowDrawingContentListeners_;
    sptr<WindowManagerAgent> windowDrawingContentListenerAgent_ = nullptr;

    // Camera float window
    ListenerSet<ICameraFloatWindowChangedListener> cameraFloatWindowChangedListeners_;
    sptr<WindowManagerAgent> cameraFloatWindowChangedListenerAgent_ = nullptr;

    // Water mark flag
    ListenerSet<IWaterMarkFlagChangedListener> waterMarkFlagChangeListeners_;
    sptr<WindowManagerAgent> waterMarkFlagChangeAgent_ = nullptr;

    // Gesture navigation enable
    ListenerSet<IGestureNavigationEnabledChangedListener> gestureNavigationEnabledListeners_;
    sptr<WindowManagerAgent> gestureNavigationEnabledAgent_ = nullptr;

    // Visible window num
    ListenerSet<IVisibleWindowNumChangedListener> visibleWindowNumChangedListeners_;
    sptr<WindowManagerAgent> visibleWindowNumChangedListenerAgent_ = nullptr;

    // Window style
    ListenerSet<IWindowStyleChangedListener> windowStyleListeners_;
    sptr<WindowManagerAgent> windowStyleListenerAgent_ = nullptr;

    // Window pid visibility
    ListenerSet<IWindowPidVisibilityChangedListener> windowPidVisibilityListeners_;
    sptr<WindowManagerAgent> windowPidVisibilityListenerAgent_ = nullptr;

    // Window system bar property
    ListenerSet<IWindowSystemBarPropertyChangedListener> windowSystemBarPropertyChangedListeners_;
    sptr<WindowManagerAgent> windowSystemBarPropertyChangeAgent_ = nullptr;
};

void WindowManager::Impl::NotifyWMSConnected(int32_t userId, int32_t screenId, int32_t pid)
{
    sptr<IWMSConnectionChangedListener> listener = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionChangedMutex_);
        if (!wmsConnectionChangedListener_) {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "listener is null, userId=%{public}d, screenId=%{public}d",
                userId, screenId);
            return;
        }
        listener = wmsConnectionChangedListener_;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "WMS on connected, userId=%{public}d, screenId=%{public}d, pid=%{public}d",
        userId, screenId, pid);
    listener->OnConnected(userId, screenId, pid);
}


void WindowManager::Impl::NotifyWMSDisconnected(int32_t userId, int32_t screenId, int32_t pid)
{
    sptr<IWMSConnectionChangedListener> listener = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionChangedMutex_);
        if (!wmsConnectionChangedListener_) {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "listener is null, userId=%{public}d, screenId=%{public}d",
                userId, screenId);
            return;
        }
        listener = wmsConnectionChangedListener_;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "WMS on disconnected, userId=%{public}d, screenId=%{public}d, pid=%{public}d",
        userId, screenId, pid);
    listener->OnDisconnected(userId, screenId, pid);
}

void WindowManager::Impl::NotifyFocused(const sptr<FocusChangeInfo>& focusChangeInfo)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "NotifyFocused [%{public}u; %{public}" PRIu64"; %{public}d; %{public}d; %{public}u]",
        focusChangeInfo->windowId_, focusChangeInfo->displayId_, focusChangeInfo->pid_, focusChangeInfo->uid_,
        static_cast<uint32_t>(focusChangeInfo->windowType_));
    std::vector<sptr<IFocusChangedListener>> focusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(focusChangedMutex_);
        focusChangeListeners.assign(focusChangedListeners_.begin(), focusChangedListeners_.end());
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "Notify focused listeners: %{public}zu", focusChangeListeners.size());
    for (auto& listener : focusChangeListeners) {
        listener->OnFocused(focusChangeInfo);
    }
}

void WindowManager::Impl::NotifyUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "NotifyUnfocused [%{public}u; %{public}" PRIu64"; %{public}d; %{public}d; %{public}u]",
        focusChangeInfo->windowId_, focusChangeInfo->displayId_, focusChangeInfo->pid_, focusChangeInfo->uid_,
        static_cast<uint32_t>(focusChangeInfo->windowType_));
    std::vector<sptr<IFocusChangedListener>> focusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(focusChangedMutex_);
        focusChangeListeners.assign(focusChangedListeners_.begin(), focusChangedListeners_.end());
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "Notify unfocused listeners: %{public}zu", focusChangeListeners.size());
    for (auto& listener : focusChangeListeners) {
        listener->OnUnfocused(focusChangeInfo);
    }
}

void WindowManager::Impl::NotifyWindowModeChange(WindowModeType type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "WindowManager::Impl UpdateWindowModeTypeInfo type: %{public}d",
        static_cast<uint8_t>(type));
    std::vector<sptr<IWindowModeChangedListener>> windowModeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowModeListeners.assign(windowModeListeners_.begin(), windowModeListeners_.end());
    }
    for (auto& listener : windowModeListeners) {
        listener->OnWindowModeUpdate(type);
    }
}

void WindowManager::Impl::NotifySystemBarChanged(DisplayId displayId, const SystemBarRegionTints& tints)
{
    for (auto tint : tints) {
        WLOGFD("type:%{public}d, enable:%{public}d," \
            "backgroundColor:%{public}x, contentColor:%{public}x " \
            "region:[%{public}d, %{public}d, %{public}d, %{public}d]",
            tint.type_, tint.prop_.enable_, tint.prop_.backgroundColor_, tint.prop_.contentColor_,
            tint.region_.posX_, tint.region_.posY_, tint.region_.width_, tint.region_.height_);
    }
    std::vector<sptr<ISystemBarChangedListener>> systemBarChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        systemBarChangeListeners.assign(systemBarChangedListeners_.begin(), systemBarChangedListeners_.end());
    }
    for (auto& listener : systemBarChangeListeners) {
        listener->OnSystemBarPropertyChange(displayId, tints);
    }
}

void WindowManager::Impl::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    if (infos.empty()) {
        WLOGFE("infos is empty");
        return;
    }
    for (auto& info : infos) {
        if (info == nullptr) {
            TLOGD(WmsLogTag::WMS_MAIN, "info is nullptr");
            continue;
        }
        TLOGD(WmsLogTag::WMS_MAIN, "NotifyAccessibilityWindowInfo: wid[%{public}u], innerWid_[%{public}u]," \
            "uiNodeId_[%{public}u], rect[%{public}d %{public}d %{public}d %{public}d]," \
            "isFocused[%{public}d], isDecorEnable[%{public}d], displayId[%{public}" PRIu64"], layer[%{public}u]," \
            "mode[%{public}u], type[%{public}u, updateType[%{public}d], bundle[%{public}s]",
            info->wid_, info->innerWid_, info->uiNodeId_, info->windowRect_.width_, info->windowRect_.height_,
            info->windowRect_.posX_, info->windowRect_.posY_, info->focused_, info->isDecorEnable_, info->displayId_,
            info->layer_, info->mode_, info->type_, type, info->bundleName_.c_str());
        for (const auto& rect : info->touchHotAreas_) {
            TLOGD(WmsLogTag::WMS_MAIN, "window touch hot areas rect[x=%{public}d,y=%{public}d," \
            "w=%{public}d,h=%{public}d]", rect.posX_, rect.posY_, rect.width_, rect.height_);
        }
    }

    std::vector<sptr<IWindowUpdateListener>> windowUpdateListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowUpdateListeners.assign(windowUpdateListeners_.begin(), windowUpdateListeners_.end());
    }
    for (auto& listener : windowUpdateListeners) {
        listener->OnWindowUpdate(infos, type);
    }
}

void WindowManager::Impl::NotifyWindowVisibilityInfoChanged(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos)
{
    std::vector<sptr<IVisibilityChangedListener>> visibilityChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        visibilityChangeListeners.assign(windowVisibilityListeners_.begin(), windowVisibilityListeners_.end());
    }
    for (auto& listener : visibilityChangeListeners) {
        WLOGD("Notify WindowVisibilityInfo to caller");
        listener->OnWindowVisibilityChanged(windowVisibilityInfos);
    }
}

void WindowManager::Impl::NotifyWindowVisibilityStateChanged(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    std::vector<sptr<IWindowInfoChangedListener>> windowVisibilityStateListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowVisibilityStateListeners.assign(windowVisibilityStateListeners_.begin(),
                                              windowVisibilityStateListeners_.end());
    }
    for (auto& listener : windowVisibilityStateListeners) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
            continue;
        }
        WindowInfoList windowChangeInfos;
        PackWindowChangeInfo(listener->GetInterestInfo(), windowVisibilityInfos, windowChangeInfos);
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Notify WindowVisibilityState to caller, info size: %{public}zu",
            windowChangeInfos.size());
        listener->OnWindowInfoChanged(windowChangeInfos);
    }
}

void WindowManager::Impl::PackWindowChangeInfo(const std::unordered_set<WindowInfoKey>& interestInfo,
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, WindowInfoList& windowChangeInfos)
{
    for (const auto& info : windowVisibilityInfos) {
        std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowChangeInfo;
        if (interestInfo.find(WindowInfoKey::WINDOW_ID) != interestInfo.end()) {
            windowChangeInfo.emplace(WindowInfoKey::WINDOW_ID, info->windowId_);
        }
        if (interestInfo.find(WindowInfoKey::BUNDLE_NAME) != interestInfo.end()) {
            windowChangeInfo.emplace(WindowInfoKey::BUNDLE_NAME, info->bundleName_);
        }
        if (interestInfo.find(WindowInfoKey::ABILITY_NAME) != interestInfo.end()) {
            windowChangeInfo.emplace(WindowInfoKey::ABILITY_NAME, info->abilityName_);
        }
        if (interestInfo.find(WindowInfoKey::APP_INDEX) != interestInfo.end()) {
            windowChangeInfo.emplace(WindowInfoKey::APP_INDEX, info->appIndex_);
        }
        if (interestInfo.find(WindowInfoKey::VISIBILITY_STATE) != interestInfo.end()) {
            windowChangeInfo.emplace(WindowInfoKey::VISIBILITY_STATE, info->visibilityState_);
        }
        windowChangeInfos.emplace_back(windowChangeInfo);
    }
}

void WindowManager::Impl::NotifyWindowDrawingContentInfoChanged(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    std::vector<sptr<IDrawingContentChangedListener>> windowDrawingContentChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowDrawingContentChangeListeners.assign(windowDrawingContentListeners_.begin(),
                                                   windowDrawingContentListeners_.end());
    }
    for (auto& listener : windowDrawingContentChangeListeners) {
        WLOGFD("Notify windowDrawingContentInfo to caller");
        listener->OnWindowDrawingContentChanged(windowDrawingContentInfos);
    }
}

void WindowManager::Impl::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    TLOGD(WmsLogTag::DEFAULT,
        "Camera float window, accessTokenId=%{private}u, isShowing=%{public}u", accessTokenId, isShowing);
    std::vector<sptr<ICameraFloatWindowChangedListener>> cameraFloatWindowChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        cameraFloatWindowChangeListeners.assign(cameraFloatWindowChangedListeners_.begin(),
                                                cameraFloatWindowChangedListeners_.end());
    }
    for (auto& listener : cameraFloatWindowChangeListeners) {
        listener->OnCameraFloatWindowChange(accessTokenId, isShowing);
    }
}

void WindowManager::Impl::NotifyWaterMarkFlagChangedResult(bool showWaterMark)
{
    TLOGI(WmsLogTag::DEFAULT, "%{public}d", showWaterMark);
    std::vector<sptr<IWaterMarkFlagChangedListener>> waterMarkFlagChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        waterMarkFlagChangeListeners.assign(waterMarkFlagChangeListeners_.begin(), waterMarkFlagChangeListeners_.end());
    }
    for (auto& listener : waterMarkFlagChangeListeners) {
        listener->OnWaterMarkFlagUpdate(showWaterMark);
    }
}

void WindowManager::Impl::NotifyApplicationFocusChangedResult(bool isFocused)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "%{public}d", isFocused);
    std::vector<sptr<IApplicationFocusChangedListener>> applicationFocusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        applicationFocusChangeListeners.assign(applicationFocusChangeListeners_.begin(),
                                               applicationFocusChangeListeners_.end());
    }
    for (auto& listener : applicationFocusChangeListeners) {
        listener->OnApplicationFocusUpdate(isFocused);
    }
}

void WindowManager::Impl::NotifyGestureNavigationEnabledResult(bool enable)
{
    WLOGFI("Notify gesture navigation enable result, enable=%{public}d", enable);
    std::vector<sptr<IGestureNavigationEnabledChangedListener>> gestureNavigationEnabledListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        gestureNavigationEnabledListeners.assign(gestureNavigationEnabledListeners_.begin(),
                                                 gestureNavigationEnabledListeners_.end());
    }
    for (auto& listener : gestureNavigationEnabledListeners) {
        listener->OnGestureNavigationEnabledUpdate(enable);
    }
}

void WindowManager::Impl::NotifyVisibleWindowNumChanged(
    const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo)
{
    std::vector<sptr<IVisibleWindowNumChangedListener>> visibleWindowNumChangedListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        visibleWindowNumChangedListeners.assign(visibleWindowNumChangedListeners_.begin(),
                                                visibleWindowNumChangedListeners_.end());
    }
    for (auto& listener : visibleWindowNumChangedListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnVisibleWindowNumChange(visibleWindowNumInfo);
    }
}

void WindowManager::Impl::NotifyDisplayInfoChanged(const sptr<IRemoteObject>& token, DisplayId displayId,
    float density, DisplayOrientation orientation)
{
    std::vector<sptr<WindowDisplayChangeAdapter>> displayInfoChangedListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = displayInfoChangedListeners_.find(token);
        if (iter == displayInfoChangedListeners_.end()) {
            TLOGI_LIMITN_HOUR(WmsLogTag::DMS, THREE_TIMES, "can not find token in listener list");
            return;
        }
        displayInfoChangedListeners = iter->second;
    }

    for (auto& listener : displayInfoChangedListeners) {
        listener->OnDisplayInfoChange(token, displayId, density, orientation);
    }
}

void WindowManager::Impl::NotifyWindowModeChangeForPropertyChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> windowModeChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowModeChangeListeners.assign(
            windowModeChangeListeners_.begin(), windowModeChangeListeners_.end());
    }

    for (auto &listener : windowModeChangeListeners) {
        WindowInfoList windowInfoListForNotify = GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
        if (listener != nullptr && !windowInfoListForNotify.empty()) {
            listener->OnWindowInfoChanged(windowInfoListForNotify);
        }
    }
}

void WindowManager::Impl::NotifyFloatingScaleChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> floatingScaleChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        floatingScaleChangeListeners.assign(
            floatingScaleChangeListeners_.begin(), floatingScaleChangeListeners_.end());
    }

    for (auto &listener : floatingScaleChangeListeners) {
        WindowInfoList windowInfoListForNotify = GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
        if (listener != nullptr && !windowInfoListForNotify.empty()) {
            listener->OnWindowInfoChanged(windowInfoListForNotify);
        }
    }
}

void WindowManager::Impl::NotifyMidSceneStatusChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> midSceneStatusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        midSceneStatusChangeListeners.assign(
            midSceneStatusChangeListeners_.begin(), midSceneStatusChangeListeners_.end());
    }
    for (auto& listener : midSceneStatusChangeListeners) {
        WindowInfoList windowInfoListForNotify = GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
        if (listener != nullptr && !windowInfoListForNotify.empty()) {
            listener->OnWindowInfoChanged(windowInfoListForNotify);
        }
    }
}

void WindowManager::Impl::NotifyDisplayIdChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> windowDisplayIdChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowDisplayIdChangeListeners.assign(
            windowDisplayIdChangeListeners_.begin(), windowDisplayIdChangeListeners_.end());
    }

    for (auto &listener : windowDisplayIdChangeListeners) {
        WindowInfoList windowInfoListForNotify = GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
        if (listener != nullptr && !windowInfoListForNotify.empty()) {
            listener->OnWindowInfoChanged(windowInfoListForNotify);
        }
    }
}

WindowInfoList WindowManager::Impl::GetWindowInfoListByInterestWindowIds(
    const sptr<IWindowInfoChangedListener>& listener, const WindowInfoList& windowInfoList)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is nullptr");
        return windowInfoList;
    }
    auto interestWindowIds = listener->GetInterestWindowIds();
    if (interestWindowIds.empty()) {
        return windowInfoList;
    }
    WindowInfoList windowInfoListForNotify;
    for (const auto& iter : windowInfoList) {
        auto windowInfo = iter;
        if (windowInfo.find(WindowInfoKey::WINDOW_ID) != windowInfo.end() &&
            interestWindowIds.find(std::get<uint32_t>(windowInfo[WindowInfoKey::WINDOW_ID])) !=
            interestWindowIds.end()) {
            windowInfoListForNotify.emplace_back(windowInfo);
        }
    }
    return windowInfoListForNotify;
}

void WindowManager::Impl::NotifyWindowStyleChange(WindowStyleType type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "WindowStyleChange type: %{public}d",
          static_cast<uint8_t>(type));
    std::vector<sptr<IWindowStyleChangedListener>> windowStyleListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowStyleListeners.assign(windowStyleListeners_.begin(), windowStyleListeners_.end());
    }
    for (auto& listener : windowStyleListeners) {
        TLOGI(WmsLogTag::WMS_MAIN, "WindowStyleChange type: %{public}d",
              static_cast<uint8_t>(type));
        listener->OnWindowStyleUpdate(type);
    }
}

void WindowManager::Impl::NotifyWindowPidVisibilityChanged(
    const sptr<WindowPidVisibilityInfo>& info)
{
    std::vector<sptr<IWindowPidVisibilityChangedListener>> windowPidVisibilityListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowPidVisibilityListeners.assign(
            windowPidVisibilityListeners_.begin(), windowPidVisibilityListeners_.end());
    }
    for (auto& listener : windowPidVisibilityListeners) {
        if (listener != nullptr) {
            listener->NotifyWindowPidVisibilityChanged(info);
        }
    }
}

void WindowManager::Impl::NotifyWindowRectChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> windowRectChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowRectChangeListeners.assign(windowRectChangeListeners_.begin(), windowRectChangeListeners_.end());
    }
    for (auto& listener : windowRectChangeListeners) {
        if (listener != nullptr) {
            listener->OnWindowInfoChanged(windowInfoList);
        }
    }
}

void WindowManager::Impl::NotifyWindowGlobalRectChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> windowGlobalRectChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowGlobalRectChangeListeners.assign(
            windowGlobalRectChangeListeners_.begin(), windowGlobalRectChangeListeners_.end());
    }
    for (auto &listener : windowGlobalRectChangeListeners) {
        if (listener != nullptr) {
            listener->OnWindowInfoChanged(windowInfoList);
        }
    }
}

void WindowManager::Impl::NotifyWMSWindowDestroyed(const WindowLifeCycleInfo& lifeCycleInfo, void* jsWindowNapiValue)
{
    TLOGD(WmsLogTag::WMS_LIFE, "notify window destroyed");
    sptr<IWindowLifeCycleListener> wmsWindowDestroyedListener;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        wmsWindowDestroyedListener = windowLifeCycleListener_;
    }
    if (wmsWindowDestroyedListener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "window destroyed listener is nullptr");
        return;
    }
    wmsWindowDestroyedListener->OnWindowDestroyed(lifeCycleInfo, jsWindowNapiValue);
}

void WindowManager::Impl::NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo)
{
    std::vector<sptr<IWindowSupportRotationListener>> windowSupportRotationListener;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowSupportRotationListener.assign(
            windowSupportRotationListeners_.begin(), windowSupportRotationListeners_.end());
    }
    for (auto& listener : windowSupportRotationListener) {
        TLOGD(WmsLogTag::WMS_ROTATION, "Notify supportRotationInfo to caller");
        listener->OnSupportRotationChange(supportRotationInfo);
    }
}

WindowManager::WindowManager(const int32_t userId) : userId_(userId),
    pImpl_(std::make_unique<Impl>(mutex_))
{
}

WindowManager::~WindowManager()
{
    WindowAdapter::GetInstance(userId_).UnregisterOutlineRecoverCallbackFunc();
    TLOGI(WmsLogTag::WMS_SCB, "destroyed, userId: %{public}d", userId_);
}

WindowManager& WindowManager::GetInstance()
{
    static auto instance = sptr<WindowManager>::MakeSptr();
    return *instance;
}

WindowManager& WindowManager::GetInstance(const int32_t userId)
{
    /**
     * Only system applications or services with a userId of 0 are allowed to communicate
     * with multiple WMS-Servers and are permitted to listen for WMS connection status.
     */
    int32_t clientUserId = GetUserIdByUid(getuid());
    if (clientUserId != SYSTEM_USERID || userId <= INVALID_USER_ID) {
        return GetInstance();
    }

    if (!WindowManager::IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "get default instance, userId: %{public}d", userId);
        return GetInstance();
    }

    /**
     * multi-instance mode
     * At present, map does not have memory leak issues. In actual business scenarios,
     * number of instances created is limited (estimated to be less than 20).
     */
    std::unique_lock<std::shared_mutex> lock(windowManagerMapMutex_);
    auto iter = windowManagerMap_.find(userId);
    if (iter != windowManagerMap_.end()) {
        return *iter->second;
    }
    if (windowManagerMap_.size() > MAX_INSTANCE_NUM) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Can not create a new instance that limited by MAX_INSTANCE_NUM");
        return GetInstance();
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    auto instance = sptr<WindowManager>::MakeSptr(userId);
    windowManagerMap_.insert({ userId, instance });
    return *instance;
}

WMError WindowManager::RemoveInstanceByUserId(const int32_t userId)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "Remove instance, userId=%{public}d", userId);
    std::unique_lock<std::shared_mutex> lock(windowManagerMapMutex_);
    windowManagerMap_.erase(userId);
    return WMError::WM_OK;
}

bool WindowManager::IsMultiInstanceEnabled()
{
    static bool enabled = [] {
        bool isConcurrentUser = system::GetBoolParameter("persist.dms.concurrentuser", true);
        TLOGNI(WmsLogTag::WMS_SCB, "isConcurrentUser: %{public}d", isConcurrentUser);
        return isConcurrentUser;
    }();
    return enabled;
}

WMError WindowManager::ActiveFaultAgentReregister(const WindowManagerAgentType type,
    const sptr<WindowManagerAgent>& agent)
{
    // Note: When register agent failed due to the sceneboard is restarting and wms proxy is not ready,
    // activate the fault recovery solution and agents will be re-registered by independent recovery.
    if (!WindowAdapter::GetInstance(userId_).IsMockSMSProxyAlive()) {
        TLOGE(WmsLogTag::DEFAULT, "SA not alive, fault recovery unavailable");
        return WMError::WM_ERROR_SAMGR;
    }
    auto isWMSProxyValid = WindowAdapter::GetInstance(userId_).IsWindowManagerServiceProxyValid();
    if (isWMSProxyValid) {
        TLOGE(WmsLogTag::DEFAULT, "Not activated, scb is ready");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    TLOGI(WmsLogTag::DEFAULT, "Activate the agent re-register solution, type: %{public}d", type);
    WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgentWhenSCBFault(type, agent);
    return WMError::WM_OK;
}


WMError WindowManager::RegisterWMSConnectionChangedListener(const sptr<IWMSConnectionChangedListener>& listener)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "enter");
    int32_t clientUserId = GetUserIdByUid(getuid());
    // only system applications or services with a userId of 0 are allowed to communicate
    // with multiple WMS-Servers and are permitted to listen for WMS connection status.
    if (clientUserId != SYSTEM_USERID) {
        TLOGW(WmsLogTag::WMS_MULTI_USER, "Permission denied");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Register failed: listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(pImpl_->wmsConnectionChangedMutex_);
        if (pImpl_->wmsConnectionChangedListener_) {
            TLOGI(WmsLogTag::WMS_MULTI_USER, "Listener already registered, skipping");
            return WMError::WM_OK;
        }
        pImpl_->wmsConnectionChangedListener_ = listener;
    }
    auto ret = WindowAdapter::GetInstance(userId_).RegisterWMSConnectionChangedListener(
        [weakThis = wptr(this)](int32_t userId, int32_t screenId, bool isConnected, int32_t pid) {
            auto windowManager = weakThis.promote();
            if (!windowManager) {
                TLOGE(WmsLogTag::WMS_SCB, "window manager is null");
                return;
            }
            windowManager->OnWMSConnectionChanged(userId, screenId, isConnected, pid);
        });
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Register callback failed");
        std::lock_guard<std::mutex> lock(pImpl_->wmsConnectionChangedMutex_);
        pImpl_->wmsConnectionChangedListener_ = nullptr;
    }
    return ret;
}

WMError WindowManager::UnregisterWMSConnectionChangedListener()
{
    WindowAdapter::GetInstance(userId_).UnregisterWMSConnectionChangedListener();
    {
        std::lock_guard<std::mutex> lock(pImpl_->wmsConnectionChangedMutex_);
        pImpl_->wmsConnectionChangedListener_ = nullptr;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER, "Unregister success");
    return WMError::WM_OK;
}

WMError WindowManager::RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret;
    auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<WindowManagerAgent> tempAgent = nullptr; // for avoid holding locks to send ipc
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->focusChangedMutex_);
        if (pImpl_->focusChangedListenerAgent_ == nullptr) {
            pImpl_->focusChangedListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
        }
        tempAgent = pImpl_->focusChangedListenerAgent_;
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(agentType, tempAgent);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(agentType, tempAgent);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "register failed");
        std::lock_guard<std::recursive_mutex> lock(pImpl_->focusChangedMutex_);
        pImpl_->focusChangedListenerAgent_ = nullptr;
        return ret;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->focusChangedMutex_);
    pImpl_->focusChangedListeners_.insert(listener);
    return ret;
}

WMError WindowManager::UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->focusChangedMutex_);
    pImpl_->focusChangedListeners_.erase(listener);
    WMError ret = WMError::WM_OK;
    if (pImpl_->focusChangedListeners_.empty() && pImpl_->focusChangedListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->focusChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowModeListenerAgent_ == nullptr) {
        pImpl_->windowModeListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE, pImpl_->windowModeListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE, pImpl_->windowModeListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_MAIN, "RegisterWindowManagerAgent failed!");
        pImpl_->windowModeListenerAgent_ = nullptr;
        return ret;
    }
    if (pImpl_->windowModeListeners_.count(listener)) {
        TLOGW(WmsLogTag::WMS_MAIN, "Listener is already registered.");
        return WMError::WM_OK;
    }
    pImpl_->windowModeListeners_.insert(listener);
    return ret;
}

WMError WindowManager::UnregisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->windowModeListeners_.erase(listener)) {
        TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowModeListeners_.empty() && pImpl_->windowModeListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE, pImpl_->windowModeListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowModeListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterSystemBarChangedListener(const sptr<ISystemBarChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->systemBarChangedListenerAgent_ == nullptr) {
        pImpl_->systemBarChangedListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR, pImpl_->systemBarChangedListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR, pImpl_->systemBarChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->systemBarChangedListenerAgent_ = nullptr;
    } else {
        pImpl_->systemBarChangedListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterSystemBarChangedListener(const sptr<ISystemBarChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->systemBarChangedListeners_.erase(listener)) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->systemBarChangedListeners_.empty() && pImpl_->systemBarChangedListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR, pImpl_->systemBarChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->systemBarChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    WLOGFD("displayId %{public}" PRIu64", excludeWindowId %{public}d", displayId, excludeWindowId);
    return WindowAdapter::GetInstance(userId_).MinimizeAllAppWindows(displayId, excludeWindowId);
}

WMError WindowManager::ToggleShownStateForAllAppWindows()
{
    WLOGFD("ToggleShownStateForAllAppWindows");
    return WindowAdapter::GetInstance(userId_).ToggleShownStateForAllAppWindows();
}

WMError WindowManager::SetWindowLayoutMode(WindowLayoutMode mode)
{
    WLOGFD("set window layout mode: %{public}u", mode);
    WMError ret  = WindowAdapter::GetInstance(userId_).SetWindowLayoutMode(mode);
    if (ret != WMError::WM_OK) {
        WLOGFE("set layout mode failed");
    }
    return ret;
}

WMError WindowManager::RegisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowUpdateListenerAgent_ == nullptr) {
        pImpl_->windowUpdateListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->windowUpdateListenerAgent_ = nullptr;
    } else {
        pImpl_->windowUpdateListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->windowUpdateListeners_.erase(listener)) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowUpdateListeners_.empty() && pImpl_->windowUpdateListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowUpdateListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto ret = WMError::WM_OK;
    auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY;

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->windowVisibilityListenerAgent_ == nullptr) {
        pImpl_->windowVisibilityListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        agentType, pImpl_->windowVisibilityListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(agentType, pImpl_->windowVisibilityListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SCB, "register failed");
        pImpl_->windowVisibilityListenerAgent_ = nullptr;
        return ret;
    }

    pImpl_->windowVisibilityListeners_.insert(listener);
    return ret;
}

WMError WindowManager::UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowVisibilityListeners_.erase(listener);

    WMError ret = WMError::WM_OK;
    if (pImpl_->windowVisibilityListeners_.empty() && pImpl_->windowVisibilityListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowVisibilityListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterDisplayIdChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            interestInfoMap_[windowInfoKey] = 1;
        } else {
            interestInfoMap_[windowInfoKey]++;
        }
        interestInfo |= static_cast<uint32_t>(windowInfoKey);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::DISPLAY_ID, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY,
                                         pImpl_->windowPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        if (pImpl_->windowDisplayIdChangeListeners_.count(listener)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowDisplayIdChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterDisplayIdChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowDisplayIdChangeListeners_.erase(listener);
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            continue;
        } else if (interestInfoMap_[windowInfoKey] == 1) {
            interestInfoMap_.erase(windowInfoKey);
            interestInfo |= static_cast<uint32_t>(windowInfoKey);
        } else {
            interestInfoMap_[windowInfoKey]--;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowDisplayIdChangeListeners_.empty() && pImpl_->windowPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(WindowInfoKey::DISPLAY_ID,
            interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterRectChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            interestInfoMap_[windowInfoKey] = 1;
        } else {
            interestInfoMap_[windowInfoKey]++;
        }
        interestInfo |= static_cast<uint32_t>(windowInfoKey);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::WINDOW_RECT, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
        ret = ActiveFaultAgentReregister(agentType, pImpl_->windowPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        if (pImpl_->windowRectChangeListeners_.count(listener)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowRectChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterRectChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowRectChangeListeners_.erase(listener);
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            continue;
        } else if (interestInfoMap_[windowInfoKey] == 1) {
            interestInfoMap_.erase(windowInfoKey);
            interestInfo |= static_cast<uint32_t>(windowInfoKey);
        } else {
            interestInfoMap_[windowInfoKey]--;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowRectChangeListeners_.empty() && pImpl_->windowPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(WindowInfoKey::WINDOW_RECT,
            interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterGlobalRectChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            interestInfoMap_[windowInfoKey] = 1;
        } else {
            interestInfoMap_[windowInfoKey]++;
        }
        interestInfo |= static_cast<uint32_t>(windowInfoKey);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::WINDOW_GLOBAL_RECT, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
        ret = ActiveFaultAgentReregister(agentType, pImpl_->windowPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        if (pImpl_->windowGlobalRectChangeListeners_.count(listener)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowGlobalRectChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterGlobalRectChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    pImpl_->windowGlobalRectChangeListeners_.erase(listener);
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            continue;
        } else if (interestInfoMap_[windowInfoKey] == 1) {
            interestInfoMap_.erase(windowInfoKey);
            interestInfo |= static_cast<uint32_t>(windowInfoKey);
        } else {
            interestInfoMap_[windowInfoKey]--;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowGlobalRectChangeListeners_.empty() && pImpl_->windowPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(
            WindowInfoKey::WINDOW_GLOBAL_RECT, interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterWindowModeChangedListenerForPropertyChange(
    const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            interestInfoMap_[windowInfoKey] = 1;
        } else {
            interestInfoMap_[windowInfoKey]++;
        }
        interestInfo |= static_cast<uint32_t>(windowInfoKey);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::WINDOW_MODE, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
        ret = ActiveFaultAgentReregister(agentType, pImpl_->windowPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        if (pImpl_->windowModeChangeListeners_.count(listener)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowModeChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterWindowModeChangedListenerForPropertyChange(
    const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowModeChangeListeners_.erase(listener);
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            continue;
        } else if (interestInfoMap_[windowInfoKey] == 1) {
            interestInfoMap_.erase(windowInfoKey);
            interestInfo |= static_cast<uint32_t>(windowInfoKey);
        } else {
            interestInfoMap_[windowInfoKey]--;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowModeChangeListeners_.empty() && pImpl_->windowPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(WindowInfoKey::WINDOW_MODE,
            interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterFloatingScaleChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            interestInfoMap_[windowInfoKey] = 1;
        } else {
            interestInfoMap_[windowInfoKey]++;
        }
        interestInfo |= static_cast<uint32_t>(windowInfoKey);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::FLOATING_SCALE, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
        ret = ActiveFaultAgentReregister(agentType, pImpl_->windowPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        if (pImpl_->floatingScaleChangeListeners_.count(listener)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->floatingScaleChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterFloatingScaleChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->floatingScaleChangeListeners_.erase(listener);
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            continue;
        } else if (interestInfoMap_[windowInfoKey] == 1) {
            interestInfoMap_.erase(windowInfoKey);
            interestInfo |= static_cast<uint32_t>(windowInfoKey);
        } else {
            interestInfoMap_[windowInfoKey]--;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->floatingScaleChangeListeners_.empty() && pImpl_->windowPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(
            WindowInfoKey::FLOATING_SCALE, interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            interestInfoMap_[windowInfoKey] = 1;
        } else {
            interestInfoMap_[windowInfoKey]++;
        }
        interestInfo |= static_cast<uint32_t>(windowInfoKey);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::MID_SCENE, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
        ret = ActiveFaultAgentReregister(agentType, pImpl_->windowPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        if (pImpl_->midSceneStatusChangeListeners_.count(listener)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->midSceneStatusChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->midSceneStatusChangeListeners_.erase(listener);
    uint32_t interestInfo = 0;
    for (auto windowInfoKey : listener->GetInterestInfo()) {
        if (interestInfoMap_.find(windowInfoKey) == interestInfoMap_.end()) {
            continue;
        } else if (interestInfoMap_[windowInfoKey] == 1) {
            interestInfoMap_.erase(windowInfoKey);
            interestInfo |= static_cast<uint32_t>(windowInfoKey);
        } else {
            interestInfoMap_[windowInfoKey]--;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->midSceneStatusChangeListeners_.empty() && pImpl_->windowPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(
            WindowInfoKey::MID_SCENE, interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    auto ret = WMError::WM_OK;
    auto agentType = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY;
    sptr<WindowManagerAgent> tempAgent = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (pImpl_->windowVisibilityStateListenerAgent_ == nullptr) {
            pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
        }
        tempAgent = pImpl_->windowVisibilityStateListenerAgent_;
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(agentType, tempAgent);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(agentType, tempAgent);
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (ret == WMError::WM_OK) {
        pImpl_->windowVisibilityStateListeners_.insert(listener);
    } else {
        pImpl_->windowVisibilityStateListenerAgent_ = nullptr;
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "register failed");
    }
    return ret;
}

WMError WindowManager::UnregisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowVisibilityStateListeners_.erase(listener);

    WMError ret = WMError::WM_OK;
    if (pImpl_->windowVisibilityStateListeners_.empty() && pImpl_->windowVisibilityStateListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityStateListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowVisibilityStateListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterCameraFloatWindowChangedListener(const sptr<ICameraFloatWindowChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->cameraFloatWindowChangedListenerAgent_ == nullptr) {
        pImpl_->cameraFloatWindowChangedListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT,
        pImpl_->cameraFloatWindowChangedListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT,
            pImpl_->cameraFloatWindowChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->cameraFloatWindowChangedListenerAgent_ = nullptr;
    } else {
        pImpl_->cameraFloatWindowChangedListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterCameraFloatWindowChangedListener(
    const sptr<ICameraFloatWindowChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->cameraFloatWindowChangedListeners_.erase(listener)) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->cameraFloatWindowChangedListeners_.empty() &&
        pImpl_->cameraFloatWindowChangedListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT,
            pImpl_->cameraFloatWindowChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->cameraFloatWindowChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterWaterMarkFlagChangedListener(const sptr<IWaterMarkFlagChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->waterMarkFlagChangeAgent_ == nullptr) {
        pImpl_->waterMarkFlagChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG,
        pImpl_->waterMarkFlagChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG,
            pImpl_->waterMarkFlagChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->waterMarkFlagChangeAgent_ = nullptr;
    } else {
        pImpl_->waterMarkFlagChangeListeners_.insert(listener);
    }
    WLOGFD("Try to registerWaterMarkFlagChangedListener && result : %{public}u", static_cast<uint32_t>(ret));
    return ret;
}

WMError WindowManager::UnregisterWaterMarkFlagChangedListener(const sptr<IWaterMarkFlagChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->waterMarkFlagChangeListeners_.erase(listener)) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->waterMarkFlagChangeListeners_.empty() &&
        pImpl_->waterMarkFlagChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG,
            pImpl_->waterMarkFlagChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->waterMarkFlagChangeAgent_ = nullptr;
        }
    }
    WLOGFD("Try to unregisterWaterMarkFlagChangedListener && result : %{public}u", static_cast<uint32_t>(ret));
    return ret;
}

WMError WindowManager::RegisterApplicationFocusChangedListener(const sptr<IApplicationFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->applicationFocusChangeListeners_.count(listener)) {
        TLOGI(WmsLogTag::WMS_FOCUS, "Listener is already registered.");
        return WMError::WM_OK;
    }
    pImpl_->applicationFocusChangeListeners_.insert(listener);
    TLOGD(WmsLogTag::WMS_FOCUS, "Try to registerApplicationFocusChangedListener end");
    return WMError::WM_OK;
}

WMError WindowManager::UnregisterApplicationFocusChangedListener(const sptr<IApplicationFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->applicationFocusChangeListeners_.erase(listener) == 0) {
        TLOGW(WmsLogTag::WMS_FOCUS, "could not find this listener");
        return WMError::WM_OK;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "Try to unregisterApplicationFocusChangedListener end");
    return WMError::WM_OK;
}

WMError WindowManager::RegisterGestureNavigationEnabledChangedListener(
    const sptr<IGestureNavigationEnabledChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->gestureNavigationEnabledAgent_ == nullptr) {
        pImpl_->gestureNavigationEnabledAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    if (pImpl_->gestureNavigationEnabledAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED,
            pImpl_->gestureNavigationEnabledAgent_);
        if (ret == WMError::WM_ERROR_SAMGR) {
            ret = ActiveFaultAgentReregister(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED,
                pImpl_->gestureNavigationEnabledAgent_);
        }
    } else {
        WLOGFE("Create windowManagerAgent object failed!");
        ret = WMError::WM_ERROR_NULLPTR;
    }
    if (ret != WMError::WM_OK) {
        WLOGFE("RegisterWindowManagerAgent failed!");
        pImpl_->gestureNavigationEnabledAgent_ = nullptr;
    } else {
        pImpl_->gestureNavigationEnabledListeners_.insert(listener);
    }
    WLOGFD("Try to registerGestureNavigationEnabledChangedListener and result is %{public}u",
        static_cast<uint32_t>(ret));
    return ret;
}

WMError WindowManager::UnregisterGestureNavigationEnabledChangedListener(
    const sptr<IGestureNavigationEnabledChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->gestureNavigationEnabledListeners_.erase(listener)) {
        WLOGFE("could not find this listener");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->gestureNavigationEnabledListeners_.empty() &&
        pImpl_->gestureNavigationEnabledAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED,
            pImpl_->gestureNavigationEnabledAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->gestureNavigationEnabledAgent_ = nullptr;
        }
    }
    WLOGFD("Try to unregisterGestureNavigationEnabledChangedListener and result is %{public}u",
        static_cast<uint32_t>(ret));
    return ret;
}

WMError WindowManager::RegisterDisplayInfoChangedListener(const sptr<IRemoteObject>& token,
    const sptr<IDisplayInfoChangedListener>& listener)
{
    if (token == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability token could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    sptr<WindowDisplayChangeAdapter> listenerAdapter = new (std::nothrow) WindowDisplayChangeAdapter(token, listener);
    if (listenerAdapter == nullptr) {
        TLOGE(WmsLogTag::DMS, "create listener adapter failed.");
        return WMError::WM_ERROR_NO_MEM;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = pImpl_->displayInfoChangedListeners_.find(token);
    if (iter == pImpl_->displayInfoChangedListeners_.end()) {
        pImpl_->displayInfoChangedListeners_.insert({token, {listenerAdapter}});
    } else {
        auto listerneIter = std::find_if(iter->second.begin(), iter->second.end(),
            [&listener](const sptr<WindowDisplayChangeAdapter>& item) {
                return listener == item->GetListener();
            });
        if (listerneIter != iter->second.end()) {
            TLOGW(WmsLogTag::DMS, "listener is already registered.");
        } else {
            iter->second.push_back(listenerAdapter);
        }
    }
    TLOGD(WmsLogTag::DMS, "try to registerDisplayInfoChangedListener success");
    return WMError::WM_OK;
}

WMError WindowManager::UnregisterDisplayInfoChangedListener(const sptr<IRemoteObject>& token,
    const sptr<IDisplayInfoChangedListener>& listener)
{
    if (token == nullptr) {
        TLOGE(WmsLogTag::DMS, "ability token could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = pImpl_->displayInfoChangedListeners_.find(token);
    if (iter == pImpl_->displayInfoChangedListeners_.end()) {
        TLOGW(WmsLogTag::DMS, "can not find the ability token");
    } else {
        auto listerneIter = std::find_if(iter->second.begin(), iter->second.end(),
            [&listener](sptr<WindowDisplayChangeAdapter>& item) {
                return listener == item->GetListener();
            });
        if (listerneIter == iter->second.end()) {
            TLOGW(WmsLogTag::DMS, "can not find the listener.");
        } else {
            iter->second.erase(listerneIter);
            if (iter->second.empty()) {
                pImpl_->displayInfoChangedListeners_.erase(iter);
            }
        }
    }
    TLOGD(WmsLogTag::DMS, "try to unregisterDisplayInfoChangedListener success");
    return WMError::WM_OK;
}

WMError WindowManager::RegisterWindowPidVisibilityChangedListener(
    const sptr<IWindowPidVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPidVisibilityListenerAgent_ == nullptr) {
        pImpl_->windowPidVisibilityListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY,
        pImpl_->windowPidVisibilityListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY,
            pImpl_->windowPidVisibilityListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "RegisterWindowManagerAgent failed!");
        pImpl_->windowPidVisibilityListenerAgent_ = nullptr;
    } else {
        pImpl_->windowPidVisibilityListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterWindowPidVisibilityChangedListener(
    const sptr<IWindowPidVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->windowPidVisibilityListeners_.erase(listener)) {
        TLOGE(WmsLogTag::WMS_LIFE, "could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPidVisibilityListeners_.empty() && pImpl_->windowPidVisibilityListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY,
            pImpl_->windowPidVisibilityListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPidVisibilityListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::NotifyDisplayInfoChange(const sptr<IRemoteObject>& token, DisplayId displayId,
    float density, DisplayOrientation orientation)
{
    TLOGD(WmsLogTag::DMS, "notify display info change, displayid=%{public}" PRIu64 ", density=%{public}f, "
        "orientation=%{public}d", displayId, density, orientation);
    if (token == nullptr) {
        TLOGE(WmsLogTag::DMS, "notify display info change failed, token is nullptr");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    pImpl_->NotifyDisplayInfoChanged(token, displayId, density, orientation);
    return WMError::WM_OK;
}

void WindowManager::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    WindowAdapter::GetInstance(userId_).GetFocusWindowInfo(focusInfo, displayId);
}

void WindowManager::GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
    const sptr<IRemoteObject>& abilityToken)
{
    WindowAdapter::GetInstance(userId_).GetFocusWindowInfoByAbilityToken(focusInfo, abilityToken);
}

void WindowManager::OnWMSConnectionChanged(int32_t userId, int32_t screenId, bool isConnected, int32_t pid) const
{
    if (isConnected) {
        pImpl_->NotifyWMSConnected(userId, screenId, pid);
    } else {
        pImpl_->NotifyWMSDisconnected(userId, screenId, pid);
    }
}

void WindowManager::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) const
{
    if (focusChangeInfo == nullptr) {
        WLOGFE("focusChangeInfo is nullptr.");
        return;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "window focus change: %{public}d, id: %{public}u", focused, focusChangeInfo->windowId_);
    if (focused) {
        pImpl_->NotifyFocused(focusChangeInfo);
    } else {
        pImpl_->NotifyUnfocused(focusChangeInfo);
    }
}

void WindowManager::UpdateWindowModeTypeInfo(WindowModeType type) const
{
    pImpl_->NotifyWindowModeChange(type);
}


WMError WindowManager::GetWindowModeType(WindowModeType& windowModeType) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetWindowModeType(windowModeType);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window mode type failed");
    }
    return ret;
}

void WindowManager::UpdateSystemBarRegionTints(DisplayId displayId,
    const SystemBarRegionTints& tints) const
{
    pImpl_->NotifySystemBarChanged(displayId, tints);
}

void WindowManager::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type) const
{
    pImpl_->NotifyAccessibilityWindowInfo(infos, type);
}

void WindowManager::UpdateWindowVisibilityInfo(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos) const
{
    pImpl_->NotifyWindowVisibilityInfoChanged(windowVisibilityInfos);
    pImpl_->NotifyWindowVisibilityStateChanged(windowVisibilityInfos);
}

void WindowManager::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos) const
{
    pImpl_->NotifyWindowDrawingContentInfoChanged(windowDrawingContentInfos);
}

WMError WindowManager::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetAccessibilityWindowInfo(infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window info failed");
    }
    return ret;
}

WMError WindowManager::ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId)
{
    WMError ret = WindowAdapter::GetInstance(userId_).ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Convert relative coordinate failed");
    }
    return ret;
}

WMError WindowManager::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetUnreliableWindowInfo(windowId, infos);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "get unreliable window info failed");
    }
    return ret;
}

WMError WindowManager::ListWindowInfo(const WindowInfoOption& windowInfoOption,
    std::vector<sptr<WindowInfo>>& infos) const
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "windowInfoOption: %{public}u %{public}u %{public}" PRIu64 " %{public}d",
        static_cast<WindowInfoFilterOptionDataType>(windowInfoOption.windowInfoFilterOption),
        static_cast<WindowInfoTypeOptionDataType>(windowInfoOption.windowInfoTypeOption),
        windowInfoOption.displayId, windowInfoOption.windowId);
    WMError ret = WindowAdapter::GetInstance(userId_).ListWindowInfo(windowInfoOption, infos);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}

WMError WindowManager::GetAllWindowLayoutInfo(DisplayId displayId, std::vector<sptr<WindowLayoutInfo>>& infos,
    const WindowInfoOptions& option) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetAllWindowLayoutInfo(displayId, infos, option);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}

WMError WindowManager::GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetAllMainWindowInfo(infos);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed");
    }
    return ret;
}

WMError WindowManager::GetMainWindowSnapshot(const std::vector<int32_t>& windowIds,
    const WindowSnapshotConfiguration& config, const sptr<IRemoteObject>& callback) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetMainWindowSnapshot(windowIds, config, callback);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed");
    }
    return ret;
}

WMError WindowManager::SetWindowSnapshotSkip(int32_t windowId, bool isSkip)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetWindowSnapshotSkip(windowId, isSkip);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "userId=%{public}d, winId=%{public}d, isSkip=%{public}d, retCode=%{public}d",
        userId_, windowId, isSkip, static_cast<int32_t>(ret));
    return ret;
}

WMError WindowManager::GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode) const
{
    return WindowAdapter::GetInstance(userId_).GetGlobalWindowMode(displayId, globalWinMode);
}

WMError WindowManager::GetFloatViewLimits(uint32_t templateType, FloatViewLimits &limits) const
{
    return WindowAdapter::GetInstance(userId_).GetFloatViewLimits(templateType, limits);
}

WMError WindowManager::GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName) const
{
    return WindowAdapter::GetInstance(userId_).GetTopNavDestinationName(windowId, topNavDestName);
}

WMError WindowManager::SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    return WindowAdapter::GetInstance(userId_).SetWatermarkImageForApp(pixelMap);
}

WMError WindowManager::SetScreenWatermarkImage(const std::shared_ptr<Media::PixelMap>& pixelMap, uint32_t priority)
{
    return WindowAdapter::GetInstance(userId_).SetScreenWatermarkImage(pixelMap, priority);
}

WMError WindowManager::CleanScreenWatermarkImage(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    return WindowAdapter::GetInstance(userId_).CleanScreenWatermarkImage(pixelMap);
}

WMError WindowManager::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetVisibilityWindowInfo(infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window visibility info failed");
    }
    return ret;
}

WMError WindowManager::DumpSessionAll(std::vector<std::string>& infos)
{
    WMError ret = WindowAdapter::GetInstance(userId_).DumpSessionAll(infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("dump session all failed");
    }
    return ret;
}

WMError WindowManager::DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos)
{
    WMError ret = WindowAdapter::GetInstance(userId_).DumpSessionWithId(persistentId, infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("dump session with id failed");
    }
    return ret;
}

WMError WindowManager::GetUIContentRemoteObj(int32_t windowId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetUIContentRemoteObj(windowId, uiContentRemoteObj);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to get UIContentRemoteObj. PersistentId=%{public}d; ret=%{public}u",
            windowId, static_cast<uint32_t>(ret));
    }
    return ret;
}

WMError WindowManager::GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "userId: %{public}d, retCode: %{public}d, displayId: %{public}" PRIu64,
        userId_, static_cast<int32_t>(ret), displayId);
    return ret;
}

WMError WindowManager::SetGestureNavigationEnabled(bool enable) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetGestureNavigationEnabled(enable);
    if (ret != WMError::WM_OK) {
        WLOGFE("set gesture navigation enabled failed");
    }
    return ret;
}

WMError WindowManager::NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible)
{
    WMError ret = WindowAdapter::GetInstance(userId_).NotifyWindowExtensionVisibilityChange(pid, uid, visible);
    if (ret != WMError::WM_OK) {
        WLOGFE("notify WindowExtension visibility change failed");
    }
    return ret;
}

void WindowManager::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing) const
{
    pImpl_->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
}

void WindowManager::NotifyWaterMarkFlagChangedResult(bool showWaterMark) const
{
    pImpl_->NotifyWaterMarkFlagChangedResult(showWaterMark);
}

void WindowManager::NotifyApplicationFocusChangedResult(bool isFocused) const
{
    pImpl_->NotifyApplicationFocusChangedResult(isFocused);
}

void WindowManager::NotifyGestureNavigationEnabledResult(bool enable) const
{
    pImpl_->NotifyGestureNavigationEnabledResult(enable);
}

void WindowManager::NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info) const
{
    pImpl_->NotifyWindowPidVisibilityChanged(info);
}

WMError WindowManager::RaiseWindowToTop(int32_t persistentId)
{
    WMError ret = WindowAdapter::GetInstance(userId_).RaiseWindowToTop(persistentId);
    if (ret != WMError::WM_OK) {
        WLOGFE("raise window to top failed");
    }
    return ret;
}

WMError WindowManager::NotifyWindowStyleChange(WindowStyleType type)
{
    pImpl_->NotifyWindowStyleChange(type);
    return WMError::WM_OK;
}

WMError WindowManager::RegisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowDrawingContentListenerAgent_ == nullptr) {
        pImpl_->windowDrawingContentListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
        pImpl_->windowDrawingContentListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
            pImpl_->windowDrawingContentListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->windowDrawingContentListenerAgent_ = nullptr;
    } else {
        pImpl_->windowDrawingContentListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowDrawingContentListeners_.erase(listener);

    WMError ret = WMError::WM_OK;
    if (pImpl_->windowDrawingContentListeners_.empty() && pImpl_->windowDrawingContentListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
            pImpl_->windowDrawingContentListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowDrawingContentListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManager::RegisterWindowSystemBarPropertyChangedListener(
    const sptr<IWindowSystemBarPropertyChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "listener is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowSystemBarPropertyChangeAgent_ == nullptr) {
        pImpl_->windowSystemBarPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_STATUS_BAR_PROPERTY,
        pImpl_->windowSystemBarPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_STATUS_BAR_PROPERTY,
            pImpl_->windowSystemBarPropertyChangeAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "RegisterWindowManagerAgent failed.");
        pImpl_->windowSystemBarPropertyChangeAgent_ = nullptr;
    } else {
        pImpl_->windowSystemBarPropertyChangedListeners_.insert(listener);
        TLOGI(WmsLogTag::WMS_IMMS, "listener registered success.");
    }
    return ret;
}

WMError WindowManager::UnregisterWindowSystemBarPropertyChangedListener(
    const sptr<IWindowSystemBarPropertyChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "listener is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->windowSystemBarPropertyChangedListeners_.erase(listener)) {
        TLOGE(WmsLogTag::WMS_IMMS, "could not find this listener");
        return WMError::WM_DO_NOTHING;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowSystemBarPropertyChangeAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_STATUS_BAR_PROPERTY,
            pImpl_->windowSystemBarPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowSystemBarPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

void WindowManager::NotifyWindowSystemBarPropertyChange(WindowType type, const SystemBarProperty& systemBarProperty)
{
    pImpl_->NotifyWindowSystemBarPropertyChange(type, systemBarProperty);
}

void WindowManager::Impl::NotifyWindowSystemBarPropertyChange(
    WindowType type, const SystemBarProperty& systemBarProperty)
{
    std::vector<sptr<IWindowSystemBarPropertyChangedListener>> windowSystemBarPropertyChangedListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowSystemBarPropertyChangedListeners.assign(windowSystemBarPropertyChangedListeners_.begin(),
                                                       windowSystemBarPropertyChangedListeners_.end());
    }
    for (auto &listener : windowSystemBarPropertyChangedListeners) {
        if (listener != nullptr) {
            TLOGI(WmsLogTag::WMS_IMMS, "type %{public}d, enable %{public}d.", type, systemBarProperty.enable_);
            listener->OnWindowSystemBarPropertyChanged(type, systemBarProperty);
        }
    }
}

WMError WindowManager::ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId)
{
    WMError ret = WindowAdapter::GetInstance(userId_).ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    if (ret != WMError::WM_OK) {
        WLOGFE("shift application window focus failed");
    }
    return ret;
}

WMError WindowManager::SetSpecificSystemWindowZIndex(WindowType windowType, int32_t zIndex)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetSpecificWindowZIndex(windowType, zIndex);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "set specific system window zIndex failed");
    }
    return ret;
}

WMError WindowManager::MoveMainWindowToTargetDisplay(DisplayId displayId, int32_t windowId)
{
    WMError ret = WindowAdapter::GetInstance(userId_).MoveMainWindowToTargetDisplay(displayId, windowId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed, windowId: %{public}d, displayId: %{public}" PRIu64, windowId, displayId);
    }
    return ret;
}

WMError WindowManager::RegisterVisibleWindowNumChangedListener(const sptr<IVisibleWindowNumChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->visibleWindowNumChangedListenerAgent_ == nullptr) {
        pImpl_->visibleWindowNumChangedListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
    }
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM,
        pImpl_->visibleWindowNumChangedListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM,
            pImpl_->visibleWindowNumChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MAIN, "RegisterWindowManagerAgent failed!");
        pImpl_->visibleWindowNumChangedListenerAgent_ = nullptr;
    } else {
        pImpl_->visibleWindowNumChangedListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManager::GetSnapshotByWindowId(int32_t windowId, std::shared_ptr<Media::PixelMap>& pixelMap)
{
    return WindowAdapter::GetInstance(userId_).GetSnapshotByWindowId(windowId, pixelMap);
}

WMError WindowManager::Snapshot(
    std::shared_ptr<Media::PixelMap>& pixelMap, int32_t windowId, const SnapshotConfig& config)
{
    WMError ret = WindowAdapter::GetInstance(userId_).Snapshot(pixelMap, windowId, config);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "snapshot failed");
    }
    return ret;
}

WMError WindowManager::UnregisterVisibleWindowNumChangedListener(const sptr<IVisibleWindowNumChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (!pImpl_->visibleWindowNumChangedListeners_.erase(listener)) {
        TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
        return WMError::WM_OK;
    }

    WMError ret = WMError::WM_OK;
    if (pImpl_->visibleWindowNumChangedListeners_.empty() && pImpl_->visibleWindowNumChangedListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM,
            pImpl_->visibleWindowNumChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->visibleWindowNumChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

void WindowManager::UpdateVisibleWindowNum(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo)
{
    pImpl_->NotifyVisibleWindowNumChanged(visibleWindowNumInfo);
}


WMError WindowManager::RegisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_MAIN, "start register");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (pImpl_->windowStyleListenerAgent_ == nullptr) {
            pImpl_->windowStyleListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
        }
        if (pImpl_->windowStyleListeners_.count(listener)) {
            TLOGW(WmsLogTag::WMS_MAIN, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowStyleListeners_.insert(listener);
    }
    WMError ret = WMError::WM_OK;
    ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_MAIN, "RegisterWindowManagerAgent failed!");
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        pImpl_->windowStyleListenerAgent_ = nullptr;
        pImpl_->windowStyleListeners_.erase(listener);
    }
    return ret;
}

WMError WindowManager::UnregisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_MAIN, "start unregister");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!pImpl_->windowStyleListeners_.erase(listener)) {
            TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
            return WMError::WM_OK;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowStyleListeners_.empty() && pImpl_->windowStyleListenerAgent_ != nullptr) {
        ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
        if (ret == WMError::WM_OK) {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            pImpl_->windowStyleListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WindowStyleType WindowManager::GetWindowStyleType()
{
    WindowStyleType styleType;
    if (WindowAdapter::GetInstance(userId_).GetWindowStyleType(styleType) == WMError::WM_OK) {
        return styleType;
    }
    return styleType;
}

WMError WindowManager::SkipSnapshotForAppProcess(int32_t pid, bool skip)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SkipSnapshotForAppProcess(pid, skip);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set process snapshot skip failed");
    }
    return ret;
}

WMError WindowManager::SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetProcessWatermark(pid, watermarkName, isEnabled);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}

WMError WindowManager::GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
    int32_t x, int32_t y, std::vector<int32_t>& windowIds) const
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetWindowIdsByCoordinate(
        displayId, windowNumber, x, y, windowIds);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "get windowIds by coordinate failed");
    }
    return ret;
}

WMError WindowManager::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    WMError ret = WindowAdapter::GetInstance(userId_).UpdateScreenLockStatusForApp(bundleName, isRelease);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "update screen lock status failed");
    }
    return ret;
}

WMError WindowManager::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}

WMError WindowManager::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetGlobalDragResizeType(dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManager::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetGlobalDragResizeType(dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManager::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetAppDragResizeType(bundleName, dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManager::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
{
    WMError ret = WindowAdapter::GetInstance(userId_).GetAppDragResizeType(bundleName, dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManager::SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManager::ShiftAppWindowPointerEvent(int32_t sourceWindowId, int32_t targetWindowId, int32_t fingerId)
{
    WMError ret = WindowAdapter::GetInstance(userId_).ShiftAppWindowPointerEvent(
        sourceWindowId, targetWindowId, fingerId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "failed");
    }
    return ret;
}

WMError WindowManager::NotifyScreenshotEvent(ScreenshotEventType type)
{
    WMError ret = WindowAdapter::GetInstance(userId_).NotifyScreenshotEvent(type);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}

WMError WindowManager::SetStartWindowBackgroundColor(
    const std::string& moduleName, const std::string& abilityName, uint32_t color)
{
    int32_t uid = static_cast<int32_t>(getuid());
    WMError ret = WindowAdapter::GetInstance(userId_).SetStartWindowBackgroundColor(
        moduleName, abilityName, color, uid);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PATTERN, "failed");
    }
    return ret;
}

WMError WindowManager::RequestFocus(int32_t persistentId, bool isFocused,
    bool byForeground, WindowFocusChangeReason reason)
{
    int32_t curReason = static_cast<int32_t>(reason);
    int32_t reasonStart = static_cast<int32_t>(FocusChangeReason::DEFAULT);
    int32_t reasonEnd = static_cast<int32_t>(FocusChangeReason::MAX);
    if (curReason < reasonStart || curReason > reasonEnd) {
        TLOGE(WmsLogTag::WMS_FOCUS, "could not find focus reason");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    WMError ret = WindowAdapter::GetInstance(userId_).RequestFocusStatusBySA(persistentId,
        isFocused, byForeground, static_cast<FocusChangeReason>(curReason));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "failed");
    }
    return ret;
}

WMError WindowManager::MinimizeByWindowId(const std::vector<int32_t>& windowIds)
{
    WMError ret = WindowAdapter::GetInstance(userId_).MinimizeByWindowId(windowIds);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed");
    }
    return ret;
}

WMError WindowManager::SetForegroundWindowNum(uint32_t windowNum)
{
    WMError ret = WindowAdapter::GetInstance(userId_).SetForegroundWindowNum(windowNum);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "failed");
    }
    return ret;
}

WMError WindowManager::ProcessRegisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    switch (observedInfo) {
        case WindowInfoKey::VISIBILITY_STATE :
            return RegisterVisibilityStateChangedListener(listener);
        case WindowInfoKey::DISPLAY_ID :
            return RegisterDisplayIdChangedListener(listener);
        case WindowInfoKey::WINDOW_RECT :
            return RegisterRectChangedListener(listener);
        case WindowInfoKey::WINDOW_MODE :
            return RegisterWindowModeChangedListenerForPropertyChange(listener);
        case WindowInfoKey::FLOATING_SCALE :
            return RegisterFloatingScaleChangedListener(listener);
        case WindowInfoKey::MID_SCENE :
            return RegisterMidSceneChangedListener(listener);
        case WindowInfoKey::WINDOW_GLOBAL_RECT :
            return RegisterGlobalRectChangedListener(listener);
        default:
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Invalid observedInfo: %{public}d", static_cast<uint32_t>(observedInfo));
            return WMError::WM_ERROR_INVALID_PARAM;
    }
}

WMError WindowManager::ProcessUnregisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    switch (observedInfo) {
        case WindowInfoKey::VISIBILITY_STATE :
            return UnregisterVisibilityStateChangedListener(listener);
        case WindowInfoKey::DISPLAY_ID :
            return UnregisterDisplayIdChangedListener(listener);
        case WindowInfoKey::WINDOW_RECT :
            return UnregisterRectChangedListener(listener);
        case WindowInfoKey::WINDOW_MODE :
            return UnregisterWindowModeChangedListenerForPropertyChange(listener);
        case WindowInfoKey::FLOATING_SCALE :
            return UnregisterFloatingScaleChangedListener(listener);
        case WindowInfoKey::MID_SCENE :
            return UnregisterMidSceneChangedListener(listener);
        case WindowInfoKey::WINDOW_GLOBAL_RECT :
            return UnregisterGlobalRectChangedListener(listener);
        default:
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Invalid observedInfo: %{public}d", static_cast<uint32_t>(observedInfo));
            return WMError::WM_ERROR_INVALID_PARAM;
    }
}

WMError WindowManager::RegisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    std::ostringstream observedInfoForLog;
    observedInfoForLog << "ObservedInfo: ";
    auto ret = WMError::WM_OK;
    if (!listener) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    for (const auto& info : observedInfo) {
        observedInfoForLog << static_cast<uint32_t>(info) << ", ";
        if (listener->GetInterestInfo().find(info) == listener->GetInterestInfo().end()) {
            listener->AddInterestInfo(info);
        }
        if (!listener->GetInterestWindowIds().empty()) {
            listener->AddInterestInfo(WindowInfoKey::WINDOW_ID);
        }
        ret = ProcessRegisterWindowInfoChangeCallback(info, listener);
        if (ret != WMError::WM_OK) {
            observedInfoForLog << "failed";
            break;
        }
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s", observedInfoForLog.str().c_str());
    return ret;
}

WMError WindowManager::UnregisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    std::ostringstream observedInfoForLog;
    observedInfoForLog << "ObservedInfo: ";
    auto ret = WMError::WM_OK;
    if (!listener) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    for (const auto& info : observedInfo) {
        observedInfoForLog << static_cast<uint32_t>(info) << ", ";
        if (listener->GetInterestInfo().find(info) == listener->GetInterestInfo().end()) {
            listener->AddInterestInfo(info);
        }
        if (!listener->GetInterestWindowIds().empty()) {
            listener->AddInterestInfo(WindowInfoKey::WINDOW_ID);
        }
        ret = ProcessUnregisterWindowInfoChangeCallback(info, listener);
        if (ret != WMError::WM_OK) {
            observedInfoForLog << "failed";
            break;
        }
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s", observedInfoForLog.str().c_str());
    return ret;
}

WMError WindowManager::RegisterWindowSupportRotationListener(const sptr<IWindowSupportRotationListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        WMError ret = WMError::WM_OK;
        if (pImpl_->windowSupportRotationListenerAgent_ == nullptr) {
            pImpl_->windowSupportRotationListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr(userId_);
        }
        ret = WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_SUPPORT_ROTATION,
            pImpl_->windowSupportRotationListenerAgent_);
        if (ret == WMError::WM_ERROR_SAMGR) {
            ret = ActiveFaultAgentReregister(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_SUPPORT_ROTATION,
                pImpl_->windowSupportRotationListenerAgent_);
        }
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_ROTATION, "RegisterWindowManagerAgent failed.");
        } else {
            if (pImpl_->windowSupportRotationListeners_.count(listener)) {
                TLOGE(WmsLogTag::WMS_ROTATION, "Listener is already registered.");
                return WMError::WM_OK;
            }
            pImpl_->windowSupportRotationListeners_.insert(listener);
        }
    }

    WMError ret = WindowAdapter::GetInstance(userId_).NotifySupportRotationRegistered();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "NotifySupportRotationRegistered failed");
    }
    return ret;
}

WMError WindowManager::UnregisterWindowSupportRotationListener(const sptr<IWindowSupportRotationListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Listener can not be null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        pImpl_->windowSupportRotationListeners_.erase(listener);
        WMError ret = WMError::WM_OK;
        if (pImpl_->windowSupportRotationListeners_.empty() && pImpl_->windowSupportRotationListenerAgent_ != nullptr) {
            ret = WindowAdapter::GetInstance(userId_).UnregisterWindowManagerAgent(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_SUPPORT_ROTATION,
                pImpl_->windowSupportRotationListenerAgent_);
            if (ret == WMError::WM_OK) {
                pImpl_->windowSupportRotationListenerAgent_ = nullptr;
            }
        }
    }
    return WMError::WM_OK;
}

void WindowManager::NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo)
{
    pImpl_->NotifySupportRotationChange(supportRotationInfo);
}

void WindowManager::SetIsModuleHookOffToSet(const std::string& moduleName)
{
    isModuleHookOffSet_.insert(moduleName);
    TLOGI(WmsLogTag::WMS_LIFE, "%{public}s", moduleName.c_str());
}

bool WindowManager::GetIsModuleHookOffFromSet(const std::string& moduleName)
{
    return isModuleHookOffSet_.find(moduleName) != isModuleHookOffSet_.end();
}

bool WindowManager::IsModuleHookOff(bool isModuleAbilityHookEnd, const std::string& moduleName)
{
    if (isModuleAbilityHookEnd) {
        TLOGI(WmsLogTag::WMS_MAIN, "set IsModuleHookOff to set");
        SetIsModuleHookOffToSet(moduleName);
    }
    if (GetIsModuleHookOffFromSet(moduleName)) {
        TLOGI(WmsLogTag::WMS_MAIN, "set IsModuleHookOff true");
        return true;
    }
    return false;
}

void WindowManager::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList)
{
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::WINDOW_RECT)) {
        pImpl_->NotifyWindowRectChange(windowInfoList);
    }
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::DISPLAY_ID)) {
        pImpl_->NotifyDisplayIdChange(windowInfoList);
    }
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::WINDOW_MODE)) {
        pImpl_->NotifyWindowModeChangeForPropertyChange(windowInfoList);
    }
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::FLOATING_SCALE)) {
        pImpl_->NotifyFloatingScaleChange(windowInfoList);
    }
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::MID_SCENE)) {
        pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    }
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::WINDOW_GLOBAL_RECT)) {
        pImpl_->NotifyWindowGlobalRectChange(windowInfoList);
    }
}

WMError WindowManager::AnimateTo(int32_t windowId, WindowAnimationProperty animationProperty,
    WindowAnimationOption animationOption)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "In, windowId: %{public}d, targetScale: %{public}f, animationOption: %{public}s",
        windowId, animationProperty.targetScale, animationOption.ToString().c_str());
    WMError ret = WindowAdapter::GetInstance(userId_).AnimateTo(windowId, animationProperty, animationOption);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed, errorCode: %{public}d", static_cast<int32_t>(ret));
    }
    return ret;
}

WMError WindowManager::RegisterWindowLifeCycleCallback(const sptr<IWindowLifeCycleListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "window lifecycle listener registered could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Register window lifecycle listener");
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (pImpl_->windowLifeCycleListener_) {
            TLOGI(WmsLogTag::WMS_LIFE, "WindowLifeCycleListener is already registered, do nothing");
            return WMError::WM_OK;
        }
        pImpl_->windowLifeCycleListener_ = listener;
    }
 
    return WMError::WM_OK;
}

WMError WindowManager::UnregisterWindowLifeCycleCallback(const sptr<IWindowLifeCycleListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "window lifecycle listener unregistered could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Unregister window lifecycle listener");
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        pImpl_->windowLifeCycleListener_ = nullptr;
    }

    return WMError::WM_OK;
}

void WindowManager::RegisterGetJSWindowCallback(GetJSWindowObjFunc&& getJSWindowFunc)
{
    getJSWindowObjFunc_ = std::move(getJSWindowFunc);
}

void WindowManager::NotifyWMSWindowDestroyed(const WindowLifeCycleInfo& lifeCycleInfo)
{
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (pImpl_->windowLifeCycleListener_ == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "window destroyed listener is nullptr");
            return;
        }
    }
    void* jsWindowNapiValue = nullptr;
    if (getJSWindowObjFunc_ != nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "window name: %{public}s, window id: %{public}d", lifeCycleInfo.windowName.c_str(),
            lifeCycleInfo.windowId);
        jsWindowNapiValue = getJSWindowObjFunc_(lifeCycleInfo.windowName);
    }
    pImpl_->NotifyWMSWindowDestroyed(lifeCycleInfo, jsWindowNapiValue);
}

WMError WindowManager::AddSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "size: [%{public}zu, %{public}zu]", bundleNames.size(), privacyWindowTags.size());
    auto ret = WindowAdapter::GetInstance(userId_).AddSessionBlackList(bundleNames, privacyWindowTags);
    return ret;
}

WMError WindowManager::RemoveSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "size: [%{public}zu, %{public}zu]", bundleNames.size(), privacyWindowTags.size());
    auto ret = WindowAdapter::GetInstance(userId_).RemoveSessionBlackList(bundleNames, privacyWindowTags);
    return ret;
}

WMError WindowManager::CheckOutlineParams(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams)
{
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "The remote object is null.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (outlineParams.type_ != OutlineType::OUTLINE_FOR_WINDOW) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Can update outline for windows only.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (outlineParams.outlineStyleParams_.outlineWidth_ < OUTLINE_WIDTH_MIN ||
        outlineParams.outlineStyleParams_.outlineWidth_ > OUTLINE_WIDTH_MAX) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Outline width: %{public}d is out of valid range.",
            outlineParams.outlineStyleParams_.outlineWidth_);
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (outlineParams.outlineStyleParams_.outlineShape_ >= OutlineShape::OUTLINE_SHAPE_END) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Invalid outline shape: %{public}d.",
              outlineParams.outlineStyleParams_.outlineShape_);
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if ((outlineParams.outlineStyleParams_.outlineColor_ >> OUTLINE_COLOR_OPAQUE_OFFSET) != 0 &&
        (outlineParams.outlineStyleParams_.outlineColor_ >> OUTLINE_COLOR_OPAQUE_OFFSET) != OUTLINE_COLOR_OPAQUE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Invalid outline color: %{public}d",
              outlineParams.outlineStyleParams_.outlineColor_);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

WMError WindowManager::UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "%{public}s", outlineParams.ToString().c_str());
    auto ret = CheckOutlineParams(remoteObject, outlineParams);
    if (ret !=WMError::WM_OK) {
        return ret;
    }

    ret = WindowAdapter::GetInstance(userId_).UpdateOutline(remoteObject, outlineParams);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Update outline failed, ret: %{public}d.", ret);
        return ret;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (outlineParams.persistentIds_.size() == 0) {
        WindowAdapter::GetInstance(userId_).UnregisterOutlineRecoverCallbackFunc();
        isOutlineRecoverRegistered_ = false;
        TLOGI(WmsLogTag::WMS_ANIMATION, "No window need to be highlight.");
        return ret;
    }

    outlineRemoteObject_ = remoteObject;
    outlineParams_ = outlineParams;
    if (isOutlineRecoverRegistered_) {
        TLOGI(WmsLogTag::WMS_ANIMATION, "Outline recover callback has already been registered.");
        return ret;
    }

    WindowAdapter::GetInstance(userId_).RegisterOutlineRecoverCallbackFunc([weakThis = wptr(this)]() -> WMError {
        auto windowManager = weakThis.promote();
        if (!windowManager) {
            TLOGE(WmsLogTag::WMS_SCB, "window adapter is null");
            return WMError::WM_DO_NOTHING;
        }
        sptr<IRemoteObject> remoteObject;
        OutlineParams outlineParams;
        {
            std::lock_guard<std::recursive_mutex> lock(windowManager->mutex_);
            remoteObject = windowManager->outlineRemoteObject_;
            outlineParams = windowManager->outlineParams_;
        }
        return windowManager->UpdateOutline(remoteObject, outlineParams);
    });
    isOutlineRecoverRegistered_ = true;
    return ret;
}
} // namespace Rosen
} // namespace OHOS