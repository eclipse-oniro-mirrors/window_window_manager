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

#include "common_test_utils.h"
#include "display_manager_adapter.h"
#include "display_manager_config.h"
#include "display_manager_ipc_service.h"
#include "mock_rs_display_node.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {

class DisplayManagerIpcServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    static std::unique_ptr<DisplayManagerIpcService> dms_;
};

std::unique_ptr<DisplayManagerIpcService> DisplayManagerIpcServiceTest::dms_ = nullptr;

void DisplayManagerIpcServiceTest::SetUpTestCase()
{
    dms_ = std::make_unique<DisplayManagerIpcService>();
    dms_->displayManagerService_.abstractScreenController_->defaultRsScreenId_ = 0;
    dms_->displayManagerService_.abstractScreenController_->screenIdManager_.rs2DmsScreenIdMap_.clear();
    dms_->displayManagerService_.abstractScreenController_->screenIdManager_.rs2DmsScreenIdMap_ = {
        { 0, 0 }
    };
    dms_->displayManagerService_.abstractScreenController_->screenIdManager_.dms2RsScreenIdMap_.clear();
    dms_->displayManagerService_.abstractScreenController_->screenIdManager_.dms2RsScreenIdMap_ = {
        { 0, 0 }
    };

    const char* perms[] = {
        "ohos.permission.CAPTURE_SCREEN"
    };
    CommonTestUtils::SetAceessTokenPermission("DisplayManagerIpcServiceTest", perms, 1);
}

void DisplayManagerIpcServiceTest::TearDownTestCase()
{
    dms_ = nullptr;
}

namespace {

/**
 * @tc.name: Dump
 * @tc.desc: DMS dump
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, Dump, TestSize.Level1)
{
    std::vector<std::u16string> args;
    EXPECT_EQ(static_cast<int>(DMError::DM_ERROR_INVALID_PARAM), dms_->Dump(-1, args));
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: DMS has private window
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, HasPrivateWindow, TestSize.Level1)
{
    bool hasPrivateWindow = false;
    dms_->displayManagerService_.abstractDisplayController_->abstractDisplayMap_.clear();
    dms_->displayManagerService_.abstractDisplayController_->abstractDisplayMap_ = {
        { 1, nullptr }
    };
    int32_t dmError;
    ASSERT_EQ(ERR_OK, dms_->HasPrivateWindow(0, hasPrivateWindow, dmError));
    EXPECT_FALSE(hasPrivateWindow);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->displayManagerService_.RegisterWindowInfoQueriedListener(nullptr);
    ASSERT_EQ(ERR_OK, dms_->HasPrivateWindow(1, hasPrivateWindow, dmError));
    EXPECT_FALSE(hasPrivateWindow);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: GetScreenInfoById01
 * @tc.desc: DMS get screen info by id
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetScreenInfoById01, TestSize.Level1)
{
    sptr<ScreenInfo> screenInfo;
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetScreenInfoById(SCREEN_ID_INVALID, screenInfo));
    EXPECT_EQ(screenInfo, nullptr);
}

/**
 * @tc.name: GetScreenBrightness
 * @tc.desc: DMS get screen brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, SetScreenBrightness, TestSize.Level1)
{
    bool isSucc = true;
    ASSERT_EQ(ERR_OK, dms_->SetScreenBrightness(0, 100, isSucc));
    EXPECT_TRUE(isSucc);
}

/**
 * @tc.name: GetScreenBrightness
 * @tc.desc: DMS get screen brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetScreenBrightness, TestSize.Level1)
{
    uint32_t level = 0;
    ASSERT_EQ(ERR_OK, dms_->GetScreenBrightness(0, level));
    EXPECT_GT(level, 0);
}

/**
 * @tc.name: GetDisplayInfo
 * @tc.desc: DMS get display info
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetDisplayInfo, TestSize.Level1)
{
    // build abstractDisplayController_ env
    std::string name = "testDisplay";
    sptr<SupportedScreenModes> info = new SupportedScreenModes();
    sptr<AbstractScreen> absScreen =
        new AbstractScreen(dms_->displayManagerService_.abstractScreenController_, name, 0, 0);
    sptr<AbstractDisplay> absDisplay = new AbstractDisplay(0, info, absScreen);

    dms_->displayManagerService_.abstractDisplayController_->abstractDisplayMap_.clear();
    sptr<DisplayInfo> displayInfo;
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetDefaultDisplayInfo(displayInfo));
    ASSERT_EQ(nullptr, displayInfo);

    dms_->displayManagerService_.abstractDisplayController_->abstractDisplayMap_ = {
        { 0, absDisplay }
    };
    ASSERT_EQ(ERR_OK, dms_->GetDefaultDisplayInfo(displayInfo));
    ASSERT_NE(nullptr, displayInfo);
    EXPECT_EQ(absDisplay->name_, displayInfo->GetName());

    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetDisplayInfoById(1, displayInfo));
    ASSERT_EQ(nullptr, displayInfo);

    ASSERT_EQ(ERR_OK, dms_->GetDisplayInfoById(0, displayInfo));
    ASSERT_NE(nullptr, displayInfo);
    EXPECT_EQ(0, displayInfo->GetDisplayId());
    EXPECT_EQ(absDisplay->name_, displayInfo->GetName());

    absDisplay->screenId_ = 0;

    sptr<ScreenInfo> screenInfo;
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetScreenInfoById(1, screenInfo));
    ASSERT_EQ(nullptr, screenInfo);
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetScreenInfoById(0, screenInfo));
    ASSERT_EQ(nullptr, screenInfo);

    sptr<ScreenGroupInfo> screenGroupInfo;
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetScreenGroupInfoById(1, screenGroupInfo));
    ASSERT_EQ(nullptr, screenGroupInfo);
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetScreenGroupInfoById(0, screenGroupInfo));
    ASSERT_EQ(nullptr, screenGroupInfo);

    std::vector<uint64_t> displayIds;
    ASSERT_EQ(ERR_OK, dms_->GetAllDisplayIds(displayIds));
    EXPECT_NE(0, displayIds.size());

    std::vector<sptr<ScreenInfo>> screenInfos;
    int32_t dmError;
    ASSERT_EQ(ERR_OK, dms_->GetAllScreenInfos(screenInfos, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
    EXPECT_EQ(0, screenInfos.size());

    dms_->displayManagerService_.abstractDisplayController_->abstractDisplayMap_.clear();
}

/**
 * @tc.name: VirtualScreen
 * @tc.desc: DMS virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, VirtualScreen, TestSize.Level1)
{
    DmVirtualScreenOption option;
    uint64_t screenId = 0;
    ASSERT_EQ(ERR_OK, dms_->CreateVirtualScreen(option, nullptr, screenId));
    EXPECT_EQ(SCREEN_ID_INVALID, screenId);

    screenId = 0;
    sptr<IBufferProducer> surface;
    ASSERT_EQ(ERR_OK, dms_->CreateVirtualScreen(option, nullptr, screenId, surface));
    EXPECT_EQ(SCREEN_ID_INVALID, screenId);

    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->SetVirtualScreenSurface(-1, nullptr, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    ASSERT_EQ(ERR_OK, dms_->SetVirtualScreenSurface(0, nullptr, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_RENDER_SERVICE_FAILED), dmError);

    std::vector<ScreenId> screens;
    ASSERT_EQ(ERR_OK, dms_->RemoveVirtualScreenFromGroup(screens));

    ASSERT_EQ(ERR_OK, dms_->DestroyVirtualScreen(10086, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_CALLING), dmError);
}

/**
 * @tc.name: OrientationAndRotation
 * @tc.desc: DMS set orientation and rotation
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, OrientationAndRotation, TestSize.Level1)
{
    int32_t dmError = 0;
    auto orientation = static_cast<uint32_t>(Orientation::VERTICAL);
    ASSERT_EQ(ERR_OK, dms_->SetOrientation(0, orientation, dmError));
    EXPECT_NE(static_cast<int32_t>(DMError::DM_OK), dmError);
    orientation = static_cast<uint32_t>(Orientation::SENSOR_VERTICAL);
    ASSERT_EQ(ERR_OK, dms_->SetOrientation(0, orientation, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    orientation = static_cast<uint32_t>(Orientation::UNSPECIFIED);
    ASSERT_EQ(ERR_OK, dms_->SetOrientation(0, orientation, dmError));
    EXPECT_NE(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: ScreenColor
 * @tc.desc: DMS screen color
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, ScreenColor, TestSize.Level1)
{
    int32_t dmError = 0;
    std::vector<uint32_t> colorGamuts;

    dms_->GetScreenSupportedColorGamuts(SCREEN_ID_INVALID, colorGamuts, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    EXPECT_EQ(0, colorGamuts.size());

    dms_->GetScreenSupportedColorGamuts(0, colorGamuts, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    EXPECT_EQ(0, colorGamuts.size());

    uint32_t colorGamut = ScreenColorGamut::COLOR_GAMUT_SRGB;
    dms_->GetScreenColorGamut(SCREEN_ID_INVALID, colorGamut, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->GetScreenColorGamut(0, colorGamut, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);


    colorGamut = ScreenColorGamut::COLOR_GAMUT_SRGB;
    dms_->SetScreenColorGamut(SCREEN_ID_INVALID, colorGamut, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->SetScreenColorGamut(0, colorGamut, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->SetScreenColorGamut(SCREEN_ID_INVALID, 0, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->SetScreenColorGamut(0, 0, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    uint32_t gamutMap = ScreenGamutMap::GAMUT_MAP_CONSTANT;
    dms_->GetScreenGamutMap(SCREEN_ID_INVALID, gamutMap, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->GetScreenGamutMap(0, gamutMap, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->SetScreenGamutMap(SCREEN_ID_INVALID, gamutMap, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->SetScreenGamutMap(0, gamutMap, dmError);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);

    dms_->SetScreenColorTransform(SCREEN_ID_INVALID);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    dms_->SetScreenColorTransform(0);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
}

/**
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: DMS register display manager agent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, RegisterDisplayManagerAgent, TestSize.Level1)
{
    auto type = static_cast<uint32_t>(DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->RegisterDisplayManagerAgent(nullptr, type, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
    ASSERT_EQ(ERR_OK, dms_->UnregisterDisplayManagerAgent(nullptr, type, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: ScreenPower
 * @tc.desc: DMS screen power
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, ScreenPower, TestSize.Level1)
{
    auto reason = static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON);
    auto screenPowerState = static_cast<uint32_t>(ScreenPowerState::POWER_ON);
    auto displayState = static_cast<uint32_t>(DisplayState::ON);
    bool isSucc = true;

    ASSERT_EQ(ERR_OK, dms_->WakeUpBegin(reason, isSucc));
    EXPECT_FALSE(isSucc);
    ASSERT_EQ(ERR_OK, dms_->WakeUpEnd(isSucc));
    EXPECT_FALSE(isSucc);

    ASSERT_EQ(ERR_OK, dms_->SuspendBegin(reason, isSucc));
    EXPECT_FALSE(isSucc);
    ASSERT_EQ(ERR_OK, dms_->SuspendEnd(isSucc));
    EXPECT_FALSE(isSucc);

    ASSERT_EQ(ERR_OK, dms_->SetScreenPowerForAll(screenPowerState, reason, isSucc));
    EXPECT_FALSE(isSucc);

    ScreenId dmsScreenId = 2;
    ASSERT_EQ(ERR_OK, dms_->GetScreenPower(dmsScreenId, screenPowerState));
    EXPECT_EQ(static_cast<uint32_t>(ScreenPowerState::INVALID_STATE), screenPowerState);

    ASSERT_EQ(ERR_OK, dms_->SetDisplayState(displayState, isSucc));
    EXPECT_TRUE(isSucc);
    uint32_t displayStateOut = 0;
    ASSERT_EQ(ERR_OK, dms_->GetDisplayState(0, displayStateOut));
    EXPECT_EQ(static_cast<uint32_t>(DisplayState::ON), displayStateOut);
}

/**
 * @tc.name: MirrorAndExpand
 * @tc.desc: DMS mirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, MirrorAndExpand, TestSize.Level1)
{
    int32_t dmError = 0;
    std::vector<uint64_t> mirrorScreenIds;
    uint64_t screenGroupId1 = DISPLAY_ID_INVALID;
    ASSERT_EQ(ERR_OK, dms_->MakeMirror(DISPLAY_ID_INVALID, mirrorScreenIds, screenGroupId1, dmError));
    EXPECT_EQ(SCREEN_ID_INVALID, screenGroupId1);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    ASSERT_EQ(ERR_OK, dms_->StopMirror(mirrorScreenIds, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);

    std::vector<uint64_t> expandScreenIds;
    std::vector<Point> startPoints;
    uint64_t screenGroupId2 = DISPLAY_ID_INVALID;
    ASSERT_EQ(ERR_OK, dms_->MakeExpand(expandScreenIds, startPoints, screenGroupId2, dmError));
    EXPECT_EQ(SCREEN_ID_INVALID, screenGroupId2);
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    ASSERT_EQ(ERR_OK, dms_->StopExpand(expandScreenIds, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: ScreenActiveMode
 * @tc.desc: DMS mirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, ScreenActiveMode, TestSize.Level1)
{
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->SetScreenActiveMode(SCREEN_ID_INVALID, 0, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: VirtualPixelRatio
 * @tc.desc: DMS mirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, VirtualPixelRatio, TestSize.Level1)
{
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->SetVirtualPixelRatio(SCREEN_ID_INVALID, 0.f, dmError));
    EXPECT_NE(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: RemoveSurfaceNodeFromDisplay01
 * @tc.desc: RemoveSurfaceNodeFromDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, RemoveSurfaceNodeFromDisplay01, TestSize.Level1)
{
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(nullptr);
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->RemoveSurfaceNodeFromDisplay(0, dmRsSurfaceNode, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: NotifyDisplayEvent
 * @tc.desc: NotifyDisplayEvent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, NotifyDisplayEvent, TestSize.Level1)
{
    auto event = static_cast<uint32_t>(DisplayEvent::KEYGUARD_DRAWN);
    ASSERT_EQ(ERR_OK, dms_->NotifyDisplayEvent(event));
    EXPECT_NE(dms_->displayManagerService_.displayPowerController_, nullptr);
}

/**
 * @tc.name: SetFreeze
 * @tc.desc: SetFreeze
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, SetFreeze, TestSize.Level1)
{
    std::vector<DisplayId> displayIds = { 0 };
    bool isFreeze = false;
    bool isSucc = true;
    ASSERT_EQ(ERR_OK, dms_->SetFreeze(displayIds, isFreeze, isSucc));
    EXPECT_TRUE(isSucc);
    EXPECT_NE(dms_->displayManagerService_.abstractDisplayController_, nullptr);
}

/**
 * @tc.name: AddSurfaceNodeToDisplay
 * @tc.desc: AddSurfaceNodeToDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, AddSurfaceNodeToDisplay, TestSize.Level1)
{
    DisplayId displayId = 1;
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->AddSurfaceNodeToDisplay(displayId, dmRsSurfaceNode, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
    EXPECT_NE(dms_->displayManagerService_.abstractScreenController_, nullptr);
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: IsScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, IsScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = true;
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->IsScreenRotationLocked(isLocked, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: SetScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, SetScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = true;
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->SetScreenRotationLocked(isLocked, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: SetScreenRotationLockedFromJs
 * @tc.desc: SetScreenRotationLockedFromJs
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, SetScreenRotationLockedFromJs, TestSize.Level1)
{
    bool isLocked = true;
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->SetScreenRotationLockedFromJs(isLocked, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, MakeMirror, TestSize.Level1)
{
    ScreenId mainScreenId = 1;
    std::vector<ScreenId> mirrorScreenIds = { 2 };
    ScreenId screenGroupId = 3;
    sptr<AbstractScreen> absScreen =
        new AbstractScreen(nullptr, "", INVALID_SCREEN_ID, INVALID_SCREEN_ID);
    dms_->displayManagerService_.abstractScreenController_->dmsScreenMap_.insert(
        std::make_pair(mainScreenId, absScreen));
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    dms_->displayManagerService_.abstractScreenController_->dmsScreenMap_.clear();
}

/**
 * @tc.name: StopMirror
 * @tc.desc: StopMirror
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, StopMirror, TestSize.Level1)
{
    std::vector<ScreenId> mirrorScreenIds = { 2 };
    sptr<AbstractScreen> absScreen = new AbstractScreen(nullptr, "", INVALID_SCREEN_ID, INVALID_SCREEN_ID);
    dms_->displayManagerService_.abstractScreenController_->dmsScreenMap_.insert(std::make_pair(2, absScreen));
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->StopMirror(mirrorScreenIds, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: RemoveSurfaceNodeFromDisplay02
 * @tc.desc: RemoveSurfaceNodeFromDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, RemoveSurfaceNodeFromDisplay02, TestSize.Level1)
{
    DisplayId displayId = 1;
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->RemoveSurfaceNodeFromDisplay(displayId, dmRsSurfaceNode, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: DMS SetOrientation
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, SetOrientation, TestSize.Level1)
{
    ScreenId screenId = 0;
    auto orientation = static_cast<uint32_t>(Orientation::VERTICAL);
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->SetOrientation(screenId, orientation, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: GetDisplaySnapshot
 * @tc.desc: DMS GetDisplaySnapshot
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetDisplaySnapshot, TestSize.Level1)
{
    DisplayId displayId = -1;
    int32_t errorCode = 0;
    std::shared_ptr<PixelMap> pixelMap;
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetDisplaySnapshot(displayId, errorCode, false, false, pixelMap));
    ASSERT_EQ(static_cast<int32_t>(DmErrorCode::DM_OK), errorCode);
    ASSERT_EQ(nullptr, pixelMap);
}

/**
 * @tc.name: AddSurfaceNodeToDisplay
 * @tc.desc: DMS AddSurfaceNodeToDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, AddSurfaceNodeToDisplay02, TestSize.Level1)
{
    DisplayId displayId = 1;
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    std::shared_ptr<DmRsSurfaceNode> dmRsSurfaceNode = std::make_shared<DmRsSurfaceNode>(surfaceNode);
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->AddSurfaceNodeToDisplay(displayId, dmRsSurfaceNode, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
}

/**
 * @tc.name: GetAllScreenInfos
 * @tc.desc: DMS GetAllScreenInfos
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetAllScreenInfos, TestSize.Level1)
{
    std::vector<sptr<ScreenInfo>> screenInfos;
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->GetAllScreenInfos(screenInfos, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: DMS MakeExpand
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, MakeExpand01, TestSize.Level1)
{
    std::vector<ScreenId> expandScreenIds{ 1 };
    std::vector<Point> startPoints(1);
    ScreenId screenGroupId = 3;
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->MakeExpand(expandScreenIds, startPoints, screenGroupId, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), dmError);
    EXPECT_EQ(3u, screenGroupId);
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: DMS MakeExpand
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, MakeExpand02, TestSize.Level1)
{
    std::vector<ScreenId> expandScreenIds{ 1, 2, 3, 4, 5 };
    std::vector<Point> startPoints(1);
    ScreenId screenGroupId = 3;
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->MakeExpand(expandScreenIds, startPoints, screenGroupId, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM), dmError);
    EXPECT_EQ(3u, screenGroupId);
}

/**
 * @tc.name: StopExpand
 * @tc.desc: DMS StopExpand
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, StopExpand, TestSize.Level1)
{
    std::vector<ScreenId> expandScreenIds{ 0, 1, 2, 3, 4, 5 };
    int32_t dmError = 0;
    ASSERT_EQ(ERR_OK, dms_->StopExpand(expandScreenIds, dmError));
    EXPECT_EQ(static_cast<int32_t>(DMError::DM_OK), dmError);
}

/**
 * @tc.name: GetVisibleAreaDisplayInfoById01
 * @tc.desc: GetVisibleAreaDisplayInfoById
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetVisibleAreaDisplayInfoById01, TestSize.Level1)
{
    DisplayId displayId = DISPLAY_ID_INVALID;
    sptr<DisplayInfo> displayInfo;
    ASSERT_EQ(ERR_INVALID_DATA, dms_->GetVisibleAreaDisplayInfoById(displayId, displayInfo));
    EXPECT_EQ(displayInfo, nullptr);
}

/**
 * @tc.name: GetVisibleAreaDisplayInfoById02
 * @tc.desc: GetVisibleAreaDisplayInfoById
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetVisibleAreaDisplayInfoById02, TestSize.Level1)
{
    DisplayId displayId = 2;
    std::string name = "testDisplay";
    sptr<SupportedScreenModes> info = new SupportedScreenModes();
    sptr<AbstractScreen> absScreen =
        new AbstractScreen(dms_->displayManagerService_.abstractScreenController_, name, 0, 0);
    sptr<AbstractDisplay> absDisplay = new AbstractDisplay(0, info, absScreen);
    dms_->displayManagerService_.abstractDisplayController_->abstractDisplayMap_.insert({ displayId, absDisplay });
    sptr<DisplayInfo> displayInfo;
    ASSERT_EQ(ERR_OK, dms_->GetVisibleAreaDisplayInfoById(displayId, displayInfo));
    EXPECT_NE(displayInfo, nullptr);
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution01
 * @tc.desc: Test GetAllDisplayPhysicalResolution function when allDisplayPhysicalResolution_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetAllDisplayPhysicalResolution01, TestSize.Level1)
{
    dms_->displayManagerService_.allDisplayPhysicalResolution_.clear();
    std::vector<DisplayPhysicalResolution> displayPhysicalResolutions;
    ASSERT_EQ(ERR_OK, dms_->GetAllDisplayPhysicalResolution(displayPhysicalResolutions));
    EXPECT_FALSE(displayPhysicalResolutions.empty());
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution02
 * @tc.desc: Test GetAllDisplayPhysicalResolution function when allDisplayPhysicalResolution_ is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, GetAllDisplayPhysicalResolution02, TestSize.Level1)
{
    dms_->displayManagerService_.allDisplayPhysicalResolution_.emplace_back();
    std::vector<DisplayPhysicalResolution> displayPhysicalResolutions;
    ASSERT_EQ(ERR_OK, dms_->GetAllDisplayPhysicalResolution(displayPhysicalResolutions));
    EXPECT_FALSE(displayPhysicalResolutions.empty());
}

/**
 * @tc.name  : SetVirtualScreenAsDefault
 * @tc.number: SetVirtualScreenAsDefault test
 * @tc.desc  : FUNC
 */
HWTEST_F(DisplayManagerIpcServiceTest, SetVirtualScreenAsDefault, TestSize.Level1) {
    bool isSucc = false;
    uint64_t screenId = SCREEN_ID_INVALID;
 
    dms_->SetVirtualScreenAsDefault(screenId, isSucc);
 
    EXPECT_FALSE(isSucc);
}
} // namespace
} // namespace OHOS::Rosen
