/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "extension/extension_business_info.h"
#include "extension_data_handler_mock.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "mock_window_scene_session_impl.h"
#include "window_extension_session_impl.h"
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Rosen {

class WindowExtensionSessionImplTouchOutsideTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    sptr<WindowExtensionSessionImpl> extWindow_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    std::shared_ptr<Extension::DataHandler> savedDataHandler_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowExtensionSessionImplTouchOutsideTest::SetUpTestCase()
{
}

void WindowExtensionSessionImplTouchOutsideTest::TearDownTestCase()
{
}

void WindowExtensionSessionImplTouchOutsideTest::SetUp()
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("TouchOutsideTest");
    extWindow_ = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, extWindow_);
    if (!handler_) {
        auto runner = AppExecFwk::EventRunner::Create("TouchOutsideTest");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    extWindow_->handler_ = handler_;
    SessionInfo sessionInfo;
    extWindow_->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    extWindow_->property_->SetPersistentId(1);
    savedDataHandler_ = std::make_shared<Extension::MockDataHandler>();
    extWindow_->dataHandler_ = savedDataHandler_;
}

void WindowExtensionSessionImplTouchOutsideTest::TearDown()
{
    if (extWindow_ != nullptr) {
        extWindow_->dataHandler_ = savedDataHandler_;
        extWindow_->touchOutsideListeners_.clear();
        extWindow_->touchOutsideUIExtListenerIds_.clear();
        extWindow_->touchOutsideUIExtListeners_.clear();
    }
    extWindow_ = nullptr;
}

class WindowSceneSessionImplTouchOutsideTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    sptr<WindowSceneSessionImpl> sceneWindow_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    std::shared_ptr<Ace::UIContent> savedUiContent_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneSessionImplTouchOutsideTest::SetUpTestCase()
{
}

void WindowSceneSessionImplTouchOutsideTest::TearDownTestCase()
{
}

void WindowSceneSessionImplTouchOutsideTest::SetUp()
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("SceneTouchOutsideTest");
    sceneWindow_ = sptr<MockWindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, sceneWindow_);
    if (!handler_) {
        auto runner = AppExecFwk::EventRunner::Create("SceneTouchOutsideTest");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    sceneWindow_->handler_ = handler_;
    sceneWindow_->property_->SetPersistentId(2);
    savedUiContent_ = nullptr;
}

void WindowSceneSessionImplTouchOutsideTest::TearDown()
{
    if (sceneWindow_ != nullptr) {
        sceneWindow_->uiContent_ = std::move(savedUiContent_);
        sceneWindow_->touchOutsideListeners_.clear();
        sceneWindow_->touchOutsideUIExtListenerIds_.clear();
        sceneWindow_->touchOutsideUIExtListeners_.clear();
    }
    sceneWindow_ = nullptr;
}

namespace {
/**
 * @tc.name: RegisterTouchOutsideListener01
 * @tc.desc: Register touchOutside listener with valid listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, RegisterTouchOutsideListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
    EXPECT_FALSE(extWindow_->touchOutsideListeners_[extWindow_->GetPersistentId()].empty());
}

/**
 * @tc.name: RegisterTouchOutsideListener02
 * @tc.desc: Register touchOutside listener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, RegisterTouchOutsideListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<ITouchOutsideListener> nullListener = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, extWindow_->RegisterTouchOutsideListener(nullListener));
}

/**
 * @tc.name: RegisterTouchOutsideListener03
 * @tc.desc: Register touchOutside listener when dataHandler is nullptr, SendExtensionMessageToHost fails
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, RegisterTouchOutsideListener03, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    extWindow_->dataHandler_ = nullptr;
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    EXPECT_NE(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
    extWindow_->dataHandler_ = savedDataHandler_;
}

/**
 * @tc.name: RegisterTouchOutsideListener04
 * @tc.desc: Register same touchOutside listener twice, expect WM_OK for duplicate registration
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, RegisterTouchOutsideListener04, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
}

/**
 * @tc.name: UnregisterTouchOutsideListener01
 * @tc.desc: Unregister touchOutside listener after successful registration, needNotifyHost is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, UnregisterTouchOutsideListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
    EXPECT_EQ(WMError::WM_OK, extWindow_->UnregisterTouchOutsideListener(listener));
    EXPECT_TRUE(extWindow_->touchOutsideListeners_[extWindow_->GetPersistentId()].empty());
}

/**
 * @tc.name: UnregisterTouchOutsideListener02
 * @tc.desc: Unregister touchOutside listener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, UnregisterTouchOutsideListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<ITouchOutsideListener> nullListener = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, extWindow_->UnregisterTouchOutsideListener(nullListener));
}

/**
 * @tc.name: UnregisterTouchOutsideListener03
 * @tc.desc: Unregister touchOutside listener when UIExt listener map exists, needNotifyHost is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, UnregisterTouchOutsideListener03, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
    extWindow_->touchOutsideUIExtListeners_[100] = sptr<ITouchOutsideListener>::MakeSptr();
    extWindow_->touchOutsideUIExtListenerIds_.emplace(100);
    EXPECT_EQ(WMError::WM_OK, extWindow_->UnregisterTouchOutsideListener(listener));
    EXPECT_TRUE(extWindow_->touchOutsideListeners_[extWindow_->GetPersistentId()].empty());
    EXPECT_FALSE(extWindow_->touchOutsideUIExtListeners_.empty());
}

/**
 * @tc.name: UnregisterTouchOutsideListener04
 * @tc.desc: Unregister one listener when multiple listeners exist, remaining listener not empty
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, UnregisterTouchOutsideListener04, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<MockTouchOutsideListener> listener1 = sptr<MockTouchOutsideListener>::MakeSptr();
    sptr<MockTouchOutsideListener> listener2 = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener1);
    ASSERT_NE(nullptr, listener2);
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener1));
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener2));
    EXPECT_EQ(WMError::WM_OK, extWindow_->UnregisterTouchOutsideListener(listener1));
    EXPECT_FALSE(extWindow_->touchOutsideListeners_[extWindow_->GetPersistentId()].empty());
}

/**
 * @tc.name: HandleUIExtRegisterTouchOutsideListener01
 * @tc.desc: Handle UIExt register touchOutside listener, persistentId added to tracking set
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, HandleUIExtRegisterTouchOutsideListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 100;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtRegisterTouchOutsideListener(code, persistentId, data));
    EXPECT_TRUE(extWindow_->touchOutsideUIExtListenerIds_.find(persistentId) !=
        extWindow_->touchOutsideUIExtListenerIds_.end());
}

/**
 * @tc.name: HandleUIExtRegisterTouchOutsideListener02
 * @tc.desc: Handle UIExt register touchOutside listener when dataHandler is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, HandleUIExtRegisterTouchOutsideListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    extWindow_->dataHandler_ = nullptr;
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 100;
    AAFwk::Want data;
    EXPECT_NE(WMError::WM_OK,
        extWindow_->HandleUIExtRegisterTouchOutsideListener(code, persistentId, data));
    extWindow_->dataHandler_ = savedDataHandler_;
}

/**
 * @tc.name: HandleUIExtUnregisterTouchOutsideListener01
 * @tc.desc: Handle UIExt unregister touchOutside listener after register, persistentId removed from set
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, HandleUIExtUnregisterTouchOutsideListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    uint32_t registerCode = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    uint32_t unregisterCode = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 100;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtRegisterTouchOutsideListener(registerCode, persistentId, data));
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtUnregisterTouchOutsideListener(unregisterCode, persistentId, data));
    EXPECT_TRUE(extWindow_->touchOutsideUIExtListenerIds_.find(persistentId) ==
        extWindow_->touchOutsideUIExtListenerIds_.end());
}

/**
 * @tc.name: HandleUIExtUnregisterTouchOutsideListener02
 * @tc.desc: Handle UIExt unregister touchOutside listener with unregistered persistentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, HandleUIExtUnregisterTouchOutsideListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 200;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtUnregisterTouchOutsideListener(code, persistentId, data));
    EXPECT_TRUE(extWindow_->touchOutsideUIExtListenerIds_.find(persistentId) ==
        extWindow_->touchOutsideUIExtListenerIds_.end());
}

/**
 * @tc.name: HandleUIExtUnregisterTouchOutsideListener03
 * @tc.desc: Handle UIExt unregister when other UIExt listener still exists, needNotifyHost is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, HandleUIExtUnregisterTouchOutsideListener03, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    uint32_t registerCode = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    uint32_t unregisterCode = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtRegisterTouchOutsideListener(registerCode, 100, data));
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtRegisterTouchOutsideListener(registerCode, 200, data));
    EXPECT_EQ(WMError::WM_OK,
        extWindow_->HandleUIExtUnregisterTouchOutsideListener(unregisterCode, 100, data));
    EXPECT_FALSE(extWindow_->touchOutsideUIExtListenerIds_.empty());
}

/**
 * @tc.name: OnTouchOutside01
 * @tc.desc: OnTouchOutside callback with registered listener, listener callback invoked
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, OnTouchOutside01, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    EXPECT_EQ(WMError::WM_OK, extWindow_->RegisterTouchOutsideListener(listener));
    AAFwk::Want data;
    std::optional<AAFwk::Want> reply;
    EXPECT_CALL(*listener, OnTouchOutside()).Times(1);
    EXPECT_EQ(WMError::WM_OK, extWindow_->OnTouchOutside(std::move(data), reply));
}

/**
 * @tc.name: OnTouchOutside02
 * @tc.desc: OnTouchOutside callback without any registered listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, OnTouchOutside02, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    AAFwk::Want data;
    std::optional<AAFwk::Want> reply;
    EXPECT_EQ(WMError::WM_OK, extWindow_->OnTouchOutside(std::move(data), reply));
}

/**
 * @tc.name: OnExtensionMessage01
 * @tc.desc: OnExtensionMessage with REGISTER_TOUCH_OUTSIDE_LISTENER business code
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, OnExtensionMessage01, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 100;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK, extWindow_->OnExtensionMessage(code, persistentId, data));
}

/**
 * @tc.name: OnExtensionMessage02
 * @tc.desc: OnExtensionMessage with UNREGISTER_TOUCH_OUTSIDE_LISTENER business code
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionSessionImplTouchOutsideTest, OnExtensionMessage02, TestSize.Level1)
{
    ASSERT_NE(nullptr, extWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 100;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK, extWindow_->OnExtensionMessage(code, persistentId, data));
}

/**
 * @tc.name: HandleUIExtRegisterTouchOutsideListener01
 * @tc.desc: Host window HandleUIExt register, proxy listener created and added to containers
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, HandleUIExtRegisterTouchOutsideListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 101;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        sceneWindow_->HandleUIExtRegisterTouchOutsideListener(code, persistentId, data));
    EXPECT_TRUE(sceneWindow_->touchOutsideUIExtListeners_.find(persistentId) !=
        sceneWindow_->touchOutsideUIExtListeners_.end());
    EXPECT_TRUE(sceneWindow_->touchOutsideUIExtListenerIds_.find(persistentId) !=
        sceneWindow_->touchOutsideUIExtListenerIds_.end());
    EXPECT_FALSE(sceneWindow_->touchOutsideListeners_[sceneWindow_->GetPersistentId()].empty());
}

/**
 * @tc.name: HandleUIExtRegisterTouchOutsideListener02
 * @tc.desc: Host window HandleUIExt register same persistentId twice, only one proxy created
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, HandleUIExtRegisterTouchOutsideListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 101;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        sceneWindow_->HandleUIExtRegisterTouchOutsideListener(code, persistentId, data));
    EXPECT_EQ(WMError::WM_OK,
        sceneWindow_->HandleUIExtRegisterTouchOutsideListener(code, persistentId, data));
    EXPECT_EQ(1u, sceneWindow_->touchOutsideUIExtListeners_.size());
}

/**
 * @tc.name: HandleUIExtUnregisterTouchOutsideListener01
 * @tc.desc: Host window HandleUIExt unregister after register, proxy listener removed from containers
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, HandleUIExtUnregisterTouchOutsideListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    uint32_t registerCode = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    uint32_t unregisterCode = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 101;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        sceneWindow_->HandleUIExtRegisterTouchOutsideListener(registerCode, persistentId, data));
    EXPECT_EQ(WMError::WM_OK,
        sceneWindow_->HandleUIExtUnregisterTouchOutsideListener(unregisterCode, persistentId, data));
    EXPECT_TRUE(sceneWindow_->touchOutsideUIExtListeners_.find(persistentId) ==
        sceneWindow_->touchOutsideUIExtListeners_.end());
    EXPECT_TRUE(sceneWindow_->touchOutsideUIExtListenerIds_.find(persistentId) ==
        sceneWindow_->touchOutsideUIExtListenerIds_.end());
}

/**
 * @tc.name: HandleUIExtUnregisterTouchOutsideListener02
 * @tc.desc: Host window HandleUIExt unregister with unregistered persistentId, no effect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, HandleUIExtUnregisterTouchOutsideListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 999;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK,
        sceneWindow_->HandleUIExtUnregisterTouchOutsideListener(code, persistentId, data));
    EXPECT_TRUE(sceneWindow_->touchOutsideUIExtListeners_.empty());
}

/**
 * @tc.name: NotifyUIExtTouchOutside01
 * @tc.desc: NotifyUIExtTouchOutside when uiContent is nullptr, early return
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, NotifyUIExtTouchOutside01, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    savedUiContent_ = std::move(sceneWindow_->uiContent_);
    sceneWindow_->uiContent_ = nullptr;
    sceneWindow_->NotifyUIExtTouchOutside();
}

/**
 * @tc.name: NotifyUIExtTouchOutside02
 * @tc.desc: NotifyUIExtTouchOutside with empty UIExt listener IDs, no message sent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, NotifyUIExtTouchOutside02, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    savedUiContent_ = std::move(sceneWindow_->uiContent_);
    sceneWindow_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    sceneWindow_->touchOutsideUIExtListenerIds_.clear();
    sceneWindow_->NotifyUIExtTouchOutside();
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyTouchOutside01
 * @tc.desc: NotifyTouchOutside with registered listener, listener callback invoked
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, NotifyTouchOutside01, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    sptr<MockTouchOutsideListener> listener = sptr<MockTouchOutsideListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    sceneWindow_->RegisterTouchOutsideListener(listener);
    EXPECT_CALL(*listener, OnTouchOutside()).Times(1);
    EXPECT_EQ(WSError::WS_OK, sceneWindow_->NotifyTouchOutside());
}

/**
 * @tc.name: NotifyTouchOutside02
 * @tc.desc: NotifyTouchOutside without any registered listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, NotifyTouchOutside02, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    EXPECT_EQ(WSError::WS_OK, sceneWindow_->NotifyTouchOutside());
}

/**
 * @tc.name: OnExtensionMessage01
 * @tc.desc: Host window OnExtensionMessage with REGISTER_TOUCH_OUTSIDE_LISTENER business code
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, OnExtensionMessage01, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 101;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK, sceneWindow_->OnExtensionMessage(code, persistentId, data));
}

/**
 * @tc.name: OnExtensionMessage02
 * @tc.desc: Host window OnExtensionMessage with UNREGISTER_TOUCH_OUTSIDE_LISTENER business code
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTouchOutsideTest, OnExtensionMessage02, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneWindow_);
    uint32_t code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_TOUCH_OUTSIDE_LISTENER);
    int32_t persistentId = 101;
    AAFwk::Want data;
    EXPECT_EQ(WMError::WM_OK, sceneWindow_->OnExtensionMessage(code, persistentId, data));
}
} // namespace
} // namespace Rosen
} // namespace OHOS