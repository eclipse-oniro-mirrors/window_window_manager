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
#include <parameters.h>
 
#include "application_context.h"
#include "ability_context_impl.h"
#include "common_test_utils.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "mock_window_adapter.h"
#include "pointer_event.h"
#include "rs_adapter.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "wm_common_inner.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace Rosen {
namespace {
using WindowAdapterMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
}
namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg += msg;
    }
}
class WindowSceneSessionImplTest6 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};
 
void WindowSceneSessionImplTest6::SetUpTestCase()
{
}
 
void WindowSceneSessionImplTest6::TearDownTestCase()
{
}
 
void WindowSceneSessionImplTest6::SetUp()
{
}
 
void WindowSceneSessionImplTest6::TearDown()
{
}
 
namespace {
/**
 * @tc.name: MaximizeEvent
 * @tc.desc: MaximizeEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, MaximizeEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("MaximizeEvent");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    property->SetPersistentId(123);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = mockHostSession;
 
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    system::SetParameter("persist.sceneboard.ispcmode", "false");
    window->property_->SetIsPcAppInPad(true);
    window->MaximizeEvent(window->hostSession_);
    EXPECT_EQ(window->property_->GetIsShowDecorInFreeMultiWindow(), true);
 
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    system::SetParameter("persist.sceneboard.ispcmode", "false");
    window->property_->SetIsPcAppInPad(true);
    window->MaximizeEvent(window->hostSession_);
    EXPECT_EQ(window->property_->GetIsShowDecorInFreeMultiWindow(), true);
 
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    system::SetParameter("persist.sceneboard.ispcmode", "true");
    window->property_->SetIsPcAppInPad(false);
    window->MaximizeEvent(window->hostSession_);
    EXPECT_EQ(window->property_->GetIsShowDecorInFreeMultiWindow(), true);
 
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    system::SetParameter("persist.sceneboard.ispcmode", "true");
    window->property_->SetIsPcAppInPad(true);
    window->MaximizeEvent(window->hostSession_);
    EXPECT_EQ(window->property_->GetIsShowDecorInFreeMultiWindow(), true);
 
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    system::SetParameter("persist.sceneboard.ispcmode", "false");
    window->property_->SetIsPcAppInPad(false);
    window->MaximizeEvent(window->hostSession_);
    EXPECT_EQ(window->property_->GetIsShowDecorInFreeMultiWindow(), false);
}

/**
 * @tc.name: AddSidebarBlur1
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddSidebarBlur1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSidebarBlur1");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->handler_ = nullptr;
    auto ret = window->AddSidebarBlur();
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: AddSidebarBlur2
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddSidebarBlur2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSidebarBlur2");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("AddSidebarBlur2");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    auto ret = window->AddSidebarBlur();
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(window->blurRadiusValue_);
}

/**
 * @tc.name: AddSidebarBlur3
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddSidebarBlur3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSidebarBlur3");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("AddSidebarBlur3");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    window->rsUIDirector_ = nullptr;
    auto ret = window->AddSidebarBlur();
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_FALSE(window->blurRadiusValue_);
}

/**
 * @tc.name: AddSidebarBlur4
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddSidebarBlur4, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSidebarBlur4");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("AddSidebarBlur4");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->surfaceNode_ = nullptr;
    auto ret = window->AddSidebarBlur();
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("surfaceNode is null") != std::string::npos);
    EXPECT_FALSE(window->blurRadiusValue_);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddSidebarBlur5
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddSidebarBlur5, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSidebarBlur5");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("AddSidebarBlur5");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = window->AddSidebarBlur();
    EXPECT_EQ(ret, WSError::WS_OK);
    window->Destroy(false);
    window = nullptr;
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("window is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddRSNodeModifier1
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier1");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(window->surfaceNode_, nullptr);
    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty has value") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddRSNodeModifier2
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier2");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(window->surfaceNode_, nullptr);
    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(true, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty has value") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddRSNodeModifier3
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier3");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(true, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    EXPECT_TRUE(logMsg.find("rsNode is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddRSNodeModifier4
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier4, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier4");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(window->surfaceNode_, nullptr);
    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(true, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
    window->blurMaskColorValue_ = nullptr;
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: AddRSNodeModifier5
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier5, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier5");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(window->surfaceNode_, nullptr);
    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(true, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
    window->blurBrightnessValue_ = nullptr;
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
}

/**
 * @tc.name: AddRSNodeModifier6
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier6, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier6");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(window->surfaceNode_, nullptr);
    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(true, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
    window->blurSaturationValue_ = nullptr;
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
}

/**
 * @tc.name: AddRSNodeModifier7
 * @tc.desc: AddRSNodeModifier
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, AddRSNodeModifier7, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddRSNodeModifier7");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(window->surfaceNode_, nullptr);
    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(true, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
    window->blurRadiusValue_ = nullptr;
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType1
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType1");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->handler_ = nullptr;
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType2
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType2");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("SetSidebarBlurStyleWithType2");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    window->Destroy(false);
    window = nullptr;
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("window is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType3
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType3");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("SetSidebarBlurStyleWithType3");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType4
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType4, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType4");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("SetSidebarBlurStyleWithType4");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
    window->blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_RADIUS_LIGHT);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType5
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType5, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType5");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("SetSidebarBlurStyleWithType5");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
    window->blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_RADIUS_LIGHT);
    window->blurSaturationValue_ =
        std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_SATURATION_LIGHT);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType6
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType6, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType6");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("SetSidebarBlurStyleWithType6");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
    window->blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_RADIUS_LIGHT);
    window->blurSaturationValue_ =
        std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_SATURATION_LIGHT);
    window->blurBrightnessValue_ =
        std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlurStyleWithType7
 * @tc.desc: SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, SetSidebarBlurStyleWithType7, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSidebarBlurStyleWithType7");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("SetSidebarBlurStyleWithType7");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    EXPECT_FALSE(window->blurRadiusValue_);
    window->AddRSNodeModifier(false, rsNodeTemp);

    window->colorMode_ = AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT;
    auto ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_SNAPSHOT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());

    window->colorMode_ = AppExecFwk::ConfigurationInner::COLOR_MODE_DARK;
    ret = window->SetSidebarBlurStyleWithType(SidebarBlurType::NONE);
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_SNAPSHOT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: UpdateSidebarBlurStyleWhenColorModeChange1
 * @tc.desc: UpdateSidebarBlurStyleWhenColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdateSidebarBlurStyleWhenColorModeChange1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSidebarBlurStyleWhenColorModeChange1");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateSidebarBlurStyleWhenColorModeChange2
 * @tc.desc: UpdateSidebarBlurStyleWhenColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdateSidebarBlurStyleWhenColorModeChange2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSidebarBlurStyleWhenColorModeChange2");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_RADIUS_LIGHT);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateSidebarBlurStyleWhenColorModeChange3
 * @tc.desc: UpdateSidebarBlurStyleWhenColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdateSidebarBlurStyleWhenColorModeChange3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSidebarBlurStyleWhenColorModeChange3");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_RADIUS_LIGHT);
    window->blurSaturationValue_ =
        std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_SATURATION_LIGHT);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateSidebarBlurStyleWhenColorModeChange4
 * @tc.desc: UpdateSidebarBlurStyleWhenColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdateSidebarBlurStyleWhenColorModeChange4, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSidebarBlurStyleWhenColorModeChange4");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_RADIUS_LIGHT);
    window->blurSaturationValue_ =
        std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_SATURATION_LIGHT);
    window->blurBrightnessValue_ =
        std::make_shared<Rosen::RSAnimatableProperty<float>>(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_TRUE(logMsg.find("RSAnimatableProperty is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateSidebarBlurStyleWhenColorModeChange5
 * @tc.desc: UpdateSidebarBlurStyleWhenColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdateSidebarBlurStyleWhenColorModeChange5, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSidebarBlurStyleWhenColorModeChange5");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    window->colorMode_ = AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT;
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());

    window->colorMode_ = AppExecFwk::ConfigurationInner::COLOR_MODE_DARK;
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: UpdateSidebarBlurStyleWhenColorModeChange6
 * @tc.desc: UpdateSidebarBlurStyleWhenColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdateSidebarBlurStyleWhenColorModeChange6, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSidebarBlurStyleWhenColorModeChange6");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->colorMode_ = AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT;
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_MAXIMIZE_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());

    window->colorMode_ = AppExecFwk::ConfigurationInner::COLOR_MODE_DARK;
    window->UpdateSidebarBlurStyleWhenColorModeChange();
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_MAXIMIZE_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty1
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty1");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(false, SidebarBlurType::NONE);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_SNAPSHOT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty2
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty2");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(false, SidebarBlurType::INITIAL);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty3
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty3");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(false, SidebarBlurType::DEFAULT_FLOAT);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty4
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty4, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty4");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(false, SidebarBlurType::DEFAULT_MAXIMIZE);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_MAXIMIZE_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty5
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty5, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty5");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(false, SidebarBlurType::END);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty6
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty6, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty6");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(true, SidebarBlurType::NONE);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_BLUR_NUMBER_ZERO, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_SNAPSHOT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty7
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty7, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty7");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(true, SidebarBlurType::INITIAL);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty8
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty8, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty8");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(true, SidebarBlurType::DEFAULT_FLOAT);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty9
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty9, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty9");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(true, SidebarBlurType::DEFAULT_MAXIMIZE);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_RADIUS_DARK, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_SATURATION_DARK, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_MAXIMIZE_BRIGHTNESS_DARK, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_MAXIMIZE_MASKCOLOR_DARK, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: ModifySidebarBlurProperty10
 * @tc.desc: ModifySidebarBlurProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ModifySidebarBlurProperty10, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty10");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
    EXPECT_NE(rsNodeTemp, nullptr);
    window->AddRSNodeModifier(false, rsNodeTemp);
    EXPECT_TRUE(window->blurRadiusValue_);
    window->ModifySidebarBlurProperty(true, SidebarBlurType::END);
    EXPECT_NEAR(SIDEBAR_DEFAULT_RADIUS_LIGHT, window->blurRadiusValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_SATURATION_LIGHT, window->blurSaturationValue_->Get(), 0.00001f);
    EXPECT_NEAR(SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT, window->blurBrightnessValue_->Get(), 0.00001f);
    EXPECT_EQ(SIDEBAR_DEFAULT_MASKCOLOR_LIGHT, window->blurMaskColorValue_->Get().AsArgbInt());
}

/**
 * @tc.name: maximizeWhenSwitchMultiWindowIfOnlySupportFullScreen
 * @tc.desc: maximizeWhenSwitchMultiWindowIfOnlySupportFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, maximizeWhenSwitchMultiWindowIfOnlySupportFullScreen, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("maximizeTest");
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->property_->SetWindowModeSupportType(1);
    window->state_ = WindowState::STATE_SHOWN;
    maximizeWhenSwitchMultiWindowIfOnlySupportFullScreen();
    EXPECT_EQ(window->IsLayoutFullScreen(), true);
}

/**
 * @tc.name: UpdatePropertyWhenTriggerMode
 * @tc.desc: UpdatePropertyWhenTriggerMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, UpdatePropertyWhenTriggerMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ModifySidebarBlurProperty9");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WSError ret = window->UpdatePropertyWhenTriggerMode(nullptr);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ret = window->UpdatePropertyWhenTriggerMode(property);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: NotifyPageEnable
 * @tc.desc: Test NotifyPageEnable with valid session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, NotifyPageEnable, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    auto ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(1)
        .WillOnce(Return(WSError::WS_OK));

    ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: NotifyPageEnable01
 * @tc.desc: Test NotifyPageEnable with null host session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, NotifyPageEnable01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    window->hostSession_ = nullptr;

    auto ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: NotifyPageEnable02
 * @tc.desc: Test NotifyPageEnable when host session returns error
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, NotifyPageEnable02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(1)
        .WillOnce(Return(WSError::WS_ERROR_INVALID_PARAM));

    auto ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: NotifyPageEnable03
 * @tc.desc: Test NotifyPageEnable with multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, NotifyPageEnable03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(3)
        .WillRepeatedly(Return(WSError::WS_OK));

    EXPECT_EQ(window->NotifyPageEnable("enter", "HomePage"), WMError::WM_OK);
    EXPECT_EQ(window->NotifyPageEnable("exit", "HomePage"), WMError::WM_OK);
    EXPECT_EQ(window->NotifyPageEnable("enter", "DetailPage"), WMError::WM_OK);
}

/**
 * @tc.name: ApplyForcibleLimitsNotForcible
 * @tc.desc: Verify no changes when window limits are not forcible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ApplyForcibleLimitsNotForcible, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    ASSERT_NE(property, nullptr);

    // Set window limits to not break system constraints
    property->SetIsWindowLimitsForcible(false);

    WindowLimits px;
    px.maxWidth_ = 100;
    px.maxHeight_ = 100;
    px.minWidth_ = 10;
    px.minHeight_ = 10;

    WindowLimits vp;
    vp.maxWidth_ = 100;
    vp.maxHeight_ = 100;
    vp.minWidth_ = 10;
    vp.minHeight_ = 10;

    window->ApplyForcibleLimits(px, vp, 2.0f, []() { return true; });

    // should remain unchanged
    EXPECT_EQ(px.maxWidth_, 100);
    EXPECT_EQ(px.maxHeight_, 100);
    EXPECT_EQ(px.minWidth_, 10);
    EXPECT_EQ(px.minHeight_, 10);

    EXPECT_EQ(vp.maxWidth_, 100);
    EXPECT_EQ(vp.maxHeight_, 100);
    EXPECT_EQ(vp.minWidth_, 10);
    EXPECT_EQ(vp.minHeight_, 10);
}

/**
 * @tc.name: ApplyForcibleLimitsNoPermission
 * @tc.desc: Verify no changes when caller has no permission to break system limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ApplyForcibleLimitsNoPermission, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    ASSERT_NE(property, nullptr);

    // Enable break system constraints
    property->SetIsWindowLimitsForcible(true);

    WindowLimits px;
    px.maxWidth_ = 200;
    px.maxHeight_ = 200;
    px.minWidth_ = 20;
    px.minHeight_ = 20;

    WindowLimits vp;
    vp.maxWidth_ = 200;
    vp.maxHeight_ = 200;
    vp.minWidth_ = 20;
    vp.minHeight_ = 20;

    // Permission denied
    window->ApplyForcibleLimits(px, vp, 2.0f, []() { return false; });

    // should remain unchanged
    EXPECT_EQ(px.maxWidth_, 200);
    EXPECT_EQ(px.maxHeight_, 200);
    EXPECT_EQ(px.minWidth_, 20);
    EXPECT_EQ(px.minHeight_, 20);

    EXPECT_EQ(vp.maxWidth_, 200);
    EXPECT_EQ(vp.maxHeight_, 200);
    EXPECT_EQ(vp.minWidth_, 20);
    EXPECT_EQ(vp.minHeight_, 20);
}

/**
 * @tc.name: ApplyForcibleLimitsWithPermission
 * @tc.desc: Verify system limits are widened when forcible mode and permission are enabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ApplyForcibleLimitsWithPermission, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    ASSERT_NE(property, nullptr);

    // Enable break system constraints
    property->SetIsWindowLimitsForcible(true);

    WindowLimits px;
    px.maxWidth_ = 300;
    px.maxHeight_ = 300;
    px.minWidth_ = 50;
    px.minHeight_ = 50;

    WindowLimits vp;
    vp.maxWidth_ = 300;
    vp.maxHeight_ = 300;
    vp.minWidth_ = 50;
    vp.minHeight_ = 50;

    constexpr float vpr = 2.0f;
    constexpr uint32_t FORCIBLE_MAX = static_cast<uint32_t>(INT32_MAX);
    constexpr uint32_t FORCIBLE_MIN = 1;

    window->ApplyForcibleLimits(px, vp, vpr, []() { return true; });

    // max should be expanded
    EXPECT_EQ(px.maxWidth_, FORCIBLE_MAX);
    EXPECT_EQ(px.maxHeight_, FORCIBLE_MAX);
    EXPECT_EQ(vp.maxWidth_, FORCIBLE_MAX);
    EXPECT_EQ(vp.maxHeight_, FORCIBLE_MAX);
    // min should be relaxed to 1
    EXPECT_EQ(px.minWidth_, FORCIBLE_MIN);
    EXPECT_EQ(px.minHeight_, FORCIBLE_MIN);
    EXPECT_EQ(vp.minWidth_, FORCIBLE_MIN);
    EXPECT_EQ(vp.minHeight_, FORCIBLE_MIN);
}

/**
 * @tc.name: ApplyForcibleLimitsMinAlreadySmaller
 * @tc.desc: Verify min limits are not increased if already smaller than forcible minimum
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest6, ApplyForcibleLimitsMinAlreadySmaller, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    ASSERT_NE(property, nullptr);

    property->SetIsWindowLimitsForcible(true);

    WindowLimits px;
    px.maxWidth_ = 300;
    px.maxHeight_ = 300;
    px.minWidth_ = 0;   // already smaller than FORCIBLE_MIN(1)
    px.minHeight_ = 0;

    WindowLimits vp;
    vp.maxWidth_ = 300;
    vp.maxHeight_ = 300;
    vp.minWidth_ = 0;
    vp.minHeight_ = 0;

    window->ApplyForcibleLimits(px, vp, 2.0f, []() { return true; });

    // should keep original smaller values
    EXPECT_EQ(px.minWidth_, 0);
    EXPECT_EQ(px.minHeight_, 0);
    EXPECT_EQ(vp.minWidth_, 0);
    EXPECT_EQ(vp.minHeight_, 0);
}
} // namespace
} // namespace Rosen
} // namespace OHOS