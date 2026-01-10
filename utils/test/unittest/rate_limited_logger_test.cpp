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
#include "rate_limited_logger.h"

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class RateLimitedLoggerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp() override
    {
        // Clear log records before each test
        RateLimitedLogger::getInstance().clear();
        RateLimitedLogger::getInstance().setEnabled(true);
    }

    void TearDown() override
    {
        // Clear log records after each test
        RateLimitedLogger::getInstance().clear();
    }
};

void RateLimitedLoggerTest::SetUpTestCase() {}

void RateLimitedLoggerTest::TearDownTestCase() {}

namespace {
/**
 * @tc.name: ShouldHandleLargeTimeWindow
 * @tc.desc: Should handle very large time windows correctly
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleLargeTimeWindow, TestSize.Level1)
{
    // Given - Very large time window (1 hour)
    std::string functionName = "testFunction";
    int32_t timeWindowMs = 60 * 60 * 1000; // 1 hour in milliseconds
    int32_t maxCount = 3;
    
    // When - Log multiple times within the large window
    bool result1 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result2 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result3 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result4 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - First three should succeed, fourth should fail
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(result3);
    EXPECT_FALSE(result4);
}

/**
 * @tc.name: ShouldHandleLargeTimeWindow
 * @tc.desc: Should handle very small time windows correctly
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleSmallTimeWindow, TestSize.Level1)
{
    // Given - Very small time window (1ms)
    std::string functionName = "testFunction";
    int32_t timeWindowMs = 1; // 1 millisecond
    int32_t maxCount = 1;
    
    // When - Log once
    bool result1 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - First call should succeed
    EXPECT_TRUE(result1);
    
    // When - Try to log again immediately (should fail)
    bool result2 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - Second call should fail
    EXPECT_FALSE(result2);
    
    // When - Wait for time window to expire and try again
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    bool result3 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - Should succeed after waiting
    EXPECT_TRUE(result3);
}

/**
 * @tc.name: ShouldHandleLargeMaxCount
 * @tc.desc: Should handle maximum integer values for maxCount
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleLargeMaxCount, TestSize.Level1)
{
    // Given - Very large maxCount
    std::string functionName = "testFunction";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = INT_MAX; // Maximum integer value
    
    // When - Log once (should always succeed with such a high limit)
    bool result = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - Should succeed
    EXPECT_TRUE(result);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName), 1);
}

/**
 * @tc.name: ShouldHandleSpecialCharactersInFunctionName
 * @tc.desc: Should handle function names with special characters
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleSpecialCharactersInFunctionName, TestSize.Level1)
{
    // Given - Function names with special characters
    std::string functionName1 = "function::with::namespace";
    std::string functionName2 = "function_with_underscores";
    std::string functionName3 = "FunctionWithCamelCase";
    std::string functionName4 = "function-with-dashes";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = 1;
    
    // When - Log with each special function name
    bool result1 = RateLimitedLogger::getInstance().logFunction(functionName1, timeWindowMs, maxCount);
    bool result2 = RateLimitedLogger::getInstance().logFunction(functionName2, timeWindowMs, maxCount);
    bool result3 = RateLimitedLogger::getInstance().logFunction(functionName3, timeWindowMs, maxCount);
    bool result4 = RateLimitedLogger::getInstance().logFunction(functionName4, timeWindowMs, maxCount);
    
    // Then - All should succeed and maintain separate counts
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(result3);
    EXPECT_TRUE(result4);
    
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName1), 1);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName2), 1);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName3), 1);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName4), 1);
}

/**
 * @tc.name: ShouldHandleEmptyFunctionName
 * @tc.desc: Should handle empty function name
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleEmptyFunctionName, TestSize.Level1)
{
    // Given - Empty function name
    std::string functionName = "";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = 2;
    
    // When - Log multiple times with empty function name
    bool result1 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result2 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result3 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - First two should succeed, third should fail
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_FALSE(result3);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName), 2);
}

/**
 * @tc.name: SameFunctionWithDifferentParametersShouldHaveSeparateCounts
 * @tc.desc: Should maintain separate counts for same function with different parameters
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, SameFunctionWithDifferentParametersShouldHaveSeparateCounts, TestSize.Level1)
{
    // Given - Same function name but different time windows and max counts
    std::string functionName = "testFunction";
    
    // When - Log with different parameters
    bool result1 = RateLimitedLogger::getInstance().logFunction(functionName, 1000, 2);
    bool result2 = RateLimitedLogger::getInstance().logFunction(functionName, 2000, 3);
    bool result3 = RateLimitedLogger::getInstance().logFunction(functionName, 1000, 2);
    
    // Then - Should use the most recent parameters for rate limiting
    // This test documents the current behavior - the function doesn't distinguish
    // between calls with different parameters for the same function name
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2); // Uses new parameters (2000ms, 3 max)
    EXPECT_FALSE(result3); // Still limited by original parameters (1000ms, 2 max)
}

/**
 * @tc.name: ShouldHandleRapidConsecutiveCalls
 * @tc.desc: Should handle rapid consecutive calls correctly
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleRapidConsecutiveCalls, TestSize.Level1)
{
    // Given - Short time window and low max count
    std::string functionName = "testFunction";
    int32_t timeWindowMs = 100; // 100ms window
    int32_t maxCount = 3;
    
    // When - Make rapid consecutive calls
    bool results[5];
    for (int32_t i = 0; i < 5; ++i) {
        results[i] = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    }
    
    // Then - First three should succeed, last two should fail
    EXPECT_TRUE(results[0]);
    EXPECT_TRUE(results[1]);
    EXPECT_TRUE(results[2]);
    EXPECT_FALSE(results[3]);
    EXPECT_FALSE(results[4]);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName), 3);
}

/**
 * @tc.name: ShouldResetAfterClear
 * @tc.desc: Should reset correctly after clear() is called
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldResetAfterClear, TestSize.Level1)
{
    // Given - Function that has reached its limit
    std::string functionName = "testFunction";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = 2;
    
    // When - Reach the limit
    bool result1 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result2 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    bool result3 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    
    // Then - Third call should fail
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_FALSE(result3);
    
    // When - Clear all records
    RateLimitedLogger::getInstance().clear();
    
    // Then - Should be able to log again
    bool result4 = RateLimitedLogger::getInstance().logFunction(functionName, timeWindowMs, maxCount);
    EXPECT_TRUE(result4);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(functionName), 1);
}

/**
 * @tc.name: ShouldHandleMultipleFunctionsWithRapidCalls
 * @tc.desc: Should handle multiple functions with rapid calls
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleMultipleFunctionsWithRapidCalls, TestSize.Level1)
{
    // Given - Multiple functions with different limits
    std::string function1 = "function1";
    std::string function2 = "function2";
    std::string function3 = "function3";
    
    // When - Make rapid calls to all functions
    std::vector<bool> results1, results2, results3;
    for (int32_t i = 0; i < 5; ++i) {
        results1.push_back(RateLimitedLogger::getInstance().logFunction(function1, 1000, 2));
        results2.push_back(RateLimitedLogger::getInstance().logFunction(function2, 1000, 3));
        results3.push_back(RateLimitedLogger::getInstance().logFunction(function3, 1000, 1));
    }
    
    // Then - Each function should respect its own limits
    // function1: 2 allowed out of 5
    EXPECT_EQ(std::count(results1.begin(), results1.end(), true), 2);
    // function2: 3 allowed out of 5
    EXPECT_EQ(std::count(results2.begin(), results2.end(), true), 3);
    // function3: 1 allowed out of 5
    EXPECT_EQ(std::count(results3.begin(), results3.end(), true), 1);
}

/**
 * @tc.name: ShouldHandleLongFunctionNames
 * @tc.desc: Should handle very long function names
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, ShouldHandleLongFunctionNames, TestSize.Level1)
{
    // Given - Very long function name
    std::string longFunctionName = "VeryLongFunctionNameWithMultipleComponentsAnd"
        "SpecialCharacters_1234567890_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = 2;
    
    // When - Log with long function name
    bool result1 = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);
    bool result2 = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);
    bool result3 = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);
    
    // Then - Should work normally with long names
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_FALSE(result3);
    EXPECT_EQ(RateLimitedLogger::getInstance().getCurrentCount(longFunctionName), 2);
}

/**
 * @tc.name: TurnOffLogRateLimitingSwitchTest
 * @tc.desc: Turn off the log rate limiting switch test
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, TurnOffLogRateLimitingSwitchTest, TestSize.Level1)
{
    // Given - Turn off the log rate limiting switch
    std::string longFunctionName = "testFunction";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = 1;
    RateLimitedLogger::getInstance().setEnabled(false);
    
    // When - Print multiple logs in a short period of time
    bool result1 = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);
    bool result2 = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);
    bool result3 = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);
    
    // Then - The log should be printed normally
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(result3);
}

/**
 * @tc.name: AbnormalTimeWindowMsParameter
 * @tc.desc: Abnormal timeWindowMs Parameter test
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, AbnormalTimeWindowMsParameter, TestSize.Level1)
{
    // Given - Invalid timeWindowMs parameter
    std::string longFunctionName = "testFunction";
    int32_t timeWindowMs = 0;
    int32_t maxCount = 1;
    
    // When - Log once
    bool result = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);

    // Then - The log should be printed fail
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AbnormalMaxCountParameter
 * @tc.desc: Abnormal maxCount Parameter test
 * @tc.type: FUNC
 */
HWTEST_F(RateLimitedLoggerTest, AbnormalMaxCountParameter, TestSize.Level1)
{
    // Given - Invalid timeWindowMs parameter
    std::string longFunctionName = "testFunction";
    int32_t timeWindowMs = 1000;
    int32_t maxCount = 0;
    
    // When - Log once
    bool result = RateLimitedLogger::getInstance().logFunction(longFunctionName, timeWindowMs, maxCount);

    // Then - The log should be printed fail
    EXPECT_FALSE(result);
}
} // namespace
} // Rosen
} // OHOS