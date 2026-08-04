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

#include "window_coordinate_helper.h"

#include <cmath>
#include <map>

#include "dm_common.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "window_manager_hilog.h"
#include "wm_math.h"

namespace OHOS::Rosen {
namespace {
/**
 * @brief Result of matching a global window rectangle to a screen.
 */
struct ScreenCandidate {
    /** @brief ID of the matched screen. */
    ScreenId screenId = SCREEN_ID_INVALID;

    /** @brief Global rectangle of the matched screen. */
    WSRect screenRect = WSRect::EMPTY_RECT;

    /** @brief Intersection area between the scaled window rectangle and the matched screen. */
    uint64_t intersectionArea = 0;
};

/**
 * @brief Builds a screen rectangle in global coordinates.
 *
 * @param screenProperty Screen property containing position and bounds.
 * @return Screen rectangle in global coordinates.
 */
WSRect BuildScreenRect(const ScreenProperty& screenProperty)
{
    return {
        screenProperty.GetX(),
        screenProperty.GetY(),
        screenProperty.GetBounds().rect_.GetWidth(),
        screenProperty.GetBounds().rect_.GetHeight()
    };
}

/**
 * @brief Scales a global window rectangle from the original screen VPR to a candidate screen VPR.
 *
 * @param globalDisplayRect Window rectangle in global coordinates.
 * @param widthInVp Window width in virtual pixels.
 * @param heightInVp Window height in virtual pixels.
 * @param candidateScreenVpr Virtual pixel ratio of the candidate screen.
 * @return Global rectangle with its size scaled for the candidate screen.
 */
WSRect ScaleGlobalDisplayRect(
    const WSRect& globalDisplayRect, float widthInVp, float heightInVp, float candidateScreenVpr)
{
    return {
        globalDisplayRect.posX_,
        globalDisplayRect.posY_,
        static_cast<int32_t>(std::round(widthInVp * candidateScreenVpr)),
        static_cast<int32_t>(std::round(heightInVp * candidateScreenVpr))
    };
}

/**
 * @brief Finds the allowed screen with the largest intersection with a global window rectangle.
 *
 * @param screenProperties Properties of all candidate screens.
 * @param globalDisplayRect Window rectangle in global coordinates.
 * @param widthInVp Window width in virtual pixels.
 * @param heightInVp Window height in virtual pixels.
 * @param screenCandidateFilter Optional predicate for filtering candidate screens.
 * @return Best matching screen candidate, or an invalid candidate when no screen intersects.
 */
ScreenCandidate FindBestScreenCandidate(const std::map<ScreenId, ScreenProperty>& screenProperties,
                                        const WSRect& globalDisplayRect, float widthInVp, float heightInVp,
                                        const WindowCoordinateHelper::ScreenCandidateFilter& screenCandidateFilter)
{
    ScreenCandidate bestCandidate;
    for (const auto& [screenId, screenProperty] : screenProperties) {
        if (screenCandidateFilter && !screenCandidateFilter(screenId)) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "Skip filtered candidate screen, screenId: %{public}" PRIu64, screenId);
            continue;
        }
        const WSRect screenRect = BuildScreenRect(screenProperty);
        const WSRect scaledGlobalRect = ScaleGlobalDisplayRect(
            globalDisplayRect, widthInVp, heightInVp, screenProperty.GetVirtualPixelRatio());
        const uint64_t intersectionArea = scaledGlobalRect.IntersectionArea<uint64_t>(screenRect);
        if (intersectionArea > bestCandidate.intersectionArea ||
            (intersectionArea != 0 && intersectionArea == bestCandidate.intersectionArea &&
             screenId < bestCandidate.screenId)) {
            bestCandidate.screenId = screenId;
            bestCandidate.screenRect = screenRect;
            bestCandidate.intersectionArea = intersectionArea;
        }
    }
    return bestCandidate;
}
} // namespace

WSRect WindowCoordinateHelper::ConvertToGlobalDisplayRect(ScreenId screenId, const WSRect& screenRelativeRect)
{
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(screenId);
    if (!screenSession) {
        TLOGD(WmsLogTag::WMS_LAYOUT,
            "Screen not found, screenId: %{public}" PRIu64 ", screenRelativeRect: %{public}s",
            screenId, screenRelativeRect.ToString().c_str());
        return screenRelativeRect;
    }
    const auto& screenProperty = screenSession->GetScreenProperty();
    WSRect globalDisplayRect = screenRelativeRect.WithOffset(screenProperty.GetX(), screenProperty.GetY());
    TLOGD(WmsLogTag::WMS_LAYOUT,
          "screenId: %{public}" PRIu64 ", screenRelativeRect: %{public}s, globalDisplayRect: %{public}s",
          screenId, screenRelativeRect.ToString().c_str(), globalDisplayRect.ToString().c_str());
    return globalDisplayRect;
}

WSScreenRelativeRect WindowCoordinateHelper::ConvertToScreenRelativeRect(
    ScreenId originalScreenId, const WSRect& globalDisplayRect, const ScreenCandidateFilter& screenCandidateFilter)
{
    const auto screenProperties = ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    const auto originalScreenIter = screenProperties.find(originalScreenId);
    if (originalScreenIter == screenProperties.end()) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
              "Original screen not found, screenId: %{public}" PRIu64 ", globalDisplayRect: %{public}s",
              originalScreenId, globalDisplayRect.ToString().c_str());
        return { MAIN_SCREEN_ID_DEFAULT, globalDisplayRect };
    }

    const auto& originalScreenProperty = originalScreenIter->second;
    const float originalScreenVpr = originalScreenProperty.GetVirtualPixelRatio();
    if (MathHelper::NearZero(originalScreenVpr)) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
              "Original screen vpr is 0, screenId: %{public}" PRIu64 ", globalDisplayRect: %{public}s",
              originalScreenId, globalDisplayRect.ToString().c_str());
        return { MAIN_SCREEN_ID_DEFAULT, globalDisplayRect };
    }

    const float widthInVp = globalDisplayRect.width_ / originalScreenVpr;
    const float heightInVp = globalDisplayRect.height_ / originalScreenVpr;
    const auto candidate =
        FindBestScreenCandidate(screenProperties, globalDisplayRect, widthInVp, heightInVp, screenCandidateFilter);
    const bool hasCandidate = candidate.screenId != SCREEN_ID_INVALID;
    const ScreenId matchedScreenId = hasCandidate ? candidate.screenId : originalScreenId;
    const WSRect matchedScreenRect = hasCandidate ? candidate.screenRect : BuildScreenRect(originalScreenProperty);
    const WSRect screenRelativeRect = globalDisplayRect.WithOffset(-matchedScreenRect.posX_, -matchedScreenRect.posY_);
    TLOGD(WmsLogTag::WMS_LAYOUT,
          "matchedScreenId: %{public}" PRIu64 ", globalDisplayRect: %{public}s, screenRelativeRect: %{public}s",
          matchedScreenId, globalDisplayRect.ToString().c_str(), screenRelativeRect.ToString().c_str());
    return { matchedScreenId, screenRelativeRect };
}
} // namespace OHOS::Rosen
