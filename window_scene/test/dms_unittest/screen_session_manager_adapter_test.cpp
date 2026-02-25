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
#include "screen_session_manager_adapter.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog += msg;
    }
}
class ScreenSessionManagerAdapterTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};
void ScreenSessionManagerAdapterTest::SetUp()
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
}
void ScreenSessionManagerAdapterTest::TearDown()
{
    LOG_SetCallback(nullptr);
}
namespace {
/**
 * @tc.name: OnDisplayChange01_DisplayInfo_Nullptr
 * @tc.desc: OnDisplayChange01_DisplayInfo_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnDisplayChange01_DisplayInfo_Nullptr, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = nullptr;
    DisplayChangeEvent event = DisplayChangeEvent::UNKNOWN;
    int32_t uid = 1000;
    ScreenSessionManagerAdapter adapter;
    adapter.OnDisplayChange(displayInfo, event, uid);
    EXPECT_TRUE(g_errLog.find("Invalid display info") != std::string::npos);
}

/**
 * @tc.name: OnDisplayChange01_Agents_Null
 * @tc.desc: OnDisplayChange01_Agents_Null
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnDisplayChange01_Agents_Null, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP();
    }
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    DisplayChangeEvent event = DisplayChangeEvent::UNKNOWN;
    int32_t uid = 1000;
    ScreenSessionManagerAdapter adapter;
    adapter.OnDisplayChange(displayInfo, event, uid);
    EXPECT_TRUE(g_errLog.find("agents is empty") != std::string::npos);
}

/**
 * @tc.name: OnDisplayChange02_DisplayInfo_Nullptr
 * @tc.desc: OnDisplayChange02_DisplayInfo_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnDisplayChange02_DisplayInfo_Nullptr, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = nullptr;
    DisplayChangeEvent event = DisplayChangeEvent::UNKNOWN;
    ScreenSessionManagerAdapter adapter;
    adapter.OnDisplayChange(displayInfo, event);
    EXPECT_TRUE(g_errLog.find("Invalid display info") != std::string::npos);
}

/**
 * @tc.name: OnDisplayChange02_Agents_UPDATE_REFRESHRATE
 * @tc.desc: OnDisplayChange02_Agents_UPDATE_REFRESHRATE
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnDisplayChange02_Agents_UPDATE_REFRESHRATE, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    DisplayChangeEvent event = DisplayChangeEvent::UPDATE_REFRESHRATE;
    ScreenSessionManagerAdapter adapter;
    adapter.OnDisplayChange(displayInfo, event);
    EXPECT_TRUE(g_errLog.find("agent size: 0") == std::string::npos);  // can't record TLOGND log
    EXPECT_TRUE(g_errLog.find("OnDisplayChange agent is null") != std::string::npos);
}

/**
 * @tc.name: OnDisplayAttributeChange_DisplayInfo_Nullptr
 * @tc.desc: OnDisplayAttributeChange_DisplayInfo_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnDisplayAttributeChange_DisplayInfo_Nullptr, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = nullptr;
    std::vector<std::string> attributes = {};
    ScreenSessionManagerAdapter adapter;
    adapter.OnDisplayAttributeChange(displayInfo, attributes);
    EXPECT_TRUE(g_errLog.find("attributes is empty") != std::string::npos);
}

/**
 * @tc.name: IsAgentListenedAttributes_ContainsAttribute_ReturnTrue
 * @tc.desc: Test when listenedAttributes contains one of the attributes
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, IsAgentListenedAttributes_ContainsAttribute_ReturnTrue, TestSize.Level1)
{
    std::set<std::string> listenedAttributes = {"brightness", "rotation", "dpi"};
    std::vector<std::string> attributes = {"rotation", "refresh_rate"};
    ScreenSessionManagerAdapter adapter;
    bool result = adapter.IsAgentListenedAttributes(listenedAttributes, attributes);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsAgentListenedAttributes_NotContainsAttribute_ReturnFalse
 * @tc.desc: Test when listenedAttributes does not contain any of the attributes
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, IsAgentListenedAttributes_NotContainsAttribute_ReturnFalse, TestSize.Level1)
{
    std::set<std::string> listenedAttributes = {"dpi", "density"};
    std::vector<std::string> attributes = {"rotation", "refresh_rate"};
    ScreenSessionManagerAdapter adapter;
    bool result = adapter.IsAgentListenedAttributes(listenedAttributes, attributes);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: NotifyScreenModeChange_ScreenInfos_Empty
 * @tc.desc: NotifyScreenModeChange_ScreenInfos_Empty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, NotifyScreenModeChange_ScreenInfos_Empty, TestSize.Level1)
{
    std::vector<sptr<ScreenInfo>> screenInfos = {};
    ScreenSessionManagerAdapter adapter;
    adapter.NotifyScreenModeChange(screenInfos);
    EXPECT_TRUE(g_errLog.find("Empty screen info vector received") != std::string::npos);
}

/**
 * @tc.name: NotifyScreenModeChange_ScreenInfos_NotEmpty
 * @tc.desc: NotifyScreenModeChange_ScreenInfos_NotEmpty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, NotifyScreenModeChange_ScreenInfos_NotEmpty, TestSize.Level1)
{
    std::vector<sptr<ScreenInfo>> screenInfos = { new ScreenInfo() };
    ScreenSessionManagerAdapter adapter;
    adapter.NotifyScreenModeChange(screenInfos);
    EXPECT_TRUE(g_errLog.find("agent is null") != std::string::npos);
}

/**
 * @tc.name: OnScreenChange_ScreenInfos_Nullptr
 * @tc.desc: OnScreenChange_ScreenInfos_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnScreenChange_ScreenInfos_Nullptr, TestSize.Level1)
{
    sptr<ScreenInfo> screenInfo = nullptr;
    ScreenChangeEvent event = ScreenChangeEvent::UPDATE_ORIENTATION;
    ScreenSessionManagerAdapter adapter;
    adapter.OnScreenChange(screenInfo, event);
    EXPECT_TRUE(g_errLog.find("error, screenInfo is nullptr") != std::string::npos);
}

/**
 * @tc.name: OnDisplayCreate_DisplayInfo_Nullptr
 * @tc.desc: OnDisplayCreate_DisplayInfo_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnDisplayCreate_DisplayInfo_Nullptr, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP();
    }
    sptr<DisplayInfo> displayInfo = nullptr;
    ScreenSessionManagerAdapter adapter;
    adapter.OnDisplayCreate(displayInfo);
    EXPECT_TRUE(g_errLog.find("displayInfo nullptr") != std::string::npos);
}

/**
 * @tc.name: OnScreenConnect_ScreenInfos_Nullptr
 * @tc.desc: OnScreenConnect_ScreenInfos_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnScreenConnect_ScreenInfos_Nullptr, TestSize.Level1)
{
    sptr<ScreenInfo> screenInfo = nullptr;
    ScreenSessionManagerAdapter adapter;
    adapter.OnScreenConnect(screenInfo);
    EXPECT_TRUE(g_errLog.find("screenInfo nullptr") != std::string::npos);
}

/**
 * @tc.name: NotifyFoldAngleChanged_FoldAngles_Empty
 * @tc.desc: NotifyFoldAngleChanged_FoldAngles_Empty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, NotifyFoldAngleChanged_FoldAngles_Empty, TestSize.Level1)
{
    std::vector<float> foldAngles = {0.0f, 90.0f, 180.0f};
    ScreenSessionManagerAdapter adapter;
    adapter.NotifyFoldAngleChanged(foldAngles);
    EXPECT_TRUE(g_errLog.find("agent is null") != std::string::npos);
}

/**
 * @tc.name: NotifyCaptureStatusChanged_Agent_Empty
 * @tc.desc: NotifyCaptureStatusChanged_Agent_Empty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, NotifyCaptureStatusChanged_Agent_Empty, TestSize.Level1)
{
    bool isCapture = false;
    ScreenSessionManagerAdapter adapter;
    adapter.NotifyCaptureStatusChanged(isCapture);
    EXPECT_TRUE(g_errLog.find("agent is null") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayChangeInfoChanged_Agent_Empty
 * @tc.desc: NotifyDisplayChangeInfoChanged_Agent_Empty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, NotifyDisplayChangeInfoChanged_Agent_Empty, TestSize.Level1)
{
    sptr<DisplayChangeInfo> info = new DisplayChangeInfo();
    ScreenSessionManagerAdapter adapter;
    adapter.NotifyDisplayChangeInfoChanged(info);
    EXPECT_TRUE(g_errLog.find("agent is null") != std::string::npos);
}

/**
 * @tc.name: OnScreenshot_ScreenshotInfo_Nullptr
 * @tc.desc: OnScreenshot_ScreenshotInfo_Nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, OnScreenshot_ScreenshotInfo_Nullptr, TestSize.Level1)
{
    sptr<ScreenshotInfo> info = nullptr;
    ScreenSessionManagerAdapter adapter;
    adapter.OnScreenshot(info);
    EXPECT_TRUE(g_errLog.find("info is null") != std::string::npos);
}

/**
 * @tc.name: NotifyAbnormalScreenConnectChange_Agent_Empty
 * @tc.desc: NotifyAbnormalScreenConnectChange_Agent_Empty
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerAdapterTest, NotifyAbnormalScreenConnectChange_Agent_Empty, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenSessionManagerAdapter adapter;
    adapter.NotifyAbnormalScreenConnectChange(screenId);
    EXPECT_TRUE(g_errLog.find("agent is null") != std::string::npos);
}
}
}
}