/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <vector>
#include "gtest/gtest.h"
#include "screen_edid_parse.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
constexpr int32_t CHECK_CODE_INVALID = -1;
void* g_libHandle = nullptr;

class ScreenEdidParseTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};
void ScreenEdidParseTest::SetUp()
{
    g_libHandle = nullptr;
}
void ScreenEdidParseTest::TearDown()
{
    g_libHandle = nullptr;
}

/**
 * @tc.name: LoadEdidPlugin_AlreadyLoaded
 * @tc.desc: test function : LoadEdidPlugin_AlreadyLoaded
 * @tc.type: FUNC
 */
HWTEST_F(ScreenEdidParseTest, LoadEdidPlugin_AlreadyLoaded, TestSize.Level1)
{
    g_libHandle = reinterpret_cast<void*>(0x12345678);
    bool result = LoadEdidPlugin();
    EXPECT_TRUE(result);
    EXPECT_EQ(g_libHandle, reinterpret_cast<void*>(0x12345678));
}

/**
 * @tc.name: GetEdidCheckCode_DataTooSmall
 * @tc.desc: test function : GetEdidCheckCode_DataTooSmall
 * @tc.type: FUNC
 */
HWTEST_F(ScreenEdidParseTest, GetEdidCheckCode_DataTooSmall, TestSize.Level1)
{
    std::vector<uint8_t> edidData = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    int32_t checkCode = GetEdidCheckCode(edidData);
    EXPECT_EQ(checkCode, CHECK_CODE_INVALID);
}

/**
 * @tc.name: GetEdidCheckCode_DataValid
 * @tc.desc: test function : GetEdidCheckCode_DataValid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenEdidParseTest, GetEdidCheckCode_DataValid, TestSize.Level1)
{
    std::vector<uint8_t> edidData = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11
    };
    int32_t checkCode = GetEdidCheckCode(edidData);
    EXPECT_EQ(checkCode, 0x08 + 0x09 + 0x0A + 0x0B + 0x0C + 0x0D + 0x0E + 0x0F + 0x10 + 0x11);
}

/**
 * @tc.name: GetEdid_RSFailed
 * @tc.desc: test function : GetEdid_RSFailed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenEdidParseTest, GetEdid_RSFailed, TestSize.Level1)
{
    struct BaseEdid edid;
    bool result = GetEdid(-1, edid);
    EXPECT_FALSE(result);
}
}
}