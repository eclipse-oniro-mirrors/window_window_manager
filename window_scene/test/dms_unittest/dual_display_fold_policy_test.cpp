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

#include "fold_screen_controller/dual_display_fold_policy.h"
#include "fold_screen_controller/fold_screen_controller.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 500000;
const std::string DUAL_DISPLAY_FOLD_POLICY_TEST = "DualDisplayFoldPolicyTest";
}
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
    g_errLog = msg;
    }
}

class DualDisplayFoldPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static ScreenSessionManager& ssm_;
};

ScreenSessionManager& DualDisplayFoldPolicyTest::ssm_ = ScreenSessionManager::GetInstance();

void DualDisplayFoldPolicyTest::SetUpTestCase()
{
}

void DualDisplayFoldPolicyTest::TearDownTestCase()
{
}

void DualDisplayFoldPolicyTest::SetUp()
{
}

void DualDisplayFoldPolicyTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

    /**
     * @tc.name: ChangeScreenDisplayMode
     * @tc.desc: ChangeScreenDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayMode, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::UNKNOWN);
        EXPECT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::SUB);
        EXPECT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::MAIN);
        EXPECT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::COORDINATION);
        EXPECT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: ChangeScreenDisplayMode
     * @tc.desc: ChangeScreenDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayMode02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> session = new ScreenSession();
        ScreenId screenId = 1001;
        ssm_.screenSessionMap_[screenId] = session;
        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::UNKNOWN);
        FoldDisplayMode mode = ssm_.GetFoldDisplayMode();
        ASSERT_EQ(mode, ssm_.GetFoldDisplayMode());

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::SUB);
        mode = ssm_.GetFoldDisplayMode();
        ASSERT_EQ(mode, ssm_.GetFoldDisplayMode());

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::MAIN);
        mode = ssm_.GetFoldDisplayMode();
        ASSERT_EQ(mode, ssm_.GetFoldDisplayMode());

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::COORDINATION);
        mode = ssm_.GetFoldDisplayMode();
        ASSERT_EQ(mode, ssm_.GetFoldDisplayMode());
    }

    /**
     * @tc.name: SendSensorResult
     * @tc.desc: SendSensorResult
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, SendSensorResult, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.SendSensorResult(FoldStatus::UNKNOWN);
        EXPECT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: SetOnBootAnimation
     * @tc.desc: SetOnBootAnimation
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, SetOnBootAnimation, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.SetOnBootAnimation(true);
        ASSERT_TRUE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.SetOnBootAnimation(false);
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: GetCurrentFoldCreaseRegion
     * @tc.desc: GetCurrentFoldCreaseRegion
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<FoldCreaseRegion> res = dualDisplayFoldPolicy.GetCurrentFoldCreaseRegion();
        ASSERT_EQ(res, dualDisplayFoldPolicy.currentFoldCreaseRegion_);
    }

    /**
     * @tc.name: GetSupportedFoldStatus
     * @tc.desc: GetSupportedFoldStatus
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, GetSupportedFoldStatus, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy policy(mutex, std::shared_ptr<TaskScheduler>());
        EXPECT_THAT(policy.GetSupportedFoldStatus(),
            UnorderedElementsAre(
                FoldStatus::EXPAND,
                FoldStatus::FOLDED,
                FoldStatus::HALF_FOLD));
    }

    /**
     * @tc.name: LockDisplayStatus
     * @tc.desc: LockDisplayStatus
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, LockDisplayStatus, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        bool locked = false;
        dualDisplayFoldPolicy.LockDisplayStatus(locked);
        ASSERT_EQ(locked, dualDisplayFoldPolicy.lockDisplayStatus_);
    }

    /**
     * @tc.name: RecoverWhenBootAnimationExit0
     * @tc.desc: RecoverWhenBootAnimationExit0
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, RecoverWhenBootAnimationExit0, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.currentFoldStatus_ = FoldStatus::UNKNOWN;
        dualDisplayFoldPolicy.currentDisplayMode_ = FoldDisplayMode::MAIN;
        dualDisplayFoldPolicy.RecoverWhenBootAnimationExit();
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: UpdateForPhyScreenPropertyChange0
     * @tc.desc: UpdateForPhyScreenPropertyChange0
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, UpdateForPhyScreenPropertyChange0, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.currentFoldStatus_ = FoldStatus::UNKNOWN;
        dualDisplayFoldPolicy.currentDisplayMode_ = FoldDisplayMode::MAIN;
        dualDisplayFoldPolicy.UpdateForPhyScreenPropertyChange();
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: ChangeScreenDisplayModeInner01
     * @tc.desc: ChangeScreenDisplayModeInner01
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayModeInner01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.onBootAnimation_ = true;
        sptr<ScreenSession> screenSession = new ScreenSession();
        ScreenId offScreenId = 0;
        ScreenId onScreenId = 5;
        dualDisplayFoldPolicy.ChangeScreenDisplayModeInner(screenSession, offScreenId, onScreenId);
        ASSERT_EQ(dualDisplayFoldPolicy.GetCurrentScreenId(), 5);
    }

    /**
     * @tc.name: ChangeScreenDisplayModeInner02
     * @tc.desc: ChangeScreenDisplayModeInner02
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayModeInner02, TestSize.Level1)
    {
        if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            std::recursive_mutex mutex;
            std::shared_ptr<TaskScheduler> taskScheduler = std::make_shared<TaskScheduler>("dualTest");
            DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, taskScheduler);
            dualDisplayFoldPolicy.onBootAnimation_ = false;
            sptr<ScreenSession> screenSession = new ScreenSession();
            screenSession->SetScreenId(0);
            ScreenId offScreenId = 0;
            ScreenId onScreenId = 5;
            dualDisplayFoldPolicy.ChangeScreenDisplayModeInner(screenSession, offScreenId, onScreenId);
            ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
        }
    }

    /**
     * @tc.name: ChangeScreenDisplayModeToCoordination
     * @tc.desc: ChangeScreenDisplayModeToCoordination
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayModeToCoordination, TestSize.Level1)
    {
        if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            std::recursive_mutex mutex;
            std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>(
            DUAL_DISPLAY_FOLD_POLICY_TEST);
            sptr<DualDisplayFoldPolicy> dualDisplayFoldPolicy = new DualDisplayFoldPolicy(
                mutex, screenPowerTaskScheduler);
            dualDisplayFoldPolicy->ChangeScreenDisplayModeToCoordination();
            EXPECT_TRUE(ssm_.keyguardDrawnDone_);
        }
    }

    /**
     * @tc.name: UpdateForPhyScreenPropertyChange
     * @tc.desc: UpdateForPhyScreenPropertyChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, UpdateForPhyScreenPropertyChange, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        dualDisplayFoldPolicy.UpdateForPhyScreenPropertyChange();
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: GetModeMatchStatus01
     * @tc.desc: GetModeMatchStatus
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, GetModeMatchStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        FoldDisplayMode ret = dualDisplayFoldPolicy.GetModeMatchStatus();
        ASSERT_EQ(ret, FoldDisplayMode::UNKNOWN);

        dualDisplayFoldPolicy.currentFoldStatus_ = FoldStatus::EXPAND;
        ret = dualDisplayFoldPolicy.GetModeMatchStatus();
        ASSERT_EQ(ret, FoldDisplayMode::MAIN);

        dualDisplayFoldPolicy.currentFoldStatus_ = FoldStatus::FOLDED;
        ret = dualDisplayFoldPolicy.GetModeMatchStatus();
        ASSERT_EQ(ret, FoldDisplayMode::SUB);

        dualDisplayFoldPolicy.currentFoldStatus_ = FoldStatus::HALF_FOLD;
        ret = dualDisplayFoldPolicy.GetModeMatchStatus();
        ASSERT_EQ(ret, FoldDisplayMode::MAIN);
    }

    /**
     * @tc.name: GetModeMatchStatus02
     * @tc.desc: test function : GetModeMatchStatus
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, GetModeMatchStatus02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy policy(mutex, std::shared_ptr<TaskScheduler>());

        FoldStatus targetFoldStatus = FoldStatus::EXPAND;
        FoldDisplayMode ret = policy.GetModeMatchStatus(targetFoldStatus);
        EXPECT_EQ(FoldDisplayMode::MAIN, ret);

        targetFoldStatus = FoldStatus::FOLDED;
        ret = policy.GetModeMatchStatus(targetFoldStatus);
        EXPECT_EQ(FoldDisplayMode::SUB, ret);

        targetFoldStatus = FoldStatus::HALF_FOLD;
        ret = policy.GetModeMatchStatus(targetFoldStatus);
        EXPECT_EQ(FoldDisplayMode::MAIN, ret);

        targetFoldStatus = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
        ret = policy.GetModeMatchStatus(targetFoldStatus);
        EXPECT_EQ(FoldDisplayMode::UNKNOWN, ret);
    }

    /**
     * @tc.name: ReportFoldDisplayModeChange
     * @tc.desc: ReportFoldDisplayModeChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ReportFoldDisplayModeChange, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        FoldDisplayMode expectMode = FoldDisplayMode::FULL;
        dualDisplayFoldPolicy.ReportFoldDisplayModeChange(expectMode);
        FoldDisplayMode mode = ssm_.GetFoldDisplayMode();
        EXPECT_FALSE(g_errLog.find("Write HiSysEvent error, ret: %{public}d") !=
                    std::string::npos);
    }

    /**
     * @tc.name: ReportFoldStatusChangeBegin
     * @tc.desc: ReportFoldStatusChangeBegin
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ReportFoldStatusChangeBegin, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        FoldDisplayMode originalmode = ssm_.GetFoldDisplayMode();
        int32_t offScreen = 0;
        int32_t onScreen = 1;
        dualDisplayFoldPolicy.ReportFoldStatusChangeBegin(offScreen, onScreen);
        ASSERT_FALSE(g_errLog.find("ReportFoldStatusChangeBegin Write HiSusEvent error") != std::string::npos);
    }

    /**
     * @tc.name: ChangeScreenDisplayModeOnBootAnimation
     * @tc.desc: ChangeScreenDisplayModeOnBootAnimation
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayModeOnBootAnimation, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        ScreenId screenId = 0;
        std::string threadName = "test";
        dualDisplayFoldPolicy.screenPowerTaskScheduler_ = std::make_shared<TaskScheduler>(threadName);
        dualDisplayFoldPolicy.ChangeScreenDisplayModeOnBootAnimation(screenSession, screenId);
        ASSERT_EQ(screenId, dualDisplayFoldPolicy.screenId_);
    }

    /**
     * @tc.name: ChangeScreenDisplayModeOnBootAnimation2
     * @tc.desc: ChangeScreenDisplayModeOnBootAnimation2
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayModeOnBootAnimation2, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        ScreenId screenId = 5;
        std::string threadName = "test";
        dualDisplayFoldPolicy.screenPowerTaskScheduler_ = std::make_shared<TaskScheduler>(threadName);
        dualDisplayFoldPolicy.ChangeScreenDisplayModeOnBootAnimation(screenSession, screenId);
        ASSERT_EQ(screenId, dualDisplayFoldPolicy.screenId_);
    }

    /**
     * @tc.name: SetdisplayModeChangeStatus01
     * @tc.desc: SetdisplayModeChangeStatus
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, SetdisplayModeChangeStatus01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        bool status = true;
        dualDisplayFoldPolicy.SetdisplayModeChangeStatus(status);
        ASSERT_EQ(dualDisplayFoldPolicy.pengdingTask_, 3);
    }

    /**
     * @tc.name: SetdisplayModeChangeStatus02
     * @tc.desc: SetdisplayModeChangeStatus
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, SetdisplayModeChangeStatus02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        bool status = false;
        dualDisplayFoldPolicy.SetdisplayModeChangeStatus(status);
        ASSERT_NE(dualDisplayFoldPolicy.pengdingTask_, 3);
    }

    /**
     * @tc.name: CheckDisplayMode01
     * @tc.desc: CheckDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, CheckDisplayMode01, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
        auto ret = dualDisplayFoldPolicy.CheckDisplayMode(displayMode);
        ASSERT_EQ(ret, false);
    }

    /**
     * @tc.name: CheckDisplayMode02
     * @tc.desc: CheckDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, CheckDisplayMode02, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        FoldDisplayMode displayMode = FoldDisplayMode::COORDINATION;
        auto ret = dualDisplayFoldPolicy.CheckDisplayMode(displayMode);
        ASSERT_EQ(ret, false);
    }

    /**
     * @tc.name: CheckDisplayMode03
     * @tc.desc: CheckDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, CheckDisplayMode03, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        FoldDisplayMode displayMode = FoldDisplayMode::FULL;
        auto ret = dualDisplayFoldPolicy.CheckDisplayMode(displayMode);
        ASSERT_EQ(ret, true);
    }

    /**
     * @tc.name: ChangeScreenDisplayMode03
     * @tc.desc: ChangeScreenDisplayMode
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, ChangeScreenDisplayMode03, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::UNKNOWN);
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::SUB);
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::MAIN);
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);

        dualDisplayFoldPolicy.ChangeScreenDisplayMode(FoldDisplayMode::COORDINATION);
        ASSERT_FALSE(dualDisplayFoldPolicy.onBootAnimation_);
    }

    /**
     * @tc.name: TriggerSensorInSubWhenSessionNull
     * @tc.desc: TriggerSensorInSub
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, TriggerSensorInSubWhenSessionNull, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        screenSession->SetScreenId(0);
        dualDisplayFoldPolicy.TriggerSensorInSub(screenSession);
        screenSession->SetScreenId(5);
        dualDisplayFoldPolicy.TriggerSensorInSub(screenSession);
        EXPECT_EQ(screenSession->GetScreenId(), 5);
    }

    /**
     * @tc.name: TriggerSensorInSubWhenSessionNotNull
     * @tc.desc: TriggerSensorInSub
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplayFoldPolicyTest, TriggerSensorInSubWhenSessionNotNull, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        auto screenSessionMapMainIter = ScreenSessionManager::GetInstance().screenSessionMap_.find(0);
        DualDisplayFoldPolicy dualDisplayFoldPolicy(mutex, std::shared_ptr<TaskScheduler>());
        sptr<ScreenSession> screenSession = new ScreenSession();
        screenSession->SetScreenId(5);
        ScreenSessionConfig config;
        sptr<ScreenSession> screenSessionMain =
            new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_REAL);
        screenSessionMain->SensorRotationChange(0.0F);
        ScreenSessionManager::GetInstance().screenSessionMap_[0] = screenSessionMain;
        dualDisplayFoldPolicy.TriggerSensorInSub(screenSession);
        screenSessionMain->SensorRotationChange(180.0F);
        ScreenSessionManager::GetInstance().screenSessionMap_[0] = screenSessionMain;
        dualDisplayFoldPolicy.TriggerSensorInSub(screenSession);
        EXPECT_EQ(screenSessionMain->GetSensorRotation(), 180.0F);
        if (screenSessionMapMainIter != ScreenSessionManager::GetInstance().screenSessionMap_.end()) {
            ScreenSessionManager::GetInstance().screenSessionMap_[0] = screenSessionMapMainIter->second;
        }
    }
}
} // namespace Rosen
} // namespace OHOS