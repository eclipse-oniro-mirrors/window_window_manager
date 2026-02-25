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

} // namespace
} // namespace Rosen
} // namespace OHOS