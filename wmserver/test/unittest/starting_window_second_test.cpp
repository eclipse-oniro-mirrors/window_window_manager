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
#include <gmock/gmock.h>
#include <bundle_mgr_interface.h>
#include <filesystem>
#include <fstream>

#include "mock_resource_manager.h"
#include "starting_window.h"

using namespace testing;
using namespace testing::ext;
namespace {
constexpr size_t MAX_SOURCE_SIZE = 300 * 1024 * 1024;
const uint8_t RAW_PNG_DATA[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
    0x89, 0x00, 0x00, 0x00, 0x0A, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0x63, 0x62, 0x00, 0x04, 0x00,
    0x00, 0xFF, 0xFF, 0x00, 0x06, 0x00, 0x03, 0x57, 0x0A, 0xC9, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x49,
    0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};
const uint8_t RAW_GIF_DATA[] = {
    'G', 'I', 'F', '8', '9', 'a', 0x01, 0x00, 0x01, 0x00, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x2C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44, 0x01, 0x00, 0x2C, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x04, 0x01, 0x00, 0x3B
};
const uint8_t RAW_INVALID_GIF_DATA[] = {
    'G', 'I', 'F', '8', '9', 'a', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3B
};
const std::string IMAGE_PLACE_HOLDER_PNG_PATH = "/etc/window/resources/bg_place_holder.png";
}

namespace OHOS {
namespace AppExecFwk {
class MockIBundleMgr : public IBundleMgr {
public:
    MockIBundleMgr() = default;
    virtual ~MockIBundleMgr() = default;
    MOCK_METHOD4(QueryAbilityInfo, bool(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo));
    virtual OHOS::sptr<OHOS::IRemoteObject> AsObject()
    {
        return nullptr;
    }
};
}

namespace Rosen {
class StartingWindowTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void StartingWindowTest::SetUpTestCase()
{
}

void StartingWindowTest::TearDownTestCase()
{
}

void StartingWindowTest::SetUp()
{
}

void StartingWindowTest::TearDown()
{
    StartingWindow::startingWindowShowRunning_ = false;
    if (StartingWindow::startingWindowShowThread_.joinable()) {
        StartingWindow::startingWindowShowThread_.join();
    }
    StartingWindow::firstFrameCompleted_ = false;
    StartingWindow::UnRegisterStartingWindowShowInfo();
}

/**
 * @tc.name: GetBundleManager01
 * @tc.desc: GetBundleManager
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetBundleManager01, TestSize.Level1)
{
    auto bundleMgr = StartingWindow::GetBundleManager();
    ASSERT_NE(bundleMgr, nullptr);
}

/**
 * @tc.name: CreateResourceManager01
 * @tc.desc: CreateResourceManager
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, CreateResourceManager01, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    auto ref = StartingWindow::CreateResourceManager(abilityInfo);
    ASSERT_EQ(ref, nullptr);

    abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->bundleName = "com.ohos.test.bundleName";
    abilityInfo->moduleName = "com.ohos.test.moduleName";
    abilityInfo->hapPath = "data/resource/testHapPath";

    ref = StartingWindow::CreateResourceManager(abilityInfo);
    ASSERT_NE(ref, nullptr);
    EXPECT_EQ(ref->bundleInfo.first, abilityInfo->bundleName);
    EXPECT_EQ(ref->bundleInfo.second, abilityInfo->moduleName);
}

/**
 * @tc.name: GetAbilityInfoFromBMS01
 * @tc.desc: GetAbilityInfoFromBMS
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetAbilityInfoFromBMS01, TestSize.Level1)
{
    auto ref = StartingWindow::GetAbilityInfoFromBMS(nullptr, nullptr);
    ASSERT_EQ(ref, nullptr);

    sptr<WindowTransitionInfo> winInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(winInfo, 1);
    node->abilityInfo_.abilityName_ = "com.ohos.test.abilityName";
    node->abilityInfo_.bundleName_ = "com.ohos.test.bundleName";
    ASSERT_NE(node, nullptr);
    ref = StartingWindow::GetAbilityInfoFromBMS(node, nullptr);
    ASSERT_EQ(ref, nullptr);

    auto mockBundleMgr = sptr<AppExecFwk::MockIBundleMgr>::MakeSptr();
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(_, _, _, _))
        .Times(1).WillOnce(Return(false));
    ref = StartingWindow::GetAbilityInfoFromBMS(node, mockBundleMgr);
    ASSERT_EQ(ref, nullptr);

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = node->abilityInfo_.abilityName_;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(_, _, _, _))
        .Times(1).WillOnce(DoAll(
            SetArgReferee<3>(abilityInfo),
            Return(true)));
    ref = StartingWindow::GetAbilityInfoFromBMS(node, mockBundleMgr);
    ASSERT_NE(ref, nullptr);
    EXPECT_EQ(ref->name, node->abilityInfo_.abilityName_);
}

/**
 * @tc.name: GetPixelMapListInfo01
 * @tc.desc: GetPixelMapListInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetPixelMapListInfo01, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    auto ref = StartingWindow::GetPixelMapListInfo(1, resourceMgr, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ref = StartingWindow::GetPixelMapListInfo(1, resourceMgr, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    resourceMgr = StartingWindow::CreateResourceManager(abilityInfo);
    ref = StartingWindow::GetPixelMapListInfo(1, resourceMgr, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    ref = StartingWindow::GetPixelMapListInfo(std::numeric_limits<uint32_t>::min(), resourceMgr, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    ref = StartingWindow::GetPixelMapListInfo(-1, resourceMgr, abilityInfo);
    ASSERT_EQ(ref, nullptr);
}

/**
 * @tc.name: GetPixelMapListInfo02
 * @tc.desc: GetPixelMapListInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetPixelMapListInfo02, TestSize.Level1)
{
    auto mockResourceManager = std::make_shared<OHOS::Global::Resource::MockResourceManager>();
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    uint32_t mediaDataId = std::numeric_limits<uint32_t>::max();

    abilityInfo->hapPath = "";
    EXPECT_CALL(*mockResourceManager, GetMediaById(_, _, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::NOT_FOUND));
    auto ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    EXPECT_CALL(*mockResourceManager, GetMediaById(_, _, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::SUCCESS));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    abilityInfo->hapPath = "/home/somepath/somefile";
    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::NOT_FOUND));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::SUCCESS));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);
}

/**
 * @tc.name: GetPixelMapListInfo03
 * @tc.desc: GetPixelMapListInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetPixelMapListInfo03, TestSize.Level1)
{
    auto mockResourceManager = std::make_shared<OHOS::Global::Resource::MockResourceManager>();
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    uint32_t mediaDataId = 1;
    abilityInfo->hapPath = "/home/somepath/somefile";

    const size_t pngDataSize = sizeof(RAW_PNG_DATA);
    auto testPngData = std::make_unique<uint8_t[]>(pngDataSize);
    for (size_t i = 0; i < pngDataSize; ++i) {
        testPngData[i] = RAW_PNG_DATA[i];
    }

    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
    .WillOnce(DoAll(
        SetArgReferee<1>(pngDataSize),
        testing::Invoke([&testPngData](uint32_t, size_t&, std::unique_ptr<uint8_t[]>& dataOut, uint32_t) {
            dataOut = std::move(testPngData);
        }),
        Return(Global::Resource::RState::SUCCESS)
    ));
    auto ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_NE(ref, nullptr);

    //SOURCE_SIZE is bigger than MAX_SOURCE_SIZE
    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
    .WillOnce(DoAll(
        SetArgReferee<1>(MAX_SOURCE_SIZE + 1),
        testing::Invoke([&testPngData](uint32_t, size_t&, std::unique_ptr<uint8_t[]>& dataOut, uint32_t) {
            dataOut = std::move(testPngData);
        }),
        Return(Global::Resource::RState::SUCCESS)
    ));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);
}


/**
 * @tc.name: GetPixelMapListInfo04
 * @tc.desc: GetPixelMapListInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetPixelMapListInfo04, TestSize.Level1)
{
    auto mockResourceManager = std::make_shared<OHOS::Global::Resource::MockResourceManager>();
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    uint32_t mediaDataId = 1;
    abilityInfo->hapPath = "/home/somepath/somefile";

    const size_t gifDataSize = sizeof(RAW_GIF_DATA);
    auto testGifData = std::make_unique<uint8_t[]>(gifDataSize);
    for (size_t i = 0; i < gifDataSize; i++) {
        testGifData[i] = RAW_GIF_DATA[i];
    }

    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
    .WillOnce(DoAll(
        SetArgReferee<1>(gifDataSize),
        testing::Invoke([&testGifData](uint32_t, size_t&, std::unique_ptr<uint8_t[]>& dataOut, uint32_t) {
            dataOut = std::move(testGifData);
        }),
        Return(Global::Resource::RState::SUCCESS)
    ));
    auto ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_NE(ref, nullptr);
    EXPECT_EQ(ref->pixelMaps.size(), 2);
    EXPECT_EQ(ref->delayTimes.size(), 2);

    const size_t invalidGifDataSize = sizeof(RAW_INVALID_GIF_DATA);
    auto testInvalidGifData = std::make_unique<uint8_t[]>(invalidGifDataSize);
    for (size_t i = 0; i < invalidGifDataSize; i++) {
        testInvalidGifData[i] = RAW_INVALID_GIF_DATA[i];
    }

    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
    .WillOnce(DoAll(
        SetArgReferee<1>(invalidGifDataSize),
        testing::Invoke([&testInvalidGifData](uint32_t, size_t&, std::unique_ptr<uint8_t[]>& dataOut, uint32_t) {
            dataOut = std::move(testInvalidGifData);
        }),
        Return(Global::Resource::RState::SUCCESS)
    ));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);
}

/**
 * @tc.name: GetPixelMapListInfo05
 * @tc.desc: GetPixelMapListInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetPixelMapListInfo05, TestSize.Level1)
{
    auto mockResourceManager = std::make_shared<OHOS::Global::Resource::MockResourceManager>();
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    uint32_t mediaDataId = 1;
    abilityInfo->hapPath = "";

    //Resource path is inValid
    std::string tmpPath1 = "home/data:";
    EXPECT_CALL(*mockResourceManager, GetMediaById(_, _, _))
        .WillOnce(DoAll(
            SetArgReferee<1>(std::string(tmpPath1)),
            Return(Global::Resource::RState::SUCCESS)
    ));
    auto ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);

    std::filesystem::path tmpDir = std::filesystem::temp_directory_path();
    std::filesystem::path tmpFile = tmpDir / "wms_test_XXXXXX.png";
    std::string tmpPath2 = tmpFile.string();
    std::ofstream outFile(tmpPath2, std::ios::binary);
    ASSERT_TRUE(outFile.is_open()) << "Failed to create temp file: " << tmpPath2;
    outFile.write(reinterpret_cast<const char*>(RAW_PNG_DATA), sizeof(RAW_PNG_DATA));
    outFile.close();

    EXPECT_CALL(*mockResourceManager, GetMediaById(_, _, _))
        .WillOnce(DoAll(
            SetArgReferee<1>(std::string(tmpPath2)),
            Return(Global::Resource::RState::SUCCESS)
        ));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_NE(ref, nullptr);
    std::filesystem::remove(tmpPath2);
}

/**
 * @tc.name: GetPixelMapListInfo06
 * @tc.desc: GetPixelMapListInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetPixelMapListInfo06, TestSize.Level1)
{
    auto mockResourceManager = std::make_shared<OHOS::Global::Resource::MockResourceManager>();
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    uint32_t mediaDataId = 1;
    abilityInfo->hapPath = "";

    std::filesystem::path tmpDir1 = std::filesystem::temp_directory_path();
    std::filesystem::path tmpFile1 = tmpDir1 / "wms_test_XXXXXX.gif";
    std::string tmpPath1 = tmpFile1.string();
    std::ofstream outFile1(tmpPath1, std::ios::binary);
    ASSERT_TRUE(outFile1.is_open()) << "Failed to create temp file: " << tmpPath1;
    outFile1.write(reinterpret_cast<const char*>(RAW_GIF_DATA), sizeof(RAW_GIF_DATA));
    outFile1.close();
    EXPECT_CALL(*mockResourceManager, GetMediaById(_, _, _))
        .WillOnce(DoAll(
            SetArgReferee<1>(std::string(tmpPath1)),
            Return(Global::Resource::RState::SUCCESS)
        ));
    auto ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_NE(ref, nullptr);
    EXPECT_EQ(ref->pixelMaps.size(), 2);
    EXPECT_EQ(ref->delayTimes.size(), 2);
    std::filesystem::remove(tmpPath1);

    std::filesystem::path tmpDir2 = std::filesystem::temp_directory_path();
    std::filesystem::path tmpFile2 = tmpDir2 / "wms_test_Invalid.gif";
    std::string tmpPath2 = tmpFile2.string();
    std::ofstream outFile2(tmpPath2, std::ios::binary);
    ASSERT_TRUE(outFile2.is_open()) << "Failed to create temp file: " << tmpPath2;
    outFile2.write(reinterpret_cast<const char*>(RAW_INVALID_GIF_DATA), sizeof(RAW_INVALID_GIF_DATA));
    outFile2.close();
    EXPECT_CALL(*mockResourceManager, GetMediaById(_, _, _))
        .WillOnce(DoAll(
            SetArgReferee<1>(std::string(tmpPath2)),
            Return(Global::Resource::RState::SUCCESS)
        ));
    ref = StartingWindow::GetPixelMapListInfo(mediaDataId, mockResourceManager, abilityInfo);
    ASSERT_EQ(ref, nullptr);
    std::filesystem::remove(tmpPath2);
}

/**
 * @tc.name: GetCustomStartingWindowInfo01
 * @tc.desc: GetCustomStartingWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, GetCustomStartingWindowInfo01, TestSize.Level1)
{
    auto ref = StartingWindow::GetCustomStartingWindowInfo(nullptr, nullptr);
    ASSERT_EQ(ref, nullptr);

    sptr<WindowTransitionInfo> winInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(winInfo, 1);
    ref = StartingWindow::GetCustomStartingWindowInfo(node, nullptr);
    ASSERT_EQ(ref, nullptr);

    auto bundleMgr = StartingWindow::GetBundleManager();
    ref = StartingWindow::GetCustomStartingWindowInfo(node, bundleMgr);
    ASSERT_EQ(ref, nullptr);

    auto mockBundleMgr = sptr<AppExecFwk::MockIBundleMgr>::MakeSptr();
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(_, _, _, _))
        .Times(1).WillOnce(Return(true));
    ref = StartingWindow::GetCustomStartingWindowInfo(node, mockBundleMgr);
    ASSERT_EQ(ref, nullptr);
}

/**
 * @tc.name: DoGetCustomStartingWindowInfo01
 * @tc.desc: DoGetCustomStartingWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, DoGetCustomStartingWindowInfo01, TestSize.Level1)
{
    auto ref = StartingWindow::DoGetCustomStartingWindowInfo(nullptr, nullptr);
    ASSERT_EQ(ref, nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->startWindowResource.startWindowBackgroundColorId = 0;
    ref = StartingWindow::DoGetCustomStartingWindowInfo(abilityInfo, nullptr);
    ASSERT_EQ(ref, nullptr);

    auto mockResourceManager = std::make_shared<OHOS::Global::Resource::MockResourceManager>();
    EXPECT_CALL(*mockResourceManager, GetColorById(_, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::NOT_FOUND));
    ref = StartingWindow::DoGetCustomStartingWindowInfo(abilityInfo, mockResourceManager);
    ASSERT_EQ(ref, nullptr);

    abilityInfo->startWindowResource.startWindowBackgroundColorId = 1;
    abilityInfo->startWindowResource.startWindowAppIconId = 1;
    abilityInfo->hapPath = "home/starting_window/hapPath";
    const size_t pngDataSize = sizeof(RAW_PNG_DATA);
    auto testData = std::make_unique<uint8_t[]>(pngDataSize);
    for (size_t i = 0; i < pngDataSize; ++i) {
        testData[i] = RAW_PNG_DATA[i];
    }

    EXPECT_CALL(*mockResourceManager, GetColorById(_, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::SUCCESS));
    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
    .WillOnce(DoAll(Return(Global::Resource::RState::NOT_FOUND)));
    ref = StartingWindow::DoGetCustomStartingWindowInfo(abilityInfo, mockResourceManager);
    ASSERT_NE(ref, nullptr);

    EXPECT_CALL(*mockResourceManager, GetColorById(_, _))
        .Times(1).WillOnce(Return(Global::Resource::RState::SUCCESS));
    EXPECT_CALL(*mockResourceManager, GetMediaDataById(_, _, _, _))
    .WillOnce(DoAll(
        SetArgReferee<1>(pngDataSize),
        testing::Invoke([&testData](uint32_t, size_t&, std::unique_ptr<uint8_t[]>& dataOut, uint32_t) {
            dataOut = std::move(testData);
        }),
        Return(Global::Resource::RState::SUCCESS)
    ));
    ref = StartingWindow::DoGetCustomStartingWindowInfo(abilityInfo, mockResourceManager);
    ASSERT_NE(ref, nullptr);
}

/**
 * @tc.name: DrawStartingWindow01
 * @tc.desc: DrawStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, DrawStartingWindow01, TestSize.Level1)
{
    sptr<WindowTransitionInfo> winInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(winInfo, 1);
    ASSERT_NE(node, nullptr);
    Rosen::RSSurfaceNodeConfig config;
    node->startingWinSurfaceNode_ = Rosen::RSSurfaceNode::Create(config);
    ASSERT_NE(node->startingWinSurfaceNode_, nullptr);

    Rosen::Rect rect {0, 0, 0, 0};
    auto ret = StartingWindow::DrawStartingWindow(nullptr, node, rect);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    auto info = std::make_shared<Rosen::StartingWindowPageDrawInfo>();
    info->bgColor = 0x00660000;
    ret = StartingWindow::DrawStartingWindow(info, node, rect);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    rect = {0, 0, 200, 300};
    ret = StartingWindow::DrawStartingWindow(info, node, rect);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: DrawStartingWindow02
 * @tc.desc: DrawStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, DrawStartingWindow02, TestSize.Level1)
{
    sptr<WindowTransitionInfo> winInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(winInfo, 1);
    ASSERT_NE(node, nullptr);
    Rosen::RSSurfaceNodeConfig config;
    node->startingWinSurfaceNode_ = Rosen::RSSurfaceNode::Create(config);
    ASSERT_NE(node->startingWinSurfaceNode_, nullptr);

    Rosen::Rect rect {0, 0, 0, -1};
    auto info = std::make_shared<Rosen::StartingWindowPageDrawInfo>();
    info->bgColor = 0x00660000;
    info->appIcon = std::make_shared<ResourceInfo>();
    std::shared_ptr<Media::PixelMap> pixelMap = SurfaceDraw::DecodeImageToPixelMap(IMAGE_PLACE_HOLDER_PNG_PATH);
    info->appIcon->pixelMaps.push_back(pixelMap);

    auto ret = StartingWindow::DrawStartingWindow(info, node, rect);
    EXPECT_EQ(StartingWindow::startingWindowShowRunning_, false);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: DrawStartingWindow03
 * @tc.desc: DrawStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, DrawStartingWindow03, TestSize.Level1)
{
    sptr<WindowTransitionInfo> winInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(winInfo, 1);
    ASSERT_NE(node, nullptr);
    Rosen::RSSurfaceNodeConfig config;
    node->startingWinSurfaceNode_ = Rosen::RSSurfaceNode::Create(config);
    ASSERT_NE(node->startingWinSurfaceNode_, nullptr);

    Rosen::Rect rect {0, 0, 300, 600};
    auto info = std::make_shared<Rosen::StartingWindowPageDrawInfo>();
    info->bgColor = 0x00660000;
    info->appIcon = std::make_shared<ResourceInfo>();
    std::shared_ptr<Media::PixelMap> pixelMap = SurfaceDraw::DecodeImageToPixelMap(IMAGE_PLACE_HOLDER_PNG_PATH);
    info->appIcon->pixelMaps.push_back(pixelMap);

    StartingWindow::startingWindowShowRunning_ = true;
    auto ret = StartingWindow::DrawStartingWindow(info, node, rect);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: RegisterStartingWindowShowInfo01
 * @tc.desc: RegisterStartingWindowShowInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, RegisterStartingWindowShowInfo01, TestSize.Level1)
{
    sptr<WindowTransitionInfo> winInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(winInfo, 1);
    ASSERT_NE(node, nullptr);
    Rosen::Rect rect { 0, 0, 200, 300 };
    auto info = std::make_shared<Rosen::StartingWindowPageDrawInfo>();
    info->bgColor = 0x00660000;
    info->appIcon = std::make_shared<ResourceInfo>();
    info->appIcon->delayTimes.push_back(0);
    float vpRatio = 1.5;

    StartingWindow::RegisterStartingWindowShowInfo(node, rect, info, vpRatio);
    ASSERT_NE(StartingWindow::startingWindowShowInfo_.info, nullptr);
}

/**
 * @tc.name: UnRegisterStartingWindowShowInfo01
 * @tc.desc: UnRegisterStartingWindowShowInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, UnRegisterStartingWindowShowInfo01, TestSize.Level1)
{
    StartingWindow::startingWindowShowInfo_.info = std::make_shared<Rosen::StartingWindowPageDrawInfo>();
    StartingWindow::startingWindowShowInfo_.info->appIcon = std::make_shared<ResourceInfo>();
    std::shared_ptr<Media::PixelMap> pixelMap = SurfaceDraw::DecodeImageToPixelMap(IMAGE_PLACE_HOLDER_PNG_PATH);
    StartingWindow::startingWindowShowInfo_.info->appIcon->pixelMaps.push_back(pixelMap);
    StartingWindow::UnRegisterStartingWindowShowInfo();
    ASSERT_EQ(StartingWindow::startingWindowShowInfo_.info, nullptr);
}

/**
 * @tc.name: UpdateWindowShowInfo01
 * @tc.desc: UpdateWindowShowInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartingWindowTest, UpdateWindowShowInfo01, TestSize.Level1)
{
    StartingWindowShowInfo startingWindowShowInfo;

    bool needDraw = false;
    StartingWindow::UpdateWindowShowInfo(startingWindowShowInfo, needDraw);
    EXPECT_NE(startingWindowShowInfo.resStates.size(), 0);
    EXPECT_NE(needDraw, true);

    startingWindowShowInfo.resStates[0].next = std::chrono::steady_clock::now() - std::chrono::seconds(1);
    startingWindowShowInfo.resStates[0].frameCount = 1;
    startingWindowShowInfo.resStates[0].delay.push_back(INT32_MAX);
    startingWindowShowInfo.resStates[0].frameIdx = 0;

    StartingWindow::UpdateWindowShowInfo(startingWindowShowInfo, needDraw);
    EXPECT_EQ(needDraw, true);
}
} // namespace Rosen
} // namespace OHOS