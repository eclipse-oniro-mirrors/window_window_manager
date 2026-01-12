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

#include <charconv>
#include <cstdint>
#include <string>
#include <sstream>
#include <cinttypes>
#include <set>

#include "screen_edid_parse.h"
#include "setting_observer.h"
#include "dm_common.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace Rosen {
class ScreenSettingHelper {
public:
    static void RegisterSettingDpiObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingDpiObserver();
    static bool GetSettingDpi(uint32_t& dpi, const std::string& key = SETTING_DPI_KEY);
    static bool SetSettingDefaultDpi(uint32_t& dpi, const std::string& key);
    static bool GetSettingValue(uint32_t& value, const std::string& key);
    static bool SetSettingValue(const std::string& key, uint32_t value);
    static bool GetSettingValue(const std::string& key, std::string& value);
    static bool SetSettingValue(const std::string& key, const std::string& value);
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
    static bool IsNumber(const std::string& str);
    static std::map<std::string, MultiScreenInfo> GetMultiScreenInfo(const std::string& key = SETTING_SCREEN_MODE_KEY);
    static std::map<std::string, SupportedScreenModes> GetResolutionMode(const std::string& key =
        SETTING_SCREEN_RESOLUTION_MODE_KEY);
    static std::map<std::string, uint32_t> GetBorderingAreaPercent(const std::string& key =
        SETTING_SCREEN_BORDERING_AREA_PERCENT_KEY);
    static bool GetScreenMode(MultiScreenInfo& info, const std::string& inputString);
    static bool UpdateScreenMode(MultiScreenInfo& info, uint32_t mode, bool isExternal);
    static bool GetScreenActiveMode(SupportedScreenModes& info, const std::string& inputString);
    static bool GetAreaPercent(uint32_t& borderingAreaPercent, const std::string& inputString);
    static bool GetScreenRelativePosition(MultiScreenInfo& info, const std::string& inputString);
    static ScreenShape GetScreenShape(ScreenId screenId);
    static void RegisterSettingWireCastObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingWireCastObserver();
    static void RegisterSettingBorderingAreaPercentObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingBorderingAreaPercentObserver();
    static void RegisterSettingExtendScreenDpiObserver(SettingObserver::UpdateFunc func);
    static void UnRegisterSettingExtendScreenDpiObserver();
    static void RegisterSettingDuringCallStateObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingDuringCallStateObserver();
    static bool GetSettingDuringCallState(bool& enable, const std::string& key = SETTING_DURING_CALL_KEY);
    static bool SetSettingDuringCallState(const std::string& key, bool value);
    static bool GetSettingExtendScreenDpi(float& coef, const std::string& key = SETTING_EXTEND_DPI_KEY);
    static bool ConvertStrToUint64(const std::string& str, uint64_t& num);
    static bool ConvertStrToInt32(const std::string& str, int32_t& num);
    static bool GetResolutionEffect(bool& enable, const std::string& serialNumber,
        const std::string& key = SETTING_RESOLUTION_EFFECT_KEY);
    static void RegisterSettingResolutionEffectObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingResolutionEffectObserver();
    static void RegisterRotationCorrectionExemptionListObserver(SettingObserver::UpdateFunc func);
    static void UnregisterRotationCorrectionExemptionListObserver();
    static bool GetRotationCorrectionExemptionList(std::vector<std::string>& exemptionApps,
        const std::string& key = SETTING_COMPATIBLE_APP_STRATEGY_KEY);
    static void GetCorrectionExemptionListFromJson(const std::string& exemptionListJsonStr,
        std::vector<std::string>& exemptionApps);
    template<typename T>
    static bool GetJsonValue(const nlohmann::json& payload, const std::string& key, T& result)
    {
        if (!payload.contains(key)) {
            return false;
        }
        if constexpr (std::is_same_v<T, std::string>) {
            if (payload[key].is_string()) {
                result = payload[key].get<std::string>();
                return true;
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            if (payload[key].is_boolean()) {
                result = payload[key].get<bool>();
                return true;
            }
        } else if constexpr (std::is_arithmetic_v<T>) {
            if (payload[key].is_number()) {
                result = payload[key].get<int32_t>();
                return true;
            }
        }
        return false;
    }
private:
    static const constexpr char* SETTING_DPI_KEY {"user_set_dpi_value"};
    static const constexpr char* SETTING_CAST_KEY {"huaweicast.data.privacy_projection_state"};
    static const constexpr char* SETTING_ROTATION_KEY {"screen_rotation_value"};
    static const constexpr char* SETTING_ROTATION_SCREEN_ID_KEY {"screen_rotation_screen_id_value"};
    static const constexpr char* SETTING_SCREEN_MODE_KEY {"user_set_screen_mode_edid"};
    static const constexpr char* SETTING_EXTEND_DPI_KEY {"user_set_dpi_extend"};
    static const constexpr char* SETTING_DURING_CALL_KEY {"during_call_state"};
    static const constexpr char* SETTING_RESOLUTION_EFFECT_KEY {"user_set_resolution_effect_select"};
    static const constexpr char* SETTING_COMPATIBLE_APP_STRATEGY_KEY {"COMPATIBLE_APP_STRATEGY"};
    static const constexpr char* SETTING_SCREEN_RESOLUTION_MODE_KEY {"user_set_resolution_mode"};
    static const constexpr char* SETTING_SCREEN_BORDERING_AREA_PERCENT_KEY {"bordering_area_percent"};
    static const constexpr uint32_t BASE_TEN = 10;
    static sptr<SettingObserver> dpiObserver_;
    static sptr<SettingObserver> castObserver_;
    static sptr<SettingObserver> rotationObserver_;
    static sptr<SettingObserver> wireCastObserver_;
    static sptr<SettingObserver> extendScreenDpiObserver_;
    static sptr<SettingObserver> duringCallStateObserver_;
    static sptr<SettingObserver> resolutionEffectObserver_;
    static sptr<SettingObserver> correctionExemptionListObserver_;
    static sptr<SettingObserver> borderingAreaPercentObserver_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_SCENE_SCREEN_SETTING_HELPER_H
