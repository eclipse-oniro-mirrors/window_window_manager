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
}
} // namespace Rosen
} // namespace OHOS