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
#include "session/host/include/main_session.h"
#include "session_info.h"
#include "session_manager/include/scene_session_manager.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class CompatibleModeMainSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void CompatibleModeMainSessionTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup test case";
}

void CompatibleModeMainSessionTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown test case";
}

void CompatibleModeMainSessionTest::SetUp()
{
    GTEST_LOG_(INFO) << "setup";
}

void CompatibleModeMainSessionTest::TearDown()
{
    GTEST_LOG_(INFO) << "teardown";
}

namespace {
/**
 * @tc.name: RegisterCompatibleModeChangeCallback
 * @tc.desc: RegisterCompatibleModeChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeMainSessionTest, RegisterCompatibleModeChangeCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCompatibleModeChangeCallback test start";
    SessionInfo info;
    info.abilityName_ = "testAbilityName";
    info.moduleName_ = "testModuleName";
    info.bundleName_ = "testBundleName";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->RegisterCompatibleModeChangeCallback([](CompatibleStyleMode mode) {
        GTEST_LOG_(INFO) << "RegisterCompatibleModeChangeCallback callback";
    });
    ASSERT_NE(session->compatibleModeChangeCallback_, nullptr);
    GTEST_LOG_(INFO) << "RegisterCompatibleModeChangeCallback test end";
}

/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithNullCallback
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeMainSessionTest, NotifyCompatibleModeChangeWithNullCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNullCallback test start";
    SessionInfo info;
    info.abilityName_ = "testAbilityName";
    info.moduleName_ = "testModuleName";
    info.bundleName_ = "testBundleName";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    auto ret = session->NotifyCompatibleModeChange(CompatibleStyleMode::LANDSCAPE_DEFAULT);
    ASSERT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNullCallback test end";
}

/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithNotNullCallback
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeMainSessionTest, NotifyCompatibleModeChangeWithNotNullCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNotNullCallback test start";
    SessionInfo info;
    info.abilityName_ = "testAbilityName";
    info.moduleName_ = "testModuleName";
    info.bundleName_ = "testBundleName";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->RegisterCompatibleModeChangeCallback([](CompatibleStyleMode mode) {
        GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNotNullCallback callback";
    });
    auto ret = session->NotifyCompatibleModeChange(CompatibleStyleMode::LANDSCAPE_DEFAULT);
    ASSERT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNotNullCallback test end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
