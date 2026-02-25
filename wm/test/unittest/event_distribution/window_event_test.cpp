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

#include "window.h"
#include "window_manager_hilog.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowEventTest::SetUpTestCase()
{
}

void WindowEventTest::TearDownTestCase()
{
}

void WindowEventTest::SetUp()
{
}

void WindowEventTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetTouchable
 * @tc.desc: get Touchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventTest, GetTouchable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window->SetTouchable(false));
    EXPECT_EQ(false, window->GetTouchable());
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTouchable
 * @tc.desc: set Touchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventTest, SetTouchable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window->SetTouchable(false));
    EXPECT_EQ(false, window->GetTouchable());
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}
}
}
}