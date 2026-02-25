/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include "iremote_object_mocker.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "window_manager.cpp"
#include "window_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowManagerAttributeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    int32_t userId_ = 100;
    sptr<WindowManager> instance_ = nullptr;
    sptr<WindowAdapter> windowAdapter_ = nullptr;
};

void WindowManagerAttributeTest::SetUpTestCase() {}

void WindowManagerAttributeTest::TearDownTestCase() {}

void WindowManagerAttributeTest::SetUp()
{
    instance_ = &WindowManager::GetInstance(userId_);
    windowAdapter_ = &WindowAdapter::GetInstance(userId_);
}

void WindowManagerAttributeTest::TearDown()
{
    WindowManager::RemoveInstanceByUserId(userId_);
}

namespace {
/**
 * @tc.name: SetProcessWatermark
 * @tc.desc: check SetProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAttributeTest, SetProcessWatermark, TestSize.Level1)
{
    ASSERT_NE(windowAdapter_, nullptr);
    windowAdapter_->isProxyValid_ = true;
    windowAdapter_->windowManagerServiceProxy_ = nullptr;
    int32_t pid = 1000;
    const std::string watermarkName = "SetProcessWatermarkName";
    bool isEnabled = true;
    auto ret = instance_->SetProcessWatermark(pid, watermarkName, isEnabled);
    EXPECT_EQ(ret, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: SetWindowSnapshotSkip
 * @tc.desc: check SetWindowSnapshotSkip
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAttributeTest, SetWindowSnapshotSkip, TestSize.Level1)
{
    int32_t windowId = 1;
    bool isSkip = true;
    auto expectRet = WindowAdapter::GetInstance(userId_).SetWindowSnapshotSkip(windowId, isSkip);
    auto ret = instance_->SetWindowSnapshotSkip(windowId, isSkip);
    EXPECT_EQ(ret, expectRet);
}
} // namespace
} // namespace Rosen
} // namespace OHOS