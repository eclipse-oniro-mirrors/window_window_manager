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

#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "fold_screen_state_internel.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}

class SensorFoldStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SensorFoldStateManagerTest::SetUpTestCase()
{
}

void SensorFoldStateManagerTest::TearDownTestCase()
{
}

void SensorFoldStateManagerTest::SetUp()
{
}

void SensorFoldStateManagerTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: HandleSensorChange
 * @tc.desc: HandleSensorChange
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, HandleSensorChange, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SensorFoldStateManager mgr = SensorFoldStateManager();
    FoldStatus nextState = FoldStatus::UNKNOWN;
    float angle = 0.0f;
    sptr<FoldScreenPolicy> foldScreenPolicy = sptr<FoldScreenPolicy>::MakeSptr();
    mgr.HandleSensorChange(nextState, angle, foldScreenPolicy);
    EXPECT_TRUE(g_errLog.find("fold state is UNKNOWN") != std::string::npos);

    mgr.mState_ = FoldStatus::EXPAND;
    mgr.HandleSensorChange(nextState, angle, foldScreenPolicy);
    EXPECT_FALSE(g_errLog.find("current state: %{public}d, next state: %{public}d.") != std::string::npos);

    nextState = FoldStatus::EXPAND;
    mgr.HandleSensorChange(nextState, angle, foldScreenPolicy);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandleSensorChange1
 * @tc.desc: HandleSensorChange1
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, HandleSensorChange1, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SensorFoldStateManager mgr = SensorFoldStateManager();
    mgr.isInOneStep_ = true;
    mgr.mState_ = FoldStatus::FOLDED;
    FoldStatus nextState = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
    std::vector<float> angles = {0.0f, 0.0f, 0.0f};
    std::vector<uint16_t> halls = {0, 0};
    sptr<FoldScreenPolicy> foldScreenPolicy = sptr<FoldScreenPolicy>::MakeSptr();
    mgr.HandleSensorChange(nextState, angles, halls, foldScreenPolicy);
    EXPECT_TRUE(g_errLog.find("fold state is UNKNOWN") == std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandleSensorChange2
 * @tc.desc: HandleSensorChange2
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, HandleSensorChange2, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SensorFoldStateManager mgr = SensorFoldStateManager();
    mgr.isInOneStep_ = true;
    mgr.mState_ = FoldStatus::FOLDED;
    FoldStatus nextState = FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND;
    std::vector<float> angles = {0.0f, 0.0f, 0.0f};
    std::vector<uint16_t> halls = {0, 0};
    sptr<FoldScreenPolicy> foldScreenPolicy = sptr<FoldScreenPolicy>::MakeSptr();
    mgr.HandleSensorChange(nextState, angles, halls, foldScreenPolicy);
    EXPECT_TRUE(g_errLog.find("fold state is UNKNOWN") == std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandleSensorChange3
 * @tc.desc: HandleSensorChange3
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, HandleSensorChange3, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SensorFoldStateManager mgr = SensorFoldStateManager();
    mgr.isInOneStep_ = false;
    mgr.mState_ = FoldStatus::FOLDED;
    FoldStatus nextState = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
    std::vector<float> angles = {0.0f, 0.0f, 0.0f};
    std::vector<uint16_t> halls = {0, 0};
    sptr<FoldScreenPolicy> foldScreenPolicy = sptr<FoldScreenPolicy>::MakeSptr();
    mgr.HandleSensorChange(nextState, angles, halls, foldScreenPolicy);
    EXPECT_TRUE(g_errLog.find("fold state is UNKNOWN") == std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandleSensorChange5
 * @tc.desc: HandleSensorChange5
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, HandleSensorChange5, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SensorFoldStateManager mgr = SensorFoldStateManager();
    FoldStatus nextState = FoldStatus::FOLDED;
    float angle = 0.0f;
    sptr<FoldScreenPolicy> foldScreenPolicy = sptr<FoldScreenPolicy>::MakeSptr();
    auto wasFoldStatus = mgr.GetCurrentState();
    std::atomic<bool> wasPhysicalFoldLockFlag = foldScreenPolicy->GetPhysicalFoldLockFlag();
    foldScreenPolicy->SetFoldLockFlagAndFoldStatus(true, FoldStatus::FOLDED);
    mgr.HandleSensorChange(nextState, angle, foldScreenPolicy);
    EXPECT_TRUE(g_errLog.find("Fold status is locked") != std::string::npos);
    foldScreenPolicy->SetFoldLockFlagAndFoldStatus(wasPhysicalFoldLockFlag, wasFoldStatus);
}

/**
 * @tc.name: ReportNotifyFoldStatusChange
 * @tc.desc: ReportNotifyFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, ReportNotifyFoldStatusChange, TestSize.Level1)
{
    SensorFoldStateManager mgr = SensorFoldStateManager();
    int32_t currentStatus = 0;
    int32_t nextStatus = 1;
    float postureAngle = 0.0f;
    mgr.ReportNotifyFoldStatusChange(currentStatus, nextStatus, postureAngle);
    ASSERT_EQ(mgr.GetCurrentState(), FoldStatus::UNKNOWN);
}

/**
 * @tc.name: ClearState
 * @tc.desc: ClearState
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, ClearState, TestSize.Level1)
{
    SensorFoldStateManager mgr = SensorFoldStateManager();
    sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
    mgr.ClearState(foldScreenPolicy);
    ASSERT_EQ(mgr.GetCurrentState(), FoldStatus::UNKNOWN);
}

/**
 * @tc.name: NotifyReportFoldStatusToScb
 * @tc.desc: NotifyReportFoldStatusToScb
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, NotifyReportFoldStatusToScb, TestSize.Level1)
{
    SensorFoldStateManager mgr = SensorFoldStateManager();
    FoldStatus currentStatus = FoldStatus::UNKNOWN;
    FoldStatus nextStatus = FoldStatus::EXPAND;
    float postureAngle = 0.0f;
    mgr.NotifyReportFoldStatusToScb(currentStatus, nextStatus, postureAngle);
    ASSERT_EQ(mgr.GetCurrentState(), FoldStatus::UNKNOWN);
}

/**
 * @tc.name: IsTentMode
 * @tc.desc: IsTentMode
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, IsTentMode, TestSize.Level1)
{
    SensorFoldStateManager mgr = SensorFoldStateManager();
    mgr.tentModeType_ = 1;
    bool ret = mgr.IsTentMode();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: SetTentMode
 * @tc.desc: SetTentMode
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, SetTentMode, TestSize.Level1)
{
    SensorFoldStateManager mgr = SensorFoldStateManager();
    mgr.tentModeType_ = 0;
    mgr.SetTentMode(0);
    bool ret = mgr.IsTentMode();
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: NotifyRunTaskSequence
 * @tc.desc: NotifyRunTaskSequence
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateManagerTest, FinishTaskSequence01, TestSize.Level0)
{
    SensorFoldStateManager mgr = SensorFoldStateManager();
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    mgr.FinishTaskSequence();
    EXPECT_TRUE(g_errLog.find("TaskSequenceProcess") != std::string::npos);
    LOG_SetCallback(nullptr);
    g_errLog.clear();
}
}
} // namespace Rosen
} // namespace OHOS