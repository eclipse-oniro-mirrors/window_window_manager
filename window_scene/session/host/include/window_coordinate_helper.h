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

#ifndef OHOS_ROSEN_WINDOW_COORDINATE_HELPER_H
#define OHOS_ROSEN_WINDOW_COORDINATE_HELPER_H

#include <functional>

#include "interfaces/include/ws_common.h"

namespace OHOS::Rosen {

/**
 * @brief Provides coordinate conversion utilities for windows across screens.
 */
class WindowCoordinateHelper final {
public:
    WindowCoordinateHelper() = delete;
    ~WindowCoordinateHelper() = delete;

    /**
     * @brief Determines whether a screen can participate in target screen derivation.
     *
     * @param candidateScreenId ID of a candidate target screen.
     * @return True if the candidate screen can participate in derivation; otherwise false.
     */
    using ScreenCandidateFilter = std::function<bool(ScreenId candidateScreenId)>;

    /**
     * @brief Converts a screen-relative rectangle to a rectangle in global display coordinates.
     *
     * @param screenId ID of the screen containing the rectangle.
     * @param screenRelativeRect Rectangle relative to the screen origin.
     * @return Rectangle in global display coordinates.
     */
    static WSRect ConvertToGlobalDisplayRect(ScreenId screenId, const WSRect& screenRelativeRect);

    /**
     * @brief Converts a global display rectangle to a rectangle relative to the derived target screen.
     *
     * The optional filter only affects target screen derivation. When no candidate screen matches,
     * the original screen is used as the fallback target.
     *
     * @param originalScreenId ID of the screen where the window currently resides.
     * @param globalDisplayRect Rectangle in global display coordinates.
     * @param screenCandidateFilter Optional predicate used to exclude candidate screens.
     * @return Target screen ID and the rectangle relative to that screen.
     */
    static WSScreenRelativeRect ConvertToScreenRelativeRect(
        ScreenId originalScreenId,
        const WSRect& globalDisplayRect,
        const ScreenCandidateFilter& screenCandidateFilter = nullptr);
};

} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_COORDINATE_HELPER_H
