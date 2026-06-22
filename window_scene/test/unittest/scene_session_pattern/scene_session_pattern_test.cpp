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

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
 
class SceneSessionPatternTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionPatternTest::SetUpTestCase() {}

void SceneSessionPatternTest::TearDownTestCase()
{
}

void SceneSessionPatternTest::SetUp() {}

void SceneSessionPatternTest::TearDown() {}

namespace {

/**
 * @tc.name: NotifySnapshotUpdate
 * @tc.desc: NotifySnapshotUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionPatternTest, NotifySnapshotUpdate, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WMError ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->collaboratorType_ = static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
    ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);
}


} // namespace
} // namespace Rosen
} // namespace OHOS
