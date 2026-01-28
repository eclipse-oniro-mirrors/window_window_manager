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

#include "scene_persistent_storage.h"
#include <gtest/gtest.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ScenePersistentStorageTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScenePersistentStorageTest::SetUpTestCase() {}

void ScenePersistentStorageTest::TearDownTestCase() {}

void ScenePersistentStorageTest::SetUp() {}

void ScenePersistentStorageTest::TearDown() {}

namespace {
/**
 * @tc.name: HasKey
 * @tc.desc: test function : HasKey
 * @tc.type: FUNC
 */
HWTEST_F(ScenePersistentStorageTest, HasKey, TestSize.Level1)
{
    ScenePersistentStorage scenePersistentStorage_;
    std::string key = "aspect_ratio";
    ScenePersistentStorageType storageType = ScenePersistentStorageType::UKNOWN;
    bool result01 = scenePersistentStorage_.HasKey(key, storageType);
    EXPECT_FALSE(result01);
    storageType = ScenePersistentStorageType::ASPECT_RATIO;
    bool result02 = scenePersistentStorage_.HasKey(key, storageType);
    EXPECT_FALSE(result02);
}

/**
 * @tc.name: Delete
 * @tc.desc: test function : Delete
 * @tc.type: FUNC
 */
HWTEST_F(ScenePersistentStorageTest, Delete, TestSize.Level1)
{
    ScenePersistentStorage scenePersistentStorage_;
    std::string key = "aspect_ratio";
    ScenePersistentStorageType storageType = ScenePersistentStorageType::UKNOWN;
    scenePersistentStorage_.Delete(key, storageType);
    storageType = ScenePersistentStorageType::ASPECT_RATIO;
    scenePersistentStorage_.Delete(key, storageType);
    EXPECT_FALSE(scenePersistentStorage_.HasKey(key, ScenePersistentStorageType::UKNOWN));
}

/**
 * @tc.name: InitDir
 * @tc.desc: test function : InitDir
 * @tc.type: FUNC
 */
HWTEST_F(ScenePersistentStorageTest, InitDir, TestSize.Level1)
{
    ScenePersistentStorage scenePersistentStorage_;
    std::string dir_ = "0/Storage/DownLoad";
    scenePersistentStorage_.InitDir(dir_);
    EXPECT_FALSE(scenePersistentStorage_.HasKey("maximize_state", ScenePersistentStorageType::MAXIMIZE_STATE));
}
} // namespace
} // namespace Rosen
} // namespace OHOS