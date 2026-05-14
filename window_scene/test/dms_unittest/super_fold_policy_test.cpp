/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "screen_session_manager/include/fold_screen_controller/super_fold_policy.h"
#include "../mock/mock_accesstoken_kit.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
constexpr ScreenId SCREEN_ID_FULL = 0;
constexpr ScreenId SCREEN_ID_MAIN = 5;
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    g_logMsg += msg;
}
}
class SuperFoldPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SuperFoldPolicyTest::SetUpTestCase()
{
}

void SuperFoldPolicyTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void SuperFoldPolicyTest::SetUp()
{
    g_logMsg.clear();
}

void SuperFoldPolicyTest::TearDown()
{
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

namespace {
HWTEST_F(SuperFoldPolicyTest, SetOnBootAnimation01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    bool onBootAnimation = true;
    SuperFoldPolicy::GetInstance().SetOnBootAnimation(onBootAnimation);
    EXPECT_TRUE(g_logMsg.find("onBootAnimation: 1") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, SetOnBootAnimation02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    bool onBootAnimation = false;
    SuperFoldPolicy::GetInstance().SetOnBootAnimation(onBootAnimation);
    EXPECT_TRUE(g_logMsg.find("onBootAnimation: 0") != std::string::npos);
    EXPECT_TRUE(g_logMsg.find("when boot animation finished") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, BootAnimationFinishPowerInit01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    SuperFoldPolicy::GetInstance().BootAnimationFinishPowerInit();
    EXPECT_TRUE(g_logMsg.find("Fold Screen Power main screen off") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, BootAnimationFinishPowerInit02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::CLOSE);
    SuperFoldPolicy::GetInstance().BootAnimationFinishPowerInit();
    EXPECT_TRUE(g_logMsg.find("Fold Screen Power all screen off") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, BootAnimationFinishPowerInit03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::UNKNOWN);
    SuperFoldPolicy::GetInstance().BootAnimationFinishPowerInit();
    EXPECT_TRUE(g_logMsg.find("invalid active screen id") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, RecoverDisplayMode01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::MAIN);
    SuperFoldPolicy::GetInstance().RecoverDisplayMode();
    EXPECT_TRUE(g_logMsg.find("recover displayMode") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, RecoverDisplayMode02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    SuperFoldPolicy::GetInstance().RecoverDisplayMode();
    EXPECT_FALSE(g_logMsg.find("recover displayMode") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, RecoverDisplayMode03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::UNKNOWN);
    SuperFoldPolicy::GetInstance().RecoverDisplayMode();
    EXPECT_FALSE(g_logMsg.find("recover displayMode") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, GetModeMatchStatus01, TestSize.Level1)
{
    ScreenClosedState state = ScreenClosedState::CLOSE;
    FoldDisplayMode mode = SuperFoldPolicy::GetInstance().GetModeMatchStatus(state);
    EXPECT_EQ(mode, FoldDisplayMode::MAIN);
}

HWTEST_F(SuperFoldPolicyTest, GetModeMatchStatus02, TestSize.Level1)
{
    ScreenClosedState state = ScreenClosedState::OPEN;
    FoldDisplayMode mode = SuperFoldPolicy::GetInstance().GetModeMatchStatus(state);
    EXPECT_EQ(mode, FoldDisplayMode::FULL);
}

HWTEST_F(SuperFoldPolicyTest, GetModeMatchStatus03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenClosedState state = ScreenClosedState::UNKNOWN;
    FoldDisplayMode mode = SuperFoldPolicy::GetInstance().GetModeMatchStatus(state);
    EXPECT_EQ(mode, FoldDisplayMode::UNKNOWN);
    EXPECT_TRUE(g_logMsg.find("invalid screenClosedState") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, GetScreenClosedState01, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    ScreenClosedState state = SuperFoldPolicy::GetInstance().GetScreenClosedState();
    EXPECT_EQ(state, ScreenClosedState::OPEN);
}

HWTEST_F(SuperFoldPolicyTest, GetScreenClosedState02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::CLOSE);
    ScreenClosedState state = SuperFoldPolicy::GetInstance().GetScreenClosedState();
    EXPECT_EQ(state, ScreenClosedState::CLOSE);
}

HWTEST_F(SuperFoldPolicyTest, GetCurrentScreenId01, TestSize.Level1)
{
    ScreenId screenId = SuperFoldPolicy::GetInstance().GetCurrentScreenId();
}

HWTEST_F(SuperFoldPolicyTest, SetCurrentScreenId01, TestSize.Level1)
{
    ScreenId screenId = 10;
    SuperFoldPolicy::GetInstance().SetCurrentScreenId(screenId);
    EXPECT_EQ(SuperFoldPolicy::GetInstance().GetCurrentScreenId(), screenId);
}

HWTEST_F(SuperFoldPolicyTest, SetLastCacheDisplayMode01, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::FULL;
    SuperFoldPolicy::GetInstance().SetLastCacheDisplayMode(mode);
    EXPECT_EQ(SuperFoldPolicy::GetInstance().lastCacheDisplayMode_.load(), mode);
}

HWTEST_F(SuperFoldPolicyTest, GetCurrentDisplayMode01, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    FoldDisplayMode mode = SuperFoldPolicy::GetInstance().GetCurrentDisplayMode();
    EXPECT_EQ(mode, FoldDisplayMode::FULL);
}

HWTEST_F(SuperFoldPolicyTest, SetCurrentDisplayMode01, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    SuperFoldPolicy::GetInstance().SetCurrentDisplayMode(mode);
    EXPECT_EQ(SuperFoldPolicy::GetInstance().GetCurrentDisplayMode(), mode);
}

HWTEST_F(SuperFoldPolicyTest, LockDisplayMode01, TestSize.Level1)
{
    bool isLock = true;
    SuperFoldPolicy::GetInstance().LockDisplayMode(isLock);
    EXPECT_TRUE(SuperFoldPolicy::GetInstance().isLockDisplayMode_.load());
}

HWTEST_F(SuperFoldPolicyTest, LockDisplayMode02, TestSize.Level1)
{
    bool isLock = false;
    SuperFoldPolicy::GetInstance().LockDisplayMode(isLock);
    EXPECT_FALSE(SuperFoldPolicy::GetInstance().isLockDisplayMode_.load());
}

HWTEST_F(SuperFoldPolicyTest, SetScreenSwitchState01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    ScreenClosedState state = ScreenClosedState::OPEN;
    bool isScreenOn = true;
    DMError ret = SuperFoldPolicy::GetInstance().SetScreenSwitchState(state, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
    EXPECT_TRUE(g_logMsg.find("current screenClosedState aready is") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, SetScreenSwitchState02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    ScreenClosedState state = ScreenClosedState::UNKNOWN;
    bool isScreenOn = true;
    DMError ret = SuperFoldPolicy::GetInstance().SetScreenSwitchState(state, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
}

HWTEST_F(SuperFoldPolicyTest, SetScreenSwitchState03, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    ScreenClosedState state = ScreenClosedState::CLOSE;
    bool isScreenOn = false;
    DMError ret = SuperFoldPolicy::GetInstance().SetScreenSwitchState(state, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
}

HWTEST_F(SuperFoldPolicyTest, SetScreenSwitchState04, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::CLOSE);
    ScreenClosedState state = ScreenClosedState::OPEN;
    bool isScreenOn = true;
    DMError ret = SuperFoldPolicy::GetInstance().SetScreenSwitchState(state, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
}

HWTEST_F(SuperFoldPolicyTest, CheckDisplayMode01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    FoldDisplayMode mode = FoldDisplayMode::FULL;
    bool ret = SuperFoldPolicy::GetInstance().CheckDisplayMode(mode);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("already in displayMode") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, CheckDisplayMode02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(true);
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    bool ret = SuperFoldPolicy::GetInstance().CheckDisplayMode(mode);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("onBootAnimation can not change mode") != std::string::npos);
    g_logMsg.clear();
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(false);
}

HWTEST_F(SuperFoldPolicyTest, CheckDisplayMode03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(true);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    bool ret = SuperFoldPolicy::GetInstance().CheckDisplayMode(mode);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("displayMode is locked") != std::string::npos);
    g_logMsg.clear();
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(false);
}

HWTEST_F(SuperFoldPolicyTest, CheckDisplayMode04, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(true);
    SuperFoldPolicy::GetInstance().startTimePoint_ = std::chrono::steady_clock::now();
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    bool ret = SuperFoldPolicy::GetInstance().CheckDisplayMode(mode);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("last process not complete") != std::string::npos);
    g_logMsg.clear();
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
}

HWTEST_F(SuperFoldPolicyTest, CheckDisplayMode05, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(false);
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(false);
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    bool ret = SuperFoldPolicy::GetInstance().CheckDisplayMode(mode);
    EXPECT_TRUE(ret);
}

HWTEST_F(SuperFoldPolicyTest, GetModeChangeRunningStatus01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().startTimePoint_ = std::chrono::steady_clock::now() -
        std::chrono::milliseconds(3000);
    bool ret = SuperFoldPolicy::GetInstance().GetModeChangeRunningStatus();
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("mode change timeout") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, GetModeChangeRunningStatus02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().startTimePoint_ = std::chrono::steady_clock::now();
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    bool ret = SuperFoldPolicy::GetInstance().GetModeChangeRunningStatus();
    EXPECT_FALSE(ret);
}

HWTEST_F(SuperFoldPolicyTest, GetdisplayModeRunningStatus01, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(true);
    bool ret = SuperFoldPolicy::GetInstance().GetdisplayModeRunningStatus();
    EXPECT_TRUE(ret);
}

HWTEST_F(SuperFoldPolicyTest, GetdisplayModeRunningStatus02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    bool ret = SuperFoldPolicy::GetInstance().GetdisplayModeRunningStatus();
    EXPECT_FALSE(ret);
}

HWTEST_F(SuperFoldPolicyTest, SwitchScreenAndSetScreenPower01, TestSize.Level1)
{
    ScreenId screenId = SCREEN_ID_MAIN;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().SetCurrentScreenId(SCREEN_ID_FULL);
    SuperFoldPolicy::GetInstance().SwitchScreenAndSetScreenPower(screenId, isScreenOn);
    EXPECT_EQ(SuperFoldPolicy::GetInstance().GetCurrentScreenId(), screenId);
    usleep(SLEEP_TIME_IN_US);
}

HWTEST_F(SuperFoldPolicyTest, SwitchScreenAndSetScreenPower02, TestSize.Level1)
{
    ScreenId screenId = SCREEN_ID_FULL;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().SetCurrentScreenId(SCREEN_ID_MAIN);
    SuperFoldPolicy::GetInstance().SwitchScreenAndSetScreenPower(screenId, isScreenOn);
    EXPECT_EQ(SuperFoldPolicy::GetInstance().GetCurrentScreenId(), screenId);
    usleep(SLEEP_TIME_IN_US);
}

HWTEST_F(SuperFoldPolicyTest, SwitchScreenAndSetScreenPower03, TestSize.Level1)
{
    ScreenId screenId = SCREEN_ID_MAIN;
    bool isScreenOn = false;
    SuperFoldPolicy::GetInstance().SwitchScreenAndSetScreenPower(screenId, isScreenOn);
    EXPECT_EQ(SuperFoldPolicy::GetInstance().GetCurrentScreenId(), screenId);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayMode01, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::FULL;
    DMError ret = SuperFoldPolicy::GetInstance().ChangeScreenDisplayMode(mode);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeInner01, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(false);
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(false);
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    DMError ret = SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeInner(mode, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeInner02, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::FULL;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(false);
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(false);
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::MAIN);
    DMError ret = SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeInner(mode, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeInner03, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::COORDINATION;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(false);
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(false);
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    DMError ret = SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeInner(mode, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeInner04, TestSize.Level1)
{
    FoldDisplayMode mode = FoldDisplayMode::UNKNOWN;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().onBootAnimation_.store(false);
    SuperFoldPolicy::GetInstance().isLockDisplayMode_.store(false);
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    DMError ret = SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeInner(mode, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_MODE_ID);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeInner05, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::MAIN);
    DMError ret = SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeInner(mode, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
    EXPECT_TRUE(g_logMsg.find("already in displayMode") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeToCoordination01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::MAIN);
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeToCoordination(isScreenOn);
    EXPECT_TRUE(g_logMsg.find("only full can enter coordination") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeToCoordination02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    bool isScreenOn = true;
    SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeToCoordination(isScreenOn);
    usleep(SLEEP_TIME_IN_US);
}

HWTEST_F(SuperFoldPolicyTest, ChangeScreenDisplayModeToCoordination03, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    bool isScreenOn = false;
    SuperFoldPolicy::GetInstance().ChangeScreenDisplayModeToCoordination(isScreenOn);
    usleep(SLEEP_TIME_IN_US);
}

HWTEST_F(SuperFoldPolicyTest, ExitCoordination01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::FULL);
    SuperFoldPolicy::GetInstance().ExitCoordination();
    EXPECT_TRUE(g_logMsg.find("not in coordination") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, ExitCoordination02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().currentDisplayMode_.store(FoldDisplayMode::COORDINATION);
    SuperFoldPolicy::GetInstance().ExitCoordination();
}

HWTEST_F(SuperFoldPolicyTest, ReportFoldDisplayModeChange01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode mode = FoldDisplayMode::FULL;
    SuperFoldPolicy::GetInstance().ReportFoldDisplayModeChange(mode);
    EXPECT_TRUE(g_logMsg.find("displayMode: 1") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, ReportFoldDisplayModeChange02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode mode = FoldDisplayMode::MAIN;
    SuperFoldPolicy::GetInstance().ReportFoldDisplayModeChange(mode);
    EXPECT_TRUE(g_logMsg.find("displayMode: 2") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, SetdisplayModeChangeStatus01, TestSize.Level1)
{
    bool status = true;
    bool isOnBootAnimation = false;
    SuperFoldPolicy::GetInstance().SetdisplayModeChangeStatus(status, isOnBootAnimation);
    EXPECT_TRUE(SuperFoldPolicy::GetInstance().displayModeChangeRunning_.load());
    SuperFoldPolicy::GetInstance().displayModeChangeRunning_.store(false);
}

HWTEST_F(SuperFoldPolicyTest, SetdisplayModeChangeStatus02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().pendingTask_.store(1);
    bool status = false;
    bool isOnBootAnimation = false;
    SuperFoldPolicy::GetInstance().SetdisplayModeChangeStatus(status, isOnBootAnimation);
    EXPECT_FALSE(SuperFoldPolicy::GetInstance().displayModeChangeRunning_.load());
}

HWTEST_F(SuperFoldPolicyTest, SetdisplayModeChangeStatus03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    SuperFoldPolicy::GetInstance().pendingTask_.store(2);
    bool status = false;
    bool isOnBootAnimation = false;
    SuperFoldPolicy::GetInstance().SetdisplayModeChangeStatus(status, isOnBootAnimation);
    EXPECT_TRUE(g_logMsg.find("pendingTask not zero") == std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, SetAndCheckFoldStatus01, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::CLOSE);
    FoldStatus status = FoldStatus::EXPAND;
    bool ret = SuperFoldPolicy::GetInstance().SetAndCheckFoldStatus(status);
    EXPECT_FALSE(ret);
}

HWTEST_F(SuperFoldPolicyTest, SetAndCheckFoldStatus02, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    FoldStatus status = FoldStatus::FOLDED;
    bool ret = SuperFoldPolicy::GetInstance().SetAndCheckFoldStatus(status);
    EXPECT_FALSE(ret);
}

HWTEST_F(SuperFoldPolicyTest, SetAndCheckFoldStatus03, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    SuperFoldPolicy::GetInstance().lastFoldStatus_ = FoldStatus::EXPAND;
    FoldStatus status = FoldStatus::EXPAND;
    bool ret = SuperFoldPolicy::GetInstance().SetAndCheckFoldStatus(status);
    EXPECT_FALSE(ret);
}

HWTEST_F(SuperFoldPolicyTest, SetAndCheckFoldStatus04, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().screenClosedState_.store(ScreenClosedState::OPEN);
    SuperFoldPolicy::GetInstance().lastFoldStatus_ = FoldStatus::UNKNOWN;
    FoldStatus status = FoldStatus::EXPAND;
    bool ret = SuperFoldPolicy::GetInstance().SetAndCheckFoldStatus(status);
    EXPECT_TRUE(ret);
}

HWTEST_F(SuperFoldPolicyTest, GetPhyFoldStatus01, TestSize.Level1)
{
    SuperFoldPolicy::GetInstance().phyFoldStatus_ = FoldStatus::EXPAND;
    FoldStatus status = SuperFoldPolicy::GetInstance().GetPhyFoldStatus();
    EXPECT_EQ(status, FoldStatus::EXPAND);
}

HWTEST_F(SuperFoldPolicyTest, NotifyFoldStatus01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenClosedState state = ScreenClosedState::CLOSE;
    SuperFoldPolicy::GetInstance().lastFoldStatus_ = FoldStatus::UNKNOWN;
    SuperFoldPolicy::GetInstance().NotifyFoldStatus(state);
    EXPECT_TRUE(g_logMsg.find("ScreenClosedState:0") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, NotifyFoldStatus02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenClosedState state = ScreenClosedState::OPEN;
    SuperFoldPolicy::GetInstance().phyFoldStatus_ = FoldStatus::HALF_FOLD;
    SuperFoldPolicy::GetInstance().lastFoldStatus_ = FoldStatus::UNKNOWN;
    SuperFoldPolicy::GetInstance().NotifyFoldStatus(state);
    EXPECT_TRUE(g_logMsg.find("ScreenClosedState:1") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, NotifyFoldStatus03, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenClosedState state = ScreenClosedState::OPEN;
    SuperFoldPolicy::GetInstance().phyFoldStatus_ = FoldStatus::EXPAND;
    SuperFoldPolicy::GetInstance().lastFoldStatus_ = FoldStatus::UNKNOWN;
    SuperFoldPolicy::GetInstance().NotifyFoldStatus(state);
    EXPECT_TRUE(g_logMsg.find("ScreenClosedState:1") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, NotifyFoldStatus04, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenClosedState state = ScreenClosedState::UNKNOWN;
    SuperFoldPolicy::GetInstance().NotifyFoldStatus(state);
    EXPECT_TRUE(g_logMsg.find("invalid foldstatus") != std::string::npos);
    g_logMsg.clear();
}

HWTEST_F(SuperFoldPolicyTest, NotifyFoldStatus05, TestSize.Level1)
{
    ScreenClosedState state = ScreenClosedState::CLOSE;
    SuperFoldPolicy::GetInstance().lastFoldStatus_ = FoldStatus::FOLDED;
    SuperFoldPolicy::GetInstance().NotifyFoldStatus(state);
}
}
}
}