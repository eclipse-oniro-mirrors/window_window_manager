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
#include <ipc_types.h>

#include "iremote_object_mocker.h"
#include "session_manager/include/zidl/scene_session_manager_lite_stub.h"
#include "session_manager/include/zidl/session_router_stack_listener_stub.h"
#include "session_manager/include/zidl/pip_change_listener_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MockSceneSessionManagerLiteStub : public SceneSessionManagerLiteStub {
    WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) override
    {
        return WSError::WS_OK;
    }
    WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override
    {
        return WSError::WS_OK;
    }
    WSError SetSessionIconForThirdParty(
        const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override
    {
        return WSError::WS_OK;
    }
    WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) override
    {
        return WSError::WS_OK;
    }
    WMError SetGlobalDragResizeType(DragResizeType dragResizeType) override
    {
        return WMError::WM_OK;
    }
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) override
    {
        return WMError::WM_OK;
    }
    WMError SetAppDragResizeType(const std::string& bundleName,
        DragResizeType dragResizeType) override
    {
        return WMError::WM_OK;
    }
    WMError GetAppDragResizeType(const std::string& bundleName,
        DragResizeType& dragResizeType) override
    {
        return WMError::WM_OK;
    }
    WMError SetAppKeyFramePolicy(const std::string& bundleName,
        const KeyFramePolicy& keyFramePolicy) override
    {
        return WMError::WM_OK;
    }
    WSError PendingSessionToForeground(const sptr<IRemoteObject>& token, int32_t windowMode) override
    {
        return WSError::WS_OK;
    }
    WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token, bool shouldBackToCaller) override
    {
        return WSError::WS_OK;
    }
    WSError GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId) override
    {
        return WSError::WS_OK;
    }
    WSError GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId) override
    {
        return WSError::WS_OK;
    }
    WSError IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent) override
    {
        return WSError::WS_OK;
    }
    WSError RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover = false) override
    {
        return WSError::WS_OK;
    }
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override
    {
        return WSError::WS_OK;
    }
    WSError GetSessionInfos(const std::string& deviceId,
                            int32_t numMax,
                            std::vector<SessionInfoBean>& sessionInfos) override
    {
        return WSError::WS_OK;
    }
    WSError GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates) override
    {
        return WSError::WS_OK;
    }
    WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) override
    {
        return WSError::WS_OK;
    }
    WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
                                              SessionInfoBean& sessionInfo) override
    {
        return WSError::WS_OK;
    }
    WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) override
    {
        return WSError::WS_OK;
    }
    WSError TerminateSessionNew(const sptr<AAFwk::SessionInfo> info,
                                bool needStartCaller,
                                bool isFromBroker = false) override
    {
        return WSError::WS_OK;
    }
    WSError GetSessionSnapshot(const std::string& deviceId,
                               int32_t persistentId,
                               SessionSnapshot& snapshot,
                               bool isLowResolution) override
    {
        return WSError::WS_OK;
    }
    WSError ClearSession(int32_t persistentId) override
    {
        return WSError::WS_OK;
    }
    WSError ClearAllSessions() override
    {
        return WSError::WS_OK;
    }
    WSError LockSession(int32_t sessionId) override
    {
        return WSError::WS_OK;
    }
    WSError UnlockSession(int32_t sessionId) override
    {
        return WSError::WS_OK;
    }
    WSError MoveSessionsToForeground(const std::vector<std::int32_t>& sessionIds, int32_t topSessionId) override
    {
        return WSError::WS_OK;
    }
    WSError MoveSessionsToBackground(const std::vector<std::int32_t>& sessionIds,
                                     std::vector<std::int32_t>& result) override
    {
        return WSError::WS_OK;
    }
    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
                                       const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
                                         const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos) override
    {
        return WMError::WM_OK;
    }
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId) override {}
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) override {}
    WMError CheckWindowId(int32_t windowId, int32_t& pid) override
    {
        return WMError::WM_OK;
    }
    WMError UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode) override
    {
        return WMError::WM_OK;
    }
    WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo) override
    {
        MainWindowInfo mainWindowInfo;
        topNInfo.push_back(mainWindowInfo);
        return WMError::WM_OK;
    }
    WMError GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo) override
    {
        return WMError::WM_OK;
    }
    WSError RaiseWindowToTop(int32_t persistentId) override
    {
        return WSError::WS_OK;
    }
    WSError RegisterIAbilityManagerCollaborator(int32_t type,
                                                const sptr<AAFwk::IAbilityManagerCollaborator>& impl) override
    {
        return WSError::WS_OK;
    }
    WSError UnregisterIAbilityManagerCollaborator(int32_t type) override
    {
        return WSError::WS_OK;
    }
    WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) override
    {
        MainWindowInfo mainWindowInfo;
        infos.push_back(mainWindowInfo);
        return WMError::WM_OK;
    }
    WMError GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo) override
    {
        return WMError::WM_OK;
    }
    WMError ClearMainSessions(const std::vector<int32_t>& persistentIds, std::vector<int32_t>& clearFailedIds) override
    {
        clearFailedIds.push_back(1);
        return WMError::WM_OK;
    }
    WMError GetWindowStyleType(WindowStyleType& windowStyleType) override
    {
        return WMError::WM_OK;
    }
    WMError TerminateSessionByPersistentId(int32_t persistentId) override
    {
        return WMError::WM_OK;
    }
    WMError CloseTargetFloatWindow(const std::string& bundleName) override
    {
        return WMError::WM_OK;
    }
    WMError CloseTargetPiPWindow(const std::string& bundleName) override
    {
        return WMError::WM_OK;
    }
    WMError GetCurrentPiPWindowInfo(std::string& bundleName) override
    {
        bundleName = "test";
        return WMError::WM_OK;
    }
    WMError GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId) override
    {
        return WMError::WM_OK;
    }
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override
    {
        return WMError::WM_OK;
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
    WMError CheckUIExtensionCreation(int32_t windowId,
                                     uint32_t tokenId,
                                     const AppExecFwk::ElementName& element,
                                     AppExecFwk::ExtensionAbilityType extensionAbilityType,
                                     int32_t& pid) override
    {
        return WMError::WM_OK;
    }
    WSError NotifyAppUseControlList(ControlAppType type,
                                    int32_t userId,
                                    const std::vector<AppUseControlInfo>& controlList) override
    {
        return WSError::WS_OK;
    }
    WMError MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId) override
    {
        return WMError::WM_OK;
    }
    WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken, bool& hasFloatingShowing) override
    {
        return WMError::WM_OK;
    }
    WMError LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock) override
    {
        return WMError::WM_OK;
    }
    WMError RegisterSessionLifecycleListenerByIds(const sptr<ISessionLifecycleListener>& listener,
                                                  const std::vector<int32_t>& persistentIdList) override
    {
        return WMError::WM_OK;
    }
    WMError RegisterSessionLifecycleListenerByBundles(const sptr<ISessionLifecycleListener>& listener,
                                                      const std::vector<std::string>& bundleNameList) override
    {
        return WMError::WM_OK;
    }
    WMError UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener) override
    {
        return WMError::WM_OK;
    }
    WSError GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList) override
    {
        return WSError::WS_OK;
    }
    WMError GetRouterStackInfo(int32_t persistentId, const sptr<ISessionRouterStackListener>& listener)
        override { return WMError::WM_OK; }
    WSError PendingSessionToBackgroundByPersistentId(const int32_t persistentId,
        bool shouldBackToCaller) override
    {
        return WSError::WS_OK;
    }
    WMError CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey) override
    {
        return WMError::WM_OK;
    }

    WMError RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey) override
    {
        return WMError::WM_OK;
    }
    WMError TransferSessionToTargetScreen(const TransferSessionInfo& info) override
    {
        return WMError::WM_OK;
    }
    WSError PendingSessionToBackground(const sptr<IRemoteObject>& token, const BackgroundParams& params) override
    {
        return WSError::WS_OK;
    }
    WMError UpdateKioskAppList(const std::vector<std::string>& kioskAppList) override
    {
        return WMError::WM_OK;
    }
    WMError EnterKioskMode(const sptr<IRemoteObject>& token) override
    {
        return WMError::WM_OK;
    }
    WMError ExitKioskMode(const sptr<IRemoteObject>& token) override
    {
        return WMError::WM_OK;
    }
    WSError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override
    {
        return WSError::WS_OK;
    }

    WMError SetPipEnableByScreenId(int32_t screenId, bool enabled) override { return WMError::WM_OK; }
    WMError UnsetPipEnableByScreenId(int32_t screenId) override { return WMError::WM_OK; }
    WMError RegisterPipChgListenerByScreenId(int32_t screenId, const sptr<IPipChangeListener>& listener)
    {
        return WMError::WM_OK;
    }
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override { return WMError::WM_OK; }
    WMError UnregisterPipChgListenerByScreenId(int32_t screenId) override { return WMError::WM_OK; }
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override { return WMError::WM_OK; }
};

class SceneSessionManagerLiteStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSessionManagerLiteStub> sceneSessionManagerLiteStub_ = nullptr;
};

void SceneSessionManagerLiteStubTest::SetUpTestCase() {}

void SceneSessionManagerLiteStubTest::TearDownTestCase() {}

void SceneSessionManagerLiteStubTest::SetUp()
{
    sceneSessionManagerLiteStub_ = sptr<MockSceneSessionManagerLiteStub>::MakeSptr();
    EXPECT_NE(nullptr, sceneSessionManagerLiteStub_);
}

void SceneSessionManagerLiteStubTest::TearDown()
{
    sceneSessionManagerLiteStub_ = nullptr;
}

namespace {
/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: test function : OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, OnRemoteRequest, TestSize.Level1)
{
    uint32_t code =
        static_cast<uint32_t>(SceneSessionManagerLiteStub::SceneSessionManagerLiteMessage::TRANS_ID_SET_SESSION_LABEL);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"OpenHarmeny");
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);
    data.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor());
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::OnRemoteRequest(1000, data, reply, option);
    EXPECT_EQ(IPC_STUB_UNKNOW_TRANS_ERR, res);
    data.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor());
    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    data.WriteString("OnRemoteRequest UT Testing.");
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetSessionIcon
 * @tc.desc: test function : HandleSetSessionIcon
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetSessionIcon, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetSessionIcon(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleIsValidSessionIds
 * @tc.desc: test function : HandleIsValidSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleIsValidSessionIds, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleIsValidSessionIds(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetGlobalDragResizeType
 * @tc.desc: test HandleSetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetGlobalDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetGlobalDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    data.WriteUint32(static_cast<uint32_t>(dragResizeType));
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetGlobalDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetGlobalDragResizeType
 * @tc.desc: test HandleGetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetGlobalDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetGlobalDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetAppDragResizeType
 * @tc.desc: test HandleSetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetAppDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetAppDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    data.WriteString(bundleName);
    data.WriteUint32(static_cast<uint32_t>(dragResizeType));
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetAppDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetAppDragResizeType
 * @tc.desc: test HandleGetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetAppDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::string bundleName = "test";
    data.WriteString(bundleName);
    int res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetAppDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetAppKeyFramePolicy
 * @tc.desc: test HandleSetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetAppKeyFramePolicy, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetAppKeyFramePolicy(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    data.WriteString(bundleName);
    data.WriteParcelable(&keyFramePolicy);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetAppKeyFramePolicy(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandlePendingSessionToForeground_InvalidToken
 * @tc.desc: test function : HandlePendingSessionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToForeground_InvalidToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToForeground(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandlePendingSessionToForeground_Fail
 * @tc.desc: test function : HandlePendingSessionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToForeground_Fail, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    data.WriteRemoteObject(token);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToForeground(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandlePendingSessionToForeground_Success
 * @tc.desc: test function : HandlePendingSessionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToForeground_Success, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    int32_t windowMode = static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN);
    data.WriteRemoteObject(token);
    data.WriteInt32(windowMode);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToForeground(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandlePendingSessionToBackgroundForDelegator
 * @tc.desc: test function : HandlePendingSessionToBackgroundForDelegator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToBackgroundForDelegator, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandlePendingSessionToBackgroundForDelegator(
        data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleRegisterSessionListener
 * @tc.desc: test function : HandleRegisterSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegisterSessionListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterSessionListener(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUnRegisterSessionListener
 * @tc.desc: test function : HandleUnRegisterSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnRegisterSessionListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleUnRegisterSessionListener(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetSessionInfos
 * @tc.desc: test function : HandleGetSessionInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetSessionInfos, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleUnRegisterSessionListener(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetMainWindowStatesByPid
 * @tc.desc: test function : HandleGetMainWindowStatesByPid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetMainWindowStatesByPid, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetMainWindowStatesByPid(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetSessionInfo
 * @tc.desc: test function : HandleGetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetSessionInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetSessionInfo(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    std::u16string deviceIdU16 = u"testDeviceId";
    data.WriteString16(deviceIdU16);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetSessionInfo(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    int32_t persistentId = 0;
    data.WriteString16(deviceIdU16);
    data.WriteInt32(persistentId);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetSessionInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetSessionInfoByContinueSessionId
 * @tc.desc: test function : HandleGetSessionInfoByContinueSessionId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetSessionInfoByContinueSessionId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res =
        sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetSessionInfoByContinueSessionId(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    std::string continueSessionId = "testSessionId";
    data.WriteString(continueSessionId);
    res =
        sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetSessionInfoByContinueSessionId(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleTerminateSessionNew
 * @tc.desc: test function : HandleTerminateSessionNew
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleTerminateSessionNew, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleTerminateSessionNew(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleGetFocusSessionToken
 * @tc.desc: test function : HandleGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetFocusSessionToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(0);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetFocusSessionToken(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetFocusSessionToken1
 * @tc.desc: test function : HandleGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetFocusSessionToken1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetFocusSessionToken(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleGetFocusSessionElement
 * @tc.desc: test function : HandleGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetFocusSessionElement, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(0);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetFocusSessionElement(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetFocusSessionElement1
 * @tc.desc: test function : HandleGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetFocusSessionElement1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetFocusSessionElement(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleIsFocusWindowParent
 * @tc.desc: test function : HandleIsFocusWindowParent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleIsFocusWindowParent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleIsFocusWindowParent(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
    const sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(token);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetSessionContinueState
 * @tc.desc: test function : HandleSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetSessionContinueState, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    int32_t continueStateValue = -3;
    data.WriteRemoteObject(token);
    data.WriteInt32(continueStateValue);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    continueStateValue = 1;
    data.WriteRemoteObject(token);
    data.WriteInt32(continueStateValue);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetSessionContinueState1
 * @tc.desc: test function : HandleSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetSessionContinueState1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteRemoteObject(nullptr);
    data.WriteInt32(-2);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetSessionContinueState2
 * @tc.desc: test function : HandleSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetSessionContinueState2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    data.WriteInt32(static_cast<int32_t>(ContinueState::CONTINUESTATE_MAX));
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(res, ERR_NONE);
    uint32_t writtenError;
    EXPECT_TRUE(reply.ReadUint32(writtenError));
    EXPECT_EQ(writtenError, static_cast<uint32_t>(ERR_NONE));
}

/**
 * @tc.name: HandleGetSessionSnapshot
 * @tc.desc: test function : HandleGetSessionSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetSessionSnapshot, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::u16string deviceId = static_cast<std::u16string>(u"123");
    data.WriteString16(deviceId);
    const std::int32_t persistentId = 1;
    data.WriteInt32(persistentId);
    const bool isLowResolution = true;
    data.WriteBool(isLowResolution);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetSessionSnapshot(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleClearSession
 * @tc.desc: test function : HandleClearSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleClearSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleClearSession(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    int32_t persistentId = 0;
    data.WriteInt32(persistentId);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleClearSession(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleClearAllSessions
 * @tc.desc: test function : HandleClearAllSessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleClearAllSessions, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleClearAllSessions(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetParentMainWindowId
 * @tc.desc: test function : HandleGetParentMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetParentMainWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t windowId = 0;
    data.WriteInt32(windowId);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetParentMainWindowId(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleLockSession
 * @tc.desc: test function : HandleLockSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleLockSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleLockSession(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    int32_t persistentId = 0;
    data.WriteInt32(persistentId);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleLockSession(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUnlockSession
 * @tc.desc: test function : HandleUnlockSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnlockSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleUnlockSession(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    int32_t sessionId = 0;
    data.WriteInt32(sessionId);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleUnlockSession(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleMoveSessionsToForeground
 * @tc.desc: test function : HandleMoveSessionsToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleMoveSessionsToForeground, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleMoveSessionsToForeground(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    std::vector<int32_t> sessionIds;
    sessionIds.push_back(0);
    data.WriteInt32Vector(sessionIds);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleMoveSessionsToForeground(data, reply);
    EXPECT_EQ(ERR_TRANSACTION_FAILED, res);

    int32_t topSessionId = 0;
    data.WriteInt32Vector(sessionIds);
    data.WriteInt32(topSessionId);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleMoveSessionsToForeground(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleMoveSessionsToBackground
 * @tc.desc: test function : HandleMoveSessionsToBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleMoveSessionsToBackground, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleMoveSessionsToBackground(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetFocusSessionInfo
 * @tc.desc: test function : HandleGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetFocusSessionInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(0);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetFocusSessionInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetFocusSessionInfo1
 * @tc.desc: test function : HandleGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetFocusSessionInfo1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetFocusSessionInfo(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleCheckWindowId
 * @tc.desc: test function : HandleCheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleCheckWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t numMax = 100;
    data.WriteInt32(numMax);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleCheckWindowId(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUpdateWindowModeByIdForUITest01
 * @tc.desc: test function : HandleUpdateWindowModeByIdForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUpdateWindowModeByIdForUITest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateWindowModeByIdForUITest(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    data.WriteInt32(1);
    res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateWindowModeByIdForUITest(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    MessageParcel data2;
    data2.WriteInt32(1);
    data2.WriteInt32(2);
    res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateWindowModeByIdForUITest(data2, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleRegisterWindowManagerAgent
 * @tc.desc: test function : HandleRegisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegisterWindowManagerAgent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS));
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_NONE, res);

    data.WriteUint32(-100);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    data.WriteUint32(100);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    data.WriteUint32(5);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUnregisterWindowManagerAgent
 * @tc.desc: test function : HandleUnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnregisterWindowManagerAgent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS));
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_NONE, res);

    data.WriteUint32(-100);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    data.WriteUint32(100);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    data.WriteUint32(5);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUpdateAnimationSpeedWithPid
 * @tc.desc: test HandleUpdateAnimationSpeedWithPid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUpdateAnimationSpeedWithPid, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateAnimationSpeedWithPid(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(10000);
    res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateAnimationSpeedWithPid(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(10000);
    data.WriteFloat(2.0f);
    res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateAnimationSpeedWithPid(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleListWindowInfo
 * @tc.desc: test function : HandleListWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleListWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    DisplayId displayId = 0;
    int32_t windowId = 0;
    data.WriteUint8(static_cast<WindowInfoFilterOptionDataType>(WindowInfoFilterOption::ALL));
    data.WriteUint8(static_cast<WindowInfoTypeOptionDataType>(WindowInfoTypeOption::ALL));
    data.WriteUint64(displayId);
    data.WriteInt32(windowId);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleListWindowInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleRegisterWindowPropertyChangeAgent
 * @tc.desc: test function : HandleRegisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegisterWindowPropertyChangeAgent, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ASSERT_NE(sceneSessionManagerLiteStub_, nullptr);

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT);
    auto res = sceneSessionManagerLiteStub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    uint32_t interestInfo = 0;
    data.WriteInt32(static_cast<int32_t>(windowInfoKey));
    data.WriteUint32(interestInfo);

    res = sceneSessionManagerLiteStub_->HandleRegisterWindowPropertyChangeAgent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetVisibilityWindowInfo
 * @tc.desc: test function : HandleGetVisibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetVisibilityWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleGetVisibilityWindowInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetMainWinodowInfo
 * @tc.desc: test function : HandleGetMainWinodowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetMainWinodowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t numMax = 100;
    data.WriteInt32(numMax);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleGetMainWinodowInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleRaiseWindowToTop
 * @tc.desc: test function : HandleRaiseWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleRaiseWindowToTop, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleRaiseWindowToTop(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetAllMainWindowInfos
 * @tc.desc: test function : HandleGetAllMainWindowInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetAllMainWindowInfos, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetAllMainWindowInfos(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetMainWindowInfoByToken
 * @tc.desc: test function : HandleGetMainWindowInfoByToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetMainWindowInfoByToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(token);
    auto res = sceneSessionManagerLiteStub_->HandleGetMainWindowInfoByToken(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleClearMainSessions
 * @tc.desc: test function : HandleClearMainSessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleClearMainSessions, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::vector<int32_t> persistentIds = { 1, 2, 3 };
    data.WriteInt32Vector(persistentIds);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleClearMainSessions(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleTerminateSessionByPersistentId
 * @tc.desc: test function : HandleTerminateSessionByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleTerminateSessionByPersistentId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res =
        sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleTerminateSessionByPersistentId(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    int32_t persistentId = 1;
    data.WriteInt32(persistentId);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleTerminateSessionByPersistentId(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetWindowStyleType
 * @tc.desc: test function : HandleGetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetWindowStyleType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetWindowStyleType(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleCloseTargetFloatWindow
 * @tc.desc: test function : HandleCloseTargetFloatWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleCloseTargetFloatWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("test");
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleCloseTargetFloatWindow(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleCloseTargetPiPWindow
 * @tc.desc: test function : HandleCloseTargetPiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleCloseTargetPiPWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleCloseTargetPiPWindow(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetCurrentPiPWindowInfo
 * @tc.desc: test function : HandleGetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetCurrentPiPWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetCurrentPiPWindowInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetRouterStackInfo
 * @tc.desc: test function : HandleGetRouterStackInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetRouterStackInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetRouterStackInfo(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    data.WriteInt32(1);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetRouterStackInfo(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleGetRootMainWindowId
 * @tc.desc: test function : HandleGetRootMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetRootMainWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t persistentId = 1;
    data.WriteInt32(persistentId);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleGetRootMainWindowId(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleNotifyAppUseControlList
 * @tc.desc: test function : HandleNotifyAppUseControlList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleNotifyAppUseControlList, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint8_t typeId = 1;
    int32_t userId = 1;
    int32_t size = 1;
    std::string bundleName = "appbundleName";
    int32_t appIndex = 1;
    bool isControl = true;

    data.WriteUint8(typeId);
    data.WriteInt32(userId);
    data.WriteInt32(size);
    data.WriteString(bundleName);
    data.WriteInt32(appIndex);
    data.WriteBool(isControl);

    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleNotifyAppUseControlList(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleNotifyAppUseControlList02
 * @tc.desc: test function : HandleNotifyAppUseControlList02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleNotifyAppUseControlList02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint8_t typeId = 1;
    int32_t userId = 1;
    int32_t size = 1;
    std::string bundleName = "appbundleName";
    int32_t appIndex = 1;
    bool isControl = true;
    bool isControlRecentOnly = true;
    data.WriteUint8(typeId);
    data.WriteInt32(userId);
    data.WriteInt32(size);
    AppUseControlInfo controlInfo;
    data.WriteParcelable(&controlInfo);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleNotifyAppUseControlList(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleMinimizeMainSession
 * @tc.desc: test function : HandleMinimizeMainSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleMinimizeMainSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "appbundleName";
    int32_t appIndex = 1;
    int32_t userId = 1;

    data.WriteString(bundleName);
    data.WriteInt32(appIndex);
    data.WriteInt32(userId);

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleMinimizeMainSession(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleLockSessionByAbilityInfo
 * @tc.desc: test function : HandleLockSessionByAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleLockSessionByAbilityInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "appbundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    int32_t appIndex = 0;
    bool isLock = true;

    data.WriteString(bundleName);
    data.WriteString(moduleName);
    data.WriteString(abilityName);
    data.WriteInt32(appIndex);
    data.WriteBool(isLock);

    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleLockSessionByAbilityInfo(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetRecentMainSessionInfoList
 * @tc.desc: test function : HandleGetRecentMainSessionInfoList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetRecentMainSessionInfoList, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleGetRecentMainSessionInfoList(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleCreateNewInstanceKey
 * @tc.desc: test function : HandleCreateNewInstanceKey
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleCreateNewInstanceKey, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    std::string bundleName = "bundleName";
    data.WriteString(bundleName);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleCreateNewInstanceKey(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandlePendingSessionToBackgroundByPersistentId
 * @tc.desc: test function : HandlePendingSessionToBackgroundByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToBackgroundByPersistentId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToBackgroundByPersistentId(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
    
    data.WriteInt32(1);
    res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToBackgroundByPersistentId(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
 
    MessageParcel data2;
    data2.WriteInt32(1);
    data2.WriteBool(true);
    res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToBackgroundByPersistentId(data2, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleRemoveInstanceKey
 * @tc.desc: test function : HandleRemoveInstanceKey
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleRemoveInstanceKey, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    std::string bundleName = "bundleName";
    std::string instanceKey = "instanceKey";
    data.WriteString(bundleName);
    data.WriteString(instanceKey);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleRemoveInstanceKey(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandlePendingSessionToBackground_ShouldReturnInvalidData_WhenTokenIsNull
 * @tc.desc: test function : HandlePendingSessionToBackgroundTest_001
 * @tc.type: 测试当 token 为 nullptr 时,函数应返回 ERR_INVALID_DATA
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToBackgroundTest_001, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int result = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToBackground(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandlePendingSessionToBackground_ShouldReturnInvalidData_WhenReadBoolFailed
 * @tc.desc: test function : HandlePendingSessionToBackgroundTest_002
 * @tc.type: 测试当读取 shouldBackToCaller 失败时,函数应返回 ERR_INVALID_DATA
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToBackgroundTest_002, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    data.WriteRemoteObject(token);

    int result = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToBackground(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandlePendingSessionToBackground_ShouldReturnInvalidData_WhenPendingSessionToBackgroundSucceeds
 * @tc.desc: test function : HandlePendingSessionToBackgroundTest_003
 * @tc.type: 测试当 wantParams 为 nullptr 时,函数应返回 ERR_NONE
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandlePendingSessionToBackgroundTest_003, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    data.WriteRemoteObject(token);
    data.WriteInt32(1);
    data.writebool(true);

    int result = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandlePendingSessionToBackground(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateKioskAppList
 * @tc.desc: test function : HandleUpdateKioskAppList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUpdateKioskAppList, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleUpdateKioskAppList(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleEnterKioskMode
 * @tc.desc: test function : HandleEnterKioskMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleEnterKioskMode, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(token);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleEnterKioskMode(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleExitKioskMode
 * @tc.desc: test function : HandleExitKioskMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleExitKioskMode, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(token);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::HandleExitKioskMode(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequest
 * @tc.desc: ProcessRemoteRequest TRANS_ID_SEND_POINTER_EVENT_FOR_HOVER
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, ProcessRemoteRequest_Hover, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    const sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(token);
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_SEND_POINTER_EVENT_FOR_HOVER);
    auto res = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleUpdateSessionScreenLock
 * @tc.desc: handle update session screen lock
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleUpdateSessionScreenLock, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ASSERT_NE(sceneSessionManagerLiteStub_, nullptr);

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_SESSION_SCREEN_LOCK);
    auto res = sceneSessionManagerLiteStub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSendPointerEventForHover
 * @tc.desc: HandleSendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleSendPointerEventForHover, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(token);
    auto res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSendPointerEventForHover(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->WriteToParcel(data);
    res = sceneSessionManagerLiteStub_->SceneSessionManagerLiteStub::HandleSendPointerEventForHover(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnRegPipChgListener_ReadIntFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleUnRegisterPipChgListener(data, reply), ERR_INVALID_DATA);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnRegPipChgListener_Success, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(
    ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_PIP_CHG_LISTENER);
    data.WriteInt32(1);
    auto result = sceneSessionManagerLiteStub_->
    SceneSessionManagerLiteStub::ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_NONE);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegPipChgListener_ReadIntFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleRegisterPipChgListener(data, reply), ERR_INVALID_DATA);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegPipChgListener_ReadListenerFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleRegisterPipChgListener(data, reply), ERR_INVALID_DATA);
}

class MockPipChgListener : public PipChangeListenerStub {
public:
    void OnPipStart(int32_t windowId) override {};
};

HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegPipChgListener_NullListenerFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);

    data.WriteRemoteObject(nullptr);
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleRegisterPipChgListener(data, reply), ERR_INVALID_DATA);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleRegPipChgListener_Success, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInt32(1);
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    data.WriteRemoteObject(listener->AsObject());
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_PIP_CHG_LISTENER);
    auto result = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_NONE);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnsetPipEnableByScreenId_Success, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInt32(1);
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_UNSET_PIP_ENABLED_BY_SCREENID);
    auto result = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_NONE);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleUnsetPipEnableByScreenId_ReadIntFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleUnsetPipEnableByScreenId(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetDisplayIdByWindowId
 * @tc.desc: test HandleGetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetDisplayIdByWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::vector<uint64_t> windowIds = { 1, 2 };
    data.WriteUInt64Vector(windowIds);

    int res = sceneSessionManagerLiteStub_->HandleGetDisplayIdByWindowId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetPipEnableByScreenId_Success, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInt32(1);
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_SET_PIP_ENABLED_BY_SCREENID);
    auto result = sceneSessionManagerLiteStub_->
        SceneSessionManagerLiteStub::ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_NONE);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetPipEnableByScreenId_ReadIntFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleSetPipEnableByScreenId(data, reply), ERR_INVALID_DATA);
}

HWTEST_F(SceneSessionManagerLiteStubTest, HandleSetPipEnableByScreenId_ReadBoolFailed, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    EXPECT_EQ(sceneSessionManagerLiteStub_->HandleSetPipEnableByScreenId(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetDisplayIdByWindowId01
 * @tc.desc: test HandleGetDisplayIdByWindowId01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetDisplayIdByWindowId01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::vector<uint64_t> windowIds = (200, 0);
    data.WriteUInt64Vector(windowIds);

    int res = sceneSessionManagerLiteStub_->HandleGetDisplayIdByWindowId(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleGetDisplayIdByWindowId02
 * @tc.desc: test HandleGetDisplayIdByWindowId02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteStubTest, HandleGetDisplayIdByWindowId02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::vector<uint64_t> windowIds = {1, 2};
    data.WriteUInt64Vector(windowIds);

    int res = sceneSessionManagerLiteStub_->HandleGetDisplayIdByWindowId(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}
} // namespace
} // namespace Rosen
} // namespace OHOS