/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "scene_input_manager.h"
#include <gtest/gtest.h>
#include "session_manager/include/scene_session_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_dirty_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg += msg;
    }
}

namespace OHOS {
namespace Rosen {
constexpr int MAX_WINDOWINFO_NUM = 15;
class SceneInputManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<SceneSessionManager> ssm_;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

sptr<SceneSessionManager> SceneInputManagerTest::ssm_ = nullptr;

void SceneInputManagerTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneInputManagerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneInputManagerTest::SetUp()
{
}

void SceneInputManagerTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
void CheckNeedUpdateTest()
{
    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    SceneInputManager::GetInstance().SetUserBackground(true);
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    SceneInputManager::GetInstance().lastFocusId_ = -1;
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));

    SceneInputManager::GetInstance().lastWindowInfoList_.clear();
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    SceneInputManager::GetInstance().lastDisplayInfos_.clear();
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    SceneInputManager::GetInstance().lastWindowInfoList_.clear();
    SceneInputManager::GetInstance().lastDisplayInfos_.clear();
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));

    if (SceneInputManager::GetInstance().lastDisplayInfos_.size() != 0) {
        MMI::DisplayInfo displayInfo;
        SceneInputManager::GetInstance().lastDisplayInfos_[0] = displayInfo;
        SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    }

    if (SceneInputManager::GetInstance().lastWindowInfoList_.size() != 0) {
        MMI::WindowInfo windowInfo;
        SceneInputManager::GetInstance().lastWindowInfoList_[0] = windowInfo;
        SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    }
}

void WindowInfoListZeroTest(sptr<SceneSessionManager> ssm_)
{
    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    const auto sceneSessionMap = ssm_->GetSceneSessionMap();
    for (auto sceneSession : sceneSessionMap) {
        ssm_->DestroyDialogWithMainWindow(sceneSession.second);
    }
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));

    for (auto sceneSession : sceneSessionMap) {
        sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
        windowSessionProperty->SetWindowType(sceneSession.second->GetWindowType());
        ssm_->RequestSceneSession(sceneSession.second->GetSessionInfo(), windowSessionProperty);
    }
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
}

void MaxWindowInfoTest(sptr<SceneSessionManager> ssm_)
{
    std::vector<sptr<SceneSession>> sessionList;
    int maxWindowInfoNum = 20;
    int32_t idStart = 1000;
    for (int i = 0; i < maxWindowInfoNum; i++) {
        SessionInfo info;
        info.abilityName_ = "test" + std::to_string(i);
        info.bundleName_ = "test" + std::to_string(i);
        info.appIndex_ = idStart + i;
        sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
        ASSERT_NE(windowSessionProperty, nullptr);
        windowSessionProperty->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
        auto sceneSession = ssm_->RequestSceneSession(info, windowSessionProperty);
        if (sceneSession != nullptr) {
            sessionList.push_back(sceneSession);
        }
    }
    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));

    for (auto session : sessionList) {
        ssm_->DestroyDialogWithMainWindow(session);
    }
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
}

/**
 * @tc.name: UpdateFocusedSessionId
 * @tc.desc: UpdateFocusedSessionId
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, UpdateFocusedSessionId, TestSize.Level1)
{
    auto sceneInputManager = &SceneInputManager::GetInstance();
    ASSERT_NE(sceneInputManager, nullptr);
    EXPECT_EQ(sceneInputManager->focusedSessionId_, -1);

    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));

    sceneInputManager->UpdateFocusedSessionId(INVALID_SESSION_ID);
    EXPECT_EQ(sceneInputManager->focusedSessionId_, -1);
    sceneInputManager->UpdateFocusedSessionId(sceneSession->GetPersistentId());
    EXPECT_EQ(sceneInputManager->focusedSessionId_, -1);
    ExtensionWindowEventInfo extensionInfo{ .persistentId = 12345 };
    sceneSession->AddNormalModalUIExtension(extensionInfo);
    sceneInputManager->UpdateFocusedSessionId(sceneSession->GetPersistentId());
    EXPECT_EQ(sceneInputManager->focusedSessionId_, extensionInfo.persistentId);

    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: CheckNeedUpdate
 * @tc.desc: CheckNeedUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdate6, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    MMI::WindowInfo windowinfo;
    windowInfoList.emplace_back(windowinfo);
    int32_t focusId = 0;
    Rosen::SceneSessionManager::GetInstance().SetFocusedSessionId(focusId, DEFAULT_DISPLAY_ID);
    SceneInputManager::GetInstance().lastFocusId_ = 0;
    SceneInputManager::GetInstance().lastDisplayInfos_ = displayInfos;
    SceneInputManager::GetInstance().lastWindowInfoList_ = windowInfoList;
    bool result = false;
    windowInfoList[0].transform.emplace_back(1.0);
    SceneInputManager::GetInstance().lastWindowInfoList_[0].transform.emplace_back(2.0);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].transform.clear();
    SceneInputManager::GetInstance().lastWindowInfoList_[0].transform.clear();
}

/**
 * @tc.name: CheckNeedUpdate
 * @tc.desc: CheckNeedUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdate7, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    MMI::WindowInfo windowinfo;
    windowInfoList.emplace_back(windowinfo);
    int32_t focusId = 0;
    Rosen::SceneSessionManager::GetInstance().SetFocusedSessionId(focusId, DEFAULT_DISPLAY_ID);
    SceneInputManager::GetInstance().lastFocusId_ = 0;
    SceneInputManager::GetInstance().lastDisplayInfos_ = displayInfos;
    SceneInputManager::GetInstance().lastWindowInfoList_ = windowInfoList;
    bool result = false;

    auto tempPixeMap = std::make_shared<Media::PixelMap>();
    windowInfoList[0].pixelMap = static_cast<void*>(tempPixeMap.get());
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].pixelMap = nullptr;

    windowInfoList[0].windowInputType = MMI::WindowInputType::TRANSMIT_ALL;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].windowInputType = SceneInputManager::GetInstance().lastWindowInfoList_[0].windowInputType;

    windowInfoList[0].windowType = static_cast<int32_t>(WindowType::WINDOW_TYPE_APP_COMPONENT);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].windowType = SceneInputManager::GetInstance().lastWindowInfoList_[0].windowType;

    windowInfoList[0].privacyMode = MMI::SecureFlag::PRIVACY_MODE;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].privacyMode = SceneInputManager::GetInstance().lastWindowInfoList_[0].privacyMode;
}

/**
 * @tc.name: UpdateSecSurfaceInfo
 * @tc.desc: UpdateSecSurfaceInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, UpdateSecSurfaceInfo, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::map<uint64_t, std::vector<SecSurfaceInfo>> emptyMap;
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    ASSERT_NE(oldDirty, nullptr);
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().UpdateSecSurfaceInfo(emptyMap);
    EXPECT_TRUE(logMsg.find("sceneSessionDirty_ is nullptr") != std::string::npos);
    logMsg.clear();

    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;
    SceneInputManager::GetInstance().UpdateSecSurfaceInfo(emptyMap);
    EXPECT_FALSE(logMsg.find("sceneSessionDirty_ is nullptr") != std::string::npos);
}

/**
 * @tc.name: UpdateConstrainedModalUIExtInfo
 * @tc.desc: UpdateConstrainedModalUIExtInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, UpdateConstrainedModalUIExtInfo, TestSize.Level1)
{
    std::map<uint64_t, std::vector<SecSurfaceInfo>> testMap;
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    ASSERT_NE(oldDirty, nullptr);
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().UpdateConstrainedModalUIExtInfo(testMap);

    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;
    std::vector<SecSurfaceInfo> surfaceInfoList;
    SecSurfaceInfo secSurfaceInfo;
    surfaceInfoList.emplace_back(secSurfaceInfo);
    testMap[0] = surfaceInfoList;
    SceneInputManager::GetInstance().UpdateConstrainedModalUIExtInfo(testMap);
}

/**
 * @tc.name: UpdateDisplayAndWindowInfo
 * @tc.desc: UpdateDisplayAndWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, UpdateDisplayAndWindowInfo, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::map<DisplayGroupId, MMI::DisplayGroupInfo> displayGroupMap;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    SceneInputManager::GetInstance().UpdateDisplayAndWindowInfo(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_EQ(windowInfoList.size(), 0);
    MMI::WindowInfo windowinfo;
    windowInfoList.emplace_back(windowinfo);
    windowinfo.defaultHotAreas = std::vector<MMI::Rect>(MMI::WindowInfo::DEFAULT_HOTAREA_COUNT + 1);
    SceneInputManager::GetInstance().UpdateDisplayAndWindowInfo(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_NE(windowInfoList[0].defaultHotAreas.size(), MMI::WindowInfo::DEFAULT_HOTAREA_COUNT);
    windowinfo.defaultHotAreas = std::vector<MMI::Rect>();
    windowInfoList = std::vector<MMI::WindowInfo>(MAX_WINDOWINFO_NUM - 1);
    SceneInputManager::GetInstance().UpdateDisplayAndWindowInfo(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_NE(windowInfoList.size(), MAX_WINDOWINFO_NUM);
    windowInfoList = std::vector<MMI::WindowInfo>(MAX_WINDOWINFO_NUM + 1);
    SceneInputManager::GetInstance().UpdateDisplayAndWindowInfo(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_NE(windowInfoList.size(), MAX_WINDOWINFO_NUM);
    windowInfoList[0].defaultHotAreas.resize(MMI::WindowInfo::DEFAULT_HOTAREA_COUNT + 1);
    SceneInputManager::GetInstance().UpdateDisplayAndWindowInfo(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_NE(windowInfoList[0].defaultHotAreas.size(), MMI::WindowInfo::DEFAULT_HOTAREA_COUNT);
}

/**
 * @tc.name: FlushEmptyInfoToMMI
 * @tc.desc: FlushEmptyInfoToMMI
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, FlushEmptyInfoToMMI, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto preEventHandler = SceneInputManager::GetInstance().eventHandler_;
    SceneInputManager::GetInstance().eventHandler_ = nullptr;
    SceneInputManager::GetInstance().FlushEmptyInfoToMMI();
    SceneInputManager::GetInstance().eventHandler_ = preEventHandler;
    SceneInputManager::GetInstance().FlushEmptyInfoToMMI();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("userId") != std::string::npos);
}

/**
 * @tc.name: GetConstrainedModalExtWindowInfo
 * @tc.desc: GetConstrainedModalExtWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, GetConstrainedModalExtWindowInfo, TestSize.Level1)
{
    auto ret = SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(nullptr);
    ASSERT_EQ(ret, std::nullopt);
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ret = SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(sceneSession);
    ASSERT_EQ(ret, std::nullopt);
    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;
    SceneInputManager::GetInstance().sceneSessionDirty_->constrainedModalUIExtInfoMap_.clear();
    ret = SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(sceneSession);
    ASSERT_EQ(ret, std::nullopt);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    surfaceNode->SetId(0);
    sceneSession->surfaceNode_ = surfaceNode;
    std::vector<SecSurfaceInfo> surfaceInfoList;
    SecSurfaceInfo secSurfaceInfo;
    surfaceInfoList.emplace_back(secSurfaceInfo);
    SceneInputManager::GetInstance().sceneSessionDirty_->constrainedModalUIExtInfoMap_[0] = surfaceInfoList;
    ret = SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(sceneSession);
    ASSERT_EQ(ret, std::nullopt);
    sceneSession->uiExtNodeIdToPersistentIdMap_[0] = 1;
    ret = SceneInputManager::GetInstance().GetConstrainedModalExtWindowInfo(sceneSession);
    ASSERT_NE(ret, std::nullopt);
}

/**
 * @tc.name: FlushDisplayInfoToMMI
 * @tc.desc: check func FlushDisplayInfoToMMI
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, FlushDisplayInfoToMMI, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SceneInputManagerTest: FlushDisplayInfoToMMI start";
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    // sceneSessionDirty_ = nullptr
    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    SceneInputManager::GetInstance().isUserBackground_ = false;
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(logMsg.find("sceneSessionDirty_ is nullptr") != std::string::npos);
    logMsg.clear();
    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;

    // NotNeedUpdate
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList), true);
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_FALSE(logMsg.find("sceneSessionDirty_ is nullptr") != std::string::npos);
    logMsg.clear();

    auto preEventHandler = SceneInputManager::GetInstance().eventHandler_;
    SceneInputManager::GetInstance().eventHandler_ = nullptr;
    SceneInputManager::GetInstance().FlushEmptyInfoToMMI();
    SceneInputManager::GetInstance().eventHandler_ = preEventHandler;

    CheckNeedUpdateTest();
    WindowInfoListZeroTest(ssm_);
    MaxWindowInfoTest(ssm_);

    GTEST_LOG_(INFO) << "SceneInputManagerTest: FlushDisplayInfoToMMI end";
}

/**
 * @tc.name: PrintWindowInfo
 * @tc.desc: check func PrintWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, PrintWindowInfo, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SceneInputManagerTest: PrintWindowInfo start";
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<MMI::WindowInfo> windowInfoList;
    SceneInputManager::GetInstance().PrintWindowInfo(windowInfoList);
    EXPECT_EQ(windowInfoList.size(), 0);
    MMI::WindowInfo windowInfo;
    windowInfo.transform = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    windowInfo.defaultHotAreas = {{0, 0, 0, 0}};
    windowInfo.pointerHotAreas = {{0, 0, 0, 0}};
    windowInfo.pointerChangeAreas = {0, 0, 0, 0, 0, 0, 0, 0};
    windowInfoList.emplace_back(windowInfo);
    EXPECT_EQ(windowInfoList.size(), 1);
    SceneInputManager::GetInstance().PrintWindowInfo(windowInfoList);
    EXPECT_TRUE(logMsg.find("LogWinInfo:") != std::string::npos);
    logMsg.clear();

    GTEST_LOG_(INFO) << "SceneInputManagerTest: PrintWindowInfo end";
}

 /**
 * @tc.name: PrintDisplayInfo
 * @tc.desc: check func PrintDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, PrintDisplayInfo, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SceneInputManagerTest: PrintDisplayInfo start";
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<MMI::DisplayInfo> displayInfos;
    SceneInputManager::GetInstance().PrintDisplayInfo(displayInfos);
    EXPECT_EQ(displayInfos.size(), 0);
    MMI::DisplayInfo displayInfo;
    displayInfo.transform = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    displayInfos.emplace_back(displayInfo);
    EXPECT_EQ(displayInfos.size(), 1);
    SceneInputManager::GetInstance().PrintDisplayInfo(displayInfos);
    EXPECT_TRUE(logMsg.find(",list:") != std::string::npos);
    logMsg.clear();

    GTEST_LOG_(INFO) << "SceneInputManagerTest: PrintDisplayInfo end";
}

/**
 * @tc.name: NotifyWindowInfoChange
 * @tc.desc: check func NotifyWindowInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, NotifyWindowInfoChange, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SceneInputManagerTest: NotifyWindowInfoChange start";
    SessionInfo info;
    info.abilityName_ = "NotifyWindowInfoChange";
    info.bundleName_ = "NotifyWindowInfoChange";
    info.appIndex_ = 10;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    // sceneSessionDirty_ = nullptr
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().NotifyWindowInfoChange(sceneSession, WindowUpdateType::WINDOW_UPDATE_ADDED);
    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;

    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    SceneInputManager::GetInstance().NotifyWindowInfoChange(sceneSession, WindowUpdateType::WINDOW_UPDATE_ADDED);
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    GTEST_LOG_(INFO) << "SceneInputManagerTest: NotifyWindowInfoChange end";
}

/**
 * @tc.name: NotifyWindowInfoChangeFromSession
 * @tc.desc: check func NotifyWindowInfoChangeFromSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, NotifyWindowInfoChangeFromSession, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SceneInputManagerTest: NotifyWindowInfoChangeFromSession start";
    SessionInfo info;
    info.abilityName_ = "NotifyWindowInfoChangeFromSession";
    info.bundleName_ = "NotifyWindowInfoChangeFromSession";
    info.appIndex_ = 100;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    // sceneSessionDirty_ = nullptr
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().NotifyWindowInfoChangeFromSession(sceneSession);
    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;

    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    SceneInputManager::GetInstance().NotifyWindowInfoChangeFromSession(sceneSession);
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    GTEST_LOG_(INFO) << "SceneInputManagerTest: NotifyWindowInfoChangeFromSession end";
}

/**
 * @tc.name: NotifyMMIWindowPidChange
 * @tc.desc: check func NotifyMMIWindowPidChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, NotifyMMIWindowPidChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneInputManagerTest: NotifyMMIWindowPidChange start";
    SessionInfo info;
    info.abilityName_ = "NotifyMMIWindowPidChange";
    info.bundleName_ = "NotifyMMIWindowPidChange";
    info.appIndex_ = 1000;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    // sceneSessionDirty_ = nullptr
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().NotifyMMIWindowPidChange(sceneSession, true);
    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;

    SceneInputManager::GetInstance().NotifyMMIWindowPidChange(sceneSession, true);
    EXPECT_TRUE(sceneSession->IsStartMoving());
    SceneInputManager::GetInstance().NotifyMMIWindowPidChange(sceneSession, false);
    EXPECT_FALSE(sceneSession->IsStartMoving());
    SceneInputManager::GetInstance().NotifyMMIWindowPidChange(nullptr, false);
    EXPECT_FALSE(sceneSession->IsStartMoving());
    std::vector<MMI::WindowInfo> windowInfoList;
    std::vector<std::shared_ptr<Media::PixelMap>> pixelMapList;
    SceneInputManager::GetInstance().FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList));
    GTEST_LOG_(INFO) << "SceneInputManagerTest: NotifyMMIWindowPidChange end";
}

/**
 * @tc.name: FlushFullInfoToMMI
 * @tc.desc: FlushFullInfoToMMI
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, FlushFullInfoToMMI, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::map<DisplayGroupId, MMI::DisplayGroupInfo> displayGroupMap;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    SceneInputManager::GetInstance().FlushFullInfoToMMI(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_EQ(displayInfos.size(), 0);
    MMI::DisplayInfo displayInfo;
    displayInfos.emplace_back(displayInfo);
    SceneInputManager::GetInstance().FlushFullInfoToMMI(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_EQ(displayInfos.size(), 1);
    auto oldDirty = SceneInputManager::GetInstance().sceneSessionDirty_;
    SceneInputManager::GetInstance().sceneSessionDirty_ = nullptr;
    SceneInputManager::GetInstance().FlushFullInfoToMMI(screenInfos, displayGroupMap, windowInfoList);
    ASSERT_EQ(windowInfoList.size(), 0);
    SceneInputManager::GetInstance().sceneSessionDirty_ = oldDirty;
}

/**
 * @tc.name: CheckNeedUpdate
 * @tc.desc: CheckNeedUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdate1, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    int32_t focusId = 0;
    Rosen::SceneSessionManager::GetInstance().SetFocusedSessionId(focusId, DEFAULT_DISPLAY_ID);
    SceneInputManager::GetInstance().lastFocusId_ = 1;
    bool ret1 = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(ret1);

    SceneInputManager::GetInstance().lastFocusId_ = 0;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    bool ret2 = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(ret2);

    displayInfos.clear();
    MMI::WindowInfo windowinfo;
    windowInfoList.emplace_back(windowinfo);
    bool ret3 = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(ret3);
}

/**
 * @tc.name: CheckNeedUpdate
 * @tc.desc: CheckNeedUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdate2, TestSize.Level0)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    MMI::WindowInfo windowinfo;
    int32_t focusId = 0;
    Rosen::SceneSessionManager::GetInstance().SetFocusedSessionId(focusId, DEFAULT_DISPLAY_ID);
    SceneInputManager::GetInstance().lastFocusId_ = 0;
    SceneInputManager::GetInstance().lastDisplayInfos_ = displayInfos;
    SceneInputManager::GetInstance().lastWindowInfoList_ = windowInfoList;
    bool result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_FALSE(result);
    windowInfoList.emplace_back(windowinfo);
    SceneInputManager::GetInstance().lastWindowInfoList_ = windowInfoList;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_FALSE(result);

    displayInfos[0].id = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    displayInfos[0].id = 0;

    windowInfoList[0].id = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].id = 0;

    windowInfoList[0].pid = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].pid = 0;

    windowInfoList[0].uid = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].uid = 0;

    windowInfoList[0].agentWindowId = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].agentWindowId = 0;

    windowInfoList[0].flags = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].flags = 0;

    windowInfoList[0].displayId = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].displayId = 0;

    windowInfoList[0].zOrder = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].zOrder = 0;
}

/**
 * @tc.name: CheckNeedUpdate
 * @tc.desc: CheckNeedUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdate3, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    MMI::WindowInfo windowinfo;
    windowInfoList.emplace_back(windowinfo);
    int32_t focusId = 0;
    Rosen::SceneSessionManager::GetInstance().SetFocusedSessionId(focusId, DEFAULT_DISPLAY_ID);
    SceneInputManager::GetInstance().lastFocusId_ = 0;
    SceneInputManager::GetInstance().lastDisplayInfos_ = displayInfos;
    SceneInputManager::GetInstance().lastWindowInfoList_ = windowInfoList;
    bool result = false;
    windowInfoList[0].area.x = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].area.x = 0;

    windowInfoList[0].area.y = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].area.y = 0;

    windowInfoList[0].area.width = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].area.width = 0;

    windowInfoList[0].area.height = 1;
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].area.height = 0;

    MMI::Rect area;
    windowInfoList[0].defaultHotAreas.emplace_back(area);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].defaultHotAreas.clear();

    windowInfoList[0].pointerHotAreas.emplace_back(area);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].pointerHotAreas.clear();

    windowInfoList[0].transform.emplace_back(1.0);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].transform.clear();
}

/**
 * @tc.name: CheckNeedUpdate
 * @tc.desc: CheckNeedUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdate4, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayinfo;
    displayInfos.emplace_back(displayinfo);
    MMI::WindowInfo windowinfo;
    windowInfoList.emplace_back(windowinfo);
    int32_t focusId = 0;
    Rosen::SceneSessionManager::GetInstance().SetFocusedSessionId(focusId, DEFAULT_DISPLAY_ID);
    SceneInputManager::GetInstance().lastFocusId_ = 0;
    SceneInputManager::GetInstance().lastDisplayInfos_ = displayInfos;
    SceneInputManager::GetInstance().lastWindowInfoList_ = windowInfoList;
    bool result = false;
    windowInfoList[0].transform.emplace_back(1.0);
    SceneInputManager::GetInstance().lastWindowInfoList_[0].transform.emplace_back(2.0);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].transform.clear();
    SceneInputManager::GetInstance().lastWindowInfoList_[0].transform.clear();

    MMI::Rect area;
    windowInfoList[0].defaultHotAreas.emplace_back(area);
    area.x = 1;
    SceneInputManager::GetInstance().lastWindowInfoList_[0].defaultHotAreas.emplace_back(area);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].defaultHotAreas.clear();
    SceneInputManager::GetInstance().lastWindowInfoList_[0].defaultHotAreas.clear();
    area.x = 0;

    windowInfoList[0].pointerHotAreas.emplace_back(area);
    area.x = 1;
    SceneInputManager::GetInstance().lastWindowInfoList_[0].pointerHotAreas.emplace_back(area);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].pointerHotAreas.clear();
    SceneInputManager::GetInstance().lastWindowInfoList_[0].pointerHotAreas.clear();
    area.x = 0;

    windowInfoList[0].pointerChangeAreas.emplace_back(1);
    SceneInputManager::GetInstance().lastWindowInfoList_[0].pointerChangeAreas.emplace_back(2);
    result = SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList);
    ASSERT_TRUE(result);
    windowInfoList[0].pointerChangeAreas.clear();
    SceneInputManager::GetInstance().lastWindowInfoList_[0].pointerChangeAreas.clear();
}

/**
 * @tc.name: ConstructScreenInfos
 * @tc.desc: ConstructScreenInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, ConstructScreenInfos, TestSize.Level1)
{
    std::map<ScreenId, ScreenProperty> screensProperties;
    std::vector<MMI::ScreenInfo> screenInfos = SceneInputManager::GetInstance().ConstructScreenInfos(screensProperties);
    EXPECT_EQ(screenInfos.size(), 0);
    ScreenProperty screenProperty;
    screensProperties[0] = screenProperty;
    screenInfos = SceneInputManager::GetInstance().ConstructScreenInfos(screensProperties);
    EXPECT_EQ(screenInfos.size(), 1);
}

/**
 * @tc.name: ConstructDisplayGroupInfos
 * @tc.desc: ConstructDisplayGroupInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, ConstructDisplayGroupInfos, TestSize.Level1)
{
    std::map<ScreenId, ScreenProperty> screensProperties;
    std::map<DisplayGroupId, MMI::DisplayGroupInfo> displayGroupMap;
    SceneInputManager::GetInstance().ConstructDisplayGroupInfos(screensProperties, displayGroupMap);
    EXPECT_EQ(displayGroupMap.size(), 0);
    ScreenProperty screenProperty0;
    screenProperty0.SetValidHeight(10);
    Rosen::ScreenSessionManagerClient::GetInstance().screenSessionMap_[0] =
        sptr<ScreenSession>::MakeSptr(0, screenProperty0, 0);
    screensProperties[0] = screenProperty0;
    SceneInputManager::GetInstance().ConstructDisplayGroupInfos(screensProperties, displayGroupMap);
    EXPECT_EQ(displayGroupMap.size(), 1);
    screensProperties[1] = screenProperty0;
    SceneInputManager::GetInstance().ConstructDisplayGroupInfos(screensProperties, displayGroupMap);
    EXPECT_EQ(displayGroupMap.size(), 1);
}

/**
 * @tc.name: CheckNeedUpdateForScreenInfos
 * @tc.desc: CheckNeedUpdateForScreenInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdateForScreenInfos, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::ScreenInfo screenInfo;
    screenInfo.id = 1;
    screenInfo.uniqueId = "test01";
    screenInfo.screenType = MMI::ScreenType::REAL;
    screenInfo.width = 1;
    screenInfo.height = 1;
    screenInfo.physicalWidth = 1;
    screenInfo.physicalHeight = 1;
    screenInfo.tpDirection = MMI::Direction::DIRECTION90;
    screenInfo.dpi = 1;
    screenInfo.ppi = 1;
    screenInfo.rotation = MMI::Rotation::ROTATION_0;
    screenInfos.emplace_back(screenInfo);
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    ASSERT_FALSE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].id = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].uniqueId = "test02";
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].screenType = MMI::ScreenType::VIRTUAL;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].width = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].height = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].physicalWidth = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].physicalHeight = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].tpDirection = MMI::Direction::DIRECTION270;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].dpi = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].ppi = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    screenInfos[0].rotation = MMI::Rotation::ROTATION_90;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    ASSERT_FALSE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
}

/**
 * @tc.name: CheckNeedUpdateForDisplayInfos
 * @tc.desc: CheckNeedUpdateForDisplayInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, CheckNeedUpdateForDisplayInfos, TestSize.Level1)
{
    std::vector<MMI::ScreenInfo> screenInfos;
    std::vector<MMI::DisplayInfo> displayInfos;
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::DisplayInfo displayInfo;
    displayInfo.id = 1;
    displayInfo.x = 1;
    displayInfo.y = 1;
    displayInfo.width = 1;
    displayInfo.height = 1;
    displayInfo.dpi = 1;
    displayInfo.name = "displayInfo01";
    displayInfo.direction = MMI::Direction::DIRECTION90;
    displayInfo.displayDirection = MMI::Direction::DIRECTION90;
    displayInfo.displayMode = MMI::DisplayMode::FULL;
    displayInfo.transform.emplace_back(0.001);
    displayInfo.scalePercent = 100;
    displayInfo.expandHeight = 1;
    displayInfo.isCurrentOffScreenRendering = true;
    displayInfo.displaySourceMode = MMI::DisplaySourceMode::SCREEN_MAIN;
    displayInfo.oneHandX = 1;
    displayInfo.oneHandY = 1;
    displayInfo.screenArea = { 1, { 1, 1, 1, 1 } };
    displayInfo.rsId = 1;
    displayInfo.offsetX = 1;
    displayInfo.offsetY = 1;
    displayInfo.pointerActiveWidth = 1;
    displayInfo.pointerActiveHeight = 1;
    displayInfo.deviceRotation = MMI::Direction::DIRECTION90;
    displayInfo.rotationCorrection = MMI::Direction::DIRECTION90;
    displayInfos.emplace_back(displayInfo);
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    ASSERT_FALSE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].id = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].x = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].y = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].width = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].height = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].dpi = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].name = "displayInfo02";
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].direction = MMI::Direction::DIRECTION180;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].displayDirection = MMI::Direction::DIRECTION180;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].displayMode = MMI::DisplayMode::SUB;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].transform[0] = 0.002;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].transform.emplace_back(0.00002);
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].scalePercent = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].expandHeight = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].isCurrentOffScreenRendering = false;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].displaySourceMode = MMI::DisplaySourceMode::SCREEN_EXTEND;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].oneHandX = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].oneHandY = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].rsId = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].offsetX = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].offsetY = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].pointerActiveWidth = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].pointerActiveHeight = 2;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].screenArea = { 2, { 1, 1, 1, 1 } };
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].screenArea = { 2, { 2, 1, 1, 1 } };
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].screenArea = { 2, { 2, 2, 1, 1 } };
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].screenArea = { 2, { 2, 2, 2, 1 } };
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].screenArea = { 2, { 2, 2, 2, 2 } };
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].deviceRotation = MMI::Direction::DIRECTION180;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    displayInfos[0].rotationCorrection = MMI::Direction::DIRECTION180;
    ASSERT_TRUE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
    ASSERT_FALSE(SceneInputManager::GetInstance().CheckNeedUpdate(screenInfos, displayInfos, windowInfoList));
}

/**
 * @tc.name: UpdateHotAreas
 * @tc.desc: UpdateHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneInputManagerTest, UpdateHotAreas, TestSize.Level1)
{
    std::vector<MMI::Rect> touchHotAreas(0);
    std::vector<MMI::Rect> pointerHotAreas(0);
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    std::vector<OHOS::Rosen::Rect> touchHotAreasInSceneSession(0);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    sceneSession->GetSessionProperty()->type_ = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    touchHotAreas.clear();
    pointerHotAreas.clear();
    sceneSession->GetSessionProperty()->SetDisplayId(-100);
    SceneInputManager::GetInstance().UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    ASSERT_EQ(touchHotAreas.size(), 1);
}
} // namespace
} // namespace Rosen
} // namespace OHOS