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

#include "session/host/include/scene_session.h"
#include "session/host/include/session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class LayoutControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<LayoutController> layoutController_ = nullptr;
};

void LayoutControllerTest::SetUpTestCase() {}

void LayoutControllerTest::TearDownTestCase() {}

void LayoutControllerTest::SetUp()
{
    sptr<WindowSessionProperty> sessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    layoutController_ = sptr<LayoutController>::MakeSptr(sessionProperty);
}

void LayoutControllerTest::TearDown()
{
    layoutController_ = nullptr;
}

namespace {
/**
 * @tc.name: SetSessionGlobalRect
 * @tc.desc: SetSessionGlobalRect
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, SetSessionGlobalRect, TestSize.Level1)
{
    WSRect rect = { 500, 500, 800, 800 };
    layoutController_->globalRect_ = rect;
    EXPECT_EQ(layoutController_->SetSessionGlobalRect(rect), false);
    EXPECT_EQ(layoutController_->globalRect_, rect);

    layoutController_->globalRect_ = { 500, 500, 1000, 1000 };
    EXPECT_EQ(layoutController_->SetSessionGlobalRect(rect), true);
    EXPECT_EQ(layoutController_->globalRect_, rect);
}

/**
 * @tc.name: GetSessionGlobalRect
 * @tc.desc: GetSessionGlobalRect
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, GetSessionGlobalRect, TestSize.Level1)
{
    WSRect globalRect = { 500, 500, 800, 800 };
    WSRect winRect = { 600, 600, 800, 800 };
    layoutController_->globalRect_ = globalRect;
    layoutController_->winRect_ = winRect;

    Session::SetScbCoreEnabled(true);
    EXPECT_EQ(layoutController_->GetSessionGlobalRect(), globalRect);

    Session::SetScbCoreEnabled(false);
    EXPECT_EQ(layoutController_->GetSessionGlobalRect(), winRect);
}

/**
 * @tc.name: SetClientRect
 * @tc.desc: SetClientRect
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, SetClientRect, TestSize.Level1)
{
    WSRect clientRect = { 500, 500, 800, 800 };
    layoutController_->clientRect_ = clientRect;
    layoutController_->SetClientRect(clientRect);
    EXPECT_EQ(layoutController_->GetClientRect(), clientRect);

    WSRect clientRect2 = { 500, 500, 1000, 1000 };
    layoutController_->SetClientRect(clientRect2);
    EXPECT_NE(layoutController_->GetClientRect(), clientRect);
}

/**
 * @tc.name: ConvertRelativeRectToGlobal
 * @tc.desc: ConvertRelativeRectToGlobal
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, ConvertRelativeRectToGlobal, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertRelativeRectToGlobal";
    info.bundleName_ = "ConvertRelativeRectToGlobal";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    DisplayId defaultDisplayId = 0;
    DisplayId invalidDisplayId = -1ULL;
    WSRect relativeRect = { 500, 500, 800, 800 };
    sceneSession->GetSessionProperty()->SetDisplayId(invalidDisplayId);
    WSRect retRect = sceneSession->GetLayoutController()->ConvertRelativeRectToGlobal(relativeRect, invalidDisplayId);
    EXPECT_EQ(retRect, relativeRect);

    sceneSession->GetSessionProperty()->SetDisplayId(defaultDisplayId);
    retRect = sceneSession->GetLayoutController()->ConvertRelativeRectToGlobal(relativeRect, invalidDisplayId);
    EXPECT_EQ(retRect, relativeRect);
    retRect = sceneSession->GetLayoutController()->ConvertRelativeRectToGlobal(relativeRect, defaultDisplayId);
    EXPECT_EQ(retRect, relativeRect);
}

/**
 * @tc.name: ConvertGlobalRectToRelative
 * @tc.desc: ConvertGlobalRectToRelative
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, ConvertGlobalRectToRelative, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertGlobalRectToRelative";
    info.bundleName_ = "ConvertGlobalRectToRelative";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    DisplayId defaultDisplayId = 0;
    DisplayId invalidDisplayId = -1ULL;
    WSRect globalRect = { 500, 500, 800, 800 };
    sceneSession->GetSessionProperty()->SetDisplayId(invalidDisplayId);
    WSRect retRect = sceneSession->GetLayoutController()->ConvertGlobalRectToRelative(globalRect, invalidDisplayId);
    EXPECT_EQ(retRect, globalRect);

    sceneSession->GetSessionProperty()->SetDisplayId(defaultDisplayId);
    retRect = sceneSession->GetLayoutController()->ConvertGlobalRectToRelative(globalRect, invalidDisplayId);
    EXPECT_EQ(retRect, globalRect);
    retRect = sceneSession->GetLayoutController()->ConvertGlobalRectToRelative(globalRect, defaultDisplayId);
    EXPECT_EQ(retRect, globalRect);
}

/**
 * @tc.name: AdjustRectByAspectRatio
 * @tc.desc: AdjustRectByAspectRatio function01
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, AdjustRectByAspectRatio, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "AdjustRectByAspectRatio";
    info.bundleName_ = "AdjustRectByAspectRatio";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->GetLayoutController()->SetSystemConfigFunc([session] {
        return session->GetSystemConfig();
    });
    WSRect rect;
    EXPECT_EQ(false, session->GetLayoutController()->AdjustRectByAspectRatio(rect, false));
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(false, session->GetLayoutController()->AdjustRectByAspectRatio(rect, false));
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    EXPECT_EQ(false, session->GetLayoutController()->AdjustRectByAspectRatio(rect, false));
    session->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_EQ(false, session->GetLayoutController()->AdjustRectByAspectRatio(rect, false));
}

/**
 * @tc.name: AdjustRectByAspectRatio01
 * @tc.desc: AdjustRectByAspectRatio function01
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, AdjustRectByAspectRatio01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "AdjustRectByAspectRatio01";
    info.bundleName_ = "AdjustRectByAspectRatio01";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect;
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    session->Session::SetAspectRatio(0.5f);
    EXPECT_NE(nullptr, DisplayManager::GetInstance().GetDefaultDisplay());

    SystemSessionConfig systemConfig;
    systemConfig.isSystemDecorEnable_ = true;
    systemConfig.decorWindowModeSupportType_ = 2;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(true, session->GetLayoutController()->AdjustRectByAspectRatio(rect, true));

    systemConfig.isSystemDecorEnable_ = false;
    EXPECT_EQ(true, session->GetLayoutController()->AdjustRectByAspectRatio(rect, false));
}

/**
 * @tc.name: SetSystemConfigFunc
 * @tc.desc: SetSystemConfigFunc
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, SetSystemConfigFunc, TestSize.Level1)
{
    WSRect rect = { 500, 500, 800, 800 };
    layoutController_->SetSessionRect({ 1, 1, 1, 1 });
    layoutController_->SetSystemConfigFunc(nullptr);
    layoutController_->SetSystemConfigFunc([layoutController = this->layoutController_, rect] {
        layoutController->SetSessionRect(rect);
        SystemSessionConfig systemConfig;
        return systemConfig;
    });
    layoutController_->getSystemConfigFunc_();
    EXPECT_EQ(layoutController_->GetSessionRect(), rect);
}

/**
 * @tc.name: TestAdjustRectByAspectRatioAbnormalCases
 * @tc.desc: Verify AdjustRectByAspectRatio handles abnormal cases correctly
 * @tc.type: FUNC
 */
HWTEST_F(LayoutControllerTest, TestAdjustRectByAspectRatioAbnormalCases, TestSize.Level1)
{
    WSRect rect{10, 20, 200, 200};
    WindowDecoration decor{0, 0, 0, 0};

    // Case 1: sessionProperty is null
    layoutController_->sessionProperty_ = nullptr;
    EXPECT_EQ(layoutController_->AdjustRectByAspectRatio(rect, decor), rect);

    auto prop = sptr<WindowSessionProperty>::MakeSptr();
    layoutController_->sessionProperty_ = prop;

    // Case 2: mode != FLOATING
    prop->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(layoutController_->AdjustRectByAspectRatio(rect, decor), rect);

    // Case 3: Not main window
    prop->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    prop->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    EXPECT_EQ(layoutController_->AdjustRectByAspectRatio(rect, decor), rect);

    prop->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Case 4: display is null
    prop->SetDisplayId(DISPLAY_ID_INVALID);
    EXPECT_EQ(layoutController_->AdjustRectByAspectRatio(rect, decor), rect);

    // Case 5: Success
    DisplayManager::GetInstance().GetDisplayByScreen(0);
    layoutController_->aspectRatio_ = 2.0f;
    prop->SetDisplayId(0);
    layoutController_->SetSystemConfigFunc([] {
        SystemSessionConfig systemConfig;
        return systemConfig;
    });
    EXPECT_NE(layoutController_->AdjustRectByAspectRatio(rect, decor), rect);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
