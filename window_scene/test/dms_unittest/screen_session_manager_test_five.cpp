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
 * @tc.name: CalcDisplayNodeTranslateOnRotation04
 * @tc.desc: CalcDisplayNodeTranslateOnRotation04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnRotation04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
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
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: OnScreenChange
 * @tc.desc: OnScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenChange, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenChangeReason reason = ScreenChangeReason::HWCDEAD;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->OnScreenChange(id, screenEvent, reason);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber01
 * @tc.desc: GetCurrentInUseScreenNumber01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetCurrentInUseScreenNumber();
    ASSERT_EQ(0, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber02
 * @tc.desc: GetCurrentInUseScreenNumber02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsCurrentInUse(false);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetCurrentInUseScreenNumber();
    ASSERT_EQ(0, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber03
 * @tc.desc: GetCurrentInUseScreenNumber03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsCurrentInUse(true);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetCurrentInUseScreenNumber();
    ASSERT_EQ(1, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetCurrentInUseScreenNumber04
 * @tc.desc: GetCurrentInUseScreenNumber04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetCurrentInUseScreenNumber04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::UNDEFINED);
    screenSession->SetIsCurrentInUse(true);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetCurrentInUseScreenNumber();
    ASSERT_EQ(0, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetCastPrivacyFromSettingData01
 * @tc.desc: SetCastPrivacyFromSettingData01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetCastPrivacyFromSettingData01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->SetCastPrivacyFromSettingData();
    ssm->GetAllDisplayIds();
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend01
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend02
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::UNDEFINED);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend03
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(true);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CallRsSetScreenPowerStatusSyncForExtend04
 * @tc.desc: CallRsSetScreenPowerStatusSyncForExtend04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CallRsSetScreenPowerStatusSyncForExtend04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(false);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIds = {50};
    ScreenPowerStatus status = ScreenPowerStatus::POWER_STATUS_ON;
    ssm->CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio01
 * @tc.desc: SetExtendPixelRatio01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    float dpi = 1.0f;
    ssm->SetExtendPixelRatio(dpi);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->SetExtendPixelRatio(dpi);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio02
 * @tc.desc: SetExtendPixelRatio02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    float dpi = 1.0f;
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(false);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->SetExtendPixelRatio(dpi);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio03
 * @tc.desc: SetExtendPixelRatio03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    float dpi = 1.0f;
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(true);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->SetExtendPixelRatio(dpi);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetExtendPixelRatio04
 * @tc.desc: SetExtendPixelRatio04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendPixelRatio04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    float dpi = 1.0f;
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::UNDEFINED);
    screenSession->SetIsInternal(true);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->SetExtendPixelRatio(dpi);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetRotation01
 * @tc.desc: SetRotation01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRotation01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    Rotation rotationAfter = Rotation::ROTATION_0;
    bool isFromWindow = true;
    auto res = ssm->SetRotation(id, rotationAfter, isFromWindow);
    ASSERT_EQ(false, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetRotation02
 * @tc.desc: SetRotation02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRotation02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_90);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    Rotation rotationAfter = Rotation::ROTATION_0;
    bool isFromWindow = true;
    auto res = ssm->SetRotation(id, rotationAfter, isFromWindow);
    ASSERT_EQ(true, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: OnMakeExpand01
 * @tc.desc: OnMakeExpand01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnMakeExpand01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenId = {1001, 1002};
    std::vector<Point> startPoint(1);
    auto res = ssm->OnMakeExpand(screenId, startPoint);
    ASSERT_EQ(false, res);
    ssm->screenSessionMap_.erase(0);
}

/**
 * @tc.name: OnMakeExpand02
 * @tc.desc: OnMakeExpand02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnMakeExpand02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->groupSmsId_ = 50;
    ScreenCombination combination =  ScreenCombination::SCREEN_ALONE;
    ScreenId rsId = 1002;
    std::string name = "ok";
    sptr<ScreenSessionGroup> groupSession = new ScreenSessionGroup(50, rsId, name, combination);
    ASSERT_NE(nullptr, groupSession);
    ssm->smsScreenGroupMap_.insert(std::make_pair(50, groupSession));
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenId = {1001, 1002};
    std::vector<Point> startPoint(1);
    auto res = ssm->OnMakeExpand(screenId, startPoint);
    ASSERT_EQ(true, res);
    ssm->screenSessionMap_.erase(0);
    ssm->smsScreenGroupMap_.erase(50);
}

/**
 * @tc.name: IsExtendMode01
 * @tc.desc: IsExtendMode01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsExtendMode01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->IsExtendMode();
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsExtendMode02
 * @tc.desc: IsExtendMode02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsExtendMode02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->IsExtendMode();
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    ssm->IsExtendMode();
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    ssm->IsExtendMode();
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UpdateCameraBackSelfie01
 * @tc.desc: UpdateCameraBackSelfie01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateCameraBackSelfie01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    bool isCameraBackSelfie = true;
    ssm->UpdateCameraBackSelfie(isCameraBackSelfie);
    ssm->UpdateCameraBackSelfie(true);
    ssm->screenSessionMap_.erase(0);
}

/**
 * @tc.name: UpdateCameraBackSelfie02
 * @tc.desc: UpdateCameraBackSelfie02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateCameraBackSelfie02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    bool isCameraBackSelfie = true;
    ssm->UpdateCameraBackSelfie(isCameraBackSelfie);
    ssm->UpdateCameraBackSelfie(false);
    ssm->screenSessionMap_.erase(0);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror01
 * @tc.desc: HasCastEngineOrPhyMirror01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    ssm->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror02
 * @tc.desc: HasCastEngineOrPhyMirror02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetName("CastEngine");
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    auto res = ssm->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ASSERT_EQ(true, res);
    std::vector<ScreenId> idsToExclude = {100, 50};
    ssm->HasCastEngineOrPhyMirror(idsToExclude);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror03
 * @tc.desc: HasCastEngineOrPhyMirror03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetName("TestCastEngine");
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    auto res = ssm->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ASSERT_EQ(false, res);
    screenSession->SetScreenType(ScreenType::REAL);
    ssm->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: HasCastEngineOrPhyMirror04
 * @tc.desc: HasCastEngineOrPhyMirror04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HasCastEngineOrPhyMirror04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 9;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetName("CastEngine");
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    std::vector<ScreenId> screenIdsToExclude = {100};
    ssm->HasCastEngineOrPhyMirror(screenIdsToExclude);
    ssm->screenSessionMap_.erase(9);
}

/**
 * @tc.name: HotSwitch
 * @tc.desc: HotSwitch
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HotSwitch, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    int32_t newUserId = 0;
    int32_t newScbPid = 1;
    ssm->HotSwitch(newUserId, newScbPid);
    sptr<IScreenSessionManagerClient> client = nullptr;
    ssm->clientProxyMap_.insert(std::make_pair(newUserId, client));
    ssm->HotSwitch(newUserId, newScbPid);
    ssm->clientProxyMap_.insert(std::make_pair(1, client));
    ssm->HotSwitch(1, newScbPid);
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
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<IScreenSessionManagerClient> client = nullptr;
    ssm->SetClient(client);
}

/**
 * @tc.name: SwitchUser
 * @tc.desc: SwitchUser
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SwitchUser, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ssm->SwitchUser();
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
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    int32_t deathScbPid = ssm->currentScbPId_;
    ssm->currentScbPId_ = -1;     // INVALID_SCB_PID
    ssm->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm->clientProxy_, nullptr);

    deathScbPid = ssm->currentScbPId_;
    ssm->currentScbPId_ = 0;
    ssm->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm->clientProxy_, nullptr);

    deathScbPid = 0;
    ssm->currentScbPId_ = -1;     // INVALID_SCB_PID
    ssm->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm->clientProxy_, nullptr);

    deathScbPid = 0;
    ssm->currentScbPId_ = 0;
    ssm->ScbClientDeathCallback(deathScbPid);
    ASSERT_EQ(ssm->clientProxy_, nullptr);
}

/**
 * @tc.name: NotifyClientProxyUpdateFoldDisplayMode?
 * @tc.desc: NotifyClientProxyUpdateFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyClientProxyUpdateFoldDisplayMode, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    FoldDisplayMode displayMode = FoldDisplayMode::FULL;
    ASSERT_EQ(ssm->clientProxy_, nullptr);
    ssm->NotifyClientProxyUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: OnScreenRotationLockedChange
 * @tc.desc: OnScreenRotationLockedChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenRotationLockedChange, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    bool isLocked = false;
    ScreenId screenId = 1050;
    ASSERT_EQ(ssm->clientProxy_, nullptr);
    ssm->OnScreenRotationLockedChange(isLocked, screenId);
}

/**
 * @tc.name: OnScreenOrientationChange
 * @tc.desc: OnScreenOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenOrientationChange, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    float screenOrientation = 75.2f;
    ScreenId screenId = 1050;
    ASSERT_EQ(ssm->clientProxy_, nullptr);
    ssm->OnScreenOrientationChange(screenOrientation, screenId);
}

/**
 * @tc.name: NotifyDisplayModeChanged
 * @tc.desc: NotifyDisplayModeChanged
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyDisplayModeChanged, TestSize.Level1)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ssm->NotifyDisplayModeChanged(FoldDisplayMode::MAIN);
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
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ssm->screenSessionMap_.erase(50);
    EXPECT_NE(ssm, nullptr);
    DisplayId id = 50;
    bool isOnboardDisplay = false;
    auto res = ssm->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, false);
    EXPECT_EQ(res, DMError::DM_ERROR_INVALID_PARAM);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsOnboardDisplay02
 * @tc.desc: IsOnboardDisplay, Check pc has board.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay02, TestSize.Level0)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ssm->screenSessionMap_.erase(0);
    EXPECT_NE(ssm, nullptr);
    bool isPcNow = ssm->GetPcStatus();
    ssm->SetPcStatus(true);
    DisplayId id = 0;
    bool isOnboardDisplay = false;
    ssm->screenIdManager_.sms2RsScreenIdMap_[id] = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, true);
    EXPECT_EQ(res, DMError::DM_OK);
    ssm->screenIdManager_.sms2RsScreenIdMap_.clear();
    ssm->screenSessionMap_.erase(0);
    ssm->SetPcStatus(isPcNow);
}

/**
 * @tc.name: IsOnboardDisplay03
 * @tc.desc: IsOnboardDisplay, Check pc has no board.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay03, TestSize.Level0)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ssm->screenSessionMap_.erase(100);
    EXPECT_NE(ssm, nullptr);
    bool isPcNow = ssm->GetPcStatus();
    ssm->SetPcStatus(true);
    DisplayId id = 100;
    bool isOnboardDisplay = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, false);
    EXPECT_EQ(res, DMError::DM_ERROR_INVALID_PARAM);
    ssm->screenSessionMap_.erase(100);
    ssm->SetPcStatus(isPcNow);
}

/**
 * @tc.name: IsOnboardDisplay04
 * @tc.desc: IsOnboardDisplay, Check phone/pad has board.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay04, TestSize.Level0)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ssm->screenSessionMap_.erase(0);
    EXPECT_NE(ssm, nullptr);
    bool isPcNow = ssm->GetPcStatus();
    ssm->SetPcStatus(false);
    DisplayId id = 0;
    bool isOnboardDisplay = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, true);
    EXPECT_EQ(res, DMError::DM_OK);
    ssm->screenSessionMap_.erase(0);
    ssm->SetPcStatus(isPcNow);
}

/**
 * @tc.name: IsOnboardDisplay05
 * @tc.desc: IsOnboardDisplay, Check phone/pad has noboard.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsOnboardDisplay05, TestSize.Level0)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ssm->screenSessionMap_.erase(100);
    EXPECT_NE(ssm, nullptr);
    bool isPcNow = ssm->GetPcStatus();
    ssm->SetPcStatus(false);
    DisplayId id = 100;
    bool isOnboardDisplay = false;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->IsOnboardDisplay(id, isOnboardDisplay);
    EXPECT_EQ(isOnboardDisplay, false);
    EXPECT_EQ(res, DMError::DM_OK);
    ssm->screenSessionMap_.erase(100);
    ssm->SetPcStatus(isPcNow);
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
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ASSERT_EQ(ssm->isCoordinationFlag_, false);
    ssm->SetCoordinationFlag(true);
    ASSERT_EQ(ssm->isCoordinationFlag_, true);
}
}
} // namespace Rosen
} // namespace OHOS