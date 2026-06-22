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

#include <gtest/gtest.h>
#include "mock_uea_list_config.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

namespace {

class UeaListConfigTest : public testing::Test {
protected:
    void SetUp() override
    {
        mockConfig_ = std::make_unique<MockUeaListConfig>();
    }
    void TearDown() override
    {
        mockConfig_.reset();
    }
    std::unique_ptr<MockUeaListConfig> mockConfig_;
};

HWTEST_F(UeaListConfigTest, GetUeaConfigFromCcmSuccess, TestSize.Level1) {
    EXPECT_CALL(*mockConfig_, GenUeaConfigPath()).WillOnce(Return(std::string(TEST_PATH)));
    EXPECT_CALL(*mockConfig_, ReadUeaConfigFile(_)).WillOnce(Return(std::string(TEST_JSON_STR)));
    nlohmann::json jsonArr = nlohmann::json::parse(TEST_JSON_STR);
    EXPECT_CALL(*mockConfig_, ParseUeaConfig(_)).WillOnce(Return(std::string(jsonArr)));
    
    auto result = mockConfig_->GetUeaConfigFromCcm();

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(std::get<0>(result[0]), "com.example.app1");
    EXPECT_EQ(std::get<1>(result[0]), "MainAbility");
    EXPECT_EQ(std::get<2>(result[0]), "entry");
    EXPECT_EQ(std::get<0>(result[1]), "com.example.app2");
}

HWTEST_F(UeaListConfigTest, GetUeaConfigFromCcmPathFailed, TestSize.Level1) {
    EXPECT_CALL(*mockConfig_, GenUeaConfigPath()).WillOnce(Return(std::nullopt));
    EXPECT_CALL(*mockConfig_, ReadUeaConfigFile(_)).Times(0);
    EXPECT_CALL(*mockConfig_, ParseUeaConfig(_)).Times(0);
    
    auto result = mockConfig_->GetUeaConfigFromCcm();
    ASSERT_TRUE(result.empty());
}

HWTEST_F(UeaListConfigTest, GetUeaConfigFromCcmReadFailed, TestSize.Level1) {
    EXPECT_CALL(*mockConfig_, GenUeaConfigPath()).WillOnce(Return(std::string(TEST_PATH)));
    EXPECT_CALL(*mockConfig_, ReadUeaConfigFile(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(*mockConfig_, ParseUeaConfig(_)).Times(0);
    
    auto result = mockConfig_->GetUeaConfigFromCcm();
    ASSERT_TRUE(result.empty());
}

HWTEST_F(UeaListConfigTest, GetUeaConfigFromCcmParseFailed, TestSize.Level1) {
    EXPECT_CALL(*mockConfig_, GenUeaConfigPath()).WillOnce(Return(std::string(TEST_PATH)));
    EXPECT_CALL(*mockConfig_, ReadUeaConfigFile(_)).WillOnce(Return(std::string(TEST_JSON_STR)));
    EXPECT_CALL(*mockConfig_, ParseUeaConfig(_)).WillOnce(Return(std::nullopt));
    
    auto result = mockConfig_->GetUeaConfigFromCcm();
    ASSERT_TRUE(result.empty());
}

HWTEST_F(UeaListConfigTest, GetUeaConfigFromCcmInvalidItem, TestSize.Level1) {
    EXPECT_CALL(*mockConfig_, GenUeaConfigPath()).WillOnce(Return(std::string(TEST_PATH)));
    EXPECT_CALL(*mockConfig_, ReadUeaConfigFile(_)).WillOnce(Return(std::string(TEST_INVALID_JSON_STR)));
    nlohmann::json jsonArr = nlohmann::json::parse(TEST_INVALID_JSON_STR);
    EXPECT_CALL(*mockConfig_, ParseUeaConfig(_)).WillOnce(Return(std::string(jsonArr)));
    
    auto result = mockConfig_->GetUeaConfigFromCcm();
    ASSERT_TRUE(result.empty());
}

} // namespace
} // namespace Rosen
} // namespace OHOS