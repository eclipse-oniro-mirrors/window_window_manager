/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "string_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class StringUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void StringUtilTest::SetUpTestCase() {}

void StringUtilTest::TearDownTestCase() {}

void StringUtilTest::SetUp() {}

void StringUtilTest::TearDown() {}

namespace {
/**
 * @tc.name: StringUtil Trim
 * @tc.desc: test trim
 * @tc.type: FUNC
 * @tc.require: issueI5M2SK
 */
HWTEST_F(StringUtilTest, Trim, TestSize.Level1)
{
    ASSERT_EQ("", StringUtil::Trim(""));
    ASSERT_EQ("", StringUtil::Trim(" "));
    ASSERT_EQ("", StringUtil::Trim("  "));
    ASSERT_EQ("", StringUtil::Trim("     "));

    ASSERT_EQ("123", StringUtil::Trim("  123"));
    ASSERT_EQ("abc", StringUtil::Trim("   abc"));
    ASSERT_EQ("A", StringUtil::Trim(" A"));

    ASSERT_EQ("123", StringUtil::Trim("123  "));
    ASSERT_EQ("abc", StringUtil::Trim("abc   "));
    ASSERT_EQ("A", StringUtil::Trim("A "));

    ASSERT_EQ("123", StringUtil::Trim(" 123 "));
    ASSERT_EQ("12 3", StringUtil::Trim(" 12 3 "));

    ASSERT_EQ("123", StringUtil::Trim("123"));
    ASSERT_EQ("hello", StringUtil::Trim("hello"));
    ASSERT_EQ("x", StringUtil::Trim("x"));

    ASSERT_EQ("12 34", StringUtil::Trim("12 34"));
    ASSERT_EQ("a b c", StringUtil::Trim("a b c"));

    ASSERT_EQ("Hello World 123", StringUtil::Trim("   Hello World 123   "));
}

/**
 * @tc.name: ConvertStringToFloat
 * @tc.desc: test ConvertStringToFloat
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilTest, ConvertStringToFloat, TestSize.Level1)
{
    float value = 0.0f;

    // valid inputs
    ASSERT_TRUE(StringUtil::ConvertStringToFloat("10.5", value));
    ASSERT_FLOAT_EQ(value, 10.5f);

    ASSERT_TRUE(StringUtil::ConvertStringToFloat("0", value));
    ASSERT_FLOAT_EQ(value, 0.0f);

    ASSERT_TRUE(StringUtil::ConvertStringToFloat("-3.14", value));
    ASSERT_FLOAT_EQ(value, -3.14f);

    ASSERT_TRUE(StringUtil::ConvertStringToFloat("  12.4", value));
    ASSERT_FLOAT_EQ(value, 12.4f);

    ASSERT_TRUE(StringUtil::ConvertStringToFloat("1.5abc", value));
    ASSERT_FLOAT_EQ(value, 1.5f);

    // invalid inputs
    ASSERT_FALSE(StringUtil::ConvertStringToFloat("abc", value));
    ASSERT_FALSE(StringUtil::ConvertStringToFloat("", value));
    ASSERT_FALSE(StringUtil::ConvertStringToFloat("inf", value));
    ASSERT_FALSE(StringUtil::ConvertStringToFloat("nan", value));
}

/**
 * @tc.name: JoinValueSet
 * @tc.desc: Test JoinValueSet
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilTest, JoinValueSet, TestSize.Level1)
{
    ASSERT_EQ("", StringUtil::JoinValueSet<int32_t>({}));
    ASSERT_EQ("-1 2 3", StringUtil::JoinValueSet<int32_t>({3, -1, 2}));
    ASSERT_EQ("1,2,3", StringUtil::JoinValueSet<int32_t>({3, 1, 2}, ','));
}

/**
 * @tc.name: ParseValueSet
 * @tc.desc: Test ParseValueSet
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilTest, ParseValueSet, TestSize.Level1)
{
    auto emptyValues = StringUtil::ParseValueSet<int32_t>("");
    ASSERT_TRUE(emptyValues.empty());

    auto values = StringUtil::ParseValueSet<int32_t>("3 -1 2 3");
    ASSERT_EQ(values, (std::set<int32_t>{-1, 2, 3}));

    values = StringUtil::ParseValueSet<int32_t>("  4 5  ");
    ASSERT_EQ(values, (std::set<int32_t>{4, 5}));

    values = StringUtil::ParseValueSet<int32_t>("1,2,3", ',');
    ASSERT_EQ(values, (std::set<int32_t>{1, 2, 3}));

    // Invalid values should be ignored.
    values = StringUtil::ParseValueSet<int32_t>("1 abc 2");
    ASSERT_EQ(values, (std::set<int32_t>{1, 2}));

    values = StringUtil::ParseValueSet<int32_t>("1,abc,2", ',');
    ASSERT_EQ(values, (std::set<int32_t>{1, 2}));

    // Empty tokens should be ignored.
    values = StringUtil::ParseValueSet<int32_t>("1,,2,,,3", ',');
    ASSERT_EQ(values, (std::set<int32_t>{1, 2, 3}));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
