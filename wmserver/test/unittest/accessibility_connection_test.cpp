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
#include "accessibility_connection.h"
#include "wm_common.h"
#include "dm_common.h"
#include "common_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class AccessibilityConnectionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<AccessibilityConnection> accessibilityConnection_;
};

void AccessibilityConnectionTest::SetUpTestCase()
{
}

void AccessibilityConnectionTest::TearDownTestCase()
{
}

void AccessibilityConnectionTest::SetUp()
{
    sptr<WindowRoot> windowRoot_ = new WindowRoot([](Event event, const sptr<IRemoteObject>& remoteObject) {});
    accessibilityConnection_ = new AccessibilityConnection(windowRoot_);
}

void AccessibilityConnectionTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetAccessibilityWindowInfo01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityConnectionTest, GetAccessibilityWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    accessibilityConnection_->GetAccessibilityWindowInfo(infos);
    ASSERT_EQ(0, infos.size());
}

/**
 * @tc.name: UpdateFocusChangeEvent01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityConnectionTest, UpdateFocusChangeEvent01, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    ScreenId displayGroupId = 0;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, displayGroupId);
    accessibilityConnection_->UpdateFocusChangeEvent(container);
    ASSERT_EQ(1, accessibilityConnection_->focusedWindowMap_.size());
}

/**
 * @tc.name: UpdateFocusChangeEvent02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityConnectionTest, UpdateFocusChangeEvent02, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    ScreenId displayGroupId = 0;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, displayGroupId);
    uint32_t newFocusWindow = 1;
    container->SetFocusWindow(newFocusWindow);
    uint32_t oldFocusWindow = 1;
    accessibilityConnection_->focusedWindowMap_.insert(std::make_pair(container, oldFocusWindow));
    accessibilityConnection_->UpdateFocusChangeEvent(container);
    ASSERT_EQ(1, accessibilityConnection_->focusedWindowMap_.size());
    ASSERT_EQ(1, accessibilityConnection_->focusedWindowMap_[container]);
}

/**
 * @tc.name: UpdateFocusChangeEvent03
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(AccessibilityConnectionTest, UpdateFocusChangeEvent03, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    ScreenId displayGroupId = 0;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, displayGroupId);
    uint32_t newFocusWindow = 2;
    container->SetFocusWindow(newFocusWindow);
    uint32_t oldFocusWindow = 1;
    accessibilityConnection_->focusedWindowMap_.insert(std::make_pair(container, oldFocusWindow));
    accessibilityConnection_->UpdateFocusChangeEvent(container);
    ASSERT_EQ(1, accessibilityConnection_->focusedWindowMap_.size());
    ASSERT_EQ(2, accessibilityConnection_->focusedWindowMap_[container]);
}
}
}
}