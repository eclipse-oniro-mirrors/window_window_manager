/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_context_impl.h"
#include "display_manager_proxy.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "window_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
class MockAceAbilityHandler : public IAceAbilityHandler {
public:
    MOCK_METHOD1(SetBackgroundColor, void(uint32_t color));
    MOCK_METHOD0(GetBackgroundColor, uint32_t());
};

class WindowImplTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void CreateStretchableWindow(sptr<WindowImpl>& window, const Rect& rect);

    static inline std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};
void WindowImplTest2::SetUpTestCase() {}

void WindowImplTest2::TearDownTestCase() {}

void WindowImplTest2::SetUp() {}

void WindowImplTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

void WindowImplTest2::CreateStretchableWindow(sptr<WindowImpl>& window, const Rect& rect)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("StretchableWindowTest");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowRect({ 1, 1, 1, 1 });
    window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->windowSystemConfig_.isStretchable_ = true;
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    window->UpdateRect(rect, true, WindowSizeChangeReason::UNDEFINED);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect);
}

namespace {
/**
 * @tc.name: SetFocusable01
 * @tc.desc: create window but not show, test SetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetFocusable01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFocusable01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->RequestFocus());
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_TRUE(window->GetFocusable());
    EXPECT_CALL(m->Mock(), RequestFocus(_)).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->RequestFocus());
    ASSERT_EQ(WMError::WM_OK, window->SetFocusable(false));
    ASSERT_FALSE(window->GetFocusable());
    ASSERT_EQ(WMError::WM_OK, window->SetFocusable(true));
    ASSERT_TRUE(window->GetFocusable());
    WindowState state = window->GetWindowState();
    window->SetWindowState(WindowState::STATE_DESTROYED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFocusable(false));
    ASSERT_TRUE(window->GetFocusable());
    window->SetWindowState(state);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetFocusable02
 * @tc.desc: create window with show, test SetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetFocusable02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFocusable02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_TRUE(window->GetFocusable());
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetFocusable(false));
    ASSERT_FALSE(window->GetFocusable());
    ASSERT_EQ(WMError::WM_OK, window->SetFocusable(true));
    ASSERT_TRUE(window->GetFocusable());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: DisableAppWindowDecor01
 * @tc.desc: disable app window decor
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, DisableAppWindowDecor01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DisableAppWindowDecor01");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->property_->isDecorEnable_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    window->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    window->DisableAppWindowDecor();
    ASSERT_FALSE(window->IsDecorEnable());
    window->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->DisableAppWindowDecor();
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_FALSE(window->IsDecorEnable());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTouchHotAreas01
 * @tc.desc: create window with show, test SetTouchHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetTouchHotAreas01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTouchHotAreas01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    std::vector<Rect> requestedTouchHotAreas;
    window->GetRequestedTouchHotAreas(requestedTouchHotAreas);
    ASSERT_TRUE(requestedTouchHotAreas.empty());

    std::vector<Rect> rects;
    rects.emplace_back(Rect{ 0, 0, 720, 400 });
    rects.emplace_back(Rect{ 0, 800, 720, 300 });
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetTouchHotAreas(rects));
    window->GetRequestedTouchHotAreas(requestedTouchHotAreas);
    ASSERT_EQ(rects.size(), requestedTouchHotAreas.size());
    for (uint32_t i = 0; i < rects.size(); ++i) {
        ASSERT_TRUE(rects[i] == requestedTouchHotAreas[i]);
    }

    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    rects.clear();
    ASSERT_EQ(WMError::WM_OK, window->SetTouchHotAreas(rects));
    window->GetRequestedTouchHotAreas(requestedTouchHotAreas);
    ASSERT_TRUE(requestedTouchHotAreas.empty());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTransform01
 * @tc.desc: set transform
 * @tc.type: FUNC
 * @tc.require: issueI5NDLK
 */
HWTEST_F(WindowImplTest2, SetTransform01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform01");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->DisableAppWindowDecor();
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_SAMGR));
    Transform trans_;
    window->UpdateZoomTransform(trans_, true);
    window->SetTransform(trans_);
    window->UpdateZoomTransform(trans_, false);
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_SAMGR));
    window->SetTransform(trans_);
    ASSERT_TRUE(trans_ == window->GetTransform());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTransform02
 * @tc.desc: set transform and getTransform
 * @tc.type: FUNC
 * @tc.require: issueI5NDLK
 */
HWTEST_F(WindowImplTest2, SetTransform02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform02");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->DisableAppWindowDecor();
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_SAMGR));
    Transform trans_;
    trans_.pivotX_ = 1.0f;
    trans_.pivotY_ = 0.6f;
    window->SetTransform(trans_);
    ASSERT_TRUE(trans_ != window->GetTransform());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTransform03
 * @tc.desc: set transform and getTransform
 * @tc.type: FUNC
 * @tc.require: issueI5NDLK
 */
HWTEST_F(WindowImplTest2, SetTransform03, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform03");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->DisableAppWindowDecor();
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    Transform trans_;
    trans_.pivotX_ = 1.0f;
    trans_.pivotY_ = 0.6f;
    window->SetTransform(trans_);
    ASSERT_TRUE(trans_ == window->GetTransform());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTransform04
 * @tc.desc: set transform and getTransform
 * @tc.type: FUNC
 * @tc.require: issueI5NDLK
 */
HWTEST_F(WindowImplTest2, SetTransform04, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform04");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->DisableAppWindowDecor();
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    Transform trans_;
    trans_.pivotX_ = 1.0f;
    trans_.pivotY_ = 0.6f;
    Transform defaultTrans_;
    window->SetTransform(trans_);
    ASSERT_TRUE(defaultTrans_ != window->GetTransform());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAPPWindowLabel
 * @tc.desc: set window label to ace
 * @tc.type: FUNC
 * @tc.require: issueI5Q2KW
 */
HWTEST_F(WindowImplTest2, SetAPPWindowLabel, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAPPWindowLabel");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::string label = "openharmony";

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, SetAppWindowTitle(_));
    ASSERT_EQ(WMError::WM_OK, window->SetAPPWindowLabel(label));
    window->uiContent_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetAPPWindowLabel(label));
}

/**
 * @tc.name: SetAPPWindowIcon
 * @tc.desc: set window Icon to ace
 * @tc.type: FUNC
 * @tc.require: issueI5Q2KW
 */
HWTEST_F(WindowImplTest2, SetAPPWindowIcon, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAPPWindowIcon");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    std::shared_ptr<Media::PixelMap> icon1(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetAPPWindowIcon(icon1));
    std::shared_ptr<Media::PixelMap> icon2 = std::make_shared<Media::PixelMap>();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetAPPWindowIcon(icon2));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, SetAppWindowIcon(_));
    ASSERT_EQ(WMError::WM_OK, window->SetAPPWindowIcon(icon2));
}

/**
 * @tc.name: NotifyMemoryLevel01
 * @tc.desc: NotifyMemoryLevel without mainWindow
 * @tc.type: FUNC
 * @tc.require: issueI5JQ04
 */
HWTEST_F(WindowImplTest2, NotifyMemoryLevel01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyMemoryLevel01");
    int32_t level = 0;
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->NotifyMemoryLevel(level));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, NotifyMemoryLevel(_));
    ASSERT_EQ(WMError::WM_OK, window->NotifyMemoryLevel(level));
}

/**
 * @tc.name: StretchableUpdateRectDragStartTest
 * @tc.desc: UpdateRect test for stretchable window when drag start.
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, StretchableUpdateRectDragStartTest, TestSize.Level1)
{
    Rect rect1{ 10, 10, 10, 10 };
    sptr<WindowImpl> window;
    CreateStretchableWindow(window, rect1);
    ASSERT_NE(window, nullptr);
    Rect rect2{ 100, 100, 100, 100 };
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _));
    window->UpdateRect(rect2, true, WindowSizeChangeReason::DRAG_START);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect1);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*content, Destroy());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: StretchableUpdateRectDragTest
 * @tc.desc: UpdateRect test for stretchable window when drag.
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, StretchableUpdateRectDragTest, TestSize.Level1)
{
    Rect rect1{ 10, 10, 10, 10 };
    sptr<WindowImpl> window;
    CreateStretchableWindow(window, rect1);
    ASSERT_NE(window, nullptr);
    Rect rect2{ 100, 100, 100, 100 };
    window->UpdateRect(rect2, true, WindowSizeChangeReason::DRAG);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect1);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: StretchableUpdateRectDragEndTest
 * @tc.desc: UpdateRect test for stretchable window when drag end.
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, StretchableUpdateRectDragEndTest, TestSize.Level1)
{
    Rect rect1{ 10, 10, 10, 10 };
    sptr<WindowImpl> window;
    CreateStretchableWindow(window, rect1);
    ASSERT_NE(window, nullptr);
    Rect rect2{ 100, 100, 100, 100 };
    window->UpdateRect(rect2, true, WindowSizeChangeReason::DRAG_END);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect1);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: StretchableUpdateRectRecoverTest
 * @tc.desc: UpdateRect test for stretchable window when recover.
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, StretchableUpdateRectRecoverTest, TestSize.Level1)
{
    Rect rect1{ 10, 10, 10, 10 };
    sptr<WindowImpl> window;
    CreateStretchableWindow(window, rect1);
    ASSERT_NE(window, nullptr);
    Rect rect2{ 100, 100, 100, 100 };
    window->UpdateRect(rect2, true, WindowSizeChangeReason::RECOVER);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect1);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: StretchableUpdateRectRecoverTest
 * @tc.desc: UpdateRect test for stretchable window when move.
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, StretchableUpdateRectMoveTest, TestSize.Level1)
{
    Rect rect1{ 10, 10, 10, 10 };
    sptr<WindowImpl> window;
    CreateStretchableWindow(window, rect1);
    ASSERT_NE(window, nullptr);
    Rect rect2{ 100, 100, 100, 100 };
    window->UpdateRect(rect2, true, WindowSizeChangeReason::MOVE);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect1);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: StretchableUpdateRectResizeTest
 * @tc.desc: UpdateRect test for stretchable window when resize.
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, StretchableUpdateRectResizeTest, TestSize.Level1)
{
    Rect rect1{ 110, 110, 10, 10 };
    sptr<WindowImpl> window;
    CreateStretchableWindow(window, rect1);
    ASSERT_NE(window, nullptr);
    Rect rect2{ 100, 100, 100, 100 };
    ASSERT_EQ(true, rect1.IsInsideOf(rect2));
    ASSERT_EQ(true, rect1 != rect2);
    window->UpdateRect(rect2, true, WindowSizeChangeReason::RESIZE);
    ASSERT_EQ(window->GetWindowProperty()->GetOriginRect(), rect2);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: PrivacyMode01
 * @tc.desc: Set window privacy mode
 * @tc.type: FUNC
 * @tc.require: issueI5MYNX
 */
HWTEST_F(WindowImplTest2, PrivacyMode01, TestSize.Level1)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("PrivacyMode01");
    sptr<WindowImpl> window = new WindowImpl(option);
    ASSERT_NE(nullptr, window);

    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        window->SetPrivacyMode(true);
        window->SetSystemPrivacyMode(true);
        ASSERT_EQ(true, window->IsPrivacyMode());

        window->SetPrivacyMode(true);
        window->SetSystemPrivacyMode(false);
        ASSERT_EQ(true, window->IsPrivacyMode());

        window->SetPrivacyMode(false);
        window->SetSystemPrivacyMode(true);
        ASSERT_EQ(false, window->IsPrivacyMode());

        window->SetPrivacyMode(false);
        window->SetSystemPrivacyMode(false);
        ASSERT_EQ(false, window->IsPrivacyMode());
    } else {
        window->SetPrivacyMode(true);
        window->SetSystemPrivacyMode(true);
        ASSERT_EQ(false, window->IsPrivacyMode());

        window->SetPrivacyMode(true);
        window->SetSystemPrivacyMode(false);
        ASSERT_EQ(false, window->IsPrivacyMode());

        window->SetPrivacyMode(false);
        window->SetSystemPrivacyMode(true);
        ASSERT_EQ(false, window->IsPrivacyMode());

        window->SetPrivacyMode(false);
        window->SetSystemPrivacyMode(false);
        ASSERT_EQ(false, window->IsPrivacyMode());
    }
}

/**
 * @tc.name: CalculatePointerDirection
 * @tc.desc: calculate mouse style id
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, CalculatePointerDirection, TestSize.Level1)
{
    Rect rect1{ 1, 1, 100, 100 };
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculatePointerDirection");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    window->moveDragProperty_->startRectExceptCorner_ = rect1;
    ASSERT_EQ(12, window->CalculatePointerDirection(0, 0));
    ASSERT_EQ(6, window->CalculatePointerDirection(50, 0));
    ASSERT_EQ(11, window->CalculatePointerDirection(102, 0));
    ASSERT_EQ(5, window->CalculatePointerDirection(102, 50));
}

/**
 * @tc.name: FindWindowById
 * @tc.desc: FindWindowById Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, FindWindowById, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FindWindowById");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(nullptr, window->FindWindowById(INVALID_WINDOW_ID));
    ASSERT_EQ(sptr<Window>(window), window->FindWindowById(window->GetWindowId()));

    ASSERT_EQ(nullptr, sptr<Window>(window)->GetTopWindowWithId(INVALID_WINDOW_ID));

    ASSERT_EQ(nullptr, sptr<Window>(window)->GetTopWindowWithId(INVALID_WINDOW_ID));
    std::shared_ptr<AbilityRuntime::Context> context;
    ASSERT_EQ(nullptr, sptr<Window>(window)->GetTopWindowWithContext(context));
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetSubWindow
 * @tc.desc: GetSubWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, GetSubWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("main");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    option = new WindowOption();
    option->SetWindowName("sub");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowImpl> subWindow = new WindowImpl(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, subWindow->Create(window->GetWindowId()));

    auto subWindowVec = sptr<Window>(window)->GetSubWindow(window->GetWindowId());
    if (subWindowVec.size() == 1) {
        ASSERT_EQ(1, subWindowVec.size());
        ASSERT_EQ(sptr<Window>(subWindow), subWindowVec[0]);
    }
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateConfigurationForAll
 * @tc.desc: UpdateConfigurationForAll Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, UpdateConfigurationForAll, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    sptr<Window>(window)->UpdateConfigurationForAll(configuration);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAlpha
 * @tc.desc: SetAlpha | GetAlpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetAlpha, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    float alpha = 0.5f;
    window->SetAlpha(alpha);
    ASSERT_EQ(alpha, window->GetAlpha());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: Test GetAvoidAreaByType when ret is not ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, GetAvoidAreaByType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    EXPECT_CALL(m->Mock(), GetAvoidAreaByType(_, _, _, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    AvoidArea avoidarea;
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea));
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetWindowType
 * @tc.desc: SetWindowType Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetWindowType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    window->SetWindowState(WindowState::STATE_DESTROYED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW));

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetWindowType(WindowType::SYSTEM_WINDOW_END));

    window->SetWindowState(WindowState::STATE_FROZEN);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetWindowMode
 * @tc.desc: SetWindowMode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetWindowMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    window->SetWindowState(WindowState::STATE_DESTROYED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, HideWindowTitleButton(_, _, _, _));
    EXPECT_CALL(*content, UpdateWindowMode(_, _));
    ASSERT_EQ(WMError::WM_OK, window->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    window->uiContent_ = nullptr;

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    window->UpdateWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, window->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    window->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL);
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    window->SetWindowState(WindowState::STATE_FROZEN);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: WindowFlag
 * @tc.desc: GetWindowFlags | SetWindowFlags | AddWindowFlag | RemoveWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, WindowFlag, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowFlag");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->AddWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    ASSERT_EQ(static_cast<uint32_t>(0), window->GetWindowFlags());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_OK, window->SetWindowFlags(static_cast<uint32_t>(0)));
    ASSERT_EQ(static_cast<uint32_t>(0), window->GetWindowFlags());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: OnNewWant
 * @tc.desc: OnNewWant test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, OnNewWant, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnNewWant");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    AAFwk::Want want;
    EXPECT_CALL(*content, OnNewWant(_)).Times(1).WillOnce(Return());
    window->OnNewWant(want);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*content, Destroy());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetContentInfo
 * @tc.desc: GetContentInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, GetContentInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetContentInfo");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(std::string(""), window->GetContentInfo());
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, GetContentInfo(_)).Times(1).WillOnce(Return("info"));
    ASSERT_EQ(std::string("info"), window->GetContentInfo());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*content, Destroy());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Snapshot
 * @tc.desc: Snapshot test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, Snapshot, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Snapshot");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    ASSERT_EQ(nullptr, window->Snapshot());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SnapshotIgnorePrivacy
 * @tc.desc: SnapshotIgnorePrivacy test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SnapshotIgnorePrivacy, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SnapshotIgnorePrivacy");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SnapshotIgnorePrivacy(pixelMap));
}

/**
 * @tc.name: DumpInfo
 * @tc.desc: DumpInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, DumpInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DumpInfo");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    std::vector<std::string> params{ "-h" };
    std::vector<std::string> info{ "" };
    window->DumpInfo(params, info);
    params.push_back("");
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, DumpInfo(_, _)).Times(1).WillOnce(Return());
    window->DumpInfo(params, info);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*content, Destroy());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetLayoutFullScreen
 * @tc.desc: SetLayoutFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetLayoutFullScreen, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetLayoutFullScreen");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->UpdateWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(true));
    window->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL);

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetLayoutFullScreen(true));

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(true));

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetLayoutFullScreen(true));

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() &
                                      (~static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(false));

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() &
                                      (~static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetLayoutFullScreen(false));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetImmersiveModeEnabledState
 * @tc.desc: SetImmersiveModeEnabledState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetImmersiveModeEnabledState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetImmersiveModeEnabledState");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->UpdateWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetImmersiveModeEnabledState(true));
    window->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL);

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetImmersiveModeEnabledState(true));

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(true));
    ASSERT_EQ(true, window->GetImmersiveModeEnabledState());

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetImmersiveModeEnabledState(true));

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() &
                                      (~static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(false));
    ASSERT_EQ(false, window->GetImmersiveModeEnabledState());

    window->property_->SetWindowFlags(window->property_->GetWindowFlags() &
                                      (~static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetImmersiveModeEnabledState(false));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetFullScreen
 * @tc.desc: SetFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetFullScreen, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFullScreen");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    window->UpdateWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(true));
    window->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL);

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(3).WillRepeatedly(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetFullScreen(true));

    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = false;
    window->property_->SetWindowFlags(window->property_->GetWindowFlags() &
                                      (~static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).Times(3).WillRepeatedly(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetFullScreen(false));

    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->SetFullScreen(true));

    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
    window->property_->SetWindowFlags(window->property_->GetWindowFlags() |
                                      (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(3)
        .WillOnce(Return(WMError::WM_DO_NOTHING))
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetFullScreen(true));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateTitleButtonVisibility
 * @tc.desc: UpdateTitleButtonVisibility test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, UpdateTitleButtonVisibility, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("BindDialogTarget");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, HideWindowTitleButton(_, _, _, _));
    window->UpdateTitleButtonVisibility();

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*content, Destroy());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: WindowCreateCheck
 * @tc.desc: WindowCreateCheck test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, WindowCreateCheck, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    ASSERT_EQ(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, window->GetType());
    ASSERT_NE(WMError::WM_OK, window->WindowCreateCheck(INVALID_WINDOW_ID));
    ASSERT_NE(WMError::WM_OK, window->WindowCreateCheck(static_cast<uint32_t>(-1)));
}

/**
 * @tc.name: BindDialogTarget
 * @tc.desc: BindDialogTarget test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, BindDialogTarget, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("BindDialogTarget");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    window->SetWindowState(WindowState::STATE_DESTROYED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    EXPECT_CALL(m->Mock(), BindDialogTarget(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    sptr<IRemoteObject> targetToken;
    ASSERT_EQ(WMError::WM_OK, window->BindDialogTarget(targetToken));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->BindDialogTarget(targetToken));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Destroy
 * @tc.desc: Destroy test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, Destroy, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Destroy");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Destroy());

    window->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_EQ(WindowType::WINDOW_TYPE_DIALOG, window->GetType());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Destroy());
}

/**
 * @tc.name: UpdateSurfaceNodeAfterCustomAnimation
 * @tc.desc: UpdateSurfaceNodeAfterCustomAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, UpdateSurfaceNodeAfterCustomAnimation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSurfaceNodeAfterCustomAnimation");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->UpdateSurfaceNodeAfterCustomAnimation(true));

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->UpdateSurfaceNodeAfterCustomAnimation(true));

    window->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->UpdateSurfaceNodeAfterCustomAnimation(true));

    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), UpdateRsTree(_, _)).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->UpdateSurfaceNodeAfterCustomAnimation(true));

    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), UpdateRsTree(_, _)).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->UpdateSurfaceNodeAfterCustomAnimation(true));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: ShowHide
 * @tc.desc: Show and Hide test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, ShowHide, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShowHide");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Show());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Hide());
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    auto state = window->GetWindowState();
    window->SetWindowState(WindowState::STATE_FROZEN);
    ASSERT_EQ(WindowState::STATE_FROZEN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->PreProcessShow(0, false));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Show());

    window->SetWindowState(state);
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show(static_cast<uint32_t>(WindowStateChangeReason::KEYGUARD)));
    EXPECT_CALL(m->Mock(), ProcessPointDown(_, _));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    ASSERT_EQ(WindowType::WINDOW_TYPE_DESKTOP, window->GetType());
    EXPECT_CALL(m->Mock(), MinimizeAllAppWindows(_, _));
    ASSERT_EQ(WMError::WM_OK, window->Show());

    EXPECT_CALL(m->Mock(), UpdateProperty(_, _)).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Hide());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    ASSERT_EQ(WMError::WM_OK, window->Hide(static_cast<uint32_t>(WindowStateChangeReason::TOGGLING)));
    ASSERT_EQ(WindowState::STATE_HIDDEN, window->GetWindowState());

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, window->Show());
    ASSERT_EQ(WindowState::STATE_HIDDEN, window->GetWindowState());

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Show());
    ASSERT_EQ(WindowState::STATE_HIDDEN, window->GetWindowState());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: CloseWindow
 * @tc.desc: CloseWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, CloseWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CloseWindow");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_EQ(WMError::WM_OK, window->Close());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: PendingCloseWindow
 * @tc.desc: PendingCloseWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, PendingCloseWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->PendingClose();
    ASSERT_EQ(nullptr, window->context_);
}

/**
 * @tc.name: InvalidWindow
 * @tc.desc: InvalidWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, InvalidWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->MoveTo(0, 0));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Resize(0, 0));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->ChangeCallingWindowId(INVALID_WINDOW_ID));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetBackgroundColor(std::string("???")));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Close());
}

/**
 * @tc.name: BackgroundColor
 * @tc.desc: GetBackgroundColor | SetBackGroundColor test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, BackgroundColor, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->SetBackgroundColor(0));

    MockAceAbilityHandler* aceAbilityHandler = new MockAceAbilityHandler();
    window->aceAbilityHandler_ = aceAbilityHandler;
    EXPECT_CALL(*aceAbilityHandler, SetBackgroundColor(_));
    ASSERT_EQ(WMError::WM_OK, window->SetBackgroundColor(0));
    EXPECT_CALL(*aceAbilityHandler, GetBackgroundColor()).WillOnce(Return(0));
    ASSERT_EQ(0, window->GetBackgroundColor());

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, SetBackgroundColor(_));
    ASSERT_EQ(WMError::WM_OK, window->SetBackgroundColor(0));
    EXPECT_CALL(*content, GetBackgroundColor()).WillOnce(Return(0));
    ASSERT_EQ(0, window->GetBackgroundColor());
}

/**
 * @tc.name: SetTransparent
 * @tc.desc: SetTransparent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, SetTransparent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransparent");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTransparent(true));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->SetTransparent(true));
    ASSERT_EQ(WMError::WM_OK, window->SetTransparent(false));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: TransferLifeCycleEventToString
 * @tc.desc: TransferLifeCycleEventToString test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, TransferLifeCycleEventToString, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(std::string("DESTROY"), window->TransferLifeCycleEventToString(LifeCycleEvent::DESTROY_EVENT));
    ASSERT_EQ(std::string("UNDEFINE"),
              window->TransferLifeCycleEventToString(static_cast<LifeCycleEvent>(uint32_t(-1))));
}

/**
 * @tc.name: NotifyWindowTransition
 * @tc.desc: NotifyWindowTransition test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, NotifyWindowTransition, TestSize.Level1)
{
    NotifyNativeWinDestroyFunc func = [](std::string) {};
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveDrag");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    window->RegisterWindowDestroyedListener(func);
    window->RestoreSplitWindowMode(0u);

    ASSERT_EQ(WMError::WM_ERROR_NO_MEM, window->NotifyWindowTransition(TransitionReason::ABILITY_TRANSITION));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Close());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateConfigurationSyncForAll
 * @tc.desc: UpdateConfigurationSyncForAll Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest2, UpdateConfigurationSyncForAll, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    WindowImpl::UpdateConfigurationSyncForAll(configuration);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}
} // namespace
} // namespace Rosen
} // namespace OHOS
