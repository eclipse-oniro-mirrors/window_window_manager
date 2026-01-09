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

#include <gtest/gtest.h>
#include "window_session_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class WindowSessionPropertyEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void WindowSessionPropertyEventTest::SetUpTestCase() {}

void WindowSessionPropertyEventTest::TearDownTestCase() {}

namespace {
/**
 * @tc.name: SetTouchable
 * @tc.desc: SetTouchable and GetTouchable to check the value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionPropertyEventTest, SetTouchable, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTouchable(true);
    ASSERT_EQ(property->GetTouchable(), true);
    property->SetTouchable(false);
    ASSERT_EQ(property->GetTouchable(), false);
}
}
}
}