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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "ability_context_impl.h"
#include "color_parser.h"
#include "extension/extension_business_info.h"
#include "mock_ability_context_impl.h"
#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "window_helper.h"
#define private public
#define protected public
#include "window_session_impl.h"
#undef private
#undef protected
#include "wm_common.h"
#include "window_manager_hilog.h"
#include <transaction/rs_transaction.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}
static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
class WindowSessionImplTest5 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr int32_t PERSISTENT_ID_ONE = 1;
    static constexpr int32_t PERSISTENT_ID_TWO = 2;
};

void WindowSessionImplTest5::SetUpTestCase() {}

void WindowSessionImplTest5::TearDownTestCase() {}

void WindowSessionImplTest5::SetUp()
{
}

void WindowSessionImplTest5::TearDown()
{
}

namespace {
sptr<WindowSessionImpl> GetTestWindowImpl(const std::string& name)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    if (option == nullptr) {
        return nullptr;
    }
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    if (window == nullptr) {
        return nullptr;
    }

    SessionInfo sessionInfo = { name, name, name };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    if (session == nullptr) {
        return nullptr;
    }

    window->hostSession_ = session;
    auto runner = AppExecFwk::EventRunner::Create("WindowSessionImpl");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    window->handler_ = handler;
    return window;
}

/**
 * @tc.name: NotifyAfterFocused
 * @tc.desc: NotifyAfterFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyAfterFocused, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyAfterFocused");
    ASSERT_NE(window, nullptr);
    window->NotifyAfterFocused();
    ASSERT_TRUE(window->shouldReNotifyFocus_);

    window->shouldReNotifyFocus_ = false;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->NotifyAfterFocused();
    ASSERT_FALSE(window->shouldReNotifyFocus_);
    window->Destroy();
}

/**
 * @tc.name: NotifyBeforeDestroy
 * @tc.desc: NotifyBeforeDestroy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyBeforeDestroy, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyBeforeDestroy");
    ASSERT_NE(window, nullptr);
    window->isAttachedOnFrameNode_ = false;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->NotifyBeforeDestroy(window->GetWindowName());
    EXPECT_EQ(window->GetUIContentSharedPtr(), nullptr);

    window->isAttachedOnFrameNode_ = false;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetIsUIExtFirstSubWindow(false);
    window->NotifyBeforeDestroy(window->GetWindowName());
    EXPECT_EQ(window->GetUIContentSharedPtr(), nullptr);

    window->isAttachedOnFrameNode_ = false;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetIsUIExtFirstSubWindow(true);
    window->NotifyBeforeDestroy(window->GetWindowName());
    EXPECT_EQ(window->GetUIContentSharedPtr(), nullptr);
}

/**
 * @tc.name: GetFocusabletest01
 * @tc.desc: GetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetFocusable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetFocusableTest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetFocusable");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool ret = window->GetFocusable();
    ASSERT_EQ(ret, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetFocusableTest01 end";
}

/**
 * @tc.name: SetTopmost
 * @tc.desc: SetTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetTopmost, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTopmost");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WMError res = window->SetTopmost(true);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, res);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->SetTopmost(true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, res);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    res = window->SetTopmost(true);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: IsTopmost
 * @tc.desc: IsTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsTopmost, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsTopmost");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    bool res = window->IsTopmost();
    ASSERT_FALSE(res);
}

/**
 * @tc.name: SetMainWindowTopmost
 * @tc.desc: SetMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetMainWindowTopmost, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetMainWindowTopmost");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError res = window->SetMainWindowTopmost(false);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    res = window->SetMainWindowTopmost(true);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    res = window->SetMainWindowTopmost(true);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    res = window->SetMainWindowTopmost(true);
    EXPECT_EQ(res, WMError::WM_OK);
    res = window->SetMainWindowTopmost(false);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: IsMainWindowTopmost
 * @tc.desc: IsMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsMainWindowTopmost, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsMainWindowTopmost");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool res = window->IsMainWindowTopmost();
    ASSERT_FALSE(res);
}

/**
 * @tc.name: GetSubWindows
 * @tc.desc: GetSubWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetSubWindows, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetSubWindows start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindows");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::vector<sptr<WindowSessionImpl>> subWindows;
    window->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 0);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("GetSubWindows_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow }));

    subWindows.clear();
    window->GetSubWindows(1, subWindows);
    ASSERT_EQ(subWindows.size(), 1);
    EXPECT_EQ(subWindows[0], subWindow);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetSubWindows end";
}

/**
 * @tc.name: RemoveSubWindow
 * @tc.desc: RemoveSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RemoveSubWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("RemoveSubWindow_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow }));

    sptr<WindowOption> subWindowOption2 = sptr<WindowOption>::MakeSptr();
    subWindowOption2->SetWindowName("RemoveSubWindow_subWindow2");
    sptr<WindowSessionImpl> subWindow2 = sptr<WindowSessionImpl>::MakeSptr(subWindowOption2);
    subWindow2->property_->SetPersistentId(3);
    subWindow2->property_->SetParentPersistentId(1);
    subWindow2->hostSession_ = session;
    subWindow2->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow2->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::subWindowSessionMap_[1].push_back(subWindow2);

    subWindow->RemoveSubWindow(2);
    std::vector<sptr<WindowSessionImpl>> subWindows;
    window->GetSubWindows(1, subWindows);
    ASSERT_EQ(subWindows.size(), 2);
    EXPECT_EQ(subWindows[0], subWindow);
    subWindow->RemoveSubWindow(1);
    subWindows.clear();
    window->GetSubWindows(1, subWindows);
    ASSERT_EQ(subWindows.size(), 1);
    EXPECT_NE(subWindows[0], subWindow);
}

/**
 * @tc.name: DestroySubWindow01
 * @tc.desc: DestroySubWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, DestroySubWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DestroySubWindow01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    sptr<WindowOption> subOption01 = sptr<WindowOption>::MakeSptr();
    subOption01->SetWindowName("DestroySubWindow01_subwindow01");
    subOption01->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow01 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow01->property_->SetPersistentId(2);
    subWindow01->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo1 = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session1 = sptr<SessionMocker>::MakeSptr(sessionInfo1);
    subWindow01->hostSession_ = session1;
    subWindow01->state_ = WindowState::STATE_CREATED;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow01 }));

    sptr<WindowOption> subOption02 = sptr<WindowOption>::MakeSptr();
    subOption02->SetWindowName("DestroySubWindow01_subwindow02");
    subOption02->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow02 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow02->property_->SetPersistentId(3);
    subWindow02->property_->SetParentPersistentId(2);
    SessionInfo sessionInfo2 = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session2 = sptr<SessionMocker>::MakeSptr(sessionInfo2);
    subWindow02->hostSession_ = session2;
    subWindow02->state_ = WindowState::STATE_CREATED;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(2, { subWindow02 }));

    std::vector<sptr<WindowSessionImpl>> subWindows;
    window->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 1);
    subWindows.clear();
    window->GetSubWindows(2, subWindows);
    EXPECT_EQ(subWindows.size(), 1);
    window->DestroySubWindow();
    EXPECT_EQ(subWindow01->state_, WindowState::STATE_DESTROYED);
    EXPECT_EQ(subWindow02->state_, WindowState::STATE_DESTROYED);
    EXPECT_EQ(subWindow01->hostSession_, nullptr);
    EXPECT_EQ(subWindow02->hostSession_, nullptr);
    subWindows.clear();
    window->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 0);
    subWindows.clear();
    window->GetSubWindows(2, subWindows);
    EXPECT_EQ(subWindows.size(), 0);
}

/**
 * @tc.name: SetUniqueVirtualPixelRatioForSub
 * @tc.desc: SetUniqueVirtualPixelRatioForSub test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetUniqueVirtualPixelRatioForSub, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUniqueVirtualPixelRatioForSub");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    sptr<WindowOption> subOption01 = sptr<WindowOption>::MakeSptr();
    subOption01->SetWindowName("SetUniqueVirtualPixelRatioForSub_subwindow01");
    subOption01->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow01 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow01->property_->SetPersistentId(2);
    subWindow01->property_->SetParentPersistentId(1);
    subWindow01->useUniqueDensity_ = false;
    subWindow01->virtualPixelRatio_ = 0.5f;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow01 }));

    sptr<WindowOption> subOption02 = sptr<WindowOption>::MakeSptr();
    subOption02->SetWindowName("SetUniqueVirtualPixelRatioForSub_subwindow02");
    subOption02->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow02 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow02->property_->SetPersistentId(3);
    subWindow02->property_->SetParentPersistentId(2);
    subWindow01->useUniqueDensity_ = false;
    subWindow01->virtualPixelRatio_ = 0.3f;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(2, { subWindow02 }));

    bool useUniqueDensity = true;
    float virtualPixelRatio = 1.0f;
    window->SetUniqueVirtualPixelRatioForSub(useUniqueDensity, virtualPixelRatio);
    EXPECT_EQ(subWindow01->useUniqueDensity_, useUniqueDensity);
    EXPECT_EQ(subWindow02->useUniqueDensity_, useUniqueDensity);
    EXPECT_NEAR(subWindow01->virtualPixelRatio_, virtualPixelRatio, 0.00001f);
    EXPECT_NEAR(subWindow02->virtualPixelRatio_, virtualPixelRatio, 0.00001f);
}

/**
 * @tc.name: RegisterWindowRotationChangeListener
 * @tc.desc: RegisterWindowRotationChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterWindowRotationChangeListener, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterWindowRotationChangeListener");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowRotationChangeListener> listener = nullptr;
    WMError ret = window->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowRotationChangeListener>::MakeSptr();
    std::vector<sptr<IWindowRotationChangeListener>> holder;
    window->windowRotationChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    holder = window->windowRotationChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_NE(existsListener, holder.end());

    ret = window->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterWindowRotationChangeListener
 * @tc.desc: UnregisterWindowRotationChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UnregisterWindowRotationChangeListener, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterWindowRotationChangeListener");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowRotationChangeListener> listener = nullptr;
    WMError ret = window->UnregisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowRotationChangeListener>::MakeSptr();
    std::vector<sptr<IWindowRotationChangeListener>> holder;
    window->windowRotationChangeListeners_[window->property_->GetPersistentId()] = holder;
    window->RegisterWindowRotationChangeListener(listener);
    ret = window->UnregisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);

    holder = window->windowRotationChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    EXPECT_EQ(existsListener, holder.end());
}

/**
 * @tc.name: CheckMultiWindowRect
 * @tc.desc: CheckMultiWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, CheckMultiWindowRect, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CheckMultiWindowRect");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t width = 100;
    uint32_t height = 100;
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;
    auto ret = windowSessionImpl->CheckMultiWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = windowSessionImpl->CheckMultiWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: IsDeviceFeatureCapableFor
 * @tc.desc: IsDeviceFeatureCapableFor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsDeviceFeatureCapableFor, Function | SmallTest | Level2)
{
    const std::string feature = "free_multi_window";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsDeviceFeatureCapableFor");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    EXPECT_EQ(window->IsDeviceFeatureCapableFor(feature), false);
    auto context = std::make_shared<MockAbilityContextImpl>();
    window->context_ = context;
    context->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    EXPECT_EQ(window->IsDeviceFeatureCapableFor(feature), false);
    std::string deviceType = system::GetParameter("const.product.devicetype", "");
    context->hapModuleInfo_->requiredDeviceFeatures = {{deviceType, {}}};
    EXPECT_EQ(window->IsDeviceFeatureCapableFor(feature), false);
    context->hapModuleInfo_->requiredDeviceFeatures = {{deviceType, {feature}}};
    EXPECT_EQ(window->IsDeviceFeatureCapableFor(feature), true);
}

/**
 * @tc.name: IsDeviceFeatureCapableForFreeMultiWindow
 * @tc.desc: IsDeviceFeatureCapableForFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsDeviceFeatureCapableForFreeMultiWindow, Function | SmallTest | Level2)
{
    const std::string feature = "free_multi_window";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsDeviceFeatureCapableForFreeMultiWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    EXPECT_EQ(window->IsDeviceFeatureCapableForFreeMultiWindow(), false);
    auto context = std::make_shared<MockAbilityContextImpl>();
    window->context_ = context;
    context->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    EXPECT_EQ(window->IsDeviceFeatureCapableForFreeMultiWindow(), false);
    std::string deviceType = system::GetParameter("const.product.devicetype", "");
    context->hapModuleInfo_->requiredDeviceFeatures = {{deviceType, {feature}}};
    EXPECT_EQ(window->IsDeviceFeatureCapableForFreeMultiWindow(),
        system::GetParameter("const.window.device_feature_support_type", "0") == "1");
}

/**
 * @tc.name: NotifyRotationChange
 * @tc.desc: NotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyRotationChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyRotationChange");
    option->SetWindowType(WindowType::APP_WINDOW_BASE);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;
    RotationChangeInfo info = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 1, { 0, 0, 2720, 1270 } };
    RotationChangeResult res = windowSessionImpl->NotifyRotationChange(info);
    EXPECT_EQ(RectType::RELATIVE_TO_SCREEN, res.rectType_);

    sptr<IWindowRotationChangeListener> listener = sptr<IWindowRotationChangeListener>::MakeSptr();
    std::vector<sptr<IWindowRotationChangeListener>> holder;
    windowSessionImpl->windowRotationChangeListeners_[windowSessionImpl->property_->GetPersistentId()] = holder;
    WMError ret = windowSessionImpl->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    res = windowSessionImpl->NotifyRotationChange(info);
    EXPECT_EQ(RectType::RELATIVE_TO_SCREEN, res.rectType_);

    info.type_ = RotationChangeType::WINDOW_DID_ROTATE;
    res = windowSessionImpl->NotifyRotationChange(info);
    EXPECT_EQ(RectType::RELATIVE_TO_SCREEN, res.rectType_);
}

/**
 * @tc.name: UpdateRectForPageRotation
 * @tc.desc: UpdateRectForPageRotation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateRectForPageRotation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UpdateRectForPageRotation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("UpdateRectForPageRotation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Rect wmRect = { 0, 0, 0, 0 };
    Rect preRect = { 0, 0, 0, 0 };
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::PAGE_ROTATION;
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config = {rsTransaction, ROTATE_ANIMATION_DURATION,
    0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    std::shared_ptr<AvoidArea> avoidArea = std::make_shared<AvoidArea>();
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    avoidAreas[type] = *avoidArea;

    window->property_->SetWindowRect(preRect);
    window->postTaskDone_ = false;
    window->UpdateRectForPageRotation(wmRect, preRect, wmReason, config, avoidAreas);
    EXPECT_EQ(window->postTaskDone_, false);

    WSRect rect = { 0, 0, 50, 50 };
    SizeChangeReason reason = SizeChangeReason::PAGE_ROTATION;
    auto res = window->UpdateRect(rect, reason, config, avoidAreas);
    EXPECT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UpdateRectForPageRotation end";
}

/**
 * @tc.name: UpdateRectForResizeAnimation
 * @tc.desc: UpdateRectForResizeAnimation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateRectForResizeAnimation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: UpdateRectForResizeAnimation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("UpdateRectForResizeAnimation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("UpdateRectForResizeAnimation");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config{ rsTransaction, ROTATE_ANIMATION_DURATION,
    0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    std::shared_ptr<AvoidArea> avoidArea = std::make_shared<AvoidArea>();
    Rect preRect = { 0, 0, 0, 0 };
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    avoidAreas[type] = *avoidArea;
    window->property_->SetWindowRect(preRect);
    window->postTaskDone_ = false;
    WSRect rect = { 0, 0, 50, 50 };
    SizeChangeReason reason = SizeChangeReason::SCENE_WITH_ANIMATION;
    auto res = window->UpdateRect(rect, reason, config, avoidAreas);
    EXPECT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: UpdateRectForResizeAnimation end";
}
 
/**
 * @tc.name: UpdateRectForResizeAnimation01
 * @tc.desc: UpdateRectForResizeAnimation01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateRectForResizeAnimation01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: UpdateRectForResizeAnimation01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("UpdateRectForResizeAnimation01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("UpdateRectForResizeAnimation01");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::SCENE_WITH_ANIMATION;
    std::shared_ptr<RSTransaction> rsTransaction = std::make_shared<RSTransaction>();
    SceneAnimationConfig config = { rsTransaction, ROTATE_ANIMATION_DURATION,
    0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    std::shared_ptr<AvoidArea> avoidArea = std::make_shared<AvoidArea>();
    Rect wmRect = { 0, 0, 0, 0 };
    Rect preRect = { 0, 0, 0, 0 };
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    avoidAreas[type] = *avoidArea;
    window->property_->SetWindowRect(preRect);
    window->postTaskDone_ = false;
    window->UpdateRectForResizeAnimation(wmRect, preRect, wmReason, config, avoidAreas);
    usleep(WAIT_SYNC_IN_NS);
 
    config.animationCurve_ = WindowAnimationCurve::INTERPOLATION_SPRING;
    window->UpdateRectForResizeAnimation(wmRect, preRect, wmReason, config, avoidAreas);
    usleep(WAIT_SYNC_IN_NS);
 
    config.animationCurve_ = WindowAnimationCurve::CUBIC_BEZIER;
    window->UpdateRectForResizeAnimation(wmRect, preRect, wmReason, config, avoidAreas);
    usleep(WAIT_SYNC_IN_NS);
    
    EXPECT_EQ(window->postTaskDone_, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: UpdateRectForResizeAnimation01 end";
}
 
/**
 * @tc.name: UpdateRectForResizeAnimation02
 * @tc.desc: UpdateRectForResizeAnimation02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateRectForResizeAnimation02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: UpdateRectForResizeAnimation02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("UpdateRectForResizeAnimation02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("UpdateRectForResizeAnimation02");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::SCENE_WITH_ANIMATION;
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config = { rsTransaction, ROTATE_ANIMATION_DURATION,
        0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    std::shared_ptr<AvoidArea> avoidArea = std::make_shared<AvoidArea>();
    Rect wmRect = { 0, 0, 0, 0 };
    Rect preRect = { 0, 0, 0, 0 };
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    avoidAreas[type] = *avoidArea;
    window->property_->SetWindowRect(preRect);
    window->postTaskDone_ = false;
    window->UpdateRectForResizeAnimation(wmRect, preRect, wmReason, config, avoidAreas);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(window->postTaskDone_, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: UpdateRectForResizeAnimation02 end";
}
 
/**
 * @tc.name: RegisterPreferredOrientationChangeListener
 * @tc.desc: RegisterPreferredOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterPreferredOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterPreferredOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreferredOrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IPreferredOrientationChangeListener> listener = nullptr;
    WMError res = window->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IPreferredOrientationChangeListener>::MakeSptr();
    sptr<IPreferredOrientationChangeListener> holder;
    window->preferredOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    res = window->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    holder = window->preferredOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_EQ(holder, listener);

    // already registered
    res = window->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterPreferredOrientationChangeListener end";
}

/**
 * @tc.name: UnregisterPreferredOrientationChangeListener
 * @tc.desc: UnregisterPreferredOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UnregisterPreferredOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterPreferredOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreferredOrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IPreferredOrientationChangeListener> listener = nullptr;
    WMError res = window->UnregisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IPreferredOrientationChangeListener>::MakeSptr();
    sptr<IPreferredOrientationChangeListener> holder;
    window->preferredOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    window->RegisterPreferredOrientationChangeListener(listener);

    res = window->UnregisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);

    holder = window->preferredOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_NE(holder, listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterPreferredOrientationChangeListener end";
}

/**
 * @tc.name: NotifyPreferredOrientationChange
 * @tc.desc: NotifyPreferredOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyPreferredOrientationChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPreferredOrientationChange");
    option->SetWindowType(WindowType::APP_WINDOW_BASE);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;

    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    windowSessionImpl->NotifyPreferredOrientationChange(orientation);

    sptr<IPreferredOrientationChangeListener> listener = sptr<IPreferredOrientationChangeListener>::MakeSptr();
    sptr<IPreferredOrientationChangeListener> holder;
    windowSessionImpl->preferredOrientationChangeListener_[windowSessionImpl->property_->GetPersistentId()] = holder;
    WMError res = windowSessionImpl->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RegisterOrientationChangeListener
 * @tc.desc: RegisterOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowOrientationChangeListener> listener = nullptr;
    WMError res = window->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowOrientationChangeListener>::MakeSptr();
    sptr<IWindowOrientationChangeListener> holder;
    window->windowOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    res = window->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    holder = window->windowOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_EQ(holder, listener);

    // already registered
    res = window->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterOrientationChangeListener end";
}

/**
 * @tc.name: UnregisterOrientationChangeListener
 * @tc.desc: UnregisterOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UnregisterOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowOrientationChangeListener> listener = nullptr;
    WMError res = window->UnregisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowOrientationChangeListener>::MakeSptr();
    sptr<IWindowOrientationChangeListener> holder;
    window->windowOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    window->RegisterOrientationChangeListener(listener);

    res = window->UnregisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);

    holder = window->windowOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_NE(holder, listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterOrientationChangeListener end";
}

/**
 * @tc.name: NotifyClientOrientationChange
 * @tc.desc: NotifyClientOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyClientOrientationChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyClientOrientationChange");
    option->SetWindowType(WindowType::APP_WINDOW_BASE);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;

    windowSessionImpl->NotifyClientOrientationChange();

    sptr<IWindowOrientationChangeListener> listener = sptr<IWindowOrientationChangeListener>::MakeSptr();
    sptr<IWindowOrientationChangeListener> holder;
    windowSessionImpl->windowOrientationChangeListener_[windowSessionImpl->property_->GetPersistentId()] = holder;
    WMError res = windowSessionImpl->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetCurrentWindowOrientation
 * @tc.desc: GetCurrentWindowOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetCurrentWindowOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetCurrentWindowOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("GetCurrentWindowOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->UpdateCurrentWindowOrientation(DisplayOrientation::PORTRAIT);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::PORTRAIT);
    window->UpdateCurrentWindowOrientation(DisplayOrientation::UNKNOWN);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::UNKNOWN);
    window->UpdateCurrentWindowOrientation(DisplayOrientation::LANDSCAPE);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::LANDSCAPE);
    window->UpdateCurrentWindowOrientation(DisplayOrientation::PORTRAIT_INVERTED);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::PORTRAIT_INVERTED);
    window->UpdateCurrentWindowOrientation(DisplayOrientation::LANDSCAPE_INVERTED);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::LANDSCAPE_INVERTED);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetCurrentWindowOrientation end";
}

/**
 * @tc.name: GetRequestedOrientation
 * @tc.desc: GetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetRequestedOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetRequestedOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("GetRequestedOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::USER_ROTATION_PORTRAIT);
    window->SetUserRequestedOrientation(Orientation::VERTICAL);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::VERTICAL);
    window->SetUserRequestedOrientation(Orientation::HORIZONTAL);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::HORIZONTAL);
    window->SetUserRequestedOrientation(Orientation::SENSOR);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::SENSOR);
    window->SetUserRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::FOLLOW_DESKTOP);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetRequestedOrientation end";
}

/**
 * @tc.name: isNeededForciblySetOrientation
 * @tc.desc: isNeededForciblySetOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, isNeededForciblySetOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: isNeededForciblySetOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("isNeededForciblySetOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
 
    Orientation ori = Orientation::VERTICAL;
    window->SetRequestedOrientation(ori);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::USER_ROTATION_PORTRAIT), true);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::VERTICAL), false);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::HORIZONTAL), true);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::SENSOR), true);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::FOLLOW_DESKTOP), true);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::INVALID), false);
    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT, false);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::INVALID), true);
    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::INVALID), false);
    window->SetRequestedOrientation(Orientation::HORIZONTAL, false);
    EXPECT_EQ(window->isNeededForciblySetOrientation(Orientation::INVALID), true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: isNeededForciblySetOrientation end";
}

/**
 * @tc.name: ConvertInvalidOrientation()
 * @tc.desc: ConvertInvalidOrientation()
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, ConvertInvalidOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: ConvertInvalidOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("ConvertInvalidOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    Orientation ori = Orientation::USER_ROTATION_PORTRAIT;
    window->SetRequestedOrientation(ori);
    window->SetRequestedOrientation(Orientation::INVALID, false);
    EXPECT_EQ(window->ConvertInvalidOrientation(), Orientation::USER_PAGE_ROTATION_PORTRAIT);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: ConvertInvalidOrientation end";
}

/**
 * @tc.name: NotifyPageRotationIsIgnored()
 * @tc.desc: NotifyPageRotationIsIgnored()
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyPageRotationIsIgnored, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: NotifyPageRotationIsIgnored start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("NotifyPageRotationIsIgnored");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    EXPECT_EQ(window->NotifyPageRotationIsIgnored(), WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: NotifyPageRotationIsIgnored end";
}

/**
 * @tc.name: ConvertOrientationAndRotation()
 * @tc.desc: ConvertOrientationAndRotation()
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, ConvertOrientationAndRotation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: ConvertOrientationAndRotation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("ConvertOrientationAndRotation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(INVALID_SESSION_ID);
    RotationInfoType from = RotationInfoType::DISPLAY_ORIENTATION;
    RotationInfoType to = RotationInfoType::DISPLAY_ORIENTATION;
    int32_t value = -1;
    int32_t convertedValue = 0;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
        window->ConvertOrientationAndRotation(from, to, value, convertedValue));
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT,
        window->ConvertOrientationAndRotation(from, to, value, convertedValue));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    value = -1;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM,
        window->ConvertOrientationAndRotation(from, to, value, convertedValue));
    value = 8;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->ConvertOrientationAndRotation(from, to, value, convertedValue));
    value = 0;
    ASSERT_EQ(WMError::WM_OK, window->ConvertOrientationAndRotation(from, to, value, convertedValue));
    to = RotationInfoType::DISPLAY_ROTATION;
    ASSERT_EQ(WMError::WM_OK, window->ConvertOrientationAndRotation(from, to, value, convertedValue));
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: ConvertOrientationAndRotation end";
}

/**
 * @tc.name: BeginRSTransaction()
 * @tc.desc: BeginRSTransaction()
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, BeginRSTransaction, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: BeginRSTransaction start";
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("NotifyPageRotationIsIgnored");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    window->BeginRSTransaction(rsTransaction);
    EXPECT_TRUE(g_errLog.find("rsTransaction is null") != std::string::npos);
    
    std::shared_ptr<RSTransaction> rsTransaction1 = std::make_shared<RSTransaction>();
    rsTransaction1->syncId_ = 1;
    rsTransaction1->isOpenSyncTransaction_ = true;
    window->BeginRSTransaction(rsTransaction1);
    EXPECT_TRUE(g_errLog.find("rsTransaction begin") != std::string::npos);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: BeginRSTransaction end";
}

/**
 * @tc.name: IsUserPageOrientation()
 * @tc.desc: IsUserPageOrientation()
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsUserPageOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: IsUserPageOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("IsUserPageOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    Orientation ori = Orientation::USER_ROTATION_PORTRAIT;
    window->IsUserPageOrientation(ori);
    EXPECT_EQ(window->IsUserPageOrientation(Orientation::USER_ROTATION_PORTRAIT), false);
    EXPECT_EQ(window->IsUserPageOrientation(Orientation::USER_PAGE_ROTATION_PORTRAIT), true);
    EXPECT_EQ(window->IsUserPageOrientation(Orientation::USER_PAGE_ROTATION_LANDSCAPE), true);
    EXPECT_EQ(window->IsUserPageOrientation(Orientation::USER_PAGE_ROTATION_PORTRAIT_INVERTED), true);
    EXPECT_EQ(window->IsUserPageOrientation(Orientation::USER_PAGE_ROTATION_LANDSCAPE_INVERTED), true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: IsUserPageOrientation end";
}

/**
 * @tc.name: SetFollowScreenChange
 * @tc.desc: SetFollowScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetFollowScreenChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFollowScreenChange");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(0);
    WMError ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, ret);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, ret);

    window->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetScaleWindow
 * @tc.desc: GetScaleWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetScaleWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetScaleWindow_window1");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    int32_t id = 1;
    mainWindow->property_->SetPersistentId(id);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    auto res = mainWindow->GetScaleWindow(id);
    EXPECT_NE(res, nullptr);

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("GetScaleWindow_extensionWindow");
    sptr<WindowSessionImpl> extensionWindow = sptr<WindowSessionImpl>::MakeSptr(option2);
    WindowSessionImpl::GetWindowExtensionSessionSet().clear();
    WindowSessionImpl::GetWindowExtensionSessionSet().insert(extensionWindow);
    extensionWindow->property_->SetPersistentId(2);
    extensionWindow->isUIExtensionAbilityProcess_ = true;
    int32_t testId = 3;
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_EQ(res, nullptr);
    extensionWindow->property_->SetParentPersistentId(testId);
    extensionWindow->property_->SetParentId(testId);
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_NE(res, nullptr);
    mainWindow->isFocused_ = true;
    extensionWindow->isUIExtensionAbilityProcess_ = false;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    mainWindow->hostSession_ = session;
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_NE(res, nullptr);
    mainWindow->isFocused_ = false;
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_EQ(res, nullptr);
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::GetWindowExtensionSessionSet().clear();
}

/**
 * @tc.name: GetWindowScaleCoordinate01
 * @tc.desc: GetWindowScaleCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetWindowScaleCoordinate01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowScaleCoordinate01_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    int32_t id = 1;
    mainWindow->property_->SetPersistentId(id);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    CursorInfo cursorInfo { 1, 1, 1, 1 };
    auto res = mainWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    res = mainWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    mainWindow->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = mainWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("GetWindowScaleCoordinate01_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(id);
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(subWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(subWindow->GetWindowId(), subWindow)));
    res = subWindow->GetWindowScaleCoordinate(subWindow->GetPersistentId(), cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    mainWindow->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    subWindow->context_ = mainWindow->context_;
    subWindow->property_->SetIsUIExtensionAbilityProcess(true);
    res = mainWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    subWindow->property_->SetIsUIExtensionAbilityProcess(false);
    res = mainWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    mainWindow->compatScaleX_ = 0.5;
    res = mainWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    WindowSessionImpl::windowSessionMap_.clear();
}

/**
 * @tc.name: GetWindowScaleCoordinate02
 * @tc.desc: GetWindowScaleCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetWindowScaleCoordinate02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowScaleCoordinate02_extensionWindow");
    sptr<WindowSessionImpl> extensionWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    WindowSessionImpl::GetWindowExtensionSessionSet().clear();
    WindowSessionImpl::GetWindowExtensionSessionSet().insert(extensionWindow);
    extensionWindow->property_->SetPersistentId(2);
    extensionWindow->isUIExtensionAbilityProcess_ = true;
    extensionWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    int32_t id = 1;
    CursorInfo cursorInfo;
    auto res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
    cursorInfo.width = 1;
    res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
    cursorInfo.width = -1;
    cursorInfo.height = 1;
    res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
    cursorInfo.width = 1;
    res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    extensionWindow->property_->SetParentPersistentId(id);
    extensionWindow->property_->SetParentId(id);
    res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    extensionWindow->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    extensionWindow->compatScaleX_ = 0.5;
    res = extensionWindow->GetWindowScaleCoordinate(id, cursorInfo);
    EXPECT_EQ(res, WMError::WM_OK);
    WindowSessionImpl::GetWindowExtensionSessionSet().clear();
}

/**
 * @tc.name: SetCurrentTransform
 * @tc.desc: SetCurrentTransform
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetCurrentTransform, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetCurrentTransform");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    Transform transform;
    transform.scaleX_ = 0.5f;
    transform.scaleY_ = 0.6f;
    window->SetCurrentTransform(transform);
    EXPECT_NEAR(window->currentTransform_.scaleX_, transform.scaleX_, 0.00001f);
    EXPECT_NEAR(window->currentTransform_.scaleY_, transform.scaleY_, 0.00001f);
    auto res = window->GetCurrentTransform();
    EXPECT_TRUE(res == transform);
}

/**
 * @tc.name: UpdateCompatScaleInfo
 * @tc.desc: UpdateCompatScaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateCompatScaleInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateCompatScaleInfo");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    Transform transform;
    transform.scaleX_ = 0.5f;
    transform.scaleY_ = 0.6f;
    EXPECT_EQ(window->UpdateCompatScaleInfo(transform), WMError::WM_DO_NOTHING);
    EXPECT_NEAR(window->compatScaleX_, 1.0f, 0.00001f);
    EXPECT_NEAR(window->compatScaleY_, 1.0f, 0.00001f);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(window->UpdateCompatScaleInfo(transform), WMError::WM_DO_NOTHING);
    EXPECT_NEAR(window->compatScaleX_, 1.0f, 0.00001f);
    EXPECT_NEAR(window->compatScaleY_, 1.0f, 0.00001f);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_NEAR(window->compatScaleX_, 1.0f, 0.00001f);
    EXPECT_NEAR(window->compatScaleY_, 1.0f, 0.00001f);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(window->UpdateCompatScaleInfo(transform), WMError::WM_OK);
    EXPECT_NEAR(window->compatScaleX_, transform.scaleX_, 0.00001f);
    EXPECT_NEAR(window->compatScaleY_, transform.scaleY_, 0.00001f);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(window->UpdateCompatScaleInfo(transform), WMError::WM_OK);
}

/**
 * @tc.name: SetCompatInfoInExtensionConfig
 * @tc.desc: SetCompatInfoInExtensionConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetCompatInfoInExtensionConfig, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetCompatInfoInExtensionConfig");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    AAFwk::WantParams want;
    window->SetCompatInfoInExtensionConfig(want);
    bool isAdaptToSimulationScale =
        static_cast<bool>(want.GetIntParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, 0));
    EXPECT_FALSE(isAdaptToSimulationScale);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->SetCompatInfoInExtensionConfig(want);
    isAdaptToSimulationScale =
        static_cast<bool>(want.GetIntParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, 0));
    EXPECT_TRUE(isAdaptToSimulationScale);
}

/**
 * @tc.name: IsAdaptToProportionalScale
 * @tc.desc: IsAdaptToProportionalScale
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsAdaptToProportionalScale, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAdaptToProportionalScale");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    EXPECT_FALSE(window->IsAdaptToProportionalScale());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_TRUE(window->IsAdaptToProportionalScale());
}

/**
 * @tc.name: IsInCompatScaleMode
 * @tc.desc: IsInCompatScaleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsInCompatScaleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsInCompatScaleMode");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    EXPECT_FALSE(window->IsInCompatScaleMode());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_TRUE(window->IsInCompatScaleMode());
    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_TRUE(window->IsInCompatScaleMode());
}

/**
 * @tc.name: IsInCompatScaleStatus
 * @tc.desc: IsInCompatScaleStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsInCompatScaleStatus, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsInCompatScaleStatus");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    EXPECT_FALSE(window->IsInCompatScaleStatus());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_FALSE(window->IsInCompatScaleStatus());
    window->compatScaleX_ = 0.5f;
    EXPECT_TRUE(window->IsInCompatScaleStatus());
    window->compatScaleX_ = 1.0f;
    window->compatScaleX_ = 1.5f;
    EXPECT_TRUE(window->IsInCompatScaleStatus());
}

/**
 * @tc.name: GetPropertyByContext
 * @tc.desc: GetPropertyByContext
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetPropertyByContext, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetPropertyByContext");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    int32_t persistentId = 663;
    window->property_->SetPersistentId(persistentId);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    auto property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), persistentId);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("GetPropertyByContext_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option1);
    int32_t mainPersistentId = 666;
    mainWindow->property_->SetPersistentId(mainPersistentId);
    window->windowSessionMap_.insert({mainWindow->GetWindowName(),
        std::pair<int32_t, sptr<WindowSessionImpl>>(mainPersistentId, mainWindow) });
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    mainWindow->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), persistentId);
    mainWindow->context_ = window->context_;
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), mainPersistentId);

    window->property_->SetIsUIExtensionAbilityProcess(true);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), persistentId);
    window->GetWindowExtensionSessionSet().insert(mainWindow);
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), mainPersistentId);
}

/**
 * @tc.name: IsAdaptToSimulationScale
 * @tc.desc: IsAdaptToSimulationScale
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsAdaptToSimulationScale, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAdaptToSimulationScale");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    window->property_->SetPersistentId(704);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(window->IsAdaptToSimulationScale(), true);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("IsAdaptToSimulationScale_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option1);
    int32_t mainPersistentId = 666;
    mainWindow->property_->SetPersistentId(mainPersistentId);
    window->windowSessionMap_.insert({mainWindow->GetWindowName(),
        std::pair<int32_t, sptr<WindowSessionImpl>>(mainPersistentId, mainWindow) });
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    mainWindow->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    mainWindow->context_ = window->context_;
    EXPECT_EQ(window->IsAdaptToSimulationScale(), false);
}

/**
 * @tc.name: IsAdaptToSubWindow
 * @tc.desc: IsAdaptToSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsAdaptToSubWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAdaptToSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    window->property_->SetPersistentId(772);
    EXPECT_EQ(window->IsAdaptToSubWindow(), false);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSubWindow(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(window->IsAdaptToSubWindow(), true);
}

/**
 * @tc.name: SetIntentParam
 * @tc.desc: SetIntentParam
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetIntentParam, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetIntentParam");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    auto testCallback = [](){};
    bool isColdStart = true;
    std::string intentParam = "testIntent";
    window->SetIntentParam(intentParam, testCallback, isColdStart);
    EXPECT_EQ(window->isColdStart_, true);
    EXPECT_EQ(window->intentParam_, intentParam);
}

/**
 * @tc.name: SetNavDestinationInfo
 * @tc.desc: SetNavDestinationInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetNavDestinationInfo, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetNavDestinationInfo");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    std::string navInfo = "testInfo";
    window->SetNavDestinationInfo(navInfo);
    EXPECT_EQ(window->navDestinationInfo_, navInfo);
}

/**
 * @tc.name: OnNewWant01
 * @tc.desc: OnNewWant01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, OnNewWant01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnNewWant01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    std::string navInfo = "testInfo";
    AAFwk::Want want;
    want.SetParam(AAFwk::Want::ATOMIC_SERVICE_SHARE_ROUTER, navInfo);
    window->OnNewWant(want);

    EXPECT_EQ(window->navDestinationInfo_, "");
}

/**
 * @tc.name: OnNewWant02
 * @tc.desc: OnNewWant when uiContent is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, OnNewWant02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnNewWant02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    std::string navInfo = "testInfo";
    AAFwk::Want want;
    want.SetParam(AAFwk::Want::ATOMIC_SERVICE_SHARE_ROUTER, navInfo);
    window->uiContent_ = nullptr;
    window->OnNewWant(want);

    EXPECT_EQ(window->navDestinationInfo_, "testInfo");
}

/**
 * @tc.name: NapiSetUIContent01
 * @tc.desc: NapiSetUIContent01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NapiSetUIContent01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NapiSetUIContent01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"NapiSetUIContent01", "NapiSetUIContent01", "NapiSetUIContent01"};
    auto hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(1);
    window->property_ = property;
    window->hostSession_ = hostSession;
    sptr<IRemoteObject> token;
    window->state_ = WindowState::STATE_SHOWN;

    window->AniSetUIContent("info", (ani_env*)nullptr, nullptr, BackupAndRestoreType::NONE, nullptr, nullptr);

    std::string navInfo = "testInfo";
    window->SetNavDestinationInfo(navInfo);

    window->NapiSetUIContent("info", (napi_env)nullptr, nullptr, BackupAndRestoreType::NONE, nullptr, nullptr);

    window->SetNavDestinationInfo(navInfo);

    window->AniSetUIContent("info", (ani_env*)nullptr, nullptr, BackupAndRestoreType::NONE, nullptr, nullptr);
    EXPECT_EQ(window->navDestinationInfo_, "");
}

/**
 * @tc.name: SetUIContentInner
 * @tc.desc: SetUIContentInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetUIContentInner, Function | SmallTest | Level2)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUIContentInner");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"SetUIContentInner", "SetUIContentInner", "SetUIContentInner"};
    auto hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(2);
    window->property_ = property;
    window->hostSession_ = hostSession;
    sptr<IRemoteObject> token;
    window->state_ = WindowState::STATE_SHOWN;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    window->iconCache_ = pixelMap;

    window->SetUIContentInner("info", nullptr, nullptr,
        WindowSetUIContentType::DEFAULT, BackupAndRestoreType::NONE, nullptr);
    EXPECT_TRUE(g_errLog.find("Use iconCache to set WindowIcon") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HideTitleButton01
 * @tc.desc: HideTitleButton01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton01, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: HideTitleButton02
 * @tc.desc: HideTitleButton02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton02, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    window->uiContent_ = nullptr;
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: HideTitleButton03
 * @tc.desc: HideTitleButton03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton03, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    option->SetSubWindowMaximizeSupported(false);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    window->property_->SetDecorEnable(true);
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: SetUIContentByName
 * @tc.desc: SetUIContentByName, load content by name
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetUIContentByName, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUIContentByName");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"SetUIContentByName", "SetUIContentByName", "SetUIContentByName"};
    auto hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(1);
    window->property_ = property;
    window->hostSession_ = hostSession;
    sptr<IRemoteObject> token;
    window->state_ = WindowState::STATE_SHOWN;

    auto testCallback = [](){};
    bool isColdStart = true;
    std::string intentParam = "test";
    window->SetIntentParam(intentParam, testCallback, isColdStart);

    window->SetUIContentByName("info", (napi_env)nullptr, nullptr, nullptr);
    EXPECT_EQ(window->intentParam_, "");

    intentParam = "";
    window->SetUIContentByName("info", (napi_env)nullptr, nullptr, nullptr);
    EXPECT_EQ(window->intentParam_, "");
}

/**
 * @tc.name: HideTitleButton04
 * @tc.desc: HideTitleButton04
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton04, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsSupportRotateFullScreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetIsLayoutFullScreen(true);
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: HideTitleButton05
 * @tc.desc: HideTitleButton05
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton05, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableFullScreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: HideTitleButton06
 * @tc.desc: HideTitleButton06
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton06, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetIsAtomicService(false);
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: HideTitleButton07
 * @tc.desc: HideTitleButton07
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton07, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetIsAtomicService(false);
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: HideTitleButton08
 * @tc.desc: HideTitleButton08
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, HideTitleButton08, Function | SmallTest | Level2) {
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool hideSplitButton = false;
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToBackButton(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);

    EXPECT_FALSE(hideSplitButton);
    EXPECT_FALSE(hideMaximizeButton);
    EXPECT_FALSE(hideMinimizeButton);
    EXPECT_FALSE(hideCloseButton);
}

/**
 * @tc.name: GetFloatingBallWindowId
 * @tc.desc: GetFloatingBallWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetFloatingBallWindowId, TestSize.Level1)
{
    uint32_t windowId = 0;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetFloatingBallWindowId");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetFloatingBallWindowId(windowId));

    auto session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    window->property_->persistentId_ = 1234;
    EXPECT_CALL(*session, GetFloatingBallWindowId(_)).Times(1).WillOnce(Return(WMError::WM_ERROR_INVALID_OPERATION));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->GetFloatingBallWindowId(windowId));

    EXPECT_CALL(*session, GetFloatingBallWindowId(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->GetFloatingBallWindowId(windowId));
}

/**
 * @tc.name: SendFbActionEvent
 * @tc.desc: SendFbActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SendFbActionEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SendFbActionEvent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    std::string action = "click";
    ASSERT_EQ(WSError::WS_OK, window->SendFbActionEvent(action));
}

/**
 * @tc.name: UpdateFloatingBall
 * @tc.desc: UpdateFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateFloatingBall, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFloatingBall");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    FloatingBallTemplateBaseInfo fbTemplateInfo;
    std::shared_ptr<Media::PixelMap> icon = nullptr;

    EXPECT_EQ(window->GetHostSession(), nullptr);
    auto error = window->UpdateFloatingBall(fbTemplateInfo, icon);
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, error);

    auto session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    window->property_->persistentId_ = 1234;

    FloatingBallTemplateInfo windowFbTemplateInfo;
    windowFbTemplateInfo.template_ = static_cast<uint32_t>(FloatingBallTemplate::STATIC);
    window->GetProperty()->SetFbTemplateInfo(windowFbTemplateInfo);
    EXPECT_EQ(WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED, window->UpdateFloatingBall(fbTemplateInfo, icon));

    windowFbTemplateInfo.template_ = static_cast<uint32_t>(FloatingBallTemplate::NORMAL);
    window->GetProperty()->SetFbTemplateInfo(windowFbTemplateInfo);
    fbTemplateInfo.template_ = static_cast<uint32_t>(FloatingBallTemplate::STATIC);
    EXPECT_EQ(WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED, window->UpdateFloatingBall(fbTemplateInfo, icon));

    fbTemplateInfo.template_ = static_cast<uint32_t>(FloatingBallTemplate::NORMAL);
    EXPECT_FALSE(window->IsWindowSessionInvalid());
    error = window->UpdateFloatingBall(fbTemplateInfo, icon);
    EXPECT_EQ(WMError::WM_OK, error);
}

/**
 * @tc.name: NotifyPrepareCloseFloatingBall
 * @tc.desc: NotifyPrepareCloseFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyPrepareCloseFloatingBall, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPrepareCloseFloatingBall");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;

    EXPECT_EQ(window->GetHostSession(), nullptr);
    window->NotifyPrepareCloseFloatingBall();

    auto session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    EXPECT_TRUE(window->IsWindowSessionInvalid());
    window->NotifyPrepareCloseFloatingBall();

    window->property_->persistentId_ = 1234;
    EXPECT_FALSE(window->IsWindowSessionInvalid());
    window->NotifyPrepareCloseFloatingBall();
}

/**
 * @tc.name: RestoreFbMainWindow
 * @tc.desc: RestoreFbMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RestoreFbMainWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RestoreFbMainWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;

    std::shared_ptr<AAFwk::Want> want = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, window->RestoreFbMainWindow(want));

    auto session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    window->property_->persistentId_ = 1234;

    EXPECT_CALL(*session, RestoreFbMainWindow(_)).Times(1).WillOnce(Return(WMError::WM_ERROR_FB_STATE_ABNORMALLY));
    ASSERT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, window->RestoreFbMainWindow(want));

    EXPECT_CALL(*session, RestoreFbMainWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->RestoreFbMainWindow(want));
}

/**
 * @tc.name: TestGetGlobalDisplayRect
 * @tc.desc: Get global display rect from window property
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestGetGlobalDisplayRect, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    Rect expected { 100, 200, 300, 400 };
    window->property_->SetGlobalDisplayRect(expected);

    Rect actual = window->GetGlobalDisplayRect();
    EXPECT_EQ(actual, expected);
}

/**
 * @tc.name: TestClientToGlobalDisplay
 * @tc.desc: Convert client position to global display position
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestClientToGlobalDisplay, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    Position inPosition;
    Position outPosition;

    // Case 1: Scaled window does not support coordinate conversion
    Transform transform;
    transform.scaleX_ = 0.5f;
    window->SetCurrentTransform(transform);
    auto ret = window->ClientToGlobalDisplay(inPosition, outPosition);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS);
    transform.scaleX_ = 1.0f;
    transform.scaleY_ = 1.0f;
    window->SetCurrentTransform(transform);

    // Case 2: Conversion overflow
    inPosition = { INT32_MAX, INT32_MAX };
    Rect globalRect { 100, 200, 300, 400 };
    window->property_->SetGlobalDisplayRect(globalRect);
    ret = window->ClientToGlobalDisplay(inPosition, outPosition);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Case 3: Successful conversion
    inPosition = { 10, 20 };
    Position expectedPosition { 110, 220 };
    ret = window->ClientToGlobalDisplay(inPosition, outPosition);
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_NE(outPosition, inPosition);
    EXPECT_EQ(outPosition, expectedPosition);
}

/**
 * @tc.name: TestGlobalDisplayToClient
 * @tc.desc: Convert global display position to client position
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestGlobalDisplayToClient, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    Position inPosition;
    Position outPosition;

    // Case 1: Scaled window does not support coordinate conversion
    Transform transform;
    transform.scaleX_ = 0.5f;
    window->SetCurrentTransform(transform);
    auto ret = window->GlobalDisplayToClient(inPosition, outPosition);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS);
    transform.scaleX_ = 1.0f;
    transform.scaleY_ = 1.0f;
    window->SetCurrentTransform(transform);

    // Case 2: Conversion overflow
    inPosition = { INT32_MIN, INT32_MIN };
    Rect globalRect { 100, 200, 300, 400 };
    window->property_->SetGlobalDisplayRect(globalRect);
    ret = window->GlobalDisplayToClient(inPosition, outPosition);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Case 3: Successful conversion
    inPosition = { 110, 220 };
    Position expectedPosition { 10, 20 };
    ret = window->GlobalDisplayToClient(inPosition, outPosition);
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_NE(outPosition, inPosition);
    EXPECT_EQ(outPosition, expectedPosition);
}

/**
 * @tc.name: TestUpdateGlobalDisplayRectFromServer
 * @tc.desc: Update global display rect if different
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestUpdateGlobalDisplayRectFromServer, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1001);

    WSRect rect = { 10, 20, 200, 100 };
    window->property_->SetGlobalDisplayRect({ 10, 20, 200, 100 });
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;

    // Case 1: No change, should do nothing
    Rect expectedRect { 10, 20, 200, 100 };
    auto ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::RESIZE);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::RESIZE);

    // Case 2: Change reason, should update
    ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::MOVE);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::MOVE);

    // Case 3: Change rect, should update
    WSRect updated = { 30, 40, 200, 100 };
    expectedRect = { 30, 40, 200, 100 };
    ret = window->UpdateGlobalDisplayRectFromServer(updated, SizeChangeReason::MOVE);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::MOVE);

    // Case 4: Change reason and rect, should update
    updated = { 0, 0, 200, 100 };
    expectedRect = { 0, 0, 200, 100 };
    ret = window->UpdateGlobalDisplayRectFromServer(updated, SizeChangeReason::DRAG);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::DRAG);

    // Case 5: Drag move, should update but keep reason as DRAG_MOVE
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::DRAG_MOVE;
    updated = { 20, 20, 200, 100 };
    expectedRect = { 20, 20, 200, 100 };
    ret = window->UpdateGlobalDisplayRectFromServer(updated, SizeChangeReason::DRAG_END);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::DRAG_MOVE);
}

/**
 * @tc.name: TestRegisterRectChangeInGlobalDisplayListener
 * @tc.desc: Register a new listener successfully
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestRegisterRectChangeInGlobalDisplayListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(123);

    // Case 1: Register successfully
    auto listener = sptr<MockRectChangeInGlobalDisplayListener>::MakeSptr();
    auto result = window->RegisterRectChangeInGlobalDisplayListener(listener);
    EXPECT_EQ(result, WMError::WM_OK);

    // Case 2: Register the same listener again
    result = window->RegisterRectChangeInGlobalDisplayListener(listener);
    EXPECT_EQ(result, WMError::WM_OK);

    // Case 3: Register nullptr listener
    sptr<IRectChangeInGlobalDisplayListener> nullListener = nullptr;
    result = window->RegisterRectChangeInGlobalDisplayListener(nullListener);
    EXPECT_EQ(result, WMError::WM_ERROR_NULLPTR);

    {
        std::lock_guard<std::mutex> lock(window->rectChangeInGlobalDisplayListenerMutex_);
        window->rectChangeInGlobalDisplayListeners_.clear();
    }
}

/**
 * @tc.name: TestUnregisterRectChangeInGlobalDisplayListener
 * @tc.desc: Unregister an existing listener successfully
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestUnregisterRectChangeInGlobalDisplayListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(123);

    auto listener = sptr<MockRectChangeInGlobalDisplayListener>::MakeSptr();
    window->RegisterRectChangeInGlobalDisplayListener(listener);

    // Case 1: Unregister successfully
    auto result = window->UnregisterRectChangeInGlobalDisplayListener(listener);
    EXPECT_EQ(result, WMError::WM_OK);

    // Case 2: Unregister the same listener again
    result = window->UnregisterRectChangeInGlobalDisplayListener(listener);
    EXPECT_EQ(result, WMError::WM_OK);

    // Case 3: Unregister nullptr listener
    sptr<IRectChangeInGlobalDisplayListener> nullListener = nullptr;
    result = window->UnregisterRectChangeInGlobalDisplayListener(nullListener);
    EXPECT_EQ(result, WMError::WM_ERROR_NULLPTR);

    {
        std::lock_guard<std::mutex> lock(window->rectChangeInGlobalDisplayListenerMutex_);
        window->rectChangeInGlobalDisplayListeners_.clear();
    }
}

/**
 * @tc.name: TestNotifyGlobalDisplayRectChange
 * @tc.desc: Notify all valid listeners and skip nullptr ones
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, TestNotifyGlobalDisplayRectChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1001);

    auto listener1 = sptr<MockRectChangeInGlobalDisplayListener>::MakeSptr();
    auto listener2 = sptr<MockRectChangeInGlobalDisplayListener>::MakeSptr();
    sptr<IRectChangeInGlobalDisplayListener> nullListener = nullptr;

    {
        std::lock_guard<std::mutex> lock(window->rectChangeInGlobalDisplayListenerMutex_);
        window->rectChangeInGlobalDisplayListeners_[window->GetPersistentId()] = {
            listener1, nullListener, listener2
        };
    }

    Rect rect { 10, 20, 100, 200 };
    WindowSizeChangeReason reason = WindowSizeChangeReason::UNDEFINED;

    EXPECT_CALL(*listener1, OnRectChangeInGlobalDisplay(rect, reason)).Times(1);
    EXPECT_CALL(*listener2, OnRectChangeInGlobalDisplay(rect, reason)).Times(1);

    window->NotifyGlobalDisplayRectChange(rect, reason);

    {
        std::lock_guard<std::mutex> lock(window->rectChangeInGlobalDisplayListenerMutex_);
        window->rectChangeInGlobalDisplayListeners_.clear();
    }
}

/**
 * @tc.name: GetPiPSettingSwitchStatus
 * @tc.desc: GetPiPSettingSwitchStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetPiPSettingSwitchStatus, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetPiPSettingSwitchStatus");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool switchStatus = false;
    WMError retCode = window->GetPiPSettingSwitchStatus(switchStatus);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->GetPiPSettingSwitchStatus(switchStatus);
}

/**
 * @tc.name: OnPointDown
 * @tc.desc: OnPointDown
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, OnPointDown, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnPointDown");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));
    EXPECT_TRUE(window->IsAnco());

    EXPECT_EQ(window->GetHostSession(), nullptr);
    EXPECT_FALSE(window->OnPointDown(0, 0, 0));

    SessionInfo sessionInfo = {"OnPointDown", "OnPointDown", "OnPointDown"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*(session), ProcessPointDownSession(_, _)).Times(1).WillOnce(Return(WSError::WS_OK));
    EXPECT_TRUE(window->OnPointDown(0, 0, 0));
}

/**
 * @tc.name: SwitchSubWindow
 * @tc.desc: SwitchSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SwitchSubWindow, Function | SmallTest | Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("SwitchSubWindow");
    subOption->SetSubWindowDecorEnable(true);
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(subWindow, nullptr);
    ASSERT_NE(subWindow->property_, nullptr);
    subWindow->property_->SetPersistentId(PERSISTENT_ID_TWO);
    subWindow->property_->SetDecorEnable(true);
    subWindow->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    subWindow->windowSystemConfig_.isSystemDecorEnable_ = true;
    subWindow->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    // freemultiwindowmode start
    EXPECT_EQ(subWindow->IsDecorEnable(), false);
    // cover empty map
    subWindow->SwitchSubWindow(false, PERSISTENT_ID_ONE);

    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(PERSISTENT_ID_ONE, vec));
    WindowSessionImpl::subWindowSessionMap_[PERSISTENT_ID_ONE].push_back(subWindow);
    subWindow->SwitchSubWindow(true, PERSISTENT_ID_ONE);
    WindowMode mode = subWindow->property_->GetWindowMode();
    bool decorVisible = mode == WindowMode::WINDOW_MODE_FLOATING ||
        mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
        (mode == WindowMode::WINDOW_MODE_FULLSCREEN && !subWindow->property_->IsLayoutFullScreen());
    if (subWindow->windowSystemConfig_.freeMultiWindowSupport_) {
        auto isSubWindow = WindowHelper::IsSubWindow(subWindow->GetType());
        decorVisible = decorVisible && (subWindow->windowSystemConfig_.freeMultiWindowEnable_ ||
            (subWindow->property_->GetIsPcAppInPad() && isSubWindow));
    }
    EXPECT_EQ(decorVisible, true);
}

/**
 * @tc.name: IsPiPActive
 * @tc.desc: IsPiPActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsPiPActive, Function | SmallTest | Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool status = false;
    EXPECT_EQ(window->IsPiPActive(status), WMError::WM_ERROR_PIP_INTERNAL_ERROR);

    SessionInfo sessionInfo = {"IsPiPActive", "IsPiPActive", "IsPiPActive"};
    auto hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = hostSession;

    EXPECT_EQ(window->IsPiPActive(status), WMError::WM_OK);
}

/**
 * @tc.name: NotifySizeChangeFlag
 * @tc.desc: NotifySizeChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifySizeChangeFlag, Function | SmallTest | Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySizeChangeFlag");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Rect requestRect = { 0, 0, 50, 50 };
    Rect windowRect = { 0, 0, 0, 0 };
    window->property_->SetWindowRect(windowRect);
    window->property_->SetRequestRect(requestRect);

    window->SetNotifySizeChangeFlag(false);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_NAVIGATION);
    window->SetNotifySizeChangeFlag(true);
    ASSERT_EQ(window->notifySizeChangeFlag_, true);

    window->SetNotifySizeChangeFlag(false);
    window->property_->SetWindowRect(requestRect);
    window->SetNotifySizeChangeFlag(true);
    ASSERT_EQ(window->notifySizeChangeFlag_, false);

    window->property_->SetWindowRect(windowRect);
    window->SetNotifySizeChangeFlag(false);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->SetNotifySizeChangeFlag(true);
    ASSERT_EQ(window->notifySizeChangeFlag_, true);
}

/**
 * @tc.name: UpdateIsShowDecorInFreeMultiWindow
 * @tc.desc: UpdateIsShowDecorInFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateIsShowDecorInFreeMultiWindow, Function | SmallTest | Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateIsShowDecorInFreeMultiWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool isShow = true;
    ASSERT_EQ(WSError::WS_OK, window->UpdateIsShowDecorInFreeMultiWindow(isShow));
}

/**
 * @tc.name: GetListeners
 * @tc.desc: GetListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetListeners, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetListeners");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(502);
    window->windowTitleChangeListeners_[502] = std::vector<sptr<IWindowTitleChangeListener>>();
 
    ASSERT_FALSE(window_->windowTitleChangeListeners_[window_->GetPersistentId()].empty());
}
 
/**
 * @tc.name: NotifyTitleChange
 * @tc.desc: NotifyTitleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyTitleChange01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyTitleChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));
 
    SessionInfo sessionInfo = {"NotifyTitleBundle", "NotifyTitleModule", "NotifyTitleAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    bool isShow = false;
    int32_t height = 0;
    window->NotifyTitleChange(isShow, height);
 
    sptr<IWindowTitleChangeListener> listener = sptr<IWindowTitleChangeListener>::MakeSptr();
    window->RegisterWindowTitleChangeListener(listener);
    window->NotifyTitleChange(isShow, height);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: NotifyTitleChange
 * @tc.desc: NotifyTitleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyTitleChange02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyTitleChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::DEFAULT_TYPE));
 
    SessionInfo sessionInfo = {"NotifyTitleBundle", "NotifyTitleModule", "NotifyTitleAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
}

/**
 * @tc.name: NotifyTitleChange
 * @tc.desc: NotifyTitleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyTitleChange03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyTitleChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));
 
    SessionInfo sessionInfo = {"NotifyTitleBundle", "NotifyTitleModule", "NotifyTitleAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    bool isShow = false;
    int32_t height = 0;
    window->NotifyTitleChange(isShow, height);
 
    sptr<IWindowTitleOrHotAreasListener> listener = sptr<IWindowTitleOrHotAreasListener>::MakeSptr();
    window->RegisterWindowTitleOrHotAreasListener(listener);
    window->NotifyTitleChange(isShow, height);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}
 
/**
 * @tc.name: IsHitTitleBar
 * @tc.desc: IsHitTitleBar
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsHitTitleBar01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsHitTitleBar01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    window->uiContent_ = nullptr;
    bool isHitTitleBar = window->IsHitTitleBar(pointerEvent);
    EXPECT_EQ(isHitTitleBar, false);
 
    
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->uiContent_->SetContainerModalTitleHeight(100);
    MMI::PointerEvent::PointerItem pointerItem;
    Rect rect = { 0, 0, 200, 200 };
    window->property_->SetWindowRect(rect);
    pointerItem.SetDisplayX(100);
    pointerItem.SetDisplayY(50);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    isHitTitleBar = window->IsHitTitleBar(pointerEvent);
    EXPECT_EQ(isHitTitleBar, false);
}
 
/**
 * @tc.name: IsHitTitleBar
 * @tc.desc: IsHitTitleBar
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsHitTitleBar02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsHitTitleBar02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    window->uiContent_ = nullptr;
    bool isHitTitleBar = window->IsHitTitleBar(pointerEvent);
    EXPECT_EQ(isHitTitleBar, false);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    MMI::PointerEvent::PointerItem pointerItem;
    Rect rect{ 0, 0, 1, 1 };
    window->property_->SetWindowRect(rect);
    pointerItem.SetDisplayX(100);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    isHitTitleBar = window->IsHitTitleBar(pointerEvent);
    EXPECT_EQ(isHitTitleBar, false);
}

/**
 * @tc.name: RegisterWindowTitleChangeListener
 * @tc.desc: RegisterWindowTitleChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterWindowTitleChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterWindowTitleChangeListener");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowTitleChangeListener> listener = nullptr;
    WMError ret = window->RegisterWindowTitleChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: RegisterWindowTitleOrHotAreasListener
 * @tc.desc: RegisterWindowTitleOrHotAreasListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterWindowTitleOrHotAreasListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterWindowTitleOrHotAreasListener");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowTitleOrHotAreasListener> listener = nullptr;
    WMError ret = window->RegisterWindowTitleOrHotAreasListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}
 
/**
 * @tc.name: GetListeners
 * @tc.desc: GetListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetTitleOrHotAreasListeners, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetListeners");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(502);
    window->windowTitleOrHotAreasListeners_[502] = std::vector<sptr<IWindowTitleOrHotAreasListener>>();
 
    ASSERT_FALSE(window_->windowTitleOrHotAreasListeners_[window_->GetPersistentId()].empty());
}
 
/**
 * @tc.name: IsHitHotAreas
 * @tc.desc: IsHitHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsHitHotAreas01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsHitHotAreas01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    window->uiContent_ = nullptr;
    bool isHitHotAreas = window->IsHitHotAreas(pointerEvent);
    EXPECT_EQ(isHitHotAreas, false);
 
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->uiContent_->SetContainerModalTitleHeight(100);
    MMI::PointerEvent::PointerItem pointerItem;
    Rect rect = { 0, 0, 200, 200 };
    window->property_->SetWindowRect(rect);
    pointerItem.SetDisplayX(0);
    pointerItem.SetDisplayY(0);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    isHitHotAreas = window->IsHitHotAreas(pointerEvent);
    EXPECT_EQ(isHitHotAreas, true);
}
 
/**
 * @tc.name: IsHitHotAreas
 * @tc.desc: IsHitHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsHitHotAreas02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsHitHotAreas02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    window->uiContent_ = nullptr;
    bool isHitHotAreas = window->IsHitHotAreas(pointerEvent);
    EXPECT_EQ(isHitHotAreas, false);
 
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->uiContent_->SetContainerModalTitleHeight(100);
    MMI::PointerEvent::PointerItem pointerItem;
    Rect rect = { 0, 0, 200, 200 };
    window->property_->SetWindowRect(rect);
    pointerItem.SetDisplayX(1000);
    pointerItem.SetDisplayY(1000);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    isHitHotAreas = window->IsHitHotAreas(pointerEvent);
    EXPECT_EQ(isHitHotAreas, false);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
