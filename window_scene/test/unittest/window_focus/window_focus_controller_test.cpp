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
#include "session/host/include/scene_session.h"
#include "session_manager/include/window_focus_controller.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_helper.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
                       const char* msg)
    {
        g_logMsg += msg;
    }
}
class WindowFocusControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<SceneSessionManager> ssm_;
};
sptr<SceneSessionManager> WindowFocusControllerTest::ssm_ = nullptr;

void WindowFocusControllerTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void WindowFocusControllerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void WindowFocusControllerTest::SetUp() {}

void WindowFocusControllerTest::TearDown() {}

namespace {

/**
 * @tc.name: AddFocusGroup
 * @tc.desc: AddFocusGroup
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, AddFocusGroup, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::AddFocusGroup start";
    EXPECT_NE(ssm_, nullptr);
    WSError res = ssm_->windowFocusController_->AddFocusGroup(0, DISPLAY_ID_INVALID);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, res);

    res = ssm_->windowFocusController_->AddFocusGroup(0, 0);
    EXPECT_EQ(WSError::WS_OK, res);

    res = ssm_->windowFocusController_->AddFocusGroup(1, 1);
    EXPECT_EQ(WSError::WS_OK, res);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::AddFocusGroup end";
}

/**
 * @tc.name: AddFocusGroup
 * @tc.desc: AddFocusGroup
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, AddFocusGroup02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::AddFocusGroup02 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    WSError res = wfc->AddFocusGroup(0, DISPLAY_ID_INVALID);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, res);

    // not found
    res = wfc->AddFocusGroup(0, 0);
    EXPECT_EQ(WSError::WS_OK, res);

    // already found
    res = wfc->AddFocusGroup(1, 1);
    res = wfc->AddFocusGroup(1, 2);
    EXPECT_EQ(2, wfc->focusGroupMap_.at(1)->displayIds_.size());
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::AddFocusGroup02 end";
}

/**
 * @tc.name: RemoveFocusGroup
 * @tc.desc: RemoveFocusGroup
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, RemoveFocusGroup, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::RemoveFocusGroup start";
    EXPECT_NE(ssm_, nullptr);
    WSError res = ssm_->windowFocusController_->RemoveFocusGroup(0, DISPLAY_ID_INVALID);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, res);

    res = ssm_->windowFocusController_->RemoveFocusGroup(0, 0);
    EXPECT_EQ(WSError::WS_OK, res);

    ssm_->windowFocusController_->AddFocusGroup(0, 0);
    res = ssm_->windowFocusController_->RemoveFocusGroup(0, 0);
    EXPECT_EQ(WSError::WS_OK, res);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::RemoveFocusGroup end";
}

/**
 * @tc.name: RemoveFocusGroup
 * @tc.desc: RemoveFocusGroup
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, RemoveFocusGroup02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::RemoveFocusGroup02 start";
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    WSError res = wfc->RemoveFocusGroup(0, DISPLAY_ID_INVALID);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, res);

    res = wfc->RemoveFocusGroup(1000, 1000);
    EXPECT_TRUE(g_logMsg.find("displayGroupId invalid") != std::string::npos);

    wfc->AddFocusGroup(0, 0);
    res = wfc->RemoveFocusGroup(0, 0);
    EXPECT_EQ(WSError::WS_OK, res);
    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::RemoveFocusGroup02 end";
}

/**
 * @tc.name: GetFocusGroupInner
 * @tc.desc: GetFocusGroupInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetFocusGroupInner, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetFocusGroupInner start";
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    sptr<FocusGroup> res = wfc->GetFocusGroupInner(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(wfc->focusGroupMap_[DEFAULT_DISPLAY_ID], res);

    wfc->displayId2GroupIdMap_.insert({ 1001, 1001 });
    res = wfc->GetFocusGroupInner(1001);
    EXPECT_TRUE(g_logMsg.find("Not found focus group") != std::string::npos);

    wfc->AddFocusGroup(100, 100);
    wfc->displayId2GroupIdMap_.insert({ 100, 100 });
    res = wfc->GetFocusGroupInner(100);
    EXPECT_EQ(wfc->focusGroupMap_.at(100), res);

    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetFocusGroupInner end";
}

/**
 * @tc.name: GetAllFocusedSessionList
 * @tc.desc: GetAllFocusedSessionList
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetAllFocusedSessionList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetAllFocusedSessionList start";
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    wfc->focusGroupMap_[100] = nullptr;
    std::vector<std::pair<DisplayId, int32_t>> res = wfc->GetAllFocusedSessionList();
    EXPECT_TRUE(g_logMsg.find("focus group is null") != std::string::npos);

    wfc->focusGroupMap_.clear();
    wfc->AddFocusGroup(1, 1);
    res = wfc->GetAllFocusedSessionList();
    EXPECT_EQ(1, res.size());

    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetAllFocusedSessionList end";
}

/**
 * @tc.name: LogDisplayIds
 * @tc.desc: LogDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, LogDisplayIds, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::LogDisplayIds start";
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();

    wfc->focusGroupMap_.clear();
    wfc->AddFocusGroup(100, 100);
    wfc->LogDisplayIds();
    EXPECT_TRUE(g_logMsg.find(", displayids:100;") != std::string::npos);

    wfc->focusGroupMap_.clear();
    wfc->AddFocusGroup(101, 101);
    wfc->AddFocusGroup(102, 102);
    wfc->LogDisplayIds();
    EXPECT_TRUE(g_logMsg.find(", displayids:102;") != std::string::npos);

    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::LogDisplayIds end";
}

/**
 * @tc.name: GetDisplayGroupId
 * @tc.desc: GetDisplayGroupId
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetDisplayGroupId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetDisplayGroupId start";
    EXPECT_NE(ssm_, nullptr);
    DisplayId res = ssm_->windowFocusController_->GetDisplayGroupId(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(DEFAULT_DISPLAY_ID, res);

    ssm_->windowFocusController_->displayId2GroupIdMap_.clear();
    res = ssm_->windowFocusController_->GetDisplayGroupId(1);
    EXPECT_EQ(DEFAULT_DISPLAY_ID, res);

    ssm_->windowFocusController_->deletedDisplayId2GroupIdMap_.insert({ 1, 1 });
    res = ssm_->windowFocusController_->GetDisplayGroupId(1);
    EXPECT_EQ(DISPLAY_ID_INVALID, res);
    ssm_->windowFocusController_->deletedDisplayId2GroupIdMap_.clear();
    
    ssm_->windowFocusController_->displayId2GroupIdMap_.insert({ 1, 1 });
    res = ssm_->windowFocusController_->GetDisplayGroupId(1);
    EXPECT_EQ(1, res);
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetDisplayGroupId end";
}

/**
 * @tc.name: GetShouldCheckBlocking01
 * @tc.desc: Test basic behavior: result equals byForeground for most reasons
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetShouldCheckBlocking01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking01 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::DEFAULT));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, nullptr, false, FocusChangeReason::DEFAULT));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::FOREGROUND));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, nullptr, false, FocusChangeReason::SCB_START_APP));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::CLICK));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::MOVE_UP));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::RECENT));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::SPLIT_SCREEN));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, nullptr, false, FocusChangeReason::FLOATING_SCENE));
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking01 end";
}

/**
 * @tc.name: GetShouldCheckBlocking02
 * @tc.desc: FORCE_FOCUSED reason always returns false regardless of byForeground or sessions
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetShouldCheckBlocking02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking02 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::FORCE_FOCUSED));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, nullptr, false, FocusChangeReason::FORCE_FOCUSED));
    
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(session, session, true, FocusChangeReason::FORCE_FOCUSED));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(session, nullptr, true, FocusChangeReason::FORCE_FOCUSED));
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking02 end";
}

/**
 * @tc.name: GetShouldCheckBlocking03
 * @tc.desc: CLIENT_REQUEST with null sessions returns byForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetShouldCheckBlocking03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking03 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, nullptr, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, nullptr, false, FocusChangeReason::CLIENT_REQUEST));
    
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, session, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(session, nullptr, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(nullptr, session, false, FocusChangeReason::CLIENT_REQUEST));
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking03 end";
}

/**
 * @tc.name: GetShouldCheckBlocking04
 * @tc.desc: CLIENT_REQUEST with same MissionId app sessions skips blocking check (returns false)
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetShouldCheckBlocking04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking04 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    
    SessionInfo info;
    info.bundleName_ = "TestApp";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 100;
    focusedSession->persistentId_ = 100;
    
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(sceneSession, focusedSession, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(sceneSession, focusedSession, false, FocusChangeReason::CLIENT_REQUEST));
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking04 end";
}

/**
 * @tc.name: GetShouldCheckBlocking05
 * @tc.desc: CLIENT_REQUEST with different MissionId or non-app session returns byForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetShouldCheckBlocking05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking05 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    
    SessionInfo appInfo;
    appInfo.bundleName_ = "TestApp";
    appInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    
    SessionInfo systemInfo;
    systemInfo.bundleName_ = "TestSystem";
    systemInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SYSTEM_ALERT_WINDOW);
    systemInfo.isSystem_ = true;
    
    sptr<SceneSession> appSession1 = sptr<SceneSession>::MakeSptr(appInfo, nullptr);
    sptr<SceneSession> appSession2 = sptr<SceneSession>::MakeSptr(appInfo, nullptr);
    appSession1->persistentId_ = 100;
    appSession2->persistentId_ = 200;
    
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(appSession1, appSession2, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(appSession1, appSession2, false, FocusChangeReason::CLIENT_REQUEST));
    
    sptr<SceneSession> systemSession = sptr<SceneSession>::MakeSptr(systemInfo, nullptr);
    systemSession->persistentId_ = 100;
    
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(systemSession, appSession1, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(appSession1, systemSession, true, FocusChangeReason::CLIENT_REQUEST));
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking05 end";
}

/**
 * @tc.name: GetShouldCheckBlocking06
 * @tc.desc: Verify all branches with mixed inputs
 * @tc.type: FUNC
 */
HWTEST_F(WindowFocusControllerTest, GetShouldCheckBlocking06, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking06 start";
    sptr<WindowFocusController> wfc = sptr<WindowFocusController>::MakeSptr();
    
    SessionInfo appInfo;
    appInfo.bundleName_ = "TestApp";
    appInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    
    sptr<SceneSession> appSession = sptr<SceneSession>::MakeSptr(appInfo, nullptr);
    appSession->persistentId_ = 100;
    
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(appSession, appSession, true, FocusChangeReason::FORCE_FOCUSED));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(appSession, appSession, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(appSession, nullptr, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(nullptr, appSession, true, FocusChangeReason::CLIENT_REQUEST));
    EXPECT_TRUE(wfc->GetShouldCheckBlocking(appSession, nullptr, true, FocusChangeReason::DEFAULT));
    EXPECT_FALSE(wfc->GetShouldCheckBlocking(appSession, nullptr, false, FocusChangeReason::DEFAULT));
    GTEST_LOG_(INFO) << "WindowFocusControllerTest::GetShouldCheckBlocking06 end";
}

} // namespace
} // namespace Rosen
} // namespace OHOS