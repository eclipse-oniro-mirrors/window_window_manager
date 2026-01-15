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

#include "mock_uicontent.h"
#include "load_intention_event.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class IntentionEventManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Ace::UIContent> uIContent_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void IntentionEventManagerTest::SetUpTestCase() {}

void IntentionEventManagerTest::TearDownTestCase() {}

void IntentionEventManagerTest::SetUp()
{
    UnloadIntentionEvent();
    uIContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_NE(nullptr, uIContent_);
    runner_ = AppExecFwk::EventRunner::Create("TestRunner");
    EXPECT_NE(nullptr, runner_);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    EXPECT_NE(nullptr, eventHandler_);
}

void IntentionEventManagerTest::TearDown()
{
    UnloadIntentionEvent();
    uIContent_ = nullptr;
    runner_ = nullptr;
    eventHandler_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: LoadIntentionEvent
 * @tc.desc: LoadIntentionEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, LoadIntentionEvent, TestSize.Level0)
{
    bool enable = LoadIntentionEvent();
    EXPECT_EQ(true, enable);
    enable = LoadIntentionEvent();
    EXPECT_EQ(true, enable);
    UnloadIntentionEvent();
}

/**
 * @tc.name: EnableInputEventListener
 * @tc.desc: EnableInputEventListener Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, EnableInputEventListener, TestSize.Level0)
{
    bool enable = EnableInputEventListener(nullptr, nullptr, nullptr);
    EXPECT_EQ(false, enable);

    enable = LoadIntentionEvent();
    EXPECT_EQ(true, enable);
    enable = EnableInputEventListener(nullptr, nullptr, nullptr);
    EXPECT_EQ(false, enable);
}
} // namespace
} // namespace Rosen
} // namespace OHOS