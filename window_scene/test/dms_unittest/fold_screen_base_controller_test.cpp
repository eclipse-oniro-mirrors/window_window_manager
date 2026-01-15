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
#include <hisysevent.h>
#include <parameters.h>
#include "gmock/gmock.h"
#include <functional>
#include "window_manager_hilog.h"

#define private public
#define protected public
#include "fold_screen_base_policy.h"
#include "fold_screen_base_controller.h"
#include "screen_session_manager.h"
#include "product_config.h"
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

class MockProductConfig : public ProductConfig {
public:
    MOCK_METHOD(bool, IsSingleDisplaySuperFoldDevice, (), (override));
    MOCK_METHOD(bool, IsSecondaryDisplayFoldDevice, (), (override));
};

class FoldScreenBaseControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

ScreenSessionManager* ssm_;
void FoldScreenBaseControllerTest::SetUpTestCase()
{
    ssm_ = &ScreenSessionManager::GetInstance();
}

void FoldScreenBaseControllerTest::TearDownTestCase() {}

void FoldScreenBaseControllerTest::SetUp() {}

void FoldScreenBaseControllerTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: GetFoldCreaseRegionJson
 * @tc.desc: test function : GetFoldCreaseRegionJson
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetFoldCreaseRegionJson, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    nlohmann::ordered_json creaseJson = controller.GetFoldCreaseRegionJson();
    std::string creaseJsonString = creaseJson.dump();

    EXPECT_NE(creaseJsonString.length(), 0);
}

/**
 * @tc.name: SetDisplayModeTest
 * @tc.desc: test function : SetDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, SetDisplayModeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;
    FoldScreenBasePolicy::GetInstance().isClearingBootAnimation_ = true;
    auto controller = FoldScreenBaseController();

    controller.SetDisplayMode(FoldDisplayMode::FULL);
    EXPECT_TRUE(g_logMsg.find("clearing bootAnimation not change displayMode") != std::string::npos);

    g_logMsg.clear();
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(false));
    controller.SetDisplayMode(FoldDisplayMode::FULL);
    EXPECT_TRUE(g_logMsg.find("clearing bootAnimation not change displayMode") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: RecoverDisplayModeTest
 * @tc.desc: test function : RecoverDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, RecoverDisplayModeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;
    FoldScreenBasePolicy::GetInstance().currentFoldStatus_ = FoldStatus::EXPAND;
    FoldScreenBasePolicy::GetInstance().lastDisplayMode_ = FoldDisplayMode::FULL;
    auto controller = FoldScreenBaseController();

    controller.RecoverDisplayMode();
    EXPECT_TRUE(g_logMsg.find("current displayMode is correct, skip") != std::string::npos);

    g_logMsg.clear();
    FoldScreenBasePolicy::GetInstance().lastDisplayMode_ = FoldDisplayMode::MAIN;
    controller.RecoverDisplayMode();
    EXPECT_TRUE(g_logMsg.find("2 -> 1") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: LockDisplayStatusTest
 * @tc.desc: test function : LockDisplayStatus
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, LockDisplayStatusTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    controller.LockDisplayStatus(true);
    EXPECT_TRUE(FoldScreenBasePolicy::GetInstance().lockDisplayStatus_);
}

/**
 * @tc.name: BootAnimationFinishPowerInitTest
 * @tc.desc: test function : BootAnimationFinishPowerInit
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, BootAnimationFinishPowerInitTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    controller.BootAnimationFinishPowerInit();
    EXPECT_EQ(FoldScreenBasePolicy::GetInstance().currentDisplayMode_, FoldDisplayMode::UNKNOWN);
}

/**
 * @tc.name: SetdisplayModeChangeStatusTest
 * @tc.desc: test function : SetdisplayModeChangeStatus
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, SetdisplayModeChangeStatusTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;
    auto controller = FoldScreenBaseController();
    controller.SetdisplayModeChangeStatus(false);

    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(false));
    controller.SetdisplayModeChangeStatus(true);
    EXPECT_TRUE(FoldScreenBasePolicy::GetInstance().displayModeChangeRunning_);

    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: GetDisplayModeTest
 * @tc.desc: test function : GetDisplayMode, get default display mode
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetDisplayModeTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    FoldScreenBasePolicy::GetInstance().lastDisplayMode_ = FoldDisplayMode::UNKNOWN;

    auto displayMode = controller.GetDisplayMode();
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, displayMode);
}

/**
 * @tc.name: GetFoldStatusTest
 * @tc.desc: test function : GetFoldStatus, get default display mode
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetFoldStatusTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    FoldStatus foldStatus = controller.GetFoldStatus();
    EXPECT_EQ(FoldStatus::UNKNOWN, foldStatus);
}

/**
 * @tc.name: GetIsFirstFrameCommitReportedTest
 * @tc.desc: test function : GetIsFirstFrameCommitReported
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetIsFirstFrameCommitReportedTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    EXPECT_EQ(false, controller.GetIsFirstFrameCommitReported());

    controller.SetIsFirstFrameCommitReported(true);
    EXPECT_EQ(true, controller.GetIsFirstFrameCommitReported());
}

/**
 * @tc.name: SetFoldStatusTest
 * @tc.desc: test function : SetFoldStatus, set fold status to expand
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, SetFoldStatusTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    controller.SetFoldStatus(FoldStatus::EXPAND);
    EXPECT_EQ(FoldStatus::EXPAND, FoldScreenBasePolicy::GetInstance().currentFoldStatus_);
    EXPECT_EQ(FoldStatus::EXPAND, FoldScreenBasePolicy::GetInstance().lastFoldStatus_);
}

/**
 * @tc.name: GetCurrentScreenIdTest
 * @tc.desc: test function : GetCurrentScreenId, get default screen id
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetCurrentScreenIdTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    ScreenId id = controller.GetCurrentScreenId();
    EXPECT_EQ(-1, id);
}

/**
 * @tc.name: GetScreenSnapshotRectTest
 * @tc.desc: test function : GetScreenSnapshotRect
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetScreenSnapshotRectTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    Drawing::Rect expectRect = {0, 0, 0, 0};
    Drawing::Rect getRect = controller.GetScreenSnapshotRect();
    EXPECT_EQ(expectRect, getRect);
}

/**
 * @tc.name: GetStartTimePointTest
 * @tc.desc: test function : GetStartTimePoint, get default time point
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, GetStartTimePointTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    auto time = controller.GetStartTimePoint();
    EXPECT_TRUE(time.time_since_epoch().count() > 0);
}

/**
 * @tc.name: ExitCoordinationTest
 * @tc.desc: test function : ExitCoordination
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, ExitCoordinationTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldScreenBaseController();

    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    controller.ExitCoordination();
    EXPECT_TRUE(g_logMsg.find("ExitCoordination skipped, current coordination flag is false") != std::string::npos);

    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    controller.ExitCoordination();
    EXPECT_TRUE(!ScreenSessionManager::GetInstance().GetCoordinationFlag());
}

/**
 * @tc.name: AddOrRemoveDisplayNodeToTreeTest
 * @tc.desc: test function : AddOrRemoveDisplayNodeToTree
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, AddOrRemoveDisplayNodeToTreeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldScreenBaseController();

    ScreenSessionManager::GetInstance().screenSessionMap_[10] = sptr<ScreenSession>::MakeSptr();
    controller.AddOrRemoveDisplayNodeToTree(10, 0);
    EXPECT_TRUE(g_logMsg.find("displayNode is null") != std::string::npos);
    ScreenSessionManager::GetInstance().screenSessionMap_.erase(10);
}

/**
 * @tc.name: SetOnBootAnimationTest
 * @tc.desc: test function : SetOnBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, SetOnBootAnimationTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldScreenBaseController();

    controller.SetOnBootAnimation(false);
    EXPECT_TRUE(g_logMsg.find("SetOnBootAnimation when boot animation finished, change display mode") !=
                std::string::npos);
}

/**
 * @tc.name: UpdateForPhyScreenPropertyChangeTest
 * @tc.desc: test function : UpdateForPhyScreenPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, UpdateForPhyScreenPropertyChangeTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto controller = FoldScreenBaseController();
    FoldScreenBasePolicy::GetInstance().currentFoldStatus_ = FoldStatus::EXPAND;
    FoldScreenBasePolicy::GetInstance().currentDisplayMode_ = FoldDisplayMode::MAIN;

    controller.UpdateForPhyScreenPropertyChange();
    EXPECT_TRUE(g_logMsg.find("clearing bootAnimation not change displayMode") != std::string::npos);
}

/**
 * @tc.name: SetIsClearingBootAnimationTest
 * @tc.desc: test function : SetIsClearingBootAnimation
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, SetIsClearingBootAnimationTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();

    controller.SetIsClearingBootAnimation(true);
    EXPECT_TRUE(FoldScreenBasePolicy::GetInstance().isClearingBootAnimation_);
}

/**
 * @tc.name: NotifyRunSensorFoldStateManagerTest
 * @tc.desc: test function : NotifyRunSensorFoldStateManager
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenBaseControllerTest, NotifyRunSensorFoldStateManagerTest, TestSize.Level1)
{
    auto controller = FoldScreenBaseController();
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    controller.NotifyRunSensorFoldStateManager();
    EXPECT_TRUE(g_logMsg.find("TaskSequenceProcess") != std::string::npos);
    LOG_SetCallback(nullptr);
    g_logMsg.clear();
}
} // namespace
} // namespace DMS
} // namespace Rosen
} // namespace OHOS