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
 * @tc.name: GetIsCurrentInUseById02
 * @tc.desc: GetIsCurrentInUseById02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetIsCurrentInUseById02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession();
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetIsCurrentInUse(false);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto res = ssm->GetIsCurrentInUseById(screenId);
    ASSERT_EQ(false, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetIsCurrentInUseById03
 * @tc.desc: GetIsCurrentInUseById03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetIsCurrentInUseById03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession();
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetIsCurrentInUse(true);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto res = ssm->GetIsCurrentInUseById(screenId);
    ASSERT_EQ(true, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: ReportHandleScreenEvent01
 * @tc.desc: ReportHandleScreenEvent01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ReportHandleScreenEvent01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenCombination screenCombination = ScreenCombination::SCREEN_MIRROR;
    ssm->ReportHandleScreenEvent(screenEvent, screenCombination);
}

/**
 * @tc.name: ReportHandleScreenEvent02
 * @tc.desc: ReportHandleScreenEvent02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ReportHandleScreenEvent02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenCombination screenCombination = ScreenCombination::SCREEN_EXTEND;
    ssm->ReportHandleScreenEvent(screenEvent, screenCombination);
}

/**
 * @tc.name: RegisterSettingWireCastObserver01
 * @tc.desc: RegisterSettingWireCastObserver01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RegisterSettingWireCastObserver01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->RegisterSettingWireCastObserver(screenSession);
}

/**
 * @tc.name: UnregisterSettingWireCastObserver01
 * @tc.desc: UnregisterSettingWireCastObserver01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UnregisterSettingWireCastObserver01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    ASSERT_EQ(screenSession, nullptr);
    ssm->UnregisterSettingWireCastObserver(screenId);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UnregisterSettingWireCastObserver02
 * @tc.desc: UnregisterSettingWireCastObserver02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UnregisterSettingWireCastObserver02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    ssm->UnregisterSettingWireCastObserver(60);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UnregisterSettingWireCastObserver03
 * @tc.desc: UnregisterSettingWireCastObserver03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UnregisterSettingWireCastObserver03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    ssm->UnregisterSettingWireCastObserver(50);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsPhysicalScreenAndInUse01
 * @tc.desc: IsPhysicalScreenAndInUse01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsPhysicalScreenAndInUse01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsCurrentInUse(true);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto res = ssm->IsPhysicalScreenAndInUse(screenSession);
    ASSERT_EQ(true, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsPhysicalScreenAndInUse02
 * @tc.desc: IsPhysicalScreenAndInUse02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsPhysicalScreenAndInUse02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetIsCurrentInUse(true);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto res = ssm->IsPhysicalScreenAndInUse(screenSession);
    ASSERT_EQ(false, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsPhysicalScreenAndInUse03
 * @tc.desc: IsPhysicalScreenAndInUse03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsPhysicalScreenAndInUse03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsCurrentInUse(false);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto res = ssm->IsPhysicalScreenAndInUse(screenSession);
    ASSERT_EQ(false, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsPhysicalScreenAndInUse04
 * @tc.desc: IsPhysicalScreenAndInUse04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsPhysicalScreenAndInUse04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId screenId = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetIsCurrentInUse(false);
    ssm->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto res = ssm->IsPhysicalScreenAndInUse(screenSession);
    ASSERT_EQ(false, res);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: IsPhysicalScreenAndInUse05
 * @tc.desc: IsPhysicalScreenAndInUse05
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsPhysicalScreenAndInUse05, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    auto res = ssm->IsPhysicalScreenAndInUse(screenSession);
    ASSERT_EQ(false, res);
}

/**
 * @tc.name: HookDisplayInfoByUid01
 * @tc.desc: HookDisplayInfo by uid01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HookDisplayInfoByUid01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<DisplayInfo> displayInfo = nullptr;
    ASSERT_EQ(displayInfo, nullptr);
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    auto res = ssm->HookDisplayInfoByUid(displayInfo, screenSession);
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: GetDisplayInfoById01
 * @tc.desc: GetDisplayInfoById01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoById01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    DisplayId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetDisplayInfoById(id);
    ASSERT_EQ(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetVisibleAreaDisplayInfoById01
 * @tc.desc: GetVisibleAreaDisplayInfoById01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetVisibleAreaDisplayInfoById01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    DisplayId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetVisibleAreaDisplayInfoById(id);
    ASSERT_EQ(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetVisibleAreaDisplayInfoById02
 * @tc.desc: GetVisibleAreaDisplayInfoById02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetVisibleAreaDisplayInfoById02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    DisplayId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetVisibleAreaDisplayInfoById(id);
    ASSERT_NE(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetDisplayInfoByScreen01
 * @tc.desc: GetDisplayInfoByScreen01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoByScreen01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetDisplayInfoByScreen(id);
    ASSERT_EQ(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetDisplayInfoByScreen02
 * @tc.desc: GetDisplayInfoByScreen02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplayInfoByScreen02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetDisplayInfoByScreen(id);
    ASSERT_NE(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetFakeDisplayId01
 * @tc.desc: GetFakeDisplayId01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetFakeDisplayId01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    sptr<ScreenSession> fakeScreenSession = nullptr;
    ASSERT_EQ(fakeScreenSession, nullptr);
    screenSession->SetFakeScreenSession(fakeScreenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetFakeDisplayId(screenSession);
    ASSERT_EQ(res, DISPLAY_ID_INVALID);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetFakeDisplayId02
 * @tc.desc: GetFakeDisplayId02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetFakeDisplayId02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    sptr<ScreenSession> fakeScreenSession = new ScreenSession(999, ScreenProperty(), 0);
    ASSERT_NE(nullptr, fakeScreenSession);
    screenSession->SetFakeScreenSession(fakeScreenSession);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetFakeDisplayId(screenSession);
    ASSERT_NE(res, DISPLAY_ID_INVALID);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalculateXYPosition01
 * @tc.desc: CalculateXYPosition01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateXYPosition01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalculateXYPosition(screenSession);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalculateXYPosition02
 * @tc.desc: CalculateXYPosition02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateXYPosition02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(true);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalculateXYPosition(screenSession);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalculateXYPosition03
 * @tc.desc: CalculateXYPosition03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalculateXYPosition03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsInternal(false);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ScreenId internalId = 51;
    sptr<ScreenSession> internalSession = new ScreenSession(internalId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, internalSession);
    internalSession->SetScreenType(ScreenType::REAL);
    internalSession->SetIsInternal(true);
    ssm->screenSessionMap_.insert(std::make_pair(internalId, internalSession));
    ssm->CalculateXYPosition(screenSession);
    ssm->screenSessionMap_.erase(50);
    ssm->screenSessionMap_.erase(51);
}

/**
 * @tc.name: NotifyAndPublishEvent01
 * @tc.desc: NotifyAndPublishEvent01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyAndPublishEvent01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    ASSERT_NE(nullptr, displayInfo);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ssm->NotifyAndPublishEvent(displayInfo, id, screenSession);
}

/**
 * @tc.name: NotifyAndPublishEvent02
 * @tc.desc: NotifyAndPublishEvent02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyAndPublishEvent02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    ASSERT_NE(nullptr, displayInfo);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->NotifyAndPublishEvent(displayInfo, id, screenSession);
}

/**
 * @tc.name: NotifyAndPublishEvent03
 * @tc.desc: NotifyAndPublishEvent03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyAndPublishEvent03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<DisplayInfo> displayInfo = nullptr;
    ASSERT_EQ(nullptr, displayInfo);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    ssm->NotifyAndPublishEvent(displayInfo, id, screenSession);
}

/**
 * @tc.name: UpdateScreenDirectionInfo01
 * @tc.desc: UpdateScreenDirectionInfo01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateScreenDirectionInfo01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ScreenDirectionInfo directionInfo;
    directionInfo.screenRotation_ = 0.0f;
    directionInfo.rotation_ = 0.0f;
    directionInfo.phyRotation_ = 0.0f;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_END;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->UpdateScreenDirectionInfo(id, directionInfo, screenPropertyChangeType, bounds);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UpdateScreenDirectionInfo02
 * @tc.desc: UpdateScreenDirectionInfo02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateScreenDirectionInfo02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ScreenDirectionInfo directionInfo;
    directionInfo.screenRotation_ = 0.0f;
    directionInfo.rotation_ = 0.0f;
    directionInfo.phyRotation_ = 0.0f;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::UNSPECIFIED;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->UpdateScreenDirectionInfo(id, directionInfo, screenPropertyChangeType, bounds);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UpdateScreenDirectionInfo03
 * @tc.desc: UpdateScreenDirectionInfo03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateScreenDirectionInfo03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    ScreenDirectionInfo directionInfo;
    directionInfo.screenRotation_ = 0.0f;
    directionInfo.rotation_ = 0.0f;
    directionInfo.phyRotation_ = 0.0f;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::UNSPECIFIED;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->UpdateScreenDirectionInfo(id, directionInfo, screenPropertyChangeType, bounds);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetRSDisplayNodeByScreenId01
 * @tc.desc: GetRSDisplayNodeByScreenId01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetRSDisplayNodeByScreenId01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetRSDisplayNodeByScreenId(id);
    ASSERT_EQ(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: GetRSDisplayNodeByScreenId02
 * @tc.desc: GetRSDisplayNodeByScreenId02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetRSDisplayNodeByScreenId02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->ReleaseDisplayNode();
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    auto res = ssm->GetRSDisplayNodeByScreenId(id);
    ASSERT_EQ(res, nullptr);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: OnScreenConnect
 * @tc.desc: OnScreenConnect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, OnScreenConnect, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    sptr<ScreenInfo> screenInfo = nullptr;
    ASSERT_EQ(nullptr, screenInfo);
    ssm->OnScreenConnect(screenInfo);
}

/**
 * @tc.name: TransferPropertyChangeTypeToString05
 * @tc.desc: TransferPropertyChangeTypeToString05
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferPropertyChangeTypeToString05, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferPropertyChangeTypeToString(
        ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY);
    std::string expectType = "ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: TransferPropertyChangeTypeToString06
 * @tc.desc: TransferPropertyChangeTypeToString06
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TransferPropertyChangeTypeToString06, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    std::string screenType = ssm->TransferPropertyChangeTypeToString(
        ScreenPropertyChangeType::UNDEFINED);
    std::string expectType = "UNDEFINED";
    ASSERT_EQ(screenType, expectType);
}

/**
 * @tc.name: UpdateDisplayScaleState01
 * @tc.desc: UpdateDisplayScaleState01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateDisplayScaleState01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(screenSession, nullptr);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->UpdateDisplayScaleState(id);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: UpdateDisplayScaleState02
 * @tc.desc: UpdateDisplayScaleState02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, UpdateDisplayScaleState02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    float scaleX = 3.0f;
    float scaleY = 3.0f;
    float pivotX = 3.0f;
    float pivotY = 3.0f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    screenSession->SetScreenScale(scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->UpdateDisplayScaleState(id);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: SetDisplayScaleInner
 * @tc.desc: SetDisplayScaleInner
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetDisplayScaleInner, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = -0.5f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->SetDisplayScaleInner(id, scaleX, scaleY, pivotX, pivotY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnFoldableRotation01
 * @tc.desc: CalcDisplayNodeTranslateOnFoldableRotation01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnFoldableRotation01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = -0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnFoldableRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnFoldableRotation02
 * @tc.desc: CalcDisplayNodeTranslateOnFoldableRotation02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnFoldableRotation02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_0);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnFoldableRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnFoldableRotation03
 * @tc.desc: CalcDisplayNodeTranslateOnFoldableRotation03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnFoldableRotation03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_90);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnFoldableRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnFoldableRotation04
 * @tc.desc: CalcDisplayNodeTranslateOnFoldableRotation04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnFoldableRotation04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_180);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnFoldableRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnFoldableRotation05
 * @tc.desc: CalcDisplayNodeTranslateOnFoldableRotation05
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnFoldableRotation05, Function | SmallTest | Level3)
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
    ssm->CalcDisplayNodeTranslateOnFoldableRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnPocketFoldRotation01
 * @tc.desc: CalcDisplayNodeTranslateOnPocketFoldRotation01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnPocketFoldRotation01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = -0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnPocketFoldRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnPocketFoldRotation02
 * @tc.desc: CalcDisplayNodeTranslateOnPocketFoldRotation02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnPocketFoldRotation02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_0);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnPocketFoldRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnPocketFoldRotation03
 * @tc.desc: CalcDisplayNodeTranslateOnPocketFoldRotation03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnPocketFoldRotation03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_90);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnPocketFoldRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnPocketFoldRotation04
 * @tc.desc: CalcDisplayNodeTranslateOnPocketFoldRotation04
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnPocketFoldRotation04, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_180);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnPocketFoldRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnPocketFoldRotation05
 * @tc.desc: CalcDisplayNodeTranslateOnPocketFoldRotation05
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnPocketFoldRotation05, Function | SmallTest | Level3)
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
    ssm->CalcDisplayNodeTranslateOnPocketFoldRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnRotation01
 * @tc.desc: CalcDisplayNodeTranslateOnRotation01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnRotation01, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = nullptr;
    ASSERT_EQ(nullptr, screenSession);
    float scaleX = 0.5f;
    float scaleY = 3.0f;
    float pivotX = 0.5f;
    float pivotY = -0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    ssm->screenSessionMap_.insert(std::make_pair(id, screenSession));
    ssm->CalcDisplayNodeTranslateOnRotation(screenSession, scaleX, scaleY, pivotX, pivotY, translateX,
        translateY);
    ssm->screenSessionMap_.erase(50);
}

/**
 * @tc.name: CalcDisplayNodeTranslateOnRotation02
 * @tc.desc: CalcDisplayNodeTranslateOnRotation02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnRotation02, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_90);
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
 * @tc.name: CalcDisplayNodeTranslateOnRotation03
 * @tc.desc: CalcDisplayNodeTranslateOnRotation03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CalcDisplayNodeTranslateOnRotation03, Function | SmallTest | Level3)
{
    ScreenSessionManager* ssm = new ScreenSessionManager();
    ASSERT_NE(ssm, nullptr);
    ScreenId id = 50;
    sptr<ScreenSession> screenSession = new ScreenSession(id, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetRotation(Rotation::ROTATION_180);
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
}
} // namespace Rosen
} // namespace OHOS