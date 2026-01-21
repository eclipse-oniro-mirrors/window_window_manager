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
#include "uea_list_config.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <optional>
#include "config_policy_utils.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
constexpr int32_t MAX_FILE_SIZE = 2 * 1024 * 1024;
const std::string DEFAULT_UEA_CONFIG_FILE_PATH = "etc/window/uea_whitelist.json";
const std::string BUNDLE_NAME = "bundleName";
const std::string ABILITY_NAME = "abilityName";
const std::string MODULE_NAME = "moduleName";

bool IsPathValid(const std::filesystem::path& path)
{
    std::error_code ec;
    bool exists = std::filesystem::exists(path, ec);
    // check file permission
    if (ec) {
        TLOGE(WmsLogTag::WMS_UIEXT, "No permission to read the UEA configuration file.");
        return false;
    }
    if (!exists) {
        TLOGE(WmsLogTag::WMS_UIEXT, "The UEA configuration file does not exists.");
        return false;
    }
    return true;
}

bool IsPathValid(const char* path)
{
    return IsPathValid(std::filesystem::path(path));
}

bool IsPathValid(const std::string& path)
{
    return IsPathValid(std::filesystem::path(path));
}

std::optional<std::string> UeaListConfig::GenUeaConfigPath()
{
    char buf[MAX_PATH_LEN + 1] = {0};
    char realPathBuf[PATH_MAX] = {0};
    char* configPath = GetOneCfgFile(DEFAULT_UEA_CONFIG_FILE_PATH.c_str(), buf, MAX_PATH_LEN);
    if (strlen(configPath) == 0 || strlen(configPath) >= MAX_PATH_LEN ||
        realpath(configPath, realPathBuf) == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get UEA configuration file path");
        return std::nullopt;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Read the UEA configuration file path succeeded.");
    return std::string(realPathBuf);
}

std::optional<std::string> UeaListConfig::ReadUeaConfigFile(const std::string& path)
{
    if (!IsPathValid(path)) {
        return std::nullopt;
    }
    std::error_code ec;
    // check file size
    auto file_size = std::filesystem::file_size(path, ec);
    if (ec) {
        TLOGE(WmsLogTag::WMS_UIEXT, "No permission to read the UEA configuration file size.");
        return std::nullopt;
    }
    if (file_size > MAX_FILE_SIZE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UEA configuration file is too large.");
        return std::nullopt;
    }
    // read the file content into a string
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to open the UEA configuration file.");
        return std::nullopt;
    }
    std::string content(file_size, '\0');
    if (!file.read(content.data(), file_size)) {
        // check if file read completely
        if (file.gcount() != file_size) {
            TLOGE(WmsLogTag::WMS_UIEXT, "Imcomplete read: expected %{public}d, got %{public}d.",
                static_cast<int>(file_size), static_cast<int>(file.gcount()));
            return std::nullopt;
        }
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Read the UEA configuration file succeeded.");
    return content;
}

std::optional<nlohmann::json> UeaListConfig::ParseUeaConfig(const std::string& jsonStr)
{
    nlohmann::json jsonArr = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonArr.is_discarded() || !jsonArr.is_array()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to parse the UEA configuration file. error: %{public}s",
            jsonArr.dump().c_str());
        return std::nullopt;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Parse the UEA configuration file succeeded. with %{public}d elements",
        static_cast<int>(jsonArr.size()));
    return jsonArr;
}

std::vector<std::tuple<std::string, std::string, std::string>> UeaListConfig::GetUeaConfigFromCcm()
{
    std::vector<std::tuple<std::string, std::string, std::string>> configurations;
    auto path = GenUeaConfigPath();
    if (!path.has_value()) {
        return configurations;
    }
    auto jsonStr = ReadUeaConfigFile(path.value());
    if (!jsonStr.has_value()) {
        return configurations;
    }
    auto jsonArr = ParseUeaConfig(jsonStr.value());
    if (!jsonArr.has_value()) {
        return configurations;
    }
    for (const auto& item : jsonArr.value()) {
        // check field necessary
        if (!item.contains(BUNDLE_NAME) || !item.contains(ABILITY_NAME) || !item.contains(MODULE_NAME)) {
            TLOGW(WmsLogTag::WMS_UIEXT, "Incomplete UEA configuration item.");
            continue;
        }
        // check file necessary type
        if (!item[BUNDLE_NAME].is_string() || !item[ABILITY_NAME].is_string() || !item[MODULE_NAME].is_string()) {
            TLOGW(WmsLogTag::WMS_UIEXT, "Inconsistent types UEA configuration item.");
            continue;
        }
        configurations.push_back({item[BUNDLE_NAME].get<std::string>(),
            item[ABILITY_NAME].get<std::string>(),
            item[MODULE_NAME].get<std::string>()});
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Load the UEA configuration file succeeded. with %{public}d elements",
        static_cast<int>(configurations.size()));
    return configurations;
}

}