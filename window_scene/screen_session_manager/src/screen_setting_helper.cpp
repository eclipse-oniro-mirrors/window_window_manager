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

#include "screen_setting_helper.h"

#include <cerrno>
#include <parameters.h>

#include "screen_session_manager/include/screen_session_manager.h"
#include "setting_provider.h"
#include "system_ability_definition.h"
#include "window_manager_hilog.h"
#include "fold_screen_state_internel.h"
#include "fold_screen_controller/super_fold_state_manager.h"

namespace OHOS {
namespace Rosen {
sptr<SettingObserver> ScreenSettingHelper::dpiObserver_;
sptr<SettingObserver> ScreenSettingHelper::castObserver_;
sptr<SettingObserver> ScreenSettingHelper::rotationObserver_;
sptr<SettingObserver> ScreenSettingHelper::wireCastObserver_;
sptr<SettingObserver> ScreenSettingHelper::extendScreenDpiObserver_;
sptr<SettingObserver> ScreenSettingHelper::duringCallStateObserver_;
sptr<SettingObserver> ScreenSettingHelper::resolutionEffectObserver_;
sptr<SettingObserver> ScreenSettingHelper::correctionExemptionListObserver_;
sptr<SettingObserver> ScreenSettingHelper::borderingAreaPercentObserver_;
constexpr int32_t PARAM_NUM_TEN = 10;
constexpr uint32_t EXPECT_ACTIVE_MODE_SIZE = 4;
constexpr uint32_t EXPECT_SCREEN_MODE_SIZE = 2;
constexpr uint32_t EXPECT_PERCENT_SIZE = 1;
constexpr uint32_t EXPECT_RELATIVE_POSITION_SIZE = 3;
constexpr uint32_t VALID_MULTI_SCREEN_INFO_SIZE = 5;
constexpr uint32_t VALID_PERCENT_SIZE = 2;
constexpr uint32_t INDEX_SCREEN_INFO = 0;
constexpr uint32_t INDEX_SCREEN_MODE = 1;
constexpr uint32_t INDEX_FIRST_RELATIVE_POSITION = 2;
constexpr uint32_t INDEX_SECOND_RELATIVE_POSITION = 3;
constexpr uint32_t INDEX_THREE_ACTIVE_MODE_ID = 4;
constexpr uint32_t DATA_INDEX_ZERO = 0;
constexpr uint32_t DATA_INDEX_ONE = 1;
constexpr uint32_t DATA_INDEX_TWO = 2;
constexpr uint32_t DATA_INDEX_THERE = 3;
constexpr uint32_t SCREEN_MAIN_IN_DATA = 0;
constexpr uint32_t SCREEN_MIRROR_IN_DATA = 1;
constexpr uint32_t SCREEN_EXTEND_IN_DATA = 2;
constexpr ScreenId RS_ID_INTERNAL = 0;
constexpr uint32_t EXTEND_SCREEN_DPI_LEVEL_NEG_ONE = -1;
constexpr uint32_t EXTEND_SCREEN_DPI_LEVEL_ZERO = 0;
constexpr uint32_t EXTEND_SCREEN_DPI_LEVEL_ONE = 1;
constexpr float EXTEND_SCREEN_DPI_NEG_ONE_PARAMETER = 0.7f;
constexpr float EXTEND_SCREEN_DPI_ZERO_PARAMETER = 0.85f;
constexpr float EXTEND_SCREEN_DPI_ONE_PARAMETER = 1.00f;
const std::string SCREEN_SHAPE = system::GetParameter("const.window.screen_shape", "0:0");
constexpr int32_t INDEX_EXTEND_SCREEN_DPI_POSITION = -1;
const std::string ENABLE_RESOLUTION_EFFECT = "1";
constexpr int32_t EXPECT_SCREEN_RESOLUTION_EFFECT_SIZE = 2;
constexpr int32_t INDEX_SCREEN_RESOLUTION_EFFECT_SN = 0;
constexpr int32_t INDEX_SCREEN_RESOLUTION_EFFECT_EN = 1;

void ScreenSettingHelper::RegisterSettingDpiObserver(SettingObserver::UpdateFunc func)
{
    if (dpiObserver_) {
        TLOGD(WmsLogTag::DMS, "setting dpi observer is registered");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    dpiObserver_ = provider.CreateObserver(SETTING_DPI_KEY, func);
    if (dpiObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
    }
    ErrCode ret = provider.RegisterObserver(dpiObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        dpiObserver_ = nullptr;
    }
}

void ScreenSettingHelper::UnregisterSettingDpiObserver()
{
    if (dpiObserver_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "dpiObserver_ is nullptr");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = provider.UnregisterObserver(dpiObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    dpiObserver_ = nullptr;
}

bool ScreenSettingHelper::GetSettingDpi(uint32_t& dpi, const std::string& key)
{
    return GetSettingValue(dpi, key);
}

bool ScreenSettingHelper::GetSettingValue(uint32_t& value, const std::string& key)
{
    SettingProvider& settingData = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    int32_t getValue;
    ErrCode ret = settingData.GetIntValue(key, getValue);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    value = static_cast<uint32_t>(getValue);
    return true;
}

// LCOV_EXCL_START
bool ScreenSettingHelper::GetSettingValue(const std::string& key, std::string& value)
{
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    std::string getValue = "";
    ErrCode ret = provider.GetStringValue(key, getValue);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    value = getValue;
    return true;
}

bool ScreenSettingHelper::SetSettingValue(const std::string& key, uint32_t value)
{
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = provider.PutIntValue(key, value, false);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool ScreenSettingHelper::SetSettingValue(const std::string& key, const std::string& value)
{
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = provider.PutStringValue(key, value, false);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}
// LCOV_EXCL_STOP

bool ScreenSettingHelper::SetSettingDefaultDpi(uint32_t& dpi, const std::string& key)
{
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = provider.PutIntValue(key, dpi, false);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "put int value failed, ret=%{public}d", ret);
        return false;
    }
    return true;
}

// LCOV_EXCL_START
void ScreenSettingHelper::RegisterSettingCastObserver(SettingObserver::UpdateFunc func)
{
    if (castObserver_) {
        TLOGD(WmsLogTag::DMS, "setting cast observer is registered");
        return;
    }
    SettingProvider& castProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    castObserver_ = castProvider.CreateObserver(SETTING_CAST_KEY, func);
    if (castObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
    }
    ErrCode ret = castProvider.RegisterObserver(castObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        castObserver_ = nullptr;
    }
}

void ScreenSettingHelper::UnregisterSettingCastObserver()
{
    if (castObserver_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "castObserver_ is nullptr");
        return;
    }
    SettingProvider& castProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = castProvider.UnregisterObserver(castObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    castObserver_ = nullptr;
}

bool ScreenSettingHelper::GetSettingCast(bool& enable, const std::string& key)
{
    SettingProvider& castProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = castProvider.GetBoolValue(key, enable);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    return true;
}
// LCOV_EXCL_STOP

void ScreenSettingHelper::RegisterSettingRotationObserver(SettingObserver::UpdateFunc func)
{
    if (rotationObserver_ != nullptr) {
        TLOGI(WmsLogTag::DMS, "setting rotation observer is registered");
        return;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    rotationObserver_ = settingProvider.CreateObserver(SETTING_ROTATION_KEY, func);
    if (rotationObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
    }
    ErrCode ret = settingProvider.RegisterObserver(rotationObserver_);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        rotationObserver_ = nullptr;
    }
}

// LCOV_EXCL_START
void ScreenSettingHelper::UnregisterSettingRotationObserver()
{
    if (rotationObserver_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "rotationObserver_ is nullptr");
        return;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.UnregisterObserver(rotationObserver_);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
    }
    rotationObserver_ = nullptr;
}

void ScreenSettingHelper::SetSettingRotation(int32_t rotation)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_ROTATION_KEY, rotation, true);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        return;
    }
    TLOGE(WmsLogTag::DMS, "succeed, ret:%{public}d", ret);
}

bool ScreenSettingHelper::GetSettingRotation(int32_t& rotation, const std::string& key)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetIntValue(key, rotation);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        return false;
    }
    TLOGE(WmsLogTag::DMS, "current rotation:%{public}d", rotation);
    return true;
}

void ScreenSettingHelper::SetSettingRotationScreenId(int32_t screenId)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_ROTATION_SCREEN_ID_KEY, screenId, false);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        return;
    }
    TLOGE(WmsLogTag::DMS, "succeed, ret:%{public}d", ret);
}

bool ScreenSettingHelper::GetSettingRotationScreenID(int32_t& screenId, const std::string& key)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetIntValue(key, screenId);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        return false;
    }
    TLOGE(WmsLogTag::DMS, "current rotation screen id:%{public}d", screenId);
    return true;
}

std::string ScreenSettingHelper::RemoveInvalidChar(const std::string& input)
{
    TLOGW(WmsLogTag::DMS, "input string: %{public}s", input.c_str());
    std::string resultString = "";
    for (char character : input) {
        if (std::isdigit(character) || character == ' ' || character == ',' ||
            character == '.' || character == ';' || character == '_') {
            resultString += character;
        }
    }
    return resultString;
}

bool ScreenSettingHelper::SplitString(std::vector<std::string>& splitValues, const std::string& input,
    char delimiter)
{
    TLOGI(WmsLogTag::DMS, "input string: %{public}s", input.c_str());
    if (input.size() == 0) {
        TLOGE(WmsLogTag::DMS, "noting in input string");
        return false;
    }
    std::stringstream stream(input);
    std::string token = "";
    while (std::getline(stream, token, delimiter)) {
        splitValues.push_back(token);
    }
    if (splitValues.size() == 0) {
        TLOGE(WmsLogTag::DMS, "failed, noting split");
        return false;
    }
    return true;
}

bool ScreenSettingHelper::IsNumber(const std::string& str)
{
    int32_t length = static_cast<int32_t>(str.size());
    int32_t start = 0;
    while (start < length && isspace(str[start])) {
        start++;
    }
    bool hasDot = false;
    bool hasDigit = false;
    if (start < length && str[start] == '-') {
        start++;
    }
    for (int32_t i = start; i < length; ++i) {
        if (str[i] == '.') {
            if (hasDot) {
                return false;
            }
            hasDot = true;
        } else if (isdigit(str[i])) {
            hasDigit = true;
        } else {
            return false;
        }
    }
    return hasDigit;
}

bool ScreenSettingHelper::ConvertStrToUint64(const std::string& str, uint64_t& num)
{
    char *endFlag;
    errno = 0;
    uint64_t tmp = static_cast<uint64_t>(std::strtoull(str.c_str(), &endFlag, BASE_TEN));
    if (errno == ERANGE) {
        return false;
    } else if (endFlag == nullptr || *endFlag != '\0') {
        return false;
    }
    num = tmp;
    return true;
}

std::map<std::string, MultiScreenInfo> ScreenSettingHelper::GetMultiScreenInfo(const std::string& key)
{
    std::map<std::string, MultiScreenInfo> multiScreenInfoMap = {};
    std::string value = "";
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetStringValueMultiUser(key, value);
    TLOGW(WmsLogTag::DMS, "value=%{public}s", value.c_str());
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return multiScreenInfoMap;
    }
    std::string validString = RemoveInvalidChar(value);
    std::vector<std::string> restoredScreen = {};
    bool split = SplitString(restoredScreen, validString, ',');
    if (!split) {
        TLOGE(WmsLogTag::DMS, "split screen failed");
        return multiScreenInfoMap;
    }
    for (auto infoString : restoredScreen) {
        std::vector<std::string> infoVector = {};
        split = SplitString(infoVector, infoString, ';');
        if (!split || infoVector.size() != VALID_MULTI_SCREEN_INFO_SIZE) {
            TLOGE(WmsLogTag::DMS, "split info failed");
            continue;
        }
        MultiScreenInfo info = {};
        if (!GetScreenMode(info, infoVector[INDEX_SCREEN_MODE])) {
            continue;
        }
        if (!GetScreenRelativePosition(info, infoVector[INDEX_FIRST_RELATIVE_POSITION])) {
            continue;
        }
        if (!GetScreenRelativePosition(info, infoVector[INDEX_SECOND_RELATIVE_POSITION])) {
            continue;
        }
        if (info.mainScreenOption.screenId_ == info.secondaryScreenOption.screenId_) {
            TLOGE(WmsLogTag::DMS, "invalid screen of relative position!");
            continue;
        }
        multiScreenInfoMap[infoVector[INDEX_SCREEN_INFO]] = info;
    }
    return multiScreenInfoMap;
}

std::map<std::string, uint32_t> ScreenSettingHelper::GetBorderingAreaPercent(const std::string& key)
{
    std::map<std::string, uint32_t> borderingAreaPercentMap = {};
    std::string value = "";
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetStringValueMultiUser(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return borderingAreaPercentMap;
    }
    std::string validString = RemoveInvalidChar(value);
    std::vector<std::string> restoredScreen = {};
    bool split = SplitString(restoredScreen, validString, ',');
    if (!split) {
        TLOGE(WmsLogTag::DMS, "split screen failed");
        return borderingAreaPercentMap;
    }
    for (auto infoString : restoredScreen) {
        std::vector<std::string> infoVector = {};
        split = SplitString(infoVector, infoString, ';');
        if (!split || infoVector.size() != VALID_PERCENT_SIZE) {
            TLOGE(WmsLogTag::DMS, "split info failed");
            continue;
        }
        uint32_t borderingAreaPercent = {};
        if (!GetAreaPercent(borderingAreaPercent, infoVector[INDEX_SCREEN_MODE])) {
            continue;
        }
        borderingAreaPercentMap[infoVector[INDEX_SCREEN_INFO]] = borderingAreaPercent;
    }
    return borderingAreaPercentMap;
}

bool ScreenSettingHelper::GetAreaPercent(uint32_t& borderingAreaPercent, const std::string& inputString)
{
    std::vector<std::string> modeStr = {};
    bool split = SplitString(modeStr, inputString, ' ');
    uint32_t dataSize = modeStr.size();
    if (!split || dataSize != EXPECT_PERCENT_SIZE) {
        TLOGE(WmsLogTag::DMS, "split failed, data size: %{public}d", dataSize);
        return false;
    }

    uint32_t tmpBorderingAreaPercent = 50;
    if (!IsNumber(modeStr[DATA_INDEX_ZERO])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        tmpBorderingAreaPercent =
            static_cast<uint32_t>(strtoll(modeStr[DATA_INDEX_ZERO].c_str(), nullptr, PARAM_NUM_TEN));
    }
    borderingAreaPercent = tmpBorderingAreaPercent;
    TLOGI(WmsLogTag::DMS, "borderingAreaPercent: %{public}d", borderingAreaPercent);
    return true;
}

std::map<std::string, SupportedScreenModes> ScreenSettingHelper::GetResolutionMode(const std::string& key)
{
    std::map<std::string, SupportedScreenModes> resolutionMap = {};
    std::string value = "";
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetStringValueMultiUser(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return resolutionMap;
    }
    std::string validString = RemoveInvalidChar(value);
    std::vector<std::string> restoredScreen = {};
    bool split = SplitString(restoredScreen, validString, ',');
    if (!split) {
        TLOGE(WmsLogTag::DMS, "split screen failed");
        return resolutionMap;
    }
    for (auto infoString : restoredScreen) {
        std::vector<std::string> infoVector = {};
        split = SplitString(infoVector, infoString, ';');
        if (!split || infoVector.size() != VALID_MULTI_SCREEN_INFO_SIZE) {
            TLOGE(WmsLogTag::DMS, "split info failed");
            continue;
        }
        SupportedScreenModes info = {};
        if (!GetScreenActiveMode(info, infoVector[INDEX_SCREEN_MODE])) {
            continue;
        }
        resolutionMap[infoVector[INDEX_SCREEN_INFO]] = info;
    }
    return resolutionMap;
}

bool ScreenSettingHelper::GetScreenMode(MultiScreenInfo& info, const std::string& inputString)
{
    std::vector<std::string> screenMode = {};
    bool split = SplitString(screenMode, inputString, ' ');
    uint32_t dataSize = screenMode.size();
    if (!split || dataSize != EXPECT_SCREEN_MODE_SIZE) {
        TLOGE(WmsLogTag::DMS, "split failed, data size: %{public}d", dataSize);
        return false;
    }

    uint32_t mode = 0;
    if (!IsNumber(screenMode[DATA_INDEX_ZERO])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        mode = static_cast<uint32_t>(strtoll(screenMode[DATA_INDEX_ZERO].c_str(), nullptr, PARAM_NUM_TEN));
        TLOGW(WmsLogTag::DMS, "internal screen mode: %{public}d", mode);
        if (!UpdateScreenMode(info, mode, false)) {
            return false;
        }
    }
    if (!IsNumber(screenMode[DATA_INDEX_ONE])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        mode = static_cast<uint32_t>(strtoll(screenMode[DATA_INDEX_ONE].c_str(), nullptr, PARAM_NUM_TEN));
        TLOGW(WmsLogTag::DMS, "external screen mode: %{public}d", mode);
        if (!UpdateScreenMode(info, mode, true)) {
            return false;
        }
    }
    return true;
}

bool ScreenSettingHelper::UpdateScreenMode(MultiScreenInfo& info, uint32_t mode, bool isExternal)
{
    if (mode == SCREEN_MAIN_IN_DATA) {
        info.isExtendMain = isExternal;
    } else if (mode == SCREEN_MIRROR_IN_DATA) {
        info.multiScreenMode = MultiScreenMode::SCREEN_MIRROR;
    } else if (mode == SCREEN_EXTEND_IN_DATA) {
        info.multiScreenMode = MultiScreenMode::SCREEN_EXTEND;
    } else {
        TLOGE(WmsLogTag::DMS, "invalid mode!");
        return false;
    }
    return true;
}

bool ScreenSettingHelper::GetScreenRelativePosition(MultiScreenInfo& info, const std::string& inputString)
{
    std::vector<std::string> relativePosition = {};
    bool split = SplitString(relativePosition, inputString, ' ');
    uint32_t dataSize = relativePosition.size();
    if (!split || dataSize != EXPECT_RELATIVE_POSITION_SIZE) {
        TLOGE(WmsLogTag::DMS, "split failed, data size: %{public}d", dataSize);
        return false;
    }

    ScreenId screenId = SCREEN_ID_INVALID;
    uint32_t startX = 0;
    uint32_t startY = 0;
    if (!IsNumber(relativePosition[DATA_INDEX_ZERO])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        screenId = static_cast<ScreenId>(strtoll(relativePosition[DATA_INDEX_ZERO].c_str(), nullptr, PARAM_NUM_TEN));
    }
    if (!IsNumber(relativePosition[DATA_INDEX_ONE])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        startX = static_cast<uint32_t>(strtoll(relativePosition[DATA_INDEX_ONE].c_str(), nullptr, PARAM_NUM_TEN));
    }
    if (!IsNumber(relativePosition[DATA_INDEX_TWO])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        startY = static_cast<uint32_t>(strtoll(relativePosition[DATA_INDEX_TWO].c_str(), nullptr, PARAM_NUM_TEN));
    }
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", startX: %{public}d, startY: %{public}d",
        screenId, startX, startY);

    if ((RS_ID_INTERNAL == screenId && !info.isExtendMain) || (RS_ID_INTERNAL != screenId && info.isExtendMain)) {
        TLOGI(WmsLogTag::DMS, "find main screen option");
        info.mainScreenOption.screenId_ = screenId;
        info.mainScreenOption.startX_ = startX;
        info.mainScreenOption.startY_ = startY;
    } else {
        TLOGI(WmsLogTag::DMS, "find secondary screen option");
        info.secondaryScreenOption.screenId_ = screenId;
        info.secondaryScreenOption.startX_ = startX;
        info.secondaryScreenOption.startY_ = startY;
    }
    return true;
}

bool ScreenSettingHelper::GetScreenActiveMode(SupportedScreenModes& info, const std::string& inputString)
{
    std::vector<std::string> modeStr = {};
    bool split = SplitString(modeStr, inputString, ' ');
    uint32_t dataSize =  modeStr.size();
    if (!split || dataSize != EXPECT_ACTIVE_MODE_SIZE) {
        TLOGE(WmsLogTag::DMS, "split failed, data size: %{public}d", dataSize);
        return false;
    }
    uint32_t activeId = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t refreshRate = 0;
    if (!IsNumber(modeStr[DATA_INDEX_ZERO])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        activeId = static_cast<uint32_t>(strtoll(modeStr[DATA_INDEX_ZERO].c_str(), nullptr, PARAM_NUM_TEN));
    }
    if (!IsNumber(modeStr[DATA_INDEX_ONE])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        width = static_cast<uint32_t>(strtoll(modeStr[DATA_INDEX_ONE].c_str(), nullptr, PARAM_NUM_TEN));
    }
    if (!IsNumber(modeStr[DATA_INDEX_TWO])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        height = static_cast<uint32_t>(strtoll(modeStr[DATA_INDEX_TWO].c_str(), nullptr, PARAM_NUM_TEN));
    }
    if (!IsNumber(modeStr[DATA_INDEX_THERE])) {
        TLOGE(WmsLogTag::DMS, "not number");
        return false;
    } else {
        refreshRate = static_cast<uint32_t>(strtoll(modeStr[DATA_INDEX_THERE].c_str(), nullptr, PARAM_NUM_TEN));
    }
    info.id_ = activeId;
    info.width_ = width;
    info.height_ = height;
    info.refreshRate_ = refreshRate;
    TLOGW(WmsLogTag::DMS, "activeId: %{public}d, width: %{public}d, height: %{public}d, refreshRate: %{public}d",
        activeId, width, height, refreshRate);
    return true;
}

ScreenShape ScreenSettingHelper::GetScreenShape(ScreenId screenId)
{
    std::istringstream iss(SCREEN_SHAPE);
    std::string id;
    std::string shape;
    while (std::getline(iss, id, ':')) {
        std::getline(iss, shape, ';');
        if (!IsNumber(id)) {
            TLOGI(WmsLogTag::DMS, "id is invalid");
            continue;
        }
        if (screenId == static_cast<ScreenId>(std::stoi(id))) {
            if (!IsNumber(shape)) {
                TLOGE(WmsLogTag::DMS, "shape is invalid");
                return ScreenShape::RECTANGLE;
            }
            return static_cast<ScreenShape>(std::stoi(shape));
        }
    }
    TLOGI(WmsLogTag::DMS, "Can not find screen shape info. ccm:%{public}s", SCREEN_SHAPE.c_str());
    return ScreenShape::RECTANGLE;
}

void ScreenSettingHelper::RegisterSettingWireCastObserver(SettingObserver::UpdateFunc func)
{
    if (wireCastObserver_) {
        TLOGD(WmsLogTag::DMS, "setting wire cast observer is registered");
        return;
    }
    SettingProvider& wireCastProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    wireCastObserver_ = wireCastProvider.CreateObserver(SETTING_CAST_KEY, func);
    if (wireCastObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
        return;
    }
    ErrCode ret = wireCastProvider.RegisterObserver(wireCastObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        wireCastObserver_ = nullptr;
    }
}

void ScreenSettingHelper::UnregisterSettingWireCastObserver()
{
    if (wireCastObserver_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "wireCastObserver_ is nullptr");
        return;
    }
    SettingProvider& wireCastProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = wireCastProvider.UnregisterObserver(wireCastObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    wireCastObserver_ = nullptr;
}
// LCOV_EXCL_STOP

void ScreenSettingHelper::RegisterSettingBorderingAreaPercentObserver(SettingObserver::UpdateFunc func)
{
    if (borderingAreaPercentObserver_ != nullptr) {
        TLOGD(WmsLogTag::DMS, "setting bordering area percent observer is registered");
        return;
    }
    SettingProvider& borderingAreaPercentProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    borderingAreaPercentObserver_ =
        borderingAreaPercentProvider.CreateObserver(SETTING_SCREEN_BORDERING_AREA_PERCENT_KEY, func);
    if (borderingAreaPercentObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
        return;
    }
    ErrCode ret = borderingAreaPercentProvider.RegisterObserver(borderingAreaPercentObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        borderingAreaPercentObserver_ = nullptr;
    }
}

void ScreenSettingHelper::UnregisterSettingBorderingAreaPercentObserver()
{
    if (borderingAreaPercentObserver_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "setting observer is nullptr");
        return;
    }
    SettingProvider& borderingAreaPercentProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = borderingAreaPercentProvider.UnregisterObserver(borderingAreaPercentObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    borderingAreaPercentObserver_ = nullptr;
}

void ScreenSettingHelper::RegisterSettingExtendScreenDpiObserver(SettingObserver::UpdateFunc func)
{
    if (extendScreenDpiObserver_ != nullptr) {
        TLOGD(WmsLogTag::DMS, "setting extend dpi observer is registered");
        return;
    }
    SettingProvider& extendScreenProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    extendScreenDpiObserver_ = extendScreenProvider.CreateObserver(SETTING_EXTEND_DPI_KEY, func);
    if (extendScreenDpiObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
        return;
    }
    ErrCode ret = extendScreenProvider.RegisterObserver(extendScreenDpiObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        extendScreenDpiObserver_ = nullptr;
    }
}

void ScreenSettingHelper::UnRegisterSettingExtendScreenDpiObserver()
{
    if (extendScreenDpiObserver_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "extendScreenDpiObserver_ is nullptr");
        return;
    }
    SettingProvider& extendScreenProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = extendScreenProvider.UnregisterObserver(extendScreenDpiObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    extendScreenDpiObserver_ = nullptr;
}

bool ScreenSettingHelper::GetSettingExtendScreenDpi(float& coef, const std::string& key)
{
    SettingProvider& extendScreenProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    int32_t value = INDEX_EXTEND_SCREEN_DPI_POSITION;
    ErrCode ret = extendScreenProvider.GetIntValue(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "get setting extend dpi is %{public}d", value);
    switch (value) {
        case EXTEND_SCREEN_DPI_LEVEL_NEG_ONE:
            coef = EXTEND_SCREEN_DPI_NEG_ONE_PARAMETER;
            break;
        case EXTEND_SCREEN_DPI_LEVEL_ZERO:
            coef = EXTEND_SCREEN_DPI_ZERO_PARAMETER;
            break;
        case EXTEND_SCREEN_DPI_LEVEL_ONE:
            coef = EXTEND_SCREEN_DPI_ONE_PARAMETER;
            break;
        default:
            coef = EXTEND_SCREEN_DPI_ONE_PARAMETER;
            break;
    }
    return true;
}

bool ScreenSettingHelper::ConvertStrToInt32(const std::string& str, int32_t& num)
{
    auto res = std::from_chars(str.data(), str.data() + str.size(), num);
    if (res.ec != std::errc()) {
        return false;
    }
    return true;
}

// LCOV_EXCL_START
void ScreenSettingHelper::RegisterSettingDuringCallStateObserver(SettingObserver::UpdateFunc func)
{
    if (duringCallStateObserver_ != nullptr) {
        TLOGW(WmsLogTag::DMS, "during call state observer is registered");
        return;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    duringCallStateObserver_ = settingProvider.CreateObserver(SETTING_DURING_CALL_KEY, func);
    if (duringCallStateObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
        return;
    }
    ErrCode ret = settingProvider.RegisterObserver(duringCallStateObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        duringCallStateObserver_ = nullptr;
    }
}

void ScreenSettingHelper::UnregisterSettingDuringCallStateObserver()
{
    if (duringCallStateObserver_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "duringCallStateObserver_ is nullptr");
        return;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.UnregisterObserver(duringCallStateObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    duringCallStateObserver_ = nullptr;
}

bool ScreenSettingHelper::GetSettingDuringCallState(bool& enable, const std::string& key)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    bool value = 0;
    ErrCode ret = settingProvider.GetBoolValue(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "get during call state is %{public}d", value);
    enable = value;
    return true;
}

bool ScreenSettingHelper::SetSettingDuringCallState(const std::string& key, bool value)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.PutBoolValue(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "put during call state is %{public}d", value);
    return true;
}
// LCOV_EXCL_STOP

void ScreenSettingHelper::RegisterSettingResolutionEffectObserver(SettingObserver::UpdateFunc func)
{
    if (resolutionEffectObserver_) {
        TLOGE(WmsLogTag::DMS, "setting observer is registered");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    resolutionEffectObserver_ = provider.CreateObserver(SETTING_RESOLUTION_EFFECT_KEY, func);
    if (resolutionEffectObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
        return;
    }
    ErrCode ret = provider.RegisterObserver(resolutionEffectObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        resolutionEffectObserver_ = nullptr;
    }
}

// LCOV_EXCL_START
void ScreenSettingHelper::UnregisterSettingResolutionEffectObserver()
{
    if (resolutionEffectObserver_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "setting observer is nullptr");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = provider.UnregisterObserver(resolutionEffectObserver_);
    if (ret != ERR_OK) {
        TLOGW(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
    }
    resolutionEffectObserver_ = nullptr;
}

bool ScreenSettingHelper::GetResolutionEffect(bool& enable, const std::string& serialNumber, const std::string& key)
{
    std::string value = "";
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetStringValue(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    std::string validString = RemoveInvalidChar(value);
    std::vector<std::string> screenResolutionEffectSet = {};
    bool split = SplitString(screenResolutionEffectSet, validString, ',');
    uint32_t dataSize = screenResolutionEffectSet.size();
    if (!split || dataSize == 0) {
        TLOGE(WmsLogTag::DMS, "split failed, data size: %{public}d", dataSize);
        return false;
    }
    for (auto& infoString : screenResolutionEffectSet) {
        std::vector<std::string> infoVector = {};
        split = SplitString(infoVector, infoString, ' ');
        dataSize = infoVector.size();
        if (!split || dataSize != EXPECT_SCREEN_RESOLUTION_EFFECT_SIZE) {
            TLOGE(WmsLogTag::DMS, "split failed, screenResolutionEffect size: %{public}d", dataSize);
            return false;
        }
        if (infoVector[INDEX_SCREEN_RESOLUTION_EFFECT_SN] == serialNumber) {
            enable = (infoVector[INDEX_SCREEN_RESOLUTION_EFFECT_EN] == ENABLE_RESOLUTION_EFFECT);
            TLOGI(WmsLogTag::DMS, "screenResolutionEffectEn: %{public}d", enable);
        }
    }
    return true;
}

void ScreenSettingHelper::RegisterRotationCorrectionExemptionListObserver(SettingObserver::UpdateFunc func)
{
    if (correctionExemptionListObserver_ != nullptr) {
        TLOGI(WmsLogTag::DMS, "observer is registered");
        return;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    correctionExemptionListObserver_ = settingProvider.CreateObserver(SETTING_COMPATIBLE_APP_STRATEGY_KEY, func);
    if (correctionExemptionListObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "create observer failed");
        return;
    }
    ErrCode ret = settingProvider.RegisterObserver(correctionExemptionListObserver_);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
        correctionExemptionListObserver_ = nullptr;
    }
}
 
void ScreenSettingHelper::UnregisterRotationCorrectionExemptionListObserver()
{
    if (correctionExemptionListObserver_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "observer is nullptr");
        return;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.UnregisterObserver(correctionExemptionListObserver_);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret:%{public}d", ret);
    }
    correctionExemptionListObserver_ = nullptr;
}
 
bool ScreenSettingHelper::GetRotationCorrectionExemptionList(std::vector<std::string>& exemptionApps,
    const std::string& key)
{
    exemptionApps.clear();
    std::string value = "";
    SettingProvider& settingProvider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_SA_ID);
    ErrCode ret = settingProvider.GetStringValue(key, value);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "failed, ret=%{public}d", ret);
        return false;
    }
    GetCorrectionExemptionListFromJson(value, exemptionApps);
    return true;
}
 
void ScreenSettingHelper::GetCorrectionExemptionListFromJson(const std::string& exemptionListJsonStr,
    std::vector<std::string>& exemptionApps)
{
    nlohmann::json exemptionListJson = nlohmann::json::parse(exemptionListJsonStr, nullptr, false);
    if (exemptionListJson.is_discarded()) {
        TLOGE(WmsLogTag::DMS, "parse json failed");
        return;
    }
    for (auto it = exemptionListJson.begin(); it != exemptionListJson.end(); ++it) {
        const std::string& key = it.key();
        const nlohmann::json& value = it.value();
        std::string name = "";
        bool exemptNaturalDirectionCorrect = false;
        GetJsonValue(value, "name", name);
        if (name.empty()) {
            continue;
        }
        GetJsonValue(value, "exemptNaturalDirectionCorrect", exemptNaturalDirectionCorrect);
        if (exemptNaturalDirectionCorrect) {
            exemptionApps.emplace_back(name);
        }
    }
}
// LCOV_EXCL_STOP
} // namespace Rosen
} // namespace OHOS
