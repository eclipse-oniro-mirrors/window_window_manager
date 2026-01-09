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
#include <parameter.h>
#include <parameters.h>

#include "common_test_utils.h"
#include "display_manager_agent_default.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "screen_scene_config.h"
#include "screen_setting_helper.h"
#include "fold_screen_state_internel.h"
#include "mock/mock_accesstoken_kit.h"
#include "window_manager_hilog.h"
#include "test_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
constexpr uint32_t M_STATUS_WIDTH = 1008;
constexpr uint32_t F_STATUS_WIDTH = 2048;
constexpr uint32_t G_STATUS_WIDTH = 3184;
const ScreenId SCREENID = 1000;
constexpr uint32_t EXCEPTION_DPI = 10;
constexpr uint32_t PC_MODE_DPI = 304;
constexpr ScreenId SCREEN_ID_FULL = 0;
constexpr ScreenId SCREEN_ID_MAIN = 5;
const bool CORRECTION_ENABLE = system::GetIntParameter<int32_t>("const.system.sensor_correction_enable", 0) == 1;
const bool IS_SUPPORT_PC_MODE = system::GetBoolParameter("const.window.support_window_pcmode_switch", false);
bool g_isPcDevice = ScreenSceneConfig::GetExternalScreenDefaultMode() == "none";
}
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}
class ScreenSessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenSessionManager> ssm_;
    ScreenId DEFAULT_SCREEN_ID {0};
    sptr<ScreenSession> InitTestScreenSession(std::string name, ScreenId& screenId);
};

sptr<ScreenSessionManager> ScreenSessionManagerTest::ssm_ = nullptr;

void ScreenSessionManagerTest::SetUpTestCase()
{
    ssm_ = new ScreenSessionManager();
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void ScreenSessionManagerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerTest::SetUp()
{
}

void ScreenSessionManagerTest::TearDown()
{
    usleep(SLEEP_TIME_IN_US);
}

namespace {

/**
 * @tc.name: SetScreenPowerForFold01
 * @tc.desc: SetScreenPowerForFold test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForFold01, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsFoldScreenDevice()) {
        GTEST_SKIP();
    }
    // 内屏预上电
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSessionManager> ssm = sptr<ScreenSessionManager>::MakeSptr();
    ssm->SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON_ADVANCED);

    // 下电
    ssm->lastPowerForAllStatus_.store(ScreenPowerStatus::POWER_STATUS_ON_ADVANCED);
    ssm->lastScreenId_.store(SCREEN_ID_FULL);
    ssm->SetScreenPowerForFold(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
    if (FoldScreenStateInternel::IsSingleDisplayFoldDevice()) {
        EXPECT_TRUE(g_errLog.find("set advancedOn powerStatus off") != std::string::npos);
    } else {
        EXPECT_TRUE(g_errLog.find("set advancedOn powerStatus off") == std::string::npos);
    }
}

/**
 * @tc.name: SetScreenPowerForFold02
 * @tc.desc: SetScreenPowerForFold test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForFold02, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsFoldScreenDevice()) {
        GTEST_SKIP();
    }
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSessionManager> ssm = sptr<ScreenSessionManager>::MakeSptr();
    ssm->SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON_ADVANCED);

    ssm->lastPowerForAllStatus_.store(ScreenPowerStatus::POWER_STATUS_OFF);
    ssm->lastScreenId_.store(SCREEN_ID_FULL);
    ssm->SetScreenPowerForFold(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
    EXPECT_TRUE(g_errLog.find("set advancedOn powerStatus off") == std::string::npos);
}

/**
 * @tc.name: SetScreenPowerForFold03
 * @tc.desc: SetScreenPowerForFold test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForFold03, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsFoldScreenDevice()) {
        GTEST_SKIP();
    }
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSessionManager> ssm = sptr<ScreenSessionManager>::MakeSptr();
    ssm->SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON_ADVANCED);

    ssm->lastPowerForAllStatus_.store(ScreenPowerStatus::POWER_STATUS_ON_ADVANCED);
    ssm->lastScreenId_.store(SCREEN_ID_MAIN);
    ssm->SetScreenPowerForFold(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
    EXPECT_TRUE(g_errLog.find("set advancedOn powerStatus off") == std::string::npos);
}

/**
 * @tc.name: SetDisplayNodeSecurity
 * @tc.desc: SetDisplayNodeSecurity test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDisplayNodeSecurity, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    virtualOption.isSecurity_ = true;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetDisplayNodeSecurity();
    EXPECT_TRUE(g_errLog.find("displayNode is null") != std::string::npos);

    Rosen::RSDisplayNodeConfig rsConfig;
    ScreenId rsScreenId = screenSession->GetRSScreenId();
    rsConfig.screenId = rsScreenId;
    screenSession->CreateDisplayNode(rsConfig);
    screenSession->SetDisplayNodeSecurity();
    EXPECT_TRUE(g_errLog.find("SetSecurityDisplay success") != std::string::npos);
    ssm->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: UpdatePropertyByActiveModeChange001
 * @tc.desc: UpdatePropertyByActiveModeChange001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdatePropertyByActiveModeChange001, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
        sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "UpdateProperty";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->UpdatePropertyByActiveModeChange();
    EXPECT_TRUE(g_errLog.find("active mode bounds") != std::string::npos);
    g_errLog.clear();
#endif
}

/**
 * @tc.name: UpdatePropertyByActiveModeChange002
 * @tc.desc: UpdatePropertyByActiveModeChange002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdatePropertyByActiveModeChange002, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
        sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "UpdateProperty";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    int32_t oldActiveIdx = screenSession->GetActiveId();
    int32_t testActiveIdx = -1;
    screenSession->SetActiveId(testActiveIdx);
    screenSession->UpdatePropertyByActiveModeChange();
    EXPECT_TRUE(g_errLog.find("mode is null") != std::string::npos);
    screenSession->SetActiveId(oldActiveIdx);
    g_errLog.clear();
#endif
}

/**
 * @tc.name: CheckAndNotifyChangeMode001
 * @tc.desc: CheckAndNotifyChangeMode001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckAndNotifyChangeMode001, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "ChangeMode";
    virtualOption.width_ = 1920;
    virtualOption.height_ = 1080;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    RRect testBounds = RRect({ 0, 0, 1280, 760}, 0.0f, 0.0f);
    RRect bounds = RRect({ 0, 0, 1920, 1080}, 0.0f, 0.0f);
    screenSession->SetBounds(bounds);
    ssm_->CheckAndNotifyChangeMode(testBounds, screenSession);
    EXPECT_TRUE(g_errLog.find("notify end") != std::string::npos);
    g_errLog.clear();
#endif
}

/**
 * @tc.name: CheckAndNotifyChangeMode002
 * @tc.desc: CheckAndNotifyChangeMode002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckAndNotifyChangeMode002, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "ChangeMode";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    RRect testBounds = RRect({ 0, 0, 1920, 1080}, 0.0f, 0.0f);
    RRect bounds = RRect({ 0, 0, 1920, 1080}, 0.0f, 0.0f);
    screenSession->SetBounds(bounds);
    ssm_->CheckAndNotifyChangeMode(testBounds, screenSession);
    EXPECT_TRUE(g_errLog.find("no notify") != std::string::npos);
    g_errLog.clear();
#endif
}

/**
 * @tc.name: CheckAndNotifyRefreshRate001
 * @tc.desc: CheckAndNotifyRefreshRate001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckAndNotifyRefreshRate001, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "RefreshRate";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    uint32_t testRefresh = 60;
    screenSession->UpdateRefreshRate(140);
    ssm_->CheckAndNotifyRefreshRate(testRefresh, screenSession);
    EXPECT_TRUE(g_errLog.find("notify end") != std::string::npos);
    g_errLog.clear();
#endif
}

/**
 * @tc.name: CheckAndNotifyRefreshRate002
 * @tc.desc: CheckAndNotifyRefreshRate002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckAndNotifyRefreshRate002, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "RefreshRate001";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    uint32_t testRefresh = 140;
    screenSession->UpdateRefreshRate(140);
    ssm_->CheckAndNotifyRefreshRate(testRefresh, screenSession);
    EXPECT_TRUE(g_errLog.find("no notify") != std::string::npos);
    g_errLog.clear();
#endif
}

/**
 * @tc.name: ReportScreenModeChangeEvent
 * @tc.desc: ReportScreenModeChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ReportScreenModeChangeEvent, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    RSScreenModeInfo screenmode;
    screenmode.SetScreenWidth(1920);
    screenmode.SetScreenHeight(1080);
    screenmode.SetScreenRefreshRate(60);
    screenmode.SetScreenModeId(1050);
    ssm_->ReportScreenModeChangeEvent(screenmode, 0);
    EXPECT_FALSE(g_errLog.find("Write HiSysEvent error") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: ReportRelativePositionChangeEvent
 * @tc.desc: ReportRelativePositionChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ReportRelativePositionChangeEvent, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    ScreenId mainScreenId = 0;
    ScreenId externalScreenId = 16;
    MultiScreenPositionOptions mianOptions = {mainScreenId, 0, 0};
    MultiScreenPositionOptions externalOptions = {mainScreenId, 1920, 1080};
    std::string errmsg = "openHarmony";
    ssm_->ReportRelativePositionChangeEvent(mianOptions, externalOptions, errmsg);
    EXPECT_FALSE(g_errLog.find("Write HiSysEvent error") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: SetScreenActiveMode001
 * @tc.desc: SetScreenActiveMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenActiveMode001, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetScreenActiveMode";
    ASSERT_NE(ssm_, nullptr);
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    MockAccesstokenKit::MockIsSystemApp(false);
    MockSessionPermission::MockIsStarByHdcd(true);
    auto ret = ssm_->SetScreenActiveMode(screenId, 0);
    EXPECT_EQ(DMError::DM_ERROR_NOT_SYSTEM_APP, ret);

    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(false);
    ret = ssm_->SetScreenActiveMode(screenId, 0);
    EXPECT_EQ(DMError::DM_ERROR_NOT_SYSTEM_APP, ret);

    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: SetScreenActiveMode002
 * @tc.desc: SetScreenActiveMode002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenActiveMode002, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    ssm_->DestroyVirtualScreen(screenId);
    ScreenId screenId = -1ULL;
    ASSERT_NE(ssm_, nullptr);
    auto ret = ssm_->SetScreenActiveMode(ScreenId, 0);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret);
#endif
}

/**
 * @tc.name: SetScreenActiveMode003
 * @tc.desc: SetScreenActiveMode003
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenActiveMode003, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    ssm_->DestroyVirtualScreen(screenId);
    ScreenId screenId = 501;
    ASSERT_NE(ssm_, nullptr);
    auto ret = ssm_->SetScreenActiveMode(ScreenId, 0);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret);
#endif
}

/**
 * @tc.name: SetScreenActiveMode004
 * @tc.desc: SetScreenActiveMode004
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenActiveMode004, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    sptr<ScreenSession> screenSession = ssm_->GetOrCreateScreenSession(1050);
    screenSession = nullptr;
    auto ret = ssm_->SetScreenActiveMode(1050, 0);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret);
    screenSession = ssm_->GetOrCreateScreenSession(1050);
    ret = ssm_->SetScreenActiveMode(1050, 0);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret)
#endif
}

/**
 * @tc.name: SetScreenActiveMode005
 * @tc.desc: SetScreenActiveMode005
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenActiveMode005, TestSize.Level1)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetScreenActiveMode005";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto ret = ssm_->SetScreenActiveMode(1050, 0);
    EXPECT_EQ(DMError::DM_OK, ret);
#endif
}

/**
 * @tc.name: UpdateSessionByActiveModeChange001
 * @tc.desc: UpdateSessionByActiveModeChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateSessionByActiveModeChange001, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    sptr<ScreenSession> screenSession = nullptr;
    RSScreenModeInfo screenMode;
    ssm_->UpdateSessionByActiveModeChange(screenSession, screenMode);
    EXPECT_FALSE(g_errLog.find("screenSession is nullptr") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: UpdateSessionByActiveModeChange002
 * @tc.desc: UpdateSessionByActiveModeChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateSessionByActiveModeChange002, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    RSScreenModeInfo screenMode;
    sptr<ScreenSession> screenSession = ssm_->GetOrCreateScreenSession(1050);
    ssm_->UpdateSessionByActiveModeChange(screenSession, screenMode);
    EXPECT_TRUE(g_errLog.find("end") != std::string::npos);
    g_errLog.clear();
}


RecoverScreenActiveMode
/**
 * @tc.name: RecoverScreenActiveMode001
 * @tc.desc: RecoverScreenActiveMode001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RecoverScreenActiveMode001, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 1058;
    ssm_->RecoverScreenActiveMode(screenId, 0);
    EXPECT_TRUE(g_errLog.find("screenSession is null") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: RecoverScreenActiveMode002
 * @tc.desc: RecoverScreenActiveMode002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RecoverScreenActiveMode002, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "RecoverScreenActiveMode002";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ssm_->screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockSessionPermission::MockIsStarByHdcd(false);
    ssm_->RecoverScreenActiveMode(rsScreenId, 0);
    EXPECT_TRUE(g_errLog.find("recover error") != std::string::npos);
    MockAccesstokenKit::MockIsSystemApp(true);
    MockSessionPermission::MockIsStarByHdcd(true);
    g_errLog.clear();
}

/**
 * @tc.name: SwitchScrollParam01
 * @tc.desc: SwitchScrollParam test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchScrollParam01, TestSize.Level1)
{
    ScreenSceneConfig::scrollableParams_.clear();
    vector<FoldDisplayMode> displayModeALL = {
        FoldDisplayMode::SUB,
        FoldDisplayMode::MAIN,
        FoldDisplayMode::FULL,
        FoldDisplayMode::UNKNOWN,
        FoldDisplayMode::COORDINATION,
    };
    std::map<FoldDisplayMode, std::string> scrollVelocityScaleParam = {
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::SUB, "1.0"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::MAIN, "1.1"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::FULL, "1.2"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::UNKNOWN, "1.3"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::COORDINATION, "1.4")
    };
    std::map<FoldDisplayMode, std::string> scrollFrictionParam = {
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::SUB, "1.0"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::MAIN, "2.0"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::FULL, "3.0"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::UNKNOWN, "4.0"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::COORDINATION, "5.0"),
    };
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    system::SetParameter("persist.scrollable.velocityScale", "0");
    system::SetParameter("persist.scrollable.friction", "0");
    std::string ret1, ret2;
    for (FoldDisplayMode displayMode : displayModeALL) {
        ScrollableParam scrollableParam;
        scrollableParam.velocityScale_ = scrollVelocityScaleParam.count(displayMode) ?
            scrollVelocityScaleParam[displayMode] : "0";
        scrollableParam.friction_ = scrollFrictionParam.count(displayMode) ?
            scrollFrictionParam[displayMode] : "0";
        ScreenSceneConfig::scrollableParams_[displayMode] = scrollableParam;
        ssm->SwitchScrollParam(displayMode);
        ret1 = system::GetParameter("persist.scrollable.velocityScale", "0");
        ret2 = system::GetParameter("persist.scrollable.friction", "0");
        EXPECT_NE(ret1, "");
        EXPECT_NE(ret2, "");
    }
}

/**
 * @tc.name: SwitchScrollParam02
 * @tc.desc: SwitchScrollParam test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchScrollParam02, TestSize.Level1)
{
    ScreenSceneConfig::scrollableParams_.clear();
    vector<FoldDisplayMode> displayModeALL = {
        FoldDisplayMode::SUB,
        FoldDisplayMode::MAIN,
        FoldDisplayMode::FULL,
        FoldDisplayMode::UNKNOWN,
        FoldDisplayMode::COORDINATION,
    };
    std::map<FoldDisplayMode, std::string> scrollVelocityScaleParam = {
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::FULL, "2.0"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::MAIN, "main"),
        pair<FoldDisplayMode, std::string>(FoldDisplayMode::UNKNOWN, "!!"),
    };
    std::map<FoldDisplayMode, std::string> scrollFrictionParam;
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    system::SetParameter("persist.scrollable.velocityScale", "0");
    system::SetParameter("persist.scrollable.friction", "0");
    std::string ret1, ret2;
    for (FoldDisplayMode displayMode : displayModeALL) {
        ScrollableParam scrollableParam;
        scrollableParam.velocityScale_ = scrollVelocityScaleParam.count(displayMode) ?
            scrollVelocityScaleParam[displayMode] : "0";
        scrollableParam.friction_ = scrollFrictionParam.count(displayMode) ?
            scrollFrictionParam[displayMode] : "0";
        ScreenSceneConfig::scrollableParams_[displayMode] = scrollableParam;
        ssm->SwitchScrollParam(displayMode);
        ret1 = system::GetParameter("persist.scrollable.velocityScale", "0");
        ret2 = system::GetParameter("persist.scrollable.friction", "0");
        EXPECT_NE(ret1, "");
        EXPECT_NE(ret2, "");
    }
}

/**
 * @tc.name: WakeUpPictureFrameBlock
 * @tc.desc: WakeUpPictureFrameBlock test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WakeUpPictureFrameBlock, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->pictureFrameReady_ = false;
    ssm_->pictureFrameBreak_ = false;
    ssm_->WakeUpPictureFrameBlock(DisplayEvent::SCREEN_LOCK_OFF);
    ASSERT_EQ(ssm_->pictureFrameReady_, false);
    ASSERT_EQ(ssm_->pictureFrameBreak_, false);
    ssm_->WakeUpPictureFrameBlock(DisplayEvent::SCREEN_LOCK_START_DREAM);
    ASSERT_EQ(ssm_->pictureFrameReady_, true);
    ssm_->WakeUpPictureFrameBlock(DisplayEvent::SCREEN_LOCK_END_DREAM);
    ASSERT_EQ(ssm_->pictureFrameBreak_, true);
}

/**
 * @tc.name: AddVirtualScreenBlockList
 * @tc.desc: AddVirtualScreenBlockList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddVirtualScreenBlockList, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<int32_t> persistentIds {0, 1, 2};
    ASSERT_EQ(DMError::DM_OK, ssm_->AddVirtualScreenBlockList(persistentIds));
}

/**
 * @tc.name: RemoveVirtualScreenBlockList
 * @tc.desc: RemoveVirtualScreenBlockList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveVirtualScreenBlockList, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<int32_t> persistentIds {0, 1, 2};
    ASSERT_EQ(DMError::DM_OK, ssm_->RemoveVirtualScreenBlockList(persistentIds));
}

/**
 * @tc.name: BlockScreenWaitPictureFrameByCV
 * @tc.desc: BlockScreenWaitPictureFrameByCV test
 * @tc.type: FUNC
 */
 HWTEST_F(ScreenSessionManagerTest, BlockScreenWaitPictureFrameByCV, Function | SmallTest | Level3)
 {
     ASSERT_NE(ssm_, nullptr);
     ssm_->pictureFrameReady_ = true;
     ssm_->pictureFrameBreak_ = true;
     bool result = ssm_->BlockScreenWaitPictureFrameByCV(true);
     ASSERT_EQ(result, true);
     result = ssm_->BlockScreenWaitPictureFrameByCV(false);
     ASSERT_EQ(result, true);

     ASSERT_EQ(ssm_->pictureFrameReady_, false);
     ASSERT_EQ(ssm_->pictureFrameBreak_, false);
}

/**
 * @tc.name: DestroyExtendVirtualScreen001
 * @tc.desc: DestroyExtendVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, DestroyExtendVirtualScreen001, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto virtualScreenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(virtualScreenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    ssm_->DestroyExtendVirtualScreen();
    screenSession->SetIsExtendVirtual(true);
    ssm_->DestroyExtendVirtualScreen();
    ssm_->DestroyVirtualScreen(virtualScreenId);
}

/**
 * @tc.name: CreateExtendVirtualScreen001
 * @tc.desc: CreateExtendVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateExtendVirtualScreen001, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto virtualScreenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<ScreenSession> virtualScreenSession = ssm_->GetScreenSession(virtualScreenId);
    virtualScreenSession->SetScreenType(ScreenType::REAL);
    ssm_->CreateExtendVirtualScreen(virtualScreenSession->GetRSScreenId());
    ssm_->DestroyVirtualScreen(virtualScreenId);
}

/**
 * @tc.name: IsPhysicalExtendScreenInUse001
 * @tc.desc: CreateExtendVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsPhysicalExtendScreenInUse001, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto virtualScreenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<ScreenSession> virtualScreenSession = ssm_->GetScreenSession(virtualScreenId);
    ASSERT_NE(virtualScreenSession, nullptr);

    virtualScreenSession->SetScreenType(ScreenType::VIRTUAL);
    DMError ret = ssm_->IsPhysicalExtendScreenInUse(virtualScreenSession->GetRSScreenId(), 100);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    VirtualScreenOption physicalOption;
    physicalOption.name_ = "createPhysicalOption";
    auto physicalScreenId = ssm_->CreateVirtualScreen(physicalOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> physicalScreenSession = ssm_->GetScreenSession(physicalScreenId);
    ASSERT_NE(physicalScreenSession, nullptr);

    physicalScreenSession->SetScreenType(ScreenType::REAL);
    physicalScreenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    DMError ret1 = ssm_->IsPhysicalExtendScreenInUse(virtualScreenSession->GetRSScreenId(),
        physicalScreenSession->GetRSScreenId());
    EXPECT_EQ(ret1, DMError::DM_OK);

    physicalScreenSession->SetScreenType(ScreenType::VIRTUAL);
    DMError ret2 = ssm_->IsPhysicalExtendScreenInUse(virtualScreenSession->GetRSScreenId(),
        physicalScreenSession->GetRSScreenId());
    EXPECT_EQ(ret2, DMError::DM_ERROR_UNKNOWN);

    ssm_->DestroyVirtualScreen(virtualScreenId);
    ssm_->DestroyVirtualScreen(physicalScreenId);
}

/**
 * @tc.name: SetForceCloseHdr01
 * @tc.desc: SetForceCloseHdr test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetForceCloseHdr01, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 0;
    bool isForceCloseHdr = true;
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ssm_->SetForceCloseHdr(screenId, isForceCloseHdr);
    EXPECT_TRUE(g_errLog.find("permission denied!") != std::string::npos);
}

/**
 * @tc.name: SetForceCloseHdr02
 * @tc.desc: SetForceCloseHdr test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetForceCloseHdr02, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 0;
    bool isForceCloseHdr = true;
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->SetForceCloseHdr(screenId, isForceCloseHdr);
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: SetForceCloseHdr03
 * @tc.desc: SetForceCloseHdr test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetForceCloseHdr03, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 0;
    bool isForceCloseHdr = true;
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->SetForceCloseHdr(screenId, isForceCloseHdr);
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: SetDefaultMultiScreenModeWhenSwitchUser01
 * @tc.desc: SetDefaultMultiScreenModeWhenSwitchUser test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDefaultMultiScreenModeWhenSwitchUser01, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ssm_->SetDefaultMultiScreenModeWhenSwitchUser();
    EXPECT_TRUE(g_errLog.find("permission denied!") != std::string::npos);
}

/**
 * @tc.name: SetDefaultMultiScreenModeWhenSwitchUser02
 * @tc.desc: SetDefaultMultiScreenModeWhenSwitchUser test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDefaultMultiScreenModeWhenSwitchUser02, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->SetDefaultMultiScreenModeWhenSwitchUser();
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: SetDefaultMultiScreenModeWhenSwitchUser03
 * @tc.desc: SetDefaultMultiScreenModeWhenSwitchUser test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDefaultMultiScreenModeWhenSwitchUser03, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->SetDefaultMultiScreenModeWhenSwitchUser();
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: NotifyExtendScreenCreateFinish01
 * @tc.desc: NotifyExtendScreenCreateFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyExtendScreenCreateFinish01, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ssm_->NotifyExtendScreenCreateFinish();
    EXPECT_TRUE(g_errLog.find("permission denied!") != std::string::npos);
}

/**
 * @tc.name: NotifyExtendScreenCreateFinish02
 * @tc.desc: NotifyExtendScreenCreateFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyExtendScreenCreateFinish02, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->NotifyExtendScreenCreateFinish();
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: NotifyExtendScreenCreateFinish03
 * @tc.desc: NotifyExtendScreenCreateFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyExtendScreenCreateFinish03, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->NotifyExtendScreenCreateFinish();
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: NotifyExtendScreenDestroyFinish01
 * @tc.desc: NotifyExtendScreenDestroyFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyExtendScreenDestroyFinish01, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ssm_->NotifyExtendScreenDestroyFinish();
    EXPECT_TRUE(g_errLog.find("permission denied!") != std::string::npos);
}

/**
 * @tc.name: NotifyExtendScreenDestroyFinish02
 * @tc.desc: NotifyExtendScreenDestroyFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyExtendScreenDestroyFinish02, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->NotifyExtendScreenDestroyFinish();
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: NotifyExtendScreenDestroyFinish03
 * @tc.desc: NotifyExtendScreenDestroyFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyExtendScreenDestroyFinish03, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->NotifyExtendScreenDestroyFinish();
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: SetFoldStatusExpandAndLocked01
 * @tc.desc: SetFoldStatusExpandAndLocked test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetFoldStatusExpandAndLocked01, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    bool isLocked = true;
    ssm_->SetFoldStatusExpandAndLocked(isLocked);
    EXPECT_TRUE(g_errLog.find("permission denied!") != std::string::npos);
}

/**
 * @tc.name: SetFoldStatusExpandAndLocked02
 * @tc.desc: SetFoldStatusExpandAndLocked test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetFoldStatusExpandAndLocked02, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(true);
    bool isLocked = true;
    ssm_->SetFoldStatusExpandAndLocked(isLocked);
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: SetFoldStatusExpandAndLocked03
 * @tc.desc: SetFoldStatusExpandAndLocked test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetFoldStatusExpandAndLocke03, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    bool isLocked = true;
    ssm_->SetFoldStatusExpandAndLocked(isLocked);
    EXPECT_TRUE(g_errLog.find("permission denied!") == std::string::npos);
}

/**
 * @tc.name: OnBeforeScreenPropertyChange
 * @tc.desc: OnBeforeScreenPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnBeforeScreenPropertyChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->OnBeforeScreenPropertyChange(FoldStatus::UNKNOWN);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);
}

/**
 * @tc.name: ScbStatusRecoveryWhenSwitchUser
 * @tc.desc: ScbStatusRecoveryWhenSwitchUser
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScbStatusRecoveryWhenSwitchUser, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        ScreenId id = ssm_->GetDefaultScreenId();
        sptr<ScreenSession> screenSession = ssm_->GetOrCreateScreenSession(id);
        ScreenProperty screenProperty = screenSession->GetScreenProperty();
        uint32_t currentWidth = screenProperty.GetBounds().rect_.GetWidth();
        std::vector<int32_t> oldScbPids_ = { 100 };
        int32_t newScbPid = 101;
        ssm_->ScbStatusRecoveryWhenSwitchUser(oldScbPids_, newScbPid);
        FoldDisplayMode mode = ssm_->GetFoldDisplayMode();
        if (mode == FoldDisplayMode::MAIN) {
            EXPECT_EQ(currentWidth, M_STATUS_WIDTH);
        } else if (mode == FoldDisplayMode::FULL) {
            EXPECT_EQ(currentWidth, F_STATUS_WIDTH);
        } else {
            EXPECT_EQ(currentWidth, G_STATUS_WIDTH);
        }
    }
}

/**
 * @tc.name: GetScreenAreaOfDisplayArea
 * @tc.desc: GetScreenAreaOfDisplayArea
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenAreaOfDisplayArea, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->screenSessionMap_.clear();
    DisplayId displayId = 0;
    DMRect displayArea = { 0, 0, 1, 1 };
    ScreenId screenId = 0;
    DMRect screenArea = DMRect::NONE();
    sptr<ScreenSession> screenSession = nullptr;
    auto ret = ssm_->GetScreenAreaOfDisplayArea(displayId, displayArea, screenId, screenArea);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    ScreenProperty property = ScreenProperty();
    screenSession = new (std::nothrow) ScreenSession(0, property, 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_.clear();
    ssm_->screenSessionMap_.insert(std::make_pair(0, screenSession));
    ret = ssm_->GetScreenAreaOfDisplayArea(displayId, displayArea, screenId, screenArea);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);

    property.SetOffsetX(0);
    property.SetOffsetY(0);
    property.SetCreaseRect({100, 100, 100, 100});
    property.SetBounds({{0, 0, 100, 100}, 0, 0});
    property.SetPhyBounds({{0, 100, 0, 100}, 0, 0});
    displayArea = { 0, 0, 0, 0 };
    screenSession->screenId_ = 0;
    ret = ssm_->GetScreenAreaOfDisplayArea(displayId, displayArea, screenId, screenArea);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: CalculateRotatedDisplay1
 * @tc.desc: CalculateRotatedDisplay1
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateRotatedDisplay1, Function | SmallTest | Level3)
{
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        return;
    }

    ASSERT_NE(ssm_, nullptr);
    DMRect screenRegion = {0, 0, 100, 200};
    DMRect displayRegion = {0, 0, 200, 100};
    DMRect displayArea = {20, 10, 30, 40};

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    Rotation rotation = Rotation::ROTATION_0;
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    DMRect expectedRect = {0, 0, 200, 100};
    EXPECT_TRUE(g_errLog.find("failed") == std::string::npos);
    g_errLog.clear();

    rotation = Rotation::ROTATION_90;
    displayRegion = {0, 0, 200, 100};
    displayArea = {20, 10, 30, 40};
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    expectedRect = {0, 0, 100, 200};
    EXPECT_EQ(displayRegion, expectedRect);
    expectedRect = {10, 150, 40, 30};
    EXPECT_EQ(displayArea, expectedRect);

    rotation = Rotation::ROTATION_180;
    displayRegion = {0, 0, 100, 200};
    displayArea = {20, 10, 30, 40};
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    EXPECT_TRUE(g_errLog.find("failed") == std::string::npos);
    LOG_SetCallback(nullptr);
    expectedRect = {50, 150, 30, 40};
    EXPECT_EQ(displayArea, expectedRect);

    rotation = Rotation::ROTATION_270;
    displayRegion = {0, 0, 200, 100};
    displayArea = {20, 10, 30, 40};
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    expectedRect = {0, 0, 100, 200};
    EXPECT_EQ(displayRegion, expectedRect);
    expectedRect = {50, 20, 40, 30};
    EXPECT_EQ(displayArea, expectedRect);
}

/**
 * @tc.name: CalculateRotatedDisplay2
 * @tc.desc: CalculateRotatedDisplay2
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateRotatedDisplay2, Function | SmallTest | Level3)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP();
    }

    ASSERT_NE(ssm_, nullptr);
    DMRect screenRegion = {0, 0, 100, 200};
    DMRect displayRegion = {0, 0, 200, 100};
    DMRect displayArea = {20, 10, 30, 40};

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    Rotation rotation = Rotation::ROTATION_90;
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    DMRect expectedRect = {0, 0, 200, 100};
    EXPECT_TRUE(g_errLog.find("failed") == std::string::npos);

    rotation = Rotation::ROTATION_180;
    displayRegion = {0, 0, 200, 100};
    displayArea = {20, 10, 30, 40};
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    expectedRect = {0, 0, 100, 200};
    EXPECT_EQ(displayRegion, expectedRect);
    expectedRect = {10, 150, 40, 30};
    EXPECT_EQ(displayArea, expectedRect);

    rotation = Rotation::ROTATION_270;
    displayRegion = {0, 0, 100, 200};
    displayArea = {20, 10, 30, 40};
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    EXPECT_TRUE(g_errLog.find("failed") == std::string::npos);
    LOG_SetCallback(nullptr);

    expectedRect = {50, 150, 30, 40};
    EXPECT_EQ(displayArea, expectedRect);

    rotation = Rotation::ROTATION_0;
    displayRegion = {0, 0, 200, 100};
    displayArea = {20, 10, 30, 40};
    ssm_->CalculateRotatedDisplay(rotation, screenRegion, displayRegion, displayArea);
    expectedRect = {0, 0, 100, 200};
    EXPECT_EQ(displayRegion, expectedRect);
    expectedRect = {50, 20, 40, 30};
    EXPECT_EQ(displayArea, expectedRect);
}

/**
 * @tc.name: CalculateScreenArea
 * @tc.desc: CalculateScreenArea
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateScreenArea, Function | SmallTest | Level3)
{
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        ASSERT_NE(ssm_, nullptr);
        DMRect displayRegion = DMRect::NONE();
        DMRect displayArea = {10, 20, 30, 40};
        DMRect screenRegion = DMRect::NONE();
        DMRect screenArea = DMRect::NONE();
        ssm_->CalculateScreenArea(displayRegion, displayArea, screenRegion, screenArea);
        EXPECT_EQ(screenArea, displayArea);
    }

    ASSERT_NE(ssm_, nullptr);
    DMRect displayRegion = {0, 0, 50, 100};
    DMRect displayArea = {10, 20, 30, 40};
    DMRect screenRegion = {0, 0, 50, 100};
    DMRect screenArea = DMRect::NONE();
    ssm_->CalculateScreenArea(displayRegion, displayArea, screenRegion, screenArea);
    EXPECT_EQ(screenArea, displayArea);

    screenRegion = {0, 0, 100, 200};
    ssm_->CalculateScreenArea(displayRegion, displayArea, screenRegion, screenArea);
    DMRect expectedRect = {20, 40, 60, 80};
    EXPECT_EQ(screenArea, expectedRect);
}

/**
 * @tc.name: ConvertIntToRotation
 * @tc.desc: ConvertIntToRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConvertIntToRotation, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t rotation = 0;
    Rotation targetRotation = Rotation::ROTATION_0;
    targetRotation = ssm_->ConvertIntToRotation(rotation);
    EXPECT_EQ(targetRotation, Rotation::ROTATION_0);
    rotation = 100;
    targetRotation = ssm_->ConvertIntToRotation(rotation);
    EXPECT_EQ(targetRotation, Rotation::ROTATION_0);
    rotation = 90;
    targetRotation = ssm_->ConvertIntToRotation(rotation);
    EXPECT_EQ(targetRotation, Rotation::ROTATION_90);
    rotation = 180;
    targetRotation = ssm_->ConvertIntToRotation(rotation);
    EXPECT_EQ(targetRotation, Rotation::ROTATION_180);
    rotation = 270;
    targetRotation = ssm_->ConvertIntToRotation(rotation);
    EXPECT_EQ(targetRotation, Rotation::ROTATION_270);
}

/**
 * @tc.name: GetPhysicalScreenIds
 * @tc.desc: GetPhysicalScreenIds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPhysicalScreenIds, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<ScreenId> screenIds;
    auto ret = ssm_->GetPhysicalScreenIds(screenIds);
    EXPECT_EQ(ret, DMError::DM_OK);

    ScreenId screenId = 11;
    sptr<ScreenSession> screenSession = nullptr;
    ssm_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    std::vector<ScreenId> screenIds1;
    auto ret1 = ssm_->GetPhysicalScreenIds(screenIds1);
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: SetVirtualScreenAutoRotation01
 * @tc.desc: SetVirtualScreenAutoRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenAutoRotation01, Function | SmallTest | Level3)
{
    ScreenId screenId = 0;
    bool enable = true;
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSASystemApp(false);
    auto ret = ssm_->SetVirtualScreenAutoRotation(screenId, enable);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetVirtualScreenAutoRotation02
 * @tc.desc: SetVirtualScreenAutoRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenAutoRotation02, Function | SmallTest | Level3)
{
    ScreenId screenId = 0;
    bool enable = true;
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSASystemApp(true);
    auto ret = ssm_->SetVirtualScreenAutoRotation(screenId, enable);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetVirtualScreenAutoRotation03
 * @tc.desc: SetVirtualScreenAutoRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenAutoRotation03, Function | SmallTest | Level3)
{
    ScreenId screenId = 0;
    bool enable = true;
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSASystemApp(true);
    auto ret = ssm_->SetVirtualScreenAutoRotation(screenId, enable);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: CreateScreenProperty
 * @tc.desc: CreateScreenProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateScreenProperty, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 0;
    ScreenProperty property = ScreenProperty();
    EXPECT_EQ(DISPLAY_GROUP_ID_INVALID, property.GetDisplayGroupId());
    EXPECT_EQ(SCREEN_ID_INVALID, property.GetMainDisplayIdOfGroup());

    ssm_->CreateScreenProperty(screenId, property);
    EXPECT_EQ(DISPLAY_GROUP_ID_DEFAULT, property.GetDisplayGroupId());
    EXPECT_EQ(MAIN_SCREEN_ID_DEFAULT, property.GetMainDisplayIdOfGroup());

    property.SetDisplayGroupId(1);
    property.SetMainDisplayIdOfGroup(1);
    ssm_->CreateScreenProperty(screenId, property);
    EXPECT_EQ(1, property.GetDisplayGroupId());
    EXPECT_EQ(1, property.GetMainDisplayIdOfGroup());
}

/**
 * @tc.name: InitScreenProperty
 * @tc.desc: InitScreenProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitScreenProperty, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 0;
    ScreenProperty property = ScreenProperty();
    RSScreenModeInfo screenModeInfo = RSScreenModeInfo();
    screenModeInfo.SetScreenWidth(100);
    screenModeInfo.SetScreenHeight(200);
    RSScreenCapability screenCapability = RSScreenCapability();

    ssm_->InitScreenProperty(screenId, screenModeInfo, screenCapability, property);
    EXPECT_EQ(0, property.GetScreenAreaOffsetX());
    EXPECT_EQ(0, property.GetScreenAreaOffsetY());
    EXPECT_EQ(100, property.GetScreenAreaWidth());
    EXPECT_EQ(200, property.GetScreenAreaHeight());
}

/**
 * @tc.name: InitExtendScreenProperty
 * @tc.desc: InitExtendScreenProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitExtendScreenProperty, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 0;
    ScreenProperty property = ScreenProperty();
    property.SetBounds({{0, 0, 4000, 4000}, 0, 0});
    ScreenSceneConfig::enableConfig_["isSupportOffScreenRendering"] = true;
    ScreenSceneConfig::offScreenPPIThreshold_ = 100;
    ScreenSceneConfig::stringConfig_["externalScreenDefaultMode"] = "none";

    ssm_->Init();
    sptr<ScreenSession> screenSession = ssm_->GetScreenSessionInner(screenId, property);
    ssm_->InitExtendScreenProperty(screenId, screenSession, property);
    EXPECT_EQ(3840, screenSession->GetValidWidth());

    property.SetBounds({{0, 0, 1000, 1000}, 0, 0});
    ssm_->InitExtendScreenProperty(screenId, screenSession, property);
    EXPECT_EQ(3120, screenSession->GetValidWidth());
}

/**
 * @tc.name: GetOrCreateScreenSession
 * @tc.desc: GetOrCreateScreenSession
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetOrCreateScreenSession, Function | SmallTest | Level3)
{
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        sptr<ScreenSession> session = ssm_->GetOrCreateScreenSession(SCREENID);
        EXPECT_NE(session, nullptr);
    }
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch001
 * @tc.desc: SetScreenPrivacyWindowTagSwitch test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPrivacyWindowTagSwitch001, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ScreenId mainScreenId = 0;
    std::vector<std::string> privacyWindowTag{"test1", "test2"};
    auto ret = ssm_->SetScreenPrivacyWindowTagSwitch(mainScreenId, privacyWindowTag, true);
    EXPECT_EQ(ret, DMError::DM_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch002
 * @tc.desc: SetScreenPrivacyWindowTagSwitch test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPrivacyWindowTagSwitch002, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ScreenId mainScreenId = 0;
    ScreenId invalidScreenId = -1;
    std::vector<std::string> privacyWindowTag{"test1", "test2"};
    auto ret = ssm_->SetScreenPrivacyWindowTagSwitch(invalidScreenId, privacyWindowTag, true);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    ret = ssm_->SetScreenPrivacyWindowTagSwitch(mainScreenId, privacyWindowTag, true);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateSuperFoldRefreshRate
 * @tc.desc: UpdateSuperFoldRefreshRate test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateSuperFoldRefreshRate, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t tempRefreshRate = 60;
    sptr<ScreenSession> screenSession = nullptr;
    ssm_->UpdateSuperFoldRefreshRate(screenSession, tempRefreshRate);
    EXPECT_TRUE(g_errLog.find("screenSession is nullptr") != std::string::npos);

    ScreenId screenId = 1050;
    DMRect area{0, 0, 600, 800};
    screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    sptr<ScreenSession> fakescreenSession = nullptr;
    screenSession->SetFakeScreenSession(fakescreenSession);
    ssm_->UpdateSuperFoldRefreshRate(screenSession, tempRefreshRate);
    EXPECT_TRUE(g_errLog.find("fakeScreenSession is nullptr") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: OnVerticalChangeBoundsWhenSwitchUser
 * @tc.desc: OnVerticalChangeBoundsWhenSwitchUser test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnVerticalChangeBoundsWhenSwitchUser, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 1050;
    DMRect area{0, 0, 600, 800};
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    auto rotation = screenProperty.GetRotation();
    constexpr float SECONDARY_ROTATION_90 = 90.0F;
    screenProperty.SetRotation(SECONDARY_ROTATION_90);
    RRect bounds = screenProperty.GetBounds();
    ssm_->OnVerticalChangeBoundsWhenSwitchUser(screenSession, FoldDisplayMode::UNKNOWN);
    RRect afterbounds = screenProperty.GetBounds();
    EXPECT_EQ(bounds.rect_.GetHeight(), bounds.rect_.GetWidth());
}

/**
 * @tc.name: NotifyDisplayChangedByUid
 * @tc.desc: NotifyDisplayChangedByUid test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayChangedByUid, TestSize.Level1)
{
    g_errLog.clear();
    ASSERT_NE(ssm_, nullptr);
    ssm_->screenSessionMap_.clear();
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy = ssm_->screenSessionMap_;
    ssm_->NotifyDisplayChangedByUid(screenSessionMapCopy, DisplayChangeEvent::DISPLAY_SIZE_CHANGED, 2002);
    ssm_->screenSessionMap_.clear();
    ssm_->screenSessionMap_[1051] = nullptr;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy1 = ssm_->screenSessionMap_;
    ssm_->NotifyDisplayChangedByUid(screenSessionMapCopy1, DisplayChangeEvent::DISPLAY_SIZE_CHANGED, 2002);
    EXPECT_TRUE(g_errLog.find("screenSession is nullptr") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayChangedByUidInner
 * @tc.desc: NotifyDisplayChangedByUidInner test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayChangedByUidInner, TestSize.Level1)
{
    g_errLog.clear();
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->NotifyDisplayChangedByUidInner(screenSession->ConvertToDisplayInfo(),
        DisplayChangeEvent::DISPLAY_SIZE_CHANGED, 2002);
    EXPECT_TRUE(g_errLog.find("notify end") != std::string::npos);
}

/**
 * @tc.name: SynchronizePowerStatusPermissionDenied
 * @tc.desc: SynchronizePowerStatus test permission denied.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SynchronizePowerStatusPermissionDenied, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ssm_->SynchronizePowerStatus(state);
    EXPECT_TRUE(g_errLog.find("permission denied!") != std::string::npos);
}

/**
 * @tc.name: SynchronizePowerStatusNotSupport
 * @tc.desc: SynchronizePowerStatus not support state.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SynchronizePowerStatusNotSupport, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ScreenPowerState state = ScreenPowerState::INVALID_STATE;
    ssm_->SynchronizePowerStatus(state);
    EXPECT_TRUE(g_errLog.find("not support!") != std::string::npos);
}

/**
 * @tc.name: SynchronizePowerStatusOk
 * @tc.desc: SynchronizePowerStatus test ok.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SynchronizePowerStatusOk, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ScreenPowerState state = ScreenPowerState::POWER_ON;
#define FOLD_ABILITY_ENABLE
#define POWERMGR_DISPLAY_MANAGER_ENABLE
    ssm_->SynchronizePowerStatus(state);
#undef POWERMGR_DISPLAY_MANAGER_ENABLE
#undef FOLD_ABILITY_ENABLE
    EXPECT_TRUE(g_errLog.find("notify brightness") != std::string::npos);
}

/**
 * @tc.name: GetSuperFoldStatus
 * @tc.desc: GetSuperFoldStatus test SystemCalling false.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSuperFoldStatus01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    SuperFoldStatus superFoldStatus = ssm_->GetSuperFoldStatus();
    EXPECT_TRUE(g_errLog.find("Permission Denied") != std::string::npos);
    EXPECT_EQ(superFoldStatus, SuperFoldStatus::UNKNOWN);
}

/**
 * @tc.name: GetSuperFoldStatus
 * @tc.desc: GetSuperFoldStatus test SystemCalling true.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSuperFoldStatus02, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSystemApp(true);
#define FOLD_ABILITY_ENABLE
    SuperFoldStatus superFoldStatus = ssm_->GetSuperFoldStatus();
    SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
#undef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_EQ(superFoldStatus, SuperFoldStatus::UNKNOWN);
    } else {
        EXPECT_EQ(superFoldStatus, status);
    }
}

/**
 * @tc.name: NotifyScreenMaskAppear
 * @tc.desc: NotifyScreenMaskAppear test SystemCalling false.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyScreenMaskAppear01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    ssm_->NotifyScreenMaskAppear();
    EXPECT_TRUE(g_errLog.find("Permission Denied") != std::string::npos);
}

/**
 * @tc.name: NotifyScreenMaskAppear
 * @tc.desc: NotifyScreenMaskAppear test SystemCalling true.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyScreenMaskAppear02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->NotifyScreenMaskAppear();
    bool hasLog = g_errLog.find("not pc device") != std::string::npos ||
        g_errLog.find("screen mask appeared") != std::string::npos;
    EXPECT_TRUE(hasLog);
}

/**
 * @tc.name: TryToCancelScreenOff01
 * @tc.desc: TryToCancelScreenOff test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TryToCancelScreenOff01, TestSize.Level1)
{
    g_errLog.clear();
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockSessionPermission::MockIsStarByHdcd(false);
    ssm_->TryToCancelScreenOff();
    EXPECT_TRUE(g_errLog.find("Permission denied!") != std::string::npos);
}

/**
 * @tc.name: WaitUpdateAvailableAreaForPc01
 * @tc.desc: WaitUpdateAvailableAreaForPc test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WaitUpdateAvailableAreaForPc01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool temp = ScreenSessionManager::GetInstance().GetPcStatus();
    ScreenSessionManager::GetInstance().SetPcStatus(true);
    ScreenSessionManager::GetInstance().WaitUpdateAvailableAreaForPc();
    EXPECT_TRUE(g_errLog.find("need wait update available area") == std::string::npos);
    g_errLog.clear();
    ScreenSessionManager::GetInstance().SetPcStatus(temp);
}

/**
 * @tc.name: WaitUpdateAvailableAreaForPc02
 * @tc.desc: WaitUpdateAvailableAreaForPc test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WaitUpdateAvailableAreaForPc02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool temp = ScreenSessionManager::GetInstance().GetPcStatus();
    ScreenSessionManager::GetInstance().SetPcStatus(true);
    ScreenSessionManager::GetInstance().needWaitAvailableArea_ = true;
    ScreenSessionManager::GetInstance().WaitUpdateAvailableAreaForPc();
    EXPECT_TRUE(g_errLog.find("wait update available area timeout") != std::string::npos);
    g_errLog.clear();
    ScreenSessionManager::GetInstance().SetPcStatus(temp);
}

/**
 * @tc.name: UpdateAvailableAreaWhenDisplayAdd01
 * @tc.desc: UpdateAvailableArea WhenDisplayAdd notify all
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool temp = ssm_->GetPcStatus();
    ssm_->SetPcStatus(true);
    ssm_->needWaitAvailableArea_ = true;

    ScreenId screenId = 1050;
    DMRect area{0, 0, 600, 900};
    DMRect area2{0, 0, 600, 800};
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ssm_->screenSessionMap_[screenId] = screenSession;
    auto screenSession1 = ssm_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession1, screenSession);
    EXPECT_TRUE(screenSession->UpdateAvailableArea(area));
    ssm_->UpdateAvailableArea(screenId, area2);
    EXPECT_FALSE(ssm_->needWaitAvailableArea_);
    g_errLog.clear();
    ssm_->SetPcStatus(temp);
}

/**
 * @tc.name: UpdateAvailableAreaWhenDisplayAdd02
 * @tc.desc: UpdateAvailableArea WhenDisplayAdd not notify all
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool temp = ssm_->GetPcStatus();
    ssm_->SetPcStatus(true);
    ssm_->needWaitAvailableArea_ = false;

    ScreenId screenId = 1050;
    DMRect area{0, 0, 600, 800};
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ssm_->screenSessionMap_[screenId] = screenSession;
    ssm_->UpdateAvailableArea(screenId, area);
    auto screenSession1 = ssm_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession1->GetAvailabelArea(), area);
    EXPECT_FALSE(ssm_->needWaitAvailableArea_);
    g_errLog.clear();
    ssm_->SetPcStatus(temp);
}

/**
 * @tc.name: ConfigureDpi01
 * @tc.desc: ConfigureDpi01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConfigureDpi01, Function | SmallTest | Level3)
{
    system::SetParameter("persist.sceneboard.ispcmode", "true");
    ScreenSceneConfig::intNumbersConfig_["pcModeDpi"].clear();
    ScreenSceneConfig::intNumbersConfig_["pcModeDpi"].emplace_back(EXCEPTION_DPI);
    ScreenSessionManager::GetInstance().ConfigureDpi();
    EXPECT_FALSE(ScreenSessionManager::GetInstance().densityDpi_ ==
        static_cast<float>(EXCEPTION_DPI) / BASELINE_DENSITY);

    ScreenSceneConfig::intNumbersConfig_["pcModeDpi"].clear();
    ScreenSceneConfig::intNumbersConfig_["pcModeDpi"].emplace_back(PC_MODE_DPI);
    ScreenSessionManager::GetInstance().ConfigureDpi();
    EXPECT_TRUE(ScreenSessionManager::GetInstance().densityDpi_ ==
        static_cast<float>(PC_MODE_DPI) / BASELINE_DENSITY);
    system::SetParameter("persist.sceneboard.ispcmode", "false");
}

/**
 * @tc.name: GetCancelSuspendStatus01
 * @tc.desc: GetCancelSuspendStatus01_BothFalse
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCancelSuspendStatus01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sessionDisplayPowerController_->needCancelNotify_ = false;
    ssm_->sessionDisplayPowerController_->canceledSuspend_ = false;
    EXPECT_FALSE(ssm_->GetCancelSuspendStatus());
}

/**
 * @tc.name: GetCancelSuspendStatus02
 * @tc.desc: GetCancelSuspendStatus02_NeedCancelNotifyTrue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCancelSuspendStatus02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sessionDisplayPowerController_->needCancelNotify_ = true;
    ssm_->sessionDisplayPowerController_->canceledSuspend_ = false;
    EXPECT_TRUE(ssm_->GetCancelSuspendStatus());
}

/**
 * @tc.name: GetCancelSuspendStatus03
 * @tc.desc: GetCancelSuspendStatus03_CanceledSuspendTrue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCancelSuspendStatus03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sessionDisplayPowerController_->needCancelNotify_ = false;
    ssm_->sessionDisplayPowerController_->canceledSuspend_ = true;
    EXPECT_TRUE(ssm_->GetCancelSuspendStatus());
}

/**
 * @tc.name: GetCancelSuspendStatus04
 * @tc.desc: GetCancelSuspendStatus04_BothTrue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCancelSuspendStatus04, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sessionDisplayPowerController_->needCancelNotify_ = true;
    ssm_->sessionDisplayPowerController_->canceledSuspend_ = true;
    EXPECT_TRUE(ssm_->GetCancelSuspendStatus());
}

/**
 * @tc.name: HasSameScreenCastInfo
 * @tc.desc: HasSameScreenCastInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasSameScreenCastInfo01, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 1660;
    ScreenId mainScreenId = 0;
    EXPECT_FALSE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR));
    ssm_->SetScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR);
    EXPECT_TRUE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR));
    ssm_->RemoveScreenCastInfo(screenId);
    EXPECT_FALSE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR));
}
 
/**
 * @tc.name: HasSameScreenCastInfo
 * @tc.desc: HasSameScreenCastInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasSameScreenCastInfo02, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 1660;
    ScreenId mainScreenId1 = 0;
    ScreenId mainScreenId2 = 1;
    ssm_->SetScreenCastInfo(screenId, mainScreenId1, ScreenCombination::SCREEN_MIRROR);
    EXPECT_FALSE(ssm_->HasSameScreenCastInfo(mainScreenId1, mainScreenId2, ScreenCombination::SCREEN_MIRROR));
    EXPECT_FALSE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId2, ScreenCombination::SCREEN_MIRROR));
    EXPECT_FALSE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId1, ScreenCombination::SCREEN_EXPAND));
    EXPECT_TRUE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId1, ScreenCombination::SCREEN_MIRROR));
    ssm_->RemoveScreenCastInfo(screenId);
}
 
/**
 * @tc.name: RemoveScreenCastInfo
 * @tc.desc: RemoveScreenCastInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveScreenCastInfo, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = 1660;
    ScreenId mainScreenId = 0;
    ScreenId screenIdNotExist = 1080;
    ssm_->SetScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR);
    EXPECT_TRUE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR));
    ssm_->RemoveScreenCastInfo(screenIdNotExist);
    EXPECT_TRUE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR));
    ssm_->RemoveScreenCastInfo(screenId);
    EXPECT_FALSE(ssm_->HasSameScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR));
}
 
/**
 * @tc.name: ChangeScreenGroup
 * @tc.desc: ChangeScreenGroup test screenCastInfo has same
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ChangeScreenGroup, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId groupId = 1;
    ScreenId screenId = 4080;
    ScreenId mainScreenId = 0;
    sptr<ScreenSessionGroup> group =
        sptr<ScreenSessionGroup>::MakeSptr(groupId, SCREEN_ID_INVALID, "test", ScreenCombination::SCREEN_MIRROR);
    group->mirrorScreenId_ = mainScreenId;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), 0);
    ssm_->screenSessionMap_[screenId] = screenSession;
    std::vector<ScreenId> screens;
    screens.emplace_back(screenId);
    DMRect region = { 0, 0, 1, 1 };
    Point point;
    std::vector<Point> startPoints;
    startPoints.insert(startPoints.begin(), screens.size(), point);
    ssm_->ChangeScreenGroup(group, screens, startPoints, true, ScreenCombination::SCREEN_MIRROR, region);
    EXPECT_TRUE(g_errLog.find("has not same cast info") != std::string::npos);
    g_errLog.clear();
    ssm_->SetScreenCastInfo(screenId, mainScreenId, ScreenCombination::SCREEN_MIRROR);
    ssm_->ChangeScreenGroup(group, screens, startPoints, true, ScreenCombination::SCREEN_MIRROR, region);
    EXPECT_FALSE(g_errLog.find("has not same cast info") != std::string::npos);
    g_errLog.clear();
    ssm_->RemoveScreenCastInfo(screenId);
}

/**
 * @tc.name: ChangeMirrorScreenConfig
 * @tc.desc: ChangeMirrorScreenConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ChangeMirrorScreenConfig, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId groupId = 1;
    ScreenId screenId = 4080;
    ScreenId mainScreenId = 0;
    sptr<ScreenSessionGroup> group =
        sptr<ScreenSessionGroup>::MakeSptr(groupId, SCREEN_ID_INVALID, "test", ScreenCombination::SCREEN_MIRROR);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), 0);
    ssm_->screenSessionMap_[screenId] = screenSession;
    group->mirrorScreenId_ = mainScreenId;
    DMRect region = { 0, 0, 1, 1 };
    ssm_->ChangeMirrorScreenConfig(group, region, screenSession);
    EXPECT_TRUE(g_errLog.find("convert to rs id failed") != std::string::npos);
    g_errLog.clear();
    ssm_->screenIdManager_.UpdateScreenId(screenId, screenId);
    ssm_->ChangeMirrorScreenConfig(group, region, screenSession);
    EXPECT_TRUE(g_errLog.find("with region") != std::string::npos);
    g_errLog.clear();
    group = nullptr;
    ssm_->ChangeMirrorScreenConfig(group, region, screenSession);
    EXPECT_FALSE(g_errLog.find("with region") != std::string::npos);
    g_errLog.clear();
    screenSession = nullptr;
    ssm_->ChangeMirrorScreenConfig(group, region, screenSession);
    EXPECT_FALSE(g_errLog.find("with region") != std::string::npos);
    g_errLog.clear();
}

/*
 * @tc.name: RegisterSettingDuringCallStateObserver
 * @tc.desc: RegisterSettingDuringCallStateObserver
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RegisterSettingDuringCallStateObserver, Function | SmallTest | Level3)
{
    if (g_isPcDevice) {
        GTEST_SKIP();
    }
    ASSERT_NE(ssm_, nullptr);
    ssm_->RegisterSettingDuringCallStateObserver();
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice() && ScreenSceneConfig::IsSupportDuringCall()) {
        ASSERT_NE(ScreenSettingHelper::duringCallStateObserver_, nullptr);
    } else {
        ASSERT_EQ(ScreenSettingHelper::duringCallStateObserver_, nullptr);
    }
}

/**
 * @tc.name: DoAodExitAndSetPowerTest
 * @tc.desc: DoAodExitAndSetPowerTest
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, DoAodExitAndSetPowerTest, TestSize.level1)
{
    ASSERT_NE(ssm_, nullptr);

    ssm_->DoAodExitAndSetPower(0, ScreenPowerStatus::POWER_STATUS_ON);
    EXPECT_EQ(ssm_->GetScreenPower(0), ScreenPowerState::POWER_ON);
    usleep(100000);
    
    constexpr int maxRetry = 3;
    int retry = 0;
    bool powerState = false;
    while (!powerState && retry < maxRetry) {
        ssm_->DoAodExitAndSetPower(0, ScreenPowerStatus::POWER_STATUS_OFF);
        usleep(100000);
        powerState = ssm_->GetScreenPower(0) == ScreenPowerStatE::POWER_OFF;
        if (powerState)
        {
            break;
        }
        retry++;
    }
    EXPECT_TRUE(powerState);
    ssm_->DoAodExitAndSetPower(0, ScreenPowerStatus::POWER_STATUS_ON);
}

/**
 * @tc.name: AddScreenUnfreezeTask
 * @tc.desc: AddScreenUnfreezeTask test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddScreenUnfreezeTask, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> screenSession = nullptr;
    g_errLog.clear();
    ssm_->AddScreenUnfreezeTask(screenSession, 0);
    EXPECT_TRUE(g_errLog.find("screenSession is nullptr") != std::string::npos);
    g_errLog.clear();
    ScreenId scrrenId = 111;
    screenSession = sptr<ScreenSession>::MakeSptr(scrrenId, ScreenProperty(), 0);
    ssm_->AddScreenUnfreezeTask(screenSession, 1000);
    EXPECT_TRUE(g_errLog.find("boot error and freeze screen over 5 minutes") != std::string::npos);
    g_errLog.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: CheckSetResolutionIsValid
 * @tc.desc: CheckSetResolutionIsValid test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckSetResolutionIsValid, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId screenId = -1;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->CheckSetResolutionIsValid(screenId, 100, 100, 0.5));
    screenId = 111;
    sptr<ScreenSession> screenSession = nullptr;
    ssm_->screenSessionMap_.erase(screenId);
    ssm_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->CheckSetResolutionIsValid(screenId, 100, 100, 0.5));
    screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ssm_->screenSessionMap_.erase(screenId);
    ssm_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    sptr<SupportedScreenModes> mode = nullptr;
    screenSession->modes_ = {mode};
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->CheckSetResolutionIsValid(screenId, 100, 100, 0.5));
    mode = new SupportedScreenModes();
    mode->width_ = 1;
    mode->height_ = 1;
    screenSession->modes_ = {mode};
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->CheckSetResolutionIsValid(screenId, 100, 100, 0.5));
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->CheckSetResolutionIsValid(screenId, 0, 0, 10.5));
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->CheckSetResolutionIsValid(screenId, 1, 1, 10.5));
}

/**
 * @tc.name: UpdateDuringCallState
 * @tc.desc: UpdateDuringCallState
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateDuringCallState, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->UpdateDuringCallState();
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice() && ScreenSceneConfig::IsSupportDuringCall()) {
        ASSERT_EQ(ssm_->duringCallState_, 0);
    } else {
        EXPECT_TRUE(g_errLog.find("get setting during call state failed") == std::string::npos);
    }
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetDuringCallState
 * @tc.desc: SetDuringCallState
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDuringCallState, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool value = false;
    ssm_->SetDuringCallState(value);
    EXPECT_TRUE(g_errLog.find("set during call state to") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: LockLandExtendIfScreenInfoNullNull
 * @tc.desc: LockLandExtendIfScreenInfoNullNull
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, LockLandExtendIfScreenInfoNull01, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
#define FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        sptr<ScreenSession> session = ssm_->GetOrCreateScreenSession(SCREENID);
        ScreenProperty property;
        ssm_->CreateScreenProperty(SCREENID, property);
        ssm_->phyScreenPropMap_[SCREENID] = property;
        EXPECT_EQ(session, nullptr);
        ssm_->SetClient(nullptr);
        ASSERT_EQ(ssm_->GetClientProxy(), nullptr);
        ssm_->LockLandExtendIfScreenInfoNull(session);
    }
#undef FOLD_ABILITY_ENABLE
}

/**
 * @tc.name: GetOldDisplayModeRotation
 * @tc.desc: GetOldDisplayModeRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetOldDisplayModeRotation, TestSize.Level1)
{
    auto foldController = sptr<FoldScreenController>::MakeSptr(ssm_->displayInfoMutex_,
        ssm_->screenPowerTaskScheduler_);
    ASSERT_NE(foldController, nullptr);
    DisplayPhysicalResolution physicalSize_full;
    physicalSize_full.foldDisplayMode_ = FoldDisplayMode::FULL;
    physicalSize_full.physicalWidth_ = 2048;
    physicalSize_full.physicalHeight_ = 2232;
    DisplayPhysicalResolution physicalSize_main;
    physicalSize_main.foldDisplayMode_ = FoldDisplayMode::MAIN;
    physicalSize_main.physicalWidth_ = 1008;
    physicalSize_main.physicalHeight_ = 2232;
    DisplayPhysicalResolution physicalSize_global_full;
    physicalSize_global_full.foldDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    physicalSize_global_full.physicalWidth_ = 3184;
    physicalSize_global_full.physicalHeight_ = 2232;
    ScreenSceneConfig::displayPhysicalResolution_.emplace_back(physicalSize_full);
    ScreenSceneConfig::displayPhysicalResolution_.emplace_back(physicalSize_main);
    ScreenSceneConfig::displayPhysicalResolution_.emplace_back(physicalSize_global_full);
    auto foldPolicy = foldController->GetFoldScreenPolicy(DisplayDeviceType::SECONDARY_DISPLAY_DEVICE);
    ASSERT_NE(foldPolicy, nullptr);
    foldPolicy->lastDisplayMode_ = FoldDisplayMode::MAIN;
    foldController->foldScreenPolicy_ = foldPolicy;
    ssm_->foldScreenController_ = foldController;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::GLOBAL_FULL, 3});
    ssm_->rotationCorrectionMap_ = rotationCorrectionMap;
    Rotation rotation = ssm_->GetOldDisplayModeRotation(FoldDisplayMode::MAIN, Rotation::ROTATION_0);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
    rotation = ssm_->GetOldDisplayModeRotation(FoldDisplayMode::GLOBAL_FULL, Rotation::ROTATION_0);
    if (CORRECTION_ENABLE) {
        EXPECT_EQ(rotation, Rotation::ROTATION_90);
    } else {
        EXPECT_EQ(rotation, Rotation::ROTATION_0);
    }
}
 
/**
 * @tc.name: SwapScreenWeightAndHeight
 * @tc.desc: SwapScreenWeightAndHeight
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwapScreenWeightAndHeight, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> screenSession = nullptr;
    ssm_->SwapScreenWeightAndHeight(screenSession);
    EXPECT_TRUE(g_errLog.find("screenSession is null") != std::string::npos);
    screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenProperty property;
    RRect bounds;
    bounds.rect_.width_ = 100;
    bounds.rect_.height_ = 200;
    property.SetBounds(bounds);
    screenSession->SetScreenProperty(property);
    ssm_->SwapScreenWeightAndHeight(screenSession);
    auto afterBounds = screenSession->GetScreenProperty().GetBounds();
    EXPECT_EQ(afterBounds.rect_.width_, bounds.rect_.height_);
    EXPECT_EQ(afterBounds.rect_.height_, bounds.rect_.width_);
    g_errLog.clear();
}
 
/**
 * @tc.name: HandleScreenRotationAndBoundsWhenSetClient
 * @tc.desc: HandleScreenRotationAndBoundsWhenSetClient
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleScreenRotationAndBoundsWhenSetClient, TestSize.Level1)
{
    auto foldController = sptr<FoldScreenController>::MakeSptr(ssm_->displayInfoMutex_,
        ssm_->screenPowerTaskScheduler_);
    ASSERT_NE(foldController, nullptr);
    auto foldPolicy = foldController->GetFoldScreenPolicy(DisplayDeviceType::SINGLE_DISPLAY_DEVICE);
    ASSERT_NE(foldPolicy, nullptr);
    foldPolicy->lastDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    foldController->foldScreenPolicy_ = foldPolicy;
    ssm_->foldScreenController_ = foldController;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::GLOBAL_FULL, 3});
    ssm_->rotationCorrectionMap_ = rotationCorrectionMap;
    ScreenId id = 123;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(id, ScreenProperty(), 0);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        screenSession->SetRotation(Rotation::ROTATION_0);
        ssm_->HandleScreenRotationAndBoundsWhenSetClient(screenSession);
        EXPECT_EQ(screenSession->GetRotation(), Rotation::ROTATION_0);
        screenSession->SetRotation(Rotation::ROTATION_180);
        ssm_->HandleScreenRotationAndBoundsWhenSetClient(screenSession);
        EXPECT_EQ(screenSession->GetRotation(), Rotation::ROTATION_0);
        screenSession->SetRotation(Rotation::ROTATION_270);
        ssm_->HandleScreenRotationAndBoundsWhenSetClient(screenSession);
        EXPECT_EQ(screenSession->GetRotation(), Rotation::ROTATION_0);
    } else {
        screenSession->SetRotation(Rotation::ROTATION_270);
        ssm_->HandleScreenRotationAndBoundsWhenSetClient(screenSession);
        EXPECT_EQ(screenSession->GetRotation(), Rotation::ROTATION_270);
        screenSession->SetRotation(Rotation::ROTATION_270);
        foldPolicy->lastDisplayMode_ = FoldDisplayMode::UNKNOWN;
        ssm_->HandleScreenRotationAndBoundsWhenSetClient(screenSession);
        EXPECT_EQ(screenSession->GetRotation(), Rotation::ROTATION_270);
        screenSession->SetRotation(Rotation::ROTATION_0);
        ssm_->HandleScreenRotationAndBoundsWhenSetClient(screenSession);
        EXPECT_EQ(screenSession->GetRotation(), Rotation::ROTATION_0);
    }
}
 
/**
 * @tc.name: RemoveRotationCorrection
 * @tc.desc: RemoveRotationCorrection
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveRotationCorrection, TestSize.Level1)
{
    auto afterRotation = ssm_->RemoveRotationCorrection(Rotation::ROTATION_270);
    EXPECT_EQ(afterRotation, Rotation::ROTATION_270);
}

/**
 * @tc.name: RemoveRotationCorrection
 * @tc.desc: RemoveRotationCorrection02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveRotationCorrection02, TestSize.Level1)
{
    auto afterRotation = ssm_->RemoveRotationCorrection(Rotation::ROTATION_270, FoldDisplayMode::MAIN);
    EXPECT_EQ(afterRotation, Rotation::ROTATION_270);
}

/**
 * @tc.name: GetConfigCorrectionByDisplayMode
 * @tc.desc: GetConfigCorrectionByDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetConfigCorrectionByDisplayMode, TestSize.Level1)
{
    ssm_->rotationCorrectionMap_.clear();
    bool correctionEnable = system::GetIntParameter<int32_t>("const.system.sensor_correction_enable", 0) == 1;
    if (correctionEnable) {
        auto rotation = ssm_->GetConfigCorrectionByDisplayMode(FoldDisplayMode::MAIN);
        EXPECT_EQ(rotation, Rotation::ROTATION_0);
        std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
        rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
        ssm_->rotationCorrectionMap_ = rotationCorrectionMap;
        rotation = ssm_->GetConfigCorrectionByDisplayMode(FoldDisplayMode::MAIN);
        EXPECT_EQ(rotation, Rotation::ROTATION_270);
 
    } else {
        auto rotation = ssm_->GetConfigCorrectionByDisplayMode(FoldDisplayMode::MAIN);
        EXPECT_EQ(rotation, Rotation::ROTATION_0);
    }
}
 
/**
 * @tc.name: InitRotationCorrectionMap
 * @tc.desc: rotationCorrectionMap not null
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitRotationCorrectionMap01, TestSize.Level1)
{
    ssm_->rotationCorrectionMap_.clear();
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    ssm_->rotationCorrectionMap_ = rotationCorrectionMap;
    ssm_->InitRotationCorrectionMap("");
    EXPECT_TRUE(ssm_->rotationCorrectionMap_.find(FoldDisplayMode::MAIN) != ssm_->rotationCorrectionMap_.end());
    ssm_->rotationCorrectionMap_.clear();
    std::string config = "1,0;2,0;5,3;";
    ssm_->InitRotationCorrectionMap(config);
    EXPECT_EQ(ssm_->rotationCorrectionMap_[FoldDisplayMode::GLOBAL_FULL], 3);
}
 
/**
 * @tc.name: InitRotationCorrectionMap
 * @tc.desc: split failed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitRotationCorrectionMap02, TestSize.Level1)
{
    ssm_->rotationCorrectionMap_.clear();
    ssm_->InitRotationCorrectionMap("");
    EXPECT_TRUE(ssm_->rotationCorrectionMap_.empty());
    ssm_->rotationCorrectionMap_.clear();
    std::string config = ";;";
    ssm_->InitRotationCorrectionMap(config);
    EXPECT_TRUE(ssm_->rotationCorrectionMap_.empty());
}
 
/**
 * @tc.name: InitRotationCorrectionMap
 * @tc.desc: InitRotationCorrectionMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitRotationCorrectionMap03, TestSize.Level1)
{
    ssm_->rotationCorrectionMap_.clear();
    std::string config = "1,0,1;1,0;2,0;5,3;3,a;a,a";
    ssm_->InitRotationCorrectionMap(config);
    EXPECT_EQ(ssm_->rotationCorrectionMap_[FoldDisplayMode::GLOBAL_FULL], 3);
}

/**
 * @tc.name: AddOrUpdateUserDisplayNode
 * @tc.desc: AddOrUpdateUserDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddOrUpdateUserDisplayNode, TestSize.Level1)
{
    RSDisplayNodeConfig config;
    auto node = std::make_shared<RSDisplayNode>(config);
    ASSERT_NE(ssm_, nullptr);
    ssm_->AddOrUpdateUserDisplayNode(100, 1, node);

    auto userIt = ssm_->userDisplayNodeMap_.find(100);
    ASSERT_NE(userIt, ssm_->userDisplayNodeMap_.end());
    
    auto& screenMap = userIt->second;
    auto screenIt = screenMap.find(1);
    ASSERT_NE(screenIt, screenMap.end());
    EXPECT_EQ(screenIt->second, node);
}

/**
 * @tc.name: RemoveUserDisplayNode
 * @tc.desc: RemoveUserDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveUserDisplayNode, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t user1 = 100;
    int32_t user2 = 200;
    ScreenId screen1 = 1;
    ScreenId screen2 = 2;
    RSDisplayNodeConfig config;
    auto node1 = std::make_shared<RSDisplayNode>(config);
    auto node2 = std::make_shared<RSDisplayNode>(config);
    ssm_->userDisplayNodeMap_[user1].insert_or_assign(screen1, node1);
    ssm_->RemoveUserDisplayNode(user2, screen1);
    EXPECT_EQ(ssm_->userDisplayNodeMap_.size(), 1);
    
    ssm_->userDisplayNodeMap_[user2].insert_or_assign(screen2, node2);
    ssm_->RemoveUserDisplayNode(user2, screen1);
    EXPECT_EQ(ssm_->userDisplayNodeMap_.size(), 2);

    ssm_->userDisplayNodeMap_[user2].insert_or_assign(screen1, node2);
    ssm_->RemoveUserDisplayNode(user2, screen1);
    auto findRes = ssm_->userDisplayNodeMap_.find(user2);
    EXPECT_NE(findRes, ssm_->userDisplayNodeMap_.end());
    EXPECT_EQ(findRes->second.find(screen1), findRes->second.end());

    ssm_->RemoveUserDisplayNode(user2, screen2);
    EXPECT_EQ(ssm_->userDisplayNodeMap_.find(user2), ssm_->userDisplayNodeMap_.end());
}

/**
 * @tc.name: GetUserDisplayNodeMap
 * @tc.desc: GetUserDisplayNodeMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetUserDisplayNodeMap, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t user1 = 100;
    ScreenId screen1 = 1;
    RSDisplayNodeConfig config;
    auto node1 = std::make_shared<RSDisplayNode>(config);
    ssm_->userDisplayNodeMap_[user1].insert_or_assign(screen1, node1);
    auto res = ssm_->GetUserDisplayNodeMap(user1);
    auto it = res.find(screen1);
    ASSERT_NE(it, res.end());
    ASSERT_EQ(it->second, node1);
}

/**
 * @tc.name: SwitchUserDealUserDisplayNode
 * @tc.desc: SwitchUserDealUserDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchUserDealUserDisplayNode, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t userId = 100;
    ScreenId sc1 = 1;
    ScreenId sc2 = 2;
    ScreenId sc3 = 3;
    ScreenId sc4 = 4;

    auto realSession1 = sptr<ScreenSession>::MakeSptr();
    realSession1->SetScreenId(sc1);
    realSession1->SetIsRealScreen(true);
    
    auto realSession2 = sptr<ScreenSession>::MakeSptr();
    realSession2->SetScreenId(sc2);
    realSession2->SetIsRealScreen(true);
    
    auto virtualSession = sptr<ScreenSession>::MakeSptr();
    virtualSession->SetScreenId(sc3);
    virtualSession->SetIsRealScreen(false);

    ssm_->screenSessionMap_[sc1] = realSession1;
    ssm_->screenSessionMap_[sc2] = realSession2;
    ssm_->screenSessionMap_[sc3] = virtualSession;

    auto& userNodeMap = ssm_->userDisplayNodeMap_[userId];
    RSDisplayNodeConfig config;
    auto presetNode = std::make_shared<RSDisplayNode>(config);
    userNodeMap[sc1] = presetNode;
    userNodeMap[sc4] = presetNode;
    ssm_->SwitchUserDealUserDisplayNode(userId);

    // 验证结果
    auto& resultMap = ssm_->userDisplayNodeMap_[userId];
    EXPECT_EQ(resultMap[sc1], presetNode);
    EXPECT_NE(resultMap.find(sc2), resultMap.end());
    EXPECT_NE(resultMap[sc2], nullptr);
    EXPECT_NE(resultMap[sc2], presetNode);
    EXPECT_EQ(resultMap.find(sc4), resultMap.end());
    EXPECT_EQ(resultMap.find(sc3), resultMap.end());
}

/**
 * @tc.name: AddUserDisplayNodeOnTree
 * @tc.desc: AddUserDisplayNodeOnTree
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddUserDisplayNodeOnTree, TestSize.Level1)
{
    g_errLog.clear();
    int32_t userId = 100;
    ScreenId screenId1 = 1;
    ScreenId screenId2 = 2;
    RSDisplayNodeConfig config;
    auto node1 = std::make_shared<RSDisplayNode>(config);
    std::shared_ptr<RSDisplayNode> node2 = nullptr;
    ssm_->userDisplayNodeMap_[userId].insert_or_assign(screenId1, node1);
    ssm_->userDisplayNodeMap_[userId].insert_or_assign(screenId2, node2);
    ssm_->AddUserDisplayNodeOnTree(userId);
    
    EXPECT_TRUE(g_errLog.find("userId: ") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: RemoveUserDisplayNodeFromTree
 * @tc.desc: RemoveUserDisplayNodeFromTree
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveUserDisplayNodeFromTree, TestSize.Level1)
{
    int32_t userId = 100;
    ScreenId screenId1 = 1;
    ScreenId screenId2 = 2;
    RSDisplayNodeConfig config;
    auto node1 = std::make_shared<RSDisplayNode>(config);
    std::shared_ptr<RSDisplayNode> node2 = nullptr;
    ssm_->userDisplayNodeMap_[userId].insert_or_assign(screenId1, node1);
    ssm_->userDisplayNodeMap_[userId].insert_or_assign(screenId2, node2);
    ssm_->AddUserDisplayNodeOnTree(userId);
    g_errLog.clear();
    ssm_->RemoveUserDisplayNodeFromTree(userId);
    EXPECT_TRUE(g_errLog.find("userId: ") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: SetUserDisplayNodePositionZ
 * @tc.desc: SetUserDisplayNodePositionZ
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetUserDisplayNodePositionZ, TestSize.Level1)
{
    g_errLog.clear();
    int32_t userId = 100;
    float positionZ = 2.0f;
    ScreenId screenId1 = 1;
    ScreenId screenId2 = 2;
    RSDisplayNodeConfig config;
    auto node1 = std::make_shared<RSDisplayNode>(config);
    std::shared_ptr<RSDisplayNode> node2 = nullptr;
    ssm_->userDisplayNodeMap_[userId].insert_or_assign(screenId1, node1);
    ssm_->userDisplayNodeMap_[userId].insert_or_assign(screenId2, node2);
    ssm_->SetUserDisplayNodePositionZ(userId, positionZ);
    
    ASSERT_TRUE(g_errLog.find("positionZ:") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: HandleNewUserDisplayNode
 * @tc.desc: HandleNewUserDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleNewUserDisplayNode, TestSize.Level1) {
    ASSERT_NE(ssm_, nullptr);
    int32_t oldUserId = 100;
    int32_t newUserId = 200;
    ssm_->currentUserId_ = oldUserId;
    g_errLog.clear();

    RSDisplayNodeConfig config;
    auto oldUserNode = std::make_shared<Rosen::RSDisplayNode>(config);
    auto newUserNode = std::make_shared<Rosen::RSDisplayNode>(config);

    ssm_->userDisplayNodeMap_[oldUserId][1] = oldUserNode;
    float position = 5.0f;
    oldUserNode->SetPositionZ(position);
    newUserNode->SetPositionZ(position);
    ssm_->HandleNewUserDisplayNode(newUserId, false);
    EXPECT_TRUE(g_errLog.find("deal with userDisplayNode") != std::string::npos);
    g_errLog.clear();
    EXPECT_TRUE(ssm_->userDisplayNodeMap_[newUserId].size() > 0);
}

/**
 * @tc.name: HandleFoldDeviceScreenConnect
 * @tc.desc: HandleFoldDeviceScreenConnect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleFoldDeviceScreenConnect, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 0;
    ssm_->isCoordinationFlag_ = false;
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    sptr<ScreenSession> session = nullptr;
    ssm_->HandleFoldDeviceScreenConnect(screenId, session, false, screenEvent);
    EXPECT_FALSE(g_errLog.find("event: connect") != std::string::npos);
    g_errLog.clear();

    session = sptr<ScreenSession>::MakeSptr();
    ssm_->clientProxy_ = nullptr;
    ssm_->HandleFoldDeviceScreenConnect(screenId, session, false, screenEvent);
    EXPECT_FALSE(g_errLog.HandleFoldDeviceScreenConnect("event: connect") != std::string::npos);
    g_errLog.clear();
 
    ssm_->clientProxy_ = sptr<ScreenSessionManagerClientTest>::MakeSptr();
    ssm_->HandleFoldDeviceScreenConnect(screenId, session, false, screenEvent);
    EXPECT_TRUE(g_errLog.find("event: connect") != std::string::npos);
    g_errLog.clear();
 
    screenId = 1;
    ssm_->HandleFoldDeviceScreenConnect(screenId, session, true, screenEvent);
    EXPECT_FALSE(g_errLog.find("event: connect") != std::string::npos);
    g_errLog.clear();
 
    screenId = SCREEN_ID_MAIN;
    ssm_->HandleFoldDeviceScreenConnect(screenId, session, true, screenEvent);
    EXPECT_FALSE(g_errLog.find("event: connect") != std::string::npos);
    g_errLog.clear();
    
    ssm_->isCoordinationFlag_ = true;
    ssm_->HandleFoldDeviceScreenConnect(screenId, session, true, screenEvent);
    EXPECT_TRUE(g_errLog.find("event: connect") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: FirstSCBConnect
 * @tc.desc: FirstSCBConnect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, FirstSCBConnect, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetFirstSCBConnect(true);
    EXPECT_TRUE(ssm_->GetFirstSCBConnect());
    ssm_->SetFirstSCBConnect(false);
    EXPECT_FALSE(ssm_->GetFirstSCBConnect());
}
/**
 * @tc.name: HandleResolutionEffectChangeWhenRotate
 * @tc.desc: HandleResolutionEffectChangeWhenRotate
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleResolutionEffectChangeWhenRotate, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    LOG_SetCallback(MyLogCallback);
    g_errLog.clear();

    sptr<ScreenSession> screenSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetIsCurrentInUse(true);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->isInternal_ = true;

    ssm_->HandleResolutionEffectChangeWhenRotate();
    EXPECT_TRUE(g_errLog.find("Internal Session null") != std::string::npos);
    g_errLog.clear();

    screenSession->property_.UpdateScreenRotation(Rotation::ROTATION_90);
    ssm_->screenSessionMap_[51] = screenSession;
    ssm_->HandleResolutionEffectChangeWhenRotate();
    EXPECT_TRUE(g_errLog.find("start") != std::string::npos);
    g_errLog.clear();

    screenSession->property_.UpdateScreenRotation(Rotation::ROTATION_0);
    ssm_->HandleResolutionEffectChangeWhenRotate();
    EXPECT_TRUE(g_errLog.find("recovery inner and external screen resolution") != std::string::npos);
    g_errLog.clear();
    ssm_->screenSessionMap_.erase(51);
}

/**
 * @tc.name: CalculateTargetResolution
 * @tc.desc: CalculateTargetResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateTargetResolution1, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession1 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->property_.SetScreenRealWidth(3120);
    screenSession1->property_.SetScreenRealHeight(2080);
    screenSession1->property_.UpdateScreenRotation(Rotation::ROTATION_0);
    sptr<ScreenSession> screenSession2 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->property_.SetScreenRealWidth(1920);
    screenSession2->property_.SetScreenRealHeight(0);
    uint32_t width = 0;
    uint32_t height = 0;

    ssm_->CalculateTargetResolution(screenSession1, screenSession2, false, width, height);
    EXPECT_EQ(width, 0);
    EXPECT_EQ(height, 0);

    screenSession2->property_.SetScreenRealHeight(1080);
    ssm_->CalculateTargetResolution(screenSession1, screenSession2, false, width, height);
    EXPECT_EQ(width, 3120);
    EXPECT_EQ(height, 2080);
    EXPECT_FALSE(ssm_->curResolutionEffectEnable_);

    ssm_->CalculateTargetResolution(screenSession1, screenSession2, true, width, height);
    EXPECT_EQ(width, 3120);
    EXPECT_EQ(height, 1755);
    EXPECT_TRUE(ssm_->curResolutionEffectEnable_);

    screenSession2->property_.SetScreenRealWidth(1280);
    screenSession2->property_.SetScreenRealHeight(1024);
    ssm_->CalculateTargetResolution(screenSession1, screenSession2, true, width, height);
    EXPECT_EQ(width, 2600);
    EXPECT_EQ(height, 2080);

    screenSession1->property_.SetScreenRealWidth(2080);
    screenSession1->property_.SetScreenRealHeight(3120);
    screenSession1->property_.UpdateScreenRotation(Rotation::ROTATION_90);
    ssm_->CalculateTargetResolution(screenSession1, screenSession2, true, width, height);
    EXPECT_EQ(width, 2080);
    EXPECT_EQ(height, 2600);
}

/**
 * @tc.name: HandleResolutionEffectChange
 * @tc.desc: HandleResolutionEffectChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleResolutionEffectChange, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (!g_isPcDevice) {
        GTEST_SKIP();
    }
    sptr<ScreenSession> screenSession1 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->SetIsCurrentInUse(true);
    screenSession1->SetScreenType(ScreenType::REAL);
    screenSession1->isInternal_ = true;

    sptr<ScreenSession> screenSession2 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetIsCurrentInUse(true);
    screenSession2->SetScreenType(ScreenType::REAL);
    screenSession2->isInternal_ = false;
    auto ret = ssm_->HandleResolutionEffectChange();
    EXPECT_FALSE(ret);

    ssm_->screenSessionMap_[51] = screenSession1;
    ssm_->screenSessionMap_[52] = screenSession2;
    sptr<ScreenSession> externalSession = nullptr;
    sptr<ScreenSession> internalSession = nullptr;
    ssm_->GetInternalAndExternalSession(internalSession, externalSession);
    EXPECT_EQ(internalSession, screenSession1);
    EXPECT_EQ(externalSession, screenSession2);

    screenSession2->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ret = ssm_->HandleResolutionEffectChange();
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_FALSE(ret);
        screenSession1->property_.UpdateScreenRotation(Rotation::ROTATION_90);
        ret = ssm_->HandleResolutionEffectChange();
        if (ssm_->GetSuperFoldStatus() != SuperFoldStatus::EXPANDED) {
            EXPECT_FALSE(ret);
        } else {
            EXPECT_TRUE(ret);
        }
    } else {
        EXPECT_TRUE(ret);
    }

    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
}

/**
 * @tc.name: SetResolutionEffect
 * @tc.desc: SetResolutionEffect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetResolutionEffect, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (!g_isPcDevice) {
        GTEST_SKIP();
    }
    sptr<ScreenSession> screenSession1 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->SetIsCurrentInUse(true);
    screenSession1->SetScreenType(ScreenType::REAL);
    screenSession1->isInternal_ = true;

    sptr<ScreenSession> screenSession2 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetIsCurrentInUse(true);
    screenSession2->SetScreenType(ScreenType::REAL);
    screenSession2->isInternal_ = false;

    ssm_->screenSessionMap_[51] = screenSession1;
    ssm_->screenSessionMap_[52] = screenSession2;
    sptr<ScreenSession> externalSession = nullptr;
    sptr<ScreenSession> internalSession = nullptr;
    ssm_->GetInternalAndExternalSession(internalSession, externalSession);
    EXPECT_EQ(internalSession, screenSession1);
    EXPECT_EQ(externalSession, screenSession2);

    auto ret = ssm_->SetResolutionEffect(51, 3120, 2080);
    EXPECT_FALSE(ret);

    ret = ssm_->SetResolutionEffect(52, 3120, 2080);
    EXPECT_FALSE(ret);

    screenSession2->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ssm_->curResolutionEffectEnable_ = true;
    ret = ssm_->SetResolutionEffect(51, 3120, 2080);
    EXPECT_TRUE(ret);
    EXPECT_EQ(screenSession1->GetScreenProperty().GetBounds().rect_.width_, 3120);
    EXPECT_EQ(screenSession1->GetScreenProperty().GetBounds().rect_.height_, 2080);
    EXPECT_EQ(screenSession2->GetScreenProperty().GetMirrorWidth(), 3120);
    EXPECT_EQ(screenSession2->GetScreenProperty().GetMirrorHeight(), 2080);
    ssm_->curResolutionEffectEnable_ = false;
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
}

/**
 * @tc.name: SetResolutionEffect
 * @tc.desc: SetResolutionEffect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RecoveryResolutionEffect, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (!g_isPcDevice) {
        GTEST_SKIP();
    }
    sptr<ScreenSession> screenSession1 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->SetIsCurrentInUse(true);
    screenSession1->SetScreenType(ScreenType::REAL);
    screenSession1->isInternal_ = true;

    sptr<ScreenSession> screenSession2 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetIsCurrentInUse(true);
    screenSession2->SetScreenType(ScreenType::REAL);
    screenSession2->isInternal_ = false;

    ssm_->screenSessionMap_[51] = screenSession1;
    ssm_->screenSessionMap_[52] = screenSession2;
    sptr<ScreenSession> externalSession = nullptr;
    sptr<ScreenSession> internalSession = nullptr;
    ssm_->GetInternalAndExternalSession(internalSession, externalSession);
    ASSERT_EQ(internalSession, screenSession1);
    ASSERT_EQ(externalSession, screenSession2);

    auto ret = ssm_->RecoveryResolutionEffect();
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_FALSE(ret);
        screenSession1->property_.UpdateScreenRotation(Rotation::ROTATION_90);
        ret = ssm_->RecoveryResolutionEffect();
        EXPECT_EQ(!ret, ssm_->GetSuperFoldStatus() != SuperFoldStatus::EXPANDED);
    } else {
        EXPECT_TRUE(ret);
        ssm_->curResolutionEffectEnable_ = true;
        ret = ssm_->RecoveryResolutionEffect();
        EXPECT_TRUE(ret);
        EXPECT_FALSE(ssm_->curResolutionEffectEnable_);
        EXPECT_EQ(screenSession1->GetScreenProperty().GetBounds().rect_.width_,
            screenSession1->GetScreenProperty().GetScreenRealWidth());
        EXPECT_EQ(screenSession1->GetScreenProperty().GetBounds().rect_.height_,
            screenSession1->GetScreenProperty().GetScreenRealHeight());
        EXPECT_EQ(screenSession2->GetScreenProperty().GetMirrorWidth(), 0);
        EXPECT_EQ(screenSession2->GetScreenProperty().GetMirrorHeight(), 0);
    }
    ssm_->curResolutionEffectEnable_ = false;
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
}

/**
 * @tc.name: SetInternalScreenResolutionEffect001
 * @tc.desc: SetInternalScreenResolutionEffect001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetInternalScreenResolutionEffect001, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);

    DMRect targetRect = {0, 10, 3120, 2080};
    ssm_->SetInternalScreenResolutionEffect(screenSession, targetRect);
    auto screenProperty = screenSession->GetScreenProperty();
    EXPECT_EQ(screenProperty.GetBounds().rect_.width_, 3120);
    EXPECT_EQ(screenProperty.GetBounds().rect_.height_, 2080);
    EXPECT_EQ(screenProperty.GetMirrorWidth(), 3120);
    EXPECT_EQ(screenProperty.GetMirrorHeight(), 2080);
    EXPECT_EQ(screenProperty.GetValidWidth(), 3120);
    EXPECT_EQ(screenProperty.GetValidHeight(), 2080);
    EXPECT_EQ(screenProperty.GetInputOffsetY(), 10);

    DMRect targetRect2 = {0, 10, 3120, 1755};
    ssm_->SetInternalScreenResolutionEffect(screenSession, targetRect2);
    screenProperty = screenSession->GetScreenProperty();
    EXPECT_EQ(screenProperty.GetBounds().rect_.width_, 3120);
    EXPECT_EQ(screenProperty.GetBounds().rect_.height_, 1755);
    EXPECT_EQ(screenProperty.GetMirrorWidth(), 3120);
    EXPECT_EQ(screenProperty.GetMirrorHeight(), 1755);
    EXPECT_EQ(screenProperty.GetValidWidth(), 3120);
    EXPECT_EQ(screenProperty.GetValidHeight(), 1755);
    EXPECT_EQ(screenProperty.GetInputOffsetY(), 10);
}

/**
 * @tc.name: SetInternalScreenResolutionEffect002
 * @tc.desc: SetInternalScreenResolutionEffect002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetInternalScreenResolutionEffect002, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    sptr<ScreenSession> screenSession = nullptr;
    DMRect targetRect = {0, 10, 3120, 2080};
    ssm_->SetInternalScreenResolutionEffect(screenSession, targetRect);
    EXPECT_TRUE(g_errLog.find("internalSession null") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: SetExternalScreenResolutionEffect001
 * @tc.desc: SetExternalScreenResolutionEffect001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExternalScreenResolutionEffect001, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
 
    sptr<ScreenSession> screenSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetRSScreenId(51);

    DMRect targetRect1 = {0, 10, 3120, 2080};
    ssm_->curResolutionEffectEnable_ = false;
    ssm_->SetExternalScreenResolutionEffect(screenSession, targetRect1);
    auto screenProperty = screenSession->GetScreenProperty();
    EXPECT_EQ(screenProperty.GetMirrorWidth(), 0);
    EXPECT_EQ(screenProperty.GetMirrorHeight(), 0);
    DMRect expectedRect1 = {0, 0, 0, 0};
    EXPECT_EQ(screenSession->GetMirrorScreenRegion().second, expectedRect1);

    sptr<ScreenSession> phyScreenSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, phyScreenSession);
    phyScreenSession->SetRSScreenId(51);
    ssm_->physicalScreenSessionMap_[51] = phyScreenSession;
    ssm_->curResolutionEffectEnable_ = true;
    DMRect targetRect2 = {0, 10, 3120, 1755};
    ssm_->SetExternalScreenResolutionEffect(screenSession, targetRect2);
    auto physcreenProperty = screenSession->GetScreenProperty();
    EXPECT_EQ(physcreenProperty.GetMirrorWidth(), 3120);
    EXPECT_EQ(physcreenProperty.GetMirrorHeight(), 1755);
    EXPECT_EQ(screenSession->GetMirrorScreenRegion().second, targetRect2);
 
    ssm_->physicalScreenSessionMap_.erase(51);
    ssm_->curResolutionEffectEnable_ = false;
}
 
/**
 * @tc.name: SetExternalScreenResolutionEffect002
 * @tc.desc: SetExternalScreenResolutionEffect002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExternalScreenResolutionEffect002, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
 
    sptr<ScreenSession> screenSession = nullptr;
    DMRect targetRect = {0, 10, 3120, 2080};
    ssm_->SetExternalScreenResolutionEffect(screenSession, targetRect);
    EXPECT_TRUE(g_errLog.find("externalSession null") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: HandleCastVirtualScreenMirrorRegion
 * @tc.desc: HandleCastVirtualScreenMirrorRegion
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleCastVirtualScreenMirrorRegion, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (!g_isPcDevice) {
        GTEST_SKIP();
    }
    sptr<ScreenSession> virtualSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, virtualSession);
    virtualSession->SetVirtualScreenFlag(VirtualScreenFlag::CAST);
    virtualSession->SetMirrorScreenType(MirrorScreenType::VIRTUAL_MIRROR);

    sptr<ScreenSession> internalSession = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, internalSession);
    internalSession->SetScreenType(ScreenType::REAL);
    internalSession->isInternal_ = true;
    RRect Bounds = RRect({ 0, 0, 100, 100}, 0.0f, 0.0f);
    internalSession->SetBounds(Bounds);
    internalSession->SetScreenAreaWidth(100);
    internalSession->SetScreenAreaHeight(100);
    bool ret = ssm_->HandleCastVirtualScreenMirrorRegion();
    EXPECT_FALSE(ret);

    ssm_->screenSessionMap_[51] = virtualSession;
    ssm_->screenSessionMap_[52] = internalSession;
    ret = ssm_->HandleCastVirtualScreenMirrorRegion();
    EXPECT_TRUE(ret);

    internalSession->SetRotation(Rotation::ROTATION_180);
    ret = ssm_->HandleCastVirtualScreenMirrorRegion();
    EXPECT_TRUE(ret);

    internalSession->SetRotation(Rotation::ROTATION_270);
    ret = ssm_->HandleCastVirtualScreenMirrorRegion();
    EXPECT_TRUE(ret);
    DMRect expectedRect1 = {0, 0, 0, 0};
    EXPECT_EQ(virtualSession->GetMirrorScreenRegion().second, expectedRect1);
    
    ssm_->curResolutionEffectEnable_ = true;
    ret = ssm_->HandleCastVirtualScreenMirrorRegion();
    DMRect expectedRect2 = {0, 0, 100, 100};
    EXPECT_EQ(virtualSession->GetMirrorScreenRegion().second, expectedRect2);
    ssm_->curResolutionEffectEnable_ = false;
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
}

/**
 * @tc.name: GetCastVirtualMirrorSession
 * @tc.desc: GetCastVirtualMirrorSession
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCastVirtualMirrorSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> virtualSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, virtualSession);
    virtualSession->SetVirtualScreenFlag(VirtualScreenFlag::CAST);
    virtualSession->SetMirrorScreenType(MirrorScreenType::VIRTUAL_MIRROR);

    sptr<ScreenSession> screenSession = nullptr;
    ssm_->GetCastVirtualMirrorSession(screenSession);
    EXPECT_EQ(screenSession, nullptr);
    ssm_->screenSessionMap_[51] = virtualSession;
    ssm_->GetCastVirtualMirrorSession(screenSession);
    EXPECT_EQ(screenSession, virtualSession);

    ssm_->screenSessionMap_.erase(51);
}

/**
 * @tc.name: SwitchModeOffScreenRenderingResetScreenProperty
 * @tc.desc: SwitchModeOffScreenRenderingResetScreenProperty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchModeOffScreenRenderingResetScreenProperty, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    sptr<ScreenSession> session = nullptr;
    g_errLog.clear();
    ssm_->SwitchModeOffScreenRenderingResetScreenProperty(session, true);
    EXPECT_TRUE(g_errLog.find("externalScreenSession is nullptr") != std::string::npos);
    session = sptr<ScreenSession>::MakeSptr();
    g_errLog.clear();
    ssm_->SwitchModeOffScreenRenderingResetScreenProperty(session, true);
    EXPECT_FALSE(g_errLog.find("SetExtendProperty screenId") != std::string::npos);
    g_errLog.clear();
    ssm_->SwitchModeOffScreenRenderingResetScreenProperty(session, false);
    EXPECT_TRUE(g_errLog.find("SetExtendProperty screenId") != std::string::npos);
    LOG_SetCallback(nullptr);
}
 
/**
 * @tc.name: SwitchModeOffScreenRenderingAdapter
 * @tc.desc: SwitchModeOffScreenRenderingAdapter
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchModeOffScreenRenderingAdapter, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    std::vector<ScreenId> externalScreenIds;
    g_errLog.clear();
    ssm_->SwitchModeOffScreenRenderingAdapter(externalScreenIds);
    EXPECT_TRUE(g_errLog.find("externalScreenIds is empty") != std::string::npos);
    g_errLog.clear();
    ScreenId screenId = 666;
    externalScreenIds.emplace_back(screenId);
    ssm_->SwitchModeOffScreenRenderingAdapter(externalScreenIds);
    EXPECT_FALSE(g_errLog.find("externalScreenIds is empty") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetBrightnessInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetBrightnessInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBrightnessInfo start";
    ScreenBrightnessInfo brightnessInfo;
    ssm_->GetBrightnessInfo(0, brightnessInfo);
    EXPECT_NE(brightnessInfo.currentHeadroom, 0);
    EXPECT_NE(brightnessInfo.maxHeadroom, 0);
    EXPECT_NE(brightnessInfo.sdrNits, 0);
    GTEST_LOG_(INFO) << "GetBrightnessInfo end";
}

/**
 * @tc.name: GetSupportsInput
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSupportsInput, TestSize.Level1)
{
    bool supportInput;
    auto ret = ssm_->GetSupportsInput(0, supportInput);
    EXPECT_EQ(ret, DMError::DM_ERROR_ILLEGAL_PARAM);
}

/**
 * @tc.name: SetSupportsInput
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetSupportsInput, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetSupportsInput start";
    bool supportInput = false;
    auto ret = ssm_->SetSupportsInput(0, supportInput);
    EXPECT_EQ(ret, DMError::DM_ERROR_ILLEGAL_PARAM);
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetBundleName, TestSize.Level1)
{
    std::string bundleName = "";
    DisplayId illegalDisplay = 10000;
    auto ret = ssm_->GetBundleName(illegalDisplay, bundleName);
    EXPECT_EQ(ret, DMError::DM_ERROR_ILLEGAL_PARAM);
}

/**
 * @tc.name: MockFoldDisplayModeAfterRotation
 * @tc.desc: test get and set foldDisplayModeAfterRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, MockFoldDisplayModeAfterRotation, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetFoldDisplayModeAfterRotation(FoldDisplayMode::FULL);
    EXPECT_EQ(ssm_->GetFoldDisplayModeAfterRotation(), FoldDisplayMode::FULL);
}

/**
 * @tc.name: HandleDefaultMultiScreenModeTest1
 * @tc.desc: Test HandleDefaultMultiScreenMode when session null
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleDefaultMultiScreenModeTest1, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    sptr<ScreenSession> Session1 = nullptr;
    sptr<ScreenSession> Session2 = nullptr;
    sptr<ScreenSession> Session3 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, Session3);

    ssm_->HandleDefaultMultiScreenMode(Session1, Session2);
    EXPECT_TRUE(g_errLog.find("Session is nullptr") != std::string::npos);
    g_errLog.clear();

    ssm_->HandleDefaultMultiScreenMode(Session3, Session2);
    EXPECT_TRUE(g_errLog.find("Session is nullptr") != std::string::npos);
    g_errLog.clear();

    ssm_->HandleDefaultMultiScreenMode(Session2, Session3);
    EXPECT_TRUE(g_errLog.find("Session is nullptr") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: HandleDefaultMultiScreenModeTest2
 * @tc.desc: Test HandleDefaultMultiScreenMode with same rsid screen Session
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleDefaultMultiScreenModeTest2, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    sptr<ScreenSession> Session1 = new ScreenSession(51, ScreenProperty(), 0);
    sptr<ScreenSession> Session2 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, Session1);
    ASSERT_NE(nullptr, Session2);

    ssm_->HandleDefaultMultiScreenMode(Session1, Session2);
    EXPECT_TRUE(g_errLog.find("same rsId") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: HandleDefaultMultiScreenModeTest3
 * @tc.desc: Test HandleDefaultMultiScreenMode with same rsid screen Session
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleDefaultMultiScreenModeTest3, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    sptr<ScreenSession> Session1 = new ScreenSession(51, ScreenProperty(), 0);
    sptr<ScreenSession> Session2 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, Session1);
    ASSERT_NE(nullptr, Session2);
    Session1->rsId_ = 51;
    Session2->rsId_ = 52;
    ssm_->HandleDefaultMultiScreenMode(Session1, Session2);
    if (IS_SUPPORT_PC_MODE) {
        EXPECT_TRUE(g_errLog.find("default mode mirror") != std::string::npos);
    } else {
        EXPECT_TRUE(g_errLog.find("default mode extend") != std::string::npos);
    }
    g_errLog.clear();
}

/**
 * @tc.name: SetScreenPowerForAll
 * @tc.desc: SetScreenPowerForAll test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll, TestSize.Level1)
{
    ScreenTransitionState temp = ScreenStateMachine::GetInstance().GetTransitionState();
    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY);
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
    ScreenPowerState state = ScreenPowerState::POWER_SUSPEND;
    EXPECT_FALSE(ssm_->SetScreenPowerForAll(state, reason));

    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY);
    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
    EXPECT_FALSE(ssm_->SetScreenPowerForAll(state, reason));

    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY);
    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    EXPECT_FALSE(ssm_->SetScreenPowerForAll(state, reason));
    ScreenStateMachine::GetInstance().SetTransitionState(temp);
}

/**
 * @tc.name: WakeupBegin
 * @tc.desc: WakeupBegin test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WakeUpBegin01, TestSize.Level1)
{
    ScreenTransitionState temp = ScreenStateMachine::GetInstance().GetTransitionState();
    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::SCREEN_ADVANCED_ON);
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
    EXPECT_FALSE(ssm_->WakeUpBegin(reason));
    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::SCREEN_ADVANCED_ON);
    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    EXPECT_FALSE(ssm_->WakeUpBegin(reason));
    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::SCREEN_ADVANCED_ON);
    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
    EXPECT_FALSE(ssm_->WakeUpBegin(reason));
    ScreenStateMachine::GetInstance().SetTransitionState(temp);
}

/*
 * @tc.name: CheckNeedNotifyTest
 * @tc.desc: Test CheckNeedNotifyTest check notify
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckNeedNotifyTest, TestSize.Level1)
{
    std::vector<DisplayId> displayIds = {1, 2, 3};
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId;
    
    bool result = ssm_->CheckNeedNotify(dispalyIds, privacyBundleDisplayId);
    EXPECT_FALSE(result);

    privacyBundleDisplayId = {{4, true}, {5, false}};
    result = ssm_->CheckNeedNotify(dispalyIds, privacyBundleDisplayId);
    EXPECT_FALSE(result);

    privacyBundleDisplayId = {{1, true}, {2, false}};
    result = ssm_->CheckNeedNotify(dispalyIds, privacyBundleDisplayId);
    EXPECT_TRUE(result);

    privacyBundleDisplayId = {{1, true}, {2, true}};
    result = ssm_->CheckNeedNotify(dispalyIds, privacyBundleDisplayId);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: AddVirtualScreenWhiteList_systemCall
 * @tc.desc: AddVirtualScreenWhiteList systemCall
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddVirtualScreenWhiteList_systemCall, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 0;
    const std::vector<uint64_t> missionIds = {5, 10, 15, 20, 25};

    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    DMError result = ScreenSessionManager::GetInstance().AddVirtualScreenWhiteList(screenId, missionIds);
    EXPECT_TRUE(g_errLog.find("Permission Denied") != std::string::npos);
    LOG_SetCallback(nullptr);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
}

/*
 * @tc.name: AddVirtualScreenWhiteList01
 * @tc.desc: Add valid missionIds to whitelist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddVirtualScreenWhiteList01, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddVirtualScreenWhiteList01";
    // create screensession insert into screenSessionMap_
    auto defaultScreenSession = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_.insert(
        std::make_pair(ScreenSessionManager::GetInstance().GetDefaultScreenId(), defaultScreenSession));

    ScreenId virtualScreenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());
    // verify virtualScreenId in screenSessionMap_
    auto it = ScreenSessionManager::GetInstance().screenSessionMap_.find(virtualScreenId);
    EXPECT_NE(it, ScreenSessionManager::GetInstance().screenSessionMap_.end());
    // create mirror
    std::vector<ScreenId> mirrorScreenIds;
    ScreenId mainScreenId(DEFAULT_SCREEN_ID);
    ScreenId screenGroupId{1};
    mirrorScreenIds.push_back(virtualScreenId);
    EXPECT_EQ(DMError::DM_OK,
        ScreenSessionManager::GetInstance().MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId));
    // verify mirror
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(virtualScreenId);
    EXPECT_EQ(screenSession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);

    // add whitelist
    const std::vector<uint64_t> validMissionIds = {5, 10, 15, 20, 25};
    DMError result = ScreenSessionManager::GetInstance().AddVirtualScreenWhiteList(virtualScreenId, validMissionIds);
    EXPECT_EQ(result, DMError::DM_OK);

    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/*
 * @tc.name: AddVirtualScreenWhiteList02
 * @tc.desc: Add empty missionIds to whitelist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddVirtualScreenWhiteList02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddVirtualScreenWhiteList02";
    ScreenId virtualScreenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());

    const std::vector<uint64_t> testMissionIds;
    DMError result = ScreenSessionManager::GetInstance().AddVirtualScreenWhiteList(virtualScreenId, testMissionIds);

    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/*
 * @tc.name: AddVirtualScreenWhiteList03
 * @tc.desc: Add empty missionIds  or nullptr screenId to whitelist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddVirtualScreenWhiteList03, TestSize.Level1)
{
    ScreenId virtualScreenId = INVALID_SCREEN_ID;
    const std::vector<uint64_t> testMissionIds = {5, 10, 15, 20, 25};
    DMError result = ScreenSessionManager::GetInstance().AddVirtualScreenWhiteList(virtualScreenId, testMissionIds);

    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/*
 * @tc.name: AddVirtualScreenWhiteList04
 * @tc.desc: screenSession nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddVirtualScreenWhiteList04, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddVirtualScreenWhiteList04";
    ScreenId virtualScreenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());
    const std::vector<uint64_t> testMissionIds = {5, 10, 15, 20, 25};
    DMError result = ScreenSessionManager::GetInstance().AddVirtualScreenWhiteList(virtualScreenId, testMissionIds);

    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/*
 * @tc.name: RemoveVirtualScreenWhiteList_systemCall
 * @tc.desc: RemoveVirtualScreenWhiteList systemCall
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveVirtualScreenWhiteList_systemCall, Function | SmallTest | Level3)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 0;
    const std::vector<uint64_t> missionIds = {5, 10, 15, 20, 25};

    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    DMError result = ScreenSessionManager::GetInstance().RemoveVirtualScreenWhiteList(screenId, missionIds);
    EXPECT_TRUE(g_errLog.find("Permission Denied") != std::string::npos);
    LOG_SetCallback(nullptr);
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
}

/*
 * @tc.name: RemoveVirtualScreenWhiteList02
 * @tc.desc: Remove invalid screenId from whitelist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveVirtualScreenWhiteList02, TestSize.Level1)
{
    const std::vector<uint64_t> removeMissionIds = {5, 15};
    DMError removeResult = ScreenSessionManager::GetInstance().RemoveVirtualScreenWhiteList(INVALID_SCREEN_ID,
        removeMissionIds);
    EXPECT_EQ(removeResult, DMError::DM_ERROR_INVALID_PARAM);
}

/*
 * @tc.name: RemoveVirtualScreenWhiteList03
 * @tc.desc: Remove empty missionIds from whitelist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveVirtualScreenWhiteList03, TestSize.Level1)
{
    //  create virtual screen and add add missionIds
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "RemoveVirtualScreenWhiteList03";
    ScreenId virtualScreenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());

    // remove missionIds
    const std::vector<uint64_t> removeMissionIds;
    DMError removeResult = ScreenSessionManager::GetInstance().RemoveVirtualScreenWhiteList(virtualScreenId,
        removeMissionIds);
    EXPECT_EQ(removeResult, DMError::DM_ERROR_INVALID_PARAM);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/*
 * @tc.name: RemoveVirtualScreenWhiteList04
 * @tc.desc: screenSession nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveVirtualScreenWhiteList04, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "RemoveVirtualScreenWhiteList04";
    ScreenId virtualScreenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());
    const std::vector<uint64_t> testMissionIds = {5, 10, 15, 20, 25};
    DMError result = ScreenSessionManager::GetInstance().RemoveVirtualScreenWhiteList(virtualScreenId, testMissionIds);

    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/*
 * @tc.name: VirtualScreenWhiteList_UniqueScreen
 * @tc.desc: VirtualScreenWhiteList UniqueScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, VirtualScreenWhiteList_UniqueScreen, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "VirtualScreenWhiteList_UniqueScreen";
    virtualOption.missionIds_ = {3, 4, 5};

    auto defaultScreenSession = new ScreenSession();
    ScreenSessionManager::GetInstance().screenSessionMap_.insert(
        std::make_pair(ScreenSessionManager::GetInstance().GetDefaultScreenId(), defaultScreenSession));

    ScreenId virtualScreenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());
    auto it = ScreenSessionManager::GetInstance().screenSessionMap_.find(virtualScreenId);
    EXPECT_NE(it, ScreenSessionManager::GetInstance().screenSessionMap_.end());
    for (uint32_t i = 10; i < 2000; ++i) {
        virtualOption.missionIds_.emplace_back(i);
    }
    const std::vector<uint64_t> missionIds = virtualOption.missionIds_;
    std::vector<ScreenId> screenIdVector {};
    screenIdVector.push_back(virtualScreenId);
    std::vector<DisplayId> displayIds;
    UniqueScreenRotationOptions rotationOptions;
    rotationOptions.isRotationLocked_ = true;
    rotationOptions.rotation_ = static_cast<int32_t>(Rotation::ROTATION_90);
    // verify make unique screen succeed
    EXPECT_EQ(DMError::DM_OK, ScreenSessionManager::GetInstance().MakeUniqueScreen(screenIdVector,
        displayIds, rotationOptions));
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(virtualScreenId);
    EXPECT_EQ(screenSession->GetScreenCombination(), ScreenCombination::SCREEN_UNIQUE);
    // verify add/remove whitelist in unique screen
    DMError result = ScreenSessionManager::GetInstance().AddVirtualScreenWhiteList(virtualScreenId, missionIds);
    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);

    result = ScreenSessionManager::GetInstance().RemoveVirtualScreenWhiteList(virtualScreenId, missionIds);
    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
    ScreenSessionManager::GetInstance().DestroyVirtualScreen(virtualScreenId);
}

/**
 * @tc.name: NotifyDisplayAttributeChanged
 * @tc.desc: NotifyDisplayAttributeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayAttributeChanged, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
 
    auto displayInfo = ssm_->GetDefaultDisplayInfo();
    std::vector<std::string> attributes = {"rotation", "id"};
    ssm_->NotifyDisplayAttributeChanged(displayInfo, attributes);
    EXPECT_TRUE(g_errLog.find("NotifyDisplayAttributeChanged") != std::string::npos);
}

/**
 * @tc.name: HandleResolutionEffectAfterSwitchUser
 * @tc.desc: HandleResolutionEffectAfterSwitchUser
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleResolutionEffectAfterSwitchUser, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (!g_isPcDevice) {
        GTEST_SKIP();
    }
    LOG_SetCallback(MyLogCallback);
    g_errLog.clear();
 
    sptr<ScreenSession> screenSession = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetIsCurrentInUse(true);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->isInternal_ = true;
 
    ssm_->HandleResolutionEffectAfterSwitchUser();
    EXPECT_TRUE(g_errLog.find("Internal Session null") != std::string::npos);
    g_errLog.clear();
 
    ssm_->screenSessionMap_[51] = screenSession;
    ssm_->HandleResolutionEffectAfterSwitchUser();
    EXPECT_FALSE(g_errLog.find("Internal Session null") != std::string::npos);
    g_errLog.clear();
    ssm_->screenSessionMap_.erase(51);
}
}
}
}