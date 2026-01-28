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

#include <gtest/gtest.h>
#include "screen_manager_lite.cpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ScreenManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static ScreenManagerLite& sml_;
};
ScreenManagerLite& ScreenManagerLiteTest::sml_ = ScreenManagerLite::GetInstance();

void ScreenManagerLiteTest::SetUpTestCase()
{
}

void ScreenManagerLiteTest::TearDownTestCase()
{
}

void ScreenManagerLiteTest::SetUp()
{
}

void ScreenManagerLiteTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetScreenInfoById
 * @tc.desc: GetScreenInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerLiteTest, GetScreenInfoById, TestSize.Level1)
{
    auto ret = sml_.GetScreenInfoById(SCREEN_ID_INVALID);
    ASSERT_EQ(ret, nullptr);
    ret = sml_.GetScreenInfoById(0);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: GetPhysicalScreenIds
 * @tc.desc: GetPhysicalScreenIds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerLiteTest, GetPhysicalScreenIds, TestSize.Level1)
{
    std::vector<ScreenId> screenIds;
    auto ret = sml_.GetPhysicalScreenIds(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenManagerLiteTest, SetResolution, TestSize.Level1)
{
    uint32_t width = 1080;
    uint32_t height = 2400;
    uint32_t dpi = 460;
    EXPECT_EQ(sml_.SetResolution(width, height, dpi), DMError::DM_OK);

    dpi = 10;
    EXPECT_EQ(sml_.SetResolution(width, height, dpi), DMError::DM_ERROR_INVALID_PARAM);
+}
}
} // namespace Rosen
} // namespace OHOS