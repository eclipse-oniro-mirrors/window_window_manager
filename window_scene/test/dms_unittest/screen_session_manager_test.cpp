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

#include "session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "iconsumer_surface.h"
#include "connection/screen_cast_connection.h"
#include "screen_scene_config.h"
#include <surface.h>
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const int32_t CV_WAIT_SCREENOFF_MS = 1500;
const int32_t CV_WAIT_SCREENOFF_MS_MAX = 3000;
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
constexpr int32_t CAST_WIRED_PROJECTION_START = 1005;
constexpr int32_t CAST_WIRED_PROJECTION_STOP = 1007;
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
};

sptr<ScreenSessionManager> ScreenSessionManagerTest::ssm_ = nullptr;

void ScreenSessionManagerTest::SetUpTestCase()
{
    ssm_ = new ScreenSessionManager();
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
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: RegisterDisplayManagerAgent test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RegisterDisplayManagerAgent, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    EXPECT_NE(DMError::DM_ERROR_NOT_SYSTEM_APP, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    EXPECT_NE(DMError::DM_ERROR_NOT_SYSTEM_APP, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));

    type = DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->RegisterDisplayManagerAgent(nullptr, type));
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->UnregisterDisplayManagerAgent(nullptr, type));

    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: WakeupBegin
 * @tc.desc: WakeupBegin test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WakeUpBegin, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SuspendBegin
 * @tc.desc: SuspendBegin test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SuspendBegin, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
    ASSERT_EQ(true, ssm_->SuspendBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    ASSERT_EQ(true, ssm_->SuspendBegin(reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll
 * @tc.desc: SetScreenPowerForAll test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
    state = ScreenPowerState::POWER_OFF;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: ScreenChange
 * @tc.desc: ScreenChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScreenChange, Function | SmallTest | Level3)
{
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ssm_->OnVirtualScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnVirtualScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    EXPECT_TRUE(1);

    screenEvent = ScreenEvent::DISCONNECTED;
    ssm_->OnVirtualScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnVirtualScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: ScreenPower
 * @tc.desc: ScreenSesionManager screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScreenPower, Function | SmallTest | Level3)
{
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    DisplayState displayState = DisplayState::ON;

    ASSERT_EQ(false, ssm_->WakeUpBegin(reason));
    ASSERT_EQ(false, ssm_->WakeUpEnd());

    ASSERT_EQ(false, ssm_->SuspendBegin(reason));
    ASSERT_EQ(false, ssm_->SuspendEnd());

    ASSERT_EQ(false, ssm_->SetScreenPowerForAll(state, reason));

    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(true, ssm_->SetDisplayState(displayState));
    ASSERT_EQ(DisplayState::ON, ssm_->GetDisplayState(0));
}

/**
 * @tc.name: GetScreenPower
 * @tc.desc: GetScreenPower screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenPower, Function | SmallTest | Level3)
{
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ssm_->GetScreenPower(0);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: IsScreenRotationLocked test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsScreenRotationLocked, Function | SmallTest | Level3)
{
    bool isLocked = false;
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(DMError::DM_OK, ssm_->IsScreenRotationLocked(isLocked));
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetOrientation, Function | SmallTest | Level3)
{
    Orientation orientation = Orientation::HORIZONTAL;
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(DMError::DM_OK, ssm_->SetOrientation(id, orientation));
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->SetOrientation(SCREEN_ID_INVALID, orientation));
    Orientation invalidOrientation = Orientation{20};
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetOrientation(id, invalidOrientation));
}

/**
 * @tc.name: SetRotationFromWindow
 * @tc.desc: SetRotationFromWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRotationFromWindow, Function | SmallTest | Level3)
{
    Rotation targetRotation = Rotation::ROTATION_0;
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(ssm_->SetRotationFromWindow(targetRotation), ssm_->SetRotation(id, targetRotation, true));
}

/**
 * @tc.name: GetDisplaySnapshot
 * @tc.desc: ScreenSesionManager screen shot
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplaySnapshot, Function | SmallTest | Level3)
{
    DisplayId displayId(0);
    DmErrorCode* errorCode = nullptr;
    ssm_->GetDisplaySnapshot(displayId, errorCode);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: VirtualScreen
 * @tc.desc: ScreenSesionManager virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, VirtualScreen, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    std::vector<ScreenId> mirrorScreenIds;
    ScreenId mainScreenId(DEFAULT_SCREEN_ID);
    ScreenId screenGroupId{1};
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId));
    mirrorScreenIds.push_back(VIRTUAL_SCREEN_ID);
    ASSERT_NE(DMError::DM_OK, ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId));

    auto result1 = ssm_->SetVirtualScreenSurface(VIRTUAL_SCREEN_ID, nullptr);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, result1);
    sptr<IConsumerSurface> surface = OHOS::IConsumerSurface::Create();
    auto result2 = ssm_->SetVirtualScreenSurface(VIRTUAL_SCREEN_ID, surface->GetProducer());
    if (DMError::DM_ERROR_RENDER_SERVICE_FAILED == result2) {
        ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, result2);
    }
    if (DMError::DM_OK != result2) {
        ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->DestroyVirtualScreen(VIRTUAL_SCREEN_ID));
    }
}

/**
 * @tc.name: AutoRotate
 * @tc.desc: ScreenSesionManager SetVirtualMirrorScreenCanvasRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AutoRotate, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testAutoRotate";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    auto result1 = ssm_->SetVirtualMirrorScreenCanvasRotation(screenId, true);
    ASSERT_EQ(DMError::DM_OK, result1);
    auto result2 = ssm_->SetVirtualMirrorScreenCanvasRotation(screenId, false);
    ASSERT_EQ(DMError::DM_OK, result2);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetScreenSession
 * @tc.desc: GetScreenSession virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenSession, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetScreenSession", screenId, rsid, 0);
    ASSERT_NE(ssm_->GetScreenSession(screenId), screenSession);
    ssm_->DestroyVirtualScreen(screenId);
}


/**
 * @tc.name: GetDefaultScreenSession
 * @tc.desc: GetDefaultScreenSession virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDefaultScreenSession, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetDefaultScreenSession", screenId, rsid, 0);
    ASSERT_NE(ssm_->GetDefaultScreenSession(), screenSession);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetDefaultDisplayInfo
 * @tc.desc: GetDefaultDisplayInfo virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDefaultDisplayInfo, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetDefaultDisplayInfo", screenId, rsid, 0);
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    if (ssm_->GetScreenSession(screenId) == nullptr) {
        ASSERT_EQ(ssm_->GetDefaultDisplayInfo(), nullptr);
    }
    ASSERT_NE(ssm_->GetScreenSession(screenId), nullptr);
    ASSERT_NE(ssm_->GetDefaultDisplayInfo(), displayInfo);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: HookDisplayInfoByUid
 * @tc.desc: HookDisplayInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HookDisplayInfoByUid, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession("GetDefaultDisplayInfo", screenId, rsid, 0);
    sptr<DisplayInfo> displayInfo = ssm_->GetDefaultDisplayInfo();
    ASSERT_NE(ssm_->GetScreenSession(screenId), nullptr);
    ASSERT_NE(displayInfo, nullptr);
    uint32_t uid = 20020001;
    DMHookInfo dmHookInfo;
    std::map<uint32_t, DMHookInfo> displayHookMap = {};
    displayHookMap[uid] = dmHookInfo;
    ASSERT_EQ(displayHookMap.find(uid) != displayHookMap.end(), true);
    displayInfo->SetWidth(100);
    ASSERT_EQ(displayInfo->GetWidth(), 100);
    displayInfo->SetHeight(100);
    ASSERT_EQ(displayInfo->GetHeight(), 100);
    displayInfo->SetVirtualPixelRatio(1.0);
    ASSERT_EQ(displayInfo->GetVirtualPixelRatio(), 1.0);
    displayInfo->SetRotation(Rotation::ROTATION_0);
    ASSERT_EQ(displayInfo->GetRotation(), Rotation::ROTATION_0);
    displayInfo->SetDisplayOrientation(DisplayOrientation::PORTRAIT);
    ASSERT_EQ(displayInfo->GetDisplayOrientation(), DisplayOrientation::PORTRAIT);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetDisplayInfoById
 * @tc.desc: GetDisplayInfoById virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoById, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    ASSERT_EQ(ssm_->GetDisplayInfoById(1), nullptr);
}

/**
 * @tc.name: GetDisplayInfoByScreen
 * @tc.desc: GetDisplayInfoByScreen virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoByScreen, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    ASSERT_EQ(ssm_->GetDisplayInfoByScreen(1), nullptr);
}

/**
 * @tc.name: GetScreenInfoById
 * @tc.desc: GetScreenInfoById virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenInfoById, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    ASSERT_EQ(ssm_->GetScreenInfoById(1), nullptr);
}

/**
 * @tc.name: SetScreenActiveMode
 * @tc.desc: SetScreenActiveMode virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenActiveMode, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetScreenActiveMode";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_EQ(ssm_->SetScreenActiveMode(screenId, 0), DMError::DM_OK);
    ssm_->DestroyVirtualScreen(screenId);
}


/**
 * @tc.name: NotifyScreenChanged
 * @tc.desc: NotifyScreenChanged virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyScreenChanged, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "NotifyScreenChanged";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenInfo> screenInfo;
    ssm_->NotifyScreenChanged(screenInfo, ScreenChangeEvent::UPDATE_ORIENTATION);
    screenInfo = new ScreenInfo();
    ssm_->NotifyScreenChanged(screenInfo, ScreenChangeEvent::UPDATE_ORIENTATION);
    ASSERT_EQ(ssm_->SetScreenActiveMode(screenId, 0), DMError::DM_OK);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: NotifyDisplayEvent
 * @tc.desc: NotifyDisplayEvent  virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayEvent, Function | SmallTest | Level3)
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
}


/**
 * @tc.name: GetScreenInfoByDisplayId
 * @tc.desc: GetScreenInfoByDisplayId  virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenInfoByDisplayId, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, GetScreenModesByDisplayId, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, UpdateDisplayHookInfo001, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, UpdateDisplayHookInfo002, Function | SmallTest | Level3)
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
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: SetVirtualPixelRatio virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualPixelRatio, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, SetVirtualPixelRatioSystem, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetVirtualPixelRatioSystem";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_EQ(DMError::DM_OK, ssm_->SetVirtualPixelRatioSystem(screenId, 0.1));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetResolution, Function | SmallTest | Level3)
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
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetScreenColorGamut
 * @tc.desc: GetScreenColorGamut virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenColorGamut, Function | SmallTest | Level3)
{
    ScreenColorGamut colorGamut = ScreenColorGamut::COLOR_GAMUT_SRGB;
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenColorGamut(1, colorGamut));
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(DMError::DM_OK, ssm_->GetScreenColorGamut(id, colorGamut));
    } else {
        ASSERT_NE(DMError::DM_OK, ssm_->GetScreenColorGamut(id, colorGamut));
    }
}

/**
 * @tc.name: LoadScreenSceneXml
 * @tc.desc: LoadScreenSceneXml virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, LoadScreenSceneXml, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "LoadScreenSceneXml";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ssm_->LoadScreenSceneXml();
    ASSERT_EQ(ssm_->SetScreenActiveMode(screenId, 0), DMError::DM_OK);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetScreenGamutMap
 * @tc.desc: GetScreenGamutMap virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenGamutMap, Function | SmallTest | Level3)
{
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
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: MakeExpand virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, MakeExpand, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, DeleteScreenId, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, HasRsScreenId, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

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
HWTEST_F(ScreenSessionManagerTest, AddAsFirstScreenLocked, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

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
HWTEST_F(ScreenSessionManagerTest, AddAsSuccedentScreenLocked, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

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
HWTEST_F(ScreenSessionManagerTest, SetMirror, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "SetMirror";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    std::vector<ScreenId> screens{0, 1, 2, 3, 4, 5, 6, 7};
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new (std::nothrow) ScreenSession("GetScreenGamutMap", 2, 2, 3);
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap_{
        {2, screenSession},
    };
    ssm_->screenSessionMap_ = screenSessionMap_;
    auto screen = ssm_->GetScreenSession(2);
    screen->GetScreenProperty().SetScreenType(ScreenType::REAL);
    ASSERT_EQ(DMError::DM_OK, ssm_->SetMirror(2, screens));
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->SetMirror(9, screens));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetAbstractScreenGroup
 * @tc.desc: GetAbstractScreenGroup virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAbstractScreenGroup, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

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
HWTEST_F(ScreenSessionManagerTest, InitAbstractScreenModesInfo, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_EQ(true, ssm_->InitAbstractScreenModesInfo(screenSession));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: AddToGroupLocked
 * @tc.desc: AddToGroupLocked virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddToGroupLocked, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

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
HWTEST_F(ScreenSessionManagerTest, InitVirtualScreen, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
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
HWTEST_F(ScreenSessionManagerTest, InitAndGetScreen, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

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
HWTEST_F(ScreenSessionManagerTest, RemoveFromGroupLocked, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";

    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    sptr<ScreenSession> newScreen = new (std::nothrow) ScreenSession();
    sptr<ScreenSessionGroup> sessiongroup;
    // sptr<ScreenSessionGroup>
    ASSERT_EQ(sessiongroup, ssm_->RemoveFromGroupLocked(newScreen));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: CreateAndGetNewScreenId
 * @tc.desc: CreateAndGetNewScreenId virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateAndGetNewScreenId, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, AddScreenToGroup, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    const std::vector<ScreenId> addScreens;
    const std::vector<Point> addChildPos;
    std::map<ScreenId, bool> removeChildResMap;
    sptr<ScreenSessionGroup> group;
    ssm_->AddScreenToGroup(group, addScreens, addChildPos, removeChildResMap);
    sptr<ScreenSession> screenSession =new  (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ssm_->InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetRSDisplayNodeByScreenId
 * @tc.desc: GetRSDisplayNodeByScreenId virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetRSDisplayNodeByScreenId, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
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
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayIds, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ssm_->GetRSDisplayNodeByScreenId(2);
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
HWTEST_F(ScreenSessionManagerTest, SetScreenGamutMap, Function | SmallTest | Level3)
{
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
}

/**
 * @tc.name: StopExpand
 * @tc.desc: StopExpand virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, StopExpand, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
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
HWTEST_F(ScreenSessionManagerTest, OnScreenDisconnect, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ssm_->GetRSDisplayNodeByScreenId(2);
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
HWTEST_F(ScreenSessionManagerTest, SetScreenColorGamut, Function | SmallTest | Level3)
{
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
}

/**
 * @tc.name: SetScreenColorTransform
 * @tc.desc: SetScreenColorTransform virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenColorTransform, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, SetScreenRotationLocked, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, UpdateScreenRotationProperty, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "DeleteScreenId";
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    int rotation = 1;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_BEGIN;
    ScreenSessionManager::GetInstance().UpdateScreenRotationProperty(1, bounds, 1, screenPropertyChangeType);
    auto screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(virtualOption,
        displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ScreenSessionManager::GetInstance().UpdateScreenRotationProperty(1, bounds, rotation,
        screenPropertyChangeType);
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession();
    ASSERT_NE(screenSession, ScreenSessionManager::GetInstance().InitAndGetScreen(2));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: MakeUniqueScreen
 * @tc.desc: Make unique screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, MakeUniqueScreen, Function | SmallTest | Level3)
{
    vector<ScreenId> screenIds;
    screenIds.clear();
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->MakeUniqueScreen(screenIds));
}

/**
 * @tc.name: ConvertScreenIdToRsScreenId
 * @tc.desc: convert screen id to RS screen id
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConvertScreenIdToRsScreenId, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, DisableMirror, Function | SmallTest | Level3)
{
    ASSERT_EQ(DMError::DM_OK, ssm_->DisableMirror(false));
}

/**
 * @tc.name: HasImmersiveWindow
 * @tc.desc: HasImmersiveWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasImmersiveWindow, Function | SmallTest | Level3)
{
    bool immersive = false;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->HasImmersiveWindow(immersive));
    immersive = true;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->HasImmersiveWindow(immersive));
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: ScreenSessionManager screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetSpecifiedScreenPower, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, NotifyFoldStatusChanged, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, NotifyPrivateWindowListChanged, Function | SmallTest | Level3)
{
    DisplayId id = 0;
    std::vector<std::string> privacyWindowList{"win0", "win1"};
    if (ssm_ != nullptr)
    {
        ssm_->NotifyPrivateWindowListChanged(id, privacyWindowList);
        ASSERT_EQ(0, 0);
    } else {
        ASSERT_EQ(1, 0);
    }
}

/**
 * @tc.name: SetPrivacyStateByDisplayId01
 * @tc.desc: SetPrivacyStateByDisplayId true test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetPrivacyStateByDisplayId01, Function | SmallTest | Level3)
{
    DisplayId id = 0;
    bool hasPrivate = true;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    ssm_->SetPrivacyStateByDisplayId(id, hasPrivate);
    bool result = screenSession->HasPrivateSessionForeground();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SetPrivacyStateByDisplayId02
 * @tc.desc: SetPrivacyStateByDisplayId false test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetPrivacyStateByDisplayId02, Function | SmallTest | Level3)
{
    DisplayId id = 0;
    bool hasPrivate = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    ssm_->SetPrivacyStateByDisplayId(id, hasPrivate);
    bool result = screenSession->HasPrivateSessionForeground();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetScreenPrivacyWindowList
 * @tc.desc: SetScreenPrivacyWindowList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPrivacyWindowList, Function | SmallTest | Level3)
{
    DisplayId id = 0;
    std::vector<std::string> privacyWindowList{"win0", "win1"};
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ssm_->SetScreenPrivacyWindowList(id, privacyWindowList);
    ASSERT_EQ(0, 0);
}

/**
 * @tc.name: GetAllScreenIds
 * @tc.desc: GetAllScreenIds screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllScreenIds, Function | SmallTest | Level3)
{
    sptr<ScreenSession> screenSession = new ScreenSession();
    ASSERT_NE(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(1, screenSession));
    ssm_->GetAllScreenIds();
    EXPECT_TRUE(1);
}

/**
 * @tc.name: GetAllScreenInfos
 * @tc.desc: GetAllScreenInfos test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllScreenInfos, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, GetScreenSupportedColorGamuts, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, GetPixelFormat, Function | SmallTest | Level3)
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
HWTEST_F(ScreenSessionManagerTest, SetPixelFormat, Function | SmallTest | Level3)
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
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: GetSupportedHDRFormats test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSupportedHDRFormats, Function | SmallTest | Level3)
{
    std::vector<ScreenHDRFormat> hdrFormats;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetSupportedHDRFormats(SCREEN_ID_INVALID, hdrFormats));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetSupportedHDRFormats(id, hdrFormats), screenSession->GetSupportedHDRFormats(hdrFormats));
}

/**
 * @tc.name: GetScreenHDRFormat
 * @tc.desc: GetScreenHDRFormat test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenHDRFormat, Function | SmallTest | Level3)
{
    ScreenHDRFormat hdrFormat;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenHDRFormat(SCREEN_ID_INVALID, hdrFormat));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetScreenHDRFormat(id, hdrFormat), screenSession->GetScreenHDRFormat(hdrFormat));
}

/**
 * @tc.name: SetScreenHDRFormat
 * @tc.desc: SetScreenHDRFormat test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenHDRFormat, Function | SmallTest | Level3)
{
    int32_t modeIdx {0};
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenHDRFormat(SCREEN_ID_INVALID, modeIdx));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->SetScreenHDRFormat(id, modeIdx), screenSession->SetScreenHDRFormat(modeIdx));
}

/**
 * @tc.name: GetSupportedColorSpaces
 * @tc.desc: GetSupportedColorSpaces test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSupportedColorSpaces, Function | SmallTest | Level3)
{
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetSupportedColorSpaces(SCREEN_ID_INVALID, colorSpaces));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetSupportedColorSpaces(id, colorSpaces), screenSession->GetSupportedColorSpaces(colorSpaces));
}

/**
 * @tc.name: GetScreenColorSpace
 * @tc.desc: GetScreenColorSpace test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenColorSpace, Function | SmallTest | Level3)
{
    GraphicCM_ColorSpaceType colorSpace;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenColorSpace(SCREEN_ID_INVALID, colorSpace));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetScreenColorSpace(id, colorSpace), screenSession->GetScreenColorSpace(colorSpace));
}

/**
 * @tc.name: SetScreenColorSpace
 * @tc.desc: SetScreenColorSpace test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenColorSpace, Function | SmallTest | Level3)
{
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType{GRAPHIC_CM_COLORSPACE_NONE};
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenColorSpace(SCREEN_ID_INVALID, colorSpace));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->SetScreenColorSpace(id, colorSpace), screenSession->SetScreenColorSpace(colorSpace));
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: HasPrivateWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasPrivateWindow, Function | SmallTest | Level3)
{
    bool hasPrivateWindow;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->HasPrivateWindow(SCREEN_ID_INVALID, hasPrivateWindow));
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(DMError::DM_OK, ssm_->HasPrivateWindow(id, hasPrivateWindow));
}

/**
 * @tc.name: GetAvailableArea
 * @tc.desc: GetAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAvailableArea, Function | SmallTest | Level3)
{
    DMRect area;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->GetAvailableArea(SCREEN_ID_INVALID, area));
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(DMError::DM_OK, ssm_->GetAvailableArea(id, area));
}

/**
 * @tc.name: ResetAllFreezeStatus
 * @tc.desc: ResetAllFreezeStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ResetAllFreezeStatus, Function | SmallTest | Level3)
{
    EXPECT_EQ(DMError::DM_OK, ssm_->ResetAllFreezeStatus());
}

/**
 * @tc.name: SetVirtualScreenRefreshRate
 * @tc.desc: SetVirtualScreenRefreshRate test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenRefreshRate, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    uint32_t refreshInterval {2};
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetVirtualScreenRefreshRate(id, refreshInterval));
    EXPECT_EQ(DMError::DM_OK, ssm_->SetVirtualScreenRefreshRate(screenId, refreshInterval));
    uint32_t invalidRefreshInterval {0};
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetVirtualScreenRefreshRate(screenId, invalidRefreshInterval));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetVirtualScreenFlag
 * @tc.desc: SetVirtualScreenFlag test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenFlag, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenFlag screenFlag = VirtualScreenFlag::DEFAULT;
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    EXPECT_EQ(DMError::DM_OK, ssm_->SetVirtualScreenFlag(screenId, screenFlag));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetVirtualScreenFlag
 * @tc.desc: GetVirtualScreenFlag test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetVirtualScreenFlag, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenFlag screenFlag = VirtualScreenFlag::DEFAULT;
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    EXPECT_EQ(DMError::DM_OK, ssm_->SetVirtualScreenFlag(screenId, screenFlag));
    EXPECT_EQ(screenFlag, ssm_->GetVirtualScreenFlag(screenId));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: ResizeVirtualScreen
 * @tc.desc: ResizeVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ResizeVirtualScreen, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);

    uint32_t width {100};
    uint32_t height {100};
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    if (screenSession->GetDisplayNode() != nullptr) {
        EXPECT_EQ(DMError::DM_OK, ssm_->ResizeVirtualScreen(screenId, width, height));
    }
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetVirtualMirrorScreenScaleMode
 * @tc.desc: SetVirtualMirrorScreenScaleMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualMirrorScreenScaleMode, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    ScreenScaleMode scaleMode = ScreenScaleMode::FILL_MODE;
    EXPECT_EQ(DMError::DM_OK, ssm_->SetVirtualMirrorScreenScaleMode(screenId, scaleMode));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: StopMirror
 * @tc.desc: StopMirror test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, StopMirror, Function | SmallTest | Level3)
{
    std::vector<ScreenId> mirrorScreenIds {0, 1, 2, 3, 4, 5};
    EXPECT_EQ(DMError::DM_OK, ssm_->StopMirror(mirrorScreenIds));
}

/**
 * @tc.name: GetDensityInCurResolution
 * @tc.desc: GetDensityInCurResolution screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDensityInCurResolution, Function | SmallTest | Level3)
{
    sptr<ScreenSession> screenSession = new ScreenSession();
    ASSERT_NE(nullptr, screenSession);
    ssm_->screenSessionMap_.insert(std::make_pair(1, screenSession));
    ScreenId screenId = 100;
    float x = 3.14;
    auto res = ssm_->GetDensityInCurResolution(screenId, x);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, res);
    screenId = 1;
    res = ssm_->GetDensityInCurResolution(screenId, x);
    EXPECT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetScreenOffDelayTime
 * @tc.desc: SetScreenOffDelayTime test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenOffDelayTime, Function | SmallTest | Level3)
{
    int32_t delay = CV_WAIT_SCREENOFF_MS - 1;
    int32_t ret = ssm_->SetScreenOffDelayTime(delay);
    EXPECT_EQ(ret, CV_WAIT_SCREENOFF_MS);

    delay = CV_WAIT_SCREENOFF_MS + 1;
    ret = ssm_->SetScreenOffDelayTime(delay);
    EXPECT_EQ(ret, delay);

    delay = CV_WAIT_SCREENOFF_MS_MAX - 1;
    ret = ssm_->SetScreenOffDelayTime(delay);
    EXPECT_EQ(ret, delay);

    delay = CV_WAIT_SCREENOFF_MS_MAX + 1;
    ret = ssm_->SetScreenOffDelayTime(delay);
    EXPECT_EQ(ret, CV_WAIT_SCREENOFF_MS_MAX);
}

/**
 * @tc.name: GetDeviceScreenConfig
 * @tc.desc: GetDeviceScreenConfig test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDeviceScreenConfig, Function | SmallTest | Level3)
{
    DeviceScreenConfig deviceScreenConfig = ssm_->GetDeviceScreenConfig();
    EXPECT_FALSE(deviceScreenConfig.rotationPolicy_.empty());
}

/**
 * @tc.name: SetVirtualScreenBlackList
 * @tc.desc: SetVirtualScreenBlackList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenBlackList01, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    std::vector<uint64_t> windowId = {10, 20, 30};
    ssm_->SetVirtualScreenBlackList(screenId, windowId);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetVirtualScreenBlackList
 * @tc.desc: ConvertScreenIdToRsScreenId = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenBlackList02, Function | SmallTest | Level3)
{
    ScreenId screenId = 1010;
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ASSERT_FALSE(ssm_->ConvertScreenIdToRsScreenId(screenId, rsScreenId));
    std::vector<uint64_t> windowId = {10, 20, 30};
    ssm_->SetVirtualScreenBlackList(screenId, windowId);
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayPhysicalResolution, Function | SmallTest | Level3)
{
    std::vector<DisplayPhysicalResolution> allSize {};
    if (ssm_ != nullptr) {
        allSize = ssm_->GetAllDisplayPhysicalResolution();
        ASSERT_TRUE(!allSize.empty());
    } else {
        ASSERT_TRUE(allSize.empty());
    }
}

/**
 * @tc.name: SetDisplayScale
 * @tc.desc: SetDisplayScale test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDisplayScale, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenId fakeScreenId = static_cast<ScreenId>(-1);
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    ssm_->SetDisplayScale(fakeScreenId, scaleX, scaleY, pivotX, pivotY);
    ssm_->SetDisplayScale(ssm_->GetDefaultScreenId(), scaleX, scaleY, pivotX, pivotY);
}

/**
 * @tc.name: ScreenCastConnection
 * @tc.desc: ScreenCastConnection test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScreenCastConnection, Function | SmallTest | Level3)
{
    std::string castBundleName = "testCastBundleName";
    std::string castAbilityName = "testCastAbilityName";

    ScreenCastConnection::GetInstance().SetBundleName("");
    ScreenCastConnection::GetInstance().SetAbilityName("");
    EXPECT_EQ(ScreenCastConnection::GetInstance().GetBundleName(), "");
    EXPECT_EQ(ScreenCastConnection::GetInstance().GetAbilityName(), "");

    EXPECT_EQ(ScreenCastConnection::GetInstance().CastConnectExtension(1), false);

    ScreenCastConnection::GetInstance().SetBundleName(castBundleName);
    ScreenCastConnection::GetInstance().SetAbilityName(castAbilityName);
    EXPECT_EQ(ScreenCastConnection::GetInstance().GetBundleName(), castBundleName);
    EXPECT_EQ(ScreenCastConnection::GetInstance().GetAbilityName(), castAbilityName);

    MessageParcel data;
    MessageParcel reply;
    ScreenCastConnection::GetInstance().SendMessageToCastService(CAST_WIRED_PROJECTION_START, data, reply);
    ScreenCastConnection::GetInstance().SendMessageToCastService(CAST_WIRED_PROJECTION_STOP, data, reply);
    ScreenCastConnection::GetInstance().CastDisconnectExtension();
    EXPECT_EQ(ScreenCastConnection::GetInstance().IsConnectedSync(), false);
}

/**
 * @tc.name: ReportFoldStatusToScb
 * @tc.desc: clientProxy_ = nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ReportFoldStatusToScb, Function | SmallTest | Level3)
{
    ssm_->clientProxy_ = nullptr;
    FoldStatus currentStatus = FoldStatus::FOLDED;
    FoldStatus nextStatus = FoldStatus::HALF_FOLD;
    int32_t duration = 1;
    float postureAngle = 1.2;
    std::vector<std::string> screenFoldInfo {std::to_string(static_cast<int32_t>(currentStatus)),
        std::to_string(static_cast<int32_t>(nextStatus)), std::to_string(duration), std::to_string(postureAngle)};
    ssm_->ReportFoldStatusToScb(screenFoldInfo);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: DisablePowerOffRenderControl
 * @tc.desc: ConvertScreenIdToRsScreenId = true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, DisablePowerOffRenderControl01, Function | SmallTest | Level3)
{
    ScreenId screenId = 1050;
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ScreenId rsScreenId1 = 1060;
    ssm_->screenIdManager_.sms2RsScreenIdMap_[screenId] = rsScreenId1;
    ASSERT_TRUE(ssm_->ConvertScreenIdToRsScreenId(screenId, rsScreenId));
    ssm_->DisablePowerOffRenderControl(screenId);
}

/**
 * @tc.name: DisablePowerOffRenderControl
 * @tc.desc: ConvertScreenIdToRsScreenId = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, DisablePowerOffRenderControl02, Function | SmallTest | Level3)
{
    ScreenId screenId = 1010;
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ASSERT_FALSE(ssm_->ConvertScreenIdToRsScreenId(screenId, rsScreenId));
    ssm_->DisablePowerOffRenderControl(screenId);
}

/**
 * @tc.name: CheckAndSendHiSysEvent
 * @tc.desc: eventName != "CREATE_VIRTUAL_SCREEN"
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckAndSendHiSysEvent, Function | SmallTest | Level3)
{
    std::string eventName =  "GET_DISPLAY_SNAPSHOT";
    std::string bundleName = "hmos.screenshot";
    ssm_->CheckAndSendHiSysEvent(eventName, bundleName);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: NotifyFoldToExpandCompletion
 * @tc.desc: NotifyFoldToExpandCompletion
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyFoldToExpandCompletion, Function | SmallTest | Level3)
{
    bool foldToExpand = false;
    ssm_->NotifyFoldToExpandCompletion(foldToExpand);

    foldToExpand = true;
    ssm_->NotifyFoldToExpandCompletion(foldToExpand);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateAvailableArea
 * @tc.desc: screenSession == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea01, Function | SmallTest | Level3)
{
    ScreenId screenId = 1050;
    DMRect area = DMRect{};
    ssm_->UpdateAvailableArea(screenId, area);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateAvailableArea
 * @tc.desc: screenSession != nullptr screenSession->UpdateAvailableArea(area) is true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea02, Function | SmallTest | Level3)
{
    ScreenId screenId = 1050;
    DMRect area{0, 0, 600, 900};
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    auto screenSession1 = ssm_->GetScreenSession(screenId);
    ASSERT_EQ(screenSession1, screenSession);
    ASSERT_TRUE(screenSession->UpdateAvailableArea(area));
    ssm_->UpdateAvailableArea(screenId, area);
}

/**
 * @tc.name: UpdateAvailableArea
 * @tc.desc: screenSession != nullptr screenSession->UpdateAvailableArea(area) is false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea03, Function | SmallTest | Level3)
{
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    DMRect area = screenSession->GetAvailableArea();
    ssm_->screenSessionMap_[screenId] = screenSession;
    ASSERT_FALSE(screenSession->UpdateAvailableArea(area));
    ssm_->UpdateAvailableArea(screenId, area);
}

/**
 * @tc.name: NotifyAvailableAreaChanged
 * @tc.desc: agents.empty()
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyAvailableAreaChanged01, Function | SmallTest | Level3)
{
    DMRect area = DMRect{};
    ASSERT_NE(ssm_, nullptr);
    ssm_->NotifyAvailableAreaChanged(area);
}

/**
 * @tc.name: NotifyFoldStatusChanged
 * @tc.desc: NotifyFoldStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyFoldStatusChanged02, Function | SmallTest | Level3)
{
    std::string statusParam;
    auto ret = ssm_->NotifyFoldStatusChanged(statusParam);
    ASSERT_EQ(ret, -1);

    statusParam =  "-z";     // STATUS_FOLD_HALF
    ret = ssm_->NotifyFoldStatusChanged(statusParam);
    ASSERT_EQ(ret, 0);

    statusParam = "-y";     // STATUS_EXPAND
    ret = ssm_->NotifyFoldStatusChanged(statusParam);
    ASSERT_EQ(ret, 0);

    statusParam = "-coor";     // ARG_FOLD_DISPLAY_COOR
    ret = ssm_->NotifyFoldStatusChanged(statusParam);

    if (!(ssm_->IsFoldable())) {
        ssm_->foldScreenController_ = new FoldScreenController(
            ssm_->displayInfoMutex_, ssm_->screenPowerTaskScheduler_);
    }
    ASSERT_NE(ssm_->foldScreenController_, nullptr);
    statusParam = "-y";
    ssm_->HandleFoldScreenPowerInit();
    ret = ssm_->NotifyFoldStatusChanged(statusParam);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: Dump
 * @tc.desc: Dump
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, Dump, Function | SmallTest | Level3)
{
    int fd = 2;
    std::vector<std::u16string> args;
    int ret = ssm_->Dump(fd, args);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: GetDisplayNode
 * @tc.desc: GetDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayNode01, Function | SmallTest | Level3)
{
    ScreenId screenId = 1051;
    auto ret = ssm_->GetDisplayNode(screenId);
    ASSERT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetDisplayNode
 * @tc.desc: !screenSession = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayNode02, Function | SmallTest | Level3)
{
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    auto ret = ssm_->GetDisplayNode(screenId);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: GetScreenProperty
 * @tc.desc: GetScreenProperty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenProperty01, Function | SmallTest | Level3)
{
    ScreenId screenId = 2000;
    ASSERT_EQ(ssm_->GetScreenSession(screenId), nullptr);
    auto ret = ssm_->GetScreenProperty(screenId);
    ASSERT_EQ(sizeof(ret), sizeof(ScreenProperty));
}

/**
 * @tc.name: GetScreenProperty
 * @tc.desc: !screenSession = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenProperty02, Function | SmallTest | Level3)
{
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ScreenProperty property = ssm_->GetScreenProperty(screenId);
    ASSERT_EQ(sizeof(property), sizeof(screenSession->property_));
}

/**
 * @tc.name: GetCurrentScreenPhyBounds
 * @tc.desc: GetCurrentScreenPhyBounds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentScreenPhyBounds01, Function | SmallTest | Level3)
{
    float phyWidth = 0.0f;
    float phyHeight = 0.0f;
    bool isReset = true;
    ScreenId screenId = 1050;
    ssm_->GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
    if (!(ssm_->IsFoldable())) {
        ssm_->foldScreenController_ = new FoldScreenController(
            ssm_->displayInfoMutex_, ssm_->screenPowerTaskScheduler_);
    }
    
    ASSERT_NE(ssm_->foldScreenController_, nullptr);
    ssm_->GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
    ASSERT_FALSE(isReset);
}

/**
 * @tc.name: PhyMirrorConnectWakeupScreen
 * @tc.desc: PhyMirrorConnectWakeupScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, PhyMirrorConnectWakeupScreen, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->PhyMirrorConnectWakeupScreen();
    ScreenSceneConfig::stringConfig_["externalScreenDefaultMode"] = "mirror";
    ssm_->PhyMirrorConnectWakeupScreen();
}
}
} // namespace Rosen
} // namespace OHOS
