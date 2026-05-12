/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

// gtest
#include <gtest/gtest.h>
#include "display_test_utils.h"
#include "display_manager_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class DisplayMinimalTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void DisplayMinimalTest::SetUpTestCase()
{
}

void DisplayMinimalTest::TearDownTestCase()
{
}

void DisplayMinimalTest::SetUp()
{
}

void DisplayMinimalTest::TearDown()
{
}

namespace {
/**
 * @tc.name: BasicDisplay
 * @tc.desc: Check all displays are valid
 * @tc.type: FUNC
 */
HWTEST_F(DisplayMinimalTest, BasicDisplay01, TestSize.Level1)
{
    std::vector<DisplayId> ids = DisplayManager::GetInstance().GetAllDisplayIds();
    for (DisplayId id : ids) {
        const sptr<Display>& display = DisplayManager::GetInstance().GetDisplayById(id);
        ASSERT_NE(nullptr, display);
    }
}
}
} // namespace Rosen
} // namespace OHOS
