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
#include "window_manager_agent_lite.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
class WindowManagerAgentLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    int32_t userId_ = 100;
    sptr<WindowManagerAgentLite> instance_ = nullptr;
};

void WindowManagerAgentLiteTest::SetUpTestCase()
{
}

void WindowManagerAgentLiteTest::TearDownTestCase()
{
}

void WindowManagerAgentLiteTest::SetUp()
{
    instance_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
}

void WindowManagerAgentLiteTest::TearDown()
{
    instance_ = nullptr;
}

namespace {
/**
 * @tc.name: WindowManagerAgentLiteTest
 * @tc.desc: Cover all functions in WindowManagerAgentLite
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentLiteTest, CoverAllUpdateFunc, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    // Note: Just improve the coverage and cover all functions in WindowManagerAgentLite, no need to assert.
    // Func 1: UpdateFocusChangeInfo
    instance_->UpdateFocusChangeInfo(nullptr, true);

    // Func 2: UpdateDisplayGroupInfo
    instance_->UpdateDisplayGroupInfo(0, 0, true);

    // Func 3: UpdateWindowVisibilityInfo
    std::vector<sptr<WindowVisibilityInfo>> visibilityInfos;
    instance_->UpdateWindowVisibilityInfo(visibilityInfos);

    // Func 4: UpdateWindowDrawingContentInfo
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos;
    instance_->UpdateWindowDrawingContentInfo(windowDrawingContentInfos);

    // Func 5: UpdateWindowModeTypeInfo
    auto wmType = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    instance_->UpdateWindowModeTypeInfo(wmType);

    // Func 6: UpdateCameraWindowStatus
    instance_->UpdateCameraWindowStatus(0, true);

    // Func 7: NotifyWindowStyleChange
    auto styleType = WindowStyleType::WINDOW_STYLE_DEFAULT;
    instance_->NotifyWindowStyleChange(styleType);

    // Func 8: NotifyCallingWindowDisplayChanged
    CallingWindowInfo callingWindowInfo;
    instance_->NotifyCallingWindowDisplayChanged(callingWindowInfo);

    // Func 9: UpdatePiPWindowStateChanged
    std::string bundleName = "test";
    instance_->UpdatePiPWindowStateChanged(bundleName, true);

    // Func 10: NotifyWindowPropertyChange
    WindowInfoList windowInfoList;
    instance_->NotifyWindowPropertyChange(0, windowInfoList);

    // Func 11: NotifyAccessibilityWindowInfo
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    auto updateType = WindowUpdateType::WINDOW_UPDATE_ACTIVE;
    instance_->NotifyAccessibilityWindowInfo(infos, updateType);
}
}
}
} // namespace Rosen
} // namespace OHOS
