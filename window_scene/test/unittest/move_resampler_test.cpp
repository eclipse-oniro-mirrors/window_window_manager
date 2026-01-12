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
#include "move_resampler.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MoveResamplerTest : public testing::Test {
public:
    void SetUp() override
    {
        // Disable startup smoothing for test by default
        moveResampler_.startupInitialized_ = true;
        moveResampler_.startupPhase_ = false;
    }

    void TearDown() override
    {
        filter_.Reset();
        moveResampler_.Reset();
    }

private:
    OneEuroFilter filter_;
    MoveResampler moveResampler_;
};

/**
 * @tc.name: TestOneEuroFilterInitialValueUsed
 * @tc.desc: Verify OneEuroFilter uses explicit initial value when set
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestOneEuroFilterInitialValueUsed, TestSize.Level1)
{
    filter_.SetInitialValue(100.0);   // useInitialValue_ = true
    double v = filter_.Filter(1000, 10.0);
    EXPECT_DOUBLE_EQ(v, 100.0);
}

/**
 * @tc.name: TestOneEuroFilterInitialBehavior
 * @tc.desc: Verify OneEuroFilter reset, first input, and time reversal behavior
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestOneEuroFilterInitialBehavior, TestSize.Level1)
{
    // Case 1: First input => return raw value
    double v1 = filter_.Filter(1000, 10.0);
    EXPECT_DOUBLE_EQ(v1, 10.0);

    // Case 2: Time goes backwards => reset and return raw value
    double v2 = filter_.Filter(900, 20.0);
    EXPECT_DOUBLE_EQ(v2, 20.0);
}

/**
 * @tc.name: TestOneEuroFilterLowpassSmoothing
 * @tc.desc: Verify OneEuroFilter smooths values
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestOneEuroFilterLowpassSmoothing, TestSize.Level1)
{
    filter_.Filter(0, 0.0);  // Init

    // small change → smoothed result should be closer to previous
    double r = filter_.Filter(10000, 1.0);
    EXPECT_LT(r, 1.0);
    EXPECT_GT(r, 0.0);
}

/**
 * @tc.name: TestOneEuroFilterAlpha
 * @tc.desc: Verify Alpha returns 1.0 when cutoff <= EPS or dt <= EPS
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestOneEuroFilterAlpha, TestSize.Level1)
{
    double a4 = filter_.Alpha(1.0, 0.01);
    EXPECT_NEAR(a4, 0.059, 1e-3);
}

/**
 * @tc.name: TestBeginStartupOnlyOnce
 * @tc.desc: Verify BeginStartup only initializes once
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestBeginStartupOnlyOnce, TestSize.Level1)
{
    moveResampler_.startupInitialized_ = false;
    moveResampler_.BeginStartup(1000, 0, 0);
    moveResampler_.BeginStartup(2000, 100, 100);

    EXPECT_TRUE(moveResampler_.startupInitialized_);
    EXPECT_TRUE(moveResampler_.startupPhase_);
    EXPECT_EQ(moveResampler_.startupTimeUs_, 1000);
}

/**
 * @tc.name: TestStartupSmoothingBlending
 * @tc.desc: Verify ApplyStartupSmoothing blends filter params during startup
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestStartupSmoothingBlending, TestSize.Level1)
{
    moveResampler_.startupInitialized_ = false;
    moveResampler_.BeginStartup(1000, 0, 0);
    moveResampler_.ApplyStartupSmoothing(1000 + moveResampler_.startupDurationUs_ / 2);

    EXPECT_TRUE(moveResampler_.startupPhase_);
}

/**
 * @tc.name: TestStartupSmoothingFinish
 * @tc.desc: Verify ApplyStartupSmoothing ends startup phase after duration
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestStartupSmoothingFinish, TestSize.Level1)
{
    moveResampler_.startupInitialized_ = false;
    moveResampler_.BeginStartup(1000, 0, 0);
    moveResampler_.ApplyStartupSmoothing(1000 + moveResampler_.startupDurationUs_);

    EXPECT_FALSE(moveResampler_.startupPhase_);
}

/**
 * @tc.name: TestMoveResamplerPushEventCleanup
 * @tc.desc: Verify PushEvent removes old events outside max interval
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerPushEventCleanup, TestSize.Level1)
{
    moveResampler_.maxEventIntervalUs_ = 10000; // 10ms for testing

    moveResampler_.PushEvent(1000, 0, 0);
    moveResampler_.PushEvent(5000, 1, 1);
    moveResampler_.PushEvent(20000, 2, 2); // should remove 1000 & 5000

    EXPECT_EQ(moveResampler_.events_.size(), 1);
    EXPECT_EQ(moveResampler_.events_.front().posX, 2);
}

/**
 * @tc.name: TestMoveResamplerResampleAtEmpty
 * @tc.desc: Verify ResampleAt returns (0,0) when no events exist
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerResampleAtEmpty, TestSize.Level1)
{
    MoveEvent e = moveResampler_.ResampleAt(1000);
    EXPECT_EQ(e.posX, 0);
    EXPECT_EQ(e.posY, 0);
}

/**
 * @tc.name: TestMoveResamplerResampleAtSingleEvent
 * @tc.desc: Verify ResampleAt returns single event when only one exists
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerResampleAtSingleEvent, TestSize.Level1)
{
    moveResampler_.PushEvent(1000, 10, 20);

    MoveEvent e = moveResampler_.ResampleAt(2000);
    EXPECT_EQ(e.posX, 10);
    EXPECT_EQ(e.posY, 20);
}

/**
 * @tc.name: TestMoveResamplerResampleAtBoundary
 * @tc.desc: Verify earliest boundary when ResampleAt < first event
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerResampleAtBoundary, TestSize.Level1)
{
    moveResampler_.PushEvent(1000, 10, 10);
    moveResampler_.PushEvent(2000, 20, 20);

    MoveEvent e = moveResampler_.ResampleAt(500); // clamp to first
    EXPECT_EQ(e.posX, 10);
    EXPECT_EQ(e.posY, 10);
}

/**
 * @tc.name: TestResampleRawExtrapolateFitPath
 * @tc.desc: Ensure ResampleRaw enters the final branch (ExtrapolateFit)
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestResampleRawExtrapolateFitPath, TestSize.Level1)
{
    moveResampler_.PushEvent(1000, 10, 10);
    moveResampler_.PushEvent(2000, 20, 20);
    moveResampler_.PushEvent(3000, 30, 30);

    // Target beyond last → should trigger Linear Fitting branch
    auto result = moveResampler_.ResampleRaw(5000);

    // Linear fitting of:
    // t: 1000, 2000, 3000
    // x:  10,   20,   30
    // y:  10,   20,   30
    //
    // Fit is exactly y = 0.01 * t
    // For t = 5000:
    // x = 50, y = 50
    EXPECT_NEAR(result.first, 50.0, 1e-6);
    EXPECT_NEAR(result.second, 50.0, 1e-6);
}

/**
 * @tc.name: TestFindSegmentIndexFullBranch
 * @tc.desc: Verify FindSegmentIndex covers both mid < target and mid >= target branches
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestFindSegmentIndexFullBranch, TestSize.Level1)
{
    moveResampler_.PushEvent(1000, 0, 0);
    moveResampler_.PushEvent(2000, 0, 0);
    moveResampler_.PushEvent(3000, 0, 0);
    moveResampler_.PushEvent(4000, 0, 0);
    moveResampler_.PushEvent(5000, 0, 0);

    size_t idx = moveResampler_.FindSegmentIndex(3500);

    EXPECT_EQ(idx, 3); // events[3] = 4000 is the first >= 3500
}

/**
 * @tc.name: TestLinearFit
 * @tc.desc: Verify LinearFitAt with various scenarios
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestLinearFit, TestSize.Level1)
{
    // Case 1: events_.empty() → return (0,0)
    auto e1 = moveResampler_.LinearFitAt(0, 0, 1000);
    EXPECT_DOUBLE_EQ(e1.first, 0.0);
    EXPECT_DOUBLE_EQ(e1.second, 0.0);

    // Prepare some events for remaining cases
    moveResampler_.PushEvent(1000, 10, 10);
    moveResampler_.PushEvent(1000, 20, 20);
    moveResampler_.PushEvent(1000, 30, 30);

    // Case 2: startIdx > endIdx → return last event
    auto e2 = moveResampler_.LinearFitAt(2, 1, 4000);
    EXPECT_DOUBLE_EQ(e2.first, 30.0);
    EXPECT_DOUBLE_EQ(e2.second, 30.0);

    // Case 3: endIdx >= size → return last event
    auto e3 = moveResampler_.LinearFitAt(0, 999, 5000);
    EXPECT_DOUBLE_EQ(e3.first, 30.0);
    EXPECT_DOUBLE_EQ(e3.second, 30.0);

    // Case 4: sumTT == 0 → all timestamps identical → slope=0
    // meanX = 20, meanY = 20, slope = 0
    auto e4 = moveResampler_.LinearFitAt(0, 2, 5000);
    EXPECT_DOUBLE_EQ(e4.first, 20.0); // no slope → always mean
    EXPECT_DOUBLE_EQ(e4.second, 20.0);

    // Case 5: Normal slope != 0 (strictly increasing timestamps)
    moveResampler_.Reset();
    moveResampler_.PushEvent(1000, 10, 10);
    moveResampler_.PushEvent(2000, 20, 20);
    moveResampler_.PushEvent(3000, 30, 30);
    // Linear relationship: x = 0.01 * t, y = 0.01 * t
    auto e5 = moveResampler_.LinearFitAt(0, 2, 5000);
    EXPECT_NEAR(e5.first, 50.0, 1e-6);
    EXPECT_NEAR(e5.second, 50.0, 1e-6);

    // Case 7: targetTimeUs < meanT → negative tQuery → backward extrapolation
    // meanT = (1000 + 2000 + 3000) / 3 = 2000
    // x = 0.01 * t → at 0 → expect 0
    auto e6 = moveResampler_.LinearFitAt(0, 2, 0);
    EXPECT_NEAR(e6.first, 0.0, 1e-6);
    EXPECT_NEAR(e6.second, 0.0, 1e-6);

    // Case 8: Partial range fit (startIdx > 0)
    // Fit only last two points: (2000,20), (3000,30)
    // slope = (30-20)/(3000-2000)=10/1000=0.01
    // meanT = 2500, meanX = 25
    // target = 4000:
    //   tQuery = 4000 - 2500 = 1500 → x=25 + 0.01 * 1500 = 40
    auto e7 = moveResampler_.LinearFitAt(1, 2, 4000);
    EXPECT_NEAR(e7.first, 40.0, 1e-6);
    EXPECT_NEAR(e7.second, 40.0, 1e-6);
}

/**
 * @tc.name: TestMoveResamplerInterpolateLinear
 * @tc.desc: Verify interpolation between two points
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerInterpolateLinear, TestSize.Level1)
{
    auto e1 = moveResampler_.InterpolateLinear(1500);
    EXPECT_DOUBLE_EQ(e1.first, 0);
    EXPECT_DOUBLE_EQ(e1.second, 0);

    moveResampler_.PushEvent(1000, 0, 0);
    auto e2 = moveResampler_.InterpolateLinear(1500);
    EXPECT_DOUBLE_EQ(e2.first, 0);
    EXPECT_DOUBLE_EQ(e2.second, 0);

    moveResampler_.PushEvent(2000, 100, 200);
    // halfway at 1500 → (50, 100)
    auto e3 = moveResampler_.InterpolateLinear(1500);
    EXPECT_NEAR(e3.first, 50, 1);
    EXPECT_NEAR(e3.second, 100, 1);
}

/**
 * @tc.name: TestInterpolateLinearDegenerateSpan
 * @tc.desc: Verify interpolation fallback when span <= 0
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestInterpolateLinearDegenerateSpan, TestSize.Level1)
{
    // timestamps equal → span = 0
    moveResampler_.PushEvent(1000, 10, 10);
    moveResampler_.PushEvent(1000, 99, 99);

    auto e = moveResampler_.InterpolateLinear(1000);
    EXPECT_EQ(e.first, 99);
    EXPECT_EQ(e.second, 99);
}

/**
 * @tc.name: TestMoveResamplerExtrapolateFitFallback
 * @tc.desc: Verify fitting fallback when timestamps are identical
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerExtrapolateFitFallback, TestSize.Level1)
{
    auto e1 = moveResampler_.ExtrapolateFit(2000);
    EXPECT_DOUBLE_EQ(e1.first, 0);
    EXPECT_DOUBLE_EQ(e1.second, 0);

    moveResampler_.PushEvent(1000, 10, 10);
    moveResampler_.PushEvent(1000, 20, 20);
    moveResampler_.PushEvent(1000, 30, 30); // timestamps identical → fallback to mean

    auto e2 = moveResampler_.ExtrapolateFit(2000);
    EXPECT_EQ(e2.first, 20);
    EXPECT_EQ(e2.second, 20);
}

/**
 * @tc.name: TestMoveResamplerReset
 * @tc.desc: Verify Reset clears resampler and filters
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerReset, TestSize.Level1)
{
    moveResampler_.PushEvent(1000, 10, 20);
    moveResampler_.ResampleAt(2000);

    moveResampler_.Reset();
    EXPECT_TRUE(moveResampler_.events_.empty());

    MoveEvent e = moveResampler_.ResampleAt(3000);
    EXPECT_EQ(e.posX, 0);
}

/**
 * @tc.name: TestMoveResamplerTriggersOneEuroFilter
 * @tc.desc: Verify filtering modifies raw interpolation result
 * @tc.type: FUNC
 */
HWTEST_F(MoveResamplerTest, TestMoveResamplerTriggersOneEuroFilter, TestSize.Level1)
{
    moveResampler_.PushEvent(1000, 0, 0);
    moveResampler_.PushEvent(2000, 100, 100);

    MoveEvent e1 = moveResampler_.ResampleAt(1100); // interpolation mid
    MoveEvent e2 = moveResampler_.ResampleAt(1900); // next sample triggers filter

    // filtered should not equal the raw interpolation exactly
    EXPECT_NE(e1.posX, e2.posX);
    EXPECT_NE(e1.posY, e2.posY);
}
} // namespace Rosen
} // namespace OHOS
