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

#include "window_helper.h"
#include "display_manager.h"
#include "pointer_event.h"

#include <gtest/gtest.h>
#include "key_event.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "session/host/include/main_session.h"
#include "wm_common.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_accesstoken_kit.h"
#include "input_event.h"
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
std::string logMsg;
void SceneSessionLogCallback(const LogType type,
                             const LogLevel level,
                             const unsigned int domain,
                             const char* tag,
                             const char* msg)
{
    logMsg = msg;
}
} // namespace
constexpr int WAIT_ASYNC_US = 1000000;
class SceneSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionTest::SetUpTestCase() {}

void SceneSessionTest::TearDownTestCase() {}

void SceneSessionTest::SetUp() {}

void SceneSessionTest::TearDown() {}

namespace {

/**
 * @tc.name: SetGlobalMaximizeMode01
 * @tc.desc: SetGlobalMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetGlobalMaximizeMode01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetGlobalMaximizeMode01";
    info.bundleName_ = "SetGlobalMaximizeMode01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;
    auto result = sceneSession->SetGlobalMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: GetGlobalMaximizeMode01
 * @tc.desc: GetGlobalMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetGlobalMaximizeMode01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetGlobalMaximizeMode01";
    info.bundleName_ = "GetGlobalMaximizeMode01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;
    MaximizeMode mode;
    auto result = sceneSession->GetGlobalMaximizeMode(mode);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: GlobalMaximizeMode01
 * @tc.desc: GlobalMaximizeMode MODE_AVOID_SYSTEM_BAR
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GlobalMaximizeMode01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GlobalMaximizeMode01";
    info.bundleName_ = "GlobalMaximizeMode01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;
    auto result = sceneSession->SetGlobalMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ASSERT_EQ(result, WSError::WS_OK);

    MaximizeMode mode;
    result = sceneSession->GetGlobalMaximizeMode(mode);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(mode, MaximizeMode::MODE_AVOID_SYSTEM_BAR);
}

/**
 * @tc.name: GlobalMaximizeMode02
 * @tc.desc: GlobalMaximizeMode MODE_FULL_FILL
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GlobalMaximizeMode02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GlobalMaximizeMode02";
    info.bundleName_ = "GlobalMaximizeMode02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;
    auto result = sceneSession->SetGlobalMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    ASSERT_EQ(result, WSError::WS_OK);
    MaximizeMode mode;
    result = sceneSession->GetGlobalMaximizeMode(mode);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(mode, MaximizeMode::MODE_FULL_FILL);
}

/**
 * @tc.name: SetAndGetPipTemplateInfo
 * @tc.desc: SetAndGetPipTemplateInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetAndGetPipTemplateInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "Background01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;
    PiPTemplateInfo pipTemplateInfo;
    pipTemplateInfo.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL);
    sceneSession->SetPiPTemplateInfo(pipTemplateInfo);
    ASSERT_EQ(sceneSession->GetPiPTemplateInfo().pipTemplateType, static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL));

    pipTemplateInfo.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_MEETING);
    sceneSession->SetPiPTemplateInfo(pipTemplateInfo);
    ASSERT_EQ(sceneSession->GetPiPTemplateInfo().pipTemplateType,
              static_cast<uint32_t>(PiPTemplateType::VIDEO_MEETING));
}

/**
 * @tc.name: UpdateWindowSceneAfterCustomAnimation01
 * @tc.desc: UpdateWindowSceneAfterCustomAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateWindowSceneAfterCustomAnimation01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "Background01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;
    auto result = sceneSession->UpdateWindowSceneAfterCustomAnimation(false);
    ASSERT_EQ(result, WSError::WS_OK);
    result = sceneSession->UpdateWindowSceneAfterCustomAnimation(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetZOrder01
 * @tc.desc: SetZOrder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetZOrder01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetZOrder01";
    info.bundleName_ = "SetZOrder01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    uint32_t zOrder = 2;
    sceneSession->SetZOrder(zOrder);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(zOrder, sceneSession->zOrder_);
}

/**
 * @tc.name: GetTouchHotAreas01
 * @tc.desc: GetTouchHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetTouchHotAreas01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "GetTouchHotAreas01";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    Rect windowRect = { 1, 1, 1, 1 };
    std::vector<Rect> rects;
    uint32_t hotAreasNum = 10;
    uint32_t hotAreaWidth = windowRect.width_ / hotAreasNum;
    uint32_t hotAreaHeight = windowRect.height_ / hotAreasNum;
    for (uint32_t i = 0; i < hotAreasNum; ++i) {
        rects.emplace_back(Rect{ hotAreaWidth * i, hotAreaHeight * i, hotAreaWidth, hotAreaHeight });
    }
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    ASSERT_NE(nullptr, property);
    property->SetTouchHotAreas(rects);
    ASSERT_NE(rects, sceneSession->GetTouchHotAreas());
}

/**
 * @tc.name: SetTurnScreenOn01
 * @tc.desc: SetTurnScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetTurnScreenOn01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetTurnScreenOn01";
    info.bundleName_ = "SetTurnScreenOn01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetTurnScreenOn(true));
    ASSERT_EQ(true, sceneSession->IsTurnScreenOn());
}

/**
 * @tc.name: SetTurnScreenOn02
 * @tc.desc: SetTurnScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetTurnScreenOn02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetTurnScreenOn02";
    info.bundleName_ = "SetTurnScreenOn02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetTurnScreenOn(false));
    ASSERT_EQ(false, sceneSession->IsTurnScreenOn());
}

/**
 * @tc.name: UpdateWindowAnimationFlag01
 * @tc.desc: UpdateWindowAnimationFlag true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateWindowAnimationFlag01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWindowAnimationFlag01";
    info.bundleName_ = "UpdateWindowAnimationFlag01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateWindowAnimationFlag(true));
}

/**
 * @tc.name: UpdateWindowAnimationFlag02
 * @tc.desc: UpdateWindowAnimationFlag false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateWindowAnimationFlag02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWindowAnimationFlag02";
    info.bundleName_ = "UpdateWindowAnimationFlag02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateWindowAnimationFlag(false));
}

/**
 * @tc.name: SetRequestedOrientation
 * @tc.desc: SetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetRequestedOrientation01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetRequestedOrientation01";
    info.bundleName_ = "SetRequestedOrientation01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    Orientation ori = Orientation::UNSPECIFIED;
    sceneSession->SetRequestedOrientation(ori);
    Orientation ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ori, ret);

    sceneSession->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
}

/**
 * @tc.name: GetRequestedOrientation
 * @tc.desc: GetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetRequestedOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetRequestedOrientation";
    info.bundleName_ = "GetRequestedOrientation";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    Orientation ori = Orientation::HORIZONTAL;
    sceneSession->SetRequestedOrientation(ori);
    Orientation ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ori, ret);

    sceneSession->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
}

/**
 * @tc.name: SetKeepScreenOn01
 * @tc.desc: SetKeepScreenOn true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetKeepScreenOn01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetKeepScreenOn01";
    info.bundleName_ = "SetKeepScreenOn01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    EXPECT_EQ(sceneSession->SetKeepScreenOn(true), WSError::WS_OK);
    EXPECT_EQ(sceneSession->IsKeepScreenOn(), true);
    EXPECT_EQ(sceneSession->SetKeepScreenOn(false), WSError::WS_OK);
    EXPECT_EQ(sceneSession->IsKeepScreenOn(), false);
}

/**
 * @tc.name: IsKeepScreenOn01
 * @tc.desc: IsKeepScreenOn true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsKeepScreenOn01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsKeepScreenOn01";
    info.bundleName_ = "IsKeepScreenOn01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetKeepScreenOn(true));
    ASSERT_EQ(true, sceneSession->IsKeepScreenOn());
}

/**
 * @tc.name: IsKeepScreenOn02
 * @tc.desc: IsKeepScreenOn false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsKeepScreenOn02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsKeepScreenOn02";
    info.bundleName_ = "IsKeepScreenOn02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetKeepScreenOn(false));
    ASSERT_EQ(false, sceneSession->IsKeepScreenOn());
}

/**
 * @tc.name: SetViewKeepScreenOn01
 * @tc.desc: SetViewKeepScreenOn true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetViewKeepScreenOn01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetViewKeepScreenOn01";
    info.bundleName_ = "SetViewKeepScreenOn01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    EXPECT_EQ(sceneSession->SetViewKeepScreenOn(true), WSError::WS_OK);
    EXPECT_EQ(sceneSession->IsViewKeepScreenOn(), true);
    EXPECT_EQ(sceneSession->SetViewKeepScreenOn(false), WSError::WS_OK);
    EXPECT_EQ(sceneSession->IsViewKeepScreenOn(), false);
}

/**
 * @tc.name: IsViewKeepScreenOn01
 * @tc.desc: IsViewKeepScreenOn true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsViewKeepScreenOn01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsViewKeepScreenOn01";
    info.bundleName_ = "IsViewKeepScreenOn01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetViewKeepScreenOn(true));
    ASSERT_EQ(true, sceneSession->IsViewKeepScreenOn());
}

/**
 * @tc.name: IsViewKeepScreenOn02
 * @tc.desc: IsViewKeepScreenOn false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsViewKeepScreenOn02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsViewKeepScreenOn02";
    info.bundleName_ = "IsViewKeepScreenOn02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetViewKeepScreenOn(false));
    ASSERT_EQ(false, sceneSession->IsViewKeepScreenOn());
}

/**
 * @tc.name: HandleActionUpdateWindowShadowEnabled01
 * @tc.desc: HandleActionUpdateWindowShadowEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, HandleActionUpdateWindowShadowEnabled01, TestSize.Level1)
{
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_WINDOW_SHADOW_ENABLED;
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateWindowShadowEnabled";
    info.bundleName_ = "HandleActionUpdateWindowShadowEnabled";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    sceneSession->containerColorList_.insert("abc");
    auto ret = sceneSession->HandleActionUpdateWindowShadowEnabled(property, action);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);

    sceneSession->containerColorList_.insert("HandleActionUpdateWindowShadowEnabled");
    ret = sceneSession->HandleActionUpdateWindowShadowEnabled(property, action);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = sceneSession->HandleActionUpdateWindowShadowEnabled(property, action);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetWindowShadowEnabled01
 * @tc.desc: SetWindowShadowEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetWindowShadowEnabled01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowShadowEnabled";
    info.bundleName_ = "SetWindowShadowEnabled";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    bool isEnabled = true;
    EXPECT_EQ(WSError::WS_OK, sceneSession->SetWindowShadowEnabled(isEnabled));
    EXPECT_EQ(isEnabled, sceneSession->GetWindowShadowEnabled());
    isEnabled = false;
    EXPECT_EQ(WSError::WS_OK, sceneSession->SetWindowShadowEnabled(isEnabled));
    EXPECT_EQ(isEnabled, sceneSession->GetWindowShadowEnabled());
}

/**
 * @tc.name: GetWindowName
 * @tc.desc: GetWindowName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetWindowName, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "GetWindowName";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_NE("ww", sceneSession->GetWindowName());
}

/**
 * @tc.name: IsDecorEnable
 * @tc.desc: IsDecorEnable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsDecorEnable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsDecorEnable";
    info.bundleName_ = "IsDecorEnable";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(true, sceneSession->IsDecorEnable());
    SessionInfo info_;
    info_.abilityName_ = "Background01";
    info_.bundleName_ = "IsDecorEnable";
    info_.windowType_ = 1000;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info_, nullptr);
    EXPECT_NE(sceneSession1, nullptr);
    ASSERT_EQ(false, sceneSession1->IsDecorEnable());
}

/**
 * @tc.name: IsDecorEnable01
 * @tc.desc: IsDecorEnable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsDecorEnable01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsDecorEnable01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    EXPECT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_->SetDecorEnable(true);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ASSERT_EQ(true, sceneSession->IsDecorEnable());
}

/**
 * @tc.name: UpdateNativeVisibility
 * @tc.desc: UpdateNativeVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateNativeVisibility, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "UpdateNativeVisibility";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->UpdateNativeVisibility(false);
    ASSERT_EQ(false, sceneSession->IsVisible());
    sceneSession->NotifyWindowVisibility();

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->NotifyWindowVisibility();
}

/**
 * @tc.name: SetPrivacyMode01
 * @tc.desc: Set PrivacyMode as false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetPrivacyMode01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetPrivacyMode01";
    info.bundleName_ = "SetPrivacyMode01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->SetPrivacyMode(false);
    ASSERT_EQ(false, sceneSession->property_->GetPrivacyMode());
    ASSERT_EQ(false, sceneSession->property_->GetSystemPrivacyMode());
}

/**
 * @tc.name: SetPrivacyMode02
 * @tc.desc: Set PrivacyMode as true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetPrivacyMode02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetPrivacyMode02";
    info.bundleName_ = "SetPrivacyMode";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->SetPrivacyMode(true);
    ASSERT_EQ(true, sceneSession->property_->GetPrivacyMode());
    ASSERT_EQ(true, sceneSession->property_->GetSystemPrivacyMode());
}

/**
 * @tc.name: SetPrivacyMode03
 * @tc.desc: Set PrivacyMode as true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetPrivacyMode03, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "TestAbility";
    info.bundleName_ = "TestBundle";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    auto runner = AppExecFwk::EventRunner::Create("SceneSessionTest");
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    sceneSession->SetEventHandler(handler, handler);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->SetPrivacyMode(true);
    EXPECT_EQ(sceneSession->appUseControlMap_.size(), 0);
}

/**
 * @tc.name: UpdatePrivacyModeControlInfo01
 * @tc.desc: UpdatePrivacyModeControlInfo01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdatePrivacyModeControlInfo01, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "testBundleName";
    info.abilityName_ = "testAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(true);
    sceneSession->property_ = property;
    sceneSession->UpdatePrivacyModeControlInfo();
    EXPECT_EQ(sceneSession->appUseControlMap_.size(), 1);
    EXPECT_EQ(sceneSession->appUseControlMap_[ControlAppType::PRIVACY_WINDOW].isNeedControl, true);
}

/**
 * @tc.name: UpdatePrivacyModeControlInfo02
 * @tc.desc: UpdatePrivacyModeControlInfo02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdatePrivacyModeControlInfo02, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "testBundleName";
    info.abilityName_ = "testAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = nullptr;
    sptr<SceneSession::SpecificSessionCallback> specificSessionCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, specificSessionCb);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(true);
    subSession->property_ = property;
    sceneSession->AddSubSession(subSession);
    sceneSession->UpdatePrivacyModeControlInfo();
    EXPECT_EQ(sceneSession->appUseControlMap_.size(), 1);
    EXPECT_EQ(sceneSession->appUseControlMap_[ControlAppType::PRIVACY_WINDOW].isNeedControl, true);
}

/**
 * @tc.name: HasChildSessionInPrivacyMode
 * @tc.desc: HasChildSessionInPrivacyMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, HasChildSessionInPrivacyMode, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "testBundleName";
    info.abilityName_ = "testAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = nullptr;
    sptr<SceneSession::SpecificSessionCallback> specificSessionCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, specificSessionCb);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(false);
    subSession->property_ = property;

    subSession->subSession_.push_back(nullptr);
    subSession->AddSubSession(sptr<SceneSession>::MakeSptr(info, specificSessionCb));
    sptr<SceneSession::SpecificSessionCallback> specificSessionCb2 =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info, specificSessionCb2);
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    property2->SetPrivacyMode(true);
    subSession2->property_ = property2;
    subSession->AddSubSession(subSession2);

    sceneSession->AddSubSession(subSession);
    EXPECT_EQ(sceneSession->HasChildSessionInPrivacyMode(), true);
}

/**
 * @tc.name: HasChildSessionInPrivacyMode02
 * @tc.desc: HasChildSessionInPrivacyMode02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, HasChildSessionInPrivacyMode02, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "testBundleName";
    info.abilityName_ = "testAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = nullptr;
    sptr<SceneSession::SpecificSessionCallback> dialogSessionCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> dialogSession = sptr<SceneSession>::MakeSptr(info, dialogSessionCb);
    dialogSessionCb->onGetSceneSessionByIdCallback_ = [&dialogSession](int32_t id) { return dialogSession; };
    sceneSession->BindDialogToParentSession(dialogSession);
    EXPECT_FALSE(sceneSession->HasChildSessionInPrivacyMode());
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(false);
    dialogSession->property_ = property;
    EXPECT_FALSE(sceneSession->HasChildSessionInPrivacyMode());
    dialogSession->property_->SetPrivacyMode(true);
    EXPECT_TRUE(sceneSession->HasChildSessionInPrivacyMode());

    dialogSession->property_->SetPrivacyMode(false);
    sptr<SceneSession> dialogSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    dialogSession2->property_ = sptr<WindowSessionProperty>::MakeSptr();
    dialogSession2->property_->SetPrivacyMode(true);
    dialogSession->BindDialogToParentSession(dialogSession2);
    EXPECT_FALSE(sceneSession->HasChildSessionInPrivacyMode());
}

/**
 * @tc.name: UpdateScreenshotAppEventRegistered
 * @tc.desc: UpdateScreenshotAppEventRegistered01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateScreenshotAppEventRegistered01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    auto ret = sceneSession->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_NE(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetNotifyScreenshotAppEventRegisteredFunc
 * @tc.desc: SetNotifyScreenshotAppEventRegisteredFunc01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetNotifyScreenshotAppEventRegisteredFunc01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetNotifyScreenshotAppEventRegisteredFunc([](int32_t persistentId, bool isRegister) {});
    EXPECT_NE(sceneSession->updateScreenshotAppEventRegisteredFunc_, nullptr);
}

/**
 * @tc.name: UpdateAcrossDisplaysChangeRegistered
 * @tc.desc: UpdateAcrossDisplaysChangeRegistered01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateAcrossDisplaysChangeRegistered01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_PANEL);
    auto ret = sceneSession->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_NE(ret, WMError::WM_ERROR_INVALID_CALLING);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_NE(ret, WMError::WM_OK);
}

/**
 * @tc.name: ColorMode01
 * @tc.desc: Test OnUpdateColorMode And GetAbilityColorMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, ColorMode01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    std::string colorMode = "dark";
    bool hasDarkRes = true;
    auto ret = sceneSession->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_NE(WMError::WM_OK, ret);

    std::string resMode = sceneSession->GetAbilityColorMode();
    EXPECT_NE("dark", resMode);

    hasDarkRes = false;
    ret = sceneSession->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_NE(WMError::WM_OK, ret);
    resMode = sceneSession->GetAbilityColorMode();
    EXPECT_NE("auto", resMode);

    colorMode = "light";
    ret = sceneSession->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_NE(WMError::WM_OK, ret);
    resMode = sceneSession->GetAbilityColorMode();
    EXPECT_NE("light", resMode);
}

/**
 * @tc.name: IsMainWindowFullScreenAcrossDisplays
 * @tc.desc: IsMainWindowFullScreenAcrossDisplays01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsMainWindowFullScreenAcrossDisplays01, TestSize.Level0)
{
    bool isAcrossDisplays = false;
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_PANEL);
    auto ret = sceneSession->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_NE(ret, WMError::WM_ERROR_INVALID_CALLING);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = sceneSession->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_NE(ret, WMError::WM_OK);

    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    parentSession->property_->SetPersistentId(12);
    sceneSession->SetParentSession(parentSession);
    ret = sceneSession->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_NE(ret, WMError::WM_ERROR_NULLPTR);

    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, callBack);
    sceneSession->specificCallback_ = callBack;
    auto task = [&parentSession](int32_t persistentId) { return parentSession; };
    callBack->onGetSceneSessionByIdCallback_ = task;

    ret = sceneSession->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_NE(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: SetFullScreenWaterfallMode
 * @tc.desc: SetFullScreenWaterfallMode true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetFullScreenWaterfallMode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->pcFoldScreenController_ =
        sptr<PcFoldScreenController>::MakeSptr(wptr(sceneSession), sceneSession->GetPersistentId());
    sceneSession->SetFullScreenWaterfallMode(true);
    EXPECT_EQ(true, sceneSession->IsFullScreenWaterfallMode());

    sceneSession->pcFoldScreenController_ = nullptr;
    sceneSession->SetFullScreenWaterfallMode(true);
    EXPECT_EQ(false, sceneSession->IsFullScreenWaterfallMode());
}

/**
 * @tc.name: NotifySubSessionAcrossDisplaysChange
 * @tc.desc: NotifySubSessionAcrossDisplaysChange01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifySubSessionAcrossDisplaysChange01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    sptr<SceneSession> subSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession1->isRegisterAcrossDisplaysChanged_ = false;
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession2->isRegisterAcrossDisplaysChanged_ = true;
    subSession2->sessionStage_ = nullptr;
    sptr<SceneSession> subSession3 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession3->isRegisterAcrossDisplaysChanged_ = true;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    subSession3->sessionStage_ = mockSessionStage;
    sptr<SceneSession> subSession4 = nullptr;
    sceneSession->subSession_.push_back(subSession1);
    sceneSession->subSession_.push_back(subSession2);
    sceneSession->subSession_.push_back(subSession3);
    sceneSession->subSession_.push_back(subSession4);
    auto ret = sceneSession->NotifySubSessionAcrossDisplaysChange(true);
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyFollowedParentWindowAcrossDisplaysChange
 * @tc.desc: NotifyFollowedParentWindowAcrossDisplaysChange01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifyFollowedParentWindowAcrossDisplaysChange01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<SceneSession> subSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession1->property_->SetPersistentId(1);
    subSession1->isFollowParentLayout_ = false;
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession2->property_->SetPersistentId(2);
    subSession2->isFollowParentLayout_ = true;
    subSession2->sessionStage_ = nullptr;
    sptr<SceneSession> subSession3 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession3->property_->SetPersistentId(3);
    subSession3->isFollowParentLayout_ = true;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    subSession3->sessionStage_ = mockSessionStage;
    sptr<SceneSession> subSession4 = nullptr;

    sceneSession->notifySurfaceBoundsChangeFuncMap_[1] = nullptr;
    sceneSession->notifySurfaceBoundsChangeFuncMap_[2] = nullptr;
    sceneSession->notifySurfaceBoundsChangeFuncMap_[3] = nullptr;
    sceneSession->notifySurfaceBoundsChangeFuncMap_[4] = nullptr;

    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, callBack);
    sceneSession->specificCallback_ = callBack;

    auto task = [&subSession1, &subSession2, &subSession3, &subSession4](int32_t persistentId) {
        if (persistentId == 1) {
            return subSession1;
        } else if (persistentId == 2) {
            return subSession2;
        } else if (persistentId == 3) {
            return subSession3;
        }
        return subSession4;
    };
    callBack->onGetSceneSessionByIdCallback_ = task;
    auto ret = sceneSession->NotifyFollowedParentWindowAcrossDisplaysChange(true);
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: IsFloatingWindowAppType01
 * @tc.desc: IsFloatingWindowAppType true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsFloatingWindowAppType01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsFloatingWindowAppType01";
    info.bundleName_ = "IsFloatingWindowAppType01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    EXPECT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->SetFloatingWindowAppType(true);
    ASSERT_EQ(true, sceneSession->IsFloatingWindowAppType());
}

/**
 * @tc.name: IsFloatingWindowAppType02
 * @tc.desc: IsFloatingWindowAppType false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsFloatingWindowAppType02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsFloatingWindowAppType02";
    info.bundleName_ = "IsFloatingWindowAppType02";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    EXPECT_NE(sceneSession->property_, nullptr);
    ASSERT_EQ(false, sceneSession->IsFloatingWindowAppType());
}

/**
 * @tc.name: NotifyIsCustomAnimationPlaying
 * @tc.desc: NotifyIsCustomAnimationPlaying
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifyIsCustomAnimationPlaying, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->NotifyIsCustomAnimationPlaying(false);

    sceneSession->onIsCustomAnimationPlaying_ = [](bool status) {};
    sceneSession->NotifyIsCustomAnimationPlaying(false);
}

/**
 * @tc.name: ModalUIExtension
 * @tc.desc: ModalUIExtension
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, ModalUIExtension, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ModalUIExtension";
    info.bundleName_ = "ModalUIExtension";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    EXPECT_FALSE(sceneSession->GetLastModalUIExtensionEventInfo());
    ExtensionWindowEventInfo extensionInfo;
    extensionInfo.persistentId = 12345;
    extensionInfo.pid = 1234;
    extensionInfo.windowRect = { 1, 2, 3, 4 };
    sceneSession->AddNormalModalUIExtension(extensionInfo);

    auto getInfo = sceneSession->GetLastModalUIExtensionEventInfo();
    EXPECT_TRUE(getInfo);
    EXPECT_EQ(getInfo.value().persistentId, extensionInfo.persistentId);
    EXPECT_EQ(getInfo.value().pid, extensionInfo.pid);
    EXPECT_EQ(getInfo.value().windowRect, extensionInfo.windowRect);

    Rect windowRect = { 5, 6, 7, 8 };
    extensionInfo.windowRect = windowRect;
    sceneSession->UpdateNormalModalUIExtension(extensionInfo);
    getInfo = sceneSession->GetLastModalUIExtensionEventInfo();
    EXPECT_TRUE(getInfo);
    EXPECT_EQ(getInfo.value().windowRect, windowRect);

    sceneSession->RemoveNormalModalUIExtension(extensionInfo.persistentId);
    EXPECT_FALSE(sceneSession->GetLastModalUIExtensionEventInfo());
}

/**
 * @tc.name: NotifySessionRectChange
 * @tc.desc: NotifySessionRectChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifySessionRectChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySessionRectChange";
    info.bundleName_ = "NotifySessionRectChangebundle";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    sceneSession->NotifySessionRectChange(overlapRect, SizeChangeReason::ROTATION, -1);
    sceneSession->NotifySessionRectChange(overlapRect, SizeChangeReason::ROTATION, 11);
    sceneSession->sessionRectChangeFunc_ = [](const WSRect& rect,
                                              SizeChangeReason reason,
                                              DisplayId displayId,
                                              const RectAnimationConfig& rectAnimationConfig) { return; };
    sceneSession->NotifySessionRectChange(overlapRect, SizeChangeReason::ROTATION, -1);
    sceneSession->NotifySessionRectChange(overlapRect, SizeChangeReason::ROTATION, 11);

    auto oriProperty = sceneSession->GetSessionProperty();
    sceneSession->property_ = oriProperty;
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->moveDragController_ = moveDragController;
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sceneSession->systemConfig_.freeMultiWindowSupport_ = true;
    sceneSession->systemConfig_.freeMultiWindowEnable_ = true;
    sceneSession->dragResizeTypeDuringDrag_ = DragResizeType::RESIZE_SCALE;
    sceneSession->moveDragController_->isStartDrag_ = true;
    sceneSession->needNotifyDragEventOnNextVsync_ = true;
    sceneSession->NotifySessionRectChange(overlapRect, reason, 11);
    EXPECT_EQ(sceneSession->IsDragResizeScale(reason), true);
}

/**
 * @tc.name: GetKeyboardAvoidArea
 * @tc.desc: GetKeyboardAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetKeyboardAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    specificCallback_->onGetSceneSessionVectorByType_ = [](WindowType type) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> backgroundSession;
        return backgroundSession;
    };

    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->GetKeyboardAvoidArea(overlapRect, avoidArea);
    ASSERT_EQ(true, overlapRect.IsEmpty());
    ASSERT_EQ(true, sceneSession->keyboardAvoidAreaActive_);
    sceneSession->keyboardAvoidAreaActive_ = false;
    sceneSession->GetKeyboardAvoidArea(overlapRect, avoidArea);
    ASSERT_EQ(false, sceneSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(true, overlapRect.IsEmpty());
}

/**
 * @tc.name: GetCutoutAvoidArea
 * @tc.desc: GetCutoutAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetCutoutAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->GetCutoutAvoidArea(overlapRect, avoidArea);
}

/**
 * @tc.name: SetSystemBarProperty
 * @tc.desc: SetSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetSystemBarProperty, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    SystemBarProperty statusBarProperty;
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_FLOAT_CAMERA, statusBarProperty),
              WSError::WS_OK);
}

/**
 * @tc.name: SetSystemBarProperty02
 * @tc.desc: SetSystemBarProperty02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetSystemBarProperty02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    SystemBarProperty statusBarProperty;
    ASSERT_EQ(sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusBarProperty),
              WSError::WS_OK);
    auto propMap = sceneSession->property_->GetSystemBarProperty();
    ASSERT_EQ(statusBarProperty, propMap[WindowType::WINDOW_TYPE_STATUS_BAR]);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_FLOAT_CAMERA, statusBarProperty),
              WSError::WS_OK);
}

/**
 * @tc.name: SetSystemBarProperty03
 * @tc.desc: SetSystemBarProperty03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetSystemBarProperty03, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetSystemBarProperty03";
    info.bundleName_ = "SetSystemBarProperty03";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallback->onNotifyWindowSystemBarPropertyChangeFunc_ = [](
        WindowType type, const SystemBarProperty& systemBarProperty) {};
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sceneSession->onSystemBarPropertyChange_ =[](
        const std::unordered_map<WindowType, SystemBarProperty>& propertyMap) {};
    SystemBarProperty statusBarProperty;
    EXPECT_EQ(sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusBarProperty),
        WSError::WS_OK);
    auto propMap = sceneSession->property_->GetSystemBarProperty();
    EXPECT_EQ(statusBarProperty, propMap[WindowType::WINDOW_TYPE_STATUS_BAR]);
    EXPECT_EQ(sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, statusBarProperty),
        WSError::WS_OK);
    sceneSession->specificCallback_->onNotifyWindowSystemBarPropertyChangeFunc_ = nullptr;
    EXPECT_EQ(sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusBarProperty),
        WSError::WS_OK);
}

/**
 * @tc.name: OnShowWhenLocked
 * @tc.desc: OnShowWhenLocked
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionTest, OnShowWhenLocked, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    auto res = sceneSession->OnShowWhenLocked(false);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: IsShowWhenLocked
 * @tc.desc: IsShowWhenLocked
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionTest, IsShowWhenLocked, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(sceneSession->IsShowWhenLocked(), false);
    sceneSession->property_ = property;
    sceneSession->SetTemporarilyShowWhenLocked(true);
    ASSERT_EQ(sceneSession->IsShowWhenLocked(), true);
    property->SetWindowFlags(4);
    sceneSession->SetTemporarilyShowWhenLocked(false);
    ASSERT_EQ(sceneSession->IsShowWhenLocked(), true);
    sceneSession->SetTemporarilyShowWhenLocked(true);
    ASSERT_EQ(sceneSession->IsShowWhenLocked(), true);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionTest, GetAvoidAreaByType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        SessionInfo info_;
        info_.abilityName_ = "Background01";
        info_.bundleName_ = "IsFloatingWindowAppType";
        std::vector<sptr<SceneSession>> backgroundSession;
        sptr<SceneSession> session2 = sptr<SceneSession>::MakeSptr(info_, nullptr);
        backgroundSession.push_back(session2);
        return backgroundSession;
    };
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 320, 240 }; // width: 320, height: 240
    sceneSession->SetSessionRect(rect);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_ = property;
    AvoidArea avoidArea;
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT);
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM);
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_KEYBOARD);
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM_GESTURE);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.desc: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionTest, GetAvoidAreaByTypeIgnoringVisibility, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "GetAvoidAreaByTypeIgnoringVisibility";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        SessionInfo info_;
        info_.abilityName_ = "Background01";
        info_.bundleName_ = "GetAvoidAreaByTypeIgnoringVisibility";
        std::vector<sptr<SceneSession>> backgroundSession;
        sptr<SceneSession> session2 = sptr<SceneSession>::MakeSptr(info_, nullptr);
        backgroundSession.push_back(session2);
        return backgroundSession;
    };
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 320, 240 }; // width: 320, height: 240
    sceneSession->SetSessionRect(rect);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_ = property;
    sing T = std::underlying_type_t<AvoidAreaType>;
    for (T avoidAreaType = static_cast<T>(AvoidAreaType::TYPE_START);
        avoidAreaType < static_cast<T>(AvoidAreaType::TYPE_END); avoidAreaType++) {
        auto type = static_cast<AvoidAreaType>(avoidAreaType);
        sceneSession->GetAvoidAreaByTypeIgnoringVisibility(type);
    }
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, TransferPointerEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent), WSError::WS_OK);

    std::shared_ptr<MMI::PointerEvent> pointerEvent_ = MMI::PointerEvent::Create();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->SetPersistentId(11);
    sceneSession->SetSessionProperty(property);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);
}

/**
 * @tc.name: TransferPointerEventDecorDialog
 * @tc.desc: TransferPointerEventDecorDialog
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, TransferPointerEventDecorDialog, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "TransferPointerEventDecorDialog";
    info.bundleName_ = "TransferPointerEventDecorDialogbundle";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    std::shared_ptr<MMI::PointerEvent> pointerEvent_ = MMI::PointerEvent::Create();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    property->SetDecorEnable(true);
    property->SetDragEnabled(true);
    property->SetPersistentId(12);
    sceneSession->property_ = property;
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: NotifyAddOrRemoveSnapshotWindowTest
 * @tc.desc: NotifyAddOrRemoveSnapshotWindowTest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifyAddOrRemoveSnapshotWindowTest, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    info.windowType_ = 1;
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->showRecent_ = true;
    sceneSession->NotifyAddOrRemoveSnapshotWindow(true);
}

/**
 * @tc.name: ProcessWindowMoving
 * @tc.desc: ProcessWindowMoving
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, ProcessWindowMoving, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessWindowMoving";
    info.bundleName_ = "ProcessWindowMovingBundle";
    info.windowType_ = 1;
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    sceneSession->ProcessWindowMoving(pointerEvent);
}

/**
 * @tc.name: PrintAvoidAreaInfo
 * @tc.desc: PrintAvoidAreaInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, PrintAvoidAreaInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "PrintAvoidAreaInfo";
    info.bundleName_ = "PrintAvoidAreaInfo";
    sptr<SceneSession>sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_->displayId_ = 0;
    WSRect winRect;
    WSRect avoidRect;
    auto type = AvoidAreaType::TYPE_SYSTEM;
    sceneSession->PrintAvoidAreaInfo(sceneSession->property_->displayId_, type, winRect, avoidRect);
    std::tuple<DisplayId, WSRect, WSRect> inputParamters(sceneSession->property_->displayId_, winRect, avoidRect);
    EXPECT_EQ(inputParamters, sceneSession->lastAvoidAreaInputParamtersMap_[type]);
    sceneSession->property_->displayId_ = 1;
    sceneSession->PrintAvoidAreaInfo(sceneSession->property_->displayId_, type, winRect, avoidRect);
    std::tuple<DisplayId, WSRect, WSRect> inputParamters1(sceneSession->property_->displayId_, winRect, avoidRect);
    EXPECT_EQ(inputParamters1, sceneSession->lastAvoidAreaInputParamtersMap_[type]);
    sceneSession->PrintAvoidAreaInfo(sceneSession->property_->displayId_, type, winRect, avoidRect);
    EXPECT_EQ(inputParamters1, sceneSession->lastAvoidAreaInputParamtersMap_[type]);
    AvoidArea avoidArea;
    sceneSession->CalculateAvoidAreaByType(type, winRect, avoidRect, avoidArea);
    EXPECT_EQ(inputParamters1, sceneSession->lastAvoidAreaInputParamtersMap_[type]);
}

/**
 * @tc.name: CalculateAvoidAreaRect
 * @tc.desc: CalculateAvoidAreaRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, CalculateAvoidAreaRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    WSRect avoidRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->CalculateAvoidAreaRect(overlapRect, avoidRect, avoidArea);
    WSRect overlapRect_ = { 1, 1, 1, 1 };
    WSRect avoidRect_ = { 1, 1, 1, 1 };
    sceneSession->CalculateAvoidAreaRect(overlapRect_, avoidRect_, avoidArea);
}

/**
 * @tc.name: OnNeedAvoid
 * @tc.desc: OnNeedAvoid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, OnNeedAvoid, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(sceneSession->OnNeedAvoid(false), WSError::WS_OK);

    sceneSession->onNeedAvoid_ = [](bool state) {};
    ASSERT_EQ(sceneSession->OnNeedAvoid(false), WSError::WS_OK);
}

/**
 * @tc.name: SetCollaboratorType
 * @tc.desc: SetCollaboratorType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetCollaboratorType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetCollaboratorType(2);
    ASSERT_EQ(sceneSession->GetCollaboratorType(), 2);
}

/**
 * @tc.name: GetAbilityInfo
 * @tc.desc: GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetAbilityInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
    sceneSession->SetAbilitySessionInfo(abilityInfo);
    ASSERT_EQ(sceneSession->GetAbilityInfo(), abilityInfo);
}

/**
 * @tc.name: UpdateCameraWindowStatus
 * @tc.desc: UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateCameraWindowStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "UpdateCameraWindowStatus";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SystemSession> sysSession;
    sysSession = sptr<SystemSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sysSession, nullptr);
    specificCallback_->onCameraFloatSessionChange_ = [](uint32_t accessTokenId, bool isShowing) {};
    specificCallback_->onCameraSessionChange_ = [](uint32_t accessTokenId, bool isShowing) {};

    sysSession->UpdateCameraWindowStatus(false);
    sysSession = sptr<SystemSession>::MakeSptr(info, specificCallback_);
    sysSession->UpdateCameraWindowStatus(false);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sysSession->property_ = property;
    sysSession->UpdateCameraWindowStatus(false);
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    sysSession->property_ = property;
    sysSession->UpdateCameraWindowStatus(false);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    PiPTemplateInfo pipType;
    pipType.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL);
    sysSession->SetPiPTemplateInfo(pipType);
    sysSession->property_ = property;
    sysSession->UpdateCameraWindowStatus(false);
}

/**
 * @tc.name: GetRatioPreferenceKey
 * @tc.desc: GetRatioPreferenceKey
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetRatioPreferenceKey, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ability";
    info.bundleName_ = "bundle";
    info.moduleName_ = "module";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    std::string key = info.bundleName_ + info.moduleName_ + info.abilityName_;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    ASSERT_EQ(key, sceneSession->GetRatioPreferenceKey());

    std::string key2(30, 'a');
    std::string key3(80, 'a');
    sceneSession->sessionInfo_.bundleName_ = key2;
    sceneSession->sessionInfo_.moduleName_ = key2;
    sceneSession->sessionInfo_.abilityName_ = key2;
    ASSERT_EQ(key3, sceneSession->GetRatioPreferenceKey());
}

/**
 * @tc.name: NotifyPropertyWhenConnect
 * @tc.desc: NotifyPropertyWhenConnect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifyPropertyWhenConnect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ability";
    info.bundleName_ = "bundle";
    info.moduleName_ = "module";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    std::string key = info.bundleName_ + info.moduleName_ + info.abilityName_;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sceneSession->NotifyPropertyWhenConnect();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_ = property;
    sceneSession->NotifyPropertyWhenConnect();
}

/**
 * @tc.name: CalcRectForStatusBar
 * @tc.desc: CalcRectForStatusBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, CalcRectForStatusBar, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CalcRectForStatusBar";
    info.bundleName_ = "CalcRectForStatusBar";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    DMRect statusBarRect = sceneSession->CalcRectForStatusBar();
    ASSERT_EQ(statusBarRect.width_, 0);
    ASSERT_EQ(statusBarRect.height_, 0);

    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    statusBarRect = sceneSession->CalcRectForStatusBar();
    ASSERT_EQ(statusBarRect.width_, 0);
    ASSERT_EQ(statusBarRect.height_, 0);

    WSRect rect({0, 0, 1, 1});
    sceneSession->GetLayoutController()->SetSessionRect(rect);
    sceneSession->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [&](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> vec;
        vec.push_back(sceneSession);
        return vec;
    };

    sceneSession->isVisible_ = false;
    statusBarRect = sceneSession->CalcRectForStatusBar();
    EXPECT_EQ(statusBarRect.width_, 1);
    EXPECT_EQ(statusBarRect.height_, 0);

    sceneSession->isVisible_ = true;
    statusBarRect = sceneSession->CalcRectForStatusBar();
    EXPECT_EQ(statusBarRect.width_, 1);
    EXPECT_EQ(statusBarRect.height_, 1);
}

/**
 * @tc.name: InitializeMoveInputBar
 * @tc.desc: InitializeMoveInputBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, InitializeMoveInputBar, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "InitializeMoveInputBar";
    info.bundleName_ = "InitializeMoveInputBar";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto property = sceneSession->GetSessionProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR);
    property->SetDisplayId(1);

    auto result = sceneSession->InitializeMoveInputBar();
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: OnSessionEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, OnSessionEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnSessionEvent";
    info.bundleName_ = "OnSessionEvent";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
    sceneSession->moveDragController_->isStartDrag_ = true;
    sceneSession->moveDragController_->hasPointDown_ = true;
    ASSERT_EQ(sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE), WSError::WS_OK);
    ASSERT_EQ(sceneSession->OnSessionEvent(SessionEvent::EVENT_END_MOVE), WSError::WS_OK);

    SessionEventParam param;
    auto ret = sceneSession->OnSessionEvent(SessionEvent::EVENT_MAXIMIZE, param);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: SyncSessionEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SyncSessionEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEvent";
    info.bundleName_ = "SyncSessionEvent";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    sceneSession->isActive_ = false;

    SessionEvent event = SessionEvent::EVENT_START_MOVE;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sceneSession->property_->isSystemCalling_ = false;
    result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetTopmost
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetTopmost, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetTopmost";
    info.bundleName_ = "SetTopmost";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    auto result = sceneSession->SetTopmost(false);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_FALSE(sceneSession->IsTopmost());
}

/**
 * @tc.name: SetMainWindowTopmost
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetMainWindowTopmost, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetMainWindowTopmost";
    info.bundleName_ = "SetMainWindowTopmost";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    auto result = sceneSession->SetMainWindowTopmost(false);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_FALSE(sceneSession->IsMainWindowTopmost());
}

/**
 * @tc.name: UpdateSessionRectPosYFromClient01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRectPosYFromClient01, TestSize.Level1)
{
    DisplayId displayId = 0;
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRectPosYFromClient";
    info.bundleName_ = "UpdateSessionRectPosYFromClient";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->sessionInfo_.screenId_ = 0;
    EXPECT_EQ(sceneSession->GetScreenId(), 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::EXPANDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    WSRect rect = { 0, 0, 0, 0 };
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_EQ(rect.posY_, 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::KEYBOARD, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    rect = { 0, 100, 0, 0 };
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_EQ(rect.posY_, 100);

    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    sceneSession->clientDisplayId_ = 0;
    rect = { 0, 100, 100, 100 };
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_EQ(rect.posY_, 100);
    sceneSession->clientDisplayId_ = 999;
    sceneSession->configDisplayId_ = 999;
    rect = { 0, 100, 100, 100 };
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::RESIZE, displayId, rect);
    EXPECT_EQ(rect.posY_, 100 + 1648 + 40);

    sceneSession->clientDisplayId_ = 999;
    sceneSession->configDisplayId_ = 999;
    rect = { 0, 1700, 100, 100 };
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::RESIZE, displayId, rect);
    EXPECT_EQ(rect.posY_, 1700);

    WSRect sessionRect = {100, 200, 1000, 1000};
    sceneSession->SetSessionRect(sessionRect);
    sceneSession->clientDisplayId_ = 999;
    sceneSession->configDisplayId_ = 999;
    rect = { 0, -100, 100, 100 };
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::RESIZE, displayId, rect);
    EXPECT_EQ(rect.posY_, -100 + sessionRect.posY_);
}

/**
 * @tc.name: UpdateSessionRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRect";
    info.bundleName_ = "UpdateSessionRect";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;

    sceneSession->SetSessionProperty(property);
    WSRect rect({ 1, 1, 1, 1 });
    SizeChangeReason reason = SizeChangeReason::MOVE;
    WSError result = sceneSession->UpdateSessionRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRect";
    info.bundleName_ = "UpdateSessionRect";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;

    sceneSession->SetSessionProperty(property);
    WSRect rect({ 1, 1, 1, 1 });
    SizeChangeReason reason = SizeChangeReason::RESIZE;
    WSError result = sceneSession->UpdateSessionRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect2, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRect";
    info.bundleName_ = "UpdateSessionRect";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;

    sceneSession->SetSessionProperty(property);
    WSRect rect({ 1, 1, 1, 1 });
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError result = sceneSession->UpdateSessionRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect3
 * @tc.desc: test for isGlobal is true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect3, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRect";
    info.bundleName_ = "UpdateSessionRect";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;

    sceneSession->SetSessionProperty(property);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSRect oldRect({ 1, 1, 1, 1 });
    WSRect parentRect({ 10, 10, 1, 1 });

    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetParentSession(parentSession);
    EXPECT_NE(sceneSession->GetParentSession(), nullptr);
    parentSession->SetSessionRect(parentRect);
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;

    bool isGlobal = true;
    WSError result = sceneSession->UpdateSessionRect(oldRect, reason, isGlobal);
    ASSERT_EQ(result, WSError::WS_OK);

    WSRect newRect = sceneSession->GetSessionRect();
    ASSERT_EQ(newRect.posX_, oldRect.posX_ - parentRect.posX_);
    ASSERT_EQ(newRect.posY_, oldRect.posY_ - parentRect.posY_);
}

/**
 * @tc.name: UpdateSessionRect4
 * @tc.desc: test for isGlobal is true and multilevel subWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect4, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRect4";
    info.bundleName_ = "UpdateSessionRect4";
    sptr<SceneSession> mainSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    mainSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainSession->property_->SetPersistentId(1111);
    mainSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->property_->SetPersistentId(1122);
    subSession->SetParentSession(mainSession);
    subSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<SceneSession> subSubSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSubSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSubSession->property_->SetPersistentId(1133);
    subSubSession->SetParentSession(subSession);
    subSubSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    // Test the changes in the rect value of the first-level subWindow in the phone scenario
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSRect oldRect1({ 20, 20, 20, 20 });
    WSRect mainRect({ 10, 10, 10, 10 });
    mainSession->SetSessionRect(mainRect);
    bool isGlobal = true;
    WSError result = subSession->UpdateSessionRect(oldRect1, reason, isGlobal);
    EXPECT_EQ(result, WSError::WS_OK);
    WSRect newRect1 = subSession->GetSessionRect();
    EXPECT_EQ(newRect1.posX_, oldRect1.posX_ - mainRect.posX_);
    EXPECT_EQ(newRect1.posY_, oldRect1.posY_ - mainRect.posY_);
    mainSession->systemConfig_.freeMultiWindowSupport_ = true;
    mainSession->systemConfig_.freeMultiWindowEnable_ = true;
    result = mainSession->UpdateSessionRect(oldRect1, reason, isGlobal);
    result = mainSession->UpdateSessionRect(oldRect1, reason, isGlobal, true);
    mainSession->systemConfig_.freeMultiWindowEnable_ = false;
    result = mainSession->UpdateSessionRect(oldRect1, reason, isGlobal);
    result = mainSession->UpdateSessionRect(oldRect1, reason, isGlobal, true);
    subSubSession->systemConfig_.freeMultiWindowSupport_ = true;
    subSubSession->systemConfig_.freeMultiWindowEnable_ = true;
    result = subSubSession->UpdateSessionRect(oldRect1, reason, isGlobal);
    result = subSubSession->UpdateSessionRect(oldRect1, reason, isGlobal, true);
    subSubSession->systemConfig_.freeMultiWindowEnable_ = false;
    result = subSubSession->UpdateSessionRect(oldRect1, reason, isGlobal);
    result = subSubSession->UpdateSessionRect(oldRect1, reason, isGlobal, true);
    ASSERT_EQ(result, WSError::WS_OK);
    // Test the changes of rect values of multi-level subWindow in the phone scenario
    WSRect oldRect2({ 30, 30, 30, 30 });
    result = subSubSession->UpdateSessionRect(oldRect2, reason, isGlobal);
    EXPECT_EQ(result, WSError::WS_OK);
    WSRect newRect2 = subSubSession->GetSessionRect();
    EXPECT_EQ(newRect2.posX_, oldRect2.posX_ - mainRect.posX_);
    EXPECT_EQ(newRect2.posY_, oldRect2.posY_ - mainRect.posY_);
}

/**
 * @tc.name: GetStatusBarHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetStatusBarHeight, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetStatusBarHeight";
    info.bundleName_ = "GetStatusBarHeight";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    int32_t height = sceneSession->GetStatusBarHeight();
    ASSERT_EQ(height, 0);
    SystemBarProperty propertyHide;
    propertyHide.enable_ = false;
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, propertyHide));
    ASSERT_EQ(height, 0);
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    height = sceneSession->GetStatusBarHeight();
    ASSERT_EQ(height, 0);
    WSRect rect({ 0, 0, 0, 1 });
    sceneSession->GetLayoutController()->SetSessionRect(rect);
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [&](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> vec;
        vec.push_back(sceneSession);
        return vec;
    };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    sceneSession->property_ = property;
    height = sceneSession->GetStatusBarHeight();
    ASSERT_EQ(height, 1);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, propertyHide));
    ASSERT_EQ(height, 1);
}

/**
 * @tc.name: GetDockHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetDockHeight, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetDockHeight";
    info.bundleName_ = "GetDockHeight";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(sceneSession->GetDockHeight(), 0);
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    ASSERT_EQ(sceneSession->GetDockHeight(), 0);
    WSRect rect({ 0, 0, 0, 112 });
    sceneSession->GetLayoutController()->SetSessionRect(rect);
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [&](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> vec;
        vec.push_back(sceneSession);
        return vec;
    };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    sceneSession->property_ = property;
    ASSERT_EQ(sceneSession->GetDockHeight(), 0);
    sceneSession->isVisible_ = true;
    ASSERT_EQ(sceneSession->GetDockHeight(), 0);
    sceneSession->property_->windowName_ = "SCBSmartDock";
    ASSERT_EQ(sceneSession->GetDockHeight(), 112);
}

/**
 * @tc.name: GetAppForceLandscapeConfig
 * @tc.desc: GetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetAppForceLandscapeConfig, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetAppForceLandscapeConfig";
    info.bundleName_ = "GetAppForceLandscapeConfig";
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    AppForceLandscapeConfig config = {};
    auto result = sceneSession->GetAppForceLandscapeConfig(config);
    ASSERT_EQ(result, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: SetDefaultDisplayIdIfNeed
 * @tc.desc: SetDefaultDisplayIdIfNeed
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetDefaultDisplayIdIfNeed, TestSize.Level0)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetDefaultDisplayIdIfNeed();

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetDisplayId(-1);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetDefaultDisplayIdIfNeed();
    EXPECT_EQ(property->GetDisplayId(), SCREEN_ID_INVALID);
}

/**
 * @tc.name: SetSessionGlobalRect/GetSessionGlobalRect
 * @tc.desc: SetSessionGlobalRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetSessionGlobalRect, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    WSRect test = { 100, 100, 100, 100 };
    sceneSession->SetSessionGlobalRect(test);
    sceneSession->SetScbCoreEnabled(true);
    EXPECT_EQ(test, sceneSession->GetSessionGlobalRect());
}

/**
 * @tc.name: SetSessionGlobalRect/GetSessionGlobalRect
 * @tc.desc: SetSessionGlobalRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetIsStatusBarVisibleInner01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetIsStatusBarVisibleInner01";
    info.bundleName_ = "SetIsStatusBarVisibleInner01";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isStatusBarVisible_ = true;
    EXPECT_EQ(sceneSession->SetIsStatusBarVisibleInner(true), WSError::WS_OK);
    EXPECT_EQ(sceneSession->SetIsStatusBarVisibleInner(false), WSError::WS_ERROR_NULLPTR);

    sceneSession->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) { return WSError::WS_ERROR_NULLPTR; };
    EXPECT_EQ(sceneSession->SetIsStatusBarVisibleInner(true), WSError::WS_ERROR_NULLPTR);

    sceneSession->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        isLayoutFinished = false;
        return WSError::WS_OK;
    };
    EXPECT_EQ(sceneSession->SetIsStatusBarVisibleInner(false), WSError::WS_OK);

    sceneSession->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        isLayoutFinished = true;
        return WSError::WS_OK;
    };
    sceneSession->specificCallback_ = nullptr;
    EXPECT_EQ(sceneSession->SetIsStatusBarVisibleInner(false), WSError::WS_OK);
}

/**
 * @tc.name: SetMousePointerDownEventStatus
 * @tc.desc: SetMousePointerDownEventStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetMousePointerDownEventStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetMousePointerDownEventStatus";
    info.bundleName_ = "SetMousePointerDownEventStatus";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->SetMousePointerDownEventStatus(true);
    EXPECT_EQ(sceneSession->GetMousePointerDownEventStatus(), true);
}

/**
 * @tc.name: SetFingerPointerDownStatus
 * @tc.desc: SetFingerPointerDownStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetFingerPointerDownStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetFingerPointerDownStatus";
    info.bundleName_ = "SetFingerPointerDownStatus";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->SetFingerPointerDownStatus(0);
    sceneSession->SetFingerPointerDownStatus(1);
    auto fingerPointerDownStatusList = sceneSession->GetFingerPointerDownStatusList();
    EXPECT_EQ(fingerPointerDownStatusList.size(), 2);
    sceneSession->RemoveFingerPointerDownStatus(0);
    fingerPointerDownStatusList = sceneSession->GetFingerPointerDownStatusList();
    EXPECT_EQ(fingerPointerDownStatusList.size(), 1);

    sceneSession->RemoveFingerPointerDownStatus(1);
    fingerPointerDownStatusList = sceneSession->GetFingerPointerDownStatusList();
    EXPECT_EQ(fingerPointerDownStatusList.size(), 0);
}

/**
 * @tc.name: SetUIFirstSwitch
 * @tc.desc: SetUIFirstSwitch
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetUIFirstSwitch, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetUIFirstSwitch";
    info.bundleName_ = "SetUIFirstSwitch";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetUIFirstSwitch(RSUIFirstSwitch::FORCE_DISABLE_NONFOCUS);
}

/**
 * @tc.name: IsFollowParentMultiScreenPolicy
 * @tc.desc: IsFollowParentMultiScreenPolicy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsFollowParentMultiScreenPolicy, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_EQ(sceneSession->IsFollowParentMultiScreenPolicy(), false);
}

/**
 * @tc.name: CloneWindow
 * @tc.desc: CloneWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, CloneWindow, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CloneWindow";
    info.bundleName_ = "CloneWindow";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);
    uint64_t surfaceNodeId = 1;
    bool needOffScreen = false;
    sceneSession->SetSurfaceNode(nullptr);
    sceneSession->CloneWindow(surfaceNodeId, needOffScreen);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    auto sfaceNode = sceneSession->GetSurfaceNode();
    sceneSession->CloneWindow(surfaceNodeId, needOffScreen);
    EXPECT_TRUE(sfaceNode != nullptr);
}

/**
 * @tc.name: GetGlobalOrWinRect
 * @tc.desc: GetGlobalOrWinRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetGlobalOrWinRect, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sceneSession->GetLayoutController()->SetSessionRect({1, 1, 1, 1});
    sceneSession->SetSessionGlobalRect({2, 2, 1, 1});
    sceneSession->isScbCoreEnabled_ = true;
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto rect = sceneSession->GetGlobalOrWinRect();
    EXPECT_EQ(1, rect.posX_);
    EXPECT_EQ(1, rect.posY_);

    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sceneSession->systemConfig_.freeMultiWindowEnable_ = true;
    sceneSession->systemConfig_.freeMultiWindowSupport_ = true;
    rect = sceneSession->GetGlobalOrWinRect();
    EXPECT_EQ(1, rect.posX_);
    EXPECT_EQ(1, rect.posY_);

    sceneSession->systemConfig_.freeMultiWindowEnable_ = false;
    sceneSession->systemConfig_.freeMultiWindowSupport_ = false;
    rect = sceneSession->GetGlobalOrWinRect();
    EXPECT_EQ(2, rect.posX_);
    EXPECT_EQ(2, rect.posY_);
}

/**
 * @tc.name: GetWindowLimits
 * @tc.desc: Verify GetWindowLimits handles null property and valid property
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetWindowLimits, TestSize.Level1)
{
    SessionInfo info;
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Case 1: property_ is null → return default WindowLimits()
    {
        sceneSession->property_ = nullptr;
        WindowLimits limits = sceneSession->GetWindowLimits();

        EXPECT_EQ(limits.maxWidth_, static_cast<uint32_t>(INT32_MAX));
        EXPECT_EQ(limits.maxHeight_, static_cast<uint32_t>(INT32_MAX));
        EXPECT_EQ(limits.minWidth_, 1);
        EXPECT_EQ(limits.minHeight_, 1);
    }

    // Case 2: property_ valid → return property->limits_
    {
        auto property = sptr<WindowSessionProperty>::MakeSptr();
        WindowLimits expectedLimits(800, 600, 200, 100, 0.0, 0.0);

        property->SetWindowLimits(expectedLimits);
        sceneSession->property_ = property;
        WindowLimits limits = sceneSession->GetWindowLimits();

        EXPECT_EQ(limits.maxWidth_, 800);
        EXPECT_EQ(limits.maxHeight_, 600);
        EXPECT_EQ(limits.minWidth_, 200);
        EXPECT_EQ(limits.minHeight_, 100);
    }
}
} // namespace
} // namespace Rosen
} // namespace OHOS