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

#include "screen_session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "iconsumer_surface.h"
#include "connection/screen_cast_connection.h"
#include "screen_scene_config.h"
#include <surface.h>
#include "scene_board_judgement.h"
#include "fold_screen_state_internel.h"
#include "common_test_utils.h"
#include "iremote_object_mocker.h"
#include "os_account_manager.h"
#include "screen_session_manager_client.h"
#include "../mock/mock_accesstoken_kit.h"
#include "test_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const int32_t CV_WAIT_SCREENOFF_MS = 1500;
const int32_t CV_WAIT_SCREENON_MS = 300;
const int32_t CV_WAIT_SCREENOFF_MS_MAX = 3500;
const uint32_t INVALID_DISPLAY_ORIENTATION = 99;
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
constexpr int32_t CAST_WIRED_PROJECTION_START = 1005;
constexpr int32_t CAST_WIRED_PROJECTION_STOP = 1007;
bool g_isPcDevice = ScreenSceneConfig::GetExternalScreenDefaultMode() == "none";
const bool IS_SUPPORT_PC_MODE = system::GetBoolParameter("const.window.support_window_pcmode_switch", false);
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_logMsg = msg;
}
const bool IS_SUPPORT_PC_MODE = system::GetBoolParameter("const.window.support_window_pcmode_switch", false);
}
class ScreenSessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenSessionManager> ssm_;

    ScreenId DEFAULT_SCREEN_ID {0};
    ScreenId VIRTUAL_SCREEN_ID {2};
    ScreenId VIRTUAL_SCREEN_RS_ID {100};
    int32_t INVALID_USER_ID {1000};
    void SetAceessTokenPermission(const std::string processName);
    sptr<ScreenSession> InitTestScreenSession(std::string name, ScreenId &screenId);
    DMHookInfo CreateDefaultHookInfo();
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

sptr<ScreenSession> ScreenSessionManagerTest::InitTestScreenSession(std::string name, ScreenId &screenId)
{
    sptr displayManagerAgent = new (std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = name;
    screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(name, screenId, rsid, 0);
    return screenSession;
}

DMHookInfo ScreenSessionManagerTest::CreateDefaultHookInfo()
{
    uint32_t hookWidth = 500;
    uint32_t hookHeight = 700;
    float_t hookDensity = 3.0;
    uint32_t hookRotation = static_cast<uint32_t>(Rotation::ROTATION_0);
    uint32_t hookDisplayOrientation = static_cast<uint32_t>(DisplayOrientation::PORTRAIT);
    DMHookInfo dmHookInfo = { hookWidth, hookHeight, hookDensity, hookRotation, true, hookDisplayOrientation, true };
    return dmHookInfo;
}

namespace {
/**
 * @tc.name: NotifyCastWhenSwitchScbNode
 * @tc.desc: NotifyCastWhenSwitchScbNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyCastWhenSwitchScbNode, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);

    ScreenId id = 2;
    sptr<ScreenSession> newSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    newSession ->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ASSERT_NE(newSession, nullptr);
    ssm_->screenSessionMap_[id] = newSession;
    ScreenId id1 = 3;
    sptr<ScreenSession> newSession1 = new (std::nothrow) ScreenSession(id1, ScreenProperty(), 0);
    newSession1 ->SetScreenCombination(ScreenCombination::SCREEN_UNIQUE);
    ASSERT_NE(newSession1, nullptr);
    ssm_->screenSessionMap_[id1] = newSession1;
    ScreenId id2 = 4;
    sptr<ScreenSession> newSession2 = nullptr;
    ssm_->screenSessionMap_[id2] = newSession2;

    ssm_->NotifyCastWhenSwitchScbNode();
}

/**
 * @tc.name: RecoverRestoredMultiScreenMode01
 * @tc.desc: RecoverRestoredMultiScreenMode01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RecoverRestoredMultiScreenMode01, TestSize.Level1)
{
    ScreenId id = 1;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(id, ScreenProperty(), 0);
    screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
    EXPECT_FALSE(ScreenSessionManager::GetInstance().RecoverRestoredMultiScreenMode(screenSession));
}

/**
 * @tc.name: RecoverRestoredMultiScreenMode02
 * @tc.desc: RecoverRestoredMultiScreenMode02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RecoverRestoredMultiScreenMode02, TestSize.Level1)
{
    ScreenId id = 2;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(id, ScreenProperty(), 0);
    screenSession->GetScreenProperty().SetScreenType(ScreenType::UNDEFINED);
    EXPECT_FALSE(ScreenSessionManager::GetInstance().RecoverRestoredMultiScreenMode(screenSession));
}

/**
 * @tc.name: TestIsFreezed_WhenPidNotExist
 * @tc.desc: Test IsFreezed function when agentPid does not exist in freezedPidList_
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TestIsFreezed_WhenPidNotExist, TestSize.Level1)
{
    int32_t agentPid = 1234;
    DisplayManagerAgentType agentType = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;

    EXPECT_FALSE(ScreenSessionManager::GetInstance().IsFreezed(agentPid, agentType));
}

/**
 * @tc.name: TestIsFreezed_WhenPidExistButAgentTypeNotExist
 * @tc.desc: Test IsFreezed function when agentPid exists in freezedPidList_ but not in pidAgentTypeMap_
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TestIsFreezed_WhenPidExistButAgentTypeNotExist, TestSize.Level1)
{
    int32_t agentPid = 1234;
    DisplayManagerAgentType agentType = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;

    ScreenSessionManager::GetInstance().freezedPidList_.insert(agentPid);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().IsFreezed(agentPid, agentType));
    EXPECT_EQ(ScreenSessionManager::GetInstance().pidAgentTypeMap_[agentPid].count(agentType), 1);
}

/**
 * @tc.name: TestIsFreezed_WhenPidAndAgentTypeExist
 * @tc.desc: Test IsFreezed function when agentPid and agentType both exists
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TestIsFreezed_WhenPidAndAgentTypeExist, TestSize.Level1)
{
    int32_t agentPid = 1234;
    DisplayManagerAgentType agentType = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;

    ScreenSessionManager::GetInstance().freezedPidList_.insert(agentPid);
    ScreenSessionManager::GetInstance().pidAgentTypeMap_[agentPid] =
        std::set<DisplayManagerAgentType>({DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER});
    EXPECT_TRUE(ScreenSessionManager::GetInstance().IsFreezed(agentPid, agentType));
    EXPECT_EQ(ScreenSessionManager::GetInstance().pidAgentTypeMap_[agentPid].count(agentType), 1);
}

/**
 * @tc.name: SetScreenOnDelayTime
 * @tc.desc: SetScreenOnDelayTime test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenOnDelayTime, TestSize.Level1)
{
    int32_t delay = CV_WAIT_SCREENON_MS - 1;
    int32_t ret = ssm_->SetScreenOnDelayTime(delay);
    EXPECT_EQ(ret, delay);

    delay = CV_WAIT_SCREENON_MS + 1;
    ret = ssm_->SetScreenOnDelayTime(delay);
    EXPECT_EQ(ret, CV_WAIT_SCREENON_MS);
}

/**
 * @tc.name: ShouldReturnOkWhenMultiScreenNotEnabled
 * @tc.desc: ShouldReturnOkWhenMultiScreenNotEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ShouldReturnOkWhenMultiScreenNotEnabled, TestSize.Level1)
{
    EXPECT_EQ(ScreenSessionManager::GetInstance().VirtualScreenUniqueSwitch({},
        UniqueScreenRotationOptions()),
    DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: ShouldReturnOkWhenMultiScreenNotEnabled
 * @tc.desc: ShouldReturnOkWhenMultiScreenNotEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ShouldReturnNullPtrWhenDefaultScreenIsNull, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ScreenSessionManager ssm;
    ssm = ScreenSessionManager::GetInstance();

    ssm.screenSessionMap_.erase(1100);
    ssm.defaultScreenId_ = 1100;
    EXPECT_EQ(ssm.VirtualScreenYniqueSwitch({}), DMError::DM_ERROR_NULLPTR);
#endif
}

/**
 * @tc.name: ShouldHandleExistingGroupSmsId
 * @tc.desc: ShouldHandleExistingGroupSmsId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ShouldHandleExistingGroupSmsId, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ScreenSessionManager ssm;
    ssm = ScreenSessionManager::GetInstance();

    Sptr<ScreenSession> screenSession = Sptr<ScreenSession>::MakeSptr();
    screenSession->groupSmsId_ = 1001;
    ssm.screenSessionMap_[1100] = screenSession;
    ssm.defaultScreenId_ = 1100;
    ssm.smsSessionGroupMap_[1001] = {};
    EXPECT_EQ(ssm.VirtualScreenYniqueSwitch({}), DMError::DM_OK);
    EXPECT_EQ(ssm.smsSessionGroupMap_.count(1), 0);
    ssm.screenSessionMap_.erase(1100);
#endif
}

/**
 * @tc.name: ShouldHandleNonExistingGroupSmsId
 * @tc.desc: ShouldHandleNonExistingGroupSmsId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ShouldHandleNonExistingGroupSmsId, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ScreenSessionManager ssm;
    ssm = ScreenSessionManager::GetInstance();

    Sptr<ScreenSession> screenSession = Sptr<ScreenSession>::MakeSptr();
    screenSession->groupSmsId_ = 1001;
    ssm.screenSessionMap_[1100] = screenSession;
    ssm.defaultScreenId_ = 1100;
    EXPECT_EQ(ssm.VirtualScreenYniqueSwitch({}), DMError::DM_OK);
    ssm.screenSessionMap_.erase(1100);
#endif
}

/**
 * @tc.name: SetRSScreenPowerStatus
 * @tc.desc: call RS power func, and notify brightness while screen on
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRSScreenPowerStatus, TestSize.Level1)
{
    ScreenPowerState state;
    EXPECT_NE(ssm_, nullptr);

    ssm_->SetRSScreenPowerStatusExt(0, ScreenPowerStatus::POWER_STATUS_ON);
    state = ssm_->GetScreenPower(0);
    EXPECT_EQ(state, ScreenPowerState::POWER_ON);

    ssm_->SetRSScreenPowerStatusExt(0, ScreenPowerStatus::POWER_STATUS_OFF);
    state = ssm_->GetScreenPower(0);
    EXPECT_EQ(state, ScreenPowerState::POWER_OFF);
}

/**
 * @tc.name: GetScreenCombination01
 * @tc.desc: GetScreenCombination01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenCombination01, TestSize.Level1)
{
    ScreenId screenId = 1051;
    auto ret = ssm_->GetScreenCombination(screenId);
    EXPECT_EQ(ret, ScreenCombination::SCREEN_ALONE);
}

/**
 * @tc.name: GetScreenCombination02
 * @tc.desc: !screenSession = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenCombination02, TestSize.Level1)
{
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    EXPECT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    auto ret = ssm_->GetScreenCombination(screenId);
    EXPECT_EQ(ret, ScreenCombination::SCREEN_ALONE);
}

/**
 * @tc.name: OnRemoteDied01
 * @tc.desc: OnRemoteDied_ShouldReturnFalse_WhenAgentIsNUllptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnRemoteDied01, TestSize.Level1)
{
    EXPECT_FALSE(ssm_->OnRemoteDied(nullptr));
}

/**
 * @tc.name: OnRemoteDied02
 * @tc.desc: OnRemoteDied_ShouldReturnTrue_WhenAgentNotFound
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnRemoteDied02, TestSize.Level1)
{
    sptr<IRemoteObject> agent = sptr<MockIRemoteObject>::MakeSptr();
    EXPECT_TRUE(ssm_->OnRemoteDied(agent));
}

/**
 * @tc.name: OnRemoteDied03
 * @tc.desc: OnRemoteDied_ShouldRemoveAgent_WhenAgentFoundAndNoScreens
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnRemoteDied03, TestSize.Level1)
{
    sptr<IRemoteObject> agent = sptr<MockIRemoteObject>::MakeSptr();
    ssm_->screenAgentMap_[agent] = {};

    EXPECT_TRUE(ssm_->OnRemoteDied(agent));
}

/**
 * @tc.name: GetExpandAvailableArea
 * @tc.desc: GetExpandAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetExpandAvailableArea, TestSize.Level1)
{
    DMRect area;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->GetExpandAvailableArea(SCREEN_ID_INVALID, area));
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(DMError::DM_OK, ssm_->GetExpandAvailableArea(id, area));
}

/**
 * @tc.name: GetFakePhysicalScreenSession001
 * @tc.desc: Test that the function returns nullptr when g_isPcDevice is false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetFakePhysicalScreenSession001, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenId defScreenId = 2;
    ScreenProperty property;

    auto screenSession = ScreenSessionManager::GetInstance().GetFakePhysicalScreenSession(screenId, defScreenId,
        property);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: CreateFakePhysicalMirrorSessionInner
 * @tc.desc: CreateFakePhysicalMirrorSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateFakePhysicalMirrorSessionInner, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenId defScreenId = 2;
    ScreenProperty property;

    auto screenSession = ScreenSessionManager::GetInstance().CreateFakePhysicalMirrorSessionInner(screenId, defScreenId,
        property);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: GetPhysicalScreenSessionInner
 * @tc.desc: GetPhysicalScreenSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPhysicalScreenSessionInner, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenProperty property;

    auto screenSession = ScreenSessionManager::GetInstance().GetPhysicalScreenSessionInner(screenId, property);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: GetOrCreatePhysicalScreenSession
 * @tc.desc: Test scenario where no existing physical screen session exists and new session creation fails
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetOrCreatePhysicalScreenSession, TestSize.Level1)
{
    ScreenId screenId = 1;
    sptr<ScreenSession> result = ScreenSessionManager::GetInstance().GetOrCreatePhysicalScreenSession(screenId);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetScreenSessionByRsId01
 * @tc.desc: GetScreenSessionByRsId01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenSessionByRsId01, TestSize.Level1)
{
    ssm_->screenSessionMap_.erase(123);
    ScreenId rsScreenId = 123;
    sptr<ScreenSession> newSession = new ScreenSession();
    newSession->SetRSScreenId(rsScreenId);
    ssm_->screenSessionMap_[rsScreenId] = newSession;

    sptr<ScreenSession> result = ssm_->GetScreenSessionByRsId(rsScreenId);
    EXPECT_NE(result, nullptr);
    ssm_->screenSessionMap_.erase(123);
}

/**
 * @tc.name: GetScreenSessionByRsId02
 * @tc.desc: GetScreenSessionByRsId02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenSessionByRsId02, TestSize.Level1)
{
    ssm_->screenSessionMap_.clear();
    ScreenId rsScreenId = 123;
    ssm_->screenSessionMap_[rsScreenId] = nullptr;

    sptr<ScreenSession> result = ssm_->GetScreenSessionByRsId(rsScreenId);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetPhysicalScreenSession001
 * @tc.desc: Test GetPhysicalScreenSession function when screenId is not found in the map
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPhysicalScreenSession001, TestSize.Level1)
{
    ScreenId screenId = 123;
    EXPECT_EQ(ssm_->GetPhysicalScreenSession(screenId), nullptr);
}

/**
 * @tc.name: GetPhysicalScreenSession002
 * @tc.desc: Test GetPhysicalScreenSession function when screenSessionMap is empty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPhysicalScreenSession002, TestSize.Level1)
{
    ssm_->screenAgentMap_.clear();
    ScreenId screenId = 123;
    EXPECT_EQ(ssm_->GetPhysicalScreenSession(screenId), nullptr);
}

/**
 * @tc.name: OnScreenModeChange
 * @tc.desc: OnScreenModeChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenModeChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    ScreenModeChangeEvent screenModeChangeEvent = ScreenModeChangeEvent::UNKNOWN;
    ssm_->OnScreenModeChange(screenModeChangeEvent);
}

/**
 * @tc.name: GetIsFoldStatusLocked/SetIsFoldStatusLocked
 * @tc.desc: GetIsFoldStatusLocked/SetIsFoldStatusLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetIsFoldStatusLocked, Function | SmallTest | Level3)
{
    ssm_->SetIsFoldStatusLocked(true);
    EXPECT_EQ(ssm_->GetIsFoldStatusLocked(), true);

    ssm_->SetIsFoldStatusLocked(false);
    EXPECT_EQ(ssm_->GetIsFoldStatusLocked(), false);
}

/**
 * @tc.name: SetFoldStatusExpandAndLocked
 * @tc.desc: SetFoldStatusExpandAndLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetFoldStatusExpandAndLocked, Function | SmallTest | Level3)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    ssm_->SetFoldStatusExpandAndLocked(true);
    EXPECT_EQ(ssm_->GetIsFoldStatusLocked(), true);

    ssm_->SetFoldStatusExpandAndLocked(false);
    EXPECT_EQ(ssm_->GetIsFoldStatusLocked(), false);
}

/**
 * @tc.name: CheckMultiScreenInfoMap01
 * @tc.desc: Test CheckMultiScreenInfoMap function when the map is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckMultiScreenInfoMap01, TestSize.Level1)
{
    std::map<std::string, MultiScreenInfo> emptyMap;
    EXPECT_FALSE(ScreenSessionManager::GetInstance().CheckMultiScreenInfoMap(emptyMap, "serial123"));
}

/**
 * @tc.name: CheckMultiScreenInfoMap02
 * @tc.desc: Test CheckMultiScreenInfoMap function when the serial number is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckMultiScreenInfoMap02, TestSize.Level1)
{
    std::map<std::string, MultiScreenInfo> nonEmptyMap;
    nonEmptyMap["serial123"] = MultiScreenInfo();
    EXPECT_FALSE(ScreenSessionManager::GetInstance().CheckMultiScreenInfoMap(nonEmptyMap, ""));
}

/**
 * @tc.name: CheckMultiScreenInfoMap03
 * @tc.desc: Test CheckMultiScreenInfoMap function when the serial number is not found in the map.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckMultiScreenInfoMap03, TestSize.Level1)
{
    std::map<std::string, MultiScreenInfo> nonEmptyMap;
    nonEmptyMap["serial123"] = MultiScreenInfo();
    EXPECT_FALSE(ScreenSessionManager::GetInstance().CheckMultiScreenInfoMap(nonEmptyMap, "serial456"));
}

/**
 * @tc.name: CheckMultiScreenInfoMap04
 * @tc.desc: Test CHeckMultiScreenInfoMap function when all checks pass.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckMultiScreenInfoMap04, TestSize.Level1)
{
    std::map<std::string, MultiScreenInfo> nonEmptyMap;
    nonEmptyMap["serial123"] = MultiScreenInfo();
    EXPECT_TRUE(ScreenSessionManager::GetInstance().CheckMultiScreenInfoMap(nonEmptyMap, "serial123"));
}

/**
 * @tc.name: AdaptSuperHorizonalBoot
 * @tc.desc: AdaptSuperHorizonalBoot
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AdaptSuperHorizonalBoot, Function | SmallTest | Level3)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    ScreenSessionConfig config;
    sptr<ScreenSession> session = new ScreenSession(config,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    ScreenId id = 0;
    
    ssm_->AdaptSuperHorizonalBoot(session, id);
    EXPECT_EQ(session->GetRotation(), Rotation::ROTATION_0);
}

/**
 * @tc.name: HandleSuperFoldStatusLocked
 * @tc.desc: HandleSuperFoldStatusLocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleSuperFoldStatusLocked, Function | SmallTest | Level3)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    ssm_->HandleSuperFoldStatusLocked(true);
    EXPECT_EQ(ssm_->GetIsFoldStatusLocked(), true);

    ssm_->HandleSuperFoldStatusLocked(false);
    EXPECT_EQ(ssm_->GetIsFoldStatusLocked(), false);
}

/**
 * @tc.name: HandleMainScreenDisconnect
 * @tc.desc: HandleMainScreenDisconnect test
 * @tc.type: not main screen
 */
HWTEST_F(ScreenSessionManagerTest, HandleMainScreenDisconnect01, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);

    ScreenId id = 1001;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);

    ssm_->HandleMainScreenDisconnect(screenSession);
    EXPECT_EQ(screenSession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);
#endif
}

/**
 * @tc.name: HandleMainScreenDisconnect
 * @tc.desc: HandleMainScreenDisconnect test
 * @tc.type: main screen
 */
HWTEST_F(ScreenSessionManagerTest, HandleMainScreenDisconnect02, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);

    ScreenId id = 1001;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);

    ssm_->HandleMainScreenDisconnect(screenSession);
    if (!g_isPcDevice) {
        EXPECT_EQ(screenSession->GetScreenCombination(), ScreenCombination::SCREEN_MAIN);
    } else {
        EXPECT_EQ(ssm_->GetIsOuterOnlyMode(), false);
    }
#endif
}

/**
 * @tc.name: HandleMainScreenDisconnect
 * @tc.desc: HandleMainScreenDisconnect test
 * @tc.type: main screen
 */
HWTEST_F(ScreenSessionManagerTest, HandleMainScreenDisconnect03, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);

    ScreenId innerId = 1000;
    ScreenId extendId = 1001;
    ScreenSessionConfig innerConfig = {
        .screenId = innerId,
        .rsId = extendId,
        .defaultScreenId = innerId,
    };
    sptr<ScreenSession> innerSession = new ScreenSession(innerConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    ASSERT_NE(nullptr, innerSession);
    innerSession->SetIsInternal(true);
    innerSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    innerSession->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_.insert(std::make_pair(innerId, innerSession));

    ScreenSessionConfig extendConfig = {
        .screenId = extendId,
        .rsId = innerId,
        .defaultScreenId = innerId,
    };
    sptr<ScreenSession> extendSession = new ScreenSession(extendConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    ASSERT_NE(nullptr, extendSession);
    extendSession->SetIsInternal(false);
    extendSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    extendSession->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_.insert(std::make_pair(extendId, extendSession));

    ssm_->HandleMainScreenDisconnect(extendSession);
    if (!g_isPcDevice) {
        EXPECT_EQ(extendSession->GetScreenCombination(), ScreenCombination::SCREEN_MAIN);
    } else {
        EXPECT_EQ(extendSession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);
    }
    ssm_->screenSessionMap_.erase(innerId);
    ssm_->screenSessionMap_.erase(extendId);
#endif
}

/**
 * @tc.name: ResetInternalScreenSession
 * @tc.desc: ResetInternalScreenSession test
 * @tc.type: session null
 */
HWTEST_F(ScreenSessionManagerTest, ResetInternalScreenSession01, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> innerSession = nullptr;
    sptr<ScreenSession> extendSession = nullptr;

    ssm_->ResetInternalScreenSession(innerSession, extendSession);
    EXPECT_EQ(nullptr, innerSession);
    EXPECT_EQ(nullptr, extendSession);
#endif
}

/**
 * @tc.name: ResetInternalScreenSession
 * @tc.desc: ResetInternalScreenSession test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ResetInternalScreenSession02, TestSize.Level1)
{
#ifndef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);

    ScreenId innerId = 1000;
    ScreenId extendId = 1001;
    ScreenSessionConfig innerConfig = {
        .screenId = innerId,
        .rsId = extendId,
        .defaultScreenId = innerId,
    };
    sptr<ScreenSession> innerSession = new ScreenSession(innerConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    ASSERT_NE(nullptr, innerSession);
    innerSession->SetIsInternal(true);
    innerSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    ssm_->screenSessionMap_.insert(std::make_pair(innerId, innerSession));

    ScreenSessionConfig extendConfig = {
        .screenId = extendId,
        .rsId = innerId,
        .defaultScreenId = innerId,
    };
    sptr<ScreenSession> extendSession = new ScreenSession(extendConfig,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    ASSERT_NE(nullptr, extendSession);
    extendSession->SetIsInternal(false);
    extendSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ssm_->screenSessionMap_.insert(std::make_pair(extendId, extendSession));

    ssm_->ResetInternalScreenSession(innerSession, extendSession);
    EXPECT_EQ(innerSession->GetIsInternal(), false);
    EXPECT_EQ(extendSession->GetIsInternal(), true);
    ssm_->screenSessionMap_.erase(innerId);
    ssm_->screenSessionMap_.erase(extendId);
#endif
}

/**
 * @tc.name: NotifyCreatedScreen
 * @tc.desc: NotifyCreatedScreen test
 * @tc.type: NotifyCreatedScreen test
 */
HWTEST_F(ScreenSessionManagerTest, NotifyCreatedScreen, TestSize.Level1)
{
    g_logMsg.clear();
    if (!g_isPcDevice) {
        GTEST_SKIP();
    }
    ASSERT_NE(ssm_, nullptr);
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 1001;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    }
    ssm_->NotifyCreatedScreen(screenSession);
    EXPECT_TRUE(g_logMsg.find("super fold device, change by rotation.") == std::string::npos);
}

/**
 * @tc.name: SetPrimaryDisplaySystemDpi
 * @tc.desc: SetPrimaryDisplaySystemDpi
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetPrimaryDisplaySystemDpi, Function | SmallTest | Level3)
{
    float dpi = 2.2f;
    ssm_->screenSessionMap_.clear();
    DMError ret = ssm_->SetPrimaryDisplaySystemDpi(dpi);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret);

    DisplayId id = 0;
    sptr<ScreenSession> invalidScreenSession = new (std::nothrow) ScreenSession(1, ScreenProperty(), 1);
    ssm_->screenSessionMap_[id] = invalidScreenSession;
    ret = ssm_->SetPrimaryDisplaySystemDpi(dpi);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret);

    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(0, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ret = ssm_->SetPrimaryDisplaySystemDpi(dpi);
    EXPECT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: HandleSwitchPcMode
 * @tc.desc: HandleSwitchPcMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HandleSwitchPcMode, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    if (IS_SUPPORT_PC_MODE) {
        bool isPcDevice = ssm_->HandleSwitchPcMode();
        ASSERT_EQ(isPcDevice, g_isPcDevice);
        GTEST_SKIP();
    }
    bool isPcMode = system::GetBoolParameter("persist.sceneboard.ispcmode", false);
    bool isPcDevice = ssm_->HandleSwitchPcMode();
    if (isPcMode) {
        EXPECT_TRUE(isPcDevice);
    } else {
        EXPECT_FALSE(isPcDevice);
    }
}

/**
 * @tc.name: SwitchModeHandleExternalScreen
 * @tc.desc: SwitchModeHandleExternalScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchModeHandleExternalScreen01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = sptr<DisplayManagerAgentDefault>::MakeSptr();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    ssm_->SwitchModeHandleExternalScreen(false);
    EXPECT_NE(screenSession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);
    screenSession->SetIsRealScreen(true);
    ssm_->screenSessionMap_.insert(std::make_pair(777, nullptr));
    sptr<IDisplayManagerAgent> displayManagerAgent1 = sptr<DisplayManagerAgentDefault>::MakeSptr();
    ASSERT_NE(displayManagerAgent1, nullptr);
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption2";
    auto virtualScreenId = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent1->AsObject());
    ssm_->SwitchModeHandleExternalScreen(false);
    EXPECT_EQ(screenSession->GetName(), "testVirtualOption");
    screenSession->SetIsRealScreen(false);
    ssm_->DestroyVirtualScreen(screenId);
    ssm_->DestroyVirtualScreen(virtualScreenId);
}

/**
 * @tc.name: SwitchModeHandleExternalScreen
 * @tc.desc: SwitchModeHandleExternalScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchModeHandleExternalScreen02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->screenSessionMap_.clear();
    sptr<IDisplayManagerAgent> displayManagerAgent = sptr<DisplayManagerAgentDefault>::MakeSptr();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetIsRealScreen(true);
    ssm_->SwitchModeHandleExternalScreen(true);
    EXPECT_NE(screenSession->GetScreenCombination(), ScreenCombination::SCREEN_MIRROR);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: CreateVirtualScreen
 * @tc.desc: CreateVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateVirtualScreen, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = sptr<DisplayManagerAgentDefault>::MakeSptr();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "CastEngine";
    virtualOption.virtualScreenFlag_ = VirtualScreenFlag::CAST;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    EXPECT_EQ(screenSession->GetVirtualScreenFlag(), VirtualScreenFlag::CAST);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetScreenOffsetFeatureTest
 * @tc.desc: SetScreenOffsetInner
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenOffsetFeatureTest, Function | SmallTest | Level3)
{
    ScreenId screenId = 0;
    EXPECT_FALSE(ssm_->SetScreenOffset(screenId, 0.0F, 0.0F));
    EXPECT_FALSE(ssm_->SetScreenOffset(screenId, 100.0F, 100.0F));
    screenId = -1;
    EXPECT_FALSE(ssm_->SetScreenOffset(screenId, 0.0F, 0.0F));
    EXPECT_FALSE(ssm_->SetScreenOffset(screenId, 100.0F, 100.0F));
}

/**
+ * @tc.name: InitSecondaryDisplayPhysicalParams
+ * @tc.desc: test function : InitSecondaryDisplayPhysicalParams
+ * @tc.type: FUNC
+ */
HWTEST_F(ScreenSessionManagerTest, InitSecondaryDisplayPhysicalParams, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP();
    }
    ASSERT_NE(ssm_, nullptr);
    ssm_->InitSecondaryDisplayPhysicalParams();
    EXPECT_FALSE(ssm_->screenParams_.empty());
}

/**
 * @tc.name: GetPhyScreenId
 * @tc.desc: test function : GetPhyScreenId
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPhyScreenId, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP();
    }
 
    ScreenId screenId = 0;
    ssm_->SetCoordinationFlag(true);
    ASSERT_EQ(ssm_->GetPhyScreenId(screenId), screenId);
 
    screenId = 5;
    ASSERT_EQ(ssm_->GetPhyScreenId(screenId), 0);
}
 
/**
 * @tc.name: UpdateCoordinationRefreshRate
 * @tc.desc: test function : UpdateCoordinationRefreshRate
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateCoordinationRefreshRate, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP();
    }
    uint32_t refreshRate = 60;
    ssm_->SetCoordinationFlag(false);
    ssm_->UpdateCoordinationRefreshRate(refreshRate);
    EXPECT_FALSE(ssm_->GetScreenSession(5));
 
    refreshRate = 90;
    ssm_->SetCoordinationFlag(true);
    ssm_->UpdateCoordinationRefreshRate(refreshRate);
    EXPECT_TRUE(ssm_->GetScreenSession(0));
}

/**
@tc.name: SyncScreenPropertyChangedToServer
@tc.desc: test function : SyncScreenPropertyChangedToServer
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerTest, SyncScreenPropertyChangedToServer, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = nullptr;
    ssm_->screenSessionMap_.erase(screenId);
    ssm_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    DMError ret = ssm_->SyncScreenPropertyChangedToServer(screenId, screenProperty);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ret);
    screenSession = new ScreenSession(screenId, screenProperty, 0);
    ssm_->screenSessionMap_.erase(screenId);
    ssm_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    std::function<void(sptr<ScreenSession> screenSession, SuperFoldStatusChangeEvents changeEvent)> func =
        [](sptr<ScreenSession> screenSession, SuperFoldStatusChangeEvents changeEvent) { return; };
    ssm_->SetPropertyChangedCallback(func);
    ret = ssm_->SyncScreenPropertyChangedToServer(screenId, screenProperty);
    EXPECT_EQ(DMError::DM_OK, ret);
}
/**
@tc.name: SetPropertyChangedCallback
@tc.desc: test function : SetPropertyChangedCallback
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerTest, SetPropertyChangedCallback, TestSize.Level1)
{
    std::function<void(sptr<ScreenSession> screenSession, SuperFoldStatusChangeEvents changeEvent)> func = nullptr;
    ssm_->SetPropertyChangedCallback(func);
    EXPECT_EQ(ssm_->propertyChangedCallback_, nullptr);
    std::function<void(sptr<ScreenSession> screenSession, SuperFoldStatusChangeEvents changeEvent)> func1 =
        [](sptr<ScreenSession> screenSession, SuperFoldStatusChangeEvents changeEvent) { return; };
    ssm_->SetPropertyChangedCallback(func1);
    EXPECT_NE(ssm_->propertyChangedCallback_, nullptr);
}
/**
@tc.name: OnFoldPropertyChange
@tc.desc: test function : OnFoldPropertyChange
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerTest, OnFoldPropertyChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 1;
    ScreenProperty screenProperty;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    ssm_->clientProxy_ = nullptr;
    ssm_->OnFoldPropertyChange(screenId, screenProperty, reason, displayMode);
    EXPECT_TRUE(g_logMsg.find("clientProxy_ is null") != std::string::npos);
    g_logMsg.clear();

    ssm_->clientProxy_ = sptr<ScreenSessionManagerClientTest>::MakeSptr();
    ssm_->OnFoldPropertyChange(screenId, screenProperty, reason, displayMode);
}

/**
 * @tc.name: SetOptionConfig
 * @tc.desc: SetOptionConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetOptionConfig, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    VirtualScreenOption option;
    option.supportsFocus_ = false;
    option.supportsInput_ = false;
    ssm_->SetOptionConfig(screenId, option);
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession->GetSupportsFocus(), false);
    EXPECT_EQ(screenSession->GetSupportsInput(), false);

    option.supportsFocus_ = true;
    option.supportsInput_ = true;
    ssm_->SetOptionConfig(screenId, option);
    screenSession = ssm_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession->GetSupportsFocus(), true);
    EXPECT_EQ(screenSession->GetSupportsInput(), true);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    screenId++;
    ssm_->SetOptionConfig(screenId, option);
    EXPECT_TRUE(g_logMsg.find("screenSession is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}
}
} // namespace Rosen
} // namespace OHOS
