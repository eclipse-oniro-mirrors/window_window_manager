/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class WindowSceneSessionImplFilterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowSceneSessionImplFilterTest::SetUpTestCase() {}
void WindowSceneSessionImplFilterTest::TearDownTestCase() {}
void WindowSceneSessionImplFilterTest::SetUp() {}
void WindowSceneSessionImplFilterTest::TearDown() {}

namespace {
static bool KeyEventFilterFunc(Input_KeyEvent*)
{
    return false;
}
static bool MouseEventFilterFunc(Input_MouseEvent*)
{
    return false;
}
static bool TouchEventFilterFunc(Input_TouchEvent*)
{
    return false;
}

HWTEST_F(WindowSceneSessionImplFilterTest, NativeKeyEventFilter, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(nullptr, window->GetNativeKeyEventFilter());
    EXPECT_EQ(WMError::WM_OK, window->SaveNativeKeyEventFilter(KeyEventFilterFunc));
    EXPECT_EQ(KeyEventFilterFunc, window->GetNativeKeyEventFilter());
    EXPECT_EQ(WMError::WM_OK, window->ClearNativeKeyEventFilter());
    EXPECT_EQ(nullptr, window->GetNativeKeyEventFilter());
}

HWTEST_F(WindowSceneSessionImplFilterTest, NativeMouseEventFilter, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(nullptr, window->GetNativeMouseEventFilter());
    EXPECT_EQ(WMError::WM_OK, window->SaveNativeMouseEventFilter(MouseEventFilterFunc));
    EXPECT_EQ(MouseEventFilterFunc, window->GetNativeMouseEventFilter());
    EXPECT_EQ(WMError::WM_OK, window->ClearNativeMouseEventFilter());
    EXPECT_EQ(nullptr, window->GetNativeMouseEventFilter());
}

HWTEST_F(WindowSceneSessionImplFilterTest, NativeTouchEventFilter, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(nullptr, window->GetNativeTouchEventFilter());
    EXPECT_EQ(WMError::WM_OK, window->SaveNativeTouchEventFilter(TouchEventFilterFunc));
    EXPECT_EQ(TouchEventFilterFunc, window->GetNativeTouchEventFilter());
    EXPECT_EQ(WMError::WM_OK, window->ClearNativeTouchEventFilter());
    EXPECT_EQ(nullptr, window->GetNativeTouchEventFilter());
}

HWTEST_F(WindowSceneSessionImplFilterTest, KeyEventFilter, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(WMError::WM_OK, window->SetKeyEventFilter([](const MMI::KeyEvent&) { return true; }));
    EXPECT_EQ(WMError::WM_OK, window->ClearKeyEventFilter());
}

HWTEST_F(WindowSceneSessionImplFilterTest, MouseEventFilter, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(WMError::WM_OK, window->SetMouseEventFilter([](const MMI::PointerEvent&) { return true; }));
    EXPECT_EQ(WMError::WM_OK, window->ClearMouseEventFilter());
}

HWTEST_F(WindowSceneSessionImplFilterTest, TouchEventFilter, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(WMError::WM_OK, window->SetTouchEventFilter([](const MMI::PointerEvent&) { return true; }));
    EXPECT_EQ(WMError::WM_OK, window->ClearTouchEventFilter());
}

HWTEST_F(WindowSceneSessionImplFilterTest, FilterKeyEvent, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto keyEvent = MMI::KeyEvent::Create();

    EXPECT_EQ(WMError::WM_OK, window->ClearKeyEventFilter());
    EXPECT_FALSE(window->FilterKeyEvent(keyEvent));

    EXPECT_EQ(WMError::WM_OK, window->SetKeyEventFilter([](const MMI::KeyEvent&) { return true; }));
    EXPECT_TRUE(window->FilterKeyEvent(keyEvent));
}

HWTEST_F(WindowSceneSessionImplFilterTest, FilterPointerEvent01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto pointerEvent = MMI::PointerEvent::Create();

    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);

    EXPECT_EQ(WMError::WM_OK, window->ClearTouchEventFilter());
    EXPECT_FALSE(window->FilterPointerEvent(pointerEvent));

    EXPECT_EQ(WMError::WM_OK, window->SetTouchEventFilter([](const MMI::PointerEvent&) { return true; }));
    EXPECT_TRUE(window->FilterPointerEvent(pointerEvent));
}

HWTEST_F(WindowSceneSessionImplFilterTest, FilterPointerEvent02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto pointerEvent = MMI::PointerEvent::Create();

    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);

    EXPECT_EQ(WMError::WM_OK, window->ClearMouseEventFilter());
    EXPECT_FALSE(window->FilterPointerEvent(pointerEvent));

    EXPECT_EQ(WMError::WM_OK, window->SetMouseEventFilter([](const MMI::PointerEvent&) { return true; }));
    EXPECT_TRUE(window->FilterPointerEvent(pointerEvent));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
