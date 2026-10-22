/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include "all_users_window_manager.h"
#include "session_manager.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
class AllUsersWindowManager::Impl {
public:
    template<typename T>
    using ListenerSet = std::unordered_set<sptr<T>, SptrHash<T>>;
    
    ListenerSet<IFocusChangedListener> focusListeners_;
    std::mutex focusListenersMutex_;
    
    ListenerSet<IVisibilityChangedListener> visibilityListeners_;
    std::mutex visibilityListenersMutex_;

    ListenerSet<IWindowPidVisibilityChangedListener> windowPidVisibilityListeners_;
    std::mutex windowPidVisibilityListenersMutex_;
    
    std::mutex userChangeMutex_;
    bool userChangeListenersRegistered_ = false;
};

AllUsersWindowManager& AllUsersWindowManager::GetInstance()
{
    static auto instance = sptr<AllUsersWindowManager>::MakeSptr();
    return *instance;
}

AllUsersWindowManager::AllUsersWindowManager()
    : pImpl_(std::make_unique<Impl>()) {}

AllUsersWindowManager::~AllUsersWindowManager() = default;

std::unordered_set<int32_t> AllUsersWindowManager::GetActiveUserIds() const
{
    std::vector<int32_t> userIdsVec;
    SessionManager::GetInstance().GetActiveUserIds(userIdsVec);
    
    std::unordered_set<int32_t> activeUserIds;
    for (int32_t userId : userIdsVec) {
        activeUserIds.insert(userId);
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetActiveUserIds returns %{public}zu users", activeUserIds.size());
    return activeUserIds;
}

WMError AllUsersWindowManager::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "GetVisibilityWindowInfo: single instance mode");
        return WindowManager::GetInstance().GetVisibilityWindowInfo(infos);
    }
    auto activeUserIds = GetActiveUserIds();
    std::vector<int32_t> activeUsers(activeUserIds.begin(), activeUserIds.end());
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetVisibilityWindowInfo users: %{public}zu", activeUsers.size());
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUsers) {
        std::vector<sptr<WindowVisibilityInfo>> userInfos;
        WMError userRet = WindowManager::GetInstance(userId).GetVisibilityWindowInfo(userInfos);
        if (userRet == WMError::WM_OK) {
            infos.insert(infos.end(), userInfos.begin(), userInfos.end());
        } else {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "GetVisibilityWindowInfo failed for user %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetVisibilityWindowInfo total: %{public}zu, ret: %{public}d",
        infos.size(), static_cast<int32_t>(ret));
    return ret;
}

WMError AllUsersWindowManager::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "GetAccessibilityWindowInfo: single instance mode");
        return WindowManager::GetInstance().GetAccessibilityWindowInfo(infos);
    }
    auto activeUserIds = GetActiveUserIds();
    std::vector<int32_t> activeUsers(activeUserIds.begin(), activeUserIds.end());
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetAccessibilityWindowInfo users: %{public}zu", activeUsers.size());
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUsers) {
        std::vector<sptr<AccessibilityWindowInfo>> userInfos;
        WMError userRet = WindowManager::GetInstance(userId).GetAccessibilityWindowInfo(userInfos);
        if (userRet == WMError::WM_OK) {
            infos.insert(infos.end(), userInfos.begin(), userInfos.end());
        } else {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "GetAccessibilityWindowInfo failed for user %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetAccessibilityWindowInfo total: %{public}zu, ret: %{public}d",
        infos.size(), static_cast<int32_t>(ret));
    return ret;
}

WMError AllUsersWindowManager::RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterFocusChangedListener: single instance mode");
        return WindowManager::GetInstance().RegisterFocusChangedListener(listener);
    }
    RegisterUserChangeListeners();
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "RegisterFocusChangedListener listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto activeUserIds = GetActiveUserIds();
    {
        std::lock_guard<std::mutex> lock(pImpl_->focusListenersMutex_);
        pImpl_->focusListeners_.insert(listener);
    }
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUserIds) {
        WMError userRet = WindowManager::GetInstance(userId).RegisterFocusChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "RegisterFocusChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterFocusChangedListener users: %{public}zu",
        activeUserIds.size());
    return ret;
}

WMError AllUsersWindowManager::UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "UnregisterFocusChangedListener: single instance mode");
        return WindowManager::GetInstance().UnregisterFocusChangedListener(listener);
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "UnregisterFocusChangedListener listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(pImpl_->focusListenersMutex_);
        pImpl_->focusListeners_.erase(listener);
    }
    auto activeUserIds = GetActiveUserIds();
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUserIds) {
        WMError userRet = WindowManager::GetInstance(userId).UnregisterFocusChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "UnregisterFocusChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    return ret;
}

WMError AllUsersWindowManager::RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterVisibilityChangedListener: single instance mode");
        return WindowManager::GetInstance().RegisterVisibilityChangedListener(listener);
    }
    RegisterUserChangeListeners();
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "RegisterVisibilityChangedListener listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto activeUserIds = GetActiveUserIds();
    {
        std::lock_guard<std::mutex> lock(pImpl_->visibilityListenersMutex_);
        pImpl_->visibilityListeners_.insert(listener);
    }
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUserIds) {
        WMError userRet = WindowManager::GetInstance(userId).RegisterVisibilityChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "RegisterVisibilityChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterVisibilityChangedListener users: %{public}zu",
        activeUserIds.size());
    return ret;
}

WMError AllUsersWindowManager::UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "UnregisterVisibilityChangedListener: single instance mode");
        return WindowManager::GetInstance().UnregisterVisibilityChangedListener(listener);
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "UnregisterVisibilityChangedListener listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(pImpl_->visibilityListenersMutex_);
        pImpl_->visibilityListeners_.erase(listener);
    }
    auto activeUserIds = GetActiveUserIds();
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUserIds) {
        WMError userRet = WindowManager::GetInstance(userId).UnregisterVisibilityChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "UnregisterVisibilityChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    return ret;
}

WMError AllUsersWindowManager::RegisterWindowPidVisibilityChangedListener(
    const sptr<IWindowPidVisibilityChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterWindowPidVisibilityChangedListener: single instance mode");
        return WindowManager::GetInstance().RegisterWindowPidVisibilityChangedListener(listener);
    }
    RegisterUserChangeListeners();
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "RegisterWindowPidVisibilityChangedListener listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto activeUserIds = GetActiveUserIds();
    {
        std::lock_guard<std::mutex> lock(pImpl_->windowPidVisibilityListenersMutex_);
        pImpl_->windowPidVisibilityListeners_.insert(listener);
    }
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUserIds) {
        WMError userRet = WindowManager::GetInstance(userId).RegisterWindowPidVisibilityChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "RegisterWindowPidVisibilityChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterWindowPidVisibilityChangedListener users: %{public}zu",
        activeUserIds.size());
    return ret;
}

WMError AllUsersWindowManager::UnregisterWindowPidVisibilityChangedListener(
    const sptr<IWindowPidVisibilityChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "UnregisterWindowPidVisibilityChangedListener: single instance mode");
        return WindowManager::GetInstance().UnregisterWindowPidVisibilityChangedListener(listener);
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "UnregisterWindowPidVisibilityChangedListener listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(pImpl_->windowPidVisibilityListenersMutex_);
        pImpl_->windowPidVisibilityListeners_.erase(listener);
    }
    auto activeUserIds = GetActiveUserIds();
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUserIds) {
        WMError userRet = WindowManager::GetInstance(userId).UnregisterWindowPidVisibilityChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "UnregisterWindowPidVisibilityChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    return ret;
}

void AllUsersWindowManager::RegisterUserChangeListeners()
{
    if (!IsMultiInstanceEnabled()) {
        return;
    }
    std::lock_guard<std::mutex> lock(pImpl_->userChangeMutex_);
    if (pImpl_->userChangeListenersRegistered_) {
        return;
    }
    SessionManager::GetInstance().RegisterUserAddedListener(
        [this](int32_t userId) { OnUserAdded(userId); });
    SessionManager::GetInstance().RegisterUserRemovedListener(
        [this](int32_t userId) { OnUserRemoved(userId); });
    pImpl_->userChangeListenersRegistered_ = true;
}

void AllUsersWindowManager::OnUserAdded(int32_t userId)
{
    Impl::ListenerSet<IFocusChangedListener> focusListeners;
    {
        std::lock_guard<std::mutex> lock(pImpl_->focusListenersMutex_);
        focusListeners = pImpl_->focusListeners_;
    }
    for (const auto& listener : focusListeners) {
        auto ret = WindowManager::GetInstance(userId).RegisterFocusChangedListener(listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                "Register global focus listener failed for user %{public}d, error %{public}d",
                userId, static_cast<int32_t>(ret));
        }
    }
    
    Impl::ListenerSet<IVisibilityChangedListener> visibilityListeners;
    {
        std::lock_guard<std::mutex> lock(pImpl_->visibilityListenersMutex_);
        visibilityListeners = pImpl_->visibilityListeners_;
    }
    for (const auto& listener : visibilityListeners) {
        auto ret = WindowManager::GetInstance(userId).RegisterVisibilityChangedListener(listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                "Register global visibility listener failed for user %{public}d, error %{public}d",
                userId, static_cast<int32_t>(ret));
        }
    }
    
    Impl::ListenerSet<IWindowPidVisibilityChangedListener> windowPidVisibilityListeners;
    {
        std::lock_guard<std::mutex> lock(pImpl_->windowPidVisibilityListenersMutex_);
        windowPidVisibilityListeners = pImpl_->windowPidVisibilityListeners_;
    }
    for (const auto& listener : windowPidVisibilityListeners) {
        auto ret = WindowManager::GetInstance(userId).RegisterWindowPidVisibilityChangedListener(listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                "Register global windowPidVisibility listener failed for user %{public}d, error %{public}d",
                userId, static_cast<int32_t>(ret));
        }
    }
}

void AllUsersWindowManager::OnUserRemoved(int32_t userId)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "User %{public}d removed", userId);
}

} // namespace Rosen
} // namespace OHOS