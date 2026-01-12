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
#include "dm_common.h"
#include "ws_common.h"
#include "common/include/session_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionHelperTest : public testing::Test {
public:
    SessionHelperTest() {}
    ~SessionHelperTest() {}
};
namespace {
/**
 * @tc.name: ConvertDisplayOrientationToFloat
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionHelperTest, ConvertDisplayOrientationToFloat, TestSize.Level1)
{
    DisplayOrientation orientation = DisplayOrientation::LANDSCAPE;
    EXPECT_EQ(SessionHelper::ConvertDisplayOrientationToFloat(orientation), 90.f); // degree 90
    orientation = DisplayOrientation::PORTRAIT_INVERTED;
    EXPECT_EQ(SessionHelper::ConvertDisplayOrientationToFloat(orientation), 180.f); // degree 180
    orientation = DisplayOrientation::LANDSCAPE_INVERTED;
    EXPECT_EQ(SessionHelper::ConvertDisplayOrientationToFloat(orientation), 270.f); // degree 270
    orientation = DisplayOrientation::UNKNOWN;
    EXPECT_EQ(SessionHelper::ConvertDisplayOrientationToFloat(orientation), 0.f); // degree 0
}

/**
 * @tc.name: GetAreaTypeForScaleResize
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionHelperTest, GetAreaTypeForScaleResize, TestSize.Level1)
{
    int32_t pointWinX = 1249;
    int32_t pointWinY = 2717;
    int outside = 7;
    const WSRect& rect = {579, -418, 1260, 2720};
    // RIGHT_BOTTOM
    EXPECT_EQ(SessionHelper::GetAreaTypeForScaleResize(pointWinX, pointWinY, outside, rect), AreaType::RIGHT_BOTTOM);

    pointWinX = 6;
    pointWinY = 2707;
    outside = 7;
    const WSRect& rect2 = {614, -342, 1260, 2720};
    // LEFT_BOTTOM
    EXPECT_EQ(SessionHelper::GetAreaTypeForScaleResize(pointWinX, pointWinY, outside, rect2), AreaType::LEFT_BOTTOM);

    pointWinX = 1255;
    pointWinY = 15;
    outside = 7;
    const WSRect& rect3 = {659, -440, 1260, 2720};
    // RIGHT_TOP
    EXPECT_EQ(SessionHelper::GetAreaTypeForScaleResize(pointWinX, pointWinY, outside, rect3), AreaType::RIGHT_TOP);

    pointWinX = -3;
    pointWinY = 10;
    outside = 7;
    const WSRect& rect4 = {697, -522, 1260, 2720};
    // LEFT_TOP
    EXPECT_EQ(SessionHelper::GetAreaTypeForScaleResize(pointWinX, pointWinY, outside, rect4), AreaType::LEFT_TOP);
}

/**
 * @tc.name: ShiftDecimalDigit
 * @tc.desc: Verify ShiftDecimalDigit
 * @tc.type: FUNC
 */
HWTEST_F(SessionHelperTest, ShiftDecimalDigit, TestSize.Level1)
{
    EXPECT_EQ(SessionHelper::ShiftDecimalDigit(4, 2), 400);
    EXPECT_EQ(SessionHelper::ShiftDecimalDigit(4, -1), 0);
    EXPECT_EQ(SessionHelper::ShiftDecimalDigit(-1, 2), 0);
    EXPECT_EQ(SessionHelper::ShiftDecimalDigit(11, 2), 0);
    EXPECT_EQ(SessionHelper::ShiftDecimalDigit(1, 100), 0);
}
} // namespace
} // namespace Rosen
} // namespace OHOS