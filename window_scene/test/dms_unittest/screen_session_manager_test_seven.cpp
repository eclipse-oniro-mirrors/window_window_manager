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
 * @tc.name: GetScreenHDRFormat
 * @tc.desc: GetScreenHDRFormat test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenHDRFormat, TestSize.Level1)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    ScreenHDRFormat hdrFormat;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenHDRFormat(SCREEN_ID_INVALID, hdrFormat));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetScreenHDRFormat(id, hdrFormat), screenSession->GetScreenHDRFormat(hdrFormat));
#endif
}

/**
 * @tc.name: SetScreenHDRFormat
 * @tc.desc: SetScreenHDRFormat test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenHDRFormat, TestSize.Level1)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    int32_t modeIdx {0};
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenHDRFormat(SCREEN_ID_INVALID, modeIdx));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->SetScreenHDRFormat(id, modeIdx), screenSession->SetScreenHDRFormat(modeIdx));
#endif
}

/**
 * @tc.name: GetSupportedColorSpaces
 * @tc.desc: GetSupportedColorSpaces test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetSupportedColorSpaces, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetSupportedColorSpaces(SCREEN_ID_INVALID, colorSpaces));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetSupportedColorSpaces(id, colorSpaces), screenSession->GetSupportedColorSpaces(colorSpaces));
#endif
}

/**
 * @tc.name: GetScreenColorSpace
 * @tc.desc: GetScreenColorSpace test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenColorSpace, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    GraphicCM_ColorSpaceType colorSpace;
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->GetScreenColorSpace(SCREEN_ID_INVALID, colorSpace));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->GetScreenColorSpace(id, colorSpace), screenSession->GetScreenColorSpace(colorSpace));
#endif
}

/**
 * @tc.name: SetScreenColorSpace
 * @tc.desc: SetScreenColorSpace test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenColorSpace, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType{GRAPHIC_CM_COLORSPACE_NONE};
    EXPECT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetScreenColorSpace(SCREEN_ID_INVALID, colorSpace));
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(ssm_->SetScreenColorSpace(id, colorSpace), screenSession->SetScreenColorSpace(colorSpace));
#endif
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: HasPrivateWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasPrivateWindow, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, GetAvailableArea01, TestSize.Level1)
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
 * @tc.name: GetAvailableArea
 * @tc.desc: GetAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAvailableArea02, TestSize.Level1)
{
    DMRect area;
    DisplayId id = 999;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_.clear();
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->GetAvailableArea(id, area));
    ssm_->screenSessionMap_.clear();
}


/**
 * @tc.name: GetExpandAvailableArea
 * @tc.desc: GetExpandAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetExpandAvailableArea02, TestSize.Level1)
{
    DMRect area;
    DisplayId id = 0;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->GetExpandAvailableArea(id, area));
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_.clear();
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_NE(nullptr, screenSession);
    EXPECT_EQ(DMError::DM_OK, ssm_->GetExpandAvailableArea(id, area));
    ssm_->screenSessionMap_.clear();
}

/**
 * @tc.name: ResetAllFreezeStatus
 * @tc.desc: ResetAllFreezeStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ResetAllFreezeStatus, TestSize.Level1)
{
    EXPECT_EQ(DMError::DM_OK, ssm_->ResetAllFreezeStatus());
}

/**
 * @tc.name: SetVirtualScreenRefreshRate
 * @tc.desc: SetVirtualScreenRefreshRate test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenRefreshRate, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenFlag, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, GetVirtualScreenFlag, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenFlag screenFlag = VirtualScreenFlag::DEFAULT;
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);

    EXPECT_EQ(DMError::DM_OK, ssm_->SetVirtualScreenFlag(screenId, screenFlag));
    EXPECT_EQ(screenFlag, ssm_->GetVirtualScreenFlag(screenId));
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: ResizeVirtualScreen
 * @tc.desc: ResizeVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ResizeVirtualScreen, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, SetVirtualMirrorScreenScaleMode, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, StopMirror, TestSize.Level1)
{
    std::vector<ScreenId> mirrorScreenIds {0, 1, 2, 3, 4, 5};
    EXPECT_EQ(DMError::DM_OK, ssm_->StopMirror(mirrorScreenIds));
}

/**
 * @tc.name: GetDensityInCurResolution
 * @tc.desc: GetDensityInCurResolution screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDensityInCurResolution, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, SetScreenOffDelayTime, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, GetDeviceScreenConfig, TestSize.Level1)
{
    DeviceScreenConfig deviceScreenConfig = ssm_->GetDeviceScreenConfig();
    EXPECT_FALSE(deviceScreenConfig.rotationPolicy_.empty());
}

/**
 * @tc.name: SetVirtualScreenBlackList
 * @tc.desc: SetVirtualScreenBlackList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenBlackList01, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenBlackList02, TestSize.Level1)
{
    ScreenId screenId = 1010;
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ASSERT_FALSE(ssm_->ConvertScreenIdToRsScreenId(screenId, rsScreenId));
    std::vector<uint64_t> windowId = {10, 20, 30};
    ssm_->SetVirtualScreenBlackList(screenId, windowId);
}

/**
 * @tc.name: SetVirtualDisplayMuteFlag
 * @tc.desc: SetVirtualDisplayMuteFlag test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualDisplayMuteFlag01, Function | SmallTest | Level3)
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
    bool muteFlag = false;
    ssm_->SetVirtualDisplayMuteFlag(screenId, muteFlag);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetVirtualDisplayMuteFlag
 * @tc.desc: ConvertScreenIdToRsScreenId = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualDisplayMuteFlag02, Function | SmallTest | Level3)
{
    ScreenId screenId = 1010;
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ASSERT_FALSE(ssm_->ConvertScreenIdToRsScreenId(screenId, rsScreenId));
    bool muteFlag = false;
    ssm_->SetVirtualDisplayMuteFlag(screenId, muteFlag);
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayPhysicalResolution01, TestSize.Level1)
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
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayPhysicalResolution02, TestSize.Level1)
{
    ssm_->allDisplayPhysicalResolution_.clear();
    ssm_->allDisplayPhysicalResolution_.emplace_back(DisplayPhysicalResolution());
    std::vector<DisplayPhysicalResolution> result = ssm_->GetAllDisplayPhysicalResolution();
    EXPECT_TRUE(!result.empty());
    ssm_->allDisplayPhysicalResolution_.clear();
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayPhysicalResolution03, TestSize.Level1)
{
    DisplayPhysicalResolution resolution;
    resolution.physicalWidth_ = 1920;
    resolution.physicalHeight_ = 1080;
    ssm_->allDisplayPhysicalResolution_.clear();
    ssm_->allDisplayPhysicalResolution_.push_back(resolution);
    std::vector<DisplayPhysicalResolution> result = ssm_->GetAllDisplayPhysicalResolution();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].physicalWidth_, 1920);
    EXPECT_EQ(result[0].physicalHeight_, 1080);
    ssm_->allDisplayPhysicalResolution_.clear();
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetAllDisplayPhysicalResolution04, TestSize.Level1)
{
    ssm_->allDisplayPhysicalResolution_.clear();
    ssm_->allDisplayPhysicalResolution_.emplace_back(DisplayPhysicalResolution());
    ssm_->allDisplayPhysicalResolution_.back().foldDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    std::vector<DisplayPhysicalResolution> resolutions = ssm_->GetAllDisplayPhysicalResolution();
    EXPECT_EQ(resolutions.back().foldDisplayMode_, FoldDisplayMode::FULL);
    ssm_->allDisplayPhysicalResolution_.clear();
}

/**
 * @tc.name: SetDisplayScale
 * @tc.desc: SetDisplayScale test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDisplayScale, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, ScreenCastConnection, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, ReportFoldStatusToScb, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, DisablePowerOffRenderControl01, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, DisablePowerOffRenderControl02, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, CheckAndSendHiSysEvent, TestSize.Level1)
{
    std::string eventName =  "GET_DISPLAY_SNAPSHOT";
    std::string bundleName = "hmos.screenshot";
    ssm_->CheckAndSendHiSysEvent(eventName, bundleName);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: ProxyForFreeze
 * @tc.desc: ProxyForFreeze
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ProxyForFreeze, TestSize.Level1)
{
    int32_t pid = 1;
    std::set<int32_t> unfreezedPidList = {1, 2, 3};
    std::set<DisplayManagerAgentType> pidAgentTypes = {DisplayManagerAgentType::SCREEN_EVENT_LISTENER};
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ssm_->NotifyUnfreezedAgents(pid, unfreezedPidList, pidAgentTypes, screenSession);

    ssm_->NotifyUnfreezed(unfreezedPidList, screenSession);

    std::set<int32_t> pidList = {1, 2, 3};
    DMError ret = ssm_->ProxyForFreeze(pidList, true);
    ASSERT_EQ(ret, DMError::DM_OK);

    ret = ssm_->ProxyForFreeze(pidList, false);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: GetOrCreateFakeScreenSession
 * @tc.desc: GetOrCreateFakeScreenSession
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetOrCreateFakeScreenSession, TestSize.Level1)
{
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    auto ret = ssm_->GetOrCreateFakeScreenSession(screenSession);
    ssm_->InitFakeScreenSession(screenSession);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: SetVirtualScreenSecurityExemption
 * @tc.desc: SetVirtualScreenSecurityExemption
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetVirtualScreenSecurityExemption, TestSize.Level1)
{
    ScreenId screenId = 0;
    uint32_t pid = 1111;
    std::vector<uint64_t> windowList = {11, 22, 33};
    auto ret = ssm_->SetVirtualScreenSecurityExemption(screenId, pid, windowList);
    ASSERT_EQ(ret, DMError::DM_ERROR_UNKNOWN);
}

/**
 * @tc.name: NotifyFoldToExpandCompletion
 * @tc.desc: NotifyFoldToExpandCompletion
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyFoldToExpandCompletion, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea01, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea02, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, UpdateAvailableArea03, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, NotifyAvailableAreaChanged01, TestSize.Level1)
{
    DMRect area = DMRect{};
    DisplayId displayId = 0;
    ASSERT_NE(ssm_, nullptr);
    ssm_->NotifyAvailableAreaChanged(area, displayId);
}

/**
 * @tc.name: TriggerFoldStatusChange01
 * @tc.desc: test interface TriggerFoldStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TriggerFoldStatusChange01, TestSize.Level1)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP();
    }
    ASSERT_NE(ssm_, nullptr);
    FoldStatus foldStatus = FoldStatus::EXPAND;
    ssm_->TriggerFoldStatusChange(foldStatus);
    FoldStatus res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLDED;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::HALF_FOLD;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);

    foldStatus = FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED;
    ssm_->TriggerFoldStatusChange(foldStatus);
    res = ssm_->GetFoldStatus();
    EXPECT_EQ(res, foldStatus);
#endif
}

/**
 * @tc.name: NotifyFoldStatusChanged
 * @tc.desc: NotifyFoldStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyFoldStatusChanged02, TestSize.Level1)
{
#ifdef FOLD_ABILITY_ENABLE
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

    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        ASSERT_EQ(ssm_->foldScreenController_, nullptr);
        GTEST_SKIP();
    }
    if (!(ssm_->IsFoldable())) {
        ssm_->foldScreenController_ = new FoldScreenController(
            ssm_->displayInfoMutex_, ssm_->screenPowerTaskScheduler_);
    }
    ASSERT_NE(ssm_->foldScreenController_, nullptr);
    statusParam = "-y";
    ssm_->HandleFoldScreenPowerInit();
    ret = ssm_->NotifyFoldStatusChanged(statusParam);
    ASSERT_EQ(ret, 0);
#endif
}

/**
 * @tc.name: Dump
 * @tc.desc: Dump
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, Dump, TestSize.Level1)
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
HWTEST_F(ScreenSessionManagerTest, GetDisplayNode01, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    ScreenId screenId = 1051;
    auto ret = ssm_->GetDisplayNode(screenId);
    ASSERT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetDisplayNode
 * @tc.desc: !screenSession = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayNode02, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    auto ret = ssm_->GetDisplayNode(screenId);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: GetDisplayNode
 * @tc.desc: SystemCalling false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayNode03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    ScreenId screenId = 1050;
    ssm_->GetDisplayNode(screenId);
    EXPECT_TRUE(g_logMsg.find("Permission Denied") != std::string::npos);
}

/**
 * @tc.name: GetScreenProperty
 * @tc.desc: GetScreenProperty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenProperty01, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(true);
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
HWTEST_F(ScreenSessionManagerTest, GetScreenProperty02, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    ScreenId screenId = 1050;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    ScreenProperty property = ssm_->GetScreenProperty(screenId);
    ASSERT_EQ(sizeof(property), sizeof(screenSession->property_));
}

/**
 * @tc.name: GetScreenProperty
 * @tc.desc: SystemCalling false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenProperty03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    ScreenId screenId = 1050;
    ssm_->GetScreenProperty(screenId);
    EXPECT_TRUE(g_logMsg.find("Permission Denied") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetCurrentScreenPhyBounds
 * @tc.desc: GetCurrentScreenPhyBounds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentScreenPhyBounds01, TestSize.Level1)
{
#ifdef FOLD_ABILITY_ENABLE
    float phyWidth = 0.0f;
    float phyHeight = 0.0f;
    bool isReset = true;
    ScreenId screenId = 1050;
    ssm_->GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        ASSERT_EQ(isReset, true);
        GTEST_SKIP();
    } else {
        ASSERT_NE(isReset, true);
    }
    if (!(ssm_->IsFoldable())) {
        ssm_->foldScreenController_ = new FoldScreenController(
            ssm_->displayInfoMutex_, ssm_->screenPowerTaskScheduler_);
    }

    ASSERT_NE(ssm_->foldScreenController_, nullptr);
    ssm_->GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
    ASSERT_FALSE(isReset);
#endif
}

/**
 * @tc.name: GetCurrentScreenPhyBounds
 * @tc.desc: GetCurrentScreenPhyBounds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentScreenPhyBounds02, TestSize.Level1)
{
#ifdef FOLD_ABILITY_ENABLE
    float phyWidth = 0.0f;
    float phyHeight = 0.0f;
    bool isReset = true;
    ScreenId screenId = 0;
    ssm_->GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
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
    foldPolicy->lastDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    foldController->foldScreenPolicy_ = foldPolicy;
    ssm_->foldScreenController_ = foldController;
    ssm_->GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
    auto phyBounds = ssm_->GetPhyScreenProperty(0).GetPhyBounds();
    float phyWidthNow = phyBounds.rect_.width_;
    float phyHeightNow = phyBounds.rect_.width_;
    int32_t screenRotationOffSet = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        EXPECT_EQ(phyWidth, phyWidthNow);
        EXPECT_EQ(phyHeight, phyHeightNow);
    } else if (screenRotationOffSet == 1 || screenRotationOffSet == 3) {
        EXPECT_EQ(phyHeight, phyHeightNow);
        EXPECT_EQ(phyWidth, phyWidthNow);
    } else {
        EXPECT_EQ(phyWidth, phyWidthNow);
        EXPECT_EQ(phyHeight, phyHeightNow);
    }
#endif
}

/**
 * @tc.name: PhyMirrorConnectWakeupScreen
 * @tc.desc: PhyMirrorConnectWakeupScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, PhyMirrorConnectWakeupScreen, TestSize.Level1)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ASSERT_NE(ssm_, nullptr);
    ssm_->PhyMirrorConnectWakeupScreen();
    ScreenSceneConfig::stringConfig_["externalScreenDefaultMode"] = "mirror";
    ssm_->PhyMirrorConnectWakeupScreen();
#endif
}

/**
 * @tc.name: GetIsCurrentInUseById01
 * @tc.desc: GetIsCurrentInUseById01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetIsCurrentInUseById01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    
    ScreenId screenId = 50;
    auto res = ssm->GetIsCurrentInUseById(screenId);
    ASSERT_EQ(false, res);
}
}
} // namespace Rosen
} // namespace OHOS