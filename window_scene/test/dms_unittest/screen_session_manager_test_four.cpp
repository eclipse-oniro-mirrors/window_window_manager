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
 * @tc.name: GetTentMode
 * @tc.desc: Test get tent mode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetTentMode, TestSize.Level1)
{
    auto tentMode = ssm_->GetTentMode();
    ASSERT_EQ(tentMode, false);
}

/**
 * @tc.name: OnTentModeChanged
 * @tc.desc: Test change tent mode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnTentModeChanged, TestSize.Level1)
{
    bool isTentMode = false;
    ssm_->OnTentModeChanged(isTentMode);
    ASSERT_EQ(ssm_->GetTentMode(), false);
}

/**
 * @tc.name: GetScreenCapture
 * @tc.desc: GetScreenCapture
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenCapture, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    CaptureOption option;
    option.displayId_ = 0;
    DmErrorCode errCode;
    std::shared_ptr<Media::PixelMap> bitMap = ssm->GetScreenCapture(option, &errCode);
    if (errCode == DmErrorCode::DM_OK) {
        ASSERT_NE(bitMap, nullptr);
    } else {
        ASSERT_EQ(bitMap, nullptr);
    }
}

/**
 * @tc.name: OnScreenCaptureNotify
 * @tc.desc: OnScreenCaptureNotify
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenCaptureNotify, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 0;
    int32_t uid = 0;
    std::string clientName = "test";
    ssm->OnScreenCaptureNotify(screenId, uid, clientName);
}

/*
 * @tc.name: TransferTypeToString
 * @tc.desc: TransferTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferTypeToString1, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferTypeToString(ScreenType::REAL);
    std::string expectType = "REAL";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferTypeToString
 * @tc.desc: TransferTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferTypeToString2, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferTypeToString(ScreenType::VIRTUAL);
    std::string expectType = "VIRTUAL";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferTypeToString
 * @tc.desc: TransferTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferTypeToString3, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferTypeToString(ScreenType::UNDEFINED);
    std::string expectType = "UNDEFINED";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferPropertyChangeTypeToString
 * @tc.desc: TransferPropertyChangeTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferPropertyChangeTypeToString1, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferPropertyChangeTypeToString(ScreenPropertyChangeType::UNSPECIFIED);
    std::string expectType = "UNSPECIFIED";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferPropertyChangeTypeToString
 * @tc.desc: TransferPropertyChangeTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferPropertyChangeTypeToString2, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferPropertyChangeTypeToString(ScreenPropertyChangeType::ROTATION_BEGIN);
    std::string expectType = "ROTATION_BEGIN";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferPropertyChangeTypeToString
 * @tc.desc: TransferPropertyChangeTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferPropertyChangeTypeToString3, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferPropertyChangeTypeToString(ScreenPropertyChangeType::ROTATION_END);
    std::string expectType = "ROTATION_END";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferPropertyChangeTypeToString
 * @tc.desc: TransferPropertyChangeTypeToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferPropertyChangeTypeToString4, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferPropertyChangeTypeToString(
        ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY);
    std::string expectType = "ROTATION_UPDATE_PROPERTY_ONLY";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: ConvertOffsetToCorrectRotation
 * @tc.desc: ConvertOffsetToCorrectRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConvertOffsetToCorrectRotation, TestSize.Level1)
{
    int32_t phyOffset = 90;
    ASSERT_EQ(ssm_->ConvertOffsetToCorrectRotation(phyOffset), ScreenRotation::ROTATION_270);
    phyOffset = 180;
    ASSERT_EQ(ssm_->ConvertOffsetToCorrectRotation(phyOffset), ScreenRotation::ROTATION_180);
    phyOffset = 270;
    ASSERT_EQ(ssm_->ConvertOffsetToCorrectRotation(phyOffset), ScreenRotation::ROTATION_90);
    phyOffset = 0;
    ASSERT_EQ(ssm_->ConvertOffsetToCorrectRotation(phyOffset), ScreenRotation::ROTATION_0);
}

/**
 * @tc.name: ConfigureScreenSnapshotParams
 * @tc.desc: ConfigureScreenSnapshotParams
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConfigureScreenSnapshotParams, TestSize.Level1)
{
    auto stringConfig = ScreenSceneConfig::GetStringConfig();
    if (g_isPcDevice) {
        ASSERT_EQ(stringConfig.count("screenSnapshotBundleName"), 1);
    } else {
        ASSERT_EQ(stringConfig.count("screenSnapshotBundleName"), 0);
    }
    ssm_->ConfigureScreenSnapshotParams();
}

/**
 * @tc.name: RegisterRefreshRateChangeListener
 * @tc.desc: RegisterRefreshRateChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RegisterRefreshRateChangeListener, TestSize.Level1)
{
    ssm_->RegisterRefreshRateChangeListener();
    std::string ret = ssm_->screenEventTracker_.recordInfos_.back().info;
    ASSERT_NE(ret, "Dms RefreshRateChange register failed.");
}

/**
 * @tc.name: FreeDisplayMirrorNodeInner
 * @tc.desc: FreeDisplayMirrorNodeInner
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, FreeDisplayMirrorNodeInner, TestSize.Level1)
{
    sptr<ScreenSession> mirrorSession = nullptr;
    ssm_->FreeDisplayMirrorNodeInner(mirrorSession);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_EQ(ssm_->GetScreenSession(screenId)->GetDisplayNode(), nullptr);
}

/**
 * @tc.name: GetPowerStatus
 * @tc.desc: GetPowerStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPowerStatus01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenPowerStatus status;
    ssm_->GetPowerStatus(ScreenPowerState::POWER_ON, PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_ON_ADVANCED);
    ssm_->GetPowerStatus(ScreenPowerState::POWER_ON, PowerStateChangeReason::POWER_BUTTON, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_ON);
}

/**
 * @tc.name: GetPowerStatus
 * @tc.desc: GetPowerStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPowerStatus02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenPowerStatus status;
    ssm_->GetPowerStatus(ScreenPowerState::POWER_OFF,
        PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED);
    ssm_->GetPowerStatus(ScreenPowerState::POWER_OFF, PowerStateChangeReason::POWER_BUTTON, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_OFF);
}

/**
 * @tc.name: GetPowerStatus
 * @tc.desc: GetPowerStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPowerStatus03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenPowerStatus status;
    ssm_->GetPowerStatus(ScreenPowerState::POWER_SUSPEND, PowerStateChangeReason::POWER_BUTTON, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_SUSPEND);
}

/**
 * @tc.name: GetPowerStatus
 * @tc.desc: GetPowerStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPowerStatus04, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenPowerStatus status;
    ssm_->GetPowerStatus(ScreenPowerState::POWER_DOZE, PowerStateChangeReason::POWER_BUTTON, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_DOZE);
}

/**
 * @tc.name: GetPowerStatus
 * @tc.desc: GetPowerStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPowerStatus05, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenPowerStatus status;
    ssm_->GetPowerStatus(ScreenPowerState::POWER_DOZE_SUSPEND, PowerStateChangeReason::POWER_BUTTON, status);
    ASSERT_EQ(status, ScreenPowerStatus::POWER_STATUS_DOZE_SUSPEND);
}

/**
 * @tc.name: SetGotScreenOffAndWakeUpBlock
 * @tc.desc: SetGotScreenOffAndWakeUpBlock test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetGotScreenOffAndWakeUpBlock, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetGotScreenOffAndWakeUpBlock();
    ASSERT_TRUE(ssm_->gotScreenOffNotify_);
    ssm_->needScreenOffNotify_ = true;
    ssm_->SetGotScreenOffAndWakeUpBlock();
    ASSERT_FALSE(ssm_->needScreenOffNotify_);
}

/**
 * @tc.name: SetLowTemp
 * @tc.desc: SetLowTemp test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetLowTemp, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetLowTemp(LowTempMode::LOW_TEMP_OFF);
    ASSERT_EQ(ssm_->lowTemp_, LowTempMode::LOW_TEMP_OFF);
    ssm_->SetLowTemp(LowTempMode::LOW_TEMP_ON);
    ASSERT_EQ(ssm_->lowTemp_, LowTempMode::LOW_TEMP_ON);
}

/**
 * @tc.name: SetScreenSkipProtectedWindow
 * @tc.desc: SetScreenSkipProtectedWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenSkipProtectedWindow, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption1";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    const std::vector<ScreenId> screenIds = {screenId, 1234};
    bool isEnable = true;
    ASSERT_EQ(ssm_->SetScreenSkipProtectedWindow(screenIds, isEnable), DMError::DM_OK);
    isEnable = false;
    ASSERT_EQ(ssm_->SetScreenSkipProtectedWindow(screenIds, isEnable), DMError::DM_OK);
}

/**
 * @tc.name: GetDisplayCapability
 * @tc.desc: GetDisplayCapability test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayCapability, TestSize.Level1)
{
    std::string info {""};
    if (ssm_ != nullptr) {
        EXPECT_EQ(DMError::DM_OK, ssm_->GetDisplayCapability(info));
        ASSERT_NE(info, "");
    } else {
        ASSERT_EQ(info, "");
    }
}

/**
 * @tc.name: GetSecondaryDisplayCapability
 * @tc.desc: GetSecondaryDisplayCapability test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSecondaryDisplayCapability, TestSize.Level1)
{
    std::string info {""};
    if (ssm_ != nullptr) {
        EXPECT_EQ(DMError::DM_OK, ssm_->GetSecondaryDisplayCapability(info));
        ASSERT_NE(info, "");
    } else {
        ASSERT_EQ(info, "");
    }
}

/**
 * @tc.name: GetSuperFoldCapability
 * @tc.desc: GetSuperFoldCapability test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSuperFoldCapability, TestSize.Level1)
{
    std::string info {""};
    if (ssm_ != nullptr) {
        EXPECT_EQ(DMError::DM_OK, ssm_->GetSuperFoldCapability(info));
        ASSERT_NE(info, "");
    } else {
        ASSERT_EQ(info, "");
    }
}

/**
 * @tc.name: GetFoldableDeviceCapability
 * @tc.desc: GetFoldableDeviceCapability test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetFoldableDeviceCapability, TestSize.Level1)
{
    std::string info {""};
    if (ssm_ != nullptr) {
        EXPECT_EQ(DMError::DM_OK, ssm_->GetFoldableDeviceCapability(info));
        ASSERT_NE(info, "");
    } else {
        ASSERT_EQ(info, "");
    }
}

/**
 * @tc.name: DoMakeUniqueScreenOld
 * @tc.desc: DoMakeUniqueScreenOld test
 * @tc.type: FUNC
 */
 HWTEST_F(ScreenSessionManagerTest, DoMakeUniqueScreenOld, TestSize.Level1)
 {
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    std::vector<ScreenId> allUniqueScreenIds = {screenId, 99};
    std::vector<DisplayId> displayIds = {};
    ssm_->DoMakeUniqueScreenOld(allUniqueScreenIds, displayIds, false);
    ASSERT_EQ(displayIds.size(), 1);
}

/**
 * @tc.name: SetCastPrivacyFromSettingData
 * @tc.desc: SetCastPrivacyFromSettingData test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetCastPrivacyFromSettingData, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    ScreenId id = 2;
    sptr<ScreenSession> newSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(newSession, nullptr);
    ssm_->screenSessionMap_[id] = newSession;
    ssm_->SetCastPrivacyFromSettingData();
}

/**
 * @tc.name: RegisterSettingWireCastObserver
 * @tc.desc: RegisterSettingWireCastObserver
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RegisterSettingWireCastObserver, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    ScreenId id = 2;
    sptr<ScreenSession> newSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(newSession, nullptr);
    ssm_->screenSessionMap_[id] = newSession;
    ssm_->RegisterSettingWireCastObserver(newSession);
}

/**
 * @tc.name: UnregisterSettingWireCastObserver
 * @tc.desc: UnregisterSettingWireCastObserver
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UnregisterSettingWireCastObserver, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    ScreenId id = 2;
    sptr<ScreenSession> newSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(newSession, nullptr);
    ssm_->screenSessionMap_[id] = newSession;
    ssm_->UnregisterSettingWireCastObserver(id);
}

/**
 * @tc.name: UpdateValidArea
 * @tc.desc: UpdateValidArea
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateValidArea, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->UpdateValidArea(2000, 800, 1000);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);

    uint32_t originValidWidth = screenSession->GetValidWidth();
    uint32_t originValidHeight = screenSession->GetValidHeight();
    ssm_->UpdateValidArea(screenId, 800, 1000);
    EXPECT_EQ(800, screenSession->GetValidWidth());
    EXPECT_EQ(1000, screenSession->GetValidHeight());
    ssm_->UpdateValidArea(screenId, originValidWidth, originValidHeight);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetIsRealScreen
 * @tc.desc: GetIsRealScreen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetIsRealScreen, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    EXPECT_EQ(ssm_->GetIsRealScreen(2000), false);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    screenSession->SetIsRealScreen(true);
    ASSERT_EQ(ssm_->GetIsRealScreen(screenId), true);
    screenSession->SetIsRealScreen(false);
    ASSERT_EQ(ssm_->GetIsRealScreen(screenId), false);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetSystemKeyboardStatus
 * @tc.desc: SetSystemKeyboardStatus with true as parameter
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetSystemKeyboardStatus01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    auto ret = ssm_->SetSystemKeyboardStatus(true);
    ASSERT_NE(ret, DMError::DM_ERROR_UNKNOWN);
}

/**
 * @tc.name: SetSystemKeyboardStatus
 * @tc.desc: SetSystemKeyboardStatus with false as parameter
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetSystemKeyboardStatus02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    auto ret = ssm_->SetSystemKeyboardStatus(false);
    ASSERT_NE(ret, DMError::DM_ERROR_UNKNOWN);
}

/**
 * @tc.name: CalculateXYPosition
 * @tc.desc: CalculateXYPosition test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateXYPosition, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(true);
    int32_t x = screenSession->GetScreenProperty().GetX();
    EXPECT_EQ(0, x);
    int32_t y = screenSession->GetScreenProperty().GetY();
    EXPECT_EQ(0, y);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: IsSpecialApp
 * @tc.desc: IsSpecialApp
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsSpecialApp, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    auto ret = ssm_->IsSpecialApp();
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: CanEnterCoordination01
 * @tc.desc: CanEnterCoordination01 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CanEnterCoordination01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->virtualScreenCount_ = 0;
    ssm_->hdmiScreenCount_ = 1;
    auto ret = ssm_->CanEnterCoordination();
    EXPECT_EQ(ret, DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING);

    ssm_->virtualScreenCount_ = 0;
    ssm_->hdmiScreenCount_ = 0;
    ret = ssm_->CanEnterCoordination();
    EXPECT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: CanEnterCoordination02
 * @tc.desc: CanEnterCoordination02 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CanEnterCoordination02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->screenSessionMap_[50] = nullptr;
    ssm_->virtualScreenCount_ = 1;
    ssm_->hdmiScreenCount_ = 0;

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    virtualOption.virtualScreenType_ = VirtualScreenType::SCREEN_CASTING;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto ret = ssm_->CanEnterCoordination();
    EXPECT_EQ(ret, DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRLESS_CASTING);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: CanEnterCoordination03
 * @tc.desc: CanEnterCoordination03 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CanEnterCoordination03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    auto oldMap = ssm_->screenSessionMap_;
    std::map<ScreenId, sptr<ScreenSession>> newScreenSessionMap_{};
    ssm_->screenSessionMap_ = newScreenSessionMap_;
    ssm_->virtualScreenCount_ = 1;
    ssm_->hdmiScreenCount_ = 0;
    auto ret = ssm_->CanEnterCoordination();
    EXPECT_EQ(ret, DMError::DM_OK);
    ssm_->screenSessionMap_ = oldMap;
}

/**
 * @tc.name: GetCameraPosition
 * @tc.desc: GetCameraPosition
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCameraPosition, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    ssm_->cameraPosition_ = 1;
    auto ret = ssm_->GetCameraPosition();
    ASSERT_EQ(ret, 1);
}

/**
 * @tc.name: GetCameraStatus
 * @tc.desc: GetCameraStatus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCameraStatus, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    ssm_->cameraStatus_ = 1;
    auto ret = ssm_->GetCameraStatus();
    ASSERT_EQ(ret, 1);
}

/**
 * @tc.name: OnSecondaryReflexionChange
 * @tc.desc: OnSecondaryReflexionChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnSecondaryReflexionChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    ssm_->OnSecondaryReflexionChange(0, false);
    ASSERT_EQ(ssm_->cameraStatus_, 1);
}

/**
 * @tc.name: OnExtendScreenConnectStatusChange
 * @tc.desc: OnExtendScreenConnectStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnExtendScreenConnectStatusChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    ExtendScreenConnectStatus status = ExtendScreenConnectStatus::UNKNOWN;
    ssm_->OnExtendScreenConnectStatusChange(0, status);
    ASSERT_EQ(ssm_->cameraStatus_, 1);
}

/**
 * @tc.name: OnSuperFoldStatusChange
 * @tc.desc: OnSuperFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnSuperFoldStatusChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ASSERT_EQ(ssm_->clientProxy_, nullptr);

    SuperFoldStatus status = SuperFoldStatus::UNKNOWN;
    ssm_->OnSuperFoldStatusChange(0, status);
    ASSERT_EQ(ssm_->cameraStatus_, 1);
}

/**
 * @tc.name: GetPrimaryDisplayInfo02
 * @tc.desc: GetPrimaryDisplayInfo02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetPrimaryDisplayInfo02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    ssm_->screenSessionMap_.insert(std::pair<ScreenId, sptr<ScreenSession>>(4, nullptr));
    auto ret = ssm_->GetPrimaryDisplayInfo();
    ASSERT_NE(ret, nullptr);

    auto screenSession = new ScreenSession;
    screenSession->SetIsExtend(true);
    ssm_->screenSessionMap_.insert(std::pair<ScreenId, sptr<ScreenSession>>(6, screenSession));
    ret = ssm_->GetPrimaryDisplayInfo();
    ASSERT_NE(ret, nullptr);

    screenSession->SetIsExtend(false);
    ret = ssm_->GetPrimaryDisplayInfo();
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: AddPermissionUsedRecord
 * @tc.desc: AddPermissionUsedRecord
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AddPermissionUsedRecord, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    ssm_->AddPermissionUsedRecord("ohos.permission.CUSTOM_SCREEN_CAPTURE", 0, 1);
    ASSERT_EQ(ssm_->cameraStatus_, 1);
}

/**
 * @tc.name: SetVirtualScreenMaxRefreshRate
 * @tc.desc: SetVirtualScreenMaxRefreshRate
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenMaxRefreshRate, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    uint32_t actualRefreshRate = 0;

    auto ret = ssm_->SetVirtualScreenMaxRefreshRate(0, 120, actualRefreshRate);
    ASSERT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);

    ret = ssm_->SetVirtualScreenMaxRefreshRate(screenId, 120, actualRefreshRate);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: OnScreenExtendChange
 * @tc.desc: OnScreenExtendChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenExtendChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    ssm_->OnScreenExtendChange(0, 12);
    ASSERT_EQ(ssm_->cameraStatus_, 1);
}

/**
 * @tc.name: SetMultiScreenDefaultRelativePosition
 * @tc.desc: SetMultiScreenDefaultRelativePosition
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenDefaultRelativePosition, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession1 = new ScreenSession(50, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->SetIsRealScreen(true);
    screenSession1->SetIsExtend(true);
    ssm_->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetIsRealScreen(false);
    ssm_->screenSessionMap_[51] = screenSession2;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession3);
    screenSession3->SetIsRealScreen(true);
    screenSession3->SetIsExtend(false);
    ssm_->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = nullptr;
    ASSERT_EQ(nullptr, screenSession4);
    ssm_->screenSessionMap_[53] = screenSession4;

    ssm_->SetMultiScreenDefaultRelativePosition();
    ASSERT_NE(ssm_->screenSessionMap_.empty(), true);
    ssm_->screenSessionMap_.erase(50);
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
    ssm_->screenSessionMap_.erase(53);
}

/**
 * @tc.name: ConvertEdidToString
 * @tc.desc: ConvertEdidToString
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ConvertEdidToString, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    struct BaseEdid edid = {
        .manufacturerName_ = "abc",
        .productCode_ = 101,
        .serialNumber_ = 1401,
        .weekOfManufactureOrModelYearFlag_ = 1,
        .yearOfManufactureOrModelYear_ = 2025,
    };
    std::string edidInfo = edid.manufacturerName_ + std::to_string(edid.productCode_)
        + std::to_string(edid.serialNumber_) + std::to_string(edid.weekOfManufactureOrModelYearFlag_)
        + std::to_string(edid.yearOfManufactureOrModelYear_);
    std::hash<std::string> hasher;
    std::size_t hashValue = hasher(edidInfo);
    std::ostringstream oss;
    oss << std::hex << std::uppercase << hashValue;

    auto str = ssm_->ConvertEdidToString(edid);
    ASSERT_EQ(str, oss.str());
}

/**
 * @tc.name: RecoverRestoredMultiScreenMode
 * @tc.desc: RecoverRestoredMultiScreenMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RecoverRestoredMultiScreenMode, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession1 = new ScreenSession(50, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->SetScreenType(ScreenType::REAL);
    auto ret = ssm_->RecoverRestoredMultiScreenMode(screenSession1);
    ASSERT_EQ(ret, false);

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetScreenType(ScreenType::VIRTUAL);
    ret = ssm_->RecoverRestoredMultiScreenMode(screenSession2);
    ASSERT_EQ(ret, true);
    ssm_->screenSessionMap_.erase(50);
    ssm_->screenSessionMap_.erase(51);
}

/**
 * @tc.name: CheckMultiScreenInfoMap
 * @tc.desc: CheckMultiScreenInfoMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CheckMultiScreenInfoMap, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    std::map<std::string, MultiScreenInfo> multiScreenInfoMap;
    ASSERT_EQ(multiScreenInfoMap.empty(), true);
    std::string serialNumber = "";
    auto ret = ssm_->CheckMultiScreenInfoMap(multiScreenInfoMap, serialNumber);
    ASSERT_EQ(ret, false);

    MultiScreenInfo info1;
    std::map<std::string, MultiScreenInfo> multiScreenInfoMap1;
    multiScreenInfoMap1["123"] = info1;
    ASSERT_EQ(multiScreenInfoMap1.empty(), false);
    std::string serialNumber1 = "";
    ret = ssm_->CheckMultiScreenInfoMap(multiScreenInfoMap1, serialNumber1);
    ASSERT_EQ(ret, false);

    MultiScreenInfo info2;
    std::map<std::string, MultiScreenInfo> multiScreenInfoMap2;
    multiScreenInfoMap2["123"] = info2;
    ASSERT_EQ(multiScreenInfoMap2.empty(), false);
    std::string serialNumber2 = "123";
    ret = ssm_->CheckMultiScreenInfoMap(multiScreenInfoMap2, serialNumber2);
    ASSERT_EQ(ret, true);

    MultiScreenInfo info3;
    std::map<std::string, MultiScreenInfo> multiScreenInfoMap3;
    multiScreenInfoMap3["123"] = info3;
    ASSERT_EQ(multiScreenInfoMap3.empty(), false);
    std::string serialNumber3 = "456";
    ret = ssm_->CheckMultiScreenInfoMap(multiScreenInfoMap3, serialNumber3);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: SetMultiScreenFrameControl
 * @tc.desc: SetMultiScreenFrameControl
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetMultiScreenFrameControl, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession1 = new ScreenSession(50, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    screenSession1->SetScreenType(ScreenType::REAL);
    screenSession1->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetScreenType(ScreenType::VIRTUAL);
    screenSession2->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_[51] = screenSession2;

    ssm_->SetMultiScreenFrameControl();
    ASSERT_NE(ssm_->screenSessionMap_.empty(), true);

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession3);
    screenSession3->SetScreenType(ScreenType::REAL);
    screenSession3->SetIsCurrentInUse(true);
    ssm_->screenSessionMap_[52] = screenSession3;

    ssm_->SetMultiScreenFrameControl();
    ASSERT_NE(ssm_->screenSessionMap_.empty(), true);
    ssm_->screenSessionMap_.erase(50);
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
}

/**
 * @tc.name: GetInternalScreenSession
 * @tc.desc: GetInternalScreenSession
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetInternalScreenSession, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession1 = nullptr;
    ASSERT_EQ(nullptr, screenSession1);
    ssm_->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetScreenType(ScreenType::REAL);
    screenSession2->isInternal_ = false;
    ssm_->screenSessionMap_[51] = screenSession2;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession3);
    screenSession3->SetScreenType(ScreenType::VIRTUAL);
    screenSession3->isInternal_ = false;
    ssm_->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = new ScreenSession(53, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession4);
    screenSession4->SetScreenType(ScreenType::VIRTUAL);
    screenSession4->isInternal_ = true;
    ssm_->screenSessionMap_[53] = screenSession4;

    sptr<ScreenSession> screenSession5 = new ScreenSession(54, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession5);
    screenSession5->SetScreenType(ScreenType::REAL);
    screenSession5->isInternal_ = true;
    ssm_->screenSessionMap_[54] = screenSession5;

    auto ret = ssm_->GetInternalScreenSession();
    ASSERT_NE(nullptr, ret);
    ssm_->screenSessionMap_.erase(50);
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
    ssm_->screenSessionMap_.erase(53);
    ssm_->screenSessionMap_.erase(54);
}

/**
 * @tc.name: GetInternalAndExternalSession
 * @tc.desc: GetInternalAndExternalSession
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetInternalAndExternalSession, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    sptr<ScreenSession> screenSession1 = nullptr;
    ASSERT_EQ(nullptr, screenSession1);
    ssm_->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession2);
    screenSession2->SetIsCurrentInUse(false);
    ssm_->screenSessionMap_[51] = screenSession2;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession3);
    screenSession3->SetIsCurrentInUse(true);
    screenSession3->SetScreenType(ScreenType::VIRTUAL);
    ssm_->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = new ScreenSession(53, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession4);
    screenSession3->SetIsCurrentInUse(true);
    screenSession3->SetScreenType(ScreenType::REAL);
    screenSession3->isInternal_ = false;
    ssm_->screenSessionMap_[53] = screenSession4;

    sptr<ScreenSession> screenSession5 = new ScreenSession(54, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession5);
    screenSession5->SetIsCurrentInUse(true);
    screenSession5->SetScreenType(ScreenType::REAL);
    screenSession5->isInternal_ = true;
    ssm_->screenSessionMap_[54] = screenSession5;

    sptr<ScreenSession> externalSession = nullptr;
    sptr<ScreenSession> internalSession = nullptr;
    ssm_->GetInternalAndExternalSession(internalSession, externalSession);
    ASSERT_EQ(internalSession, screenSession5);
    ssm_->screenSessionMap_.erase(50);
    ssm_->screenSessionMap_.erase(51);
    ssm_->screenSessionMap_.erase(52);
    ssm_->screenSessionMap_.erase(53);
    ssm_->screenSessionMap_.erase(54);
}

/**
 * @tc.name: GetFakePhysicalScreenSession
 * @tc.desc: GetFakePhysicalScreenSession
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetFakePhysicalScreenSession, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);

    auto ret = ssm_->GetFakePhysicalScreenSession(50, 0, ScreenProperty());
    if (g_isPcDevice) {
        ASSERT_NE(nullptr, ret);
    } else {
        ASSERT_EQ(nullptr, ret);
    }
}
}
} // namespace Rosen
} // namespace OHOS