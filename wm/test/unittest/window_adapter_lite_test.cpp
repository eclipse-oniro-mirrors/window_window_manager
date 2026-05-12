/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "iremote_object_mocker.h"
#include "mock_window_manager_service.h"
#include "pointer_event.h"
#include "window_adapter_lite.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg = msg;
    }
}

namespace OHOS {
namespace Rosen {
class WindowAdapterLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<WindowAdapterLite> instance_ = nullptr;
    int32_t userId_ = 100;
};

void WindowAdapterLiteTest::SetUpTestCase() {}

void WindowAdapterLiteTest::TearDownTestCase() {}

void WindowAdapterLiteTest::SetUp()
{
    instance_ = &WindowAdapterLite::GetInstance(userId_);
}

void WindowAdapterLiteTest::TearDown()
{
    LOG_SetCallback(nullptr);
    instance_ = nullptr;
}

namespace {
/**
 * @tc.name: UnregisterWindowManagerAgent
 * @tc.desc: WindowAdapterLite/UnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent, TestSize.Level1)
{
    sptr<IWindowManagerAgent> windowManagerAgent = nullptr;
    std::set<sptr<IWindowManagerAgent>> iWindowManagerAgent = { nullptr };
    instance_->windowManagerLiteAgentMap_.insert(
        std::make_pair(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT, iWindowManagerAgent));
    int32_t pid = 0;
    auto ret = instance_->CheckWindowId(0, pid);
    ASSERT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowManagerAgent01
 * @tc.desc: WindowAdapterLite/UnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent01, TestSize.Level1)
{
    instance_->ReregisterWindowManagerLiteAgent();
    instance_->OnUserSwitch();
    instance_->ClearWMSProxy();

    sptr<WMSDeathRecipient> wmSDeathRecipient = sptr<WMSDeathRecipient>::MakeSptr();
    ASSERT_NE(wmSDeathRecipient, nullptr);
    wptr<IRemoteObject> wptrDeath;
    wmSDeathRecipient->OnRemoteDied(wptrDeath);

    FocusChangeInfo focusInfo;
    instance_->GetFocusWindowInfo(focusInfo);
    WindowModeType windowModeType;
    instance_->GetWindowModeType(windowModeType);
    int32_t pid = 0;
    auto ret = instance_->CheckWindowId(0, pid);

    ASSERT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: UpdateAnimationSpeedWithPid
 * @tc.desc: WindowAdapterLite/UpdateAnimationSpeedWithPid
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UpdateAnimationSpeedWithPid, TestSize.Level1)
{
    auto err = instance_->UpdateAnimationSpeedWithPid(10000, 2.0f);
    ASSERT_EQ(WMError::WM_OK, err);
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: WMSDeathRecipient
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, OnRemoteDied, TestSize.Level1)
{
    sptr<WMSDeathRecipient> wmSDeathRecipient = new WMSDeathRecipient();
    ASSERT_NE(wmSDeathRecipient, nullptr);

    wptr<IRemoteObject> wptrDeath = nullptr;
    wmSDeathRecipient->OnRemoteDied(wptrDeath);

    sptr<IRemoteObject> service = sptr<IRemoteObjectMocker>::MakeSptr();
    wptrDeath = wptr(service);
    wmSDeathRecipient->OnRemoteDied(wptrDeath);
}

/**
 * @tc.name: GetAllMainWindowInfos
 * @tc.desc: WindowAdapterLite/GetAllMainWindowInfos
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, GetAllMainWindowInfos, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->OnUserSwitch();

    MainWindowInfo info;
    std::vector<MainWindowInfo> infos;
    infos.push_back(info);
    ASSERT_EQ(WMError::WM_OK, instance_->GetAllMainWindowInfos(infos));
}

/**
 * @tc.name: ClearMainSessions
 * @tc.desc: WindowAdapterLite/ClearMainSessions
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ClearMainSessions, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->OnUserSwitch();

    const std::vector<int32_t> persistentIds;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, instance_->ClearMainSessions(persistentIds));
}

/**
 * @tc.name: ClearMainSessions01
 * @tc.desc: WindowAdapterLite/ClearMainSessions
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ClearMainSessions01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->OnUserSwitch();

    std::vector<int32_t> persistentIds;
    std::vector<int32_t> clearFailedIds;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION,
        instance_->ClearMainSessions(persistentIds, clearFailedIds));
}

/**
 * @tc.name: RaiseWindowToTop
 * @tc.desc: WindowAdapterLite/RaiseWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RaiseWindowToTop, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->OnUserSwitch();

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, instance_->RaiseWindowToTop(0));
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: WindowAdapterLite/GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, GetWindowStyleType, TestSize.Level1)
{
    WindowStyleType windowStyleType = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, instance_->GetWindowStyleType(windowStyleType));
}

/**
 * @tc.name: TerminateSessionByPersistentId
 * @tc.desc: WindowAdapterLite/TerminateSessionByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, TerminateSessionByPersistentId, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(instance_, nullptr);
    instance_->OnUserSwitch();

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, instance_->TerminateSessionByPersistentId(0));
}

/**
 * @tc.name: CloseTargetFloatWindow
 * @tc.desc: WindowAdapterLite/CloseTargetFloatWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, CloseTargetFloatWindow, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(instance_, nullptr);
    instance_->OnUserSwitch();

    const std::string& bundleName = "test";
    ASSERT_EQ(WMError::WM_OK, instance_->CloseTargetFloatWindow(bundleName));
}

/**
 * @tc.name: CloseTargetPiPWindow
 * @tc.desc: WindowAdapterLite/CloseTargetPiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, CloseTargetPiPWindow, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    instance_->OnUserSwitch();

    const std::string& bundleName = "test";
    ASSERT_EQ(WMError::WM_OK, instance_->CloseTargetPiPWindow(bundleName));
}

/**
 * @tc.name: GetCurrentPiPWindowInfo
 * @tc.desc: WindowAdapterLite/GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, GetCurrentPiPWindowInfo, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    instance_->OnUserSwitch();

    std::string bundleName = "test";
    ASSERT_EQ(WMError::WM_OK, instance_->GetCurrentPiPWindowInfo(bundleName));
}

/**
 * @tc.name: ListWindowInfo01
 * @tc.desc: WindowAdapter/ListWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ListWindowInfo01, Function | SmallTest | Level2)
{
    WindowInfoOption windowInfoOption;
    std::vector<sptr<WindowInfo>> infos;
    auto err = instance_->ListWindowInfo(windowInfoOption, infos);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, err);
}

/**
 * @tc.name: RecoverWindowPropertyChangeFlag
 * @tc.desc: test RecoverWindowPropertyChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RecoverWindowPropertyChangeFlag, TestSize.Level1)
{
    WMError ret;

    // branch 1: windowManagerServiceProxy_ is null
    instance_->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    // branch 2: Mock wms proxy return failed
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RecoverWindowPropertyChangeFlag(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));

    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    ret = instance_->RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(ret, WMError::WM_DO_NOTHING);

    // branch 3: Mock wms proxy return ok
    EXPECT_CALL(*remoteObject, RecoverWindowPropertyChangeFlag(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ret = instance_->RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SendPointerEventForHover
 * @tc.desc: SendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, SendPointerEventForHover, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, instance_);
    instance_->SendPointerEventForHover(pointerEvent);
}

/**
 * @tc.name: GetInstance
 * @tc.desc: sptr<WindowAdapterLite>
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, GetInstanceMulti, Function | SmallTest | Level2)
{
    int32_t userId = -1;
    sptr<WindowAdapterLite> instance = nullptr;
    instance = &WindowAdapterLite::GetInstance(userId);
    ASSERT_NE(instance, nullptr);

    userId = 101;
    instance = &WindowAdapterLite::GetInstance(userId);
    ASSERT_NE(instance, nullptr);

    // branch 2
    instance = &WindowAdapterLite::GetInstance(userId);
    ASSERT_NE(instance, nullptr);
}

/**
 * @tc.name: InitSSMProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, InitSSMProxy, TestSize.Level1)
{
    // branch 1
    instance_->isProxyValid_ = true;
    ASSERT_EQ(true, instance_->InitSSMProxy());

    // branch 2
    instance_->isProxyValid_ = false;
    instance_->isRegisteredUserSwitchListener_ = true;
    instance_->InitSSMProxy();

    // branch 3
    ASSERT_NE(nullptr, instance_);
    instance_->isRegisteredUserSwitchListener_ = false;
    instance_->InitSSMProxy();
}

/**
 * @tc.name: UnregisterWMSConnectionChangedListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWMSConnectionChangedListener, TestSize.Level1)
{
    instance_->UnregisterWMSConnectionChangedListener();
    ASSERT_NE(nullptr, instance_);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto callbackFunc = [](int32_t, int32_t, bool, int32_t) { return; };
    instance_->RegisterWMSConnectionChangedListener(callbackFunc);
}

/**
 * @tc.name: WindowAdapterLite::IsWindowManagerServiceProxyValid
 * @tc.desc: 测试检查 WMS 代理是否有效
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, IsWindowManagerServiceProxyValid, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    // Test with isProxyValid_ = false
    instance_->isProxyValid_ = false;
    EXPECT_FALSE(instance_->IsWindowManagerServiceProxyValid());

    // Test with isProxyValid_ = true
    instance_->isProxyValid_ = true;
    EXPECT_TRUE(instance_->IsWindowManagerServiceProxyValid());
}

/**
 * @tc.name: WindowAdapterLite::IsMockSMSProxyAlive
 * @tc.desc: 测试检查 Mock SMS 是否存活
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, IsMockSMSProxyAlive, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    // This test depends on SessionManager state
    // We just verify the method can be called without crashing
    auto isAlive = instance_->IsMockSMSProxyAlive();
    // The result depends on whether SessionManager has a mock proxy
    // We don.t assert specific value, just ensure it doesn't crash
}

/**
 * @tc.name: WindowAdapterLite::RegisterUserSwitchCallback_AlreadyRegistered
 * @tc.desc: 测试已注册时直接返回
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RegisterUserSwitchCallback_AlreadyRegistered, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    // Set isRegisteredUserSwitchListener_ to true
    instance_->isRegisteredUserSwitchListener_ = true;

    // Call RegisterUserSwitchCallback should return early
    instance_->RegisterUserSwitchCallback();

    // isRegisteredUserSwitchListener_ should still be true
    EXPECT_TRUE(instance_->isRegisteredUserSwitchListener_);

    // Restore
    instance_->isRegisteredUserSwitchListener_ = false;
}

/**
 * @tc.name: WindowAdapterLite::RegisterUserSwitchCallback_MockSMSNull
 * @tc.desc: 测试 Mock SMS 为空时直接返回
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RegisterUserSwitchCallback_MockSMSNull, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    // Ensure isRegisteredUserSwitchListener_ is false
    instance_->isRegisteredUserSwitchListener_ = false;

    // Call RegisterUserSwitchCallback
    // If SessionManager doesn.t have mock proxy, it should return early
    instance_->RegisterUserSwitchCallback();

    // The result depends on SessionManager state
    // We just verify it doesn.t crash
}

/**
 * @tc.name: WindowAdapterLite::RegisterUserSwitchCallback_NonSystemUser
 * @tc.desc: 测试非 SYSTEM_USER 时直接返回
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RegisterUserSwitchCallback_NonSystemUser, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    // Ensure isRegisteredUserSwitchListener_ is false
    instance_->isRegisteredUserSwitchListener_ = false;

    // Call RegisterUserSwitchCallback
    // If current user is not SYSTEM_USERID, it should return early
    instance_->RegisterUserSwitchCallback();

    // The result depends on GetUserIdByUid(getuid()) value
    // We just verify it doesn.t crash
}

/**
 * @tc.name: WindowAdapterLite::RegisterUserSwitchCallback_Success
 * @tc.desc: 测试的后常注册回调
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, RegisterUserSwitchCallback_Success, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    // Ensure isRegisteredUserSwitchListener_ is false
    instance_->isRegisteredUserSwitchListener_ = false;

    // Call RegisterUserSwitchCallback
    // If SessionManager has mock proxy and user is SYSTEM_USERID, it should register callback
    instance_->RegisterUserSwitchCallback();

    // The result depends on SessionManager state and user ID
    // We just verify it doesn.t crash
}

/**
 * @tc.name: WindowAdapterLite::ReregisterWindowManagerFaultAgent_ProxyNull
 * @tc.desc: 测试 proxy 为空的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ReregisterWindowManagerFaultAgent_ProxyNull, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    LOG_SetCallback(MyLogCallback);
    g_logMsg.clear();

    sptr<IWindowManagerLite> wmsProxy = nullptr;
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);

    EXPECT_TRUE(g_logMsg.find("WMS proxy is null") != std::string::npos);
}

/**
 * @tc.name: WindowAdapterLite::ReregisterWindowManagerFaultAgent_RegisterFailed
 * @tc.desc: 测试注册失败的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ReregisterWindowManagerFaultAgent_RegisterFailed, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    LOG_SetCallback(MyLogCallback);
    g_logMsg.clear();

    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _))
        .Times(1)
        .WillOnce(Return(WMError::WM_ERROR_SAMGR));

    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    auto agent = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    instance_->windowManagerLiteFaultAgentMap_[type].insert(agent);

    g_logMsg.clear();
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);

    EXPECT_TRUE(g_logMsg.find("Re-register fault agent failed") != std::string::npos);
    EXPECT_FALSE(instance_->windowManagerLiteFaultAgentMap_[type].empty());
}

/**
 * @tc.name: WindowAdapterLite::ReregisterWindowManagerFaultAgent_RegisterSuccess
 * @tc.desc: 测试注册成功的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ReregisterWindowManagerFaultAgent_RegisterSuccess, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    LOG_SetCallback(MyLogCallback);
    g_logMsg.clear();

    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _))
        .WillRepeatedly(Return(WMError::WM_OK));

    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    auto agent = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    instance_->windowManagerLiteFaultAgentMap_[type].insert(agent);

    g_logMsg.clear();
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);
}

/**
 * @tc.name: WindowAdapterLite::ReregisterWindowManagerFaultAgent_MultipleAgents
 * @tc.desc: 测试多个 agent 混合成功/失败的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ReregisterWindowManagerFaultAgent_MultipleAgents, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    LOG_SetCallback(MyLogCallback);
    g_logMsg.clear();

    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_ERROR_SAMGR))
        .WillOnce(Return(WMError::WM_OK));

    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    WindowManagerAgentType type1 = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    WindowManagerAgentType type2 = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PIP;
    auto agent1 = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    auto agent2 = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    instance_->windowManagerLiteFaultAgentMap_[type1].insert(agent1);
    instance_->windowManagerLiteFaultAgentMap_[type2].insert(agent2);

    g_logMsg.clear();
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);
}

/**
 * @tc.name: WindowAdapterLite::ReregisterWindowManagerFaultAgent_EmptyFaultMap
 * @tc.desc: 测试空的 fault map
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, ReregisterWindowManagerFaultAgent_EmptyFaultMap, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);

    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _))
        .Times(0);

    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerLiteFaultAgentMap_.clear();

    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);

    EXPECT_TRUE(instance_->windowManagerLiteFaultAgentMap_.empty());
}
/**
 * @tc.name: UnregisterWindowManagerAgent01
 * @tc.desc: 测试 wmsProxy 为空的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent01, TestSize.Level1)
{
    instance_->isProxyValid_ = true;
    instance_->windowManagerServiceProxy_ = nullptr;
    
    sptr<IWindowManagerAgent> agent = nullptr;
    auto ret = instance_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, agent);
    
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: UnregisterWindowManagerAgent02
 * @tc.desc: 测试 windowManagerLiteAgentMap_ 中找不到 type 的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent02, TestSize.Level1)
{
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _))
        .Times(1)
        .WillOnce(Return(WMError::WM_OK));
    
    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->isProxyValid_ = true;
    
    instance_->windowManagerLiteAgentMap_.clear();
    
    sptr<IWindowManagerAgent> agent = nullptr;
    auto ret = instance_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, agent);
    
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowManagerAgent03
 * @tc.desc: 测试 agentSet 中找不到 agent 的情况
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent03, TestSize.Level1)
{
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _))
        .Times(1)
        .WillOnce(Return(WMError::WM_OK));
    
    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->isProxyValid_ = true;
    
    sptr<IWindowManagerAgent> existingAgent = nullptr;
    std::set<sptr<IWindowManagerAgent>> agentSet = { existingAgent };
    instance_->windowManagerLiteAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = agentSet;
    
    sptr<IWindowManagerAgent> newAgent = nullptr;
    auto ret = instance_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, newAgent);
    
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowManagerAgent04
 * @tc.desc: 测试正常注销成功，windowManagerLiteFaultAgentMap_ 为空
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent04, TestSize.Level1)
{
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _))
        .Times(1)
        .WillOnce(Return(WMError::WM_OK));
    
    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->isProxyValid_ = true;
    
    sptr<IWindowManagerAgent> agent = nullptr;
    std::set<sptr<IWindowManagerAgent>> agentSet = { agent };
    instance_->windowManagerLiteAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = agentSet;
    
    instance_->windowManagerLiteFaultAgentMap_.clear();
    
    auto ret = instance_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, agent);
}

/**
 * @tc.name: UnregisterWindowManagerAgent05
 * @tc.desc: 测试正常注销成功，windowManagerLiteFaultAgentMap_ 中找不到 type
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent05, TestSize.Level1)
{
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));

    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->isProxyValid_ = true;

    sptr<IWindowManagerAgent> agent = nullptr;
    std::set<sptr<IWindowManagerAgent>> agentSet = { agent };
    instance_->windowManagerLiteAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = agentSet;

    sptr<IWindowManagerAgent> faultAgent = nullptr;
    std::set<sptr<IWindowManagerAgent>> faultAgentSet = { faultAgent };
    instance_->windowManagerLiteFaultAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT] =
        faultAgentSet;

    auto ret = instance_->UnregisterWindowManagerAgent(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, agent);
}

/**
 * @tc.name: UnregisterWindowManagerAgent06
 * @tc.desc: 测试正常注销成功，fault map 中找到 type 和 agent，删除后 set 不为空
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent06, TestSize.Level1)
{
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _))
        .Times(1)
        .WillOnce(Return(WMError::WM_OK));
    
    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->isProxyValid_ = true;
    
    sptr<IWindowManagerAgent> agent = nullptr;
    std::set<sptr<IWindowManagerAgent>> agentSet = { agent };
    instance_->windowManagerLiteAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = agentSet;
    
    sptr<IWindowManagerAgent> faultAgent1 = agent;
    sptr<IWindowManagerAgent> faultAgent2 = nullptr;
    std::set<sptr<IWindowManagerAgent>> faultAgentSet = { faultAgent1, faultAgent2 };
    instance_->windowManagerLiteFaultAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = faultAgentSet;
    
    auto ret = instance_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, agent);
}

/**
 * @tc.name: UnregisterWindowManagerAgent07
 * @tc.desc: 测试正常注销成功，fault map 中找到 type 和 agent，删除后 set 为空，删除整个 entry
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterLiteTest, UnregisterWindowManagerAgent07, TestSize.Level1)
{
    auto remoteObject = sptr<WindowManagerLiteServiceMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _))
        .Times(1)
        .WillOnce(Return(WMError::WM_OK));
    
    auto wmsProxy = iface_cast<IWindowManagerLite>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->isProxyValid_ = true;
    
    sptr<IWindowManagerAgent> agent = nullptr;
    std::set<sptr<IWindowManagerAgent>> agentSet = { agent };
    instance_->windowManagerLiteAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = agentSet;
    
    std::set<sptr<IWindowManagerAgent>> faultAgentSet = { agent };
    instance_->windowManagerLiteFaultAgentMap_[WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS] = faultAgentSet;
    
    auto ret = instance_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, agent);
}

} // namespace
} // namespace Rosen
} // namespace OHOS
