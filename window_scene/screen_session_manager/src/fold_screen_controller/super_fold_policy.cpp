/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "fold_screen_controller/super_fold_policy.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(SuperFoldPolicy)
namespace {
constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
constexpr ScreenId SCREEN_ID_DEFAULT = 0;
constexpr uint64_t SCREEN_ID_SIZE_MAX = 2;
constexpr uint64_t SCREEN_ID_SIZE_ONE = 1;
}

bool SuperFoldPolicy::IsFakeDisplayExist()
{
    return ScreenSessionManager::GetInstance().GetDisplayInfoById(DISPLAY_ID_FAKE) != nullptr;
}

bool SuperFoldPolicy::IsNeedSetSnapshotRect(DisplayId displayId)
{
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_DEFAULT);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "session nullptr.");
        return false;
    }
    auto rotation = screenSession->GetRotation();
    return ((displayId == DEFAULT_DISPLAY_ID) || (displayId == DISPLAY_ID_FAKE)) &&
        (rotation == Rotation::ROTATION_0 || rotation == Rotation::ROTATION_180);
}

Drawing::Rect SuperFoldPolicy::GetSnapshotRect(DisplayId displayId, bool isCaptureFullOfScreen)
{
    Drawing::Rect snapshotRect = {0, 0, 0, 0};
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_DEFAULT);
    auto defaultInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(DEFAULT_DISPLAY_ID);
    if (screenSession == nullptr || defaultInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "session or display nullptr.");
        return snapshotRect;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    auto screenWidth = screenProperty.GetPhyBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetPhyBounds().rect_.GetHeight();
    DMRect creaseRect = screenProperty.GetCreaseRect();
    auto fakeInfo =  ScreenSessionManager::GetInstance().GetDisplayInfoById(DISPLAY_ID_FAKE);
    if (displayId == DISPLAY_ID_FAKE && !isCaptureFullOfScreen) {
        if (fakeInfo != nullptr) {
            snapshotRect = {0, defaultInfo->GetHeight() + static_cast<int32_t>(creaseRect.height_),
                screenWidth, screenHeight};
        }
    } else {
        SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
        bool isSystemKeyboardOn = SuperFoldStateManager::GetInstance().GetSystemKeyboardStatus();
        if (status == SuperFoldStatus::KEYBOARD ||
            (fakeInfo != nullptr && !isCaptureFullOfScreen)|| isSystemKeyboardOn) {
            snapshotRect = {0, 0, defaultInfo->GetWidth(), defaultInfo->GetHeight()};
        } else {
            snapshotRect = {0, 0, screenWidth, screenHeight};
        }
    }
    std::ostringstream oss;
    oss << "snapshot displayId: " << static_cast<uint32_t>(displayId)
        << " left: 0"
        << " top:" << snapshotRect.top_
        << " right:" << snapshotRect.right_
        << " bottom:" << snapshotRect.bottom_
        << " isCaptureFullOfScreen:" << isCaptureFullOfScreen;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return snapshotRect;
}

DMRect SuperFoldPolicy::GetRecordRect(const std::vector<ScreenId>& screenIds)
{
    DMRect recordRect = {0, 0, 0, 0};
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_DEFAULT);
    auto defaultInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(DEFAULT_DISPLAY_ID);
    if (screenSession == nullptr || defaultInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "session or display nullptr.");
        return recordRect;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    DMRect creaseRect = screenProperty.GetCreaseRect();
    SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    auto fakeInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(DISPLAY_ID_FAKE);
    if ((std::find(screenIds.begin(), screenIds.end(), DISPLAY_ID_FAKE) != screenIds.end()) && screenIds.size() == 1) {
        // record for fake screensession
        if (fakeInfo != nullptr) {
            recordRect = {0, defaultInfo->GetHeight() + static_cast<int32_t>(creaseRect.height_),
                defaultInfo->GetWidth(), fakeInfo->GetHeight()};
        }
    } else if ((std::find(screenIds.begin(), screenIds.end(), DEFAULT_DISPLAY_ID) != screenIds.end())
        && screenIds.size() == 1) {
        // record for 0 for one display
        bool isSystemKeyboardOn = SuperFoldStateManager::GetInstance().GetSystemKeyboardStatus();
        if (status == SuperFoldStatus::EXPANDED) {
            recordRect = {0, 0, 0, 0};
        } else if (status == SuperFoldStatus::KEYBOARD || fakeInfo != nullptr || isSystemKeyboardOn) {
            recordRect = {0, 0, defaultInfo->GetWidth(), defaultInfo->GetHeight()};
        }
    }
    std::ostringstream oss;
    oss << "snapshot left: 0"
        << " top:" << recordRect.posY_
        << " right:" << recordRect.width_
        << " bottom:" << recordRect.height_;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return recordRect;
}

ScreenId SuperFoldPolicy::GetRealScreenId(const std::vector<ScreenId>& screenIds)
{
    if (screenIds.empty() || screenIds.size() > SCREEN_ID_SIZE_MAX) {
        TLOGE(WmsLogTag::DMS, "mainScreenIds null");
        return SCREEN_ID_INVALID;
    }
    if (screenIds.size() == SCREEN_ID_SIZE_ONE) {
        TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64, static_cast<uint64_t>(screenIds[0]));
        if ((screenIds[0] == SCREEN_ID_DEFAULT || screenIds[0] == DISPLAY_ID_FAKE)) {
            return SCREEN_ID_DEFAULT;
        } else {
            return screenIds[0];
        }
    } else if (std::find(screenIds.begin(), screenIds.end(), SCREEN_ID_DEFAULT) != screenIds.end()
        && std::find(screenIds.begin(), screenIds.end(), DISPLAY_ID_FAKE) != screenIds.end()) {
        TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", %{public}" PRIu64,
            static_cast<uint64_t>(screenIds[0]), static_cast<uint64_t>(screenIds[1]));
        return SCREEN_ID_DEFAULT;
    }
    return SCREEN_ID_INVALID;
}

} // namespace OHOS::Rosen
