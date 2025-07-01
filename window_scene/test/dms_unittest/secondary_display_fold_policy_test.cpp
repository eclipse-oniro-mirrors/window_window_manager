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

#include <parameter.h>
#include <parameters.h>
#include "screen_session_manager.h"
#include "screen_session_manager/include/fold_screen_controller/secondary_display_fold_policy.h"
#include "fold_screen_state_internel.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
constexpr uint32_t INDEX_TWO = 2;
constexpr uint32_t INDEX_THREE = 3;
constexpr uint32_t SECONDARY_INIT_PARAM_SIZE = 5;
#define ONLY_FOR_SECONDARY_DISPLAY_FOLD if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {return;}
}
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}

class SecondaryDisplayFoldPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SecondaryDisplayFoldPolicyTest::SetUpTestCase()
{
}

void SecondaryDisplayFoldPolicyTest::TearDownTestCase()
{
}

void SecondaryDisplayFoldPolicyTest::SetUp()
{
}

void SecondaryDisplayFoldPolicyTest::TearDown()
{
    g_errLog.clear();
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: ChangeScreenDisplayMode
 * @tc.desc: test function : ChangeScreenDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, ChangeScreenDisplayMode, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.ChangeScreenDisplayMode(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::MAIN;
    policy.ChangeScreenDisplayMode(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::FULL;
    policy.ChangeScreenDisplayMode(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    displayMode = FoldDisplayMode::GLOBAL_FULL;
    policy.ChangeScreenDisplayMode(displayMode, reason);
    EXPECT_FALSE(policy.onBootAnimation_);

    ScreenId screenId = 0;
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (nullptr == screenSession) {
        return;
    }
    displayMode = FoldDisplayMode::MAIN;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "0");

    displayMode = FoldDisplayMode::GLOBAL_FULL;
    policy.ChangeScreenDisplayMode(displayMode);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "4");
}

/**
 * @tc.name: SendSensorResult
 * @tc.desc: test function : SendSensorResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendSensorResult, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    policy.SendSensorResult(foldStatus);
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetOnBootAnimation
 * @tc.desc: test function : SetOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SetOnBootAnimation, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

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
HWTEST_F(SecondaryDisplayFoldPolicyTest, RecoverWhenBootAnimationExit, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::FULL;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);

    policy.currentDisplayMode_ = FoldDisplayMode::MAIN;
    policy.RecoverWhenBootAnimationExit();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: GetModeMatchStatus
 * @tc.desc: test function : GetModeMatchStatus
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, GetModeMatchStatus, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
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

    policy.currentFoldStatus_ = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::GLOBAL_FULL, displayMode);

    policy.currentFoldStatus_ = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::GLOBAL_FULL, displayMode);

    policy.currentFoldStatus_ = FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::MAIN, displayMode);

    policy.currentFoldStatus_ = FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::MAIN, displayMode);

    policy.currentFoldStatus_ = FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::GLOBAL_FULL, displayMode);

    policy.currentFoldStatus_ = FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED;
    displayMode = policy.GetModeMatchStatus();
    EXPECT_EQ(FoldDisplayMode::GLOBAL_FULL, displayMode);
}

/**
 * @tc.name: ChangeSuperScreenDisplayMode01
 * @tc.desc: test function : ChangeSuperScreenDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, ChangeSuperScreenDisplayMode01, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;

    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::MAIN;
    policy.ChangeSuperScreenDisplayMode(screenSession, displayMode);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t currentWidth = screenProperty.GetBounds().rect_.GetWidth();
    uint32_t currentHeight = screenProperty.GetBounds().rect_.GetHeight();
    EXPECT_EQ(currentWidth, secondaryDisplayParams[0]);
    EXPECT_EQ(currentHeight, secondaryDisplayParams[INDEX_THREE]);

    displayMode = FoldDisplayMode::FULL;
    policy.ChangeSuperScreenDisplayMode(screenSession, displayMode);
    screenProperty = screenSession->GetScreenProperty();
    currentWidth = screenProperty.GetBounds().rect_.GetWidth();
    currentHeight = screenProperty.GetBounds().rect_.GetHeight();
    EXPECT_EQ(currentWidth, secondaryDisplayParams[1]);
    EXPECT_EQ(currentHeight, secondaryDisplayParams[INDEX_THREE]);

    displayMode = FoldDisplayMode::GLOBAL_FULL;
    policy.ChangeSuperScreenDisplayMode(screenSession, displayMode);
    screenProperty = screenSession->GetScreenProperty();
    currentWidth = screenProperty.GetBounds().rect_.GetWidth();
    currentHeight = screenProperty.GetBounds().rect_.GetHeight();
    EXPECT_EQ(currentWidth, secondaryDisplayParams[INDEX_TWO]);
    EXPECT_EQ(currentHeight, secondaryDisplayParams[INDEX_THREE]);
    usleep(200000);
}

/**
 * @tc.name: SendPropertyChangeResult01
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendPropertyChangeResult01, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::MAIN;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t currentWidth = screenProperty.GetBounds().rect_.GetWidth();
    uint32_t currentHeight = screenProperty.GetBounds().rect_.GetHeight();
    EXPECT_EQ(currentWidth, secondaryDisplayParams[0]);
    EXPECT_EQ(currentHeight, secondaryDisplayParams[INDEX_THREE]);
    usleep(200000);
}

/**
 * @tc.name: SendPropertyChangeResult02
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendPropertyChangeResult02, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::FULL;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t currentWidth = screenProperty.GetBounds().rect_.GetWidth();
    uint32_t currentHeight = screenProperty.GetBounds().rect_.GetHeight();
    EXPECT_EQ(currentWidth, secondaryDisplayParams[1]);
    EXPECT_EQ(currentHeight, secondaryDisplayParams[INDEX_THREE]);
    usleep(200000);
}

/**
 * @tc.name: SendPropertyChangeResult03
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendPropertyChangeResult03, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::GLOBAL_FULL;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t currentWidth = screenProperty.GetBounds().rect_.GetWidth();
    uint32_t currentHeight = screenProperty.GetBounds().rect_.GetHeight();
    EXPECT_EQ(currentWidth, secondaryDisplayParams[INDEX_TWO]);
    EXPECT_EQ(currentHeight, secondaryDisplayParams[INDEX_THREE]);
    usleep(200000);
}

/**
 * @tc.name: SendPropertyChangeResult04
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendPropertyChangeResult04, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    EXPECT_TRUE(g_errLog.find("unKnown displayMode") != std::string::npos);
    usleep(200000);
}

/**
 * @tc.name: SendPropertyChangeResult05
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendPropertyChangeResult05, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = nullptr;
    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::GLOBAL_FULL;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    EXPECT_TRUE(g_errLog.find("screenSession is null") != std::string::npos);
    usleep(200000);
}

/**
 * @tc.name: SendPropertyChangeResult06
 * @tc.desc: test function : SendPropertyChangeResult
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SendPropertyChangeResult06, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = std::make_shared<TaskScheduler>("test");
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    sptr<ScreenSession> screenSession = new ScreenSession;
    std::vector<uint32_t> secondaryDisplayParams = policy.GetScreenParams();
    EXPECT_EQ(secondaryDisplayParams.size(), SECONDARY_INIT_PARAM_SIZE);

    FoldDisplayMode displayMode = FoldDisplayMode::FULL;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    EXPECT_EQ(screenSession->GetScreenAreaOffsetY(), secondaryDisplayParams[4]);
    EXPECT_EQ(screenSession->GetScreenAreaHeight(), secondaryDisplayParams[1]);

    displayMode = FoldDisplayMode::MAIN;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    EXPECT_EQ(screenSession->GetScreenAreaOffsetY(), 0);
    EXPECT_EQ(screenSession->GetScreenAreaHeight(), secondaryDisplayParams[0]);

    displayMode = FoldDisplayMode::GLOBAL_FULL;
    policy.SendPropertyChangeResult(screenSession, 0, ScreenPropertyChangeReason::UNDEFINED, displayMode);
    EXPECT_EQ(screenSession->GetScreenAreaOffsetY(), 0);
    EXPECT_EQ(screenSession->GetScreenAreaHeight(), secondaryDisplayParams[2]);
}

/**
 * @tc.name: SetStatusFullActiveRectAndTpFeature
 * @tc.desc: test function : SetStatusFullActiveRectAndTpFeature
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SetStatusFullActiveRectAndTpFeature, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    ScreenProperty property = policy.screenProperty_;

    policy.SetStatusFullActiveRectAndTpFeature();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetStatusMainActiveRectAndTpFeature
 * @tc.desc: test function : SetStatusMainActiveRectAndTpFeature
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SetStatusMainActiveRectAndTpFeature, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    ScreenProperty property = policy.screenProperty_;

    policy.SetStatusMainActiveRectAndTpFeature();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetStatusGlobalFullActiveRectAndTpFeature
 * @tc.desc: test function : SetStatusGlobalFullActiveRectAndTpFeature
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SetStatusGlobalFullActiveRectAndTpFeature, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);
    ScreenProperty property = policy.screenProperty_;

    policy.SetStatusGlobalFullActiveRectAndTpFeature();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: ReportFoldDisplayModeChange
 * @tc.desc: test function : ReportFoldDisplayModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, ReportFoldDisplayModeChange, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    policy.ReportFoldDisplayModeChange(displayMode);
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, displayMode);

    displayMode = FoldDisplayMode::FULL;
    policy.ReportFoldDisplayModeChange(displayMode);
    EXPECT_NE(FoldDisplayMode::UNKNOWN, displayMode);
}

/**
 * @tc.name: GetCurrentFoldCreaseRegion
 * @tc.desc: test function : GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    sptr<FoldCreaseRegion> foldCreaseRegion;
    foldCreaseRegion = policy.GetCurrentFoldCreaseRegion();
    EXPECT_EQ(policy.currentFoldCreaseRegion_, foldCreaseRegion);
}

/**
 * @tc.name: InitScreenParams
 * @tc.desc: test function : InitScreenParams
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, InitScreenParams, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.InitScreenParams();
    EXPECT_FALSE(policy.onBootAnimation_);
}

/**
 * @tc.name: SetSecondaryDisplayModeChangeStatus01
 * @tc.desc: test function : SetSecondaryDisplayModeChangeStatus
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SetSecondaryDisplayModeChangeStatus01, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.SetSecondaryDisplayModeChangeStatus(true);
    EXPECT_EQ(policy.secondaryPengdingTask_, 2);

    policy.SetSecondaryDisplayModeChangeStatus(false);
    EXPECT_EQ(policy.secondaryPengdingTask_, 1);

    policy.SetSecondaryDisplayModeChangeStatus(false);
    EXPECT_EQ(policy.secondaryPengdingTask_, 0);
}

/**
 * @tc.name: GetScreenSnapshotRect
 * @tc.desc: test function : GetScreenSnapshotRect
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, GetScreenSnapshotRect, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    Drawing::Rect snapshotRect = {0, 0, 0, 0};
    Drawing::Rect targetSnapshotRect = policy.GetScreenSnapshotRect();
    EXPECT_EQ(snapshotRect, targetSnapshotRect);

    policy.currentDisplayMode_ = FoldDisplayMode::MAIN;
    snapshotRect = {0, 0, 2232, 1008};
    targetSnapshotRect = policy.GetScreenSnapshotRect();
    EXPECT_EQ(snapshotRect, targetSnapshotRect);

    policy.currentDisplayMode_ = FoldDisplayMode::FULL;
    snapshotRect = {0, 1136, 2232, 3184};
    targetSnapshotRect = policy.GetScreenSnapshotRect();
    EXPECT_EQ(snapshotRect, targetSnapshotRect);

    policy.currentDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    snapshotRect = {0, 0, 2232, 3184};
    targetSnapshotRect = policy.GetScreenSnapshotRect();
    EXPECT_EQ(snapshotRect, targetSnapshotRect);
}

/**
 * @tc.name: SetMainScreenRegion
 * @tc.desc: test function : SetMainScreenRegion
 * @tc.type: FUNC
 */
HWTEST_F(SecondaryDisplayFoldPolicyTest, SetMainScreenRegion, TestSize.Level1)
{
    ONLY_FOR_SECONDARY_DISPLAY_FOLD
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    SecondaryDisplayFoldPolicy policy(displayInfoMutex, screenPowerTaskScheduler);

    policy.currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    DMRect mainScreenRegion = {0, 0, 0, 0};
    DMRect targetMainScreenRegion = {0, 0, 0, 0};
    policy.SetMainScreenRegion(mainScreenRegion);
    EXPECT_EQ(mainScreenRegion, targetMainScreenRegion);

    policy.currentDisplayMode_ = FoldDisplayMode::MAIN;
    mainScreenRegion = {0, 0, 0, 0};
    targetMainScreenRegion = {0, 0, 2232, 1008};
    policy.SetMainScreenRegion(mainScreenRegion);
    EXPECT_EQ(mainScreenRegion, targetMainScreenRegion);

    policy.currentDisplayMode_ = FoldDisplayMode::FULL;
    mainScreenRegion = {0, 0, 0, 0};
    targetMainScreenRegion = {0, 1136, 2232, 2048};
    policy.SetMainScreenRegion(mainScreenRegion);
    EXPECT_EQ(mainScreenRegion, targetMainScreenRegion);

    policy.currentDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    mainScreenRegion = {0, 0, 0, 0};
    targetMainScreenRegion = {0, 0, 2232, 3184};
    policy.SetMainScreenRegion(mainScreenRegion);
    EXPECT_EQ(mainScreenRegion, targetMainScreenRegion);
}
}
} // namespace Rosen
} // namespace OHOS