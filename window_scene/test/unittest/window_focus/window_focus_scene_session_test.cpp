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
#include "../../../../interfaces/innerkits/dm/display_manager.h"
#include "../../../../utils/include/display_info.h"
#include "session/host/include/scene_session.h"
#include "window_helper.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class WindowFocusSceneSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowFocusSceneSessionTest::SetUpTestCase() {}

void WindowFocusSceneSessionTest::TearDownTestCase() {}

void WindowFocusSceneSessionTest::SetUp() {}

void WindowFocusSceneSessionTest::TearDown() {}

namespace {

/**
 * @tc.name: IsAppSession
 * @tc.desc: IsAppSession true
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusSceneSessionTest, IsAppSession01, TestSize.Level1)
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
HWTEST_F(WindowFocusSceneSessionTest, IsAppSession02, TestSize.Level1)
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
HWTEST_F(WindowFocusSceneSessionTest, IsAppOrLowerSystemSession01, TestSize.Level1)
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
HWTEST_F(WindowFocusSceneSessionTest, IsAppOrLowerSystemSession02, TestSize.Level1)
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
HWTEST_F(WindowFocusSceneSessionTest, IsSystemSessionAboveApp01, TestSize.Level1)
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
HWTEST_F(WindowFocusSceneSessionTest, IsSystemSessionAboveApp02, TestSize.Level1)
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
} // namespace
} // namespace Rosen
} // namespace OHOS