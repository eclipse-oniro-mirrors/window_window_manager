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

#ifndef OHOS_ROSEN_FOLD_SCREEN_COMMON_H
#define OHOS_ROSEN_FOLD_SCREEN_COMMON_H

namespace OHOS {
namespace Rosen {
namespace {
    /**
     * ACE call to native event name, use for fold screen
     */
    const std::string WINDOW_WATERFALL_EVENT = "win_waterfall_window_event";
    const std::string WINDOW_RELOCATION_EVENT = "win_relocation_to_top";
    const std::string WINDOW_WATERFALL_VISIBILITY_EVENT = "win_waterfall_visibility";
    const std::string WINDOW_GRAY_OUT_MAXIMIZE_EVENT = "win_gray_out_maximize_event";
    const std::string EVENT_NAME_CHANGE_WATER_FALL_BUTTON = "win_change_water_fall_button";
    const std::string WATER_FALL_BUTTON_SHOW = "win_water_fall_button_show";
    const std::string WATER_FALL_BUTTON_HIDE = "win_water_fall_button_hide";
}
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_FOLD_SCREEN_COMMON_H

