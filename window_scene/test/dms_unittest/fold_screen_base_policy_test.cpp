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

#include <gtest/gtest.h>
#include <hisysevent.h>
#include <parameters.h>
#include "gmock/gmock.h"
#include <functional>
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "fold_screen_base_controller.h"

#define private public
#define protected public
#include "fold_screen_base_policy.h"
#include "fold_crease_region_controller.h"
#include "fold_screen_state_internel.h"

#undef private
#undef protected

namespace {
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
                   const char* msg)
{
    g_logMsg += msg;
}
}
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace DMS {
constexpr uint32_t SLEEP_TIME_US = 100000;

class MockFoldScreenBasePolicy : public : MockFoldScreenBasePolicy {
public:
    MOCK_METHOD(bool, GetModeChangeRunningStatus, (), (override));
    MOCK_METHOD(FoldStatus, GetFoldStatus, (), (override));
    MOCK_METHOD(FoldStatus, GetPhysicalFoldStatus, (), (override));
    MOCK_METHOD(void, ChangeScreenDisplayMode, (FoldDisplayMode displayMode,
    DisplayModeChangeReason reason), (override));
    MOCK_METHOD(bool, GetPhysicalFoldLockFlag, (), (override, const));

    MOCK_METHOD(FoldDisplayMode, GetModeMatchStatus, (FoldStatus status), (override));
    MOCK_METHOD(FoldStatus, GetForcedFoldStatus, (), (override, const));

    MOCK_METHOD(FoldDisplayMode, GetModeMatchStatus, (), (override));
    MOCK_METHOD(const std::unordered_set<FoldStatus>&, GetSupportedFoldStates, (override, const));
    MOCK_METHOD(bool, IsFoldStatusSupported, (const std::unordered_set<FoldStatus>& supportedFoldStates,
    FoldStatus targetFoldStatus), (override, const));
};

class FoldScreenBasePolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static const std::unordered_set<FoldStatus> supportedFoldStatusForTest;
    bool newController = false;
    std::unique_ptr<MockFoldScreenBasePolicy> mockBasePolicy;
};

void FoldScreenBasePolicyTest::SetUpTestCase() {}

void FoldScreenBasePolicyTest::TearDownTestCase() {}

void FoldScreenBasePolicyTest::SetUp()
{
    if (ScreenSessionManager::GetInstance().foldScreenController_ == nullptr) {
        ScreenSessionManager::GetInstance().foldScreenController_ = new (std::nothrow) DMS::FoldScreenBaseController();
        newController = true;
    }
    mockBasePolicy = std::make_unique<MockFoldScreenBasePolicy>();
    ::testing::Mock::VerifyAndClearExpectations(mockBasePolicy.get());
}

void FoldScreenBasePolicyTest::TearDown()
{
    if (newController) {
        ScreenSessionManager::GetInstance().foldScreenController_ == nullptr;
        newController = false;
    }
    ::testing::Mock::VerifyAndClearExpectations(mockBasePolicy.get());
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

const std::unordered_set<FoldStatus> FoldScreenBasePolicyTest::supportedFoldStatusForTest = {
    FoldStatus::EXPAND,
    FoldStatus::FOLDED,
    FoldStatus::HALF_FOLD
};

namespace {

/**
 * @tc.name: ClearStateTest
 * @tc.desc: test function : ClearState
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ClearStateTest, TestSize.Level1)
{
    FoldScreenBasePolicy::GetInstance().currentDisplayMode_ = FoldDisplayMode::MAIN;
    FoldScreenBasePolicy::GetInstance().currentFoldStatus_ = FoldStatus::FOLDED;
    FoldScreenBasePolicy::GetInstance().ClearState();
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, FoldScreenBasePolicy::GetInstance().currentDisplayMode_);
    EXPECT_EQ(FoldStatus::UNKNOWN, FoldScreenBasePolicy::GetInstance().currentFoldStatus_);
}

/**
 * @tc.name: ChangeOnTentModeTest
 * @tc.desc: test function : ChangeOnTentMode
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeOnTentModeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    FoldScreenBasePolicy::GetInstance().ChangeOnTentMode(FoldStatus::UNKNOWN);
    EXPECT_TRUE(g_logMsg.find("current state:0 invalid") != std::string::npos);

    FoldScreenBasePolicy::GetInstance().ChangeOnTentMode(FoldStatus::HALF_FOLD);
    EXPECT_TRUE(g_logMsg.find("Enter tent mode, current state:3") != std::string::npos);

    FoldScreenBasePolicy::GetInstance().ChangeOnTentMode(FoldStatus::FOLDED);
    EXPECT_TRUE(g_logMsg.find("Enter tent mode, current state:2") != std::string::npos);
}

/**
 * @tc.name: ChangeScreenDisplayModeToCoordinationTest
 * @tc.desc: test function : ChangeScreenDisplayModeToCoordination
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeToCoordinationTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeToCoordination();
    EXPECT_TRUE(g_logMsg.find("change displaymode to coordination skipped, current coordination flag is true") !=
                std::string::npos);

    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeToCoordination();
    EXPECT_TRUE(g_logMsg.find("screenId: 5, command: 1") != std::string::npos);
}

/**
 * @tc.name: ChangeScreenDisplayModeToCoordinationTest_wait
 * @tc.desc: test function : ChangeScreenDisplayModeToCoordination waiting path
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeToCoordinationTest_wait, TestSize.Level1)
{
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    ScreenSessionManager::GetInstance().SetWaitingForCoordinationReady(true);
    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeToCoordination();
    EXPECT_TRUE(ScreenSessionManager::GetInstance().GetCoordinationFlag());
    EXPECT_TRUE(ScreenSessionManager::GetInstance().GetWaitingForCoordinationReady());

    ScreenSessionManager::GetInstance().SetWaitingForCoordinationReady(false);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
}

/**
 * @tc.name: CloseCoordinationScreenTest
 * @tc.desc: test function : CloseCoordinationScreen
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, CloseCoordinationScreenTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    FoldScreenBasePolicy::GetInstance().CloseCoordinationScreen();
    EXPECT_TRUE(g_logMsg.find("CloseCoordinationScreen skipped, current coordination flag is false") !=
                std::string::npos);

    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    FoldScreenBasePolicy::GetInstance().CloseCoordinationScreen();
    EXPECT_TRUE(!ScreenSessionManager::GetInstance().GetCoordinationFlag());
}

/**
 * @tc.name: CloseCoordinationScreenTest_wait
 * @tc.desc: test function : CloseCoordinationScreen waiting path
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, CloseCoordinationScreenTest_wait, TestSize.Level1)
{
    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    ScreenSessionManager::GetInstance().SetWaitingForCoordinationReady(true);
    FoldScreenBasePolicy::GetInstance().CloseCoordinationScreen();
    EXPECT_TRUE(!ScreenSessionManager::GetInstance().GetCoordinationFlag());
    EXPECT_TRUE(ScreenSessionManager::GetInstance().GetWaitingForCoordinationReady());

    ScreenSessionManager::GetInstance().SetWaitingForCoordinationReady(false);
}

/**
 * @tc.name: ExitCoordinationTest
 * @tc.desc: test function : ExitCoordination
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ExitCoordinationTest, TestSize.Level1)
{
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    FoldScreenBasePolicy::GetInstance().ExitCoordination();
    EXPECT_FALSE(ScreenSessionManager::GetInstance().GetCoordinationFlag());

    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    ScreenSessionManager::GetInstance().SetWaitingForCoordinationReady(true);
    FoldScreenBasePolicy::GetInstance().ExitCoordination();
    EXPECT_TRUE(!ScreenSessionManager::GetInstance().GetCoordinationFlag());
    EXPECT_TRUE(ScreenSessionManager::GetInstance().GetWaitingForCoordinationReady());

    ScreenSessionManager::GetInstance().SetWaitingForCoordinationReady(false);
}

/**
 * @tc.name: ReportFoldDisplayModeChangeTest
 * @tc.desc: test function : ReportFoldDisplayModeChange
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ReportFoldDisplayModeChangeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    FoldScreenBasePolicy::GetInstance().ReportFoldDisplayModeChange(FoldDisplayMode::FULL);
    EXPECT_TRUE(g_logMsg.find("ReportFoldDisplayModeChange displayMode: 1") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ChangeScreenDisplayModeToMainOnBootAnimationTest
 * @tc.desc: test function : ChangeScreenDisplayModeToMainOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeToMainOnBootAnimationTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto screenSession = sptr<ScreenSession>::MakeSptr();

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeToMainOnBootAnimation(screenSession);
    EXPECT_EQ(5, FoldScreenBasePolicy::GetInstance().screenId_);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ChangeScreenDisplayModeToFullOnBootAnimationTest
 * @tc.desc: test function : ChangeScreenDisplayModeToFullOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeToFullOnBootAnimationTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto screenSession = sptr<ScreenSession>::MakeSptr();

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeToFullOnBootAnimation(screenSession);
    EXPECT_EQ(0, FoldScreenBasePolicy::GetInstance().screenId_);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: CheckDisplayModeChangeTest
 * @tc.desc: test function : CheckDisplayModeChange
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, CheckDisplayModeChangeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    bool ret = FoldScreenBasePolicy::GetInstance().CheckDisplayModeChange(FoldDisplayMode::FULL, true);
    EXPECT_EQ(FoldDisplayMode::FULL, FoldScreenBasePolicy::GetInstance().lastCachedisplayMode_);
    EXPECT_TRUE(ret);

    g_logMsg.clear();
    FoldScreenBasePolicy::GetInstance().isClearingBootAnimation_ = true;
    ret = FoldScreenBasePolicy::GetInstance().CheckDisplayModeChange(FoldDisplayMode::FULL, false);
    EXPECT_TRUE(g_logMsg.find("clearing bootAnimation not change displayMode") != std::string::npos);
    EXPECT_TRUE(!ret);

    g_logMsg.clear();
    FoldScreenBasePolicy::GetInstance().isClearingBootAnimation_ = false;
    FoldScreenBasePolicy::GetInstance().displayModeChangeRunning_ = true;
    FoldScreenBasePolicy::GetInstance().startTimePoint_ = std::chrono::steady_clock::now();
    ret = FoldScreenBasePolicy::GetInstance().CheckDisplayModeChange(FoldDisplayMode::FULL, false);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.name: ChangeScreenDisplayModeTest
 * @tc.desc: test function : ChangeScreenDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayMode(FoldDisplayMode::FULL, true);
    EXPECT_TRUE(g_logMsg.find("force change displayMode") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ChangeScreenDisplayModeInnerTest
 * @tc.desc: test function : ChangeScreenDisplayModeInner
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeInnerTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSessionManager::GetInstance().screenSessionMap_[0] = sptr<ScreenSession>::MakeSptr();

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeInner(FoldDisplayMode::UNKNOWN,
                                                                     DisplayModeChangeReason::DEFAULT);
    EXPECT_TRUE(g_logMsg.find("ChangeScreenDisplayMode displayMode is invalid") != std::string::npos);

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeInner(FoldDisplayMode::MAIN,
                                                                     DisplayModeChangeReason::DEFAULT);
    EXPECT_EQ(FoldDisplayMode::MAIN, FoldScreenBasePolicy::GetInstance().currentDisplayMode_);

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeInner(FoldDisplayMode::FULL,
                                                                     DisplayModeChangeReason::DEFAULT);
    EXPECT_EQ(FoldDisplayMode::FULL, FoldScreenBasePolicy::GetInstance().currentDisplayMode_);

    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeInner(FoldDisplayMode::COORDINATION,
                                                                     DisplayModeChangeReason::DEFAULT);
    EXPECT_EQ(FoldDisplayMode::COORDINATION, FoldScreenBasePolicy::GetInstance().currentDisplayMode_);
}

/**
 * @tc.name: ChangeScreenDisplayModeInner
 * @tc.desc: test fold exit coordination
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, ChangeScreenDisplayModeInnerTest_foldExitCoordination, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    FoldScreenBasePolicy::GetInstance().currentFoldStatus_ = FoldStatus::FOLDED;
    FoldScreenBasePolicy::GetInstance().currentDisplayMode_ = FoldDisplayMode::COORDINATION;
    
    FoldDisplayMode displayMode = FoldDisplayMode::FULL;
    DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT;
    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeInner(displayMode, reason);
    EXPECT_EQ(FoldScreenBasePolicy::GetInstance().currentDisplayMode_, FoldDisplayMode::FULL);
    EXPECT_TRUE(g_logMsg.find("Exit coordination and recover full") != std::string::npos);

    FoldScreenBasePolicy::GetInstance().currentFoldStatus_ = FoldStatus::EXPAND;
    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayModeInner(displayMode, reason);
    EXPECT_EQ(FoldScreenBasePolicy::GetInstance().currentDisplayMode_, FoldDisplayMode::FULL);
    EXPECT_TRUE(g_logMsg.find("Exit coordination and recover full") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetAllCreaseRegionTest
 * @tc.desc: test function : GetAllCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, GetAllCreaseRegionTest, TestSize.Level1)
{
    std::vector<FoldCreaseRegionItem> regions;
    FoldCreaseRegionController::GetInstance().GetAllCreaseRegion(regions);
    EXPECT_EQ(3, regions.size());
}

/**
 * @tc.name: GetCurrentFoldCreaseRegionTest
 * @tc.desc: test function : GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBasePolicyTest, GetCurrentFoldCreaseRegionTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = FoldCreaseRegionController::GetInstance().GetCurrentFoldCreaseRegion();
    EXPECT_TRUE(ret != nullptr);
    LOG_SetCallback(nullptr);
}

/**
*@tc.name: SetFoldStatusAndLockControl01
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/

HWTEST_F(FoldScreenBasePolicyTest, SetFoldStatusAndLockControl01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldScreenBasePolicy* policy = mockBasePolicy.get();
    FoldStatus targetStatus = FoldStatus::FOLDED;

    EXPECT_CALL(*mockBasePolicy, GetModeChangeRunningStatus()).Times(1).WillOnce(Return(true));
    g_logMsg.clear();
    DMError ret = policy->SetFoldStatusAndLockControl(true, targetStatus);
    EXPECT_EQ(ret, DMError::DM_ERROR_DISPLAY_MODE_SWITCH_PENDING);

    EXPECT_TRUE(g_logMsg.find("last process not complete") != std::string::npos);
    g_logMsg.clear();
}

/**
*@tc.name: SetFoldStatusAndLockControl02
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/

HWTEST_F(FoldScreenBasePolicyTest, SetFoldStatusAndLockControl02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldScreenBasePolicy* policy = mockBasePolicy.get();
    FoldStatus targetStatus = FoldStatus::UNKNOWN;

    EXPECT_CALL(*mockBasePolicy, GetModeChangeRunningStatus()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockBasePolicy, GetSupportedFoldStatus()).Times(1).WillOnce(ReturnRef(supportedFoldStatusForTest));

    EXPECT_CALL(*mockBasePolicy, IsFoldStatusSupported(testing::_, targetStatus)).Times(1).WillOnce(Return(false));
    g_logMsg.clear();
    DMError ret = policy->SetFoldStatusAndLockControl(true, targetStatus);
    EXPECT_EQ(ret, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);

    EXPECT_TRUE(g_logMsg.find("Current device does not support this fold status") != std::string::npos);
    g_logMsg.clear();
}

/**
*@tc.name: SetFoldStatusAndLockControl03
*@tc.desc:test function : SetFoldStatusAndLockControl
*@tc.type: FUNC
*/

HWTEST_F(FoldScreenBasePolicyTest, SetFoldStatusAndLockControl03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldScreenBasePolicy* policy = mockBasePolicy.get();
    FoldStatus targetStatus = FoldStatus::UNKNOWN;

    EXPECT_CALL(*mockBasePolicy, GetModeChangeRuningStatus()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockBasePolicy, GetPhysicalFoldStatus()).Times(1).WillOnce(Return(targetStatus));
    g_logMsg.clear();
    DMError ret = policy->SetFoldStatusAndLockControl(false, targetStatus);
    EXPECT_EQ(ret, DMError::DM_OK);
    g_logMsg.clear();
}

/**
*@tc.name: SetFoldStatusAndLockControl04
*@tc.desc:test function: SetFoldStatusAndLockControl
*@tc.type: FUNC
*/

HWTEST_F(FoldScreenBasePolicyTest, SetFoldStatusAndLockControl04, TestSize.Level1)
{
    if (FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {

    LOG_SetCallback(MyLogCallback);
    FoldScreenBasePolicy* policy = mockBasePolicy.get();
    FoldStatus oldStatus = FoldStatus::FOLDED;
    FoldStatus targetStatus = FoldStatus::UNKNOWN;

    EXPECT_CALL(*mockBasePolicy, GetModeChangeRuningStatus()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockBasePolicy, GetSupportedFoldStatus()).Times(1).WillOnce(ReturnRef(supportedFoldStatusForTest));

    EXPECT_CALL(*mockBasePolicy, IsFoldStatusSupported(testing::_, targetStatus)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockBasePolicy, GetFoldStatus()).Times(1).WillOnce(Return(oldStatus));

    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    EXPECT_CALL(*mockBasePolicy, GetModeMatchStatus(targetStatus)).Times(1).WillOnce(Return(mode));
    EXPECT_CALL(*mockBasePolicy, ChangeScreenDisplayMode(mode, DisplayModeChangeReason::FORCE_SET)).Times(1);
    g_logMsg.clear();
    DMError ret = policy->SetFoldStatusAndLockControl(true, targetStatus);
    EXPECT_EQ(ret, DMError::DM_OK);

    EXPECT_TRUE(g_logMsg.find("Change fold status from")!= std::string::npos);
    g_logMsg.clear();
    }
}

/**
*@tc.name: SetFoldStatusAndLockControl05
*@tc.desc:test function: SetFoldStatusAndLockControl
*@tc.type: FUNC
*/

HWTEST_F(FoldScreenBasePolicyTest, SetFoldStatusAndLockControl05, TestSize.Level1)
{
    if (FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {

    LOG_SetCallback(MyLogCallback);
    FoldScreenBasePolicy* policy = mockBasePolicy.get();
    FoldStatus oldStatus = FoldStatus::FOLDED;
    FoldStatus physicStatus = FoldStatus::UNKNOWN;

    EXPECT_CALL(*mockBasePolicy, GetModeChangeRunningStatus()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockBasePolicy, GetSupportedFoldStatus()).Times(1).WillOnce(ReturnRef(supportedFoldStatusForTest));

    EXPECT_CALL(*mockBasePolicy, IsFoldStatusSupported(testing::_, physicStatus)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockBasePolicy, GetFoldStatus()).Times(1).WillOnce(Return(oldStatus));

    EXPECT_CALL(*mockBasePolicy, GetPhysicalFoldStatus()).Times(1).WillOnce(Return(physicStatus));

    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    EXPECT_CALL(*mockBasePolicy, GetModeMatchStatus(physicStatus)).Times(1).WillOnce(Return(mode));
    EXPECT_CALL(*mockBasePolicy, ChangeScreenDisplayMode(mode, DisplayModeChangeReason::FORCE_SET)).Times(1);

    g_logMsg.clear();
    DMError ret = policy->SetFoldStatusAndLockControl(false, physicStatus);
    EXPECT_EQ(ret, DMError::DM_OK);

    EXPECT_TRUE(g_logMsg.find("Change fold status from") != std::string::npos);
    g_logMsg.clear();
    }
}

}
}
} // namespace Rosen
} // namespace OHOS
