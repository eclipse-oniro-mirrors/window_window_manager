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
 * @tc.name: GetDisplayInfoById
 * @tc.desc: GetDisplayInfoById virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoById, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDisplayInfoById";
    ASSERT_EQ(ssm_->GetDisplayInfoById(1), nullptr);
}

/**
 * @tc.name: GetDisplayInfoByScreen
 * @tc.desc: GetDisplayInfoByScreen virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoByScreen, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDisplayInfoByScreen";
    ASSERT_EQ(ssm_->GetDisplayInfoByScreen(1), nullptr);
}

/**
 * @tc.name: GetScreenInfoById
 * @tc.desc: GetScreenInfoById virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenInfoById, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetScreenInfoById";
    ASSERT_EQ(ssm_->GetScreenInfoById(1), nullptr);
}

/**
 * @tc.name: NotifyDisplayEvent
 * @tc.desc: NotifyDisplayEvent  virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayEvent, TestSize.Level1)
{
    DisplayEvent event = DisplayEvent::KEYGUARD_DRAWN;
    ssm_->NotifyDisplayEvent(event);
    ASSERT_EQ(ssm_->keyguardDrawnDone_, true);

    event = DisplayEvent::SCREEN_LOCK_SUSPEND;
    ssm_->NotifyDisplayEvent(event);
    ASSERT_EQ(ssm_->gotScreenOffNotify_, true);

    event = DisplayEvent::SCREEN_LOCK_OFF;
    ssm_->NotifyDisplayEvent(event);
    ASSERT_EQ(ssm_->gotScreenOffNotify_, true);

    event = DisplayEvent::SCREEN_LOCK_FINGERPRINT;
    ssm_->NotifyDisplayEvent(event);
    ASSERT_EQ(ssm_->gotScreenlockFingerprint_, true);

    ssm_->gotScreenOffNotify_ = false;
    event = DisplayEvent::SCREEN_LOCK_DOZE_FINISH;
    ssm_->NotifyDisplayEvent(event);
    ASSERT_EQ(ssm_->gotScreenOffNotify_, true);
}

/**
 * @tc.name: GetScreenInfoByDisplayId
 * @tc.desc: GetScreenInfoByDisplayId  virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenInfoByDisplayId, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetScreenInfoByDisplayId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenInfo> screenInfo;
    screenInfo = ssm_->GetScreenInfoByDisplayId(screenId);
    ASSERT_NE(screenInfo, nullptr);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetScreenModesByDisplayId
 * @tc.desc: GetScreenModesByDisplayId  virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenModesByDisplayId, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetScreenModesByDisplayId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<SupportedScreenModes> screenModes;
    screenModes = ssm_->GetScreenModesByDisplayId(screenId);
    ASSERT_NE(screenModes, nullptr);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: UpdateDisplayHookInfo001
 * @tc.desc: UpdateDisplayHookInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateDisplayHookInfo001, TestSize.Level1)
{
    uint32_t uid = 20020001;
    DMHookInfo dmHookInfo;
    dmHookInfo.width_ = 100;
    dmHookInfo.height_ = 100;
    dmHookInfo.density_ = 1.0;
    std::map<uint32_t, DMHookInfo> displayHookMap = {};
    displayHookMap[uid] = dmHookInfo;
    ASSERT_EQ(displayHookMap.find(uid) != displayHookMap.end(), true);
    auto info = displayHookMap[uid];
    ASSERT_EQ(info.width_, 100);
    ASSERT_EQ(info.height_, 100);
    ASSERT_EQ(info.density_, 1.0);
}

/**
 * @tc.name: UpdateDisplayHookInfo002
 * @tc.desc: UpdateDisplayHookInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateDisplayHookInfo002, TestSize.Level1)
{
    uint32_t uid = 20020001;
    DMHookInfo dmHookInfo;
    dmHookInfo.width_ = 100;
    dmHookInfo.height_ = 100;
    dmHookInfo.density_ = 1.0;
    std::map<uint32_t, DMHookInfo> displayHookMap = {};
    displayHookMap[uid] = dmHookInfo;
    displayHookMap.erase(uid);
    ASSERT_EQ(displayHookMap.find(uid) != displayHookMap.end(), false);
}

/**
 * @tc.name: GetDisplayHookInfo
 * @tc.desc: GetDisplayHookInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayHookInfo, Function | SmallTest | Level2)
{
    int32_t uid = 10086;
    DMHookInfo hookInfo;
    hookInfo.enableHookRotation_ = true;
    hookInfo.rotation_ = false;
    hookInfo.density_ = 2.5f;
    hookInfo.width_ = 1920;
    hookInfo.height_ = 1080;

    ssm_->UpdateDisplayHookInfo(uid, true, hookInfo);

    DMHookInfo actualInfo;
    actualInfo.enableHookRotation_ = false;
    actualInfo.rotation_ = true;
    actualInfo.density_ = -1.0f;
    actualInfo.width_ = -1;
    actualInfo.height_ = -1;
    ssm_->GetDisplayHookInfo(uid, actualInfo);

    EXPECT_EQ(actualInfo.enableHookRotation_, hookInfo.enableHookRotation_);
    EXPECT_EQ(actualInfo.rotation_, hookInfo.rotation_);
    EXPECT_FLOAT_EQ(actualInfo.density_, hookInfo.density_);
    EXPECT_EQ(actualInfo.width_, hookInfo.width_);
    EXPECT_EQ(actualInfo.height_, hookInfo.height_);
}

/**
 * @tc.name: NotifyIsFullScreenInForceSplitMode
 * @tc.desc: NotifyIsFullScreenInForceSplitMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyIsFullScreenInForceSplitMode, Function | SmallTest | Level2)
{
    int32_t uid = 0;
    MockAccesstokenKit::MockIsSystemApp(false);
    ssm_->NotifyIsFullScreenInForceSplitMode(uid, true);
    MockAccesstokenKit::MockIsSystemApp(true);
    DMHookInfo hookInfo;
    hookInfo.enableHookRotation_ = true;
    hookInfo.rotation_ = true;
    hookInfo.density_ = 1.1;
    hookInfo.width_ = 100;
    hookInfo.height_ = 200;
    ssm_->UpdateDisplayHookInfo(uid, true, hookInfo);
    DMHookInfo hookInfo2;
    ssm_->GetDisplayHookInfo(uid, hookInfo2);
    ASSERT_EQ(hookInfo2.width_, 0);

    uid = 100;
    ssm_->NotifyIsFullScreenInForceSplitMode(uid, true);
    ssm_->UpdateDisplayHookInfo(uid, true, hookInfo);
    ssm_->NotifyIsFullScreenInForceSplitMode(uid, true);
    DMHookInfo hookInfo3;
    ssm_->GetDisplayHookInfo(uid, hookInfo3);
    ASSERT_TRUE(hookInfo3.isFullScreenInForceSplit_);
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: SetVirtualPixelRatio virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualPixelRatio, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetVirtualPixelRatio";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_EQ(DMError::DM_OK, ssm_->SetVirtualPixelRatio(screenId, 0.1));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetVirtualPixelRatioSystem
 * @tc.desc: SetVirtualPixelRatioSystem virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualPixelRatioSystem, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetVirtualPixelRatioSystem";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_EQ(DMError::DM_OK, ssm_->SetVirtualPixelRatioSystem(screenId, 0.1));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetDefaultDensityDpi
 * @tc.desc: SetDefaultDensityDpi virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDefaultDensityDpi, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetDefaultDensityDpi";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_EQ(DMError::DM_OK, ssm_->SetDefaultDensityDpi(screenId, 0.1));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetResolution, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetResolution";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ssm_->screenSessionMap_[screenId] = screenSession;
    sptr<SupportedScreenModes> mode = new SupportedScreenModes();
    mode->width_ = 1;
    mode->height_ = 1;
    screenSession->modes_ = {mode};

    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetResolution(screenId, 100, 100, 0.5));

    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetResolution(screenId, 0, 0, 0.5));

    ScreenId invalidScreenId = -1;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->SetResolution(invalidScreenId, 100, 100, 0.5));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetScreenColorGamut
 * @tc.desc: GetScreenColorGamut virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenColorGamut, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    ScreenColorGamut colorGamut = ScreenColorGamut::COLOR_GAMUT_SRGB;
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenColorGamut(1, colorGamut));
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, ssm_->GetScreenColorGamut(id, colorGamut));
    } else {
        ASSERT_NE(DMError::DM_OK, ssm_->GetScreenColorGamut(id, colorGamut));
    }
#endif
}

/**
 * @tc.name: GetScreenGamutMap
 * @tc.desc: GetScreenGamutMap virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenGamutMap, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetScreenGamutMap";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetScreenGamutMap", screenId, rsid, 0);
    ScreenGamutMap gamutMap;
    ASSERT_EQ(DMError::DM_OK, ssm_->GetScreenGamutMap(screenId, gamutMap));
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: MakeExpand virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, MakeExpand, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "MakeExpand";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    std::vector<ScreenId> mirrorScreenIds;
    std::vector<Point> startPoints;
    ScreenId screenGroupId2 = DISPLAY_ID_INVALID;
    ASSERT_NE(DMError::DM_ERROR_RENDER_SERVICE_FAILED, ssm_->MakeExpand(mirrorScreenIds, startPoints, screenGroupId2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: DeleteScreenId
 * @tc.desc: DeleteScreenId virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, DeleteScreenId, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ScreenSessionManager::ScreenIdManager sim;
    ASSERT_EQ(false, sim.DeleteScreenId(99));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: HasRsScreenId
 * @tc.desc: HasRsScreenId virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasRsScreenId, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "HasRsScreenId";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ScreenSessionManager::ScreenIdManager sim;
    ASSERT_EQ(false, sim.HasRsScreenId(99));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: AddAsFirstScreenLocked
 * @tc.desc: AddAsFirstScreenLocked virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddAsFirstScreenLocked, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddAsFirstScreenLocked";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> newScreen = new (std::nothrow) ScreenSession();
    sptr<ScreenSessionGroup> sessiongroup=nullptr;
    ASSERT_NE(sessiongroup, ssm_->AddAsFirstScreenLocked(newScreen));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: AddAsSuccedentScreenLocked
 * @tc.desc: AddAsSuccedentScreenLocked virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddAsSuccedentScreenLocked, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddAsSuccedentScreenLocked";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> newScreen;
    sptr<ScreenSessionGroup> sessiongroup=nullptr;
    ASSERT_EQ(sessiongroup, ssm_->AddAsSuccedentScreenLocked(newScreen));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetMirror
 * @tc.desc: SetMirror virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMirror, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetMirror";

    std::vector<ScreenId> screens{0, 1, 2, 3, 4, 5, 6, 7};
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new (std::nothrow) ScreenSession("GetScreenGamutMap", 2, 2, 3);
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap_{
        {2, screenSession},
    };
    ssm_->screenSessionMap_ = screenSessionMap_;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto screen = ssm_->GetScreenSession(2);
    screen->GetScreenProperty().SetScreenType(ScreenType::REAL);
    ASSERT_EQ(DMError::DM_OK, ssm_->SetMirror(2, screens, DMRect::NONE(), {Rotation::ROTATION_0, false}));
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->SetMirror(9, screens, DMRect::NONE(), {Rotation::ROTATION_0, false}));
    ASSERT_EQ(DMError::DM_OK, ssm_->SetMirror(screenId, screens, DMRect::NONE(), {Rotation::ROTATION_0, false}));
    auto mirrorscreen = ssm_->GetScreenSession(screenId);
    ASSERT_TRUE(mirrorscreen != nullptr);
    mirrorscreen->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->SetMirror(screenId, screens, DMRect::NONE(),
        {Rotation::ROTATION_0, false}));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetAbstractScreenGroup
 * @tc.desc: GetAbstractScreenGroup virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAbstractScreenGroup, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetAbstractScreenGroup";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSessionGroup> sreengroup=nullptr;
    ASSERT_EQ(sreengroup, ssm_->GetAbstractScreenGroup(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: InitAbstractScreenModesInfo
 * @tc.desc: InitAbstractScreenModesInfo virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitAbstractScreenModesInfo, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "InitAbstractScreenModesInfo";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_EQ(false, ssm_->InitAbstractScreenModesInfo(screenSession));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: AddToGroupLocked
 * @tc.desc: AddToGroupLocked virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddToGroupLocked, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddToGroupLocked";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> newScreen=nullptr;
    sptr<ScreenSessionGroup> sessiongroup=nullptr;
    ASSERT_EQ(sessiongroup, ssm_->AddToGroupLocked(newScreen));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: InitVirtualScreen
 * @tc.desc: InitVirtualScreen virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitVirtualScreen, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "InitVirtualScreen";
    sptr<ScreenSession> screenSession;
    VirtualScreenOption option;
    ASSERT_NE(screenSession, ssm_->InitVirtualScreen(1, 2, option));
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ssm_->InitVirtualScreen(1, 2, option));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: InitAndGetScreen
 * @tc.desc: InitAndGetScreen virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitAndGetScreen, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "InitAndGetScreen";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ssm_->InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: RemoveFromGroupLocked
 * @tc.desc: RemoveFromGroupLocked virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RemoveFromGroupLocked, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "RemoveFromGroupLocked";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> newScreen = new (std::nothrow) ScreenSession();
    sptr<ScreenSessionGroup> sessiongroup;
    ASSERT_EQ(sessiongroup, ssm_->RemoveFromGroupLocked(newScreen));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: CreateAndGetNewScreenId
 * @tc.desc: CreateAndGetNewScreenId virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateAndGetNewScreenId, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "CreateAndGetNewScreenId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ScreenSessionManager::ScreenIdManager sim;
    ASSERT_EQ(1000, sim.CreateAndGetNewScreenId(screenId));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: AddScreenToGroup
 * @tc.desc: AddScreenToGroup virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddScreenToGroup, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "AddScreenToGroup";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    const std::vector<ScreenId> addScreens;
    const std::vector<Point> addChildPos;
    std::map<ScreenId, bool> removeChildResMap;
    sptr<ScreenSessionGroup> group;
    ssm_->AddScreenToGroup(group, addScreens, addChildPos, removeChildResMap, {Rotation::ROTATION_0, false});
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ssm_->InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetRSDisplayNodeByScreenId
 * @tc.desc: GetRSDisplayNodeByScreenId virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetRSDisplayNodeByScreenId, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetRSDisplayNodeByScreenId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ssm_->GetRSDisplayNodeByScreenId(2);
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ssm_->InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetAllDisplayIds
 * @tc.desc: GetAllDisplayIds virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayIds, TestSize.Level0)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetAllDisplayIds";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    std::vector<DisplayId>  displays(2);
    ASSERT_NE(displays, ssm_->GetAllDisplayIds());
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetScreenGamutMap
 * @tc.desc: SetScreenGamutMap virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenGamutMap, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetScreenGamutMap";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("SetScreenGamutMap", screenId, rsid, 0);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM,
              ssm_->SetScreenGamutMap(SCREEN_ID_INVALID, ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION));
    ASSERT_EQ(DMError::DM_OK, ssm_->SetScreenGamutMap(screenId, ScreenGamutMap::GAMUT_MAP_EXTENSION));
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: StopExpand
 * @tc.desc: StopExpand virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, StopExpand, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "StopExpand";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    std::vector<ScreenId> expandScreenIds {0, 1, 2, 3, 4, 5};
    ASSERT_NE(DMError::DM_ERROR_NOT_SYSTEM_APP, ssm_->StopExpand(expandScreenIds));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: OnScreenDisconnect
 * @tc.desc: OnScreenDisconnect virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenDisconnect, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "OnScreenDisconnect";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ssm_->OnScreenDisconnect(1);
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ssm_->InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetScreenColorGamut
 * @tc.desc: SetScreenColorGamut virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenColorGamut, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetScreenColorGamut";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("SetScreenColorGamut", screenId, rsid, 0);
    ASSERT_EQ(DMError::DM_OK, ssm_->SetScreenColorGamut(screenId, 2));
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenColorGamut(SCREEN_ID_INVALID, 2));
    ssm_->DestroyVirtualScreen(screenId);
#endif
}

/**
 * @tc.name: SetScreenColorTransform
 * @tc.desc: SetScreenColorTransform virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenColorTransform, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetScreenColorTransform";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("SetScreenColorTransform", screenId, rsid, 0);
    ASSERT_EQ(DMError::DM_OK, ssm_->SetScreenColorTransform(screenId));
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenColorTransform(SCREEN_ID_INVALID));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: SetScreenRotationLocked virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenRotationLocked, TestSize.Level1)
{
    ScreenId id = 0;
    ssm_->screenSessionMap_[id] = nullptr;
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenRotationLocked(false));
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(DMError::DM_OK, ssm_->SetScreenRotationLocked(false));
}

/**
 * @tc.name: UpdateScreenRotationProperty
 * @tc.desc: UpdateScreenRotationProperty virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateScreenRotationProperty, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "UpdateScreenRotationProperty";
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    int rotation = 1;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_BEGIN;
    ScreenSessionManager::GetInstance().UpdateScreenRotationProperty(1, bounds, 1, screenPropertyChangeType, false);
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ScreenSessionManager::GetInstance().UpdateScreenRotationProperty(screenId, bounds, rotation,
        screenPropertyChangeType, false);
    screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_END;
    ScreenSessionManager::GetInstance().UpdateScreenRotationProperty(screenId, bounds, rotation,
        screenPropertyChangeType, false);
    screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY;
    ScreenSessionManager::GetInstance().UpdateScreenRotationProperty(screenId, bounds, rotation,
        screenPropertyChangeType, true);
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ScreenSessionManager::GetInstance().InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: MakeUniqueScreen
 * @tc.desc: Make unique screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, MakeUniqueScreen, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    vector<ScreenId> screenIds;
    screenIds.clear();
    std::vector<DisplayId> displayIds;
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->MakeUniqueScreen(screenIds, displayIds,
        UniqueScreenRotationOptions()));
#endif
}

/**
 * @tc.name: ConvertScreenIdToRsScreenId
 * @tc.desc: convert screen id to RS screen id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConvertScreenIdToRsScreenId, TestSize.Level1)
{
    ScreenId rsScreenId = VIRTUAL_SCREEN_RS_ID;
    ssm_->screenIdManager_.rs2SmsScreenIdMap_.erase(rsScreenId);
    ScreenId screenId = ssm_->screenIdManager_.CreateAndGetNewScreenId(rsScreenId);
    ASSERT_EQ(true, ssm_->screenIdManager_.HasRsScreenId(rsScreenId));
    ScreenId tmpRsScreenId = SCREEN_ID_INVALID;
    ASSERT_EQ(true, ssm_->ConvertScreenIdToRsScreenId(screenId, tmpRsScreenId));
    ASSERT_EQ(tmpRsScreenId, rsScreenId);
    ssm_->screenIdManager_.DeleteScreenId(screenId);
    ASSERT_EQ(false, ssm_->ConvertScreenIdToRsScreenId(screenId, tmpRsScreenId));
}

/**
 * @tc.name: DisableMirror
 * @tc.desc: DisableMirror test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, DisableMirror, TestSize.Level1)
{
    ASSERT_EQ(DMError::DM_OK, ssm_->DisableMirror(false));
}

/**
 * @tc.name: HasImmersiveWindow
 * @tc.desc: HasImmersiveWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasImmersiveWindow, TestSize.Level1)
{
    bool immersive = false;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->HasImmersiveWindow(0u, immersive));
    immersive = true;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->HasImmersiveWindow(0u, immersive));
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: ScreenSessionManager screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetSpecifiedScreenPower, TestSize.Level1)
{
    ScreenId mainScreenId(DEFAULT_SCREEN_ID);
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    ASSERT_EQ(false, ssm_->SetSpecifiedScreenPower(mainScreenId, state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION;
    EXPECT_TRUE(ssm_->SetSpecifiedScreenPower(mainScreenId, state, reason));
}

/**
 * @tc.name: NotifyFoldStatusChanged
 * @tc.desc: ScreenSessionManager notify foldStatus changed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyFoldStatusChanged, TestSize.Level1)
{
    const std::string& dumpParam = "-p";
    if (ssm_ != nullptr)
    {
        int errCode = ssm_->NotifyFoldStatusChanged(dumpParam);
        ASSERT_EQ(errCode, 0);
    } else {
        ASSERT_EQ(1, 0);
    }
}

/**
 * @tc.name: NotifyPrivateWindowListChanged
 * @tc.desc: ScreenSessionManager notify PrivateWindowList changed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyPrivateWindowListChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr) << "ScreenSessionManager instance is null";

    DisplayId id = 100;
    std::vector<std::string> privacyWindowList{"win0", "win1"};
    ssm_->NotifyPrivateWindowListChanged(id, privacyWindowList);

    EXPECT_TRUE(g_logMsg.find("agent is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetPrivacyStateByDisplayId01
 * @tc.desc: SetPrivacyStateByDisplayId true test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetPrivacyStateByDisplayId01, TestSize.Level1)
{
    DisplayId id = 0;
    bool hasPrivate = true;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId = {{id, hasPrivate}};
    ssm_->SetPrivacyStateByDisplayId(privacyBundleDisplayId);
    bool curHasPrivate = false;
    ssm_->HasPrivateWindow(id, curHasPrivate);
    EXPECT_TRUE(curHasPrivate);
}

/**
 * @tc.name: SetPrivacyStateByDisplayId02
 * @tc.desc: SetPrivacyStateByDisplayId false test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetPrivacyStateByDisplayId02, TestSize.Level1)
{
    DisplayId id = 0;
    bool hasPrivate = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId = {{id, hasPrivate}};
    ssm_->SetPrivacyStateByDisplayId(privacyBundleDisplayId);
    bool curHasPrivate = false;
    ssm_->HasPrivateWindow(id, curHasPrivate);
    EXPECT_FALSE(curHasPrivate);
}

/**
 * @tc.name: SetScreenPrivacyWindowList
 * @tc.desc: SetScreenPrivacyWindowList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPrivacyWindowList, Function | SmallTest | Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);

    DisplayId invalidId = 999;
    std::vector<std::string> windowList{ "win0" };
    ScreenSessionManager::GetInstance().SetScreenPrivacyWindowList(invalidId, windowList);

    std::string expectedKeyword = "Permmission Denied!";
    bool found = (g_logMsg.find(expectedKeyword) != std::string::npos);
    EXPECT_TRUE(found) << "Expected log to contain '" << expectedKeyword
                       << "', but got: " << g_logMsg;

    MockAccesstokenKit::ChangeMockStateToInit();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: CalculateStartWhenTransferState001
 * @tc.desc: CalculateStartWhenTransferState test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateStartWhenTransferState001, Function | SmallTest | Level1)
{
    ScreenId mainScreenId = 1051;
    sptr<ScreenSession> staticSession = sptr<ScreenSession>::MakeSptr(mainScreenId, ScreenProperty(), 0);
    DMRect staticScreenBounds = {{0, 0, 1920, 1080}, 0.0f, 0.0f};
    ssm_->screenSessionMap_.insert(std::make_pair(mainScreenId, staticSession));
    staticSession->SetBounds(staticScreenBounds);
    staticSession->SetRSScreenId(mainScreenId);
    staticSession->SetStartPosition(0, 0);
    ScreenId dynamicScreenId = 1052;
    sptr<ScreenSession> dynamicSession = sptr<ScreenSession>::MakeSptr(dynamicScreenId, ScreenProperty(), 0);
    DMRect dynamicScreenBounds = {{0, 0, 3296, 2472}, 0.0f, 0.0f};
    ssm_->screenSessionMap_.insert(std::make_pair(dynamicScreenId, dynamicSession));
    dynamicSession->SetBounds(dynamicScreenBounds);
    dynamicSession->SetRSScreenId(dynamicScreenId);
    dynamicSession->SetStartPosition(0, 1080);
    uint32_t borderingAreaPercent = 50;
    uint32_t exceptStaticStartX = 0;
    uint32_t exceptStaticStartY = 0;
    uint32_t exceptDynamicStartX = 0;
    uint32_t exceptDynamicStartY = 1080;
    ssm_->CalculateStartWhenTransferState(staticSession, dynamicSession, borderingAreaPercent);
    uint32_t testStaticStartX = staticSession->GetScreenProperty().GetStartX();
    uint32_t testStaticStartY = staticSession->GetScreenProperty().GetStartY();
    uint32_t testDynamicStartX = dynamicSession->GetScreenProperty().GetStartX();
    uint32_t testDynamicStartY = dynamicSession->GetScreenProperty().GetStartY();
    EXPECT_EQ(exceptStaticStartX, testStaticStartX);
    EXPECT_EQ(exceptStaticStartY, testStaticStartY);
    EXPECT_EQ(exceptDynamicStartX, testDynamicStartX);
    EXPECT_EQ(exceptDynamicStartY, testDynamicStartY);
}

/**
 * @tc.name: CalculateStartWhenTransferState002
 * @tc.desc: CalculateStartWhenTransferState test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateStartWhenTransferState002, Function | SmallTest | Level1)
{
    ScreenId mainScreenId = 1051;
    sptr<ScreenSession> staticSession = sptr<ScreenSession>::MakeSptr(mainScreenId, ScreenProperty(), 0);
    DMRect staticScreenBounds = {{0, 0, 1920, 1080}, 0.0f, 0.0f};
    ssm_->screenSessionMap_.insert(std::make_pair(mainScreenId, staticSession));
    staticSession->SetBounds(staticScreenBounds);
    staticSession->SetRSScreenId(mainScreenId);
    staticSession->SetStartPosition(0, 0);
    ScreenId dynamicScreenId = 1052;
    sptr<ScreenSession> dynamicSession = sptr<ScreenSession>::MakeSptr(dynamicScreenId, ScreenProperty(), 0);
    DMRect dynamicScreenBounds = {{0, 0, 3296, 2472}, 0.0f, 0.0f};
    ssm_->screenSessionMap_.insert(std::make_pair(dynamicScreenId, dynamicSession));
    dynamicSession->SetBounds(dynamicScreenBounds);
    dynamicSession->SetRSScreenId(dynamicScreenId);
    dynamicSession->SetStartPosition(1919, 1080);
    uint32_t borderingAreaPercent = 50;
    uint32_t exceptStaticStartX = 0;
    uint32_t exceptStaticStartY = 0;
    uint32_t exceptDynamicStartX = 1919;
    uint32_t exceptDynamicStartY = 1080;
    ssm_->CalculateStartWhenTransferState(staticSession, dynamicSession, borderingAreaPercent);
    uint32_t testStaticStartX = staticSession->GetScreenProperty().GetStartX();
    uint32_t testStaticStartY = staticSession->GetScreenProperty().GetStartY();
    uint32_t testDynamicStartX = dynamicSession->GetScreenProperty().GetStartX();
    uint32_t testDynamicStartY = dynamicSession->GetScreenProperty().GetStartY();
    EXPECT_EQ(exceptStaticStartX, testStaticStartX);
    EXPECT_EQ(exceptStaticStartY, testStaticStartY);
    EXPECT_EQ(exceptDynamicStartX, testDynamicStartX);
    EXPECT_EQ(exceptDynamicStartY, testDynamicStartY);
}

/**
 * @tc.name: CalculateStartWhenTransferState003
 * @tc.desc: CalculateStartWhenTransferState test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateStartWhenTransferState003, Function | SmallTest | Level1)
{
    ScreenId mainScreenId = 1051;
    sptr<ScreenSession> staticSession = sptr<ScreenSession>::MakeSptr(mainScreenId, ScreenProperty(), 0);
    DMRect staticScreenBounds = {{0, 0, 1920, 1080}, 0.0f, 0.0f};
    ssm_->screenSessionMap_.insert(std::make_pair(mainScreenId, staticSession));
    staticSession->SetBounds(staticScreenBounds);
    staticSession->SetRSScreenId(mainScreenId);
    staticSession->SetStartPosition(0, 0);
    ScreenId dynamicScreenId = 1052;
    sptr<ScreenSession> dynamicSession = sptr<ScreenSession>::MakeSptr(dynamicScreenId, ScreenProperty(), 0);
    DMRect dynamicScreenBounds = {{0, 0, 3296, 2472}, 0.0f, 0.0f};
    ssm_->screenSessionMap_.insert(std::make_pair(dynamicScreenId, dynamicSession));
    dynamicSession->SetBounds(dynamicScreenBounds);
    dynamicSession->SetRSScreenId(dynamicScreenId);
    dynamicSession->SetStartPosition(1920, 1080);
    uint32_t borderingAreaPercent = 50;
    uint32_t exceptStaticStartX = 0;
    uint32_t exceptStaticStartY = 0;
    uint32_t exceptDynamicStartX = 1920;
    uint32_t exceptDynamicStartY = 1080;
    ssm_->CalculateStartWhenTransferState(staticSession, dynamicSession, borderingAreaPercent);
    uint32_t testStaticStartX = staticSession->GetScreenProperty().GetStartX();
    uint32_t testStaticStartY = staticSession->GetScreenProperty().GetStartY();
    uint32_t testDynamicStartX = dynamicSession->GetScreenProperty().GetStartX();
    uint32_t testDynamicStartY = dynamicSession->GetScreenProperty().GetStartY();
    EXPECT_EQ(exceptStaticStartX, testStaticStartX);
    EXPECT_EQ(exceptStaticStartY, testStaticStartY);
    EXPECT_EQ(exceptDynamicStartX, testDynamicStartX);
    EXPECT_EQ(exceptDynamicStartY, testDynamicStartY);
}

/**
 * @tc.name: GetAllScreenIds
 * @tc.desc: GetAllScreenIds screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllScreenIds, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = new ScreenSession();
    ASSERT_NE(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(1, screenSession));
    ssm_->GetAllScreenIds();
}

/**
 * @tc.name: GetAllScreenInfos
 * @tc.desc: GetAllScreenInfos test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllScreenInfos, TestSize.Level1)
{
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    std::vector<sptr<ScreenInfo>> screenInfos;
    EXPECT_EQ(DMError::DM_OK, ssm_->GetAllScreenInfos(screenInfos));
}

/**
 * @tc.name: GetScreenSupportedColorGamuts
 * @tc.desc: GetScreenSupportedColorGamuts test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenSupportedColorGamuts, TestSize.Level1)
{
    std::vector<ScreenColorGamut> colorGamuts;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenSupportedColorGamuts(SCREEN_ID_INVALID, colorGamuts));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetScreenSupportedColorGamuts(id, colorGamuts),
        screenSession->GetScreenSupportedColorGamuts(colorGamuts));
}

/**
 * @tc.name: GetPixelFormat
 * @tc.desc: GetPixelFormat test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPixelFormat, TestSize.Level1)
{
    GraphicPixelFormat pixelFormat;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetPixelFormat(SCREEN_ID_INVALID, pixelFormat));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetPixelFormat(id, pixelFormat), screenSession->GetPixelFormat(pixelFormat));
}

/**
 * @tc.name: SetPixelFormat
 * @tc.desc: SetPixelFormat test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetPixelFormat, TestSize.Level1)
{
    GraphicPixelFormat pixelFormat = GraphicPixelFormat{GRAPHIC_PIXEL_FMT_CLUT8};
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetPixelFormat(SCREEN_ID_INVALID, pixelFormat));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->SetPixelFormat(id, pixelFormat), screenSession->SetPixelFormat(pixelFormat));
}

/**
 * @tc.name: GetRoundedCorner
 * @tc.desc: GetRoundedCorner test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetRoundedCorner, TestSize.Level1)
{
    std::vector<RoundedCorner> roundedCorner;
    int radius = 0;
    EXPECT_EQ(DMError::DM_OK, ssm_->GetRoundedCorner(0, radius));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    auto ret = ssm_->GetRoundedCorner(0, radius);
    EXPECT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: GetSupportedHDRFormats test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSupportedHDRFormats, TestSize.Level1)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    std::vector<ScreenHDRFormat> hdrFormats;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetSupportedHDRFormats(SCREEN_ID_INVALID, hdrFormats));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetSupportedHDRFormats(id, hdrFormats), screenSession->GetSupportedHDRFormats(hdrFormats));
#endif
}

/**
 * @tc.name: TestNotifyRunSensorFoldStateManager
 * @tc.desc: TestNotifyRunSensorFoldStateManager
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TestNotifyRunSensorFoldStateManager, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->foldScreenController_->NotifyRunSensorFoldStateManager();
    EXPECT_TRUE(g_logMsg.find("NotifyRunSensorFoldStateManager") != std::string::npos);
    LOG_SetCallback(nullptr);
    g_logMsg.clear();
}

/*
 * @tc.name: NotifyRSCoordination
 * @tc.desc: NotifyRSCoordination
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyRSCoordination, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr) << "ScreenSessionManager instance is null";
    ssm_->NotifyRSCoordination(true);
    EXPECT_TRUE(g_logMsg.find("isEnterCoordination:1") != std::string::npos);
    g_logMsg.clear();
    ssm_->NotifyRSCoordination(false);
    EXPECT_TRUE(g_logMsg.find("isEnterCoordination:0") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}
}
} // namespace Rosen
} // namespace OHOS