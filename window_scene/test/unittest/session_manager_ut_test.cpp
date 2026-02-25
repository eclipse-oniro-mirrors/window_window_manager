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
#include "session_manager.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <ipc_skeleton.h>

#include "scene_board_judgement.h"
#include "session_manager_service_recover_interface.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"
#include "session_manager_lite.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
class SessionManagerUTTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    std::shared_ptr<SessionManager> sm_;
};

void SessionManagerUTTest::SetUpTestCase() {}

void SessionManagerUTTest::TearDownTestCase() {}

void SessionManagerUTTest::SetUp()
{
    sm_ = std::make_shared<SessionManager>();
    ASSERT_NE(nullptr, sm_);
}

void SessionManagerUTTest::TearDown()
{
    sm_ = nullptr;
}

namespace {
/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnRemoteRequest, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    IPCObjectStub iPCObjectStub;

    uint32_t code =
        static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::SessionManagerServiceRecoverMessage::
                                  TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER);
    auto ret = iPCObjectStub.OnRemoteRequest(code, data, reply, option);
    ASSERT_NE(ret, 0);

    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED);
    ret = iPCObjectStub.OnRemoteRequest(code, data, reply, option);
    ASSERT_NE(ret, 0);

    code = 10;
    ret = iPCObjectStub.OnRemoteRequest(code, data, reply, option);
    ASSERT_NE(0, ret);
}

/**
 * @tc.name: ClearSessionManagerProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, ClearSessionManagerProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->ClearSessionManagerProxy();
    ASSERT_EQ(sm_->sessionManagerServiceProxy_, nullptr);

    sm_->isRecoverListenerRegistered_ = true;
    sm_->sessionManagerServiceProxy_ = SessionManagerLite::GetInstance().GetSessionManagerServiceProxy();
    sm_->ClearSessionManagerProxy();
    ASSERT_EQ(sm_->sessionManagerServiceProxy_, nullptr);
}

/**
 * @tc.name: OnWMSConnectionChangedCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnWMSConnectionChangedCallback, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(nullptr, sm_);
    sm_->wmsConnectionChangedFunc_ = nullptr;
    sm_->OnWMSConnectionChangedCallback(101, DEFAULT_SCREEN_ID, true);
    EXPECT_FALSE(g_logMsg.find("WMS CallbackFunc is null.") != std::string::npos);

    sm_->wmsConnectionChangedFunc_ = [&](int32_t userId, int32_t screenId, bool isConnected) {};
    sm_->OnWMSConnectionChangedCallback(101, DEFAULT_SCREEN_ID, true);
    EXPECT_TRUE(g_logMsg.find("WMS connection changed") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnWMSConnectionChanged1
 * @tc.desc: wms disconnected
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnWMSConnectionChanged1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sptr<ISessionManagerService> sessionManagerService;
    sm_->isWMSConnected_ = true;
    sm_->currentWMSUserId_ = 100;
    sm_->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, false, sessionManagerService);
    ASSERT_EQ(sm_->isWMSConnected_, false);

    sm_->currentWMSUserId_ = 101;
    sm_->isWMSConnected_ = true;
    sm_->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, false, sessionManagerService);
    ASSERT_EQ(sm_->isWMSConnected_, true);
}

/**
 * @tc.name: OnWMSConnectionChanged2
 * @tc.desc: wms connected
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnWMSConnectionChanged2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sptr<ISessionManagerService> sessionManagerService;
    sm_->isWMSConnected_ = false;
    sm_->currentWMSUserId_ = INVALID_USER_ID;
    sm_->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, true, sessionManagerService);
    ASSERT_EQ(sm_->isWMSConnected_, true);

    // user switch
    sm_->currentWMSUserId_ = 100;
    sm_->isWMSConnected_ = true;
    sm_->OnWMSConnectionChanged(101, DEFAULT_SCREEN_ID, true, sessionManagerService);
    ASSERT_EQ(sm_->isWMSConnected_, true);
}

/**
 * @tc.name: RecoverSessionManagerService
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RecoverSessionManagerService, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(nullptr, sm_);
    sptr<ISessionManagerService> sessionManagerService;
    sm_->RecoverSessionManagerService(sessionManagerService);
    EXPECT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterUserSwitchListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RegisterUserSwitchListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->RegisterUserSwitchListener(nullptr);
    ASSERT_EQ(sm_->userSwitchCallbackFunc_, nullptr);

    sm_->RegisterUserSwitchListener([]() {});
    ASSERT_NE(sm_->userSwitchCallbackFunc_, nullptr);
}

/**
 * @tc.name: OnUserSwitch
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnUserSwitch, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(nullptr, sm_);
    sm_->OnUserSwitch(nullptr);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
    EXPECT_FALSE(g_logMsg.find("sceneSessionManagerServiceProxy is null") != std::string::npos);

    sm_->userSwitchCallbackFunc_ = [&]() {};
    auto sessionManagerService = SessionManagerLite::GetInstance().GetSessionManagerServiceProxy();
    sm_->OnUserSwitch(sessionManagerService);
    EXPECT_FALSE(g_logMsg.find("sceneSessionManagerServiceProxy is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnRemoteDied1
 * @tc.desc: foundation died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnRemoteDied1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    FoundationDeathRecipient foundationDeathRecipient;
    wptr<IRemoteObject> wptrDeath;
    foundationDeathRecipient.OnRemoteDied(wptrDeath);
    ASSERT_EQ(false, sm_->isWMSConnected_);
    ASSERT_EQ(false, sm_->isFoundationListenerRegistered_);
    ASSERT_EQ(false, sm_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, sm_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sceneSessionManagerProxy_);
}

/**
 * @tc.name: OnRemoteDied2
 * @tc.desc: scb died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnRemoteDied2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    SSMDeathRecipient sSMDeathRecipient;
    wptr<IRemoteObject> wptrDeath;
    sSMDeathRecipient.OnRemoteDied(wptrDeath);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
}

/**
 * @tc.name: OnFoundationDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, OnFoundationDied, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->OnFoundationDied();
    ASSERT_EQ(false, sm_->isWMSConnected_);
    ASSERT_EQ(false, sm_->isFoundationListenerRegistered_);
    ASSERT_EQ(false, sm_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, sm_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sceneSessionManagerProxy_);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: WMSConnectionChangedCallbackFunc is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    auto ret = sm_->RegisterWMSConnectionChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener1
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RegisterWMSConnectionChangedListener1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->isRecoverListenerRegistered_ = true;
    sm_->currentWMSUserId_ = 100;
    sm_->currentScreenId_ = 0;
    sm_->isWMSConnected_ = true;
    auto callbackFunc = [](int32_t userId, int32_t screenId, bool isConnected) {};
    auto ret = sm_->RegisterWMSConnectionChangedListener(callbackFunc);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterSMSRecoverListener1
 * @tc.desc: mockSessionManagerServiceProxy_ is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RegisterSMSRecoverListener1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->isRecoverListenerRegistered_ = false;
    sm_->mockSessionManagerServiceProxy_ = nullptr;
    sm_->RegisterSMSRecoverListener();
    ASSERT_EQ(sm_->isRecoverListenerRegistered_, false);
}

/**
 * @tc.name: RegisterSMSRecoverListener2
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RegisterSMSRecoverListener2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->isRecoverListenerRegistered_ = false;
    sm_->InitMockSMSProxy();
    sm_->RegisterSMSRecoverListener();
    ASSERT_EQ(sm_->isRecoverListenerRegistered_, true);
}

/**
 * @tc.name: InitMockSMSProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, InitMockSMSProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->InitMockSMSProxy();
    ASSERT_NE(sm_->foundationDeath_, nullptr);
}

/**
 * @tc.name: RegisterWindowManagerRecoverCallbackFunc
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerUTTest, RegisterWindowManagerRecoverCallbackFunc, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    auto testFunc = []() { return; };
    sm_->RegisterWindowManagerRecoverCallbackFunc(testFunc);
    ASSERT_NE(sm_->windowManagerRecoverFunc_, nullptr);
}
} // namespace
} // namespace Rosen
} // namespace OHOS