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

#include "ability_context_impl.h"

#include "mock_session.h"
#include "window_scene_session_impl.h"
#include "window_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Utils = WindowTestUtils;

class WindowSpecialWindowTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t TEST_SLEEP_S = 1; // test sleep time
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

void WindowSpecialWindowTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSpecialWindowTest::TearDown()
{
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: SetSubWindowModal01
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10011);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(true));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    EXPECT_EQ(WMError::WM_OK, window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal02
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10012);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal04
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_4");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10014);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal05
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal05, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_5");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10015);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(false, ModalityType::WINDOW_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal06
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal06, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_6");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10016);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(false, ModalityType::WINDOW_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal07
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal07, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_7");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10017);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(false, ModalityType::WINDOW_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal08
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal08, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_8");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10018);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WMError res1 = window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY);
    ASSERT_EQ(res1, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetSubWindowModal(false, ModalityType::APPLICATION_MODALITY));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    WMError res3 = window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY);
    ASSERT_EQ(res3, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal09
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal09, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_9");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10019);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(false, ModalityType::APPLICATION_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetSubWindowModal10
 * @tc.desc: SubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetSubWindowModal10, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_10");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10110);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(false, ModalityType::APPLICATION_MODALITY));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetWindowModal01
 * @tc.desc: WindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetWindowModal01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window4_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10041);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowModal(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowModal(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowModal(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetWindowModal02
 * @tc.desc: WindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetWindowModal02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window4_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10042);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowModal(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetWindowModal(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowModal(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetWindowModal03
 * @tc.desc: WindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, SetWindowModal03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window4_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10043);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowModal(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetWindowModal(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowModal(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: setWindowMask01
 * @tc.desc: setWindowMask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, setWindowMask01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window3_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10031);
    window->hostSession_ = session;

    std::vector<std::vector<uint32_t>> windowMask;
    windowMask = { { 1, 1, 1 }, { 1, 0, 1 }, { 1, 1, 1 } };
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));

    window->Destroy(true, true);
}

/**
 * @tc.name: setWindowMask02
 * @tc.desc: setWindowMask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, setWindowMask02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window3_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10032);
    window->hostSession_ = session;

    std::vector<std::vector<uint32_t>> windowMask;
    windowMask = { { 1, 1, 1 }, { 1, 0, 1 }, { 1, 1, 1 } };
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));

    window->Destroy(true, true);
}

/**
 * @tc.name: setWindowMask03
 * @tc.desc: setWindowMask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSpecialWindowTest, setWindowMask03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window3_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10033);
    window->hostSession_ = session;

    std::vector<std::vector<uint32_t>> windowMask;
    windowMask = { { 1, 1, 1 }, { 1, 0, 1 }, { 1, 1, 1 } };
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMask(windowMask));

    window->Destroy(true, true);
}

} // namespace
} // namespace Rosen
} // namespace OHOS