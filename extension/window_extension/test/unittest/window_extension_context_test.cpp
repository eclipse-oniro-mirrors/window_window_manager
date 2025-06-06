/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "window_extension_context.h"
#include "window_extension_proxy.h"
#include "window_extension_stub.h"
#include "window_extension_stub_impl.h"
#include "window_extension_client_interface.h"
#include "window_extension_client_stub_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowExtensionContextTest::SetUpTestCase()
{
}

void WindowExtensionContextTest::TearDownTestCase()
{
}

void WindowExtensionContextTest::SetUp()
{
}

void WindowExtensionContextTest::TearDown()
{
}

namespace {
/**
 * @tc.name: StartAbility
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionContextTest, StartAbility, TestSize.Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    WindowExtensionContext windowExtensionContext;
    WMError res = windowExtensionContext.StartAbility(want, startOptions);
    ASSERT_EQ(WMError::WM_ERROR_START_ABILITY_FAILED, res);
}

}
}
}