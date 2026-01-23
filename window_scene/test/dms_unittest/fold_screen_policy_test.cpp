/*
* Copyright (c) 2026 Huawei Device Co.,Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "screen_session_manager/include/fold_screen_controller/fold_screen_policy.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain,
        const char *tag, const char *msg)
    {
        g_errLog += msg;
    }
}
namespace OHOS {
    namespace Rosen {
        namespace {
            constexpr uint32_t SLEEP_TIME_US = 100000;
}

class MockFoldScreenPolicy : public FoldScreenPolicy {
public:
    MOCK_METHOD(FoldStatus, GetFoldStatus, (), (override));
    MOCK_METHOD(FoldStatus, GetPhysicalFoldStatus, (), (override));
    MOCK_METHOD(void, ChangeScreenDisplayMode, (FoldDisplayMode displayMode,
    DisplayModeChangeReason reason), (override));
    MOCK_METHOD(bool, GetPhysicalFoldLockFlag, (), (override, const));
    MOCK_METHOD(FoldStatus, GetForcedFoldStatus, (), (override, const));

    MOCK_METHOD(FoldDisplayMode, GetModeMatchStatus, (), (override));

    MOCK_METHOD(FoldDisplayMode, GetModeMatchStatus, (FoldStatus status), (override));
    MOCK_METHOD(const std::unordered_set<FoldStatus>&, GetSupportedFoldStates, (), (override, const));
    MOCK_METHOD(bool, IsFoldStatusSupported, (const std::unordered_set<FoldStatus>& supportedFoldStates,
    FoldStatus targetFoldStatus), (override, const));
};

class FoldScreenPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static const std::unordered_set<FoldStatus>supportedFoldStatusForTest;

    std::unique_ptr<MockFoldScreenPolicy> mockPolicy;
};

void FoldScreenPolicyTest::SetUpTestCase()
{
}

void FoldScreenPolicyTest::TearDownTestCase()
{
}

void FoldScreenPolicyTest::SetUp()
{
    mockPolicy = std::make_unique<MockFoldScreenPolicy>();
    ::testing::Mock::VerifyAndClearExpectations(mockPolicy.get());
}

void FoldScreenPolicyTest::TearDown()
{
    ::testing::Mock::VerifyAndClearExpectations(mockPolicy.get());
    usleep(SLEEP_TIME_US);
}

const std::unordered_set<FoldStatus> FoldScreenPolicyTest::supportedFoldStatusForTest = {
    FoldStatus::EXPAND,
    FoldStatus::FOLDED,
    FoldStatus::HALF_FOLD
};

namespace {

/**
*@tc.name: SetFoldStatusAndLockControl01
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/

HWTEST_F(FoldScreenPolicyTest, SetFoldStatusAndLockControl01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldStatus targetStatus = FoldStatus::UNKNOWN;
    EXPECT_CALL(*mockPolicy, GetSupportedFoldStatus()).Times(1).WillOnce(ReturnRef(supportedFoldStatusForTest));
    EXPECT_CALL(*mockPolicy, IsFoldStatusSupported(testing::_, targetStatus)).Times(1).WillOnce(Return(false));
    g_errLog.clear();
    DMError ret = mockPolicy->SetFoldStatusAndLockControl(true, targetStatus);
    EXPECT_EQ(ret, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);

    EXPECT_TRUE(g_errLog.find("Current device does not support this fold status") != std::string::npos);
    g_errLog.clear();
}

/**
*@tc.name: SetFoldStatusAndLockControl02
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/
HWTEST_F(FoldScreenPolicyTest, SetFoldStatusAndLockControl02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldStatus targetStatus = FoldStatus::UNKNOWN;
    EXPECT_CALL(*mockPolicy, GetFoldStatus()).Times(1).WillOnce(Return(targetStatus));
    EXPECT_CALL(*mockPolicy, GetPhysicalFoldStatus()).Times(1).WillOnce(Return(targetStatus));
    g_errLog.clear();
    DMError ret = mockPolicy->SetFoldStatusAndLockControl(false, targetStatus);
    EXPECT_EQ(ret, DMError::DM_OK);
    g_errLog.clear();
}

/**
*@tc.name: SetFoldStatusAndLockControl03
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/
HWTEST_F(FoldScreenPolicyTest, SetFoldStatusAndLockControl03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldStatus oldStatus = FoldStatus::FOLDED;
    FoldStatus targetStatus = FoldStatus::UNKNOWN;
    EXPECT_CALL(*mockPolicy, GetSupportedFoldStatus()).Times(1).WillOnce(ReturnRef(supportedFoldStatusForTest));
    EXPECT_CALL(*mockPolicy, IsFoldStatusSupported(testing::_, targetStatus)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockPolicy, GetFoldStatus()).Times(1).WillOnce(Return(oldStatus));

    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    EXPECT_CALL(*mockPolicy, GetModeMatchStatus(targetStatus)).Times(1).WillOnce(Return(mode));
    EXPECT_CALL(*mockPolicy, ChangeScreenDisplayMode(mode, DisplayModeChangeReason::FORCE_SET)).Times(1);
    EXPECT_CALL(*mockPolicy, GetFoldStatus()).Times(1).WillOnce(Return(oldStatus));

    g_errLog.clear();
    DMError ret = mockPolicy->SetFoldStatusAndLockControl(true, targetStatus);
    EXPECT_EQ(ret, DMError::DM_OK);

    EXPECT_TRUE(g_errLog.find("Change fold status from") != std::string::npos);
    g_errLog.clear();
}

/**
*@tc.name: SetFoldStatusAndLockControl04
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/
HWTEST_F(FoldScreenPolicyTest, SetFoldStatusAndLockControl04, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldStatus oldStatus = FoldStatus::FOLDED;
    FoldStatus physicStatus = FoldStatus::UNKNOWN;
    EXPECT_CALL(*mockPolicy, GetFoldStatus()).Times(1).WillOnce(Return(oldStatus));
    EXPECT_CALL(*mockPolicy, GetPhysicalFoldStatus()).Times(1).WillOnce(Return(physicStatus));

    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    EXPECT_CALL(*mockPolicy, GetModeMatchStatus(physicStatus)).Times(1).WillOnce(Return(mode));
    EXPECT_CALL(*mockPolicy, ChangeScreenDisplayMode(mode, DisplayModeChangeReason::FORCE_SET)).Times(1);

    g_errLog.clear();
    DMError ret = mockPolicy->SetFoldStatusAndLockControl(false, physicStatus);
    EXPECT_EQ(ret, DMError::DM_OK);

    EXPECT_TRUE(g_errLog.find("Change fold status from") != std::string::npos);
    g_errLog.clear();
}
}
} // namespace Rosen
} // namespace OHOS