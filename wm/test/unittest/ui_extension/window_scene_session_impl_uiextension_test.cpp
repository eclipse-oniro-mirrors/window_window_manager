/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "mock_uicontent.h"
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Rosen {
class WindowSceneSessionImplUIExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    sptr<WindowSceneSessionImpl> window_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneSessionImplUIExtensionTest::SetUpTestCase()
{
}

void WindowSceneSessionImplUIExtensionTest::TearDownTestCase()
{
}

void WindowSceneSessionImplUIExtensionTest::SetUp()
{
    sptr<WindowOption> option = new(std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("WindowSceneSessionImplTest");
    window_ = new(std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, window_);
    if (!handler_) {
        auto runner = AppExecFwk::EventRunner::Create("WindowSceneSessionImplTest");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    window_->handler_ = handler_;
}

void WindowSceneSessionImplUIExtensionTest::TearDown()
{
    window_ = nullptr;
}

/**
 * @tc.name: RecoverExtension
 * @tc.desc: RecoverExtension Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplUIExtensionTest, RecoverExtension, TestSize.Level2)
{
    ASSERT_NE(nullptr, window_);
    window_->RecoverExtension();
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window_->RecoverExtension();
}
} // namespace Rosen
} // namespace OHOS
