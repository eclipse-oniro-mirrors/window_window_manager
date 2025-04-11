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
#include "input_event.h"
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
constexpr int WAIT_ASYNC_US = 1000000;
class SceneSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionTest::SetUpTestCase()
{
}

void SceneSessionTest::TearDownTestCase()
{
}

void SceneSessionTest::SetUp()
{
}

void SceneSessionTest::TearDown()
{
}

namespace {

/**
 * @tc.name: SetGlobalMaximizeMode01
 * @tc.desc: SetGlobalMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetGlobalMaximizeMode01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, GetGlobalMaximizeMode01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, GlobalMaximizeMode01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, GlobalMaximizeMode02, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetAndGetPipTemplateInfo, Function | SmallTest | Level2)
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
    ASSERT_EQ(sceneSession->GetPiPTemplateInfo().pipTemplateType,
        static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL));

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
HWTEST_F(SceneSessionTest, UpdateWindowSceneAfterCustomAnimation01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetZOrder01, Function | SmallTest | Level2)
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
    sceneSession->SetZOrder(zOrder);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(zOrder, sceneSession->zOrder_);
}

/**
 * @tc.name: GetTouchHotAreas01
 * @tc.desc: GetTouchHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetTouchHotAreas01, Function | SmallTest | Level2)
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
    Rect windowRect = {1, 1, 1, 1};
    std::vector<Rect> rects;
    uint32_t hotAreasNum = 10;
    uint32_t hotAreaWidth = windowRect.width_ / hotAreasNum;
    uint32_t hotAreaHeight = windowRect.height_ / hotAreasNum;
    for (uint32_t i = 0; i < hotAreasNum; ++i) {
        rects.emplace_back(Rect{hotAreaWidth * i, hotAreaHeight * i, hotAreaWidth, hotAreaHeight});
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
HWTEST_F(SceneSessionTest, SetTurnScreenOn01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetTurnScreenOn02, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, UpdateWindowAnimationFlag01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, UpdateWindowAnimationFlag02, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetRequestedOrientation01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, GetRequestedOrientation, Function | SmallTest | Level2)
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
 * @tc.name: SetDefaultRequestedOrientation
 * @tc.desc: SetDefaultRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetDefaultRequestedOrientation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetDefaultRequestedOrientation";
    info.bundleName_ = "SetDefaultRequestedOrientation";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    Orientation orientation = Orientation::AUTO_ROTATION_UNSPECIFIED;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    Orientation ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_PORTRAIT;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_LANDSCAPE;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_PORTRAIT_INVERTED;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_LANDSCAPE_INVERTED;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::FOLLOW_DESKTOP;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);
}

/**
 * @tc.name: IsKeepScreenOn01
 * @tc.desc: IsKeepScreenOn true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsKeepScreenOn01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, IsKeepScreenOn02, Function | SmallTest | Level2)
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
 * @tc.name: IsAppSession
 * @tc.desc: IsAppSession true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsAppSession01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsAppSession01";
    info.bundleName_ = "IsAppSession01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(true, sceneSession->IsAppSession());
}

/**
 * @tc.name: IsAppSession
 * @tc.desc: IsAppSession false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsAppSession02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsAppSession02";
    info.bundleName_ = "IsAppSession02";
    info.windowType_ = 2106;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(false, sceneSession->IsAppSession());

    SessionInfo parentInfo;
    parentInfo.abilityName_ = "testSession1";
    parentInfo.moduleName_ = "testSession2";
    parentInfo.bundleName_ = "testSession3";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(parentInfo);
    ASSERT_NE(parentSession, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    parentSession->SetSessionProperty(property);
    sceneSession->SetParentSession(parentSession);
    ASSERT_EQ(false, sceneSession->IsAppSession());

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentSession->SetSessionProperty(property);
    sceneSession->SetParentSession(parentSession);
    ASSERT_EQ(true, sceneSession->IsAppSession());
}

/**
 * @tc.name: IsAppOrLowerSystemSession
 * @tc.desc: IsAppOrLowerSystemSession true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsAppOrLowerSystemSession01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsAppOrLowerSystemSession01";
    info.windowType_ = 2126;

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(true, sceneSession->IsAppOrLowerSystemSession());
}

/**
 * @tc.name: IsAppOrLowerSystemSession
 * @tc.desc: IsAppOrLowerSystemSession false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsAppOrLowerSystemSession02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "Background02";
    info.bundleName_ = "IsAppOrLowerSystemSession02";
    info.windowType_ = 2106;

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(false, sceneSession->IsAppOrLowerSystemSession());

    SessionInfo parentInfo;
    parentInfo.abilityName_ = "testSession1";
    parentInfo.moduleName_ = "testSession2";
    parentInfo.bundleName_ = "testSession3";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(parentInfo);
    ASSERT_NE(parentSession, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    parentSession->SetSessionProperty(property);
    sceneSession->SetParentSession(parentSession);
    ASSERT_EQ(false, sceneSession->IsAppOrLowerSystemSession());

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentSession->SetSessionProperty(property);
    sceneSession->SetParentSession(parentSession);
    ASSERT_EQ(true, sceneSession->IsAppOrLowerSystemSession());
}

/**
 * @tc.name: IsSystemSessionAboveApp
 * @tc.desc: IsSystemSessionAboveApp true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsSystemSessionAboveApp01, Function | SmallTest | Level2)
{
    SessionInfo info1;
    info1.abilityName_ = "HighZOrder01";
    info1.bundleName_ = "IsSystemSessionAboveApp01";
    info1.windowType_ = 2122;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_EQ(true, sceneSession1->IsSystemSessionAboveApp());

    SessionInfo info2;
    info2.abilityName_ = "HighZOrder02";
    info2.bundleName_ = "IsSystemSessionAboveApp02";
    info2.windowType_ = 2104;

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_EQ(true, sceneSession2->IsSystemSessionAboveApp());

    SessionInfo info3;
    info3.abilityName_ = "HighZOrder03";
    info3.bundleName_ = "SCBDropdownPanel13";
    info3.windowType_ = 2109;

    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(info3, nullptr);
    ASSERT_EQ(true, sceneSession3->IsSystemSessionAboveApp());

    SessionInfo info4;
    info4.abilityName_ = "HighZOrder04";
    info4.bundleName_ = "IsSystemSessionAboveApp04";
    info4.windowType_ = 2109;

    sptr<SceneSession> sceneSession4 = sptr<SceneSession>::MakeSptr(info4, nullptr);
    ASSERT_EQ(false, sceneSession4->IsSystemSessionAboveApp());

    SessionInfo info5;
    info5.abilityName_ = "HighZOrderGestureDock";
    info5.bundleName_ = "SCBGestureDock21";
    info5.windowType_ = 2106;

    sptr<SceneSession> sceneSession5 = sptr<SceneSession>::MakeSptr(info5, nullptr);
    ASSERT_EQ(true, sceneSession5->IsSystemSessionAboveApp());
}

/**
 * @tc.name: IsSystemSessionAboveApp
 * @tc.desc: IsSystemSessionAboveApp false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsSystemSessionAboveApp02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HighZOrder05";
    info.bundleName_ = "IsSystemSessionAboveApp05";
    info.windowType_ = 1;

    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_EQ(false, sceneSession->IsSystemSessionAboveApp());
}

/**
 * @tc.name: GetWindowName
 * @tc.desc: GetWindowName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetWindowName, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, IsDecorEnable, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, IsDecorEnable01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, UpdateNativeVisibility, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetPrivacyMode01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetPrivacyMode01";
    info.bundleName_ = "SetPrivacyMode01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->surfaceNode_ = surfaceNode;
    sceneSession->SetPrivacyMode(false);
    ASSERT_EQ(false, sceneSession->property_->GetPrivacyMode());
    ASSERT_EQ(false, sceneSession->property_->GetSystemPrivacyMode());
}

/**
 * @tc.name: SetPrivacyMode02
 * @tc.desc: Set PrivacyMode as true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetPrivacyMode02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetPrivacyMode02";
    info.bundleName_ = "SetPrivacyMode";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->surfaceNode_ = surfaceNode;
    sceneSession->SetPrivacyMode(true);
    ASSERT_EQ(true, sceneSession->property_->GetPrivacyMode());
    ASSERT_EQ(true, sceneSession->property_->GetSystemPrivacyMode());
}

/**
 * @tc.name: IsFloatingWindowAppType01
 * @tc.desc: IsFloatingWindowAppType true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, IsFloatingWindowAppType01, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, IsFloatingWindowAppType02, Function | SmallTest | Level2)
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
 * @tc.name: DumpSessionElementInfo01
 * @tc.desc: DumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, DumpSessionElementInfo, Function | SmallTest | Level2)
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
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    std::vector<std::string> params;
    sceneSession->DumpSessionElementInfo(params);
    int ret = 1;
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->DumpSessionElementInfo(params);
    ASSERT_EQ(ret, 1);
}

/**
 * @tc.name: NotifyIsCustomAnimationPlaying
 * @tc.desc: NotifyIsCustomAnimationPlaying
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, NotifyIsCustomAnimationPlaying, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, ModalUIExtension, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, NotifySessionRectChange, Function | SmallTest | Level2)
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
        SizeChangeReason reason, DisplayId displayId, const RectAnimationConfig& rectAnimationConfig) {
        return;
    };
    sceneSession->NotifySessionRectChange(overlapRect, SizeChangeReason::ROTATION, -1);
    sceneSession->NotifySessionRectChange(overlapRect, SizeChangeReason::ROTATION, 11);
}

/**
 * @tc.name: GetKeyboardAvoidArea
 * @tc.desc: GetKeyboardAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetKeyboardAvoidArea, Function | SmallTest | Level2)
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
    WSRect overlapRect = {0, 0, 0, 0};
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
HWTEST_F(SceneSessionTest, GetCutoutAvoidArea, Function | SmallTest | Level2)
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
    int ret = 1;
    sceneSession->GetCutoutAvoidArea(overlapRect, avoidArea);
    ASSERT_EQ(ret, 1);
}

/**
 * @tc.name: SetSystemBarProperty
 * @tc.desc: SetSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetSystemBarProperty, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetSystemBarProperty02, Function | SmallTest | Level2)
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
 * @tc.name: OnShowWhenLocked
 * @tc.desc: OnShowWhenLocked
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionTest, OnShowWhenLocked, Function | SmallTest | Level2)
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
    int ret = 0;
    sceneSession->OnShowWhenLocked(false);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: IsShowWhenLocked
 * @tc.desc: IsShowWhenLocked
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionTest, IsShowWhenLocked, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, GetAvoidAreaByType, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
        specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = [](WindowType type,
            uint64_t displayId)-> std::vector<sptr<SceneSession>>
    {
        SessionInfo info_;
        info_.abilityName_ = "Background01";
        info_.bundleName_ = "IsFloatingWindowAppType";
        std::vector<sptr<SceneSession>> backgroundSession;
        sptr<SceneSession> session2= sptr<SceneSession>::MakeSptr(info_, nullptr);
        backgroundSession.push_back(session2);
        return backgroundSession;
    };
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 320, 240}; // width: 320, height: 240
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
 * @tc.name: TransferPointerEvent
 * @tc.desc: TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, TransferPointerEvent, Function | SmallTest | Level2)
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
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent), WSError::WS_OK);
    std::shared_ptr<MMI::PointerEvent> pointerEvent_ = MMI::PointerEvent::Create();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->SetPersistentId(11);
    sceneSession->property_ = property;
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);
}

/**
 * @tc.name: TransferPointerEventDecorDialog
 * @tc.desc: TransferPointerEventDecorDialog
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, TransferPointerEventDecorDialog, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "TransferPointerEventDecorDialog";
    info.bundleName_ = "TransferPointerEventDecorDialogbundle";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession =
        sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(12, WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    std::shared_ptr<MMI::PointerEvent> pointerEvent_ =  MMI::PointerEvent::Create();
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
 * @tc.name: ProcessWindowMoving
 * @tc.desc: ProcessWindowMoving
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, ProcessWindowMoving, Function | SmallTest | Level2)
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
 * @tc.name: CalculateAvoidAreaRect
 * @tc.desc: CalculateAvoidAreaRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, CalculateAvoidAreaRect, Function | SmallTest | Level2)
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
    int ret = 0;
    WSRect overlapRect = { 0, 0, 0, 0 };
    WSRect avoidRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->CalculateAvoidAreaRect(overlapRect, avoidRect, avoidArea);
    WSRect overlapRect_ = { 1, 1, 1, 1 };
    WSRect avoidRect_ = { 1, 1, 1, 1 };
    sceneSession->CalculateAvoidAreaRect(overlapRect_, avoidRect_, avoidArea);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: OnNeedAvoid
 * @tc.desc: OnNeedAvoid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, OnNeedAvoid, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetCollaboratorType, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, GetAbilityInfo, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, UpdateCameraWindowStatus, Function | SmallTest | Level2)
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
    int ret = 1;
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
    ASSERT_EQ(ret, 1);
}

/**
 * @tc.name: GetRatioPreferenceKey
 * @tc.desc: GetRatioPreferenceKey
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetRatioPreferenceKey, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, NotifyPropertyWhenConnect, Function | SmallTest | Level2)
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
    int ret = 1;
    std::string key = info.bundleName_ + info.moduleName_ + info.abilityName_;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sceneSession->NotifyPropertyWhenConnect();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_ = property;
    sceneSession->NotifyPropertyWhenConnect();
    ASSERT_EQ(ret, 1);
}

/**
 * @tc.name: DumpSessionInfo
 * @tc.desc: DumpSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, DumpSessionInfo, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.bundleName_ = "SceneSessionTest";
    info.abilityName_ = "DumpSessionInfo";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    std::vector<std::string> infos;
    sceneSession->DumpSessionInfo(infos);
    ASSERT_FALSE(infos.empty());
}

/**
 * @tc.name: CalcRectForStatusBar
 * @tc.desc: CalcRectForStatusBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, CalcRectForStatusBar, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "CalcRectForStatusBar";
    info.bundleName_ = "CalcRectForStatusBar";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint32_t width = sceneSession->CalcRectForStatusBar().width_;
    uint32_t height = sceneSession->CalcRectForStatusBar().height_;
    EXPECT_EQ(width, 0);
    EXPECT_EQ(height, 0);
}

/**
 * @tc.name: InitializeMoveInputBar
 * @tc.desc: InitializeMoveInputBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, InitializeMoveInputBar, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, OnSessionEvent, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnSessionEvent";
    info.bundleName_ = "OnSessionEvent";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(1, WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
    sceneSession->moveDragController_->isStartDrag_ = true;
    sceneSession->moveDragController_->hasPointDown_ = true;
    ASSERT_EQ(sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE), WSError::WS_OK);
    ASSERT_EQ(sceneSession->OnSessionEvent(SessionEvent::EVENT_END_MOVE), WSError::WS_OK);
}

/**
 * @tc.name: SyncSessionEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SyncSessionEvent, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetTopmost, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetMainWindowTopmost, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, UpdateSessionRectPosYFromClient01, Function | SmallTest | Level2)
{
    DisplayId displayId = 0;
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRectPosYFromClient";
    info.bundleName_ = "UpdateSessionRectPosYFromClient";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->sessionInfo_.screenId_ = 0;
    EXPECT_EQ(sceneSession->GetScreenId(), 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::EXPANDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    WSRect rect = {0, 0, 0, 0};
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_EQ(rect.posY_, 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::KEYBOARD,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    rect = {0, 100, 0, 0};
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_EQ(rect.posY_, 100);

    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::HALF_FOLDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    sceneSession->clientDisplayId_ = 0;
    rect = {0, 100, 100, 100};
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_EQ(rect.posY_, 100);
    sceneSession->clientDisplayId_ = 999;
    rect = {0, 100, 100, 100};
    auto rect2 = rect;
    sceneSession->UpdateSessionRectPosYFromClient(SizeChangeReason::UNDEFINED, displayId, rect);
    EXPECT_NE(rect.posY_, rect2.posY_);
}

/**
 * @tc.name: UpdateSessionRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect, Function | SmallTest | Level2)
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
    WSRect rect({1, 1, 1, 1});
    SizeChangeReason reason = SizeChangeReason::MOVE;
    WSError result = sceneSession->UpdateSessionRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect1, Function | SmallTest | Level2)
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
    WSRect rect({1, 1, 1, 1});
    SizeChangeReason reason = SizeChangeReason::RESIZE;
    WSError result = sceneSession->UpdateSessionRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect2, Function | SmallTest | Level2)
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
    WSRect rect({1, 1, 1, 1});
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError result = sceneSession->UpdateSessionRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect3
 * @tc.desc: test for isGlobal is true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, UpdateSessionRect3, Function | SmallTest | Level2)
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
    WSRect oldRect({1, 1, 1, 1});
    WSRect parentRect({10, 10, 1, 1});

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
 * @tc.name: GetStatusBarHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetStatusBarHeight, Function | SmallTest | Level1)
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
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR,
        propertyHide));
    ASSERT_EQ(height, 0);
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    height = sceneSession->GetStatusBarHeight();
    ASSERT_EQ(height, 0);
    WSRect rect({0, 0, 0, 1});
    sceneSession->winRect_ = rect;
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = [&](WindowType type,
        uint64_t displayId)->std::vector<sptr<SceneSession>>
    {
        std::vector<sptr<SceneSession>> vec;
        vec.push_back(sceneSession);
        return vec;
    };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    sceneSession->property_ = property;
    height = sceneSession->GetStatusBarHeight();
    ASSERT_EQ(height, 1);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR,
        propertyHide));
    ASSERT_EQ(height, 1);
}

/**
 * @tc.name: GetDockHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetDockHeight, Function | SmallTest | Level1)
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
    WSRect rect({0, 0, 0, 112});
    sceneSession->winRect_ = rect;
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = [&](WindowType type,
        uint64_t displayId)->std::vector<sptr<SceneSession>>
    {
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
HWTEST_F(SceneSessionTest, GetAppForceLandscapeConfig, Function | SmallTest | Level2)
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
 * @tc.name: HandleCompatibleModeMoveDrag
 * @tc.desc: HandleCompatibleModeMoveDrag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, HandleCompatibleModeMoveDrag, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleCompatibleModeMoveDrag";
    info.bundleName_ = "HandleCompatibleModeMoveDrag";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(12, WindowType::WINDOW_TYPE_FLOAT);

    WSRect rect = {1, 1, 1, 1};
    WSRect rect2 = {2, 2, 2, 1};
    sceneSession->winRect_ = rect2;
    sceneSession->moveDragController_->moveDragProperty_.originalRect_ = rect;
    sceneSession->HandleCompatibleModeMoveDrag(rect2, SizeChangeReason::HIDE);
    WSRect rect3 = {1, 1, 2, 1};
    ASSERT_NE(rect2, rect3);
    ASSERT_EQ(rect2.posX_, 2);
    ASSERT_EQ(rect2.posY_, 2);

    sceneSession->HandleCompatibleModeMoveDrag(rect2, SizeChangeReason::DRAG_MOVE);
    ASSERT_NE(rect2, rect3);
    ASSERT_EQ(rect2.posX_, 2);
    ASSERT_EQ(rect2.posY_, 2);
}

/**
 * @tc.name: HandleCompatibleModeDrag
 * @tc.desc: HandleCompatibleModeDrag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, HandleCompatibleModeDrag, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleCompatibleModeDrag";
    info.bundleName_ = "HandleCompatibleModeDrag";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(12, WindowType::WINDOW_TYPE_FLOAT);

    WSRect rect = {1, 1, 1, 1};
    WSRect rect2 = {2, 1, 1, 1};
    sceneSession->winRect_ = rect2;
    sceneSession->HandleCompatibleModeDrag(rect, SizeChangeReason::DRAG_MOVE, false);
    ASSERT_EQ(sceneSession->winRect_, rect2);

    rect2 = {1, 2, 1, 1};
    sceneSession->winRect_ = rect2;
    sceneSession->HandleCompatibleModeDrag(rect, SizeChangeReason::DRAG_MOVE, false);
    ASSERT_EQ(sceneSession->winRect_, rect2);

    rect2 = {1, 1, 2, 1};
    sceneSession->winRect_ = rect2;
    sceneSession->HandleCompatibleModeDrag(rect, SizeChangeReason::DRAG_MOVE, false);
    ASSERT_EQ(sceneSession->winRect_, rect2);

    rect2 = {1, 1, 1, 2};
    sceneSession->winRect_ = rect2;
    sceneSession->HandleCompatibleModeDrag(rect, SizeChangeReason::DRAG_MOVE, false);
    ASSERT_EQ(sceneSession->winRect_, rect2);
}

/**
 * @tc.name: SetMoveDragCallback
 * @tc.desc: SetMoveDragCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetMoveDragCallback, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetMoveDragCallback";
    info.bundleName_ = "SetMoveDragCallback";
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->moveDragController_ = nullptr;
    sceneSession->SetMoveDragCallback();
}

/**
 * @tc.name: GetScreenWidthAndHeightFromServer
 * @tc.desc: GetScreenWidthAndHeightFromServer
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, GetScreenWidthAndHeightFromServer, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetScreenWidthAndHeightFromServer";
    info.bundleName_ = "GetScreenWidthAndHeightFromServer";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    sceneSession->SetSessionProperty(property);

    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    bool result = sceneSession->GetScreenWidthAndHeightFromServer(property, screenWidth, screenHeight);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: SetDefaultDisplayIdIfNeed
 * @tc.desc: SetDefaultDisplayIdIfNeed
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest, SetDefaultDisplayIdIfNeed, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetSessionGlobalRect, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetIsStatusBarVisibleInner01, Function | SmallTest | Level2)
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

    sceneSession->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        return WSError::WS_ERROR_NULLPTR;
    };
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
HWTEST_F(SceneSessionTest, SetMousePointerDownEventStatus, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetFingerPointerDownStatus, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, SetUIFirstSwitch, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionTest, IsFollowParentMultiScreenPolicy, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_EQ(sceneSession->IsFollowParentMultiScreenPolicy(), false);
}
} // namespace
} // Rosen
} // OHOS