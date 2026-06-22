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

#include <message_parcel.h>
#include "load_mmi_client_adapter.h"
#include "pointer_event.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class LoadMMIClientAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = std::make_shared<MMI::PointerEvent>(0);
};

void LoadMMIClientAdapterTest::SetUpTestCase() {}

void LoadMMIClientAdapterTest::TearDownTestCase() {}

void LoadMMIClientAdapterTest::SetUp()
{
    UnloadMMIClientAdapter();
}

void LoadMMIClientAdapterTest::TearDown()
{
    UnloadMMIClientAdapter();
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: LoadMMIClientAdapter
 * @tc.desc: LoadMMIClientAdapter Test
 * @tc.type: FUNC
 */
HWTEST_F(LoadMMIClientAdapterTest, LoadMMIClientAdapter, TestSize.Level0)
{
    bool enable = LoadMMIClientAdapter();
    EXPECT_EQ(true, enable);
    enable = LoadMMIClientAdapter();
    EXPECT_EQ(true, enable);
    UnloadMMIClientAdapter();
}

/**
 * @tc.name: IsHoverDown
 * @tc.desc: IsHoverDown Test
 * @tc.type: FUNC
 */
HWTEST_F(LoadMMIClientAdapterTest, IsHoverDown, TestSize.Level0)
{
    bool enable = IsHoverDown(pointerEvent);
    EXPECT_EQ(false, enable);

    enable = LoadMMIClientAdapter();
    EXPECT_EQ(true, enable);
    enable = IsHoverDown(pointerEvent);
    EXPECT_EQ(false, enable);
}

/**
 * @tc.name: PointerEventWriteToParcel
 * @tc.desc: PointerEventWriteToParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(LoadMMIClientAdapterTest, PointerEventWriteToParcel, TestSize.Level0)
{
    MessageParcel data;
    bool enable = PointerEventWriteToParcel(pointerEvent, data);
    EXPECT_EQ(false, enable);

    enable = LoadMMIClientAdapter();
    EXPECT_EQ(true, enable);
    enable = PointerEventWriteToParcel(pointerEvent, data);
    EXPECT_EQ(true, enable);
}
}
} // namespace Rosen
} // namespace OHOS