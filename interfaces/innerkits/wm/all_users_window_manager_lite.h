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

#ifndef OHOS_ROSEN_ALL_USERS_WINDOW_MANAGER_LITE_H
#define OHOS_ROSEN_ALL_USERS_WINDOW_MANAGER_LITE_H

#include <memory>
#include <unordered_set>
#include <vector>
#include "refbase.h"
#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
class WindowVisibilityInfo;
class AccessibilityWindowInfo;
class IFocusChangedListener;
class IVisibilityChangedListener;

/**
 * @brief AllUsersWindowManagerLite is a singleton class that aggregates window information
 *        from all active users. It provides unified query interfaces for multi-user scenarios.
 * @attention This class is designed for multi-user scenarios (e.g., automotive systems).
 *            It aggregates data from all active users by calling WindowManagerLite instances
 *            of each user.
 */
class AllUsersWindowManagerLite : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(AllUsersWindowManagerLite);
public:
    /**
     * @brief Get active user IDs from SessionManagerLite.
     * @return Active user IDs set.
     */
    std::unordered_set<int32_t> GetActiveUserIds() const;

    /**
     * @brief Get visibility window info from all active users.
     * @param infos Output vector of WindowVisibilityInfo from all users.
     * @return WM_OK means success, others means failed.
     */
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const;

    /**
     * @brief Get accessibility window info from all active users.
     * @param infos Output vector of AccessibilityWindowInfo from all users.
     * @return WM_OK means success, others means failed.
     */
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const;

    WMError RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener);
    WMError UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener);

    WMError RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener);
    WMError UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener);

private:
    explicit AllUsersWindowManagerLite();
    ~AllUsersWindowManagerLite() override;
    friend class sptr<AllUsersWindowManagerLite>;

    void OnUserAdded(int32_t userId);
    void OnUserRemoved(int32_t userId);
    void RegisterUserChangeListeners();

    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_ALL_USERS_WINDOW_MANAGER_LITE_H