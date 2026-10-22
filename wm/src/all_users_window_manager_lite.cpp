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

#include "all_users_window_manager_lite.h"
#include "session_manager_lite.h"
#include "window_manager_lite.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
class AllUsersWindowManagerLite::Impl {
public:
    template<typename T>
    using ListenerSet = std::unordered_set<sptr<T>, SptrHash<T>>;

    ListenerSet<IFocusChangedListener> focusListeners_;
    std::mutex focusListenersMutex_;

    ListenerSet<IVisibilityChangedListener> visibilityListeners_;
    std::mutex visibilityListenersMutex_;

    std::mutex userChangeMutex_;
    bool userChangeListenersRegistered_ = false;
};

AllUsersWindowManagerLite& AllUsersWindowManagerLite::GetInstance()
{
    static auto instance = sptr<AllUsersWindowManagerLite>::MakeSptr();
    return *instance;
}

AllUsersWindowManagerLite::AllUsersWindowManagerLite()
    : pImpl_(std::make_unique<Impl>()) {}

AllUsersWindowManagerLite::~AllUsersWindowManagerLite() = default;

std::unordered_set<int32_t> AllUsersWindowManagerLite::GetActiveUserIds() const
{
    std::vector<int32_t> userIdsVec;
    SessionManagerLite::GetInstance().GetActiveUserIds(userIdsVec);

    std::unordered_set<int32_t> activeUserIds;
    for (int32_t userId : userIdsVec) {
        activeUserIds.insert(userId);
    }
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetActiveUserIds returns %{public}zu users", activeUserIds.size());
    return activeUserIds;
}

WMError AllUsersWindowManagerLite::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "GetVisibilityWindowInfo: single instance mode");
        return WindowManagerLite::GetInstance().GetVisibilityWindowInfo(infos);
    }
    auto activeUserIds = GetActiveUserIds();
    std::vector<int32_t> activeUsers(activeUserIds.begin(), activeUserIds.end());
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetVisibilityWindowInfo users: %{public}zu", activeUsers.size());
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUsers) {
        std::vector<sptr<WindowVisibilityInfo>> userInfos;
        WMError userRet = WindowManagerLite::GetInstance(userId).GetVisibilityWindowInfo(userInfos);
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

WMError AllUsersWindowManagerLite::GetAccessibilityWindowInfo(
    std::vector<sptr<AccessibilityWindowInfo>>& infos) const
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "GetAccessibilityWindowInfo: single instance mode");
        return WindowManagerLite::GetInstance().GetAccessibilityWindowInfo(infos);
    }
    auto activeUserIds = GetActiveUserIds();
    std::vector<int32_t> activeUsers(activeUserIds.begin(), activeUserIds.end());
    TLOGD(WmsLogTag::WMS_MULTI_USER, "GetAccessibilityWindowInfo users: %{public}zu", activeUsers.size());
    WMError ret = WMError::WM_OK;
    for (int32_t userId : activeUsers) {
        std::vector<sptr<AccessibilityWindowInfo>> userInfos;
        WMError userRet = WindowManagerLite::GetInstance(userId).GetAccessibilityWindowInfo(userInfos);
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

WMError AllUsersWindowManagerLite::RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterFocusChangedListener: single instance mode");
        return WindowManagerLite::GetInstance().RegisterFocusChangedListener(listener);
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
        WMError userRet = WindowManagerLite::GetInstance(userId).RegisterFocusChangedListener(listener);
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

WMError AllUsersWindowManagerLite::UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "UnregisterFocusChangedListener: single instance mode");
        return WindowManagerLite::GetInstance().UnregisterFocusChangedListener(listener);
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
        WMError userRet = WindowManagerLite::GetInstance(userId).UnregisterFocusChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "UnregisterFocusChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    return ret;
}

WMError AllUsersWindowManagerLite::RegisterVisibilityChangedListener(
    const sptr<IVisibilityChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "RegisterVisibilityChangedListener: single instance mode");
        return WindowManagerLite::GetInstance().RegisterVisibilityChangedListener(listener);
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
        WMError userRet = WindowManagerLite::GetInstance(userId).RegisterVisibilityChangedListener(listener);
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

WMError AllUsersWindowManagerLite::UnregisterVisibilityChangedListener(
    const sptr<IVisibilityChangedListener>& listener)
{
    if (!IsMultiInstanceEnabled()) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "UnregisterVisibilityChangedListener: single instance mode");
        return WindowManagerLite::GetInstance().UnregisterVisibilityChangedListener(listener);
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
        WMError userRet = WindowManagerLite::GetInstance(userId).UnregisterVisibilityChangedListener(listener);
        if (userRet != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_MULTI_USER,
                "UnregisterVisibilityChangedListener failed for userId %{public}d, error %{public}d",
                userId, static_cast<int32_t>(userRet));
            ret = userRet;
        }
    }
    return ret;
}

void AllUsersWindowManagerLite::RegisterUserChangeListeners()
{
    if (!IsMultiInstanceEnabled()) {
        return;
    }
    std::lock_guard<std::mutex> lock(pImpl_->userChangeMutex_);
    if (pImpl_->userChangeListenersRegistered_) {
        return;
    }
    SessionManagerLite::GetInstance().RegisterUserAddedListener(
        [this](int32_t userId) { OnUserAdded(userId); });
    SessionManagerLite::GetInstance().RegisterUserRemovedListener(
        [this](int32_t userId) { OnUserRemoved(userId); });
    pImpl_->userChangeListenersRegistered_ = true;
}

void AllUsersWindowManagerLite::OnUserAdded(int32_t userId)
{
    Impl::ListenerSet<IFocusChangedListener> focusListeners;
    {
        std::lock_guard<std::mutex> lock(pImpl_->focusListenersMutex_);
        focusListeners = pImpl_->focusListeners_;
    }
    for (const auto& listener : focusListeners) {
        auto ret = WindowManagerLite::GetInstance(userId).RegisterFocusChangedListener(listener);
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
        auto ret = WindowManagerLite::GetInstance(userId).RegisterVisibilityChangedListener(listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_MULTI_USER,
                "Register global visibility listener failed for user %{public}d, error %{public}d",
                userId, static_cast<int32_t>(ret));
        }
    }
}

void AllUsersWindowManagerLite::OnUserRemoved(int32_t userId)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "User %{public}d removed", userId);
}

} // namespace Rosen
} // namespace OHOS