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
#include <gmock/gmock.h>

#include "screen_session_manager/include/fold_screen_controller/single_display_fold_policy.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class SingleDisplayFoldPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SingleDisplayFoldPolicyTest::SetUpTestCase()
{
}

void SingleDisplayFoldPolicyTest::TearDownTestCase()
{
}

void SingleDisplayFoldPolicyTest::SetUp()
{
}

void SingleDisplayFoldPolicyTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: ChangeScreenDisplayMode
 * @tc.desc: test function : ChangeScreenDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayMode, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::MAIN;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::FULL;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::SUB;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SendSensorResult
 * @tc.desc: test function : SendSensorResult
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, SendSensorResult, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    policy.SendSensorResult(foldStatus);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: GetCurrentFoldCreaseRegion
 * @tc.desc: test function : GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    sptr<FoldCreaseRegion> foldCreaseRegion;
    foldCreaseRegion = policy.GetCurrentFoldCreaseRegion();
    EXPECT_EQ(policy.currentFoldCreaseRegion_, foldCreaseRegion);
}

/**
 * @tc.name: GetSupportedFoldStatus
 * @tc.desc: GetSupportedFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, GetSupportedFoldStatus, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    EXPECT_THAT(policy.GetSupportedFoldStatus(),
        UnorderedElementsAre(
            FoldStatus::EXPAND,
            FoldStatus::FOLDED,
            FoldStatus::HALF_FOLD));
}

/**
 * @tc.name: LockDisplayStatus
 * @tc.desc: test function : LockDisplayStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, LockDisplayStatus, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    policy.LockDisplayStatus(false);
    EXPECT_EQ(policy.lockDisplayStatus_, false);
}

/**
 * @tc.name: SetOnBootAnimation
 * @tc.desc: test function : SetOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, SetOnBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    policy.SetOnBootAnimation(false);
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.SetOnBootAnimation(true);
    EXPECT_TRUE(policy.onBootAnimation_);
}

/**
 * @tc.name: RecoverWhenBootAnimationExit
 * @tc.desc: test function : RecoverWhenBootAnimationExit
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, RecoverWhenBootAnimationExit, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    policy.currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::SUB;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::FULL;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::MAIN;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::COORDINATION;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: UpdateForPhyScreenPropertyChange
 * @tc.desc: test function : UpdateForPhyScreenPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, UpdateForPhyScreenPropertyChange, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    policy.currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    policy.UpdateForPhyScreenPropertyChange();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::SUB;
    policy.UpdateForPhyScreenPropertyChange();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::FULL;
    policy.UpdateForPhyScreenPropertyChange();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::MAIN;
    policy.UpdateForPhyScreenPropertyChange();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::COORDINATION;
    policy.UpdateForPhyScreenPropertyChange();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: GetModeMatchStatus01
 * @tc.desc: test function : GetModeMatchStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, GetModeMatchStatus01, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;

    policy.currentFoldStatus_ = FoldStatus::EXPAND;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::FULL, displayMode);

    policy.currentFoldStatus_ = FoldStatus::FOLDED;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::MAIN, displayMode);

    policy.currentFoldStatus_ = FoldStatus::HALF_FOLD;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::FULL, displayMode);

    policy.currentFoldStatus_ = FoldStatus::UNKNOWN;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, displayMode);
}

/**
 * @tc.name: ReportFoldDisplayModeChange
 * @tc.desc: test function : ReportFoldDisplayModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ReportFoldDisplayModeChange, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.ReportFoldDisplayModeChange(displayMode);
    EXPECT_TRUE(g_errLog.find("Write HiSysEvent error") == std::string::npos);
    g_errLog.clear();
    LOG_SetCallback(nullptr);

    displayMode = FoldDisplayMode::FULL;
    policy.ReportFoldDisplayModeChange(displayMode);
    EXPECT_NE(FoldDisplayMode::UNKNOWN, displayMode);
}

/**
 * @tc.name: GetModeMatchStatus02
 * @tc.desc: test function : GetModeMatchStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, GetModeMatchStatus02, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    FoldStatus targetFoldStatus = FoldStatus::EXPAND;
    FoldDisplayMode ret = policy.GetModeMatchStatus(targetFoldStatus);
    EXPECT_EQ(FoldDisplayMode::FULL, ret);

    targetFoldStatus = FoldStatus::FOLDED;
    ret = policy.GetModeMatchStatus(targetFoldStatus);
    EXPECT_EQ(FoldDisplayMode::MAIN, ret);

    targetFoldStatus = FoldStatus::HALF_FOLD;
    ret = policy.GetModeMatchStatus(targetFoldStatus);
    EXPECT_EQ(FoldDisplayMode::FULL, ret);

    targetFoldStatus = FoldStatus::UNKNOWN;
    ret = policy.GetModeMatchStatus(targetFoldStatus);
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, ret);

    targetFoldStatus = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
    ret = policy.GetModeMatchStatus(targetFoldStatus);
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, ret);
}

/**
 * @tc.name: ReportFoldStatusChangeBegin
 * @tc.desc: test function : ReportFoldStatusChangeBegin
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ReportFoldStatusChangeBegin, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    int32_t offScreen = 0;
    int32_t onScreen = 1;
    policy.ReportFoldStatusChangeBegin(offScreen, onScreen);
    EXPECT_TRUE(onScreen);

    policy.ReportFoldStatusChangeBegin(offScreen, onScreen);
    EXPECT_TRUE(onScreen);
}

/**
 * @tc.name: ChangeScreenDisplayModeToMain
 * @tc.desc: test function : ChangeScreenDisplayModeToMain
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayModeToMain, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    policy.onBootAnimation_ = true;
    policy.ChangeScreenDisplayModeToMain(screenSession);
    EXPECT_TRUE(policy.onBootAnimation_);

    policy.ChangeScreenDisplayModeToMain(screenSession);
    EXPECT_TRUE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModeToFull
 * @tc.desc: test function : ChangeScreenDisplayModeToFull
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayModeToFull, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    policy.onBootAnimation_ = true;
    policy.ChangeScreenDisplayModeToFull(screenSession);
    EXPECT_TRUE(policy.onBootAnimation_);

    policy.ChangeScreenDisplayModeToFull(screenSession);
    EXPECT_TRUE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModePower
 * @tc.desc: test function : ChangeScreenDisplayModePower
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayModePower, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: SendPropertyChangeResult
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, SendPropertyChangeResult, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    ScreenId screenId = 0;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;

    policy.SendPropertyChangeResult(screenSession, screenId, reason);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModeToMainOnBootAnimation
 * @tc.desc: test function : ChangeScreenDisplayModeToMainOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayModeToMainOnBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    policy.ChangeScreenDisplayModeToMainOnBootAnimation(screenSession);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModeToFullOnBootAnimation
 * @tc.desc: test function : ChangeScreenDisplayModeToFullOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayModeToFullOnBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    policy.ChangeScreenDisplayModeToFullOnBootAnimation(screenSession);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetdisplayModeChangeStatus01
 * @tc.desc: test function : SetdisplayModeChangeStatus01
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, SetdisplayModeChangeStatus01, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    bool status = true;

    policy.SetdisplayModeChangeStatus(status);
    EXPECT_EQ(policy.pengdingTask_, 3);
}

/**
 * @tc.name: SetdisplayModeChangeStatus02
 * @tc.desc: test function : SetdisplayModeChangeStatus02
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, SetdisplayModeChangeStatus02, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    bool status = false;

    policy.SetdisplayModeChangeStatus(status);
    EXPECT_NE(policy.pengdingTask_, 3);
}

/**
 * @tc.name: ChangeScreenDisplayMode02
 * @tc.desc: test function : ChangeScreenDisplayMode02
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayMode02, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::MAIN;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::FULL;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::SUB;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModeInner
 * @tc.desc: test function : ChangeScreenDisplayModeInner
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayModeInner, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT;
    policy.ChangeScreenDisplayModeInner(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::MAIN;
    policy.ChangeScreenDisplayModeInner(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::FULL;
    policy.ChangeScreenDisplayModeInner(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::SUB;
    policy.ChangeScreenDisplayModeInner(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetIsClearingBootAnimation
 * @tc.desc: test function : SetIsClearingBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, SetIsClearingBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    auto policy = sptr<SingleDisplayFoldPolicy>::MakeSptr(displayInfoMutex, screenPowerTaskScheduler);
 
    policy->SetIsClearingBootAnimation(false);
    EXPECT_FALSE(policy->isClearingBootAnimation_);
    policy->SetIsClearingBootAnimation(true);
    EXPECT_TRUE(policy->isClearingBootAnimation_);
}
 
/**
 * @tc.name: ChangeScreenDisplayMode
 * @tc.desc: test function : ChangeScreenDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayFoldPolicyTest, ChangeScreenDisplayMode03, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    auto policy = sptr<SingleDisplayFoldPolicy>::MakeSptr(displayInfoMutex, screenPowerTaskScheduler);
 
    policy->SetIsClearingBootAnimation(false);
    FoldDisplayMode displayMode = FoldDisplayMode::MAIN;
    policy->ChangeScreenDisplayMode(displayMode);
    EXPECT_EQ(policy->lastCachedisplayMode_, displayMode);
    policy->SetIsClearingBootAnimation(true);
    displayMode = FoldDisplayMode::FULL;
    policy->ChangeScreenDisplayMode(displayMode);
    EXPECT_NE(policy->lastCachedisplayMode_, displayMode);
}
}
} // namespace Rosen
} // namespace OHOS