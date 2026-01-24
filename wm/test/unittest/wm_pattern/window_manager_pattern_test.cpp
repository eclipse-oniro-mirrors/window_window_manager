/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "window_adapter.h"
#include "iremote_object_mocker.h"
#include "scene_board_judgement.h"
#include "window_manager.cpp"
#include "window_manager.h"
#include "window_manager_hilog.h"



using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowManagerPatternTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowManagerPatternTest::SetUpTestCase() {}

void WindowManagerPatternTest::TearDownTestCase() {}

void WindowManagerPatternTest::SetUp() {}

void WindowManagerPatternTest::TearDown() {}


namespace {

/**
 * @tc.name: SetStartWindowBackgroundColor
 * @tc.desc: WindowAdapter/SetStartWindowBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerPatternTest, SetStartWindowBackgroundColor, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    const std::string& moduleName = "testModuleName";
    const std::string& abilityName = "testAbilityName";
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.SetStartWindowBackgroundColor(moduleName, abilityName, 0, 0));
}

} // namespace
} // namespace Rosen
} // namespace OHOS
