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
#include <regex>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>

#include "context.h"
#include "interfaces/include/ws_common.h"
#include "session_info.h"
#include "session/host/include/scene_persistence.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session_manager/include/rdb/starting_window_rdb_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string TEST_RDB_PATH = "/data/test/";
const std::string TEST_INVALID_PATH = "";
const std::string TEST_RDB_NAME = "starting_window_config_test.db";
const uint32_t WAIT_SLEEP_TIME = 200000;
std::string g_logMsg;
void RdbLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    g_logMsg = msg;
}
} // namespace

class WindowPatternStartingWindowTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    void InitTestStartingWindowRdb();
    void CreateSession(SessionInfo sessionInfo, int32_t persistentId);
};

sptr<SceneSessionManager> WindowPatternStartingWindowTest::ssm_ = nullptr;
static sptr<ScenePersistence> scenePersistence =
    sptr<ScenePersistence>::MakeSptr("WindowPatternStartingWindowTest", 1423);

void WindowPatternStartingWindowTest::SetUpTestCase()
{
    ssm_ = new SceneSessionManager();
    ssm_->Init();
    if (ssm_ != nullptr) {
        ssm_->ClearStartWindowPersistencePath("testBundle");
        ssm_->ClearStartWindowPersistencePath("emptyBundle");
    }
}

void WindowPatternStartingWindowTest::TearDownTestCase()
{
    if (ssm_ != nullptr) {
        ssm_->ClearStartWindowPersistencePath("testBundle");
        ssm_->ClearStartWindowPersistencePath("emptyBundle");
    }
    ssm_ = nullptr;
    NativeRdb::RdbHelper::DeleteRdbStore(TEST_RDB_PATH + TEST_RDB_NAME);
}

void WindowPatternStartingWindowTest::SetUp()
{
}

void WindowPatternStartingWindowTest::TearDown()
{
}

void WindowPatternStartingWindowTest::InitTestStartingWindowRdb()
{
    NativeRdb::RdbHelper::DeleteRdbStore(TEST_RDB_PATH + TEST_RDB_NAME);
    WmsRdbConfig config;
    config.dbName = TEST_RDB_NAME;
    config.dbPath = TEST_RDB_PATH;
    ssm_->startingWindowRdbMgr_ = std::make_shared<StartingWindowRdbManager>(config);
}

void WindowPatternStartingWindowTest::CreateSession(SessionInfo sessionInfo, int32_t persistentId)
{
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_NE(ssm_->GetSceneSession(persistentId), nullptr);
}

namespace {
/**
 * @tc.name: CreateStartWindowDir
 * @tc.desc: test function : CreateStartWindowDir
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, CreateStartWindowDir, TestSize.Level1)
{
    std::string directory = "0/Storage";
    ASSERT_NE(nullptr, scenePersistence);
    bool result = scenePersistence->CreateStartWindowDir(directory);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: SetAndGetStartWindowPersistencePath01
 * @tc.desc: Test setting and getting the path corresponding to an existing bundleName and saveStartWindowKey
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, SetAndGetStartWindowPersistencePath01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    const std::string bundleName = "testBundle";
    const std::string darkKey = "DarkStartWindow";
    const std::string lightKey = "LightStartWindow";
    const std::string darkPath = "/data/testBundle/dark_start_window.png";
    const std::string lightPath = "/data/testBundle/light_start_window.png";

    ssm_->SetStartWindowPersistencePath(bundleName, darkKey, darkPath);
    std::string retDarkPath = ssm_->GetStartWindowPersistencePath(bundleName, darkKey);
    ASSERT_EQ(retDarkPath, darkPath);

    ssm_->SetStartWindowPersistencePath(bundleName, lightKey, lightPath);
    std::string retLightPath = ssm_->GetStartWindowPersistencePath(bundleName, lightKey);
    ASSERT_EQ(retLightPath, lightPath);

    const std::string emptyPath = "";
    ssm_->SetStartWindowPersistencePath(bundleName, darkKey, emptyPath);
    std::string retEmptyPath = ssm_->GetStartWindowPersistencePath(bundleName, darkKey);
    ASSERT_EQ(retEmptyPath, emptyPath);

    const std::string customKey = "CustomStartWindow";
    const std::string customPath = "/data/testBundle/custom_start_window.png";
    ssm_->SetStartWindowPersistencePath(bundleName, customKey, customPath);
    std::string retCustomPath = ssm_->GetStartWindowPersistencePath(bundleName, customKey);
    ASSERT_EQ(retCustomPath, customPath);
}

/**
 * @tc.name: GetStartWindowPersistencePath02
 * @tc.desc: Test getting the path for non-existent bundleName or saveStartWindowKey (boundary scenario)
 * @tc.type: FUNC
 * @tc.level: Level2
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartWindowPersistencePath02, TestSize.Level2)
{
    ASSERT_NE(ssm_, nullptr);

    const std::string nonExistBundle = "nonExistBundle";
    const std::string testKey = "DarkStartWindow";
    std::string retPath = ssm_->GetStartWindowPersistencePath(nonExistBundle, testKey);
    ASSERT_EQ(retPath, "");

    const std::string existBundle = "emptyBundle";
    const std::string validKey = "LightStartWindow";
    const std::string invalidKey = "InvalidStartWindow";
    const std::string lightPath = "/data/emptyBundle/light.png";
    ssm_->SetStartWindowPersistencePath(existBundle, validKey, lightPath);
    retPath = ssm_->GetStartWindowPersistencePath(existBundle, invalidKey);
    ASSERT_EQ(retPath, "");

    const std::string emptyBundle = "";
    const std::string emptyKey = "";
    retPath = ssm_->GetStartWindowPersistencePath(emptyBundle, emptyKey);
    ASSERT_EQ(retPath, "");

    ssm_->SetStartWindowPersistencePath(existBundle, emptyKey, "/data/emptyBundle/empty_key.png");
    retPath = ssm_->GetStartWindowPersistencePath(existBundle, emptyKey);
    ASSERT_EQ(retPath, "/data/emptyBundle/empty_key.png");
}

/**
 * @tc.name: ClearStartWindowPersistencePath01
 * @tc.desc: Test clearing all path configurations for the specified bundleName
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, ClearStartWindowPersistencePath01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    const std::string bundleName = "testBundle";
    const std::string darkKey = "DarkStartWindow";
    const std::string lightKey = "LightStartWindow";
    const std::string darkPath = "/data/testBundle/dark.png";
    const std::string lightPath = "/data/testBundle/light.png";

    ssm_->SetStartWindowPersistencePath(bundleName, darkKey, darkPath);
    ssm_->SetStartWindowPersistencePath(bundleName, lightKey, lightPath);

    ASSERT_EQ(ssm_->GetStartWindowPersistencePath(bundleName, darkKey), darkPath);
    ASSERT_EQ(ssm_->GetStartWindowPersistencePath(bundleName, lightKey), lightPath);

    ssm_->ClearStartWindowPersistencePath(bundleName);

    ASSERT_EQ(ssm_->GetStartWindowPersistencePath(bundleName, darkKey), "");
    ASSERT_EQ(ssm_->GetStartWindowPersistencePath(bundleName, lightKey), "");
}

/**
 * @tc.name: GetStartupPage01
 * @tc.desc: GetStartupPage from want
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartupPage01, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    StartingWindowInfo startingWindowInfo;
    startingWindowInfo.iconPathEarlyVersion_ = "default";
    ssm_->GetStartupPage(sessionInfo, startingWindowInfo);
    sptr<AppExecFwk::IBundleMgr> tempBundleMgr = ssm_->bundleMgr_;
    ssm_->bundleMgr_ = nullptr;
    ssm_->GetStartupPage(sessionInfo, startingWindowInfo);
    ssm_->bundleMgr_ = tempBundleMgr;
    sessionInfo.want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(sessionInfo.want, nullptr);
    const std::string pathFromDesk = "pathFromDesk";
    sessionInfo.want->SetParam("realAppIcon", pathFromDesk);
    ssm_->GetStartupPage(sessionInfo, startingWindowInfo);
    ASSERT_EQ(startingWindowInfo.iconPathEarlyVersion_, "pathFromDesk");
}

/**
 * @tc.name: GetStartupPage02
 * @tc.desc: GetStartupPage from cache
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartupPage02, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "moduleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.bundleName_ = "bundleName";
    StartingWindowInfo cachedInfo;
    cachedInfo.iconPathEarlyVersion_ = "pathFromCache";
    ssm_->startingWindowMap_.clear();
    bool isDark = false;
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_ + std::to_string(isDark);
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap { { key, cachedInfo } };
    ssm_->startingWindowMap_.insert({ sessionInfo.bundleName_, startingWindowInfoMap });
    StartingWindowInfo outInfo;
    outInfo.iconPathEarlyVersion_ = "default";
    ssm_->GetStartupPage(sessionInfo, outInfo);
    ASSERT_EQ(outInfo.iconPathEarlyVersion_, "pathFromCache");
}

/**
 * @tc.name: GetStartupPage03
 * @tc.desc: GetStartupPage from rdb
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartupPage03, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    InitTestStartingWindowRdb();
    ASSERT_NE(ssm_->startingWindowRdbMgr_, nullptr);
    ssm_->startingWindowMap_.clear();
    SessionInfo sessionInfo;
    StartingWindowRdbItemKey itemKey;
    sessionInfo.moduleName_ = itemKey.moduleName = "moduleName";
    sessionInfo.abilityName_ = itemKey.abilityName = "abilityName";
    sessionInfo.bundleName_ = itemKey.bundleName = "bundleName";
    itemKey.darkMode = false;
    StartingWindowInfo rdbInfo;
    rdbInfo.iconPathEarlyVersion_ = "pathFromRdb";
    StartingWindowInfo outInfo;
    outInfo.iconPathEarlyVersion_ = "default";
    bool insertRes = ssm_->startingWindowRdbMgr_->InsertData(itemKey, rdbInfo);
    EXPECT_EQ(insertRes, true);
    ssm_->GetStartupPage(sessionInfo, outInfo);
    EXPECT_EQ(outInfo.iconPathEarlyVersion_, "pathFromRdb");
}

/**
 * @tc.name: GetStartingWindowInfoFromCache
 * @tc.desc: GetStartingWindowInfoFromCache
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartingWindowInfoFromCache, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "moduleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.bundleName_ = "bundleName";
    StartingWindowInfo startingWindowInfo;
    bool isDark = false;
    auto res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo, isDark);
    EXPECT_EQ(res, false);
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_ + std::to_string(isDark);
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{ { key, startingWindowInfo } };
    ssm_->startingWindowMap_.insert({ sessionInfo.bundleName_, startingWindowInfoMap });
    res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo, isDark);
    EXPECT_EQ(res, true);
    sessionInfo.moduleName_ = "invalidModuleName";
    res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo, isDark);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: GetStartingWindowInfoFromRdb
 * @tc.desc: GetStartingWindowInfoFromRdb
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartingWindowInfoFromRdb, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    InitTestStartingWindowRdb();
    ASSERT_NE(ssm_->startingWindowRdbMgr_, nullptr);
    NativeRdb::RdbHelper::DeleteRdbStore(TEST_RDB_PATH + TEST_RDB_NAME);
    SessionInfo sessionInfo;
    StartingWindowRdbItemKey itemKey;
    StartingWindowInfo outInfo;
    sessionInfo.moduleName_ = itemKey.moduleName = "moduleName";
    sessionInfo.abilityName_ = itemKey.abilityName = "abilityName";
    sessionInfo.bundleName_ = itemKey.bundleName = "bundleName";
    itemKey.darkMode = false;
    bool isDark = false;
    auto res = ssm_->GetStartingWindowInfoFromRdb(sessionInfo, outInfo, isDark);
    EXPECT_EQ(res, false);
    bool insertRes = ssm_->startingWindowRdbMgr_->InsertData(itemKey, outInfo);
    EXPECT_EQ(insertRes, true);
    res = ssm_->GetStartingWindowInfoFromRdb(sessionInfo, outInfo, isDark);
    EXPECT_EQ(res, true);
    ssm_->startingWindowRdbMgr_ = nullptr;
    res = ssm_->GetStartingWindowInfoFromRdb(sessionInfo, outInfo, isDark);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: GetIconFromDesk
 * @tc.desc: GetIconFromDesk
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetIconFromDesk, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    std::string outPath;
    bool res = ssm_->GetIconFromDesk(sessionInfo, outPath);
    ASSERT_EQ(res, false);
    sessionInfo.want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(sessionInfo.want, nullptr);
    res = ssm_->GetIconFromDesk(sessionInfo, outPath);
    ASSERT_EQ(res, false);
    const std::string pathFromDesk = "pathFromDesk";
    sessionInfo.want->SetParam("realAppIcon", pathFromDesk);
    res = ssm_->GetIconFromDesk(sessionInfo, outPath);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: GetStartupPageFromResource
 * @tc.desc: GetStartupPageFromResource
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartupPageFromResource, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    AppExecFwk::AbilityInfo info;
    info.startWindowBackgroundId = 1;
    StartingWindowInfo outInfo;
    ASSERT_EQ(false, ssm_->GetStartupPageFromResource(info, outInfo));
    info.startWindowIconId = 1;
    ASSERT_EQ(false, ssm_->GetStartupPageFromResource(info, outInfo));
    info.hapPath = "hapPath";
    ASSERT_EQ(false, ssm_->GetStartupPageFromResource(info, outInfo));
}

/**
 * @tc.name: CacheStartingWindowInfo01
 * @tc.desc: Cache new starting window info
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, CacheStartingWindowInfo01, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    /**
     * @tc.steps: step1. Build input parameter.
     */
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "abilityName";
    abilityInfo.bundleName = "bundleName";
    abilityInfo.moduleName = "moduleName";
    StartingWindowInfo startingWindowInfo;
    startingWindowInfo.backgroundColorEarlyVersion_ = 0xff000000;
    startingWindowInfo.iconPathEarlyVersion_ = "cachedPath";

    /**
     * @tc.steps: step2. Cache info and check result.
     */
    bool isDark = false;
    ssm_->CacheStartingWindowInfo(
        abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name, startingWindowInfo, isDark);
    auto iter = ssm_->startingWindowMap_.find(abilityInfo.bundleName);
    ASSERT_NE(iter, ssm_->startingWindowMap_.end());
    auto& infoMap = iter->second;
    auto infoIter = infoMap.find(abilityInfo.moduleName + abilityInfo.name + std::to_string(isDark));
    ASSERT_NE(infoIter, infoMap.end());
    EXPECT_EQ(infoIter->second.backgroundColorEarlyVersion_, 0xff000000);
    EXPECT_EQ(infoIter->second.iconPathEarlyVersion_, "cachedPath");
}

/**
 * @tc.name: CacheStartingWindowInfo02
 * @tc.desc: Execute when info is cached
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, CacheStartingWindowInfo02, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    /**
     * @tc.steps: step1. Build input parameter.
     */
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "abilityName";
    abilityInfo.bundleName = "bundleName";
    abilityInfo.moduleName = "moduleName";
    StartingWindowInfo startingWindowInfo;
    startingWindowInfo.backgroundColorEarlyVersion_ = 0xff000000;
    startingWindowInfo.iconPathEarlyVersion_ = "cachedPath";

    /**
     * @tc.steps: step2. Insert one item.
     */
    auto key = abilityInfo.moduleName + abilityInfo.name;
    StartingWindowInfo anotherStartingWindowInfo = {
        .backgroundColorEarlyVersion_ = 0x00000000,
        .iconPathEarlyVersion_ = "path",
    };
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{{ key, anotherStartingWindowInfo }};
    ssm_->startingWindowMap_.insert({abilityInfo.bundleName, startingWindowInfoMap});

    /**
     * @tc.steps: step3. Execute and check result.
     */
    bool isDark = false;
    ssm_->CacheStartingWindowInfo(
        abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name, startingWindowInfo, isDark);
    auto iter = ssm_->startingWindowMap_.find(abilityInfo.bundleName);
    ASSERT_NE(iter, ssm_->startingWindowMap_.end());
    auto& infoMap = iter->second;
    auto infoIter = infoMap.find(abilityInfo.moduleName + abilityInfo.name + std::to_string(isDark));
    ASSERT_NE(infoIter, infoMap.end());
    EXPECT_EQ(infoIter->second.backgroundColorEarlyVersion_, 0xff000000);
    EXPECT_EQ(infoIter->second.iconPathEarlyVersion_, "cachedPath");
}

/**
 * @tc.name: CacheStartingWindowInfo03
 * @tc.desc: Execute when cache is full
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, CacheStartingWindowInfo03, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    bool isDark = false;
    for (int index = 0; index < 100; index++) {
        AppExecFwk::AbilityInfo tempAbilityInfo;
        tempAbilityInfo.name = "abilityName";
        tempAbilityInfo.bundleName = "bundleName" + std::to_string(index);
        tempAbilityInfo.moduleName = "moduleName";
        StartingWindowInfo tempStartingWindowInfo;
        tempStartingWindowInfo.backgroundColorEarlyVersion_ = 0xffffffff;
        tempStartingWindowInfo.iconPathEarlyVersion_ = "otherPath";
        auto key = tempAbilityInfo.moduleName + tempAbilityInfo.name + std::to_string(isDark);
        std::map<std::string, StartingWindowInfo> startingWindowInfoMap{{ key, tempStartingWindowInfo }};
        ssm_->startingWindowMap_.insert({tempAbilityInfo.bundleName, startingWindowInfoMap});
    }

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "bundleName";
    StartingWindowInfo startingWindowInfo;
    startingWindowInfo.backgroundColorEarlyVersion_ = 0xff000000;
    startingWindowInfo.iconPathEarlyVersion_ = "cachedPath";
    auto key = abilityInfo.moduleName + abilityInfo.name + std::to_string(isDark);
    ssm_->CacheStartingWindowInfo(
        abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name, startingWindowInfo, isDark);
    // first item should be removed
    auto removedIter = ssm_->startingWindowMap_.find("bundleName0");
    ASSERT_EQ(removedIter, ssm_->startingWindowMap_.end());
    ASSERT_EQ(ssm_->startingWindowMap_.size(), 100);
    // check insert result
    auto iter = ssm_->startingWindowMap_.find(abilityInfo.bundleName);
    ASSERT_NE(iter, ssm_->startingWindowMap_.end());
    auto& infoMap = iter->second;
    auto infoIter = infoMap.find(abilityInfo.moduleName + abilityInfo.name + std::to_string(isDark));
    ASSERT_NE(infoIter, infoMap.end());
    EXPECT_EQ(infoIter->second.backgroundColorEarlyVersion_, 0xff000000);
    EXPECT_EQ(infoIter->second.iconPathEarlyVersion_, "cachedPath");
}

/**
 * @tc.name: GetPathInfoFromResource
 * @tc.desc: GetPathInfoFromResource
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPathInfoFromResource, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    uint32_t resourceId = 0;
    std::string path;
    bool res = ssm_->GetPathInfoFromResource(nullptr, true, resourceId, path);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: GetBundleStartingWindowInfos
 * @tc.desc: GetBundleStartingWindowInfos
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetBundleStartingWindowInfos, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    AppExecFwk::BundleInfo bundleInfo;
    std::vector<std::pair<StartingWindowRdbItemKey, StartingWindowInfo>> outValues;
    ssm_->GetBundleStartingWindowInfos(false, bundleInfo, outValues);
    EXPECT_EQ(outValues.size(), 0);
}

/**
 * @tc.name: GetPreloadStartingWindow_WithoutAnyData
 * @tc.desc: Test GetPreloadStartingWindow when neither PixelMap nor SVG buffer is set
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPreloadStartingWindow_WithoutAnyData, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    
    SessionInfo sessionInfo;
 
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> bufferInfo;
    pixelMap = nullptr;
    bufferInfo = {nullptr, 0};
    
    sceneSession->GetPreloadStartingWindow(pixelMap, bufferInfo);
    EXPECT_EQ(pixelMap, nullptr);
    EXPECT_EQ(bufferInfo.first, nullptr);
    EXPECT_EQ(bufferInfo.second, 0);
}
 
/**
 * @tc.name: GetPreloadStartingWindow_WithValidPixelMap
 * @tc.desc: Test GetPreloadStartingWindow when valid PixelMap is set (SVG buffer remains unchanged)
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPreloadStartingWindow_WithValidPixelMap, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    
    sceneSession->ResetPreloadStartingWindow();
    auto validPixelMap = std::make_shared<Media::PixelMap>();
    sceneSession->SetPreloadStartingWindow(validPixelMap);
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> bufferInfo;
    pixelMap = nullptr;
    bufferInfo = {nullptr, 5};
    sceneSession->GetPreloadStartingWindow(pixelMap, bufferInfo);
    EXPECT_EQ(pixelMap, validPixelMap);
    EXPECT_EQ(bufferInfo.second, 0);
}
 
/**
 * @tc.name: GetPreloadStartingWindow_WithValidSvgBuffer
 * @tc.desc: Test GetPreloadStartingWindow when valid SVG buffer is set (PixelMap remains null)
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPreloadStartingWindow_WithValidSvgBuffer, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    
    sceneSession->ResetPreloadStartingWindow();
    auto svgBufferVec = std::make_shared<std::vector<uint8_t>>(10);
    std::shared_ptr<uint8_t[]> validSvgBuffer(svgBufferVec->data(), [](uint8_t*) {});
    std::pair<std::shared_ptr<uint8_t[]>, size_t> validBufferInfo = {validSvgBuffer, 10};
    sceneSession->SetPreloadStartingWindow(validBufferInfo);
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> bufferInfo;
    pixelMap = nullptr;
    bufferInfo = {nullptr, 0};
    
    sceneSession->GetPreloadStartingWindow(pixelMap, bufferInfo);
    EXPECT_EQ(pixelMap, nullptr);
    EXPECT_EQ(bufferInfo.first, validBufferInfo.first);
    EXPECT_EQ(bufferInfo.second, validBufferInfo.second);
}
 
/**
 * @tc.name: GetPreloadStartingWindow_WithInvalidSvgBuffer_SizeZero
 * @tc.desc: Test GetPreloadStartingWindow when invalid SVG buffer (size 0) is set
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPreloadStartingWindow_WithInvalidSvgBuffer_SizeZero, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    
    sceneSession->ResetPreloadStartingWindow();
    auto svgBufferVec = std::make_shared<std::vector<uint8_t>>(10);
    std::shared_ptr<uint8_t[]> invalidSvgBuffer(svgBufferVec->data(), [](uint8_t*) {});
    std::pair<std::shared_ptr<uint8_t[]>, size_t> invalidBufferInfo = {invalidSvgBuffer, 0};
    sceneSession->SetPreloadStartingWindow(invalidBufferInfo);
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> bufferInfo;
    pixelMap = nullptr;
    bufferInfo = {nullptr, 0};
    
    sceneSession->GetPreloadStartingWindow(pixelMap, bufferInfo);
    EXPECT_EQ(pixelMap, nullptr);
    EXPECT_EQ(bufferInfo.first, nullptr);
    EXPECT_EQ(bufferInfo.second, 0);
}
 
/**
 * @tc.name: GetPreloadStartingWindow_WithInvalidSvgBuffer_Nullptr
 * @tc.desc: Test GetPreloadStartingWindow when invalid SVG buffer is nullptr
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPreloadStartingWindow_WithInvalidSvgBuffer_Nullptr, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    
    sceneSession->ResetPreloadStartingWindow();
    std::shared_ptr<uint8_t[]> invalidSvgBuffer = nullptr;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> invalidBufferInfo = {invalidSvgBuffer, 5};
    sceneSession->SetPreloadStartingWindow(invalidBufferInfo);
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> bufferInfo;
    pixelMap = nullptr;
    bufferInfo = {nullptr, 0};
    
    sceneSession->GetPreloadStartingWindow(pixelMap, bufferInfo);
    EXPECT_EQ(pixelMap, nullptr);
    EXPECT_EQ(bufferInfo.first, nullptr);
    EXPECT_EQ(bufferInfo.second, 0);
}
 
/**
 * @tc.name: GetPreloadStartingWindow_WithInvalidPixelMap_Nullptr
 * @tc.desc: Test GetPreloadStartingWindow when invalid PixelMap (nullptr) is set
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(WindowPatternStartingWindowTest, GetPreloadStartingWindow_WithInvalidPixelMap_Nullptr, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    
    sceneSession->ResetPreloadStartingWindow();
    std::shared_ptr<Media::PixelMap> invalidPixelMap = nullptr;
    sceneSession->SetPreloadStartingWindow(invalidPixelMap);
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> bufferInfo;
    pixelMap = nullptr;
    bufferInfo = {nullptr, 0};
    
    sceneSession->GetPreloadStartingWindow(pixelMap, bufferInfo);
    EXPECT_EQ(pixelMap, nullptr);
    EXPECT_EQ(bufferInfo.first, nullptr);
    EXPECT_EQ(bufferInfo.second, 0);
}

/**
 * @tc.name: EraseSceneSessionMapById
 * @tc.desc: release preload starting window when EraseSceneSessionMapById
 * @tc.type: FUNC
 */
HETEST_F(WindowPatternStartingWindowTest, EraseSceneSessionMapById, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    std::string key = sessionInfo.bundleName_ + '_' + sessionInfo.moduleName_ + '_' + sessionInfo.abilityName_;
    ssm_->EraseSceneSessionMapById(0);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    ASSERT_NE(nullptr, ssm_->GetSceneSession(sceneSession->GetPersistentId()));
    ssm_->EraseSceneSessionMapById(sceneSession->GetPersistentId());
    ASSERT_EQ(nullptr, ssm_->GetSceneSession(sceneSession->GetPersistentId()));
}

/**
 * @tc.name: CheckAndGetPreLoadResourceId
 * @tc.desc: CheckAndGetPreLoadResourceId
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, CheckAndGetPreLoadResourceId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    StartingWindowInfo startingWindowInfo;
    uint32_t resId = 0;
    bool isSvg = false;
    startingWindowInfo.configFileEnabled_ = true;
    EXPECT_EQ(false, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(false, isSvg);
    startingWindowInfo.configFileEnabled_ = false;
    EXPECT_EQ(false, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///12345678.svg";
    EXPECT_EQ(true, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(true, isSvg);
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///abc12345678.jpg";
    EXPECT_EQ(false, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(false, isSvg);
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///12345678.png";
    EXPECT_EQ(true, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(false, isSvg);
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///12345678.jpg";
    EXPECT_EQ(true, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(false, isSvg);
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///12345678.webp";
    EXPECT_EQ(true, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(false, isSvg);
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///12345678.astc";
    EXPECT_EQ(true, ssm_->CheckAndGetPreLoadResourceId(startingWindowInfo, resId, isSvg));
    EXPECT_EQ(false, isSvg);
}

/**
 * @tc.name: PreLoadStartingWindow
 * @tc.desc: PreLoadStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, PreLoadStartingWindow, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->systemConfig_.supportPreloadStartingWindow_ = false;
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    ssm_->systemConfig_.supportPreloadStartingWindow_ = true;
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    SessionInfo info;
    info.bundleName_ = "bundleName_";
    info.moduleName_ = "moduleName_";
    info.abilityName_ = "abilityName_";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    sceneSession->SetSessionProperty(property);
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    sceneSession->state_ = SessionState::STATE_CONNECT;
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    sceneSession->state_ = SessionState::STATE_DISCONNECT;
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);

    StartingWindowInfo startingWindowInfo;
    startingWindowInfo.configFileEnabled_ = false;
    startingWindowInfo.iconPathEarlyVersion_ = "resource:///12345678.png";
    std::string keyForCached = info.moduleName_ + info.abilityName_;
    ssm_->startingWindowMap_[info.bundleName_][keyForCached + std::to_string(true)] = startingWindowInfo;
    ssm_->startingWindowMap_[info.bundleName_][keyForCached + std::to_string(false)] = startingWindowInfo;
    sceneSession->sessionInfo_.abilityInfo = nullptr;
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, abilityInfo);
    sceneSession->sessionInfo_.abilityInfo = abilityInfo;
    ssm_->PreLoadStartingWindow(sceneSession);
    usleep(WAIT_SLEEP_TIME);
    ASSERT_NE(nullptr, sceneSession);
}

/**
 * @tc.name: GetCropInfoByDisplaySize
 * @tc.desc: GetCropInfoByDisplaySize
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetCropInfoByDisplaySize, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    Media::ImageInfo imageInfo;
    imageInfo.size.width = 2200;
    imageInfo.size.height = 3200;
    Media::DecodeOptions decodeOpts;
    ssm_->GetCropInfoByDisplaySize(imageInfo, decodeOpts);
    EXPECT_EQ(decodeOpts.CropRect.top, 0);

    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    displayInfo->SetDisplayId(defaultScreenId);
    displayInfo->SetWidth(5000);
    displayInfo->SetHeight(5000);
    ssm_->UpdateDisplayRegion(displayInfo);
    ssm_->GetCropInfoByDisplaySize(imageInfo, decodeOpts);
    EXPECT_EQ(decodeOpts.CropRect.top, 0);

    displayInfo->SetWidth(3000);
    displayInfo->SetHeight(3000);
    ssm_->UpdateDisplayRegion(displayInfo);
    ssm_->GetCropInfoByDisplaySize(imageInfo, decodeOpts);
    EXPECT_EQ(decodeOpts.CropRect.top, 100);

    imageInfo.size.width = 3200;
    imageInfo.size.height = 2200;
    ssm_->UpdateDisplayRegion(displayInfo);
    ssm_->GetCropInfoByDisplaySize(imageInfo, decodeOpts);
    EXPECT_EQ(decodeOpts.CropRect.left, 100);

    displayInfo->SetWidth(2000);
    displayInfo->SetHeight(2000);
    ssm_->UpdateDisplayRegion(displayInfo);
    ssm_->GetCropInfoByDisplaySize(imageInfo, decodeOpts);
    EXPECT_EQ(decodeOpts.CropRect.left, 600);
}

/**
 * @tc.name: NotifyPreLoadStartingWindowFinished
 * @tc.desc: NotifyPreLoadStartingWindowFinished
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, NotifyPreLoadStartingWindowFinished, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "bundleName_";
    info.moduleName_ = "moduleName_";
    info.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->NotifyPreLoadStartingWindowFinished();
    ASSERT_NE(nullptr, sceneSession);
}

/**
 * @tc.name: GetIsDarkFromConfiguration
 * @tc.desc: GetIsDarkFromConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetIsDarkFromConfiguration, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::string autoColorMode = AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO;
    bool isDark = ssm_->GetIsDarkFromConfiguration(autoColorMode);
    EXPECT_EQ(false, isDark);
    std::string darkColorMode = AppExecFwk::ConfigurationInner::COLOR_MODE_DARK;
    isDark = ssm_->GetIsDarkFromConfiguration(darkColorMode);
    EXPECT_EQ(true, isDark);
    std::string lightColorMode = AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT;
    isDark = ssm_->GetIsDarkFromConfiguration(lightColorMode);
    EXPECT_EQ(false, isDark);
}

/**
 * @tc.name: UpdateAllStartingWindowRdb
 * @tc.desc: UpdateAllStartingWindowRdb
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, UpdateAllStartingWindowRdb, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(RdbLogCallback);
    ASSERT_NE(ssm_, nullptr);
    ssm_->UpdateAllStartingWindowRdb();
    usleep(WAIT_SLEEP_TIME);
    EXPECT_TRUE(g_logMsg.find("GetBundleInfosV9 error") != std::string::npos);
}

/**
 * @tc.name: GetCallerSessionColorMode
 * @tc.desc: GetCallerSessionColorMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetCallerSessionColorMode, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    int32_t persistentId = 1000;

    auto res = ssm_->GetCallerSessionColorMode(sessionInfo);
    EXPECT_EQ(res, AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO);

    sessionInfo.callerBundleName_ = "bundleName_";
    sessionInfo.callerPersistentId_ = persistentId;
    res = ssm_->GetCallerSessionColorMode(sessionInfo);
    EXPECT_EQ(res, AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO);

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_NE(ssm_->GetSceneSession(persistentId), nullptr);

    sceneSession->OnUpdateColorMode(AppExecFwk::ConfigurationInner::COLOR_MODE_DARK, true);
    res = ssm_->GetCallerSessionColorMode(sessionInfo);
    ssm_->sceneSessionMap_.erase(persistentId);
    EXPECT_EQ(res, AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
}

/**
 * @tc.name: ClearStartWindowColorFollowApp
 * @tc.desc: ClearStartWindowColorFollowApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, ClearStartWindowColorFollowApp, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowFollowAppMap_.clear();
    EXPECT_EQ(true, ssm_->startingWindowFollowAppMap_.empty());
    std::string bundleName = "bundleName_";
    std::unordered_set<std::string> infoSet({ "key" });
    ssm_->startingWindowFollowAppMap_.emplace(bundleName, infoSet);
    EXPECT_EQ(false, ssm_->startingWindowFollowAppMap_.empty());
    ssm_->ClearStartWindowColorFollowApp(bundleName);
    EXPECT_EQ(true, ssm_->startingWindowFollowAppMap_.empty());
}

/**
 * @tc.name: GetStartWindowColorFollowApp
 * @tc.desc: GetStartWindowColorFollowApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, GetStartWindowColorFollowApp, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowFollowAppMap_.clear();
    EXPECT_EQ(true, ssm_->startingWindowFollowAppMap_.empty());
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName_";
    sessionInfo.moduleName_ = "moduleName_";
    sessionInfo.abilityName_ = "abilityName_";
    auto res = ssm_->GetStartWindowColorFollowApp(sessionInfo);
    EXPECT_EQ(false, res);
    ssm_->startingWindowFollowAppMap_.emplace(sessionInfo.bundleName_, std::unordered_set<std::string> {});
    res = ssm_->GetStartWindowColorFollowApp(sessionInfo);
    EXPECT_EQ(false, res);
    ssm_->startingWindowFollowAppMap_[sessionInfo.bundleName_].insert(
        sessionInfo.moduleName_ + sessionInfo.abilityName_);
    res = ssm_->GetStartWindowColorFollowApp(sessionInfo);
    EXPECT_EQ(true, res);
    ssm_->startingWindowFollowAppMap_.clear();
    EXPECT_EQ(true, ssm_->startingWindowFollowAppMap_.empty());
}

/**
 * @tc.name: UpdateCachedColorToAppSet
 * @tc.desc: SceneSessionManager set update cached color to app set
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, UpdateCachedColorToAppSet, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    ssm_->startingWindowColorFromAppMap_.clear();
    std::string bundleName = "testBundleName";
    std::string moduleName = "testModuleName";
    std::string abilityName = "testAbilityName";
    std::string keyForCached = moduleName + abilityName + std::to_string(true);
    std::string keyForAppSet = moduleName + abilityName;
    StartingWindowInfo info;
    StartingWindowInfo tempInfo;
    info.backgroundColor_ = 0x00000000;
    ssm_->startingWindowMap_[bundleName][keyForCached] = info;
    ssm_->startingWindowMap_[bundleName][keyForAppSet + std::to_string(false)] = info;
    ssm_->UpdateCachedColorToAppSet(bundleName, moduleName, abilityName, tempInfo);
    EXPECT_EQ(0x00000000, ssm_->startingWindowMap_[bundleName][keyForCached].backgroundColor_);

    ssm_->startingWindowColorFromAppMap_[bundleName][keyForAppSet] = 0xffffffff;
    ssm_->UpdateCachedColorToAppSet(bundleName, moduleName, abilityName, tempInfo);
    EXPECT_EQ(0xffffffff, ssm_->startingWindowMap_[bundleName][keyForCached].backgroundColor_);

    ssm_->startingWindowMap_.clear();
    ssm_->UpdateCachedColorToAppSet(bundleName, moduleName, abilityName, tempInfo);
    EXPECT_EQ(0, ssm_->startingWindowMap_.size());
}

/**
 * @tc.name: InitStartingWindow
 * @tc.desc: InitStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, InitStartingWindow, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(RdbLogCallback);
    ASSERT_NE(ssm_, nullptr);
    ssm_->syncLoadStartingWindow_ = false;
    EXPECT_EQ(ssm_->IsSyncLoadStartingWindow(), false);
    ssm_->InitStartingWindow();
    EXPECT_TRUE(g_logMsg.find("Sync Load StartingWindow:") != std::string::npos);
}

/**
 * @tc.name: IsSyncLoadStartingWindow
 * @tc.desc: IsSyncLoadStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, IsSyncLoadStartingWindow, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->syncLoadStartingWindow_ = false;
    EXPECT_EQ(ssm_->IsSyncLoadStartingWindow(), false);
    ssm_->syncLoadStartingWindow_ = true;
    EXPECT_EQ(ssm_->IsSyncLoadStartingWindow(), true);
}

/**
 * @tc.name: SetPreloadingStartingWindow
 * @tc.desc: SetPreloadingStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowPatternStartingWindowTest, SetPreloadingStartingWindow, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "moduleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->preloadingStartingWindow_ = false;
    sceneSession->SetPreloadingStartingWindow(true);
    EXPECT_EQ(sceneSession->GetPreloadingStartingWindow(), true);
    sceneSession->SetPreloadingStartingWindow(false);
    EXPECT_EQ(sceneSession->GetPreloadingStartingWindow(), false);
}
} // namespace
} // namespace Rosen
} // namespace OHOS