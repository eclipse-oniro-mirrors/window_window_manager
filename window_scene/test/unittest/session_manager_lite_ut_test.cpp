/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "session_manager.h"
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <ipc_skeleton.h>
#include "scene_board_judgement.h"
#include "session_manager_lite.h"
#include "session_manager_service_recover_interface.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionManagerLiteUTTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    std::shared_ptr<SessionManagerLite> sml_;
};

void SessionManagerLiteUTTest::SetUpTestCase() {}

void SessionManagerLiteUTTest::TearDownTestCase() {}

void SessionManagerLiteUTTest::SetUp()
{
    sml_ = std::make_shared<SessionManagerLite>();
    ASSERT_NE(nullptr, sml_);
}

void SessionManagerLiteUTTest::TearDown()
{
    sml_ = nullptr;
}

namespace {
/**
 * @tc.name: GetSceneSessionManagerLiteProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, GetSceneSessionManagerLiteProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->RemoveSSMDeathRecipient();
    sml_->ClearSessionManagerProxy();
    auto sceneSessionManagerLiteProxy = sml_->GetSceneSessionManagerLiteProxy();
    ASSERT_EQ(nullptr, sceneSessionManagerLiteProxy);

    sml_->ClearSessionManagerProxy();
    sml_->GetSessionManagerServiceProxy();
    sceneSessionManagerLiteProxy = sml_->GetSceneSessionManagerLiteProxy();
    ASSERT_EQ(nullptr, sceneSessionManagerLiteProxy);
}

/**
 * @tc.name: InitSceneSessionManagerLiteProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, InitSceneSessionManagerLiteProxy01, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->InitSceneSessionManagerLiteProxy();
    ASSERT_EQ(nullptr, sml_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: InitSceneSessionManagerLiteProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, InitSceneSessionManagerLiteProxy02, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->GetSceneSessionManagerLiteProxy();
    sml_->InitSceneSessionManagerLiteProxy();
    ASSERT_EQ(nullptr, sml_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: ClearSessionManagerProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, ClearSessionManagerProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->ClearSessionManagerProxy();
    ASSERT_EQ(sml_->sessionManagerServiceProxy_, nullptr);

    sml_->isRecoverListenerRegistered_ = true;
    sml_->GetSessionManagerServiceProxy();
    sml_->ClearSessionManagerProxy();
    ASSERT_EQ(sml_->sessionManagerServiceProxy_, nullptr);
}

/**
 * @tc.name: RecoverSessionManagerService
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, RecoverSessionManagerService, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    bool funcInvoked = false;
    sml_->RecoverSessionManagerService(nullptr);

    sml_->userSwitchCallbackFunc_ = [&]() { funcInvoked = true; };
    sml_->RecoverSessionManagerService(nullptr);
    ASSERT_EQ(funcInvoked, true);
}

/**
 * @tc.name: ReregisterSessionListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, ReregisterSessionListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->ReregisterSessionListener();
    ASSERT_EQ(nullptr, sml_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: OnWMSConnectionChangedCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnWMSConnectionChangedCallback, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    bool funcInvoked = false;
    sml_->wmsConnectionChangedFunc_ = nullptr;
    sml_->OnWMSConnectionChangedCallback(101, DEFAULT_SCREEN_ID, true);
    ASSERT_EQ(funcInvoked, false);

    sml_->wmsConnectionChangedFunc_ = [&](int32_t userId, int32_t screenId, bool isConnected) { funcInvoked = true; };
    sml_->OnWMSConnectionChangedCallback(101, DEFAULT_SCREEN_ID, true);
    ASSERT_EQ(funcInvoked, true);
}

/**
 * @tc.name: OnWMSConnectionChanged1
 * @tc.desc: wms disconnected
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnWMSConnectionChanged1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sptr<ISessionManagerService> sessionManagerService;
    sml_->isWMSConnected_ = true;
    sml_->currentWMSUserId_ = 100;
    sml_->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, false, sessionManagerService);
    ASSERT_EQ(sml_->isWMSConnected_, false);

    sml_->currentWMSUserId_ = 101;
    sml_->isWMSConnected_ = true;
    sml_->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, false, sessionManagerService);
    ASSERT_EQ(sml_->isWMSConnected_, true);
}

/**
 * @tc.name: OnWMSConnectionChanged2
 * @tc.desc: wms connected
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnWMSConnectionChanged2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sptr<ISessionManagerService> sessionManagerService;
    sml_->isWMSConnected_ = false;
    sml_->currentWMSUserId_ = INVALID_USER_ID;
    sml_->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, true, sessionManagerService);
    ASSERT_EQ(sml_->isWMSConnected_, true);

    // user switch
    sml_->currentWMSUserId_ = 100;
    sml_->isWMSConnected_ = true;
    sml_->OnWMSConnectionChanged(101, DEFAULT_SCREEN_ID, true, sessionManagerService);
    ASSERT_EQ(sml_->isWMSConnected_, true);
}

/**
 * @tc.name: OnUserSwitch
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnUserSwitch, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->OnUserSwitch(nullptr);
    ASSERT_EQ(nullptr, sml_->sessionManagerServiceProxy_);

    sml_->isRecoverListenerRegistered_ = true;
    bool funInvoked = false;
    sml_->userSwitchCallbackFunc_ = [&]() { funInvoked = true; };
    auto sessionManagerService = sml_->GetSessionManagerServiceProxy();
    sml_->OnUserSwitch(sessionManagerService);
}

/**
 * @tc.name: OnRemoteDied1
 * @tc.desc: foundation died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnRemoteDied1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    FoundationDeathRecipient foundationDeathRecipient;
    wptr<IRemoteObject> wptrDeath;
    foundationDeathRecipient.OnRemoteDied(wptrDeath);
    ASSERT_EQ(false, sml_->isWMSConnected_);
    ASSERT_EQ(false, sml_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, sml_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sml_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sml_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: OnRemoteDied2
 * @tc.desc: scb died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnRemoteDied2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    SSMDeathRecipient sSMDeathRecipient;
    wptr<IRemoteObject> wptrDeath;
    sSMDeathRecipient.OnRemoteDied(wptrDeath);
    ASSERT_EQ(nullptr, sml_->sessionManagerServiceProxy_);
}

/**
 * @tc.name: OnFoundationDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, OnFoundationDied, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->OnFoundationDied();
    ASSERT_EQ(false, sml_->isWMSConnected_);
    ASSERT_EQ(false, sml_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, sml_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sml_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sml_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: WMSConnectionChangedCallbackFunc is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    auto ret = sml_->RegisterWMSConnectionChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener1
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, RegisterWMSConnectionChangedListener1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->isRecoverListenerRegistered_ = true;
    sml_->currentWMSUserId_ = 100;
    sml_->currentScreenId_ = 0;
    sml_->isWMSConnected_ = true;
    auto callbackFunc = [](int32_t userId, int32_t screenId, bool isConnected) {};
    auto ret = sml_->RegisterWMSConnectionChangedListener(callbackFunc);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterSMSRecoverListener1
 * @tc.desc: mockSessionManagerServiceProxy_ is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, RegisterSMSRecoverListener1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->isRecoverListenerRegistered_ = false;
    sml_->mockSessionManagerServiceProxy_ = nullptr;
    sml_->RegisterSMSRecoverListener();
    ASSERT_EQ(sml_->isRecoverListenerRegistered_, false);
}

/**
 * @tc.name: RegisterSMSRecoverListener2
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, RegisterSMSRecoverListener2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->isRecoverListenerRegistered_ = false;
    sml_->GetMockSessionManagerServiceProxy();
    sml_->RegisterSMSRecoverListener();
    ASSERT_EQ(sml_->isRecoverListenerRegistered_, true);
}

/**
 * @tc.name: RegisterUserSwitchListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, RegisterUserSwitchListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->RegisterUserSwitchListener(nullptr);
    ASSERT_EQ(sml_->userSwitchCallbackFunc_, nullptr);

    sml_->RegisterUserSwitchListener([]() {});
    ASSERT_NE(sml_->userSwitchCallbackFunc_, nullptr);
}

/**
 * @tc.name: GetMockSessionManagerServiceProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteUTTest, GetMockSessionManagerServiceProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->GetMockSessionManagerServiceProxy();
    sml_->GetMockSessionManagerServiceProxy();
    ASSERT_NE(sml_->mockFoundationDeathRecipient_, nullptr);
}
} // namespace
} // namespace Rosen
} // namespace OHOS