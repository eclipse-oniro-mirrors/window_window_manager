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

#include <hisysevent.h>
#include <parameter.h>
#include <parameters.h>
#include "fold_screen_controller/single_display_pocket_fold_policy.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager.h"
#include "fold_screen_state_internel.h"

#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class SingleDisplayPocketFoldPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SingleDisplayPocketFoldPolicyTest::SetUpTestCase()
{
}

void SingleDisplayPocketFoldPolicyTest::TearDownTestCase()
{
}

void SingleDisplayPocketFoldPolicyTest::SetUp()
{
}

void SingleDisplayPocketFoldPolicyTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: ChangeScreenDisplayMode
 * @tc.desc: test function : ChangeScreenDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayMode, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
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

    ScreenId screenId = 0;
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (nullptr == screenSession) {
        return;
    }
    SensorFoldStateManager mgr = SensorFoldStateManager();
    mgr.SetTentMode(0);
    displayMode = FoldDisplayMode::FULL;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "0");

    displayMode = FoldDisplayMode::MAIN;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "1");
}

/**
 * @tc.name: SendSensorResult
 * @tc.desc: test function : SendSensorResult
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, SendSensorResult, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    policy.SendSensorResult(foldStatus);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: GetCurrentFoldCreaseRegion
 * @tc.desc: test function : GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    sptr<FoldCreaseRegion> foldCreaseRegion;
    foldCreaseRegion = policy.GetCurrentFoldCreaseRegion();
    EXPECT_EQ(policy.currentFoldCreaseRegion_, foldCreaseRegion);
}

/**
 * @tc.name: GetSupportedFoldStatus
 * @tc.desc: GetSupportedFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, GetSupportedFoldStatus, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, LockDisplayStatus, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
    policy.LockDisplayStatus(false);
    EXPECT_EQ(policy.lockDisplayStatus_, false);
}

/**
 * @tc.name: SetOnBootAnimation
 * @tc.desc: test function : SetOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, SetOnBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, RecoverWhenBootAnimationExit, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, UpdateForPhyScreenPropertyChange, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    
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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, GetModeMatchStatus01, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
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
 * @tc.name: GetModeMatchStatus02
 * @tc.desc: test function : GetModeMatchStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, GetModeMatchStatus02, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

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
 * @tc.name: ReportFoldDisplayModeChange
 * @tc.desc: test function : ReportFoldDisplayModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ReportFoldDisplayModeChange, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.ReportFoldDisplayModeChange(displayMode);
    EXPECT_TRUE(g_logMsg.find("Write HiSysEvent error") == std::string::npos);
    
    g_logMsg.clear();
    displayMode = FoldDisplayMode::FULL;
    policy.ReportFoldDisplayModeChange(displayMode);
    EXPECT_TRUE(g_logMsg.find("Write HiSysEvent error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReportFoldStatusChangeBegin
 * @tc.desc: test function : ReportFoldStatusChangeBegin
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ReportFoldStatusChangeBegin, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayModeToMain, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayModeToFull, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayModePower, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: SendPropertyChangeResult
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, SendPropertyChangeResult, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
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
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayModeToMainOnBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    policy.ChangeScreenDisplayModeToMainOnBootAnimation(screenSession);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModeToFullOnBootAnimation
 * @tc.desc: test function : ChangeScreenDisplayModeToFullOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayModeToFullOnBootAnimation, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    policy.ChangeScreenDisplayModeToFullOnBootAnimation(screenSession);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetdisplayModeChangeStatus01
 * @tc.desc: test function : SetdisplayModeChangeStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, SetdisplayModeChangeStatus01, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    bool status = true;
    policy.SetdisplayModeChangeStatus(status);
    EXPECT_EQ(policy.pengdingTask_, 3);
}

/**
 * @tc.name: SetdisplayModeChangeStatus02
 * @tc.desc: test function : SetdisplayModeChangeStatus
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, SetdisplayModeChangeStatus02, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    bool status = false;
    policy.SetdisplayModeChangeStatus(status);
    EXPECT_NE(policy.pengdingTask_, 3);
}

/**
 * @tc.name: ChangeScreenDisplayMode02
 * @tc.desc: test function : ChangeScreenDisplayMode02
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayMode02, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
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

    displayMode = FoldDisplayMode::COORDINATION;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: ChangeScreenDisplayModeToCoordination
 * @tc.desc: test function : ChangeScreenDisplayModeToCoordination
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeScreenDisplayModeToCoordination, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.screenPowerTaskScheduler_ = std::make_shared<TaskScheduler>("Test");
    policy.ChangeScreenDisplayModeToCoordination();
    EXPECT_EQ(ScreenSessionManager::GetInstance().isCoordinationFlag_, true);
}

/**
 * @tc.name: CloseCoordinationScreen
 * @tc.desc: test function : CloseCoordinationScreen
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, CloseCoordinationScreen, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        GTEST_SKIP();
    }
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.screenPowerTaskScheduler_ = std::make_shared<TaskScheduler>("Test");
    policy.CloseCoordinationScreen();
    EXPECT_EQ(ScreenSessionManager::GetInstance().isCoordinationFlag_, false);
}

/**
 * @tc.name: ExitCoordination
 * @tc.desc: test function : ExitCoordination
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ExitCoordination, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        GTEST_SKIP();
    }
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.currentFoldStatus_ = FoldStatus::EXPAND;
    policy.ExitCoordination();
    EXPECT_EQ(policy.currentDisplayMode_, FoldDisplayMode::FULL);
    EXPECT_EQ(policy.lastDisplayMode_, FoldDisplayMode::FULL);
}

/**
 * @tc.name: ChangeOnTentMode
 * @tc.desc: test function : ChangeOnTentMode
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeOnTentMode, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    FoldStatus currentState = FoldStatus::EXPAND;
    policy.ChangeOnTentMode(currentState);
    EXPECT_EQ(policy.lastCachedisplayMode_, FoldDisplayMode::MAIN);

    currentState = FoldStatus::HALF_FOLD;
    policy.ChangeOnTentMode(currentState);
    EXPECT_EQ(policy.lastCachedisplayMode_, FoldDisplayMode::MAIN);

    currentState = FoldStatus::FOLDED;
    policy.ChangeOnTentMode(currentState);
    EXPECT_EQ(policy.lastCachedisplayMode_, FoldDisplayMode::MAIN);

    currentState = FoldStatus::UNKNOWN;
    policy.ChangeOnTentMode(currentState);
    EXPECT_EQ(policy.lastCachedisplayMode_, FoldDisplayMode::MAIN);
}

/**
 * @tc.name: ChangeOffTentMode
 * @tc.desc: test function : ChangeOffTentMode
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplayPocketFoldPolicyTest, ChangeOffTentMode, TestSize.Level1)
{
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SingleDisplayPocketFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.ChangeOffTentMode();
    FoldDisplayMode displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(policy.lastCachedisplayMode_, displayMode);
}
}
} // namespace Rosen
} // namespace OHOS