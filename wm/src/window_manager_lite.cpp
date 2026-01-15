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

#include "marshalling_helper.h"
#include "window_adapter_lite.h"
#include "window_manager_agent_lite.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManagerLite"};
}
std::unordered_map<int32_t, sptr<WindowManagerLite>> WindowManagerLite::windowManagerLiteMap_ = {};
std::mutex WindowManagerLite::windowManagerLiteMapMutex_;

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
    void NotifyMidSceneStatusChange(const WindowInfoList& windowInfoList);
    WindowInfoList GetWindowInfoListByInterestWindowIds(
        const sptr<IWindowInfoChangedListener>& listener, const WindowInfoList& windowInfoList);
    void PackWindowChangeInfo(const std::unordered_set<WindowInfoKey>& interestInfo,
        const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, WindowInfoList& windowChangeInfos);
    void NotifyWindowDrawingContentInfoChanged(const std::vector<sptr<WindowDrawingContentInfo>>&
        windowDrawingContentInfos);
    void NotifyWindowModeChange(WindowModeType type);
    void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing);
    void NotifyWMSConnected(int32_t userId, int32_t screenId);
    void NotifyWMSDisconnected(int32_t userId, int32_t screenId);
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
    std::vector<sptr<IFocusChangedListener>> focusChangedListeners_;
    sptr<WindowManagerAgentLite> focusChangedListenerAgent_;
    std::vector<sptr<IWindowUpdateListener>> windowUpdateListeners_;
    sptr<WindowManagerAgentLite> windowUpdateListenerAgent_;
    std::vector<sptr<IVisibilityChangedListener>> windowVisibilityListeners_;
    sptr<WindowManagerAgentLite> windowVisibilityListenerAgent_;
    std::vector<sptr<IWindowInfoChangedListener>> windowVisibilityStateListeners_;
    sptr<WindowManagerAgentLite> windowVisibilityStateListenerAgent_;
    std::vector<sptr<IWindowInfoChangedListener>> midSceneStatusChangeListeners_;
    sptr<WindowManagerAgentLite> windowPropertyChangeAgent_;
    std::vector<sptr<IDrawingContentChangedListener>> windowDrawingContentListeners_;
    sptr<WindowManagerAgentLite> windowDrawingContentListenerAgent_;
    std::vector<sptr<IWindowModeChangedListener>> windowModeListeners_;
    sptr<WindowManagerAgentLite> windowModeListenerAgent_;
    std::vector<sptr<ICameraWindowChangedListener>> cameraWindowChangedListeners_;
    sptr<WindowManagerAgentLite> cameraWindowChangedListenerAgent_;
    sptr<IWMSConnectionChangedListener> wmsConnectionChangedListener_;
    std::vector<sptr<IWindowStyleChangedListener>> windowStyleListeners_;
    sptr<WindowManagerAgentLite> windowStyleListenerAgent_;
    std::vector<sptr<IPiPStateChangedListener>> pipStateChangedListeners_;
    sptr<WindowManagerAgentLite> pipStateChangedListenerAgent_;
    std::vector<sptr<IKeyboardCallingWindowDisplayChangedListener>> callingDisplayChangedListeners_;
    sptr<WindowManagerAgentLite> callingDisplayListenerAgent_;
    std::vector<sptr<IAllGroupInfoChangedListener>> allGroupInfoChangedListeners_;
    sptr<WindowManagerAgentLite> allGroupInfoChangedListenerAgent_;
};

void WindowManagerLite::Impl::NotifyWMSConnected(int32_t userId, int32_t screenId)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "WMS connected [userId:%{public}d; screenId:%{public}d]", userId, screenId);
    sptr<IWMSConnectionChangedListener> wmsConnectionChangedListener;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        wmsConnectionChangedListener = wmsConnectionChangedListener_;
    }
    if (wmsConnectionChangedListener != nullptr) {
        wmsConnectionChangedListener->OnConnected(userId, screenId);
    }
}

void WindowManagerLite::Impl::NotifyWMSDisconnected(int32_t userId, int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "WMS disconnected [userId:%{public}d; screenId:%{public}d]", userId, screenId);
    sptr<IWMSConnectionChangedListener> wmsConnectionChangedListener;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        wmsConnectionChangedListener = wmsConnectionChangedListener_;
    }
    if (wmsConnectionChangedListener != nullptr) {
        wmsConnectionChangedListener->OnDisconnected(userId, screenId);
    }
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
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        focusChangeListeners = focusChangedListeners_;
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
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        focusChangeListeners = focusChangedListeners_;
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
        allGroupInfoChangedListeners = allGroupInfoChangedListeners_;
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
        visibilityChangeListeners = windowVisibilityListeners_;
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
        windowVisibilityStateListeners = windowVisibilityStateListeners_;
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
        midSceneStatusChangeListeners = midSceneStatusChangeListeners_;
    }
    for (auto& listener : midSceneStatusChangeListeners) {
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
        windowDrawingContentChangeListeners = windowDrawingContentListeners_;
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
        windowModeListeners = windowModeListeners_;
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
        windowUpdateListeners = windowUpdateListeners_;
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
        cameraWindowChangeListeners = cameraWindowChangedListeners_;
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
        windowStyleListeners = windowStyleListeners_;
    }
    for (auto &listener : windowStyleListeners) {
        TLOGI(WmsLogTag::WMS_MAIN, "real WindowStyleChange type: %{public}d",
              static_cast<uint8_t>(type));
        listener->OnWindowStyleUpdate(type);
    }
}

void WindowManagerLite::Impl::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    std::vector<sptr<IKeyboardCallingWindowDisplayChangedListener>> displayChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displayChangeListeners = callingDisplayChangedListeners_;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "notify persistentId: %{public}d, pid: %{public}d, "
        "displayId: %{public}" PRIu64" , userId: %{public}d, size: %{public}zu",
        callingWindowInfo.windowId_, callingWindowInfo.callingPid_, callingWindowInfo.displayId_,
        callingWindowInfo.userId_, displayChangeListeners.size());

    for (const auto& listener : displayChangeListeners) {
        if (!listener) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "listener is nullptr");
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
        pipStateChangedListeners = pipStateChangedListeners_;
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
    static int32_t clientUserId = GetUserIdByUid(getuid());
    if (clientUserId != SYSTEM_USERID || userId <= INVALID_USER_ID) {
        return GetInstance();
    }

    /**
     * multi-instance mode
     * At present, the map does not have memory leak issues. In actual business scenarios,
     * the number of instances created is limited (estimated to be less than 20).
     */
    std::lock_guard<std::mutex> lock(windowManagerLiteMapMutex_);
    auto iter = windowManagerLiteMap_.find(userId);
    if (iter != windowManagerLiteMap_.end() && iter->second) {
        return *iter->second;
    }
    auto instance = sptr<WindowManagerLite>::MakeSptr(userId);
    windowManagerLiteMap_.insert({ userId, instance });
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    return *windowManagerLiteMap_[userId];
}

WMError WindowManagerLite::RemoveInstanceByUserId(const int32_t userId)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "remove instance userId: %{public}d", userId);
    std::lock_guard<std::mutex> lock(windowManagerLiteMapMutex_);
    windowManagerLiteMap_.erase(userId);
    return WMError::WM_OK;
}

WMError WindowManagerLite::RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    WMError ret = WMError::WM_OK;
    if (pImpl_->focusChangedListenerAgent_ == nullptr) {
        pImpl_->focusChangedListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed !");
        pImpl_->focusChangedListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->focusChangedListeners_.begin(), pImpl_->focusChangedListeners_.end(), listener);
        if (iter != pImpl_->focusChangedListeners_.end()) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->focusChangedListeners_.push_back(listener);
    }
    return ret;
}

WMError WindowManagerLite::UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->focusChangedListeners_.begin(), pImpl_->focusChangedListeners_.end(), listener);
    if (iter == pImpl_->focusChangedListeners_.end()) {
        WLOGFE("could not find this listener");
        return WMError::WM_OK;
    }
    pImpl_->focusChangedListeners_.erase(iter);
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
        pImpl_->allGroupInfoChangedListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO,
            pImpl_->allGroupInfoChangedListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_FOCUS, "register WindowManagerAgent failed");
        pImpl_->allGroupInfoChangedListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->allGroupInfoChangedListeners_.begin(),
                              pImpl_->allGroupInfoChangedListeners_.end(), listener);
        if (iter != pImpl_->allGroupInfoChangedListeners_.end()) {
            TLOGW(WmsLogTag::WMS_FOCUS, "listener is already registered");
            return WMError::WM_OK;
        }
        pImpl_->allGroupInfoChangedListeners_.emplace_back(listener);
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
    auto iter = std::find(pImpl_->allGroupInfoChangedListeners_.begin(), pImpl_->allGroupInfoChangedListeners_.end(),
                          listener);
    if (iter == pImpl_->allGroupInfoChangedListeners_.end()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "could not find this listener");
        return WMError::WM_OK;
    }
    pImpl_->allGroupInfoChangedListeners_.erase(iter);
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
        pImpl_->windowVisibilityListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed !");
        pImpl_->windowVisibilityListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->windowVisibilityListeners_.begin(), pImpl_->windowVisibilityListeners_.end(),
            listener);
        if (iter != pImpl_->windowVisibilityListeners_.end()) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowVisibilityListeners_.emplace_back(listener);
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
    pImpl_->windowVisibilityListeners_.erase(std::remove_if(pImpl_->windowVisibilityListeners_.begin(),
        pImpl_->windowVisibilityListeners_.end(), [listener](sptr<IVisibilityChangedListener> registeredListener) {
            return registeredListener == listener;
        }), pImpl_->windowVisibilityListeners_.end());

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
        pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgentLite>::MakeSptr();
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
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowPropertyChangeAgent failed!");
        pImpl_->windowPropertyChangeAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->midSceneStatusChangeListeners_.begin(),
            pImpl_->midSceneStatusChangeListeners_.end(), listener);
        if (iter != pImpl_->midSceneStatusChangeListeners_.end()) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->midSceneStatusChangeListeners_.emplace_back(listener);
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
    pImpl_->midSceneStatusChangeListeners_.erase(std::remove_if(pImpl_->midSceneStatusChangeListeners_.begin(),
        pImpl_->midSceneStatusChangeListeners_.end(), [listener](sptr<IWindowInfoChangedListener> registeredListener) {
            return registeredListener == listener;
        }), pImpl_->midSceneStatusChangeListeners_.end());
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

void WindowManagerLite::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "lite flags=%{public}u", propertyDirtyFlags);
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
        pImpl_->windowVisibilityStateListenerAgent_ = new WindowManagerAgentLite();
    }
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
        pImpl_->windowVisibilityStateListenerAgent_);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RegisterWindowManagerAgent failed!");
        pImpl_->windowVisibilityStateListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->windowVisibilityStateListeners_.begin(),
            pImpl_->windowVisibilityStateListeners_.end(), listener);
        if (iter != pImpl_->windowVisibilityStateListeners_.end()) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowVisibilityStateListeners_.emplace_back(listener);
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
    pImpl_->windowVisibilityStateListeners_.erase(std::remove_if(pImpl_->windowVisibilityStateListeners_ .begin(),
        pImpl_->windowVisibilityStateListeners_.end(), [listener](sptr<IWindowInfoChangedListener> registeredListener) {
            return registeredListener == listener;
        }), pImpl_->windowVisibilityStateListeners_.end());

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
        pImpl_->windowDrawingContentListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
        ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE,
            pImpl_->windowDrawingContentListenerAgent_);
    }
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed !");
        pImpl_->windowDrawingContentListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->windowDrawingContentListeners_.begin(),
            pImpl_->windowDrawingContentListeners_.end(), listener);
        if (iter != pImpl_->windowDrawingContentListeners_.end()) {
            WLOGFW("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowDrawingContentListeners_.emplace_back(listener);
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
    pImpl_->windowDrawingContentListeners_.erase(std::remove_if(pImpl_->windowDrawingContentListeners_.begin(),
        pImpl_->windowDrawingContentListeners_.end(),
        [listener](sptr<IDrawingContentChangedListener> registeredListener) { return registeredListener == listener; }),
        pImpl_->windowDrawingContentListeners_.end());

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

WMError WindowManagerLite::RegisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    if (pImpl_->windowModeListenerAgent_ == nullptr) {
        pImpl_->windowModeListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE, pImpl_->windowModeListenerAgent_);
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_MAIN, "RegisterWindowManagerAgent failed!");
        pImpl_->windowModeListenerAgent_ = nullptr;
        return ret;
    }
    auto iter = std::find(pImpl_->windowModeListeners_.begin(), pImpl_->windowModeListeners_.end(), listener);
    if (iter != pImpl_->windowModeListeners_.end()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Listener is already registered.");
        return WMError::WM_OK;
    }
    pImpl_->windowModeListeners_.push_back(listener);
    return ret;
}

WMError WindowManagerLite::UnregisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->windowModeListeners_.begin(), pImpl_->windowModeListeners_.end(), listener);
    if (iter == pImpl_->windowModeListeners_.end()) {
        TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
        return WMError::WM_OK;
    }
    pImpl_->windowModeListeners_.erase(iter);
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
        pImpl_->cameraWindowChangedListenerAgent_ = new WindowManagerAgentLite();
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_WINDOW, pImpl_->cameraWindowChangedListenerAgent_);
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "RegisterWindowManagerAgent failed!");
        pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->cameraWindowChangedListeners_.begin(),
            pImpl_->cameraWindowChangedListeners_.end(), listener);
        if (iter != pImpl_->cameraWindowChangedListeners_.end()) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->cameraWindowChangedListeners_.push_back(listener);
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
    auto iter = std::find(pImpl_->cameraWindowChangedListeners_.begin(),
        pImpl_->cameraWindowChangedListeners_.end(), listener);
    if (iter == pImpl_->cameraWindowChangedListeners_.end()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "could not find this listener");
        return WMError::WM_OK;
    }
    pImpl_->cameraWindowChangedListeners_.erase(iter);
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

WMError WindowManagerLite::UpdateAnimationSpeedWithPid(pid_t pid, float speed)
{
    WMError ret = WindowAdapterLite::GetInstance(userId_).UpdateAnimationSpeedWithPid(pid, speed);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "failed");
    }
    return ret;
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
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (pImpl_->wmsConnectionChangedListener_) {
            TLOGI(WmsLogTag::WMS_MULTI_USER, "Listener already registered, skipping");
            return WMError::WM_OK;
        }
        pImpl_->wmsConnectionChangedListener_ = listener;
    }
    auto ret = WindowAdapterLite::GetInstance(userId_).RegisterWMSConnectionChangedListener(
        [weakThis = wptr(this)](int32_t userId, int32_t screenId, bool isConnected) {
            auto wml = weakThis.promote();
            if (!wml) {
                TLOGE(WmsLogTag::WMS_SCB, "window manager lite is null");
                return;
            }
            wml->OnWMSConnectionChanged(userId, screenId, isConnected);
        });
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Register failed: error = %{public}d", static_cast<int32_t>(ret));
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        pImpl_->wmsConnectionChangedListener_ = nullptr;
    }
    return ret;
}

WMError WindowManagerLite::UnregisterWMSConnectionChangedListener()
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "Unregister enter");
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->wmsConnectionChangedListener_ = nullptr;
    WindowAdapterLite::GetInstance(userId_).UnregisterWMSConnectionChangedListener();
    return WMError::WM_OK;
}

void WindowManagerLite::OnWMSConnectionChanged(int32_t userId, int32_t screenId, bool isConnected) const
{
    if (isConnected) {
        pImpl_->NotifyWMSConnected(userId, screenId);
    } else {
        pImpl_->NotifyWMSDisconnected(userId, screenId);
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

WMError WindowManagerLite::GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken,
    MainWindowInfo& windowInfo)
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
            pImpl_->windowStyleListenerAgent_ = new WindowManagerAgentLite();
        }
        auto iter = std::find(pImpl_->windowStyleListeners_.begin(), pImpl_->windowStyleListeners_.end(), listener);
        if (iter != pImpl_->windowStyleListeners_.end()) {
            TLOGW(WmsLogTag::WMS_MAIN, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowStyleListeners_.push_back(listener);
    }
    WMError ret = WMError::WM_OK;
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_STYLE, pImpl_->windowStyleListenerAgent_);
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_MAIN, "RegisterWindowManagerAgent failed!");
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        pImpl_->windowStyleListenerAgent_ = nullptr;
        auto iter = std::find(pImpl_->windowStyleListeners_.begin(), pImpl_->windowStyleListeners_.end(), listener);
        if (iter != pImpl_->windowStyleListeners_.end()) {
            pImpl_->windowStyleListeners_.erase(iter);
        }
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
        auto iter = std::find(pImpl_->windowStyleListeners_.begin(), pImpl_->windowStyleListeners_.end(), listener);
        if (iter == pImpl_->windowStyleListeners_.end()) {
            TLOGE(WmsLogTag::WMS_MAIN, "could not find this listener");
            return WMError::WM_OK;
        }
        pImpl_->windowStyleListeners_.erase(iter);
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
    TLOGI(WmsLogTag::WMS_KEYBOARD, "start register callingDisplayChangeListener");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        if (pImpl_->callingDisplayListenerAgent_ == nullptr) {
            pImpl_->callingDisplayListenerAgent_ = new WindowManagerAgentLite();
        }
        auto iter = std::find(pImpl_->callingDisplayChangedListeners_.begin(),
            pImpl_->callingDisplayChangedListeners_.end(), listener);
        if (iter != pImpl_->callingDisplayChangedListeners_.end()) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "listener is already registered");
            return WMError::WM_OK;
        }
        pImpl_->callingDisplayChangedListeners_.emplace_back(listener);
    }
    WMError ret = WMError::WM_OK;
    ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, pImpl_->callingDisplayListenerAgent_);
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Register agent failed!");
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        pImpl_->callingDisplayListenerAgent_ = nullptr;
        auto iter = std::find(pImpl_->callingDisplayChangedListeners_.begin(),
            pImpl_->callingDisplayChangedListeners_.end(), listener);
        if (iter != pImpl_->callingDisplayChangedListeners_.end()) {
            pImpl_->callingDisplayChangedListeners_.erase(iter);
        }
    }
    return ret;
}

WMError  WindowManagerLite::UnregisterCallingWindowDisplayChangedListener(
    const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "start unRegister callingDisplayChangeListener");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        auto iter = std::find(pImpl_->callingDisplayChangedListeners_.begin(),
            pImpl_->callingDisplayChangedListeners_.end(), listener);
        if (iter == pImpl_->callingDisplayChangedListeners_.end()) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "could not find this listener");
            return WMError::WM_OK;
        }
        pImpl_->callingDisplayChangedListeners_.erase(iter);
    }
    WMError ret = WMError::WM_OK;
    if (pImpl_->callingDisplayChangedListeners_.empty() && pImpl_->callingDisplayListenerAgent_ != nullptr) {
        ret = WindowAdapterLite::GetInstance(userId_).UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, pImpl_->callingDisplayListenerAgent_);
        if (ret == WMError::WM_OK) {
            std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
            pImpl_->callingDisplayListenerAgent_ = nullptr;
        }
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
        pImpl_->pipStateChangedListenerAgent_ = new WindowManagerAgentLite();
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PIP, pImpl_->pipStateChangedListenerAgent_);
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_PIP, "RegisterWindowManagerAgent failed!");
        pImpl_->pipStateChangedListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->pipStateChangedListeners_.begin(),
            pImpl_->pipStateChangedListeners_.end(), listener);
        if (iter != pImpl_->pipStateChangedListeners_.end()) {
            TLOGW(WmsLogTag::WMS_PIP, "Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->pipStateChangedListeners_.push_back(listener);
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
    auto iter = std::find(pImpl_->pipStateChangedListeners_.begin(),
        pImpl_->pipStateChangedListeners_.end(), listener);
    if (iter == pImpl_->pipStateChangedListeners_.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "could not find this listener");
        return WMError::WM_OK;
    }
    pImpl_->pipStateChangedListeners_.erase(iter);
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
        pImpl_->windowUpdateListenerAgent_ = new WindowManagerAgentLite();
    }
    WMError ret = WindowAdapterLite::GetInstance(userId_).RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
    if (ret != WMError::WM_OK) {
        WLOGFW("RegisterWindowManagerAgent failed!");
        pImpl_->windowUpdateListenerAgent_ = nullptr;
    } else {
        auto iter = std::find(pImpl_->windowUpdateListeners_.begin(), pImpl_->windowUpdateListeners_.end(), listener);
        if (iter != pImpl_->windowUpdateListeners_.end()) {
            WLOGI("Listener is already registered.");
            return WMError::WM_OK;
        }
        pImpl_->windowUpdateListeners_.emplace_back(listener);
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
    bool isHoverDown = pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER &&
        pointerEvent->GetSourceType() ==  MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
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
