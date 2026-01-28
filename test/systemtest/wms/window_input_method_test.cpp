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
#include "scene_board_judgement.h"
#include "window_test_utils.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
using Utils = WindowTestUtils;
constexpr uint32_t MAX_INT = -1;
}

class WindowInputMethodTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t TEST_SLEEP_SECOND = 1;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowInputMethodTest::SetUpTestCase()
{
    auto display = DisplayManager::GetInstance().GetDisplayById(0);
    ASSERT_TRUE((display != nullptr));
    Rect displayRect = { 0, 0, display->GetWidth(), display->GetHeight() };
    Utils::InitByDisplayRect(displayRect);
}

void WindowInputMethodTest::TearDownTestCase() {}

void WindowInputMethodTest::SetUp() {}

void WindowInputMethodTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: ShowKeyboard1
 * @tc.desc: create window and show keyboard.
 * @tc.type: FUNC
 */
HWTEST_F(WindowInputMethodTest, ShowKeyboard01, TestSize.Level1)
{
    WindowTestUtils::TestWindowInfo windowInfo = {
        .name = "ShowKeyboard",
        .rect = Utils::customAppRect_,
        .type = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = false,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& fullWindow = Utils::CreateTestWindow(windowInfo);
    ASSERT_NE(nullptr, fullWindow);
    uint32_t callingWindowId = 3;
    uint64_t targetDisplayId = 0;
    KeyboardEffectOption effectOption;
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        sleep(TEST_SLEEP_SECOND);
        EXPECT_EQ(WMError::WM_OK, fullWindow->ShowKeyboard(callingWindowId, targetDisplayId, effectOption));
        sleep(TEST_SLEEP_SECOND);
        EXPECT_EQ(WMError::WM_OK, fullWindow->ChangeKeyboardEffectOption(effectOption));
        sleep(TEST_SLEEP_SECOND);
        fullWindow->Destroy();
        GTEST_SKIP();
    }
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    EXPECT_EQ(WMError::WM_OK, fullWindow->ShowKeyboard(callingWindowId, targetDisplayId, effectOption));
    sleep(TEST_SLEEP_SECOND);

    effectOption.viewMode_ = KeyboardViewMode::LIGHT_IMMERSIVE_MODE;
    EXPECT_EQ(WMError::WM_OK, fullWindow->ChangeKeyboardEffectOption(effectOption));
    sleep(TEST_SLEEP_SECOND);

    effectOption.viewMode_ = static_cast<KeyboardViewMode>(MAX_INT);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, fullWindow->ChangeKeyboardEffectOption(effectOption));
    sleep(TEST_SLEEP_SECOND);

    EXPECT_EQ(WMError::WM_OK, fullWindow->Hide());
    sleep(TEST_SLEEP_SECOND);

    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              fullWindow->ChangeKeyboardEffectOption(effectOption));
    sleep(TEST_SLEEP_SECOND);
    fullWindow->Destroy();
}
} // namespace
} // namespace Rosen
} // namespace OHOS
