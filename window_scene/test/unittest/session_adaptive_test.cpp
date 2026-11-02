/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "session/host/include/main_session.h"
#include "session/host/include/session.h"
#include "mock/mock_session_stage.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t TEST_PERSISTENT_ID = 1;
}

class SessionAdaptiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MainSession> mainSession_;
    GameAdaptiveResolutionConfig MakeConfig(int32_t windowId, bool enable);
};

void SessionAdaptiveTest::SetUpTestCase() {}

void SessionAdaptiveTest::TearDownTestCase() {}

void SessionAdaptiveTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "sessionAdaptiveAbility";
    info.moduleName_ = "sessionAdaptiveModule";
    info.bundleName_ = "sessionAdaptiveBundle";
    info.persistentId_ = TEST_PERSISTENT_ID;
    sptr<MainSession::SpecificSessionCallback> specificCallback = nullptr;
    mainSession_ = sptr<MainSession>::MakeSptr(info, specificCallback);
    ASSERT_NE(nullptr, mainSession_);
    mainSession_->persistentId_ = TEST_PERSISTENT_ID;
}

void SessionAdaptiveTest::TearDown()
{
    mainSession_ = nullptr;
}

GameAdaptiveResolutionConfig SessionAdaptiveTest::MakeConfig(int32_t windowId, bool enable)
{
    GameAdaptiveResolutionConfig config;
    config.enable_ = enable;
    config.scaleRatio_ = 0.5f;
    config.windowId_ = windowId;
    return config;
}

namespace {
/**
 * @tc.name: MainSessionUpdateAdaptiveConfigMismatchedWindow
 * @tc.desc: configs targeting another window are rejected
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveTest, MainSessionUpdateAdaptiveConfigMismatchedWindow, TestSize.Level1)
{
    auto ret = mainSession_->UpdateAdaptiveResolutionConfig(MakeConfig(TEST_PERSISTENT_ID + 1, true));
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: MainSessionUpdateAdaptiveConfigNullFunc
 * @tc.desc: missing registered func returns NULLPTR
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveTest, MainSessionUpdateAdaptiveConfigNullFunc, TestSize.Level1)
{
    auto ret = mainSession_->UpdateAdaptiveResolutionConfig(MakeConfig(TEST_PERSISTENT_ID, true));
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: MainSessionUpdateAdaptiveConfigForwards
 * @tc.desc: registered func receives persistentId, uid and the config
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveTest, MainSessionUpdateAdaptiveConfigForwards, TestSize.Level1)
{
    int32_t observedWindowId = -1;
    int32_t observedUid = -1;
    GameAdaptiveResolutionConfig observedConfig;
    mainSession_->SetAdaptiveResolutionConfigFunc(
        [&observedWindowId, &observedUid, &observedConfig](int32_t persistentId, int32_t callingUid,
            const GameAdaptiveResolutionConfig& config) {
            observedWindowId = persistentId;
            observedUid = callingUid;
            observedConfig = config;
            return WMError::WM_OK;
        });
    mainSession_->callingUid_ = 100;
    auto ret = mainSession_->UpdateAdaptiveResolutionConfig(MakeConfig(TEST_PERSISTENT_ID, true));
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_EQ(observedWindowId, TEST_PERSISTENT_ID);
    EXPECT_EQ(observedUid, 100);
    EXPECT_EQ(observedConfig.scaleRatio_, 0.5f);
    EXPECT_TRUE(observedConfig.enable_);

    // func result propagates
    mainSession_->SetAdaptiveResolutionConfigFunc(
        [](int32_t persistentId, int32_t callingUid, const GameAdaptiveResolutionConfig& config) {
            return WMError::WM_ERROR_INVALID_PARAM;
        });
    EXPECT_EQ(mainSession_->UpdateAdaptiveResolutionConfig(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: MainSessionNotifyAdaptiveResolutionChanged
 * @tc.desc: push to a session without a stage completes without side effects
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveTest, MainSessionNotifyAdaptiveResolutionChanged, TestSize.Level1)
{
    auto ret = mainSession_->NotifyAdaptiveResolutionChanged(
        GameAdaptiveResolutionStatus::SET, MakeConfig(TEST_PERSISTENT_ID, true));
    EXPECT_EQ(ret, WSError::WS_OK);
    ret = mainSession_->NotifyAdaptiveResolutionChanged(
        GameAdaptiveResolutionStatus::CANCELLED, MakeConfig(TEST_PERSISTENT_ID, false));
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(200000); // 200ms, let the posted task run
}

/**
 * @tc.name: SessionAdaptiveNotifyFuncs
 * @tc.desc: notify without registered funcs is a no-op; with funcs the value passes through
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveTest, SessionAdaptiveNotifyFuncs, TestSize.Level1)
{
    // no func registered: no crash
    mainSession_->NotifyAdaptiveResolutionModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    mainSession_->NotifyAdaptiveResolutionHookWindowChange(true);

    WindowMode observedMode = WindowMode::WINDOW_MODE_UNDEFINED;
    bool observedEnable = false;
    int32_t modeCount = 0;
    int32_t hookCount = 0;
    mainSession_->SetAdaptiveResolutionModeChangeFunc(
        [&observedMode, &modeCount](WindowMode newMode) {
            observedMode = newMode;
            modeCount++;
        });
    mainSession_->SetAdaptiveResolutionHookWindowChangeFunc(
        [&observedEnable, &hookCount](bool enableHookWindow) {
            observedEnable = enableHookWindow;
            hookCount++;
        });
    mainSession_->NotifyAdaptiveResolutionModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    mainSession_->NotifyAdaptiveResolutionHookWindowChange(true);
    EXPECT_EQ(observedMode, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_TRUE(observedEnable);
    EXPECT_EQ(modeCount, 1);
    EXPECT_EQ(hookCount, 1);

    mainSession_->NotifyAdaptiveResolutionHookWindowChange(false);
    EXPECT_FALSE(observedEnable);
    EXPECT_EQ(hookCount, 2);
}
}
} // namespace Rosen
} // namespace OHOS
