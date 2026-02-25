/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "ability_context_impl.h"
#include "common_test_utils.h"
#include "extension_data_handler.h"
#include "iremote_object_mocker.h"
#include "mock_ability_context_impl.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "parameters.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
uint32_t MaxWith = 32;
constexpr uint32_t NUMBER_ONE = 1;
constexpr uint32_t NUMBER_TWO = 2;

class TestAnimationTransitionController : public IAnimationTransitionController {
public:
    uint32_t testCount_ = 0;

    void AnimationForShown() override;
    void AnimationForHidden() override;
};

void TestAnimationTransitionController::AnimationForShown()
{
    testCount_ = NUMBER_ONE;
}

void TestAnimationTransitionController::AnimationForHidden()
{
    testCount_ = NUMBER_TWO;
}

class WindowSceneSessionImplAnimationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneSessionImplAnimationTest::SetUpTestCase() {}

void WindowSceneSessionImplAnimationTest::TearDownTestCase() {}

void WindowSceneSessionImplAnimationTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplAnimationTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplAnimationTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: SetShadowRadius01
 * @tc.desc: SetShadowRadius01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetShadowRadius01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetShadowRadius01");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetShadowRadius(-1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowRadius(0.0));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowRadius(1.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetShadowRadius(1.0));
}

/**
 * @tc.name: SetShadowRadius02
 * @tc.desc: SetShadowRadius02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetShadowRadius02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetShadowRadius02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->surfaceNode_ = nullptr;
    auto ret = windowSceneSessionImpl->SetShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    ret = windowSceneSessionImpl->SetShadowOffsetY(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    Transform trans;
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->SetTransform(trans);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    ret = windowSceneSessionImpl->SetTransform(trans);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetShadowColor
 * @tc.desc: SetShadowColor test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetShadowColor, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetShadowColor");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetShadowColor("111ff22ee44"));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowColor("#ff22ee44"));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowColor("#000999"));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetShadowColor("#ff22ee44"));
}

/**
 * @tc.name: SetShadowOffsetX01
 * @tc.desc: SetShadowOffsetX01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetShadowOffsetX01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetShadowOffsetX01");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_OK, window->SetShadowOffsetX(1.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetShadowOffsetX(1.0));
}

/**
 * @tc.name: SetShadowOffsetX02
 * @tc.desc: SetShadowOffsetX02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetShadowOffsetX02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetShadowOffsetX02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto ret = windowSceneSessionImpl->SetShadowOffsetX(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
    std::string color = "#ff22ee44";
    ret = windowSceneSessionImpl->SetShadowColor(color);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetShadowOffsetY
 * @tc.desc: SetShadowOffsetY test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetShadowOffsetY, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetShadowOffsetY");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_OK, window->SetShadowOffsetY(1.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetShadowOffsetY(1.0));
}

/**
 * @tc.name: SetWindowShadowRadius
 * @tc.desc: SetWindowShadowRadius
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetWindowShadowRadius, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowShadowRadius");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowShadowRadius(0.0f));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);

    window->surfaceNode_ = nullptr;
    auto ret = window->SetWindowShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window->SetWindowShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = window->SetWindowShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    auto surfaceNodeMocker = CreateRSSurfaceNode();
    ASSERT_NE(nullptr, surfaceNodeMocker);
    window->surfaceNode_ = surfaceNodeMocker;
    ret = window->SetWindowShadowRadius(-1.0f);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = window->SetWindowShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
    
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->SetWindowShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);

    const std::string feature = "large_screen";
    std::string deviceType = OHOS::system::GetParameter("const.product.devicetype", "");
    auto context = std::make_shared<MockAbilityContextImpl>();
    window->context_ = context;
    context->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    context->hapModuleInfo_->requiredDeviceFeatures = {{deviceType, {feature}}};
    ret = window->SetWindowShadowRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetCornerRadius
 * @tc.desc: SetCornerRadius test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetCornerRadius, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetCornerRadius");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_OK, window->SetCornerRadius(1.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetCornerRadius(1.0));
}

/**
 * @tc.name: SetWindowCornerRadius
 * @tc.desc: SetWindowCornerRadius
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetWindowCornerRadius, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowCornerRadius");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowCornerRadius(0.0f));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    auto ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->SetWindowCornerRadius(-1.0f);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetWindowCornerRadius
 * @tc.desc: GetWindowCornerRadius
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, GetWindowCornerRadius, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowCornerRadius");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    float cornerRadius = 0.0f;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetWindowCornerRadius(cornerRadius));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    window->property_->SetWindowCornerRadius(WINDOW_CORNER_RADIUS_INVALID);
    window->windowSystemConfig_.defaultCornerRadius_ = WINDOW_CORNER_RADIUS_INVALID;
    auto ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(0.0f, cornerRadius);

    window->windowSystemConfig_.defaultCornerRadius_ = 1.0f; // 1.0f is valid default window corner radius
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(window->windowSystemConfig_.defaultCornerRadius_, cornerRadius);

    window->property_->SetWindowCornerRadius(2.0f); // 2.0f is valid window corner radius
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(2.0f, cornerRadius); // 2.0f is valid window corner radius

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(2.0f, cornerRadius); // 2.0f is valid window corner radius

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(2.0f, cornerRadius); // 2.0f is valid window corner radius

    ret = window->SetWindowCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(1.0f, cornerRadius); //1.0f is valid window corner radius

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    
    ret = window->SetWindowCornerRadius(1.5f);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(1.5f, cornerRadius);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_OK, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window->GetWindowCornerRadius(cornerRadius);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
}

/**
 * @tc.name: SetBlur01
 * @tc.desc: SetBlur01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetBlur01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetBlur01");

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetBlur(-1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetBlur(1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetBlur(0.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetBlur(1.0));
}

/**
 * @tc.name: SetBlur02
 * @tc.desc: SetBlur02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetBlur02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetBlur02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->surfaceNode_ = nullptr;
    auto ret = windowSceneSessionImpl->SetBlur(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    ret = windowSceneSessionImpl->SetBackdropBlur(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    ret = windowSceneSessionImpl->SetBackdropBlurStyle(WindowBlurStyle::WINDOW_BLUR_OFF);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: SetBackdropBlur
 * @tc.desc: SetBackdropBlur test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetBackdropBlur, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetBackdropBlur");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetBackdropBlur(-1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetBackdropBlur(1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetBackdropBlur(0.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetBackdropBlur(1.0));
}

/**
 * @tc.name: SetBackdropBlurStyle
 * @tc.desc: SetBackdropBlurStyle test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetBackdropBlurStyle, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetBackdropBlurStyle");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    window->property_->SetDisplayId(3);

    ASSERT_EQ(WMError::WM_OK, window->SetBackdropBlurStyle(WindowBlurStyle::WINDOW_BLUR_OFF));
    window->Destroy(true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetBackdropBlurStyle(WindowBlurStyle::WINDOW_BLUR_THICK));
    window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetBackdropBlurStyle(WindowBlurStyle::WINDOW_BLUR_OFF));
}

/**
 * @tc.name: SetAlpha01
 * @tc.desc: SetAlpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetAlpha01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAlpha01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(11);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->SetAlpha(1.0));
}

/**
 * @tc.name: SetAlpha02
 * @tc.desc: SetAlpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetAlpha02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAlpha02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestMode", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->SetAlpha(1.0));
}

/**
 * @tc.name: SetTransform01
 * @tc.desc: set transform
 * @tc.type: FUNC
 * @tc.require:issueI7IJVV
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetTransform01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    Transform trans_;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTransform(trans_));
}

/**
 * @tc.name: SetTransform02
 * @tc.desc: set transform
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SetTransform02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    Transform trans_;
    ASSERT_EQ(WMError::WM_OK, window->SetTransform(trans_));
    ASSERT_EQ(trans_, window->GetTransform());
}

/**
 * @tc.name: SyncShadowsToComponent
 * @tc.desc: SyncShadowsToComponent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, SyncShadowsToComponent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SyncShadowsToComponent");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    ShadowsInfo shadowsInfo = { 20.0, "#FF0000", 0.0, 0.0, true, true, true, true };
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SyncShadowsToComponent(shadowsInfo));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    auto ret = window->SyncShadowsToComponent(shadowsInfo);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: CustomHideAnimation
 * @tc.desc: CustomHideAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, CustomHideAnimation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);

    auto testController = sptr<TestAnimationTransitionController>::MakeSptr();
    window->property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));

    int result = 0;
    window->CustomHideAnimation();
    ASSERT_EQ(testController->testCount_, result);
}

/**
 * @tc.name: CustomHideAnimation_CustomFlag
 * @tc.desc: CustomHideAnimation_CustomFlag test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, CustomHideAnimation_CustomFlag, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);

    auto testController = sptr<TestAnimationTransitionController>::MakeSptr();
    window->property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));

    window->animationTransitionControllers_.push_back(testController);
    int result = NUMBER_TWO;
    window->CustomHideAnimation();
    ASSERT_EQ(testController->testCount_, result);
}

/**
 * @tc.name: CustomHideAnimation_DefaultFlag
 * @tc.desc: CustomHideAnimation_DefaultFlag test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplAnimationTest, CustomHideAnimation_DefaultFlag, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::DEFAULT));

    auto testController = sptr<TestAnimationTransitionController>::MakeSptr();
    window->animationTransitionControllers_.push_back(testController);

    int result = 0;
    window->CustomHideAnimation();
    ASSERT_EQ(testController->testCount_, result);
}
} // namespace
} // namespace Rosen
} // namespace OHOS