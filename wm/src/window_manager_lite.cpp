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

#include "window_manager_lite.h"

#include <algorithm>
#include <cinttypes>

#include "load_mmi_client_adapter.h"
#include "marshalling_helper.h"
#include "window_adapter_lite.h"
#include "window_manager_agent_lite.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "parameters.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManagerLite"};
}
std::unordered_map<int32_t, sptr<WindowManagerLite>> WindowManagerLite::windowManagerLiteMap_ = {};
std::shared_mutex WindowManagerLite::windowManagerLiteMapMutex_;

class WindowManagerLite::Impl {
public:
    explicit Impl(std::recursive_mutex& mutex) : mutex_(mutex) {}
    void NotifyFocused(uint32_t windowId, const sptr<IRemoteObject>& abilityToken,
        WindowType windowType, DisplayId displayId);
    void NotifyUnfocused(uint32_t windowId, const sptr<IRemoteObject>& abilityToken,
        WindowType windowType, DisplayId displayId);
    void NotifyFocused(const sptr<FocusChangeInfo>& focusChangeInfo);
    void NotifyDisplayGroupInfoChanged(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd);
    void NotifyUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo);
    void NotifyWindowVisibilityInfoChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos);
    void NotifyWindowVisibilityStateChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos);
    void NotifyDisplayIdChange(const WindowInfoList& windowInfoList);
    void NotifyMidSceneStatusChange(const WindowInfoList& windowInfoList);
    WindowInfoList GetWindowInfoListByInterestWindowIds(
        const sptr<IWindowInfoChangedListener>& listener, const WindowInfoList& windowInfoList);
    void PackWindowChangeInfo(const std::unordered_set<WindowInfoKey>& interestInfo,
        const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, WindowInfoList& windowChangeInfos);
    void NotifyWindowDrawingContentInfoChanged(const std::vector<sptr<WindowDrawingContentInfo>>&
        windowDrawingContentInfos);
    void NotifyWindowModeChange(WindowModeType type);
    void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing);
    void NotifyWMSConnected(int32_t userId, int32_t screenId, int32_t pid = INVALID_PID);
    void NotifyWMSDisconnected(int32_t userId, int32_t screenId, int32_t pid = INVALID_PID);
    void NotifyWindowStyleChange(WindowStyleType type);
    void NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo);
    void UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground);
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
        WindowUpdateType type);

    /**
     * Compatible with the singleton pattern, ensuring that calls can
     * still be made through SingletonContainer::Get<WindowManagerLite>
     */
    static inline SingletonDelegator<WindowManagerLite> delegator_;

    std::recursive_mutex& mutex_;
    template<typename T>
    using ListenerSet = std::unordered_set<sptr<T>, SptrHash<T>>;

    // Focus
    ListenerSet<IFocusChangedListener> focusChangedListeners_;
    sptr<WindowManagerAgentLite> focusChangedListenerAgent_;
    std::recursive_mutex focusChangedMutex_;

    // Window update
    ListenerSet<IWindowUpdateListener> windowUpdateListeners_;
    sptr<WindowManagerAgentLite> windowUpdateListenerAgent_;

    // Window visibility
    ListenerSet<IVisibilityChangedListener> windowVisibilityListeners_;
    sptr<WindowManagerAgentLite> windowVisibilityListenerAgent_;

    // Window visibility state
    ListenerSet<IWindowInfoChangedListener> windowVisibilityStateListeners_;
    sptr<WindowManagerAgentLite> windowVisibilityStateListenerAgent_;

    ListenerSet<IWindowInfoChangedListener> windowDisplayIdChangeListeners_;
    ListenerSet<IWindowInfoChangedListener> midSceneStatusChangeListeners_;
    sptr<WindowManagerAgentLite> windowPropertyChangeAgent_;

    // Window drawing content
    ListenerSet<IDrawingContentChangedListener> windowDrawingContentListeners_;
    sptr<WindowManagerAgentLite> windowDrawingContentListenerAgent_;

    // Window mode
    ListenerSet<IWindowModeChangedListener> windowModeListeners_;
    sptr<WindowManagerAgentLite> windowModeListenerAgent_;

    // Camera window
    ListenerSet<ICameraWindowChangedListener> cameraWindowChangedListeners_;
    sptr<WindowManagerAgentLite> cameraWindowChangedListenerAgent_;

    // Window style
    ListenerSet<IWindowStyleChangedListener> windowStyleListeners_;
    sptr<WindowManagerAgentLite> windowStyleListenerAgent_;

    // PiP state
    ListenerSet<IPiPStateChangedListener> pipStateChangedListeners_;
    sptr<WindowManagerAgentLite> pipStateChangedListenerAgent_;

    // All group info
    ListenerSet<IAllGroupInfoChangedListener> allGroupInfoChangedListeners_;
    sptr<WindowManagerAgentLite> allGroupInfoChangedListenerAgent_;

    // WMS Connection listener
    sptr<IWMSConnectionChangedListener> wmsConnectionChangedListener_;
    std::mutex wmsConnectionChangedMutex_;

    // Keyboard
    using CallingDisplayChangedListeners = std::unordered_set<sptr<IKeyboardCallingWindowDisplayChangedListener>,
        SptrHash<IKeyboardCallingWindowDisplayChangedListener>>;
    CallingDisplayChangedListeners callingDisplayChangedListeners_;
    sptr<WindowManagerAgentLite> callingDisplayListenerAgent_;
    std::mutex callingDisplayChangedMutex_;
};

void WindowManagerLite::Impl::NotifyWMSConnected(int32_t userId, int32_t screenId, int32_t pid)
{
    sptr<IWMSConnectionChangedListener> listener = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionChangedMutex_);
        if (!wmsConnectionChangedListener_) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                  "listener is null, userId=%{public}d, screenId=%{public}d",
                  userId,
                  screenId);
            return;
        }
        listener = wmsConnectionChangedListener_;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "WMS on connected, userId=%{public}d, screenId=%{public}d, pid=%{public}d",
        userId, screenId, pid);
    listener->OnConnected(userId, screenId, pid);
}

void WindowManagerLite::Impl::NotifyWMSDisconnected(int32_t userId, int32_t screenId, int32_t pid)
{
    sptr<IWMSConnectionChangedListener> listener = nullptr;
    {
        std::lock_guard<std::mutex> lock(wmsConnectionChangedMutex_);
        if (!wmsConnectionChangedListener_) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                  "listener is null, userId=%{public}d, screenId=%{public}d",
                  userId,
                  screenId);
            return;
        }
        listener = wmsConnectionChangedListener_;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "WMS on disconnected, userId=%{public}d, screenId=%{public}d, pid=%{public}d",
        userId, screenId, pid);
    listener->OnDisconnected(userId, screenId, pid);
}

void WindowManagerLite::Impl::NotifyFocused(const sptr<FocusChangeInfo>& focusChangeInfo)
{
    if (focusChangeInfo == nullptr) {
        return;
    }
    WLOGFD("[WMSFocus]NotifyFocused [%{public}u; %{public}" PRIu64"; %{public}d; %{public}d; %{public}u]",
        focusChangeInfo->windowId_, focusChangeInfo->displayId_, focusChangeInfo->pid_, focusChangeInfo->uid_,
        static_cast<uint32_t>(focusChangeInfo->windowType_));

    std::vector<sptr<IFocusChangedListener>> focusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(focusChangedMutex_);
        focusChangeListeners.assign(focusChangedListeners_.begin(), focusChangedListeners_.end());
    }
    WLOGFD("NotifyFocused listeners: %{public}zu", focusChangeListeners.size());
    for (auto& listener : focusChangeListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnFocused(focusChangeInfo);
    }
}

void WindowManagerLite::Impl::NotifyUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo)
{
    if (focusChangeInfo == nullptr) {
        return;
    }
    WLOGFD("[WMSFocus]NotifyUnfocused [%{public}u; %{public}" PRIu64"; %{public}d; %{public}d; %{public}u]",
        focusChangeInfo->windowId_, focusChangeInfo->displayId_, focusChangeInfo->pid_, focusChangeInfo->uid_,
        static_cast<uint32_t>(focusChangeInfo->windowType_));

    std::vector<sptr<IFocusChangedListener>> focusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(focusChangedMutex_);
        focusChangeListeners.assign(focusChangedListeners_.begin(), focusChangedListeners_.end());
    }
    WLOGFD("NotifyUnfocused listeners: %{public}zu", focusChangeListeners.size());
    for (auto& listener : focusChangeListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnUnfocused(focusChangeInfo);
    }
}

void WindowManagerLite::Impl::NotifyDisplayGroupInfoChanged(DisplayGroupId displayGroupId, DisplayId displayId,
                                                            bool isAdd)
{
    std::vector<sptr<IAllGroupInfoChangedListener>> allGroupInfoChangedListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        allGroupInfoChangedListeners.assign(allGroupInfoChangedListeners_.begin(), allGroupInfoChangedListeners_.end());
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "listeners num: %{public}zu",
          allGroupInfoChangedListeners.size());
    for (auto& listener: allGroupInfoChangedListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnDisplayGroupInfoChange(displayGroupId, displayId, isAdd);
    }
}

void WindowManagerLite::Impl::NotifyWindowVisibilityInfoChanged(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos)
{
    std::vector<sptr<IVisibilityChangedListener>> visibilityChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        visibilityChangeListeners.assign(windowVisibilityListeners_.begin(), windowVisibilityListeners_.end());
    }
    for (auto& listener : visibilityChangeListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnWindowVisibilityChanged(windowVisibilityInfos);
    }
}

void WindowManagerLite::Impl::NotifyWindowVisibilityStateChanged(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    std::vector<sptr<IWindowInfoChangedListener>> windowVisibilityStateListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowVisibilityStateListeners.assign(
            windowVisibilityStateListeners_.begin(), windowVisibilityStateListeners_.end());
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

void WindowManagerLite::Impl::NotifyMidSceneStatusChange(const WindowInfoList& windowInfoList)
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

void WindowManagerLite::Impl::NotifyDisplayIdChange(const WindowInfoList& windowInfoList)
{
    std::vector<sptr<IWindowInfoChangedListener>> windowDisplayIdChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowDisplayIdChangeListeners.assign(
            windowDisplayIdChangeListeners_.begin(), windowDisplayIdChangeListeners_.end());
    }
    for (auto& listener : windowDisplayIdChangeListeners) {
        WindowInfoList windowInfoListForNotify = GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
        if (listener != nullptr && !windowInfoListForNotify.empty()) {
            listener->OnWindowInfoChanged(windowInfoListForNotify);
        }
    }
}

WindowInfoList WindowManagerLite::Impl::GetWindowInfoListByInterestWindowIds(
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

void WindowManagerLite::Impl::PackWindowChangeInfo(const std::unordered_set<WindowInfoKey>& interestInfo,
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

void WindowManagerLite::Impl::NotifyWindowDrawingContentInfoChanged(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    std::vector<sptr<IDrawingContentChangedListener>> windowDrawingContentChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowDrawingContentChangeListeners.assign(
            windowDrawingContentListeners_.begin(), windowDrawingContentListeners_.end());
    }
    for (auto& listener : windowDrawingContentChangeListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnWindowDrawingContentChanged(windowDrawingContentInfos);
    }
}

void WindowManagerLite::Impl::NotifyWindowModeChange(WindowModeType type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "WindowManager::Impl UpdateWindowModeTypeInfo type: %{public}d",
          static_cast<uint8_t>(type));
    std::vector<sptr<IWindowModeChangedListener>> windowModeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowModeListeners.assign(windowModeListeners_.begin(), windowModeListeners_.end());
    }
    for (auto &listener : windowModeListeners) {
        listener->OnWindowModeUpdate(type);
    }
}

void WindowManagerLite::Impl::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
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
        TLOGD(WmsLogTag::WMS_MAIN, "wid[%{public}u], innerWid[%{public}u], "
            "uiNodeId[%{public}u], rect[%{public}d %{public}d %{public}d %{public}d], "
            "isFocused[%{public}d], isDecorEnable[%{public}d], displayId[%{public}" PRIu64 "], layer[%{public}u], "
            "mode[%{public}u], type[%{public}u, updateType[%{public}d], bundle[%{public}s]",
            info->wid_, info->innerWid_, info->uiNodeId_, info->windowRect_.width_, info->windowRect_.height_,
            info->windowRect_.posX_, info->windowRect_.posY_, info->focused_, info->isDecorEnable_, info->displayId_,
            info->layer_, info->mode_, info->type_, type, info->bundleName_.c_str());
        for (const auto& rect : info->touchHotAreas_) {
            TLOGD(WmsLogTag::WMS_MAIN, "window touch hot areas rect[x=%{public}d, y=%{public}d, "
            "w=%{public}d, h=%{public}d]", rect.posX_, rect.posY_, rect.width_, rect.height_);
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

void WindowManagerLite::Impl::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    TLOGI(WmsLogTag::WMS_PIP, "Camera window, accessTokenId=%{public}u, isShowing=%{public}u",
        accessTokenId, isShowing);
    std::vector<sptr<ICameraWindowChangedListener>> cameraWindowChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        cameraWindowChangeListeners.assign(
            cameraWindowChangedListeners_.begin(), cameraWindowChangedListeners_.end());
    }
    for (auto& listener : cameraWindowChangeListeners) {
        listener->OnCameraWindowChange(accessTokenId, isShowing);
    }
}

void WindowManagerLite::Impl::NotifyWindowStyleChange(WindowStyleType type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "WindowStyleChange: %{public}d",
          static_cast<uint8_t>(type));
    std::vector<sptr<IWindowStyleChangedListener>> windowStyleListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowStyleListeners.assign(windowStyleListeners_.begin(), windowStyleListeners_.end());
    }
    for (auto &listener : windowStyleListeners) {
        TLOGI(WmsLogTag::WMS_MAIN, "real WindowStyleChange type: %{public}d",
              static_cast<uint8_t>(type));
        listener->OnWindowStyleUpdate(type);
    }
}

void WindowManagerLite::Impl::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    CallingDisplayChangedListeners listeners;
    {
        std::lock_guard<std::mutex> lock(callingDisplayChangedMutex_);
        listeners = callingDisplayChangedListeners_;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "notify persistentId: %{public}d, pid: %{public}d, "
        "displayId: %{public}" PRIu64" , userId: %{public}d, size: %{public}zu",
        callingWindowInfo.windowId_, callingWindowInfo.callingPid_, callingWindowInfo.displayId_,
        callingWindowInfo.userId_, listeners.size());

    for (const auto& listener : listeners) {
        if (!listener) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "listener is null");
            continue;
        }
        listener->OnCallingWindowDisplayChanged(callingWindowInfo);
    }
}

void WindowManagerLite::Impl::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground)
{
    std::vector<sptr<IPiPStateChangedListener>> pipStateChangedListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        pipStateChangedListeners.assign(pipStateChangedListeners_.begin(), pipStateChangedListeners_.end());
    }
    for (auto& listener : pipStateChangedListeners) {
        if (listener == nullptr) {
            continue;
        }
        listener->OnPiPStateChanged(bundleName, isForeground);
    }
}

WindowManagerLite::WindowManagerLite(const int32_t userId) : pImpl_(std::make_unique<Impl>(mutex_)), userId_(userId) {}

WindowManagerLite::~WindowManagerLite()
{
    TLOGI(WmsLogTag::WMS_SCB, "destroyed, userId: %{public}d", userId_);
}

WindowManagerLite& WindowManagerLite::GetInstance()
{
    static auto instance = sptr<WindowManagerLite>::MakeSptr();
    return *instance;
}

WindowManagerLite& WindowManagerLite::GetInstance(const int32_t userId)
{
    /**
     * Only system applications or services with a userId of 0 are allowed to communicate
     * with multiple WMS-Servers and are permitted to listen for WMS connection status.
     */
    int32_t clientUserId = GetUserIdByUid(getuid());
    if (clientUserId != SYSTEM_USERID || userId <= INVALID_USER_ID) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "get default instance , userId: %{public}d", userId);
        return GetInstance();
    }

    if (!WindowManagerLite::IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "get default instance in multi, userId: %{public}d", userId);
        return GetInstance();
    }
    /**
     * multi-instance mode
     * At present, map does not have memory leak issues. In actual business scenarios,
     * number of instances created is limited (estimated to be less than 20).
     */
    std::unique_lock<std::shared_mutex> lock(windowManagerLiteMapMutex_);
    auto iter = windowManagerLiteMap_.find(userId);
    if (iter != windowManagerLiteMap_.end() && iter->second) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "get existing instance, userId: %{public}d", userId);
        return *iter->second;
    }
    if (windowManagerLiteMap_.size() >= MAX_INSTANCE_NUM) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Can not create a new instance that limited by MAX_INSTANCE_NUM");
        return GetInstance();
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    auto instance = sptr<WindowManagerLite>::MakeSptr(userId);
    windowManagerLiteMap_.insert({ userId, instance });
    TLOGI(WmsLogTag::WMS_MULTI_USER,
        "After insert, map size: %{public}zu, userId: %{public}d, instance ptr: %{public}p",
        windowManagerLiteMap_.size(), userId, instance.GetRefPtr());
    return *instance;
}

WMError WindowManagerLite::RemoveInstanceByUserId(const int32_t userId)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "Remove instance, userId=%{public}d", userId);
    std::unique_lock<std::shared_mutex> lock(windowManagerLiteMapMutex_);
    windowManagerLiteMap_.erase(userId);
    return WMError::WM_OK;
}

bool WindowManagerLite::IsMultiInstanceEnabled()
{
    static bool enabled = [] {
        bool isConcurrentUser = system::GetBoolParameter("persist.dms.concurrentuser", true);
        TLOGNI(WmsLogTag::WMS_SCB, "isConcurrentUser: %{public}d", isConcurrentUser);
        return isConcurrentUser;
    }();
    return enabled;
}

WMError WindowManagerLite::ActiveFaultAgentReregister(const WindowManagerAgentType type,
    const sptr<WindowManagerAgentLite>& agent)
{
    if (!WindowAdapterLite::GetInstance(userId_).IsMockSMSProxyAlive()) {
        TLOGE(WmsLogTag::DEFAULT, "SA not alive, fault recovery unavailable");
        return WMError::WM_ERROR_SAMGR;
    }
    auto isWMSProxyValid = WindowAdapterLite::GetInstance(userId_).IsWindowManagerServiceProxyValid();
    if (isWMSProxyValid) {
        TLOGE(WmsLogTag::DEFAULT, "Not activated, scb is ready");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    TLOGI(WmsLogTag::DEFAULT, "Activate the agent re-register solution, type: %{public}d", type);
    WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgentWhenSCBFault(type, agent);
    return WMError::WM_OK;
}

WMError WindowManagerLite::RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->focusChangedMutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->focusChangedListenerAgent_ == nullptr) {
        pImpl_->focusChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
        if (ret == WMError::WM_ERROR_SAMGR) {
            ret = ActiveFaultAgentReregister(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
        }
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed !");
        pImpl_->focusChangedListenerAgent_ = nullptr;
    } else {
        if (pImpl_->focusChangedListeners_.count(listener)) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->focusChangedListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->focusChangedMutex_);
    if (pImpl_->focusChangedListeners_.erase(listener) == 0) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->focusChangedListeners_.empty() && pImpl_->focusChangedListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->focusChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::RegisterAllGroupInfoChangedListener(const sptr<IAllGroupInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->allGroupInfoChangedListenerAgent_ == nullptr) {
        pImpl_->allGroupInfoChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO,
            pImpl_->allGroupInfoChangedListenerAgent_);
        if (ret == WMError::WM_ERROR_SAMGR) {
            ret = ActiveFaultAgentReregister(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO,
                pImpl_->allGroupInfoChangedListenerAgent_);
        }
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_FOCUS, "register WindowManagerAgent failed");
        pImpl_->allGroupInfoChangedListenerAgent_ = nullptr;
    } else {
        if (pImpl_->allGroupInfoChangedListeners_.count(listener)) {
            TLOGW(WmsLogTag::WMS_FOCUS, "listener is already registered");
            return WMError::WM_OK;
        }
        pImpl_->allGroupInfoChangedListeners_.insert(listener);
        TLOGI(WmsLogTag::WMS_FOCUS, "success");
    }
    return ret;
}

WMError WindowManagerLite::UnregisterAllGroupInfoChangedListener(const sptr<IAllGroupInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->allGroupInfoChangedListeners_.erase(listener) == 0) {
        TLOGE(WmsLogTag::WMS_FOCUS, "could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->allGroupInfoChangedListeners_.empty() && pImpl_->allGroupInfoChangedListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO,
            pImpl_->allGroupInfoChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->allGroupInfoChangedListenerAgent_ = nullptr;
            TLOGI(WmsLogTag::WMS_FOCUS, "success");
        }
    }
    return ret;
}

WMError WindowManagerLite::RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowVisibilityListenerAgent_ == nullptr) {
        pImpl_->windowVisibilityListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityListenerAgent_);
        if (ret == WMError::WM_ERROR_SAMGR) {
            ret = ActiveFaultAgentReregister(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
                pImpl_->windowVisibilityListenerAgent_);
        }
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed !");
        pImpl_->windowVisibilityListenerAgent_ = nullptr;
    } else {
        if (pImpl_->windowVisibilityListeners_.count(listener)) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowVisibilityListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowVisibilityListeners_.erase(listener);

    WMError ret = WMError::WM_OK;
    if (pImpl_->windowVisibilityListeners_.empty() && pImpl_->windowVisibilityListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowVisibilityListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::RegisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "lite in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
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
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
        WindowInfoKey::MID_SCENE, interestInfo, pImpl_->windowPropertyChangeAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY,
                                         pImpl_->windowPropertyChangeAgent_);
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

WMError WindowManagerLite::RegisterDisplayIdChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->windowPropertyChangeAgent_ == nullptr) {
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
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
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowPropertyChangeAgent(
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
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowDisplayIdChangeListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "lite in");
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
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(
            WindowInfoKey::MID_SCENE, interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::UnregisterDisplayIdChangedListener(const sptr<IWindowInfoChangedListener>& listener)
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
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowPropertyChangeAgent(
            WindowInfoKey::DISPLAY_ID, interestInfo, pImpl_->windowPropertyChangeAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowPropertyChangeAgent_ = nullptr;
        }
    }
    return ret;
}

void WindowManagerLite::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "lite flags=%{public}u", propertyDirtyFlags);
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::DISPLAY_ID)) {
        pImpl_->NotifyDisplayIdChange(windowInfoList);
    }
    if (propertyDirtyFlags & static_cast<int32_t>(WindowInfoKey::MID_SCENE)) {
        pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    }
}

WMError WindowManagerLite::RegisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowVisibilityStateListenerAgent_ == nullptr) {
        pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    }
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
        pImpl_->windowVisibilityStateListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityStateListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowManagerAgent failed!");
        pImpl_->windowVisibilityStateListenerAgent_ = nullptr;
    } else {
        if (pImpl_->windowVisibilityStateListeners_.count(listener)) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowVisibilityStateListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowVisibilityStateListeners_.erase(listener);

    WMError ret = WMError::WM_OK;
    if (pImpl_->windowVisibilityStateListeners_.empty() && pImpl_->windowVisibilityStateListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityStateListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowVisibilityStateListenerAgent_ = nullptr;
        }
    }
    return ret;
}

void WindowManagerLite::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    WLOGFD("In");
    WindowAdapterLite::GetInstance(userId_).GetFocusWindowInfo(focusInfo, displayId);
}

void WindowManagerLite::GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                        std::vector<sptr<FocusChangeInfo>>& allFocusInfoList)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "In");
    WindowAdapterLite::GetInstance(userId_).GetAllGroupInfo(displayId2GroupIdMap, allFocusInfoList);
}

void WindowManagerLite::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) const
{
    if (focusChangeInfo == nullptr) {
        WLOGFE("focusChangeInfo is nullptr.");
        return;
    }
    WLOGFD("[WMSFocus]window focus change: %{public}d, id: %{public}u", focused, focusChangeInfo->windowId_);
    if (focused) {
        pImpl_->NotifyFocused(focusChangeInfo);
    } else {
        pImpl_->NotifyUnfocused(focusChangeInfo);
    }
}

void WindowManagerLite::UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd) const
{
    if (displayGroupId == DISPLAY_GROUP_ID_INVALID || displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_FOCUS, "displayGroupId or displayId is invalid");
        return;
    }
    pImpl_->NotifyDisplayGroupInfoChanged(displayGroupId, displayId, isAdd);
}

void WindowManagerLite::UpdateWindowVisibilityInfo(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos) const
{
    pImpl_->NotifyWindowVisibilityInfoChanged(windowVisibilityInfos);
    pImpl_->NotifyWindowVisibilityStateChanged(windowVisibilityInfos);
}

WMError WindowManagerLite::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).GetVisibilityWindowInfo(infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window visibility info failed");
    }
    return ret;
}

void WindowManagerLite::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos) const
{
    pImpl_->NotifyWindowDrawingContentInfoChanged(windowDrawingContentInfos);
}

void WindowManagerLite::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing) const
{
    pImpl_->UpdateCameraWindowStatus(accessTokenId, isShowing);
}

void WindowManagerLite::OnRemoteDied()
{
    TLOGI(WmsLogTag::WMS_SCB, "wms is died");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    pImpl_->focusChangedListenerAgent_ = nullptr;
    pImpl_->windowUpdateListenerAgent_ = nullptr;
    pImpl_->windowVisibilityListenerAgent_ = nullptr;
    pImpl_->windowDrawingContentListenerAgent_ = nullptr;
}

WMError WindowManagerLite::RegisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowDrawingContentListenerAgent_ == nullptr) {
        pImpl_->windowDrawingContentListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
            pImpl_->windowDrawingContentListenerAgent_);
        if (ret == WMError::WM_ERROR_SAMGR) {
            ret = ActiveFaultAgentReregister(
                WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
                pImpl_->windowDrawingContentListenerAgent_);
        }
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed !");
        pImpl_->windowDrawingContentListenerAgent_ = nullptr;
    } else {
        if (pImpl_->windowDrawingContentListeners_.count(listener)) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowDrawingContentListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowDrawingContentListeners_.erase(listener);

    WMError ret = WMError::WM_OK;
    if (pImpl_->windowDrawingContentListeners_.empty() && pImpl_->windowDrawingContentListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
            pImpl_->windowDrawingContentListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowDrawingContentListenerAgent_ = nullptr;
        }
    }
    return ret;
}

void WindowManagerLite::UpdateWindowModeTypeInfo(WindowModeType type) const
{
    pImpl_->NotifyWindowModeChange(type);
}

void WindowManagerLite::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type) const
{
    pImpl_->NotifyAccessibilityWindowInfo(infos, type);
}

WMError WindowManagerLite::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).UpdateScreenLockStatusForApp(bundleName, isRelease);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "lite bundleName=%{public}s, isRelease=%{public}d, ret=%{public}d",
        bundleName.c_str(), isRelease, static_cast<int32_t>(ret));
    return ret;
}

WMError WindowManagerLite::GetWindowModeType(WindowModeType& windowModeType) const
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).GetWindowModeType(windowModeType);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window visibility info failed");
    }
    return ret;
}

WMError WindowManagerLite::UpdateAnimationSpeedWithPid(pid_t pid, float speed)
{
    // 0.f:less than 0.f is invalid, animation speed cannot be negative
    // 10.f:greater than 10.f is invalid, unexpected UI behavior
    if (speed < 0.0f || speed > 10.0f) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "update animation speed with invalid speed, speed = %{public}f", speed);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).UpdateAnimationSpeedWithPid(pid, speed);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "update animation speed with pid failed, pid = %{public}d", pid);
    }
    return ret;
}

WMError WindowManagerLite::RegisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->windowModeListenerAgent_ == nullptr) {
        pImpl_->windowModeListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
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

WMError WindowManagerLite::UnregisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->windowModeListeners_.erase(listener) == 0) {
        TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowModeListeners_.empty() && pImpl_->windowModeListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE, pImpl_->windowModeListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowModeListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::RegisterCameraWindowChangedListener(const sptr<ICameraWindowChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->cameraWindowChangedListenerAgent_ == nullptr) {
        pImpl_->cameraWindowChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_WINDOW, pImpl_->cameraWindowChangedListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_WINDOW,
            pImpl_->cameraWindowChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "RegisterWindowManagerAgent failed!");
        pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
    } else {
        if (pImpl_->cameraWindowChangedListeners_.count(listener)) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->cameraWindowChangedListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterCameraWindowChangedListener(const sptr<ICameraWindowChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->cameraWindowChangedListeners_.erase(listener) == 0) {
        TLOGE(WmsLogTag::WMS_PIP, "could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->cameraWindowChangedListeners_.empty() &&
        pImpl_->cameraWindowChangedListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_WINDOW,
            pImpl_->cameraWindowChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::RaiseWindowToTop(int32_t persistentId)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).RaiseWindowToTop(persistentId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "raise window to top failed.");
    }
    return ret;
}

WMError WindowManagerLite::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    TLOGI(WmsLogTag::WMS_MAIN, "Get main window info lite");
    return WindowAdapterLite::GetInstance(userId_).GetMainWindowInfos(topNum, topNInfo);
}

WMError WindowManagerLite::GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo)
{
    return WindowAdapterLite::GetInstance(userId_).GetCallingWindowInfo(callingWindowInfo);
}

WMError WindowManagerLite::RegisterWMSConnectionChangedListener(const sptr<IWMSConnectionChangedListener>& listener)
{
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
    auto ret = WindowAdapterLite::GetInstance(userId_).RegisterWMSConnectionChangedListener(
        [weakThis = wptr(this)](int32_t userId, int32_t screenId, bool isConnected, int32_t pid) {
            auto wml = weakThis.promote();
            if (!wml) {
                TLOGE(WmsLogTag::WMS_SCB, "window manager lite is null");
                return;
            }
            wml->OnWMSConnectionChanged(userId, screenId, isConnected, pid);
        });
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Register callback failed");
        std::lock_guard<std::mutex> lock(pImpl_->wmsConnectionChangedMutex_);
        pImpl_->wmsConnectionChangedListener_ = nullptr;
    }
    return ret;
}

WMError WindowManagerLite::UnregisterWMSConnectionChangedListener()
{
    WindowAdapterLite::GetInstance(userId_).UnregisterWMSConnectionChangedListener();
    {
        std::lock_guard<std::mutex> lock(pImpl_->wmsConnectionChangedMutex_);
        pImpl_->wmsConnectionChangedListener_ = nullptr;
    }
    TLOGI(WmsLogTag::WMS_MULTI_USER, "Unregister success");
    return WMError::WM_OK;
}

void WindowManagerLite::OnWMSConnectionChanged(int32_t userId, int32_t screenId, bool isConnected, int32_t pid) const
{
    if (isConnected) {
        pImpl_->NotifyWMSConnected(userId, screenId, pid);
    } else {
        pImpl_->NotifyWMSDisconnected(userId, screenId, pid);
    }
}


WMError WindowManagerLite::GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) const
{
    if (!infos.empty()) {
        TLOGE(WmsLogTag::WMS_MAIN, "infos is not empty.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WindowAdapterLite::GetInstance(userId_).GetAllMainWindowInfos(infos);
}

WMError WindowManagerLite::GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo)
{
    return WindowAdapterLite::GetInstance(userId_).GetMainWindowInfoByToken(abilityToken, windowInfo);
}

WMError WindowManagerLite::ClearMainSessions(const std::vector<int32_t>& persistentIds)
{
    if (persistentIds.empty()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Clear main Session failed, persistentIds is empty.");
        return WMError::WM_OK;
    }
    return WindowAdapterLite::GetInstance(userId_).ClearMainSessions(persistentIds);
}

WMError WindowManagerLite::ClearMainSessions(const std::vector<int32_t>& persistentIds,
    std::vector<int32_t>& clearFailedIds)
{
    if (persistentIds.empty()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Clear main Session failed, persistentIds is empty.");
        return WMError::WM_OK;
    }
    return WindowAdapterLite::GetInstance(userId_).ClearMainSessions(persistentIds, clearFailedIds);
}

WMError WindowManagerLite::NotifyWindowStyleChange(WindowStyleType type)
{
    pImpl_->NotifyWindowStyleChange(type);
    return WMError::WM_OK;
}

WMError WindowManagerLite::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    pImpl_->NotifyCallingWindowDisplayChanged(callingWindowInfo);
    return WMError::WM_OK;
}

WMError WindowManagerLite::RegisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_MAIN, "start register windowStyleChangedListener");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (pImpl_->windowStyleListenerAgent_ == nullptr) {
            pImpl_->windowStyleListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
        }
        if (pImpl_->windowStyleListeners_.count(listener)) {
            TLOGW(WmsLogTag::WMS_MAIN, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowStyleListeners_.insert(listener);
    }
    WMError ret = WMError::WM_OK;
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_MAIN, "RegisterWindowManagerAgent failed!");
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        pImpl_->windowStyleListenerAgent_ = nullptr;
        pImpl_->windowStyleListeners_.erase(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_MAIN, "start unregister windowStyleChangedListener");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (!pImpl_->windowStyleListeners_.erase(listener)) {
            TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
            return WMError::WM_OK;
        }
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowStyleListeners_.empty() && pImpl_->windowStyleListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
        if (ret == WMError::WM_OK) {
            std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
            pImpl_->windowStyleListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError  WindowManagerLite::RegisterCallingWindowDisplayChangedListener(
    const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
        if (pImpl_->callingDisplayChangedListeners_.count(listener)) {
            TLOGI(WmsLogTag::WMS_KEYBOARD, "listener is already registered");
            return WMError::WM_OK;
        }
    }
    // Begin register listener.
    WMError ret;
    sptr<WindowManagerAgentLite> agent = nullptr;
    {
        // Init wm agent.
        std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
        if (pImpl_->callingDisplayListenerAgent_ == nullptr) {
            pImpl_->callingDisplayListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
        }
        agent = pImpl_->callingDisplayListenerAgent_;
    }
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, agent);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, agent);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Register agent failed");
        {
            std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
            pImpl_->callingDisplayListenerAgent_ = nullptr;
        }
        return ret;
    }
    std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
    pImpl_->callingDisplayChangedListeners_.insert(listener);
    return ret;
}

WMError WindowManagerLite::UnregisterCallingWindowDisplayChangedListener(
    const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "enter");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
        if (pImpl_->callingDisplayChangedListeners_.erase(listener) == 0) {
            TLOGW(WmsLogTag::WMS_KEYBOARD, "listener is not found");
            return WMError::WM_OK;
        }
        if (pImpl_->callingDisplayChangedListeners_.size() > 0) {
            return WMError::WM_OK;
        }
    }
    // When listeners is empty, unregister wm agent.
    sptr<WindowManagerAgentLite> agent = nullptr;
    {
        std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
        if (pImpl_->callingDisplayListenerAgent_ == nullptr) {
            TLOGW(WmsLogTag::WMS_KEYBOARD, "Not unregistered agent");
            return WMError::WM_OK;
        }
        agent = pImpl_->callingDisplayListenerAgent_;
    }
    auto ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, agent);
    if (ret == WMError::WM_OK) {
        std::lock_guard<std::mutex> lock(pImpl_->callingDisplayChangedMutex_);
        pImpl_->callingDisplayListenerAgent_ = nullptr;
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Unregister agent failed");
    }
    return ret;
}

WindowStyleType WindowManagerLite::GetWindowStyleType()
{
    WindowStyleType styleType;
    if (WindowAdapterLite::GetInstance(userId_).GetWindowStyleType(styleType) == WMError::WM_OK) {
        return styleType;
    }
    return styleType;
}

WMError WindowManagerLite::SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).SetProcessWatermark(pid, watermarkName, isEnabled);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE,
        "lite pid:%{public}d, watermarkName:%{public}s, isEnabled:%{public}u, ret:%{public}d",
        pid, watermarkName.c_str(), isEnabled, static_cast<int32_t>(ret));
    return ret;
}

WMError WindowManagerLite::TerminateSessionByPersistentId(int32_t persistentId)
{
    if (persistentId == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_LIFE, "persistentId is invalid.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WindowAdapterLite::GetInstance(userId_).TerminateSessionByPersistentId(persistentId);
}

WMError WindowManagerLite::CloseTargetFloatWindow(const std::string& bundleName)
{
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "bundleName is empty.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WindowAdapterLite::GetInstance(userId_).CloseTargetFloatWindow(bundleName);
}

WMError WindowManagerLite::RegisterPiPStateChangedListener(const sptr<IPiPStateChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->pipStateChangedListenerAgent_ == nullptr) {
        pImpl_->pipStateChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PIP, pImpl_->pipStateChangedListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PIP, pImpl_->pipStateChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_PIP, "RegisterWindowManagerAgent failed!");
        pImpl_->pipStateChangedListenerAgent_ = nullptr;
    } else {
        if (pImpl_->pipStateChangedListeners_.count(listener)) {
            TLOGW(WmsLogTag::WMS_PIP, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->pipStateChangedListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterPiPStateChangedListener(const sptr<IPiPStateChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->pipStateChangedListeners_.erase(listener) == 0) {
        TLOGE(WmsLogTag::WMS_PIP, "could not find this listener");
        return WMError::WM_OK;
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->pipStateChangedListeners_.empty() &&
        pImpl_->pipStateChangedListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PIP,
            pImpl_->pipStateChangedListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->pipStateChangedListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::CloseTargetPiPWindow(const std::string& bundleName)
{
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_PIP, "bundleName is empty.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    TLOGD(WmsLogTag::WMS_PIP, "bundleName:%{public}s", bundleName.c_str());
    return WindowAdapterLite::GetInstance(userId_).CloseTargetPiPWindow(bundleName);
}

WMError WindowManagerLite::GetCurrentPiPWindowInfo(std::string& bundleName)
{
    return WindowAdapterLite::GetInstance(userId_).GetCurrentPiPWindowInfo(bundleName);
}

void WindowManagerLite::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground) const
{
    pImpl_->UpdatePiPWindowStateChanged(bundleName, isForeground);
}

WMError WindowManagerLite::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).GetAccessibilityWindowInfo(infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window info failed");
    }
    return ret;
}

WMError WindowManagerLite::RegisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->windowUpdateListenerAgent_ == nullptr) {
        pImpl_->windowUpdateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
    if (ret == WMError::WM_ERROR_SAMGR) {
        ret = ActiveFaultAgentReregister(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->windowUpdateListenerAgent_ = nullptr;
    } else {
        if (pImpl_->windowUpdateListeners_.count(listener)) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowUpdateListeners_.insert(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->windowUpdateListeners_.begin(), pImpl_->windowUpdateListeners_.end(), listener);
    if (iter == pImpl_->windowUpdateListeners_.end()) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    pImpl_->windowUpdateListeners_.erase(iter);
    WMError ret = WMError::WM_OK;
    if (pImpl_->windowUpdateListeners_.empty() && pImpl_->windowUpdateListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
        if (ret == WMError::WM_OK) {
            pImpl_->windowUpdateListenerAgent_ = nullptr;
        }
    }
    return ret;
}

WMError WindowManagerLite::ProcessRegisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    switch (observedInfo) {
        case WindowInfoKey::VISIBILITY_STATE :
            return RegisterVisibilityStateChangedListener(listener);
        case WindowInfoKey::DISPLAY_ID :
            return RegisterDisplayIdChangedListener(listener);
        case WindowInfoKey::MID_SCENE :
            return RegisterMidSceneChangedListener(listener);
        default:
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Invalid observedInfo: %{public}d", static_cast<uint32_t>(observedInfo));
            return WMError::WM_ERROR_INVALID_PARAM;
    }
}

WMError WindowManagerLite::ProcessUnregisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    switch (observedInfo) {
        case WindowInfoKey::VISIBILITY_STATE :
            return UnregisterVisibilityStateChangedListener(listener);
        case WindowInfoKey::DISPLAY_ID :
            return UnregisterDisplayIdChangedListener(listener);
        case WindowInfoKey::MID_SCENE :
            return UnregisterMidSceneChangedListener(listener);
        default:
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Invalid observedInfo: %{public}d", static_cast<uint32_t>(observedInfo));
            return WMError::WM_ERROR_INVALID_PARAM;
    }
}

WMError WindowManagerLite::RegisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::ostringstream observedInfoForLog;
    observedInfoForLog << "ObservedInfo: ";
    auto ret = WMError::WM_OK;
    for (const auto& info : observedInfo) {
        observedInfoForLog << static_cast<uint32_t>(info) << ", ";
        if (listener->GetInterestInfo().find(info) == listener->GetInterestInfo().end()) {
            listener->AddInterestInfo(info);
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

WMError WindowManagerLite::UnregisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
    const sptr<IWindowInfoChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::ostringstream observedInfoForLog;
    observedInfoForLog << "ObservedInfo: ";
    auto ret = WMError::WM_OK;
    for (const auto& info : observedInfo) {
        observedInfoForLog << static_cast<uint32_t>(info) << ", ";
        if (listener->GetInterestInfo().find(info) == listener->GetInterestInfo().end()) {
            listener->AddInterestInfo(info);
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

WMError WindowManagerLite::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).SetGlobalDragResizeType(dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManagerLite::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).GetGlobalDragResizeType(dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManagerLite::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).SetAppDragResizeType(bundleName, dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManagerLite::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).GetAppDragResizeType(bundleName, dragResizeType);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManagerLite::SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed");
    }
    return ret;
}

WMError WindowManagerLite::ListWindowInfo(const WindowInfoOption& windowInfoOption,
    std::vector<sptr<WindowInfo>>& infos) const
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "windowInfoOption: %{public}u %{public}u %{public}" PRIu64 " %{public}d",
        static_cast<WindowInfoFilterOptionDataType>(windowInfoOption.windowInfoFilterOption),
        static_cast<WindowInfoTypeOptionDataType>(windowInfoOption.windowInfoTypeOption),
        windowInfoOption.displayId, windowInfoOption.windowId);
    WMError ret = WindowAdapterLite::GetInstance(userId_).ListWindowInfo(windowInfoOption, infos);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}

WMError WindowManagerLite::SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    bool isHoverDown = LoadMMIClientAdapter() && IsHoverDown(pointerEvent);
    if (!isHoverDown) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointer event is not hover down");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).SendPointerEventForHover(pointerEvent);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "send failed");
    }
    return ret;
}

WMError WindowManagerLite::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed");
    }
    return ret;
}
} // namespace Rosen
} // namespace OHOS
