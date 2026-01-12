/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
    instance_->ClearWindowAdapter();

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
    LOG_SetCallback(nullptr);
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
    LOG_SetCallback(nullptr);
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
    LOG_SetCallback(nullptr);
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
    LOG_SetCallback(nullptr);
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
HWTEST_F(WindowAdapterLiteTest, RecoverWindowPropertyChangeFlag01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    auto ret = instance_->RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(ret, WMError::WM_OK);

    auto oldProxy = instance_->windowManagerServiceProxy_;
    instance_->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    instance_->windowManagerServiceProxy_ = oldProxy;
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
    auto callbackFunc = [](int32_t, int32_t, bool) { return; };
    instance_->RegisterWMSConnectionChangedListener(callbackFunc);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
