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

#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "iremote_object_mocker.h"
#include "interfaces/include/ws_common.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_proxy.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowManagerServiceDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void DumpRootSceneElementInfoFuncTest(const sptr<SceneSession>& session,
    const std::vector<std::string>& params, std::vector<std::string>& infos) {}

void WindowManagerServiceDumpTest::SetUpTestCase() {}

void WindowManagerServiceDumpTest::TearDownTestCase() {}

void WindowManagerServiceDumpTest::SetUp()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void WindowManagerServiceDumpTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    ssm_ = nullptr;
}

namespace {

/**
 * @tc.name: GetSessionDumpInfo
 * @tc.desc: GetSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetSessionDumpInfo01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::string dumpInfo = "testDumpInfo";
    std::vector<std::string> params = { "testDumpInfo" };
    MockAccesstokenKit::MockIsSACalling(false);
    WSError result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
    MockAccesstokenKit::MockIsSACalling(true);
    result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);

    params.clear();
    params.push_back("-w");
    params.push_back("a");
    result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);

    params.clear();
    params.push_back("-b");
    params.push_back("a");
    result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_OK);

    params.clear();
    params.push_back("-p");
    params.push_back("1");
    result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_OK);

    params.clear();
    params.push_back("-b");
    params.push_back("a");
    result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_OK);

    params.clear();
    params.push_back("testDumpInfo");
    params.push_back("a");
    result = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: GetSessionDumpInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetSessionDumpInfo02, TestSize.Level1)
{
    std::vector<std::string> params;
    std::string info = "info";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->GetSessionDumpInfo(params, info));
}

/**
 * @tc.name: DumpSessionInfo
 * @tc.desc: SceneSesionManager dump session info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, DumpSessionInfo, TestSize.Level1)
{
    SessionInfo info;
    std::ostringstream oss;
    std::string dumpInfo;
    info.abilityName_ = "DumpSessionInfo";
    info.bundleName_ = "DumpSessionInfo";
    info.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->DumpSessionInfo(sceneSession, oss);
    EXPECT_FALSE(sceneSession->IsVisible());

    sptr<SceneSession::SpecificSessionCallback> specific = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, specific);
    sceneSession = sptr<SceneSession>::MakeSptr(info, specific);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->DumpSessionInfo(sceneSession, oss);
    EXPECT_FALSE(sceneSession->IsVisible());
    sceneSession = nullptr;
    info.isSystem_ = true;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->DumpSessionInfo(sceneSession, oss);
}

/**
 * @tc.name: DumpSessionAll
 * @tc.desc: ScreenSesionManager dump all session info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, DumpSessionAll, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "WindowManagerServiceDumpTest";
    sessionInfo.abilityName_ = "DumpSessionAll";
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, windowSessionProperty);
    ASSERT_EQ(nullptr, sceneSession);
    std::vector<std::string> infos;
    WSError result = ssm_->DumpSessionAll(infos);
    ASSERT_EQ(WSError::WS_OK, result);
    ASSERT_FALSE(infos.empty());
}

/**
 * @tc.name: DumpSesGetFloatWidthsionAll
 * @tc.desc: Get Float Width
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetFloatWidth, TestSize.Level1)
{
    float value = 0.1234;
    std::string strValue = ssm_->GetFloatWidth(5, value);
    ASSERT_EQ("0.123", strValue);
}

/**
 * @tc.name: DumpSessionWithId
 * @tc.desc: ScreenSesionManager dump session with id
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, DumpSessionWithId, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "WindowManagerServiceDumpTest";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, windowSessionProperty);
    ASSERT_EQ(nullptr, sceneSession);
    std::vector<std::string> infos;
    WSError result = ssm_->DumpSessionWithId(windowSessionProperty->GetPersistentId(), infos);
    ASSERT_EQ(WSError::WS_OK, result);
    ASSERT_FALSE(infos.empty());
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: SceneSesionManager dump session element info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, DumpSessionElementInfo, TestSize.Level1)
{
    DumpRootSceneElementInfoFunc func_ = DumpRootSceneElementInfoFuncTest;
    ssm_->SetDumpRootSceneElementInfoListener(func_);
    SessionInfo info;
    info.abilityName_ = "DumpSessionElementInfo";
    info.bundleName_ = "DumpSessionElementInfo";
    info.isSystem_ = false;
    std::string strId = "10086";
    sptr<SceneSession> sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    std::vector<std::string> params_(5, "");
    std::string dumpInfo;
    ssm_->DumpSessionElementInfo(sceneSession, params_, dumpInfo);
    sceneSession = nullptr;
    info.isSystem_ = true;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->DumpSessionElementInfo(sceneSession, params_, dumpInfo);
    WSError result01 = ssm_->GetSpecifiedSessionDumpInfo(dumpInfo, params_, strId);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: NotifyDumpInfoResult
 * @tc.desc: SceneSesionManager notify dump info result
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, NotifyDumpInfoResult, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(true);
    std::vector<std::string> info = { "std::", "vector", "<std::string>" };
    ssm_->NotifyDumpInfoResult(info);
    std::vector<std::string> params = { "-a" };
    std::string dumpInfo = "";
    WSError result01 = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_EQ(result01, WSError::WS_OK);
    params.clear();
    params.push_back("-w");
    params.push_back("23456");
    WSError result02 = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_NE(result02, WSError::WS_OK);
    params.clear();
    WSError result03 = ssm_->GetSessionDumpInfo(params, dumpInfo);
    EXPECT_NE(result03, WSError::WS_OK);
}

/**
 * @tc.name: GetAllSessionDumpInfo
 * @tc.desc: SceneSesionManager test GetAllSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetAllSessionDumpInfo01, TestSize.Level1)
{
    SessionInfo info1;
    info1.abilityName_ = "GetAllSessionDumpInfo1";
    info1.bundleName_ = "GetAllSessionDumpInfo1";
    info1.persistentId_ = 1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->UpdateNativeVisibility(true);

    SessionInfo info2;
    info2.abilityName_ = "GetAllSessionDumpInfo2";
    info2.bundleName_ = "GetAllSessionDumpInfo2";
    info2.persistentId_ = 2;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->UpdateNativeVisibility(false);

    ssm_->sceneSessionMap_.insert({ 1, sceneSession1 });
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });
    std::string dumpInfo;
    ASSERT_EQ(ssm_->GetAllSessionDumpInfo(dumpInfo), WSError::WS_OK);
}

/**
 * @tc.name: GetAllSessionDumpInfo
 * @tc.desc: GetAllSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetAllSessionDumpInfo02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "GetAllSessionDumpInfo";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    std::string dumpInfo = "";
    auto ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession->state_ = SessionState::STATE_END;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sessionInfo.isSystem_ = true;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession = nullptr;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetAllSessionDumpInfo01
 * @tc.desc: GetAllSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetAllSessionDumpInfo03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "GetAllSessionDumpInfo01";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    std::string dumpInfo = "";
    auto ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession->isVisible_ = false;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetAllSessionDumpDetailInfo
 * @tc.desc: SceneSesionManager test GetAllSessionDumpDetailInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetAllSessionDumpDetailInfo, TestSize.Level1)
{
    SessionInfo info1;
    info1.abilityName_ = "GetAllSessionDumpDetailInfo1";
    info1.bundleName_ = "GetAllSessionDumpDetailInfo1";
    info1.persistentId_ = 1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->UpdateNativeVisibility(true);

    SessionInfo info2;
    info2.abilityName_ = "GetAllSessionDumpDetailInfo2";
    info2.bundleName_ = "GetAllSessionDumpDetailInfo2";
    info2.persistentId_ = 2;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->UpdateNativeVisibility(false);

    ssm_->sceneSessionMap_.insert({ 0, nullptr });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession1 });
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });
    std::string dumpInfo;
    ASSERT_EQ(ssm_->GetAllSessionDumpDetailInfo(dumpInfo), WSError::WS_OK);
}

/**
 * @tc.name: GetSpecifiedSessionDumpInfo
 * @tc.desc: SceneSesionManager test GetSpecifiedSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetSpecifiedSessionDumpInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSpecifiedSessionDumpInfo";
    info.bundleName_ = "GetSpecifiedSessionDumpInfo";
    info.persistentId_ = 1234;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1234, sceneSession });
    std::string dumpInfo;
    std::string strId = "1234";
    std::vector<std::string> params_(5, "");
    ASSERT_EQ(ssm_->GetSpecifiedSessionDumpInfo(dumpInfo, params_, strId), WSError::WS_OK);
}
/**
 * @tc.name: GetTotalUITreeInfo
 * @tc.desc: GetTotalUITreeInfo set gesture navigation enabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetTotalUITreeInfo, TestSize.Level1)
{
    std::string dumpInfo = "dumpInfo";
    ssm_->SetDumpUITreeFunc(nullptr);
    EXPECT_EQ(WSError::WS_OK, ssm_->GetTotalUITreeInfo(dumpInfo));
    DumpUITreeFunc func = [](std::string& dumpInfo) { return; };
    ssm_->SetDumpUITreeFunc(func);
    EXPECT_EQ(WSError::WS_OK, ssm_->GetTotalUITreeInfo(dumpInfo));
}

/**
 * @tc.name: DumpSessionInfo
 * @tc.desc: DumpSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, SceneSessionDumpSessionInfo, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "SceneSessionTest";
    info.abilityName_ = "DumpSessionInfo";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    std::vector<std::string> infos;
    sceneSession->DumpSessionInfo(infos);
    ASSERT_FALSE(infos.empty());
}

/**
 * @tc.name: DumpSessionElementInfo01
 * @tc.desc: DumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, SceneSessionDumpSessionElementInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    std::vector<std::string> params;
    sceneSession->DumpSessionElementInfo(params);
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->DumpSessionElementInfo(params);
}

/**
 * @tc.name: GetSCBDebugDumpInfo01
 * @tc.desc: GetSCBDebugDumpInfo01
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetSCBDebugDumpInfo01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "Background01";
    sessionInfo.bundleName_ = "GetSCBDebugDumpInfo01";
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, windowSessionProperty);
    ASSERT_EQ(nullptr, sceneSession);
    std::string infos;
    WSError result = ssm_->GetSCBDebugDumpInfo("SCBScenePanel getContainerSession 0", infos);
    ASSERT_EQ(WSError::WS_OK, result);
    ASSERT_FALSE(infos.empty());
}

/**
 * @tc.name: GetSCBDebugDumpInfo02
 * @tc.desc: GetSCBDebugDumpInfo02
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceDumpTest, GetSCBDebugDumpInfo02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "Background01";
    sessionInfo.bundleName_ = "GetSCBDebugDumpInfo02";
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, windowSessionProperty);
    ASSERT_EQ(nullptr, sceneSession);
    std::string infos;
    WSError result = ssm_->GetSCBDebugDumpInfo("SCBScenePanel getContainerSession -f", infos);
    ASSERT_EQ(WSError::WS_OK, result);
    ASSERT_FALSE(infos.empty());
}
} // namespace
} // namespace Rosen
} // namespace OHOS