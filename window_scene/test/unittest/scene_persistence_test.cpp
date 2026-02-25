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

#include <image_type.h>
#include "scene_persistence.h"
#include "session/host/include/scene_session.h"
#include "session.h"
#include <gtest/gtest.h>
#include "session_info.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ScenePersistenceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    std::shared_ptr<Media::PixelMap> mPixelMap = std::make_shared<Media::PixelMap>();
};

constexpr const char* UNDERLINE_SEPARATOR = "_";
constexpr const char* IMAGE_SUFFIX = ".jpg";

static sptr<ScenePersistence> scenePersistence = sptr<ScenePersistence>::MakeSptr("testBundleName", 1423);

void ScenePersistenceTest::SetUpTestCase() {}

void ScenePersistenceTest::TearDownTestCase() {}

void ScenePersistenceTest::SetUp() {}

void ScenePersistenceTest::TearDown() {}

std::shared_ptr<Media::PixelMap> ConstructPixmap(int32_t width, int32_t height)
{
    Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    opts.pixelFormat = Media::PixelFormat::RGB_888;
    std::unique_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opts);
    std::shared_ptr<Media::PixelMap> ret = std::move(pixelMap);
    return ret;
}

namespace {
/**
 * @tc.name: CreateSnapshotDir
 * @tc.desc: test function : CreateSnapshotDir
 * @tc.type: FUNC
 */
HWTEST_F(ScenePersistenceTest, CreateSnapshotDir, TestSize.Level1)
{
    std::string directory = "0/Storage";
    ASSERT_NE(nullptr, scenePersistence);
    bool result = scenePersistence->CreateSnapshotDir(directory);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CreateUpdatedIconDir
 * @tc.desc: test function : CreateUpdatedIconDir
 * @tc.type: FUNC
 */
HWTEST_F(ScenePersistenceTest, CreateUpdatedIconDir, TestSize.Level1)
{
    std::string directory = "0/Storage";
    ASSERT_NE(nullptr, scenePersistence);
    bool result = scenePersistence->CreateUpdatedIconDir(directory);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SaveUpdatedIcon
 * @tc.desc: test function : SaveUpdatedIcon
 * @tc.type: FUNC
 */
HWTEST_F(ScenePersistenceTest, SaveUpdatedIcon, TestSize.Level1)
{
    sptr<ScenePersistence> scenePersistenceTmp = sptr<ScenePersistence>::MakeSptr("testBundleName", 1423);
    scenePersistenceTmp->CreateUpdatedIconDir("/tmp");
    std::shared_ptr<Media::PixelMap> pixelMap = ConstructPixmap(1, 1);
    scenePersistenceTmp->SaveUpdatedIcon(pixelMap);
 
    std::shared_ptr<Media::PixelMap> pixelMap2 = ConstructPixmap(1025, 1);
    scenePersistenceTmp->SaveUpdatedIcon(pixelMap2);
    EXPECT_EQ(pixelMap2->GetHeight(), 1);
 
    std::shared_ptr<Media::PixelMap> pixelMap3 = ConstructPixmap(1, 1025);
    scenePersistenceTmp->SaveUpdatedIcon(pixelMap3);
    EXPECT_EQ(pixelMap3->GetWidth(), 1);
}
} // namespace
} // namespace Rosen
} // namespace OHOS