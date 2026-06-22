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
    LOG_SetCallback(nullptr);
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
 * @tc.name: CalcDisplayNodeTranslateOnRotation04
 * @tc.desc: CalcDisplayNodeTranslateOnRotation04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnRotation04, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_270);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->CalcDisplayNodeTranslateOnRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: OnScreenChange
 * @tc.desc: OnScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenChangeReason reason = ScreenChangeReason::HWCDEAD;
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->OnScreenChangeInner(id, screenEvent, reason);
    ssm_->screenSessionMap_.erase(50);
}


/**
 * @tc.name: WaitForDefaultDisplayReady01
 * @tc.desc: Test HasInternalScreen() == true path and while loop exit conditions
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WaitForDefaultDisplayReady01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    ssm_->WaitForDefaultDisplayReady();
    
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "VirtualScreenForWait";
    ScreenId screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    EXPECT_NE(screenId, INVALID_SCREEN_ID);
    EXPECT_NE(ssm_->GetScreenSession(screenId), nullptr);
    
    EXPECT_GE(ssm_->virtualScreenCount_, 1);
    ssm_->WaitForDefaultDisplayReady();
    
    ssm_->ClearAllVirtualScreens();
    
    ssm_->OnScreenChange(INVALID_SCREEN_ID, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
    ssm_->WaitForDefaultDisplayReady();
    ssm_->OnScreenChange(INVALID_SCREEN_ID, ScreenEvent::CONNECTED, ScreenChangeReason::HWCDEAD);
}

/**
 * @tc.name: OnScreenChange01
 * @tc.desc: Test INVALID_SCREEN_ID + CONNECTED with DEFAULT/HWCDEAD reason and g_isVirtualScreenBoot states
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenChange01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    ssm_->OnScreenChange(INVALID_SCREEN_ID, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
    
    ssm_->OnScreenChange(INVALID_SCREEN_ID, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
    
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "VirtualScreenForHWCDEAD";
    ScreenId screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    EXPECT_NE(screenId, INVALID_SCREEN_ID);
    EXPECT_NE(ssm_->GetScreenSession(screenId), nullptr);
    
    EXPECT_GE(ssm_->virtualScreenCount_, 1);
    ssm_->OnScreenChange(INVALID_SCREEN_ID, ScreenEvent::CONNECTED, ScreenChangeReason::HWCDEAD);
}

/**
 * @tc.name: OnScreenChange02
 * @tc.desc: Test physical screen CONNECTED/DISCONNECTED events
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenChange02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    ScreenId physicalScreenId = 1;
    
    ssm_->OnScreenChange(physicalScreenId, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
    ssm_->OnScreenChange(physicalScreenId, ScreenEvent::DISCONNECTED, ScreenChangeReason::DEFAULT);
    
    ScreenId anotherScreenId = 2;
    ssm_->OnScreenChange(anotherScreenId, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
    ssm_->OnScreenChange(anotherScreenId, ScreenEvent::DISCONNECTED, ScreenChangeReason::HWCDEAD);
    
    ssm_->OnScreenChange(physicalScreenId, ScreenEvent::CONNECTED, ScreenChangeReason::HWCDEAD);
    ssm_->OnScreenChange(anotherScreenId, ScreenEvent::DISCONNECTED, ScreenChangeReason::DEFAULT);
}

/**
 * @tc.name: ClearAllVirtualScreens01
 * @tc.desc: Test clearing multiple virtual screens and single screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ClearAllVirtualScreens01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "VirtualScreen1";
    ScreenId screenId1 = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent->AsObject());
    EXPECT_NE(screenId1, INVALID_SCREEN_ID);
    EXPECT_NE(ssm_->GetScreenSession(screenId1), nullptr);
    
    VirtualScreenOption virtualOption2;
    virtualOption2.name_ = "VirtualScreen2";
    ScreenId screenId2 = ssm_->CreateVirtualScreen(virtualOption2, displayManagerAgent->AsObject());
    EXPECT_NE(screenId2, INVALID_SCREEN_ID);
    EXPECT_NE(ssm_->GetScreenSession(screenId2), nullptr);
    
    EXPECT_GE(ssm_->virtualScreenCount_, 2);
    
    ssm_->ClearAllVirtualScreens();
    
    EXPECT_EQ(ssm_->virtualScreenCount_, 0);
    EXPECT_EQ(ssm_->GetScreenSession(screenId1), nullptr);
    EXPECT_EQ(ssm_->GetScreenSession(screenId2), nullptr);
}

/**
 * @tc.name: ClearAllVirtualScreens02
 * @tc.desc: Test empty screenSessionMap and continue branches (nullptr + non-VIRTUAL)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ClearAllVirtualScreens02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    uint32_t beforeCount = ssm_->virtualScreenCount_;
    ssm_->ClearAllVirtualScreens();
    EXPECT_EQ(ssm_->virtualScreenCount_, beforeCount);
    
    ScreenId nullScreenId = 99999;
    ssm_->screenSessionMap_[nullScreenId] = nullptr;
    
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "VirtualScreenForNullTest";
    ScreenId virtualScreenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    EXPECT_NE(virtualScreenId, INVALID_SCREEN_ID);
    EXPECT_NE(ssm_->GetScreenSession(virtualScreenId), nullptr);
    
    EXPECT_GE(ssm_->virtualScreenCount_, 1);
    
    ssm_->ClearAllVirtualScreens();
    
    EXPECT_EQ(ssm_->virtualScreenCount_, 0);
    EXPECT_EQ(ssm_->GetScreenSession(virtualScreenId), nullptr);
    EXPECT_EQ(ssm_->screenIdManager_.HasRsScreenId(virtualScreenId), false);
}

/**
 * @tc.name: ClearAllVirtualScreensAndRecreate
 * @tc.desc: Test clearing virtual screens and recreating zero screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ClearAllVirtualScreensAndRecreate, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "VirtualScreenForRecreate";
    ScreenId screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    EXPECT_NE(screenId, INVALID_SCREEN_ID);
    EXPECT_NE(ssm_->GetScreenSession(screenId), nullptr);
    
    EXPECT_GE(ssm_->virtualScreenCount_, 1);
    
    ssm_->ClearAllVirtualScreens();
    
    EXPECT_EQ(ssm_->virtualScreenCount_, 0);
    EXPECT_EQ(ssm_->GetScreenSession(screenId), nullptr);
    
    ssm_->OnScreenChange(INVALID_SCREEN_ID, ScreenEvent::CONNECTED, ScreenChangeReason::HWCDEAD);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber01
 * @tc.desc: GetCurrentInUseScreenNumber01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->GetCurrentInUseScreenNumber();
    ASSERT_EQ(0, res);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber02
 * @tc.desc: GetCurrentInUseScreenNumber02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsCurrentInUse(false);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->GetCurrentInUseScreenNumber();
    ASSERT_EQ(0, res);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber03
 * @tc.desc: GetCurrentInUseScreenNumber03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->GetCurrentInUseScreenNumber();
    ASSERT_EQ(1, res);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber04
 * @tc.desc: GetCurrentInUseScreenNumber04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber04, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::UNDEFINED);
    screenSession->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->GetCurrentInUseScreenNumber();
    ASSERT_EQ(0, res);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetCastPrivacyFromSettingData01
 * @tc.desc: SetCastPrivacyFromSettingData01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetCastPrivacyFromSettingData01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->SetCastPrivacyFromSettingData();
    ssm_->GetAllDisplayIds();
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend01
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm_->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend02
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::UNDEFINED);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm_->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend03
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(true);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm_->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend04
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend04, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(false);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm_->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio01
 * @tc.desc: SetExtendPixelRatio01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    float dpi = 1.0f;
    ssm_->SetExtendPixelRatio(dpi);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->SetExtendPixelRatio(dpi);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio02
 * @tc.desc: SetExtendPixelRatio02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    float dpi = 1.0f;
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(false);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->SetExtendPixelRatio(dpi);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio03
 * @tc.desc: SetExtendPixelRatio03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    float dpi = 1.0f;
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(true);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->SetExtendPixelRatio(dpi);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio04
 * @tc.desc: SetExtendPixelRatio04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio04, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    float dpi = 1.0f;
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::UNDEFINED);
    screenSession->SetIsInternal(true);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->SetExtendPixelRatio(dpi);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetRotation01
 * @tc.desc: SetRotation01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRotation01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    Rotation rotationAfter = Rotation::ROTATION_0;
    bool isFromWindow = true;
    auto res = ssm_->SetRotation(id, rotationAfter, isFromWindow);
    ASSERT_EQ(false, res);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetRotation02
 * @tc.desc: SetRotation02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRotation02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_90);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    Rotation rotationAfter = Rotation::ROTATION_0;
    bool isFromWindow = true;
    auto res = ssm_->SetRotation(id, rotationAfter, isFromWindow);
    ASSERT_EQ(true, res);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: OnMakeExpand01
 * @tc.desc: OnMakeExpand01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnMakeExpand01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenId = {1001, 1002};
    std::vector<Point> startPoint(1);
    auto res = ssm_->OnMakeExpand(screenId, startPoint);
    ASSERT_EQ(false, res);
    ssm_->screenSessionMap_.erase(0);
}

/**
 * @tc.name: OnMakeExpand02
 * @tc.desc: OnMakeExpand02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnMakeExpand02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->groupSmsId_ = 50;
    ScreenCombination combination =  ScreenCombination::SCREEN_ALONE;
    ScreenId rsId = 1002;
    std::string name = "ok";
    sptr<ScreenSessionGroup> groupSession = new ScreenSessionGroup(50, rsId, name, combination);
    ASSERT_NE(nullptr, groupSession);
    ssm_->smsScreenGroupMap_.insert(std::make_pair(50, groupSession));
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenId = {1001, 1002};
    std::vector<Point> startPoint(1);
    auto res = ssm_->OnMakeExpand(screenId, startPoint);
    ASSERT_EQ(true, res);
    ssm_->screenSessionMap_.erase(0);
    ssm_->smsScreenGroupMap_.erase(50);
}

/**
 * @tc.name: IsExtendMode01
 * @tc.desc: IsExtendMode01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsExtendMode01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->IsExtendMode();
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsExtendMode02
 * @tc.desc: IsExtendMode02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsExtendMode02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm_->IsExtendMode();
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    ssm_->IsExtendMode();
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ssm_->IsExtendMode();
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UpdateCameraBackSelfie01
 * @tc.desc: UpdateCameraBackSelfie01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateCameraBackSelfie01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    bool isCameraBackSelfie = true;
    ssm_->UpdateCameraBackSelfie(isCameraBackSelfie);
    ssm_->UpdateCameraBackSelfie(true);
    ssm_->screenSessionMap_.erase(0);
}

/**
 * @tc.name: UpdateCameraBackSelfie02
 * @tc.desc: UpdateCameraBackSelfie02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateCameraBackSelfie02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    bool isCameraBackSelfie = true;
    ssm_->UpdateCameraBackSelfie(isCameraBackSelfie);
    ssm_->UpdateCameraBackSelfie(false);
    ssm_->screenSessionMap_.erase(0);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror01
 * @tc.desc: HasCastEngineOrPhyMirror01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    ssm_->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror02
 * @tc.desc: HasCastEngineOrPhyMirror02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetName("CastEngine");
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    auto res = ssm_->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ASSERT_EQ(true, res);
    std::vector<ScreenId> idsToExclude = {100, 50};
    ssm_->HasCastEngineOrPhyMirror(idsToExclude);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror03
 * @tc.desc: HasCastEngineOrPhyMirror03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetName("TestCastEngine");
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    auto res = ssm_->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ASSERT_EQ(false, res);
    screenSession->SetScreenType(ScreenType::REAL);
    ssm_->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror04
 * @tc.desc: HasCastEngineOrPhyMirror04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror04, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId id = 9;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetName("CastEngine");
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    ssm_->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ssm_->screenSessionMap_.erase(9);
}

/**
 * @tc.name: HotSwitch
 * @tc.desc: HotSwitch
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HotSwitch, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t newUserId = 0;
    int32_t newScbPid = 1;
    ssm_->HotSwitch(newUserId, newScbPid);
    sptr<IScreenSessionManagerClient> client = nullptr;
    ssm_->clientProxyMap_.insert(std::make_pair(newUserId, client));
    ssm_->HotSwitch(newUserId, newScbPid);
    ssm_->clientProxyMap_.insert(std::make_pair(1, client));
    ssm_->HotSwitch(1, newScbPid);
}

/**
 * @tc.name: SetVirtualScreenStatus
 * @tc.desc: SetVirtualScreenStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenStatus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ASSERT_FALSE(ssm_->SetVirtualScreenStatus(SCREEN_ID_INVALID, VirtualScreenStatus::VIRTUAL_SCREEN_PAUSE));

    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    ASSERT_TRUE(ssm_->SetVirtualScreenStatus(screenId, VirtualScreenStatus::VIRTUAL_SCREEN_PAUSE));
    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
}

/**
 * @tc.name: SetClient
 * @tc.desc: SetClient
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetClient, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IScreenSessionManagerClient> client = nullptr;
    ssm_->SetClient(client);
}

/**
 * @tc.name: SwitchUser
 * @tc.desc: SwitchUser
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchUser, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SwitchUser();
}

/**
 * @tc.name: SetScreenPrivacyMaskImage001
 * @tc.desc: SetScreenPrivacyMaskImage001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPrivacyMaskImage001, TestSize.Level1)
{
    ScreenId screenId = DEFAULT_SCREEN_ID;
    auto ret = ssm_->SetScreenPrivacyMaskImage(screenId, nullptr);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: ScbClientDeathCallback
 * @tc.desc: ScbClientDeathCallback
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScbClientDeathCallback, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t deathScbPid = ssm_->currentScbPId_;
    ssm_->currentScbPId_ = -1;     // INVALID_SCB_PID
    ssm_->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    deathScbPid = ssm_->currentScbPId_;
    ssm_->currentScbPId_ = 0;
    ssm_->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    deathScbPid = 0;
    ssm_->currentScbPId_ = -1;     // INVALID_SCB_PID
    ssm_->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    deathScbPid = 0;
    ssm_->currentScbPId_ = 0;
    ssm_->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);
}

/**
 * @tc.name: NotifyClientProxyUpdateFoldDisplayMode?
 * @tc.desc: NotifyClientProxyUpdateFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyClientProxyUpdateFoldDisplayMode, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    FoldDisplayMode displayMode = FoldDisplayMode::FULL;
    ASSERT_EQ(ssm_->clientProxy_, nullptr);
    ssm_->NotifyClientProxyUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: OnScreenRotationLockedChange
 * @tc.desc: OnScreenRotationLockedChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenRotationLockedChange, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    bool isLocked = false;
    ScreenId screenId = 1050;
    ASSERT_EQ(ssm_->clientProxy_, nullptr);
    ssm_->OnScreenRotationLockedChange(isLocked, screenId);
}

/**
 * @tc.name: OnScreenOrientationChange
 * @tc.desc: OnScreenOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenOrientationChange, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    
    float screenOrientation = 75.2f;
    ScreenId screenId = 1050;
    ASSERT_EQ(ssm_->clientProxy_, nullptr);
    ssm_->OnScreenOrientationChange(screenOrientation, screenId);
}

/**
 * @tc.name: NotifyDisplayModeChanged
 * @tc.desc: NotifyDisplayModeChanged
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayModeChanged, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->NotifyDisplayModeChanged(FoldDisplayMode::MAIN);
}

/**
 * @tc.name: SetMultiScreenMode
 * @tc.desc: MultiScreenMode::SCREEN_EXTEND
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenMode01, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    MultiScreenMode screenMode = MultiScreenMode::SCREEN_EXTEND;
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetScreenType(ScreenType::REAL);

    auto ret = ssm_->SetMultiScreenMode(0, screenSession->GetRSScreenId(), screenMode);
    ASSERT_EQ(ret, DMError::DM_OK);
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: SetMultiScreenMode
 * @tc.desc: MultiScreenMode::SCREEN_MIRROR
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenMode02, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    MultiScreenMode screenMode = MultiScreenMode::SCREEN_MIRROR;
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetScreenType(ScreenType::REAL);

    auto ret = ssm_->SetMultiScreenMode(0, screenSession->GetRSScreenId(), screenMode);
    ASSERT_EQ(ret, DMError::DM_OK);
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: SetMultiScreenMode
 * @tc.desc: operate mode error
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenMode03, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetScreenType(ScreenType::REAL);

    uint32_t testNum = 2;
    auto ret = ssm_->SetMultiScreenMode(0, screenSession->GetRSScreenId(), static_cast<MultiScreenMode>(testNum));
    ASSERT_EQ(ret, DMError::DM_OK);
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: IsOnboardDisplay01
 * @tc.desc: IsOnboardDisplay, Check whether displayInfo is abnormal.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay01, TestSize.Level0)
{
    ssm_->screenSessionMap_.erase(50);
    ASSERT_NE(ssm_, nullptr);
    DisplayId id = 50;
    bool isOnboardDisplay = false;
    auto res = ssm_->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, false);
    EXPECT_EQ(res, DMError::DM_ERROR_INVALID_PARAM);
    ssm_->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsOnboardDisplay02
 * @tc.desc: IsOnboardDisplay, Check pc has board.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay02, TestSize.Level0)
{
    ssm_->screenSessionMap_.erase(0);
    ASSERT_NE(ssm_, nullptr);
    bool isPcNow = ssm_->GetPcStatus();
    ssm_->SetPcStatus(true);
    DisplayId id = 0;
    bool isOnboardDisplay = false;
    ssm_->screenIdManager_.sms2RsScreenIdMap_[id] = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, true);
    EXPECT_EQ(res, DMError::DM_OK);
    ssm_->screenIdManager_.sms2RsScreenIdMap_.clear();
    ssm_->screenSessionMap_.erase(0);
    ssm_->SetPcStatus(isPcNow);
}

/**
 * @tc.name: IsOnboardDisplay03
 * @tc.desc: IsOnboardDisplay, Check pc has no board.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay03, TestSize.Level0)
{
    ssm_->screenSessionMap_.erase(100);
    ASSERT_NE(ssm_, nullptr);
    bool isPcNow = ssm_->GetPcStatus();
    ssm_->SetPcStatus(true);
    DisplayId id = 100;
    bool isOnboardDisplay = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, false);
    EXPECT_EQ(res, DMError::DM_ERROR_INVALID_PARAM);
    ssm_->screenSessionMap_.erase(100);
    ssm_->SetPcStatus(isPcNow);
}

/**
 * @tc.name: IsOnboardDisplay04
 * @tc.desc: IsOnboardDisplay, Check phone/pad has board.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay04, TestSize.Level0)
{
    ssm_->screenSessionMap_.erase(0);
    ASSERT_NE(ssm_, nullptr);
    bool isPcNow = ssm_->GetPcStatus();
    ssm_->SetPcStatus(false);
    DisplayId id = 0;
    bool isOnboardDisplay = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, true);
    EXPECT_EQ(res, DMError::DM_OK);
    ssm_->screenSessionMap_.erase(0);
    ssm_->SetPcStatus(isPcNow);
}

/**
 * @tc.name: IsOnboardDisplay05
 * @tc.desc: IsOnboardDisplay, Check phone/pad has noboard.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay05, TestSize.Level0)
{
    ssm_->screenSessionMap_.erase(100);
    ASSERT_NE(ssm_, nullptr);
    bool isPcNow = ssm_->GetPcStatus();
    ssm_->SetPcStatus(false);
    DisplayId id = 100;
    bool isOnboardDisplay = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm_->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, false);
    EXPECT_EQ(res, DMError::DM_OK);
    ssm_->screenSessionMap_.erase(100);
    ssm_->SetPcStatus(isPcNow);
}

/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: ScreenSession is null
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenRelativePosition01, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    ScreenId testId = 2060;
    ScreenId testId1 = 3060;
    MultiScreenPositionOptions mainScreenOptions = {testId, 100, 100};
    MultiScreenPositionOptions secondScreenOption = {testId1, 100, 100};
    auto ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
#endif
}

/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: B is located on the right side of A
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenRelativePosition02, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent1, nullptr);
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    virtualOption1.width_ = 200;
    virtualOption1.height_ = 100;
    auto screenId1 = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent1->AsObject());

    MultiScreenPositionOptions mainScreenOptions = {screenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {screenId1, 200, 50};
    auto ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, DMError::DM_OK);

    ssm_->DestroyVirtualScreen(screenId);
    ssm_->DestroyVirtualScreen(screenId1);
#endif
}


/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: B is located below A
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenRelativePosition03, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent1, nullptr);
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    virtualOption1.width_ = 200;
    virtualOption1.height_ = 100;
    auto screenId1 = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent1->AsObject());

    MultiScreenPositionOptions mainScreenOptions = {screenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {screenId1, 100, 100};
    auto ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, DMError::DM_OK);

    ssm_->DestroyVirtualScreen(screenId);
    ssm_->DestroyVirtualScreen(screenId1);
#endif
}

/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenRelativePosition04, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent1, nullptr);
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    virtualOption1.width_ = 200;
    virtualOption1.height_ = 100;
    auto screenId1 = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent1->AsObject());

    MultiScreenPositionOptions mainScreenOptions = {screenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {screenId1, 100, 50};
    auto ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        ASSERT_EQ(ret, DMError::DM_OK);
    } else {
        ASSERT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
    }

    ssm_->DestroyVirtualScreen(screenId);
    ssm_->DestroyVirtualScreen(screenId1);
#endif
}

/**
 * @tc.name: NotifyRecordingDisplayChanged
 * @tc.desc: NotifyRecordingDisplayChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyRecordingDisplayChanged, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<DisplayId> displayIds{};
    ssm_->NotifyRecordingDisplayChanged(displayIds);
    EXPECT_TRUE(g_logMsg.find("Agent is empty") != std::string::npos);
    g_logMsg.clear();
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ssm_->RegisterDisplayManagerAgent(displayManagerAgent, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    ssm_->NotifyRecordingDisplayChanged(displayIds);
    EXPECT_TRUE(g_logMsg.find("Agent is empty") == std::string::npos);
}

/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenRelativePosition05, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    virtualOption.width_ = 200;
    virtualOption.height_ = 100;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent1, nullptr);
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    virtualOption1.width_ = 200;
    virtualOption1.height_ = 100;
    auto screenId1 = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent1->AsObject());

    MultiScreenPositionOptions mainScreenOptions = {screenId, 0, 0};
    MultiScreenPositionOptions secondScreenOption = {screenId1, 200, 100};
    auto ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        ASSERT_EQ(ret, DMError::DM_OK);
    } else {
        ASSERT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
    }

    ssm_->DestroyVirtualScreen(screenId);
    ssm_->DestroyVirtualScreen(screenId1);
#endif
}

/**
 * @tc.name: SetMultiScreenRelativePosition
 * @tc.desc: DisplayNode is null
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenRelativePosition06, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    sptr<IDisplayManagerAgent> displayManagerAgent1 = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent1, nullptr);
    VirtualScreenOption virtualOption1;
    virtualOption1.name_ = "createVirtualOption";
    auto screenId1 = ssm_->CreateVirtualScreen(virtualOption1, displayManagerAgent1->AsObject());

    MultiScreenPositionOptions mainScreenOptions = {screenId, 0, 100};
    MultiScreenPositionOptions secondScreenOption = {screenId1, 100, 100};

    sptr<ScreenSession> secondScreenSession = ssm_->GetScreenSession(screenId1);
    secondScreenSession->ReleaseDisplayNode();
    auto ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
    ASSERT_EQ(ret, DMError::DM_OK);

    sptr<ScreenSession> firstScreenSession = ssm_->GetScreenSession(screenId);
    firstScreenSession->ReleaseDisplayNode();
    ret = ssm_->SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);

    ssm_->DestroyVirtualScreen(screenId);
    ssm_->DestroyVirtualScreen(screenId1);
#endif
}

/**
 * @tc.name: SetCoordinationFlag
 * @tc.desc: SetCoordinationFlag
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetCoordinationFlag, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ASSERT_EQ(ssm_->isCoordinationFlag_, false);
    ssm_->SetCoordinationFlag(true);
    ASSERT_EQ(ssm_->isCoordinationFlag_, true);
}
}
} // namespace Rosen
} // namespace OHOS