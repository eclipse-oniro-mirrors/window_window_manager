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

#ifndef OHOS_ROSEN_MOCK_UEA_LIST_CONFIG_H
#define OHOS_ROSEN_MOCK_UEA_LIST_CONFIG_H

#include <gmock/gmock.h>
#include "uea_list_config.h"

namespace OHOS::Rosen {
const std::string TEST_JSON_STR = R"([
    {"bundleName":"com.example.app1", "abilityName":"MainAbility", "moduleName":"entry"},
    {"bundleName":"com.example.app2", "abilityName":"MainAbility", "moduleName":"entry"}
])";
const std::string TEST_INVALID_JSON_STR = R"([
    {"bundleName":"com.example.app1", "abilityName":"MainAbility"}
])";
const std::string TEST_PATH = "/test/path/uea_whitelist.json";

class MockUeaListConfig : public UeaListConfig {
public:
    MOCK_METHOD(std::optional<std::string>, GenUeaConfigPath, ());
    MOCK_METHOD(std::optional<std::string>, ReadUeaConfigFile, (const std::string& path));
    MOCK_METHOD(std::optional<nlohmann::json>, ParseUeaConfig, (const std::string& jsonStr));
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_MOCK_UEA_LIST_CONFIG_H
