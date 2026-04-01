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

#include "session_manager.h"
#include "session_manager_lite.h"
#include "iremote_object_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionManagerTest : public Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void SessionManagerTest::SetUp()
{
}

void SessionManagerTest::TearDown()
{
}

/**
 * @tc.name: OnWMSConnectionChangedCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnWMSConnectionChangedCallback, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    ASSERT_NE(nullptr, sm);
    bool funcInvoked = false;
    sm->wmsConnectionChangedFunc_ = [&](int32_t userId, int32_t screenId, bool isConnected) { funcInvoked = true; };
    sm->OnWMSConnectionChangedCallback(101, DEFAULT_SCREEN_ID, true);
    sm->wmsConnectionChangedFunc_ = nullptr;
    ASSERT_EQ(funcInvoked, true);
}

/**
 * @tc.name: OnWMSConnectionChanged1
 * @tc.desc: wms disconnected
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnWMSConnectionChanged1, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    sptr<ISessionManagerService> sessionManagerService;
    sm->isWMSConnected_ = true;
    sm->currentWMSUserId_ = 100;
    sm->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, false, sessionManagerService);
    ASSERT_EQ(sm->isWMSConnected_, false);

    sm->currentWMSUserId_ = 101;
    sm->isWMSConnected_ = true;
    sm->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, false, sessionManagerService);
    ASSERT_EQ(sm->isWMSConnected_, true);
}

/**
 * @tc.name: OnWMSConnectionChanged2
 * @tc.desc: wms connected
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnWMSConnectionChanged2, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    sptr<ISessionManagerService> sessionManagerService;
    sm->isWMSConnected_ = false;
    sm->currentWMSUserId_ = INVALID_USER_ID;
    sm->OnWMSConnectionChanged(100, DEFAULT_SCREEN_ID, true, sessionManagerService);
    ASSERT_EQ(sm->isWMSConnected_, true);

    // user switch
    sm->currentWMSUserId_ = 100;
    sm->isWMSConnected_ = true;
    sm->OnWMSConnectionChanged(101, DEFAULT_SCREEN_ID, true, sessionManagerService);
    ASSERT_EQ(sm->isWMSConnected_, true);
}

/**
 * @tc.name: RegisterUserSwitchListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, RegisterUserSwitchListener, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    sm->RegisterUserSwitchListener(nullptr);
    ASSERT_EQ(sm->userSwitchCallbackFunc_, nullptr);

    sm->RegisterUserSwitchListener([]() {});
    ASSERT_NE(sm->userSwitchCallbackFunc_, nullptr);
}

/**
 * @tc.name: OnUserSwitch
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnUserSwitch, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    sm->OnUserSwitch(nullptr);
    ASSERT_EQ(nullptr, sm->sessionManagerServiceProxy_);

    bool funInvoked = false;
    sm->userSwitchCallbackFunc_ = [&]() { funInvoked = true; };
    auto sessionManagerService = SessionManagerLite::GetInstance().GetSessionManagerServiceProxy();
    ASSERT_NE(nullptr, sessionManagerService);

    // branch 1
    sm->OnUserSwitch(sessionManagerService);
    EXPECT_NE(funInvoked, true);

    // branch 2
    sm->userSwitchCallbackFunc_ = nullptr;
    sm->OnUserSwitch(sessionManagerService);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: WMSConnectionChangedCallbackFunc is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, RegisterWMSConnectionChangedListener, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    auto ret = sm->RegisterWMSConnectionChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener1
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, RegisterWMSConnectionChangedListener1, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    sm->isRecoverListenerRegistered_ = true;
    sm->currentWMSUserId_ = 100;
    sm->currentScreenId_ = 0;
    sm->isWMSConnected_ = true;
    auto callbackFunc = [](int32_t userId, int32_t screenId, bool isConnected) {};
    auto ret = sm->RegisterWMSConnectionChangedListener(callbackFunc);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWMSConnectionChangedListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, UnregisterWMSConnectionChangedListener, Function | SmallTest | Level2)
{
    sptr<SessionManager> sm = &SessionManager::GetInstance(100);
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    sm->mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remoteObject);
    auto ret = sm->UnregisterWMSConnectionChangedListener();
    ASSERT_EQ(WMError::WM_OK, ret);
}
} // namespace Rosen
} // namespace OHOS
