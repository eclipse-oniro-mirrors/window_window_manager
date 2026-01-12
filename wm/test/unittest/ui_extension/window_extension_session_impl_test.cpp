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

#include <ability_context_impl.h>
#include <context_impl.h>
#include <float_wrapper.h>
#include <int_wrapper.h>
#include <transaction/rs_transaction.h>
#include <want_params_wrapper.h>

#include "accessibility_event_info.h"
#include "ani.h"
#include "display_info.h"
#include "extension/extension_business_info.h"
#include "extension_data_handler.h"
#include "extension_data_handler_mock.h"
#include "iremote_object_mocker.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "ui_extension/provider_data_handler.h"
#include "window_extension_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Accessibility;
using namespace std;

namespace {
    std::string logMsg;
    void WindowExtensionSessionImplLogCallback(const LogType type, const LogLevel level, const unsigned int domain,
        const char* tag, const char* msg)
    {
        logMsg = msg;
    }
}

namespace OHOS {
namespace Rosen {
using WindowAdapterMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
class WindowExtensionSessionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    sptr<WindowExtensionSessionImpl> window_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowExtensionSessionImplTest::SetUpTestCase()
{
}

void WindowExtensionSessionImplTest::TearDownTestCase()
{
}

void WindowExtensionSessionImplTest::SetUp()
{
    sptr<WindowOption> option = new(std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("WindowExtensionSessionImplTest");
    window_ = new(std::nothrow) WindowExtensionSessionImpl(option);
    ASSERT_NE(nullptr, window_);
    if (!handler_) {
        auto runner = AppExecFwk::EventRunner::Create("WindowExtensionSessionImplTest");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    window_->handler_ = handler_;
}

void WindowExtensionSessionImplTest::TearDown()
{
    window_ = nullptr;
}

namespace {
/**
 * @tc.name: WindowExtensionSessionImpl
 * @tc.desc: WindowExtensionSessionImpl contructor
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, WindowExtensionSessionImpl, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->uiExtensionUsage_ = static_cast<uint32_t>(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    option->SetWindowName("WindowExtensionSessionImplTest");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    window = nullptr;
}

/**
 * @tc.name: Create01
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Create01, TestSize.Level0)
{
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_OK, window_->Create(abilityContext, session));
    ASSERT_EQ(WMError::WM_OK, window_->Destroy(false));
}

/**
 * @tc.name: Create02
 * @tc.desc: context is nullptr, session is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Create02, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->Create(nullptr, nullptr));
}

/**
 * @tc.name: Create03
 * @tc.desc: context is not nullptr, session is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Create03, TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->Create(abilityContext, nullptr));
}

/**
 * @tc.name: Create04
 * @tc.desc: connet failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Create04, TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    EXPECT_CALL(*session, Connect).WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->Create(abilityContext, session));
}

/**
 * @tc.name: Create05
 * @tc.desc: normal test, create modal uiextension
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Create05, TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window_->property_->SetPersistentId(1);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    ASSERT_EQ(WMError::WM_OK, window_->Create(abilityContext, session));
    ASSERT_EQ(WMError::WM_OK, window_->Destroy(false));
}

/**
 * @tc.name: Create06
 * @tc.desc: normal test, create secure uiextension
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Create06, TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window_->property_->SetPersistentId(1);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    ASSERT_EQ(WMError::WM_OK, window_->Create(abilityContext, session));
    ASSERT_EQ(WMError::WM_OK, window_->Destroy(false));
}

/**
 * @tc.name: Destroy01
 * @tc.desc: Destroy Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Destroy01, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window_->hostSession_ = session;
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    window_->dataHandler_ = std::make_shared<Extension::ProviderDataHandler>();
    ASSERT_EQ(WMError::WM_OK, window_->Destroy(false, false));
}

/**
 * @tc.name: Destroy02
 * @tc.desc: Destroy Test, window session is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Destroy02, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_->property_);
    window_->hostSession_ = nullptr;
    window_->property_->SetPersistentId(0);
    window_->state_= WindowState::STATE_DESTROYED;
    ASSERT_NE(WMError::WM_OK, window_->Destroy(false, false));
}

/**
 * @tc.name: AddExtensionWindowStageToSCB
 * @tc.desc: AddExtensionWindowStageToSCB Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, AddExtensionWindowStageToSCB, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->AddExtensionWindowStageToSCB();

    sptr<IRemoteObject> iRemoteObject = new IRemoteObjectMocker();
    ASSERT_NE(nullptr, iRemoteObject);
    window_->abilityToken_ = iRemoteObject;
    window_->AddExtensionWindowStageToSCB();

    window_->surfaceNode_ = nullptr;
    window_->AddExtensionWindowStageToSCB();
}

/**
 * @tc.name: RemoveExtensionWindowStageFromSCB
 * @tc.desc: RemoveExtensionWindowStageFromSCB Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RemoveExtensionWindowStageFromSCB, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->RemoveExtensionWindowStageFromSCB();

    sptr<IRemoteObject> iRemoteObject = new IRemoteObjectMocker();
    ASSERT_NE(nullptr, iRemoteObject);
    window_->abilityToken_ = iRemoteObject;
    window_->RemoveExtensionWindowStageFromSCB();
}

/**
 * @tc.name: UpdateConfiguration01
 * @tc.desc: UpdateConfiguration Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfiguration01, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    ASSERT_NE(nullptr, window_);
    window_->UpdateConfiguration(configuration);
}

/**
 * @tc.name: UpdateConfiguration02
 * @tc.desc: UpdateConfiguration Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfiguration02, TestSize.Level1)
{
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    window_->UpdateConfiguration(configuration);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateConfigurationForSpecified02
 * @tc.desc: UpdateConfigurationForSpecified02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfigurationForSpecified02, TestSize.Level1)
{
    window_->uiContent_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager;
    window_->UpdateConfigurationForSpecified(configuration, resourceManager);
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    window_->UpdateConfigurationForSpecified(configuration, resourceManager);
    configuration = std::make_shared<AppExecFwk::Configuration>();
    window_->UpdateConfigurationForSpecified(configuration, resourceManager);
}

/**
 * @tc.name: UpdateDefaultStatusBarColor
 * @tc.desc: UpdateDefaultStatusBarColor Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateDefaultStatusBarColor, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->UpdateDefaultStatusBarColor();
    window_->property_->isAtomicService_ = true;
    window_->UpdateDefaultStatusBarColor();
    window_->specifiedColorMode_ = "light";
    window_->UpdateDefaultStatusBarColor();
}

/**
 * @tc.name: UpdateConfigurationForAll01
 * @tc.desc: UpdateConfigurationForAll01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfigurationForAll01, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    ASSERT_NE(nullptr, window_);
    window_->UpdateConfigurationForAll(configuration);
}

/**
 * @tc.name: UpdateConfigurationForAll02
 * @tc.desc: UpdateConfigurationForAll02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfigurationForAll02, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    ASSERT_NE(nullptr, window_);
    window_->GetWindowExtensionSessionSet().insert(window_);
    window_->UpdateConfigurationForAll(configuration);
    window_->GetWindowExtensionSessionSet().erase(window_);
}

/**
 * @tc.name: UpdateConfigurationForAll03
 * @tc.desc: UpdateConfigurationForAll03 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfigurationForAll03, TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_OK, window_->Create(abilityContext, session));
    ASSERT_NE(nullptr, window_);
    window_->GetWindowExtensionSessionSet().insert(window_);
    std::vector<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowContexts;
    ignoreWindowContexts.push_back(abilityContext);
    window_->UpdateConfigurationForAll(configuration, ignoreWindowContexts);
    window_->GetWindowExtensionSessionSet().erase(window_);
}

/**
 * @tc.name: MoveTo01
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, MoveTo01, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window_->MoveTo(0, 1));
}

/**
 * @tc.name: MoveTo02
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, MoveTo02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_OK, window_->MoveTo(0, 1));
}

/**
 * @tc.name: Resize01
 * @tc.desc: Resize
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Resize01, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window_->Resize(0, 1));
}

/**
 * @tc.name: Resize02
 * @tc.desc: Resize
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Resize02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_OK, window_->Resize(0, 1));
}

/**
 * @tc.name: TransferAbilityResult01
 * @tc.desc: TransferAbilityResult
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TransferAbilityResult01, TestSize.Level1)
{
    AAFwk::Want want;
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window_->TransferAbilityResult(1, want));
}

/**
 * @tc.name: TransferAbilityResult02
 * @tc.desc: TransferAbilityResult
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TransferAbilityResult02, TestSize.Level0)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    AAFwk::Want want;
    ASSERT_EQ(WMError::WM_OK, window_->TransferAbilityResult(1, want));
}

/**
 * @tc.name: TransferExtensionData01
 * @tc.desc: TransferExtensionData
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TransferExtensionData01, TestSize.Level1)
{
    AAFwk::WantParams wantParams;
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window_->TransferExtensionData(wantParams));
}

/**
 * @tc.name: TransferExtensionData02
 * @tc.desc: TransferExtensionData
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TransferExtensionData02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    AAFwk::WantParams wantParams;
    ASSERT_EQ(WMError::WM_OK, window_->TransferExtensionData(wantParams));
}

/**
 * @tc.name: RegisterTransferComponentDataListener01
 * @tc.desc: RegisterTransferComponentDataListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterTransferComponentDataListener01, TestSize.Level1)
{
    NotifyTransferComponentDataFunc func;
    window_->RegisterTransferComponentDataListener(func);
    ASSERT_EQ(nullptr, window_->notifyTransferComponentDataFunc_);
}

/**
 * @tc.name: RegisterTransferComponentDataListener02
 * @tc.desc: RegisterTransferComponentDataListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterTransferComponentDataListener02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    NotifyTransferComponentDataFunc func;
    window_->RegisterTransferComponentDataListener(func);
}

/**
 * @tc.name: NotifyTransferComponentData01
 * @tc.desc: NotifyTransferComponentData Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyTransferComponentData01, TestSize.Level1)
{
    AAFwk::WantParams wantParams;
    ASSERT_EQ(WSError::WS_OK, window_->NotifyTransferComponentData(wantParams));
}

/**
 * @tc.name: NotifyTransferComponentData02
 * @tc.desc: NotifyTransferComponentData Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyTransferComponentData02, TestSize.Level0)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    NotifyTransferComponentDataFunc func = [](const AAFwk::WantParams& wantParams) -> AAFwk::WantParams {
        AAFwk::WantParams retWantParams;
        return retWantParams;
    };
    window_->RegisterTransferComponentDataListener(func);
    AAFwk::WantParams wantParams;
    ASSERT_EQ(WSError::WS_OK, window_->NotifyTransferComponentData(wantParams));
}

/**
 * @tc.name: NotifyTransferComponentDataSync01
 * @tc.desc: NotifyTransferComponentDataSync Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyTransferComponentDataSync01, TestSize.Level1)
{
    AAFwk::WantParams wantParams;
    AAFwk::WantParams reWantParams;
    ASSERT_EQ(WSErrorCode::WS_ERROR_NOT_REGISTER_SYNC_CALLBACK,
        window_->NotifyTransferComponentDataSync(wantParams, reWantParams));
}

/**
 * @tc.name: NotifyTransferComponentDataSync02
 * @tc.desc: NotifyTransferComponentDataSync Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyTransferComponentDataSync02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    NotifyTransferComponentDataForResultFunc func = [](const AAFwk::WantParams& wantParams) -> AAFwk::WantParams {
        AAFwk::WantParams retWantParams;
        return retWantParams;
    };
    window_->RegisterTransferComponentDataForResultListener(func);
    AAFwk::WantParams wantParams;
    AAFwk::WantParams reWantParams;
    ASSERT_EQ(WSErrorCode::WS_OK,
        window_->NotifyTransferComponentDataSync(wantParams, reWantParams));
}

/**
 * @tc.name: RegisterTransferComponentDataForResultListener01
 * @tc.desc: RegisterTransferComponentDataForResultListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterTransferComponentDataForResultListen01, TestSize.Level1)
{
    NotifyTransferComponentDataForResultFunc func;
    window_->RegisterTransferComponentDataForResultListener(func);
    ASSERT_EQ(nullptr, window_->notifyTransferComponentDataForResultFunc_);
}

/**
 * @tc.name: RegisterTransferComponentDataForResultListener02
 * @tc.desc: RegisterTransferComponentDataForResultListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterTransferComponentDataForResultListen02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    NotifyTransferComponentDataForResultFunc func;
    window_->RegisterTransferComponentDataForResultListener(func);
}

/**
 * @tc.name: RegisterHostWindowRectChangeListener
 * @tc.desc: RegisterHostWindowRectChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterHostWindowRectChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterHostWindowRectChangeListener");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    window->dataHandler_ = nullptr;
    sptr<IWindowRectChangeListener> listener = nullptr;

    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->RegisterHostWindowRectChangeListener(listener));
    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, window->RegisterHostWindowRectChangeListener(listener));
    listener = sptr<MockWindowRectChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window->RegisterHostWindowRectChangeListener(listener));
    // Test listener alreday registered
    EXPECT_EQ(WMError::WM_OK, window->RegisterHostWindowRectChangeListener(listener));
}

/**
 * @tc.name: UnregisterHostWindowRectChangeListener
 * @tc.desc: UnregisterHostWindowRectChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UnregisterHostWindowRectChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterHostWindowRectChangeListener");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    window->dataHandler_ = nullptr;
    sptr<IWindowRectChangeListener> listener = nullptr;

    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, window->UnregisterHostWindowRectChangeListener(listener));
    listener = sptr<MockWindowRectChangeListener>::MakeSptr();
    window->rectChangeUIExtListenerIds_.emplace(111);
    ASSERT_FALSE(window->rectChangeUIExtListenerIds_.empty());
    EXPECT_EQ(WMError::WM_OK, window->UnregisterHostWindowRectChangeListener(listener));
    window->rectChangeUIExtListenerIds_.clear();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->UnregisterHostWindowRectChangeListener(listener));
    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    EXPECT_EQ(WMError::WM_OK, window->UnregisterHostWindowRectChangeListener(listener));
}

/**
 * @tc.name: RegisterRectChangeInGlobalDisplayListener
 * @tc.desc: RegisterRectChangeInGlobalDisplayListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterRectChangeInGlobalDisplayListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterRectChangeInGlobalDisplayListener");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    window->dataHandler_ = nullptr;
    sptr<IRectChangeInGlobalDisplayListener> listener = nullptr;

    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->RegisterRectChangeInGlobalDisplayListener(listener));
    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, window->RegisterRectChangeInGlobalDisplayListener(listener));
    listener = sptr<IRectChangeInGlobalDisplayListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window->RegisterRectChangeInGlobalDisplayListener(listener));
    // Test listener alreday registered
    EXPECT_EQ(WMError::WM_OK, window->RegisterRectChangeInGlobalDisplayListener(listener));
}

/**
 * @tc.name: UnregisterRectChangeInGlobalDisplayListener
 * @tc.desc: UnregisterRectChangeInGlobalDisplayListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UnregisterRectChangeInGlobalDisplayListener, TestSize.Level1)
{
    window_->dataHandler_ = nullptr;
    sptr<IRectChangeInGlobalDisplayListener> listener = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, window_->UnregisterRectChangeInGlobalDisplayListener(listener));

    listener = sptr<IRectChangeInGlobalDisplayListener>::MakeSptr();
    window_->rectChangeInGlobalDisplayUIExtListenerIds_.emplace(111);
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterRectChangeInGlobalDisplayListener(listener));
    window_->rectChangeInGlobalDisplayUIExtListenerIds_.clear();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window_->UnregisterRectChangeInGlobalDisplayListener(listener));
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterRectChangeInGlobalDisplayListener(listener));
}

/**
 * @tc.name: TriggerBindModalUIExtension01
 * @tc.desc: TriggerBindModalUIExtension01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TriggerBindModalUIExtension01, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->TriggerBindModalUIExtension();
}

/**
 * @tc.name: TriggerBindModalUIExtension02
 * @tc.desc: TriggerBindModalUIExtension02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TriggerBindModalUIExtension02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    window_->TriggerBindModalUIExtension();
}

/**
 * @tc.name: SetPrivacyMode01
 * @tc.desc: SetPrivacyMode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetPrivacyMode01, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_OK, window_->SetPrivacyMode(false));
    ASSERT_FALSE(window_->extensionWindowFlags_.privacyModeFlag);
    ASSERT_EQ(WMError::WM_OK, window_->SetPrivacyMode(true));
    ASSERT_TRUE(window_->extensionWindowFlags_.privacyModeFlag);
}

/**
 * @tc.name: SetPrivacyMode02
 * @tc.desc: SetPrivacyMod
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetPrivacyMod02, TestSize.Level1)
{
    bool isPrivacyMode = true;
    window_->surfaceNode_ = nullptr;
    auto ret = window_->SetPrivacyMode(isPrivacyMode);

    struct RSSurfaceNodeConfig config;

    if (ret == WMError::WM_ERROR_NULLPTR) {
        window_->surfaceNode_ = RSSurfaceNode::Create(config);
    }
    ASSERT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetPrivacyMod03
 * @tc.desc: SetPrivacyMod03
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetPrivacyMod03, TestSize.Level1)
{
    struct RSSurfaceNodeConfig config;
    window_->surfaceNode_ = RSSurfaceNode::Create(config);
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_NE(WMError::WM_OK, window_->SetPrivacyMode(true));
}

/**
 * @tc.name: SetPrivacyMod04
 * @tc.desc: SetPrivacyMod04
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetPrivacyMod04, TestSize.Level1)
{
    struct RSSurfaceNodeConfig config;
    window_->surfaceNode_ = RSSurfaceNode::Create(config);
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_NE(WMError::WM_ERROR_INVALID_WINDOW, window_->SetPrivacyMode(false));
}

/**
 * @tc.name: SetPrivacyMod05
 * @tc.desc: SetPrivacyMod05
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetPrivacyMod05, TestSize.Level1)
{
    struct RSSurfaceNodeConfig config;
    window_->surfaceNode_ = RSSurfaceNode::Create(config);
    window_->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(WMError::WM_OK, window_->SetPrivacyMode(true));
}

HWTEST_F(WindowExtensionSessionImplTest, HidePrivacyContentForHost, TestSize.Level1)
{
    struct RSSurfaceNodeConfig config;
    window_->surfaceNode_ = RSSurfaceNode::Create(config);
    SessionInfo sessionInfo;
    window_->hostSession_ = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_EQ(WMError::WM_OK, window_->HidePrivacyContentForHost(true));
}

/**
 * @tc.name: NotifyFocusStateEvent01
 * @tc.desc: NotifyFocusStateEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyFocusStateEvent01, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->NotifyFocusStateEvent(false);
}

/**
 * @tc.name: NotifyFocusStateEvent02
 * @tc.desc: NotifyFocusStateEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyFocusStateEvent02, TestSize.Level1)
{
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    window_->NotifyFocusStateEvent(true);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyFocusActiveEvent01
 * @tc.desc: NotifyFocusActiveEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyFocusActiveEvent01, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->NotifyFocusActiveEvent(false);
}

/**
 * @tc.name: NotifyFocusActiveEvent02
 * @tc.desc: NotifyFocusActiveEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyFocusActiveEvent02, TestSize.Level1)
{
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    window_->NotifyFocusActiveEvent(true);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyBackpressedEvent01
 * @tc.desc: NotifyFocusActiveEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyBackpressedEvent01, TestSize.Level1)
{
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    bool isConsumed = false;
    window_->NotifyBackpressedEvent(isConsumed);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyBackpressedEvent02
 * @tc.desc: NotifyFocusActiveEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyBackpressedEvent02, TestSize.Level1)
{
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    bool isConsumed = true;
    window_->NotifyBackpressedEvent(isConsumed);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyBackpressedEvent03
 * @tc.desc: NotifyFocusActiveEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyBackpressedEvent03, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->uiContent_ = nullptr;
    bool isConsumed = true;
    window_->NotifyBackpressedEvent(isConsumed);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback01
 * @tc.desc: InputMethodKeyEventResultCallback01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback01, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool consumed = false;
    auto isConsumedPromise = std::make_shared<std::promise<bool>>();
    auto isTimeout = std::make_shared<bool>(false);
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(keyEvent, consumed, isConsumedPromise, isTimeout);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback02
 * @tc.desc: InputMethodKeyEventResultCallback02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback02, TestSize.Level1)
{
    bool consumed = false;
    auto isConsumedPromise = std::make_shared<std::promise<bool>>();
    auto isTimeout = std::make_shared<bool>(false);
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(nullptr, consumed, isConsumedPromise, isTimeout);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback03
 * @tc.desc: InputMethodKeyEventResultCallback03 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback03, TestSize.Level1)
{
    bool consumed = false;
    auto isTimeout = std::make_shared<bool>(false);
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(nullptr, consumed, nullptr, isTimeout);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback04
 * @tc.desc: InputMethodKeyEventResultCallback04 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback04, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool consumed = false;
    auto isTimeout = std::make_shared<bool>(false);
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(keyEvent, consumed, nullptr, isTimeout);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback05
 * @tc.desc: InputMethodKeyEventResultCallback05 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback05, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool consumed = false;
    auto isConsumedPromise = std::make_shared<std::promise<bool>>();
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(keyEvent, consumed, isConsumedPromise, nullptr);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback06
 * @tc.desc: InputMethodKeyEventResultCallback06 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback06, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    auto isConsumedPromise = std::make_shared<std::promise<bool>>();
    auto isTimeout = std::make_shared<bool>(false);
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(keyEvent, true, isConsumedPromise, isTimeout);
}

/**
 * @tc.name: InputMethodKeyEventResultCallback07
 * @tc.desc: InputMethodKeyEventResultCallback07 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, InputMethodKeyEventResultCallback07, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    auto isConsumedPromise = std::make_shared<std::promise<bool>>();
    auto isTimeout = std::make_shared<bool>(true);
    ASSERT_NE(nullptr, window_);
    window_->InputMethodKeyEventResultCallback(keyEvent, true, isConsumedPromise, isTimeout);
}

/**
 * @tc.name: NotifyKeyEvent01
 * @tc.desc: NotifyKeyEvent01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyKeyEvent01, TestSize.Level0)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_FN);
    bool consumed = false;
    bool notifyInputMethod = true;
    ASSERT_NE(nullptr, window_);
    window_->NotifyKeyEvent(keyEvent, consumed, notifyInputMethod);
}

/**
 * @tc.name: NotifyKeyEvent02
 * @tc.desc: NotifyKeyEvent02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyKeyEvent02, TestSize.Level1)
{
    bool consumed = false;
    bool notifyInputMethod = true;
    ASSERT_NE(nullptr, window_);
    window_->NotifyKeyEvent(nullptr, consumed, notifyInputMethod);
}

/**
 * @tc.name: NotifyKeyEvent03
 * @tc.desc: NotifyKeyEvent03 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyKeyEvent03, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool consumed = false;
    bool notifyInputMethod = true;
    ASSERT_NE(nullptr, window_);
    window_->NotifyKeyEvent(keyEvent, consumed, notifyInputMethod);
}

/**
 * @tc.name: NotifyKeyEvent04
 * @tc.desc: NotifyKeyEvent04 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyKeyEvent04, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_FN);
    bool consumed = false;
    bool notifyInputMethod = false;
    ASSERT_NE(nullptr, window_);
    window_->NotifyKeyEvent(keyEvent, consumed, notifyInputMethod);
}

/**
 * @tc.name: NotifyKeyEvent05
 * @tc.desc: NotifyKeyEvent05 Test branch: uiExtensionUsage_ == UIExtensionUsage::PREVIEW_EMBEDDED
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyKeyEvent05, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool consumed = false;
    bool notifyInputMethod = false;
    ASSERT_NE(nullptr, window_);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::PREVIEW_EMBEDDED);
    window_->NotifyKeyEvent(keyEvent, consumed, notifyInputMethod);
    ASSERT_EQ(false, consumed);
}

/**
 * @tc.name: ArkUIFrameworkSupport01
 * @tc.desc: ArkUIFrameworkSupport01 Test, context_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ArkUIFrameworkSupport01, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->context_ = nullptr;
    window_->ArkUIFrameworkSupport();
}

/**
 * @tc.name: ArkUIFrameworkSupport02
 * @tc.desc: ArkUIFrameworkSupport02 Test, context_->GetApplicationInfo() == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ArkUIFrameworkSupport02, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    abilityContext->stageContext_ = nullptr;
    window_->context_ = abilityContext;
    window_->ArkUIFrameworkSupport();
}

/**
 * @tc.name: ArkUIFrameworkSupport03
 * @tc.desc: ArkUIFrameworkSupport03 Test, version < 10 and isSystembarPropertiesSet_ is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ArkUIFrameworkSupport03, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->context_ = nullptr;
    window_->isSystembarPropertiesSet_ = true;
    window_->ArkUIFrameworkSupport();
}

/**
 * @tc.name: ArkUIFrameworkSupport04
 * @tc.desc: ArkUIFrameworkSupport04 Test, version < 10 and isSystembarPropertiesSet_ is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ArkUIFrameworkSupport04, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->context_ = nullptr;
    window_->isSystembarPropertiesSet_ = false;
    window_->ArkUIFrameworkSupport();
}

/**
 * @tc.name: ArkUIFrameworkSupport05
 * @tc.desc: ArkUIFrameworkSupport05 Test, version >= 10 and isIgnoreSafeAreaNeedNotify_ is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ArkUIFrameworkSupport05, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    auto stageContent = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(nullptr, stageContent);
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    ASSERT_NE(nullptr, applicationInfo);
    applicationInfo->apiCompatibleVersion = 12;
    stageContent->SetApplicationInfo(applicationInfo);
    abilityContext->stageContext_ = stageContent;
    window_->context_ = abilityContext;
    window_->isIgnoreSafeAreaNeedNotify_ = false;
    window_->ArkUIFrameworkSupport();
}

/**
 * @tc.name: ArkUIFrameworkSupport06
 * @tc.desc: ArkUIFrameworkSupport06 Test, version >= 10 and isIgnoreSafeAreaNeedNotify_ is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ArkUIFrameworkSupport06, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    auto stageContent = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(nullptr, stageContent);
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    ASSERT_NE(nullptr, applicationInfo);
    applicationInfo->apiCompatibleVersion = 12;
    stageContent->SetApplicationInfo(applicationInfo);
    abilityContext->stageContext_ = stageContent;
    window_->context_ = abilityContext;
    window_->isIgnoreSafeAreaNeedNotify_ = true;
    window_->ArkUIFrameworkSupport();
}

/**
 * @tc.name: NapiSetUIContent
 * @tc.desc: NapiSetUIContent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NapiSetUIContent, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, window_);
    std::string contentInfo = "NapiSetUIContent test";
    napi_env env = napi_env();
    napi_value storage = napi_value();
    sptr<IRemoteObject> token;
    window_->uiContent_ = nullptr;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::UIEXTENSION_USAGE_END);
    window_->focusState_ = std::nullopt;
    window_->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));

    auto uiContent = std::make_shared<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, uiContent);
    window_->uiContent_ = uiContent;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    window_->focusState_ = true;
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    usleep(WAIT_SYNC_IN_NS);

    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::EMBEDDED);
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::PREVIEW_EMBEDDED);
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::UIEXTENSION_USAGE_END);
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
}

/**
 * @tc.name: AniSetUIContent
 * @tc.desc: AniSetUIContent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, AniSetUIContent, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, window_);
    std::string contentInfo = "AniSetUIContent test";
    ani_env* env = nullptr;
    ani_object storage = ani_object();
    sptr<IRemoteObject> token;
    window_->uiContent_ = nullptr;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::UIEXTENSION_USAGE_END);
    window_->focusState_ = std::nullopt;
    window_->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));

    auto uiContent = std::make_shared<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, uiContent);
    window_->uiContent_ = uiContent;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    window_->focusState_ = true;
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    usleep(WAIT_SYNC_IN_NS);

    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::EMBEDDED);
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::PREVIEW_EMBEDDED);
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::UIEXTENSION_USAGE_END);
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContent(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
}

/**
 * @tc.name: NapiSetUIContentByName
 * @tc.desc: NapiSetUIContentByName Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NapiSetUIContentByName, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, window_);
    std::string contentInfo = "NapiSetUIContentByName test";
    napi_env env = napi_env();
    napi_value storage = napi_value();
    sptr<IRemoteObject> token;
    window_->uiContent_ = nullptr;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::UIEXTENSION_USAGE_END);
    window_->focusState_ = std::nullopt;
    window_->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContentByName(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));

    auto uiContent = std::make_shared<Ace::UIContentMocker>();
    window_->uiContent_ = uiContent;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    window_->focusState_ = true;
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK,
        window_->NapiSetUIContentByName(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: AniSetUIContentByName
 * @tc.desc: AniSetUIContentByName Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, AniSetUIContentByName, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, window_);
    std::string contentInfo = "AniSetUIContentByName test";
    ani_env* env = nullptr;
    ani_object storage = ani_object();
    sptr<IRemoteObject> token;
    window_->uiContent_ = nullptr;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::UIEXTENSION_USAGE_END);
    window_->focusState_ = std::nullopt;
    window_->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContentByName(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));

    auto uiContent = std::make_shared<Ace::UIContentMocker>();
    window_->uiContent_ = uiContent;
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    window_->focusState_ = true;
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK,
        window_->AniSetUIContentByName(contentInfo, env, storage, BackupAndRestoreType::NONE, token, nullptr));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateRect01
 * @tc.desc: UpdateRect Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateRect01, TestSize.Level0)
{
    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.height_ = 50;
    rect.width_ = 50;

    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 200;
    preRect.width_ = 200;

    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetWindowRect(preRect);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    ASSERT_EQ(WSError::WS_OK, window_->UpdateRect(rect, reason));

    preRect.height_ = 50;
    window_->property_->SetWindowRect(preRect);
    ASSERT_EQ(WSError::WS_OK, window_->UpdateRect(rect, reason));

    preRect.height_ = 200;
    preRect.width_ = 50;
    window_->property_->SetWindowRect(preRect);
    ASSERT_EQ(WSError::WS_OK, window_->UpdateRect(rect, reason));

    preRect.height_ = 50;
    preRect.width_ = 50;
    window_->property_->SetWindowRect(preRect);
    reason = SizeChangeReason::ROTATION;
    ASSERT_EQ(WSError::WS_OK, window_->UpdateRect(rect, reason));

    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, window_->UpdateRect(rect, reason));
}

/**
 * @tc.name: NotifyAccessibilityHoverEvent01
 * @tc.desc: NotifyAccessibilityHoverEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyAccessibilityHoverEvent01, TestSize.Level1)
{
    float pointX = 0.0f;
    float pointY = 0.0f;
    int32_t sourceType = 0;
    int32_t eventType = 0;
    int64_t timeMs = 0;
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    ASSERT_EQ(WSError::WS_OK,
        window_->NotifyAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyAccessibilityHoverEvent02
 * @tc.desc: NotifyAccessibilityHoverEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyAccessibilityHoverEvent02, TestSize.Level1)
{
    float pointX = 0.0f;
    float pointY = 0.0f;
    int32_t sourceType = 0;
    int32_t eventType = 0;
    int64_t timeMs = 0;
    window_->uiContent_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NO_UI_CONTENT_ERROR,
        window_->NotifyAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: TransferAccessibilityEvent
 * @tc.desc: TransferAccessibilityEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, TransferAccessibilityEvent, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    Accessibility::AccessibilityEventInfo info;
    int64_t uiExtensionIdLevel = 1;
    ASSERT_NE(WMError::WM_OK, window_->TransferAccessibilityEvent(info, uiExtensionIdLevel));

    window_->hostSession_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window_->TransferAccessibilityEvent(info, uiExtensionIdLevel));
}

/**
 * @tc.name: UpdateSessionViewportConfig1
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSessionViewportConfig1, TestSize.Level0)
{
    ASSERT_NE(nullptr, window_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetDisplayId(0);
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);

    SessionViewportConfig config;
    window_->lastDensity_ = 1.0f;
    window_->lastOrientation_ = 0;
    config.isDensityFollowHost_ = true;
    config.density_ = 1.0f;
    config.orientation_ = 0;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);

    window_->lastDensity_ = 1.0f;
    window_->lastOrientation_ = 0;
    config.density_ = 2.0f;
    config.orientation_ = 0;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);

    window_->lastDensity_ = 1.0f;
    window_->lastOrientation_ = 0;
    config.density_ = 1.0f;
    config.orientation_ = 1;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);

    config.isDensityFollowHost_ = false;
    window_->lastDensity_ = 0.0f;
    window_->lastOrientation_ = 0;
    config.density_ = 1.0f;
    config.orientation_ = 0;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateSessionViewportConfig2
 * @tc.desc: invalid density
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSessionViewportConfig2, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    SessionViewportConfig config;
    config.isDensityFollowHost_ = true;
    config.density_ = -1.0f;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: UpdateSessionViewportConfig3
 * @tc.desc: handler_ is null
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSessionViewportConfig3, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    SessionViewportConfig config;
    window_->handler_ = nullptr;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateSessionViewportConfig4
 * @tc.desc: displayId is changed
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSessionViewportConfig4, TestSize.Level1)
{
    window_->property_->SetDisplayId(0);
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);

    SessionViewportConfig config;
    window_->lastDensity_ = 1.0f;
    window_->lastOrientation_ = 0;
    config.density_ = 1.0f;
    config.orientation_ = 0;
    config.displayId_ = 999;
    ASSERT_EQ(window_->UpdateSessionViewportConfig(config), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(999, window_->GetDisplayId());
}

/**
 * @tc.name: UpdateSystemViewportConfig1
 * @tc.desc: handler_ is null
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSystemViewportConfig1, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->handler_ = nullptr;
    window_->UpdateSystemViewportConfig();
}

/**
 * @tc.name: UpdateSystemViewportConfig2
 * @tc.desc: Follow host
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSystemViewportConfig2, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->isDensityFollowHost_ = true;
    window_->UpdateSystemViewportConfig();
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateSystemViewportConfig3
 * @tc.desc: Do not follow host
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSystemViewportConfig3, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_->property_);
    window_->isDensityFollowHost_ = false;
    window_->property_->SetDisplayId(0);
    window_->UpdateSystemViewportConfig();
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateSystemViewportConfig_DensityCustomized
 * @tc.desc: Density customized
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSystemViewportConfig_DensityCustomized, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->isDensityCustomized_ = true;
    window_->UpdateSystemViewportConfig();
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyDisplayInfoChange1
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDisplayInfoChange1, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    sptr<IRemoteObject> contextToken = sptr<IRemoteObjectMocker>::MakeSptr();
    abilityContext->SetToken(contextToken);
    window_->context_ = abilityContext;
    EXPECT_NE(nullptr, window_->context_->GetToken());
    SessionViewportConfig config;
    config.displayId_ = 999;
    window_->lastDisplayId_ = 0;
    window_->NotifyDisplayInfoChange(config);
    window_->lastDisplayId_ = 999;
    window_->NotifyDisplayInfoChange(config);
}

/**
 * @tc.name: NotifyDisplayInfoChange2
 * @tc.desc: context_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDisplayInfoChange2, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->context_ = nullptr;
    SessionViewportConfig config;
    window_->NotifyDisplayInfoChange(config);
}

/**
 * @tc.name: NotifyDisplayInfoChange3
 * @tc.desc: system demsity change
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDisplayInfoChange3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionViewportConfig config;
    config.displayId_ = -1ULL;
    auto lastDensity = window->lastSystemDensity_;
    window->NotifyDisplayInfoChange(config);
    EXPECT_EQ(lastDensity, window->lastSystemDensity_);

    config.displayId_ = 0;
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(config.displayId_);
    ASSERT_NE(nullptr, display);
    auto displayInfo = display->GetDisplayInfo();
    ASSERT_NE(nullptr, displayInfo);
    auto vpr = displayInfo->GetVirtualPixelRatio();
    window->NotifyDisplayInfoChange(config);
    EXPECT_EQ(vpr, window->lastSystemDensity_);
    window->lastSystemDensity_ = vpr;
    window->NotifyDisplayInfoChange(config);
    EXPECT_EQ(vpr, window->lastSystemDensity_);
}

/**
 * @tc.name: NotifyAccessibilityChildTreeRegister01
 * @tc.desc: NotifyAccessibilityChildTreeRegister Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyAccessibilityChildTreeRegister01, TestSize.Level1)
{
    uint32_t windowId = 0;
    int32_t treeId = 0;
    int64_t accessibilityId = 0;
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    auto ret = window_->NotifyAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
    ASSERT_EQ(WSError::WS_OK, ret);

    window_->uiContent_ = nullptr;
    ret = window_->NotifyAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
    ASSERT_EQ(WSError::WS_OK, ret);

    window_->handler_ = nullptr;
    ret = window_->NotifyAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
    ASSERT_EQ(WSError::WS_ERROR_INTERNAL_ERROR, ret);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyAccessibilityChildTreeUnregister01
 * @tc.desc: NotifyAccessibilityChildTreeUnregister Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyAccessibilityChildTreeUnregister01, TestSize.Level1)
{
    window_->uiContent_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, window_->NotifyAccessibilityChildTreeUnregister());

    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    ASSERT_EQ(WSError::WS_OK, window_->NotifyAccessibilityChildTreeUnregister());

    window_->handler_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_INTERNAL_ERROR, window_->NotifyAccessibilityChildTreeUnregister());
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyAccessibilityDumpChildInfo01
 * @tc.desc: NotifyAccessibilityDumpChildInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyAccessibilityDumpChildInfo01, TestSize.Level1)
{
    std::vector<std::string> params;
    std::vector<std::string> info;
    window_->uiContent_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, window_->NotifyAccessibilityDumpChildInfo(params, info));

    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    ASSERT_EQ(WSError::WS_OK, window_->NotifyAccessibilityDumpChildInfo(params, info));

    window_->handler_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_INTERNAL_ERROR, window_->NotifyAccessibilityDumpChildInfo(params, info));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateAccessibilityTreeInfo
 * @tc.desc: UpdateAccessibilityTreeInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateAccessibilityTreeInfo, TestSize.Level1)
{
    std::optional<AccessibilityChildTreeInfo> accessibilityChildTreeInfo =
		std::make_optional<AccessibilityChildTreeInfo>();
    ASSERT_NE(accessibilityChildTreeInfo, std::nullopt);
    window_->accessibilityChildTreeInfo_ = accessibilityChildTreeInfo;
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    window_->UpdateAccessibilityTreeInfo();

    window_->uiContent_ = nullptr;
    window_->UpdateAccessibilityTreeInfo();

    window_->accessibilityChildTreeInfo_= std::nullopt;
    window_->UpdateAccessibilityTreeInfo();
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfo01
 * @tc.desc: NotifyExecuteAction Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyOccupiedAreaChangeInfo01, TestSize.Level1)
{
    sptr<OccupiedAreaChangeInfo> info = new(std::nothrow) OccupiedAreaChangeInfo();
    ASSERT_NE(nullptr, info);
    window_->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfo02
 * @tc.desc: NotifyExecuteAction Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyOccupiedAreaChangeInfo02, TestSize.Level1)
{
    sptr<IOccupiedAreaChangeListener> iOccupiedAreaChangeListener = new(std::nothrow) IOccupiedAreaChangeListener();
    ASSERT_NE(nullptr, iOccupiedAreaChangeListener);
    window_->RegisterOccupiedAreaChangeListener(iOccupiedAreaChangeListener);
    sptr<OccupiedAreaChangeInfo> info = new(std::nothrow) OccupiedAreaChangeInfo();
    ASSERT_NE(nullptr, info);
    window_->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});
}

/**
 * @tc.name: GetAvoidAreaByType01
 * @tc.desc: NotifyExecuteAction Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetAvoidAreaByType01, TestSize.Level0)
{
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    AvoidArea avoidArea;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_CUTOUT;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_KEYBOARD;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_NAVIGATION_INDICATOR;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
}

/**
 * @tc.name: GetAvoidAreaByType02
 * @tc.desc: NotifyExecuteAction Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetAvoidAreaByType02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(mockHostSession, nullptr);
    window_->hostSession_ = mockHostSession;
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    AvoidArea avoidArea;
    EXPECT_CALL(*mockHostSession, GetAvoidAreaByType).Times(5);
    ASSERT_EQ(WMError::WM_OK, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_CUTOUT;
    ASSERT_EQ(WMError::WM_OK, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    ASSERT_EQ(WMError::WM_OK, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_KEYBOARD;
    ASSERT_EQ(WMError::WM_OK, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    avoidAreaType = AvoidAreaType::TYPE_NAVIGATION_INDICATOR;
    ASSERT_EQ(WMError::WM_OK, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));

    AvoidArea expectedAvoidArea;
    expectedAvoidArea.topRect_ = {10, 20, 30, 40};
    EXPECT_CALL(*mockHostSession, GetAvoidAreaByType).Times(1).WillOnce(Return(expectedAvoidArea));
    ASSERT_EQ(WMError::WM_OK, window_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    ASSERT_EQ(avoidArea, expectedAvoidArea);
}

/**
 * @tc.name: RegisterAvoidAreaChangeListener
 * @tc.desc: RegisterAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterAvoidAreaChangeListener, TestSize.Level1)
{
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->RegisterAvoidAreaChangeListener(listener));
}

/**
 * @tc.name: UnregisterAvoidAreaChangeListener
 * @tc.desc: UnregisterAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UnregisterAvoidAreaChangeListener, TestSize.Level1)
{
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->UnregisterAvoidAreaChangeListener(listener));
}

/**
 * @tc.name: Show
 * @tc.desc: Show
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Show, TestSize.Level0)
{
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->persistentId_ = 12345;

    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(mockHostSession, nullptr);
    window_->hostSession_ = mockHostSession;

    window_->property_->SetDisplayId(0);
    EXPECT_CALL(*mockHostSession, Foreground).Times(1).WillOnce(Return(WSError::WS_DO_NOTHING));
    ASSERT_EQ(static_cast<WMError>(WSError::WS_DO_NOTHING), window_->Show());
    EXPECT_CALL(*mockHostSession, Foreground).Times(1).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window_->Show());
}

/**
 * @tc.name: Hide
 * @tc.desc: Hide
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, Hide, TestSize.Level0)
{
    ASSERT_NE(nullptr, window_->property_);

    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(mockHostSession, nullptr);
    window_->hostSession_ = mockHostSession;

    window_->property_->persistentId_ = INVALID_SESSION_ID;
    auto res = window_->Hide(0, false, false);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    window_->property_->persistentId_ = 12345;
    window_->state_ = WindowState::STATE_HIDDEN;
    EXPECT_CALL(*mockHostSession, Background).Times(0);
    res = window_->Hide(0, false, false);
    ASSERT_EQ(res, WMError::WM_OK);

    window_->state_ = WindowState::STATE_CREATED;
    EXPECT_CALL(*mockHostSession, Background).Times(0);
    res = window_->Hide(0, false, false);
    ASSERT_EQ(res, WMError::WM_OK);

    window_->state_ = WindowState::STATE_SHOWN;
    EXPECT_CALL(*mockHostSession, Background).Times(1).WillOnce(Return(WSError::WS_OK));
    res = window_->Hide(0, false, false);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(window_->state_, WindowState::STATE_HIDDEN);

    window_->state_ = WindowState::STATE_SHOWN;
    EXPECT_CALL(*mockHostSession, Background).Times(1).WillOnce(Return(WSError::WS_DO_NOTHING));
    res = window_->Hide(0, false, false);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(window_->state_, WindowState::STATE_SHOWN);
}

/**
 * @tc.name: NotifyDensityFollowHost01
 * @tc.desc: test isFollowHost is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDensityFollowHost01, TestSize.Level1)
{
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetDisplayId(displayId);

    auto isFollowHost = true;
    auto densityValue = 0.1f;

    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window_->uiContent_.get());
    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 200;
    preRect.width_ = 200;
    window_->property_->SetWindowRect(preRect);
    EXPECT_CALL(*content, UpdateViewportConfig(Field(&Ace::ViewportConfig::density_, densityValue), _, _, _, _));

    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyDensityFollowHost02
 * @tc.desc: test isFollowHost is true -> false
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDensityFollowHost02, TestSize.Level1)
{
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetDisplayId(displayId);

    auto isFollowHost = false;
    auto densityValue = 0.1f;

    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window_->property_->GetDisplayId());
    ASSERT_NE(display, nullptr);
    ASSERT_NE(display->GetDisplayInfo(), nullptr);
    auto vpr = display->GetDisplayInfo()->GetVirtualPixelRatio();

    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window_->uiContent_.get());
    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 100;
    preRect.width_ = 100;
    window_->property_->SetWindowRect(preRect);
    EXPECT_CALL(*content, UpdateViewportConfig(Field(&Ace::ViewportConfig::density_, vpr), _, _, _, _));

    window_->isDensityFollowHost_ = true;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyDensityFollowHost03
 * @tc.desc: test isFollowHost not change
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDensityFollowHost03, TestSize.Level1)
{
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetDisplayId(displayId);

    auto isFollowHost = false;
    auto densityValue = 0.1f;
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window_->uiContent_.get());
    EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _)).Times(0);

    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyDensityFollowHost04
 * @tc.desc: test densityValue invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDensityFollowHost04, TestSize.Level1)
{
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetDisplayId(displayId);

    auto isFollowHost = true;
    auto densityValue = 0.0f;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_ERROR_INVALID_PARAM);
    densityValue = -0.1f;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: NotifyDensityFollowHost05
 * @tc.desc: test densityValue not change
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDensityFollowHost05, TestSize.Level1)
{
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetDisplayId(displayId);
    window_->property_->windowRect_ = {1, 1, 1, 1};

    auto isFollowHost = true;
    auto densityValue = 0.1f;
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window_->uiContent_.get());
    EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _)).Times(3);

    window_->hostDensityValue_ = densityValue;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    ASSERT_FALSE(window_->isDensityFollowHost_);

    window_->hostDensityValue_ = 0.2f;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    ASSERT_TRUE(window_->isDensityFollowHost_);
    ASSERT_EQ(window_->hostDensityValue_, densityValue);

    densityValue = FLT_MAX;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    ASSERT_TRUE(window_->isDensityFollowHost_);
    ASSERT_EQ(window_->hostDensityValue_, densityValue);

    densityValue = FLT_MIN;
    window_->isDensityFollowHost_ = false;
    ASSERT_EQ(window_->NotifyDensityFollowHost(isFollowHost, densityValue), WSError::WS_OK);
    ASSERT_TRUE(window_->isDensityFollowHost_);
    ASSERT_EQ(window_->hostDensityValue_, densityValue);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetVirtualPixelRatio01
 * @tc.desc: follow host density value
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetVirtualPixelRatio01, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    displayInfo->SetVirtualPixelRatio(3.25f);
    window_->isDensityFollowHost_ = true;
    window_->hostDensityValue_ = 2.0f;
    ASSERT_EQ(window_->hostDensityValue_, window_->GetVirtualPixelRatio(displayInfo));
}

/**
 * @tc.name: GetVirtualPixelRatio02
 * @tc.desc: follow system density value
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetVirtualPixelRatio02, TestSize.Level1)
{
    auto systemDensity = 3.25;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    displayInfo->SetVirtualPixelRatio(systemDensity);
    window_->isDensityFollowHost_ = false;
    window_->hostDensityValue_ = 2.0f;
    ASSERT_EQ(systemDensity, window_->GetVirtualPixelRatio(displayInfo));
}

/**
 * @tc.name: GetVirtualPixelRatio03
 * @tc.desc: hostDensityValue_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetVirtualPixelRatio03, TestSize.Level1)
{
    auto systemDensity = 3.25;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    displayInfo->SetVirtualPixelRatio(systemDensity);
    window_->isDensityFollowHost_ = true;
    ASSERT_EQ(systemDensity, window_->GetVirtualPixelRatio(displayInfo));
}

/**
 * @tc.name: GetVirtualPixelRatio04
 * @tc.desc: GetVirtualPixelRatio04 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetVirtualPixelRatio04, TestSize.Level1)
{
    ASSERT_EQ(1.0f, window_->GetVirtualPixelRatio(nullptr));
}

/**
 * @tc.name: GetVirtualPixelRatio_compat
 * @tc.desc: GetVirtualPixelRatio_compat test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetVirtualPixelRatio_compat, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio_compat");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    auto value = window->GetVirtualPixelRatio(nullptr);
    EXPECT_NEAR(value, COMPACT_SIMULATION_SCALE_DPI, 0.00001f);
}

/**
 * @tc.name: GetVirtualPixelRatio_DensityCustomized
 * @tc.desc: GetVirtualPixelRatio_DensityCustomized test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetVirtualPixelRatio_DensityCustomized, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio_DensityCustomized");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->isDensityCustomized_ = true;
    window->customizedDensity_ = 2.0f;
    EXPECT_NEAR(window->GetVirtualPixelRatio(nullptr), window->customizedDensity_, 0.00001f);
}

/**
 * @tc.name: HideNonSecureWindows01
 * @tc.desc: HideNonSecureWindows Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, HideNonSecureWindows01, TestSize.Level0)
{
    ASSERT_EQ(WMError::WM_OK, window_->HideNonSecureWindows(false));
    ASSERT_FALSE(window_->extensionWindowFlags_.hideNonSecureWindowsFlag);
    ASSERT_EQ(WMError::WM_OK, window_->HideNonSecureWindows(true));
    ASSERT_TRUE(window_->extensionWindowFlags_.hideNonSecureWindowsFlag);
}

/**
 * @tc.name: HideNonSecureWindows02
 * @tc.desc: HideNonSecureWindows Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, HideNonSecureWindows02, TestSize.Level1)
{
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, window_->HideNonSecureWindows(false));

    sptr<IRemoteObject> iRemoteObject = new IRemoteObjectMocker();
    ASSERT_NE(nullptr, iRemoteObject);
    window_->abilityToken_ = iRemoteObject;
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);

    ASSERT_EQ(WMError::WM_OK, window_->HideNonSecureWindows(false));
}

/**
 * @tc.name: HideNonSecureWindows03
 * @tc.desc: HideNonSecureWindows Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, HideNonSecureWindows03, TestSize.Level1)
{
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window_->HideNonSecureWindows(true));
}

/**
 * @tc.name: HideNonSecureWindows04
 * @tc.desc: HideNonSecureWindows Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, HideNonSecureWindows04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->HideNonSecureWindows(true));
}

/**
 * @tc.name: HideNonSecureWindows06
 * @tc.desc: HideNonSecureWindows Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, HideNonSecureWindows06, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window_->HideNonSecureWindows(false));
}

/**
 * @tc.name: SetWaterMarkFlag01
 * @tc.desc: SetWaterMarkFlag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetWaterMarkFlag01, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_OK, window_->SetWaterMarkFlag(false));
    ASSERT_FALSE(window_->extensionWindowFlags_.waterMarkFlag);
    ASSERT_EQ(WMError::WM_OK, window_->SetWaterMarkFlag(true));
    ASSERT_TRUE(window_->extensionWindowFlags_.waterMarkFlag);
}

/**
 * @tc.name: SetWaterMarkFlag02
 * @tc.desc: SetWaterMarkFlag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetWaterMarkFlag02, TestSize.Level1)
{
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, window_->SetWaterMarkFlag(false));
}

/**
 * @tc.name: SetWaterMarkFlag03
 * @tc.desc: SetWaterMarkFlag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetWaterMarkFlag03, TestSize.Level1)
{
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window_->SetWaterMarkFlag(true));
}

/**
 * @tc.name: SetWaterMarkFlag04
 * @tc.desc: SetWaterMarkFlag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetWaterMarkFlag04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->SetWaterMarkFlag(true));
}

/**
 * @tc.name: CheckAndAddExtWindowFlags01
 * @tc.desc: CheckAndAddExtWindowFlags01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, CheckAndAddExtWindowFlags01, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->CheckAndAddExtWindowFlags();
}

/**
 * @tc.name: CheckAndAddExtWindowFlags02
 * @tc.desc: CheckAndAddExtWindowFlags02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, CheckAndAddExtWindowFlags02, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->extensionWindowFlags_.bitData = 1;
    window_->CheckAndAddExtWindowFlags();
}

/**
 * @tc.name: CheckAndRemoveExtWindowFlags01
 * @tc.desc: CheckAndRemoveExtWindowFlags01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, CheckAndRemoveExtWindowFlags01, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->CheckAndRemoveExtWindowFlags();
}

/**
 * @tc.name: CheckAndRemoveExtWindowFlags02
 * @tc.desc: CheckAndRemoveExtWindowFlags02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, CheckAndRemoveExtWindowFlags02, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->extensionWindowFlags_.bitData = 1;
    window_->CheckAndRemoveExtWindowFlags();
}

/**
 * @tc.name: UpdateExtWindowFlags01
 * @tc.desc: UpdateExtWindowFlags Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtWindowFlags01, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window_->UpdateExtWindowFlags(ExtensionWindowFlags(),
        ExtensionWindowFlags()));
}

/**
 * @tc.name: UpdateExtWindowFlags02
 * @tc.desc: UpdateExtWindowFlags Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtWindowFlags02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->UpdateExtWindowFlags(ExtensionWindowFlags(), ExtensionWindowFlags()));

    sptr<IRemoteObject> iRemoteObject = new IRemoteObjectMocker();
    ASSERT_NE(nullptr, iRemoteObject);
    window_->abilityToken_ = iRemoteObject;
    ASSERT_NE(WMError::WM_ERROR_NULLPTR, window_->UpdateExtWindowFlags(ExtensionWindowFlags(), ExtensionWindowFlags()));
}

/**
 * @tc.name: UpdateExtWindowFlags03
 * @tc.desc: UpdateExtWindowFlags Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtWindowFlags03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->UpdateExtWindowFlags(ExtensionWindowFlags(), ExtensionWindowFlags()));
}

/**
 * @tc.name: GetHostWindowRect01
 * @tc.desc: GetHostWindowRect Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetHostWindowRect01, TestSize.Level1)
{
    Rect rect;
    ASSERT_EQ(rect, window_->GetHostWindowRect(-1));
}

/**
 * @tc.name: GetHostWindowRect02
 * @tc.desc: GetHostWindowRect Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetHostWindowRect02, TestSize.Level1)
{
    Rect rect;
    ASSERT_EQ(rect, window_->GetHostWindowRect(0));
}

/**
 * @tc.name: GetGlobalScaledRect
 * @tc.desc: GetGlobalScaledRect Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetGlobalScaledRect, TestSize.Level1)
{
    Rect rect;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_SESSION, window_->GetGlobalScaledRect(rect));
}

/**
 * @tc.name: ConsumePointerEvent
 * @tc.desc: ConsumePointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ConsumePointerEvent, TestSize.Level0)
{
    struct RSSurfaceNodeConfig config;
    window_->surfaceNode_ = RSSurfaceNode::Create(config);
    window_->state_ = WindowState::STATE_SHOWN;

    auto pointerEvent = MMI::PointerEvent::Create();
    window_->ConsumePointerEvent(nullptr);

    window_->ConsumePointerEvent(pointerEvent);

    SessionInfo sessionInfo;
    window_->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    window_->ConsumePointerEvent(pointerEvent);

    MMI::PointerEvent::PointerItem item;
    pointerEvent->SetPointerId(0);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    item.SetPointerId(0);
    item.SetDisplayX(15); // 15 : position x
    item.SetDisplayY(15); // 15 : position y
    item.SetWindowX(15); // 15 : position x
    item.SetWindowY(15); // 15 : position y
    pointerEvent->AddPointerItem(item);
    window_->ConsumePointerEvent(pointerEvent);

    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    window_->ConsumePointerEvent(pointerEvent);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_MOVE);
    pointerEvent->UpdatePointerItem(0, item);
    window_->ConsumePointerEvent(pointerEvent);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerEvent->UpdatePointerItem(0, item);
    window_->ConsumePointerEvent(pointerEvent);
}

/**
 * @tc.name: ConsumePointerEvent02
 * @tc.desc: ConsumePointerEvent02 Test branch: uiExtensionUsage_ == UIExtensionUsage::PREVIEW_EMBEDDED
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ConsumePointerEvent02, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::PREVIEW_EMBEDDED);
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    window_->ConsumePointerEvent(pointerEvent);
}
/**
 * @tc.name: NotifyPointerEvent
 * @tc.desc: NotifyPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyPointerEvent, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(WindowExtensionSessionImplLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPointerEvent");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->uiContent_ = nullptr;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    window->NotifyPointerEvent(pointerEvent);
    EXPECT_TRUE(logMsg.find("pointerEvent is nullptr") != std::string::npos);
    logMsg.clear();

    pointerEvent = MMI::PointerEvent::Create();
    window->inputEventConsumer_ = std::make_shared<MockInputEventConsumer>();
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    EXPECT_EQ(pointerEvent->GetPointerAction(), MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    window->NotifyPointerEvent(pointerEvent);

    bool isHostWindowDelayRaiseEnabled = true;
    window->property_->SetWindowDelayRaiseEnabled(isHostWindowDelayRaiseEnabled);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    window->NotifyPointerEvent(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    window->NotifyPointerEvent(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    window->NotifyPointerEvent(pointerEvent);
    EXPECT_TRUE(logMsg.find("uiContent is null") != std::string::npos);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->NotifyPointerEvent(pointerEvent);

    window->inputEventConsumer_ = nullptr;
    window->NotifyPointerEvent(pointerEvent);
}

/**
 * @tc.name: ProcessPointerEventWithHostWindowDelayRaise
 * @tc.desc: ProcessPointerEventWithHostWindowDelayRaise Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ProcessPointerEventWithHostWindowDelayRaise, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(WindowExtensionSessionImplLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ProcessPointerEventWithHostWindowDelayRaise");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    bool isHitTargetDraggable = false;
    window->ProcessPointerEventWithHostWindowDelayRaise(pointerEvent, isHitTargetDraggable);
    EXPECT_TRUE(logMsg.find("pointerEvent is nullptr") != std::string::npos);
    logMsg.clear();

    pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    window->ProcessPointerEventWithHostWindowDelayRaise(pointerEvent, isHitTargetDraggable);

    isHitTargetDraggable = true;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    window->dataHandler_ = nullptr;
    window->ProcessPointerEventWithHostWindowDelayRaise(pointerEvent, isHitTargetDraggable);
    EXPECT_TRUE(logMsg.find("No need to notify") != std::string::npos);
    logMsg.clear();

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    window->ProcessPointerEventWithHostWindowDelayRaise(pointerEvent, isHitTargetDraggable);
    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    window->ProcessPointerEventWithHostWindowDelayRaise(pointerEvent, isHitTargetDraggable);
    EXPECT_TRUE(logMsg.find("Notify host window to raise") != std::string::npos);
}

/**
 * @tc.name: PreNotifyKeyEvent
 * @tc.desc: PreNotifyKeyEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, PreNotifyKeyEvent, TestSize.Level1)
{
    bool ret = window_->PreNotifyKeyEvent(nullptr);
    ASSERT_EQ(ret, false);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(nullptr, keyEvent);
    ASSERT_NE(nullptr, window_->property_);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    ret = window_->PreNotifyKeyEvent(keyEvent);
    ASSERT_EQ(ret, false);

    std::shared_ptr<Ace::UIContent> uiContent = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, uiContent);
    window_->uiContent_ = uiContent;
    Ace::UIContentMocker* uiContentMocker = reinterpret_cast<Ace::UIContentMocker*>(window_->uiContent_.get());
    EXPECT_CALL(*uiContentMocker, ProcessKeyEvent).Times(2).WillOnce(Return(true));
    ret = window_->PreNotifyKeyEvent(keyEvent);
    ASSERT_EQ(ret, true);

    window_->property_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    ret = window_->PreNotifyKeyEvent(keyEvent);
    ASSERT_EQ(ret, true);

    window_->focusState_ = false;
    ret = window_->PreNotifyKeyEvent(keyEvent);
    ASSERT_EQ(ret, true);

    window_->focusState_ = true;
    ret = window_->PreNotifyKeyEvent(keyEvent);
    ASSERT_EQ(ret, false);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: PreNotifyKeyEvent02
 * @tc.desc: PreNotifyKeyEvent02 Test branch: uiExtensionUsage_ == UIExtensionUsage::PREVIEW_EMBEDDED
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, PreNotifyKeyEvent02, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(nullptr, keyEvent);
    ASSERT_NE(nullptr, window_);
    window_->property_->SetUIExtensionUsage(UIExtensionUsage::PREVIEW_EMBEDDED);
    ASSERT_FALSE(window_->PreNotifyKeyEvent(keyEvent));
}

/**
 * @tc.name: GetFreeMultiWindowModeEnabledState
 * @tc.desc: GetFreeMultiWindowModeEnabledState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetFreeMultiWindowModeEnabledState, TestSize.Level1)
{
    ASSERT_EQ(false, window_->GetFreeMultiWindowModeEnabledState());
}

/**
 * @tc.name: NotifyExtensionTimeout
 * @tc.desc: NotifyExtensionTimeout Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyExtensionTimeout, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window_->hostSession_ = session;
    EXPECT_CALL(*session, NotifyExtensionTimeout).Times(1);
    window_->NotifyExtensionTimeout(WindowExtensionSessionImpl::TimeoutErrorCode::SET_UICONTENT_TIMEOUT);

    window_->hostSession_ = nullptr;
    EXPECT_CALL(*session, NotifyExtensionTimeout).Times(0);
    window_->NotifyExtensionTimeout(WindowExtensionSessionImpl::TimeoutErrorCode::SET_UICONTENT_TIMEOUT);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetRealParentId
 * @tc.desc: GetRealParentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetRealParentId, TestSize.Level1)
{
    ASSERT_NE(window_->property_, nullptr);
    window_->property_->SetRealParentId(12345);
    EXPECT_EQ(window_->GetRealParentId(), 12345);
}

/**
 * @tc.name: GetParentWindowType
 * @tc.desc: GetParentWindowType Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetParentWindowType, TestSize.Level1)
{
    ASSERT_NE(window_->property_, nullptr);
    window_->property_->SetParentWindowType(WindowType::WINDOW_TYPE_TOAST);
    EXPECT_EQ(window_->GetParentWindowType(), WindowType::WINDOW_TYPE_TOAST);
}

/**
 * @tc.name: CheckHideNonSecureWindowsPermission
 * @tc.desc: CheckHideNonSecureWindowsPermission Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, CheckHideNonSecureWindowsPermission, TestSize.Level1)
{
    ASSERT_NE(window_->property_, nullptr);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::EMBEDDED;
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(true), WMError::WM_OK);
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(false), WMError::WM_OK);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::MODAL;
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(true), WMError::WM_OK);
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(false), WMError::WM_ERROR_INVALID_OPERATION);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    window_->modalUIExtensionMayBeCovered_ = true;
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(true), WMError::WM_OK);
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(false), WMError::WM_ERROR_INVALID_OPERATION);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::PREVIEW_EMBEDDED;
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(true), WMError::WM_OK);
    EXPECT_EQ(window_->CheckHideNonSecureWindowsPermission(false), WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: NotifyModalUIExtensionMayBeCovered
 * @tc.desc: NotifyModalUIExtensionMayBeCovered Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyModalUIExtensionMayBeCovered, TestSize.Level1)
{
    ASSERT_NE(window_, nullptr);
    ASSERT_NE(window_->property_, nullptr);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::EMBEDDED;
    window_->NotifyModalUIExtensionMayBeCovered(true);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::MODAL;
    window_->extensionWindowFlags_.hideNonSecureWindowsFlag = true;
    window_->NotifyModalUIExtensionMayBeCovered(true);
    ASSERT_TRUE(window_->modalUIExtensionMayBeCovered_);
    ASSERT_TRUE(window_->modalUIExtensionSelfLoadContent_);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    window_->extensionWindowFlags_.hideNonSecureWindowsFlag = false;
    window_->NotifyModalUIExtensionMayBeCovered(false);
    ASSERT_TRUE(window_->modalUIExtensionMayBeCovered_);
    ASSERT_TRUE(window_->modalUIExtensionSelfLoadContent_);

    window_->property_->uiExtensionUsage_ = UIExtensionUsage::PREVIEW_EMBEDDED;
    window_->extensionWindowFlags_.hideNonSecureWindowsFlag = false;
    window_->NotifyModalUIExtensionMayBeCovered(false);
    ASSERT_TRUE(window_->modalUIExtensionMayBeCovered_);
    ASSERT_TRUE(window_->modalUIExtensionSelfLoadContent_);
}

/**
 * @tc.name: ReportModalUIExtensionMayBeCovered
 * @tc.desc: ReportModalUIExtensionMayBeCovered Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ReportModalUIExtensionMayBeCovered, TestSize.Level1)
{
    ASSERT_NE(window_, nullptr);
    window_->ReportModalUIExtensionMayBeCovered(true);
    window_->NotifyModalUIExtensionMayBeCovered(false);
}

/**
 * @tc.name: NotifyExtensionEventAsync
 * @tc.desc: NotifyExtensionEventAsync Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyExtensionEventAsync, TestSize.Level1)
{
    window_->NotifyExtensionEventAsync(0);

    SessionInfo sessionInfo;
    window_->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, window_->hostSession_);
    window_->property_->SetPersistentId(1);
    window_->NotifyExtensionEventAsync(0);
}

/**
 * @tc.name: NotifyDumpInfo
 * @tc.desc: NotifyDumpInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyDumpInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, window_);
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window_->uiContent_);
    std::vector<std::string> params;
    std::vector<std::string> info;
    auto ret = window_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_OK, ret);

    window_->uiContent_ = nullptr;
    ret = window_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, ret);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateConfigurationSyncForAll
 * @tc.desc: UpdateConfigurationSyncForAll Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateConfigurationSyncForAll, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    ASSERT_NE(nullptr, window_);
    window_->GetWindowExtensionSessionSet().insert(window_);
    window_->UpdateConfigurationSyncForAll(configuration);
    window_->GetWindowExtensionSessionSet().erase(window_);
}

/**
 * @tc.name: SetCompatInfo
 * @tc.desc: SetCompatInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetCompatInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetCompatInfo");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    AAFwk::WantParams configParam;
    EXPECT_EQ(WMError::WM_DO_NOTHING, window->SetCompatInfo(configParam));

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(WMError::WM_OK, window->SetCompatInfo(configParam));

    window->property_->SetCompatibleModeProperty(nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    configParam.SetParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(true)));
    float compatScaleX = 0.5f;
    float compatScaleY = 0.55f;
    configParam.SetParam(Extension::COMPAT_SCALE_X_FIELD, AAFwk::Float::Box(compatScaleX));
    configParam.SetParam(Extension::COMPAT_SCALE_Y_FIELD, AAFwk::Float::Box(compatScaleY));
    EXPECT_EQ(WMError::WM_OK, window->SetCompatInfo(configParam));
    EXPECT_NEAR(window->compatScaleX_, compatScaleX, 0.00001f);
    EXPECT_NEAR(window->compatScaleY_, compatScaleY, 0.00001f);
}

/**
 * @tc.name: OnHostWindowCompatInfoChange
 * @tc.desc: OnHostWindowCompatInfoChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnHostWindowCompatInfoChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnHostWindowCompatInfoChange");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    want.SetParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, false);
    auto res = window->OnHostWindowCompatInfoChange(std::move(want), reply);
    EXPECT_EQ(WMError::WM_DO_NOTHING, res);

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = window->OnHostWindowCompatInfoChange(std::move(want), reply);
    EXPECT_EQ(WMError::WM_OK, res);

    window->property_->SetCompatibleModeProperty(nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    want.SetParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, true);
    float compatScaleX = 0.5f;
    float compatScaleY = 0.55f;
    want.SetParam(Extension::COMPAT_SCALE_X_FIELD, compatScaleX);
    want.SetParam(Extension::COMPAT_SCALE_Y_FIELD, compatScaleY);
    res = window->OnHostWindowCompatInfoChange(std::move(want), reply);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_NEAR(window->compatScaleX_, compatScaleX, 0.00001f);
    EXPECT_NEAR(window->compatScaleY_, compatScaleY, 0.00001f);
}

/**
 * @tc.name: GetSystemViewportConfig
 * @tc.desc: GetSystemViewportConfig Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetSystemViewportConfig, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSystemViewportConfig");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetDisplayId(0);
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window->property_->GetDisplayId());
    ASSERT_NE(display, nullptr);
    ASSERT_NE(display->GetDisplayInfo(), nullptr);
    auto vpr = display->GetDisplayInfo()->GetVirtualPixelRatio();
    SessionViewportConfig config;
    EXPECT_EQ(WMError::WM_OK, window->GetSystemViewportConfig(config));
    EXPECT_NEAR(config.density_, vpr, 0.00001f);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(WMError::WM_OK, window->GetSystemViewportConfig(config));
    EXPECT_NEAR(config.density_, COMPACT_SIMULATION_SCALE_DPI, 0.00001f);
}

/**
 * @tc.name: UpdateExtensionDensity
 * @tc.desc: UpdateExtensionDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtensionDensity, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateExtensionDensity");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetDisplayId(0);
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window->property_->GetDisplayId());
    ASSERT_NE(display, nullptr);
    ASSERT_NE(display->GetDisplayInfo(), nullptr);
    auto vpr = display->GetDisplayInfo()->GetVirtualPixelRatio();
    SessionViewportConfig config;
    config.displayId_ = window->property_->GetDisplayId();
    config.isDensityFollowHost_ = true;
    config.density_ = 2.0f;
    window->UpdateExtensionDensity(config);
    EXPECT_TRUE(window->isDensityFollowHost_);
    if (window->hostDensityValue_ != std::nullopt) {
        EXPECT_NEAR(config.density_, window->hostDensityValue_->load(), 0.00001f);
    }
    config.isDensityFollowHost_ = false;
    window->UpdateExtensionDensity(config);
    EXPECT_FALSE(window->isDensityFollowHost_);
    EXPECT_NEAR(config.density_, vpr, 0.00001f);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->UpdateExtensionDensity(config);
    EXPECT_FALSE(window->isDensityFollowHost_);
    EXPECT_NEAR(config.density_, COMPACT_SIMULATION_SCALE_DPI, 0.00001f);
}

/**
 * @tc.name: UpdateExtensionDensity
 * @tc.desc: UpdateExtensionDensity_DensityCustomized Density customized test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtensionDensity_DensityCustomized, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateExtensionDensity");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->isDensityCustomized_ = true;
    window->customizedDensity_ = 2.0f;

    SessionViewportConfig config;
    config.isDensityFollowHost_ = false;
    config.density_ = 3.0f;
    window->UpdateExtensionDensity(config);
    EXPECT_NEAR(config.density_, window->customizedDensity_, 0.00001f);
}

/**
 * @tc.name: UpdateExtensionDensity
 * @tc.desc: UpdateExtensionDensity_DensityCustomized1 Density customized test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtensionDensity_DensityCustomized1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateExtensionDensity");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->isDensityCustomized_ = true;
    window->customizedDensity_ = 2.0f;

    SessionViewportConfig config;
    config.isDensityFollowHost_ = true;
    config.density_ = 3.0f;
    window->UpdateExtensionDensity(config);
    EXPECT_NEAR(3.0f, window->hostDensityValue_->load(), 0.00001f);
    EXPECT_NEAR(config.density_, window->customizedDensity_, 0.00001f);
}

/**
 * @tc.name: SetUIExtCustomDensity
 * @tc.desc: SetUIExtCustomDensity Density customized test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetUIExtCustomDensity, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateExtensionDensity");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetUIExtCustomDensity(0.0f));

    window->handler_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_SYSTEM_ABNORMALLY, window->SetUIExtCustomDensity(1.0f));

    auto runner = AppExecFwk::EventRunner::Create("WindowExtensionSessionImplTest");
    window->handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    EXPECT_EQ(WMError::WM_OK, window->SetUIExtCustomDensity(1.0f));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetDefaultDensity
 * @tc.desc: GetDefaultDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetDefaultDensity, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDefaultDensity");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    sptr<DisplayInfo> displayInfo = nullptr;
    EXPECT_NEAR(1.0f, window->GetDefaultDensity(displayInfo), 0.00001f);
    auto systemDensity = 2.0;
    displayInfo = sptr<DisplayInfo>::MakeSptr();
    displayInfo->SetVirtualPixelRatio(systemDensity);
    EXPECT_NEAR(systemDensity, window->GetDefaultDensity(displayInfo), 0.00001f);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_NEAR(COMPACT_SIMULATION_SCALE_DPI, window->GetDefaultDensity(displayInfo), 0.00001f);
}

/**
 * @tc.name: NotifyExtensionDataConsumer01
 * @tc.desc: Test NotifyExtensionDataConsumer with valid window mode data
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, NotifyExtensionDataConsumer01, TestSize.Level1)
{
    // Prepare and write config
    MessageParcel data;
    MessageParcel reply;
    Extension::DataTransferConfig config;
    config.subSystemId = SubSystemId::WM_UIEXT;
    config.customId = static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_WINDOW_MODE);
    config.needReply = false;
    config.needSyncSend = true;
    ASSERT_TRUE(data.WriteParcelable(&config));
 
    // Prepare and write want data
    AAFwk::Want want;
    want.SetParam(Extension::WINDOW_MODE_FIELD, static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING));
    ASSERT_TRUE(data.WriteParcelable(&want));

    // Send data
    MessageOption option;
    window_->SendExtensionData(data, reply, option);

    // Verify reply contains success code
    uint32_t replyCode;
    ASSERT_TRUE(reply.ReadUint32(replyCode));
    ASSERT_EQ(static_cast<uint32_t>(DataHandlerErr::OK), replyCode);
 
    // Verify window mode was updated
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window_->GetWindowMode());
}

/**
 * @tc.name: RegisterConsumer
 * @tc.desc: RegisterConsumer Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterConsumer, TestSize.Level1)
{
    window_->RegisterConsumer(Extension::Businesscode::SYNC_CROSS_AXIS_STATE,
        std::bind(&WindowExtensionSessionImpl::OnCrossAxisStateChange,
        window_, std::placeholders::_1, std::placeholders::_2));
    ASSERT_NE(nullptr,
        window_->dataConsumers_[static_cast<uint32_t>(Extension::Businesscode::SYNC_CROSS_AXIS_STATE)]);
}

/**
 * @tc.name: OnCrossAxisStateChange
 * @tc.desc: OnCrossAxisStateChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnCrossAxisStateChange, TestSize.Level1)
{
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    want.SetParam(Extension::CROSS_AXIS_FIELD, static_cast<int32_t>(CrossAxisState::STATE_CROSS));
    ASSERT_EQ(WMError::WM_OK, window_->OnCrossAxisStateChange(std::move(want), reply));
    ASSERT_EQ(CrossAxisState::STATE_CROSS, window_->crossAxisState_.load());
}

/**
 * @tc.name: OnWaterfallModeChange
 * @tc.desc: OnWaterfallModeChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnWaterfallModeChange, TestSize.Level1)
{
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    want.SetParam(Extension::WATERFALL_MODE_FIELD, true);
    EXPECT_EQ(WMError::WM_OK, window_->OnWaterfallModeChange(std::move(want), reply));
    EXPECT_TRUE(window_->IsWaterfallModeEnabled());
}

/**
 * @tc.name: OnHostWindowDelayRaiseStateChange
 * @tc.desc: OnHostWindowDelayRaiseStateChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnHostWindowDelayRaiseStateChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnHostWindowDelayRaiseStateChange");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    bool isHostWindowDelayRaiseEnabled = true;
    want.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, isHostWindowDelayRaiseEnabled);
    window->property_->SetWindowDelayRaiseEnabled(isHostWindowDelayRaiseEnabled);
    EXPECT_EQ(WMError::WM_OK, window->OnHostWindowDelayRaiseStateChange(std::move(want), reply));

    window->property_->SetWindowDelayRaiseEnabled(false);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(WMError::WM_OK, window->OnHostWindowDelayRaiseStateChange(std::move(want), reply));
    EXPECT_TRUE(window->IsWindowDelayRaiseEnabled());
    isHostWindowDelayRaiseEnabled = false;
    want.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, isHostWindowDelayRaiseEnabled);
    EXPECT_EQ(WMError::WM_OK, window->OnHostWindowDelayRaiseStateChange(std::move(want), reply));
    EXPECT_FALSE(window->IsWindowDelayRaiseEnabled());
}

/**
 * @tc.name: OnHostWindowRectChange
 * @tc.desc: OnHostWindowRectChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnHostWindowRectChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnHostWindowRectChange");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();

    EXPECT_EQ(WMError::WM_OK, window->OnHostWindowRectChange(std::move(want), reply));
    window->rectChangeUIExtListenerIds_.emplace(111);
    ASSERT_FALSE(window->rectChangeUIExtListenerIds_.empty());
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(WMError::WM_OK, window->OnHostWindowRectChange(std::move(want), reply));
}

/**
 * @tc.name: OnHostRectChangeInGlobalDisplay
 * @tc.desc: OnHostRectChangeInGlobalDisplay Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnHostRectChangeInGlobalDisplay, TestSize.Level1)
{
    window_->property_->SetPersistentId(1);
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    sptr<IRectChangeInGlobalDisplayListener> listener = sptr<IRectChangeInGlobalDisplayListener>::MakeSptr();
    window_->RegisterRectChangeInGlobalDisplayListener(listener);

    EXPECT_EQ(WMError::WM_OK, window_->OnHostRectChangeInGlobalDisplay(std::move(want), reply));
    window_->rectChangeInGlobalDisplayUIExtListenerIds_.emplace(111);
    EXPECT_EQ(WMError::WM_OK, window_->OnHostRectChangeInGlobalDisplay(std::move(want), reply));
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(WMError::WM_OK, window_->OnHostRectChangeInGlobalDisplay(std::move(want), reply));
}

/**
 * @tc.name: OnResyncExtensionConfig
 * @tc.desc: OnResyncExtensionConfig Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnResyncExtensionConfig, Function | SmallTest | Level3)
{
    AAFwk::Want want;
    AAFwk::WantParams configParam;
    AAFwk::WantParams wantParam;
    configParam.SetParam(Extension::CROSS_AXIS_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(CrossAxisState::STATE_CROSS)));
    configParam.SetParam(Extension::WATERFALL_MODE_FIELD, AAFwk::Integer::Box(static_cast<int32_t>(1)));
    configParam.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(true)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want.SetParams(wantParam);
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    EXPECT_EQ(WMError::WM_OK, window_->OnResyncExtensionConfig(std::move(want), reply));
    EXPECT_EQ(CrossAxisState::STATE_CROSS, window_->crossAxisState_.load());
    EXPECT_TRUE(window_->IsWaterfallModeEnabled());
    EXPECT_TRUE(window_->IsWindowDelayRaiseEnabled());
}

/**
 * @tc.name: SendExtensionMessageToHost
 * @tc.desc: SendExtensionMessageToHost Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SendExtensionMessageToHost, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SendExtensionMessageToHost");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    window->dataHandler_ = nullptr;
    uint32_t code = 111;
    AAFwk::Want data;

    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SendExtensionMessageToHost(code, data));
    window->dataHandler_ = std::make_shared<Extension::ProviderDataHandler>();
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, window->SendExtensionMessageToHost(code, data));
    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    EXPECT_EQ(WMError::WM_OK, window->SendExtensionMessageToHost(code, data));
}

/**
 * @tc.name: OnExtensionMessage
 * @tc.desc: OnExtensionMessage test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnExtensionMessage, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnExtensionMessage");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t code = 9999;
    int32_t persistentId = 1111;
    AAFwk::Want want;
    window->dataHandler_ = nullptr;
    auto ret = window->OnExtensionMessage(code, persistentId, want);
    EXPECT_EQ(WMError::WM_OK, ret);

    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->OnExtensionMessage(code, persistentId, want));

    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER);
    window->rectChangeInGlobalDisplayUIExtListenerIds_.emplace(111);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
}

/**
 * @tc.name: OnExtensionMessage_WindowRise
 * @tc.desc: OnExtensionMessage_WindowRise test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnExtensionMessage_WindowRise, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnExtensionMessage");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t code = 9999;
    int32_t persistentId = 1111;
    AAFwk::Want want;
    window->dataHandler_ = nullptr;

    code = static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_TO_RAISE);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->OnExtensionMessage(code, persistentId, want));

    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    code = static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_TO_RAISE);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
}

/**
 * @tc.name: OnExtensionMessage_KeyboardListener
 * @tc.desc: OnExtensionMessage_KeyboardListener test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnExtensionMessage_KeyboardListener, TestSize.Level1)
{
    SessionInfo sessionInfo;
    window_->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t code = 9999;
    int32_t persistentId = 1111;
    AAFwk::Want want;
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_SHOW_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window_->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_HIDE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window_->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_SHOW_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window_->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_HIDE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window_->OnExtensionMessage(code, persistentId, want));
}

/**
 * @tc.name: OnExtensionMessage_WindowRise
 * @tc.desc: OnExtensionMessage_WindowRise test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnExtensionMessage_HostWindowRect, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnExtensionMessage");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t code = 9999;
    int32_t persistentId = 1111;
    AAFwk::Want want;
    window->dataHandler_ = nullptr;

    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->OnExtensionMessage(code, persistentId, want));

    window->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER);
    window->rectChangeUIExtListenerIds_.emplace(111);
    ASSERT_FALSE(window->rectChangeUIExtListenerIds_.empty());
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    window->rectChangeUIExtListenerIds_.clear();
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
}

/**
 * @tc.name: UpdateExtensionConfig
 * @tc.desc: UpdateExtensionConfig test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateExtensionConfig, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateExtensionConfig");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->crossAxisState_ = CrossAxisState::STATE_INVALID;
    auto want = std::make_shared<AAFwk::Want>();
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->crossAxisState_.load(), CrossAxisState::STATE_INVALID);

    AAFwk::WantParams configParam;
    AAFwk::WantParams wantParam(want->GetParams());
    configParam.SetParam(Extension::CROSS_AXIS_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(CrossAxisState::STATE_CROSS)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want->SetParams(wantParam);
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->crossAxisState_.load(), CrossAxisState::STATE_CROSS);
    configParam.SetParam(Extension::CROSS_AXIS_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(CrossAxisState::STATE_INVALID)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want->SetParams(wantParam);
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->crossAxisState_.load(), CrossAxisState::STATE_INVALID);
    configParam.SetParam(Extension::CROSS_AXIS_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(CrossAxisState::STATE_NO_CROSS)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want->SetParams(wantParam);
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->crossAxisState_.load(), CrossAxisState::STATE_NO_CROSS);
    configParam.SetParam(Extension::CROSS_AXIS_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(CrossAxisState::STATE_END)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want->SetParams(wantParam);
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->crossAxisState_.load(), CrossAxisState::STATE_NO_CROSS);

    bool isHostWindowDelayRaiseEnabled = true;
    configParam.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(isHostWindowDelayRaiseEnabled)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want->SetParams(wantParam);
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->IsWindowDelayRaiseEnabled(), isHostWindowDelayRaiseEnabled);
    isHostWindowDelayRaiseEnabled = false;
    configParam.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(isHostWindowDelayRaiseEnabled)));
    wantParam.SetParam(Extension::UIEXTENSION_CONFIG_FIELD, AAFwk::WantParamWrapper::Box(configParam));
    want->SetParams(wantParam);
    window->UpdateExtensionConfig(want);
    EXPECT_EQ(window->IsWindowDelayRaiseEnabled(), isHostWindowDelayRaiseEnabled);
}

/**
 * @tc.name: IsComponentFocused
 * @tc.desc: IsComponentFocused test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, IsComponentFocused, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsComponentFocused");
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    EXPECT_FALSE(window->IsComponentFocused());

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_FALSE(window->IsComponentFocused());
    window->focusState_ = false;
    EXPECT_FALSE(window->IsComponentFocused());
    window->focusState_ = true;
    EXPECT_TRUE(window->IsComponentFocused());
}

/**
 * @tc.name: GetGestureBackEnabled
 * @tc.desc: GetGestureBackEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetGestureBackEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("GetGestureBackEnabled");
    bool enable = false;
    window->hostGestureBackEnabled_ = false;
    EXPECT_EQ(WMError::WM_OK, window->GetGestureBackEnabled(enable));
    EXPECT_EQ(false, enable);
    window->hostGestureBackEnabled_ = true;
    EXPECT_EQ(WMError::WM_OK, window->GetGestureBackEnabled(enable));
    EXPECT_EQ(true, enable);
}

/**
 * @tc.name: SetLayoutFullScreen
 * @tc.desc: SetLayoutFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetLayoutFullScreen, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetLayoutFullScreen");
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(false));
    EXPECT_EQ(false, window->isIgnoreSafeArea_);
    EXPECT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(true));
    EXPECT_EQ(true, window->isIgnoreSafeArea_);
}

/**
 * @tc.name: UpdateSystemBarProperties
 * @tc.desc: UpdateSystemBarProperties test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateSystemBarProperties, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("UpdateSystemBarProperties");
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = false;
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED,
        window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_CALL(*session, TransferExtensionData).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
}

/**
 * @tc.name: SetGestureBackEnabled
 * @tc.desc: SetGestureBackEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetGestureBackEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetGestureBackEnabled");
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, window->SetGestureBackEnabled(true));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_CALL(*session, TransferExtensionData).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(WMError::WM_OK, window->SetGestureBackEnabled(true));
}

/**
 * @tc.name: SetImmersiveModeEnabledState
 * @tc.desc: SetImmersiveModeEnabledState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetImmersiveModeEnabledState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetImmersiveModeEnabledState");

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(true));
}

/**
 * @tc.name: GetImmersiveModeEnabledState
 * @tc.desc: GetImmersiveModeEnabledState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetImmersiveModeEnabledState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("GetImmersiveModeEnabledState");
    window->immersiveModeEnabled_ = false;
    EXPECT_EQ(false, window->GetImmersiveModeEnabledState());
}

/**
 * @tc.name: GetImmersiveModeEnabledState1
 * @tc.desc: GetImmersiveModeEnabledState1 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, GetImmersiveModeEnabledState1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("GetImmersiveModeEnabledState");
    window->immersiveModeEnabled_ = true;
    EXPECT_EQ(true, window->GetImmersiveModeEnabledState());
}

/**
 * @tc.name: UpdateHostSpecificSystemBarEnabled
 * @tc.desc: UpdateHostSpecificSystemBarEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateHostSpecificSystemBarEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("UpdateHostSpecificSystemBarEnabled");
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, window->UpdateHostSpecificSystemBarEnabled("status", true, true));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_CALL(*session, TransferExtensionData).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(WMError::WM_OK, window->UpdateHostSpecificSystemBarEnabled("status", true, true));
}

/**
 * @tc.name: ExtensionSetKeepScreenOn
 * @tc.desc: ExtensionSetKeepScreenOn test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ExtensionSetKeepScreenOn, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("ExtensionSetKeepScreenOn");
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, window->ExtensionSetKeepScreenOn(true));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_CALL(*session, TransferExtensionData).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(WMError::WM_OK, window->ExtensionSetKeepScreenOn(true));
}

/**
 * @tc.name: ExtensionSetBrightness
 * @tc.desc: ExtensionSetBrightness test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, ExtensionSetBrightness, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowExtensionSessionImpl> window = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("ExtensionSetBrightness");
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, window->ExtensionSetBrightness(0.5));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_CALL(*session, TransferExtensionData).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(WMError::WM_OK, window->ExtensionSetBrightness(0.5));
}

/**
 * @tc.name: OnScreenshot
 * @tc.desc: OnScreenshot Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnScreenshot, TestSize.Level1)
{
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    EXPECT_EQ(WMError::WM_OK, window_->OnScreenshot(std::move(want), reply));
}

/**
 * @tc.name: OnExtensionSecureLimitChange
 * @tc.desc: OnExtensionSecureLimitChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnExtensionSecureLimitChange, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam(Extension::EXTENSION_SECURE_LIMIT_CHANGE, true);
    std::optional<AAFwk::Want> reply = std::make_optional<AAFwk::Want>();
    EXPECT_EQ(WMError::WM_OK, window_->OnExtensionSecureLimitChange(std::move(want), reply));
}

/**
 * @tc.name: RegisterKeyboardDidShowListener
 * @tc.desc: RegisterKeyboardDidShowListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterKeyboardDidShowListener, TestSize.Level1)
{
    window_->dataHandler_ = nullptr;
    sptr<IKeyboardDidShowListener> listener = sptr<IKeyboardDidShowListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window_->RegisterKeyboardDidShowListener(listener));
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();

    EXPECT_EQ(WMError::WM_OK, window_->RegisterKeyboardDidShowListener(listener));
    // Test listener already registered
    EXPECT_EQ(WMError::WM_OK, window_->RegisterKeyboardDidShowListener(listener));
}

/**
 * @tc.name: UnregisterKeyboardDidShowListener
 * @tc.desc: UnregisterKeyboardDidShowListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UnregisterKeyboardDidShowListener, TestSize.Level1)
{
    window_->dataHandler_ = nullptr;
    sptr<IKeyboardDidShowListener> listener = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, window_->UnregisterKeyboardDidShowListener(listener));

    // Test need notify host
    listener = sptr<IKeyboardDidShowListener>::MakeSptr();
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    window_->RegisterKeyboardDidShowListener(listener);
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterKeyboardDidShowListener(listener));
    ASSERT_TRUE(window_->keyboardDidShowListenerList_.empty());

    // Test no need notify host
    window_->keyboardDidShowUIExtListeners_[1] = listener;
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterKeyboardDidShowListener(listener));
}

/**
 * @tc.name: RegisterKeyboardDidHideListener
 * @tc.desc: RegisterKeyboardDidHideListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterKeyboardDidHideListener, TestSize.Level1)
{
    window_->dataHandler_ = nullptr;
    sptr<IKeyboardDidHideListener> listener = sptr<IKeyboardDidHideListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window_->RegisterKeyboardDidHideListener(listener));
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();

    EXPECT_EQ(WMError::WM_OK, window_->RegisterKeyboardDidHideListener(listener));
    // Test listener already registered
    EXPECT_EQ(WMError::WM_OK, window_->RegisterKeyboardDidHideListener(listener));
}

/**
 * @tc.name: UnregisterKeyboardDidHideListener
 * @tc.desc: UnregisterKeyboardDidHideListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UnregisterKeyboardDidHideListener, TestSize.Level1)
{
    window_->dataHandler_ = nullptr;
    sptr<IKeyboardDidHideListener> listener = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, window_->UnregisterKeyboardDidHideListener(listener));

    // Test need notify host
    listener = sptr<IKeyboardDidHideListener>::MakeSptr();
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    window_->RegisterKeyboardDidHideListener(listener);
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterKeyboardDidHideListener(listener));
    ASSERT_TRUE(window_->keyboardDidHideListenerList_.empty());

    // Test no need notify host
    window_->keyboardDidHideUIExtListeners_[1] = listener;
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterKeyboardDidHideListener(listener));
}

/**
 * @tc.name: OnKeyboardDidShow
 * @tc.desc: OnKeyboardDidShow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnKeyboardDidShow, TestSize.Level1)
{
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply;
    sptr<IKeyboardDidShowListener> listener = sptr<IKeyboardDidShowListener>::MakeSptr();
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    window_->RegisterKeyboardDidShowListener(listener);
    EXPECT_EQ(WMError::WM_OK, window_->OnKeyboardDidShow(std::move(want), reply));
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(WMError::WM_OK, window_->OnKeyboardDidShow(std::move(want), reply));
}

/**
 * @tc.name: OnKeyboardDidHide
 * @tc.desc: OnKeyboardDidHide Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnKeyboardDidHide, TestSize.Level1)
{
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply;
    sptr<IKeyboardDidHideListener> listener = sptr<IKeyboardDidHideListener>::MakeSptr();
    window_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    window_->RegisterKeyboardDidHideListener(listener);
    EXPECT_EQ(WMError::WM_OK, window_->OnKeyboardDidHide(std::move(want), reply));
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(WMError::WM_OK, window_->OnKeyboardDidHide(std::move(want), reply));
}

/**
 * @tc.name: OnHostStatusBarContentColorChange
 * @tc.desc: OnHostStatusBarContentColorChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, OnHostStatusBarContentColorChange, TestSize.Level1)
{
    AAFwk::Want want;
    std::optional<AAFwk::Want> reply;
    want.SetParam(Extension::HOST_STATUS_BAR_CONTENT_COLOR, static_cast<int32_t>(8888));
    EXPECT_EQ(WMError::WM_OK, window_->OnHostStatusBarContentColorChange(std::move(want), reply));
    EXPECT_EQ(8888, window_->GetHostStatusBarContentColor());
}

/**
 * @tc.name: RegisterOccupiedAreaChangeListener
 * @tc.desc: RegisterOccupiedAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, RegisterOccupiedAreaChangeListener, TestSize.Level1)
{
    sptr<IOccupiedAreaChangeListener> listener = sptr<IOccupiedAreaChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window_->RegisterOccupiedAreaChangeListener(listener));
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    window_->NotifyOccupiedAreaChange(info);
    EXPECT_EQ(WMError::WM_OK, window_->UnregisterOccupiedAreaChangeListener(listener));
}

/**
 * @tc.name: UpdateRotateDuration
 * @tc.desc: UpdateRotateDuration Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, UpdateRotateDuration, TestSize.Level2)
{
    WindowSizeChangeReason reason = WindowSizeChangeReason::SNAPSHOT_ROTATION;
    int32_t duration = -1;
    int32_t transactionDuration = 123;
    std::shared_ptr<RSTransaction> rsTransaction = std::make_shared<RSTransaction>();
    rsTransaction->duration_ = transactionDuration;
    window_->UpdateRotateDuration(reason, duration, rsTransaction);
    EXPECT_EQ(duration, transactionDuration);
    EXPECT_EQ(reason, WindowSizeChangeReason::ROTATION);

    duration = -1;
    reason = WindowSizeChangeReason::ROTATION;
    window_->UpdateRotateDuration(reason, duration, rsTransaction);
    EXPECT_EQ(duration, transactionDuration);
    EXPECT_EQ(reason, WindowSizeChangeReason::ROTATION);
}

/**
 * @tc.name: SetStatusBarColorForExtension
 * @tc.desc: SetStatusBarColorForExtension test
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTest, SetStatusBarColorForExtension, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, window_->SetStatusBarColorForExtension(255));

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window_->hostSession_ = session;
    window_->property_->SetPersistentId(1);
    EXPECT_CALL(*session, TransferExtensionData).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(WMError::WM_OK, window_->SetStatusBarColorForExtension(255));
}
}
} // namespace Rosen
} // namespace OHOS