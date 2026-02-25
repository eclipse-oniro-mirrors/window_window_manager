/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "window_option.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowOptionEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
void WindowOptionEventTest::SetUpTestCase() {}

void WindowOptionEventTest::TearDownTestCase() {}

void WindowOptionEventTest::SetUp() {}

void WindowOptionEventTest::TearDown() {}

namespace {
/**
 * @tc.name: Touchable01
 * @tc.desc: SetTouchable/GetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowOptionEventTest, Touchable01, TestSize.Level1)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetTouchable(true);
    ASSERT_EQ(true, option->GetTouchable());
}
}
}
}