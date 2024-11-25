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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCREEN_SETTING_HELPER_H
#define OHOS_ROSEN_WINDOW_SCENE_SCREEN_SETTING_HELPER_H

#include <cstdint>
#include <string>
#include <sstream>

#include "setting_observer.h"

namespace OHOS {
namespace Rosen {
class ScreenSettingHelper {
public:
    static void RegisterSettingDpiObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingDpiObserver();
    static void RegisterExtendSettingDpiObserver(SettingObserver::UpdateFunc func);
    static void UnregisterExtendSettingDpiObserver();
    static bool GetSettingDpi(uint32_t& dpi, const std::string& key = SETTING_DPI_KEY);
    static bool SetSettingDefaultDpi(uint32_t& dpi, const std::string& key);
    static bool GetSettingValue(uint32_t& value, const std::string& key);
    static void RegisterSettingCastObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingCastObserver();
    static bool GetSettingCast(bool& enable, const std::string& key = SETTING_CAST_KEY);
    static void RegisterSettingRotationObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingRotationObserver();
    static void SetSettingRotation(int32_t rotation);
    static void SetSettingRotationScreenId(int32_t screenId);
    static bool GetSettingRotation(int32_t& rotation, const std::string& key = SETTING_ROTATION_KEY);
    static bool GetSettingRotationScreenID(int32_t& screenId, const std::string& key = SETTING_ROTATION_SCREEN_ID_KEY);
    static std::string RemoveInvalidChar(const std::string& input);
    static bool SplitString(std::vector<std::string>& splitValues, const std::string& input, char delimiter = ',');
    static int32_t GetDataFromString(std::vector<uint64_t>& datas, const std::string& inputString);
    static bool GetSettingRecoveryResolutionString(std::vector<std::string>& resolutionStrings,
        const std::string& key = SETTING_RECOVERY_RESOLUTION_KEY);
    static bool GetSettingRecoveryResolutionMap(std::map<uint64_t, std::pair<int32_t, int32_t>>& resolution);
    static bool GetSettingScreenModeString(std::vector<std::string>& screenModeStrings,
        const std::string& key = SETTING_SCREEN_MODE_KEY);
    static bool GetSettingScreenModeMap(std::map<uint64_t, uint32_t>& screenMode);
    static bool GetSettingRelativePositionString(std::vector<std::string>& relativePositionStrings,
        const std::string& key = SETTING_RELATIVE_POSITION_KEY);
    static bool GetSettingRelativePositionMap(std::map<uint64_t, std::pair<uint32_t, uint32_t>>& relativePosition);

private:
    static const constexpr char* SETTING_DPI_KEY {"user_set_dpi_value"};
    static const constexpr char* SETTING_DPI_KEY_EXTEND {"user_set_dpi_value_extend"};
    static const constexpr char* SETTING_CAST_KEY {"huaweicast.data.privacy_projection_state"};
    static const constexpr char* SETTING_ROTATION_KEY {"screen_rotation_value"};
    static const constexpr char* SETTING_ROTATION_SCREEN_ID_KEY {"screen_rotation_screen_id_value"};
    static const constexpr char* SETTING_RECOVERY_RESOLUTION_KEY {"user_set_recovery_resolution"};
    static const constexpr char* SETTING_SCREEN_MODE_KEY {"user_set_last_screen_mode"};
    static const constexpr char* SETTING_RELATIVE_POSITION_KEY {"user_set_relative_position"};
    static sptr<SettingObserver> dpiObserver_;
    static sptr<SettingObserver> extendDpiObserver_;
    static sptr<SettingObserver> castObserver_;
    static sptr<SettingObserver> rotationObserver_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_SCENE_SCREEN_SETTING_HELPER_H
