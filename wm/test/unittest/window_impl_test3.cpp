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

class MockAnimationTransitionController : public IAnimationTransitionController {
public:
    MOCK_METHOD0(AnimationForShown, void());
    MOCK_METHOD0(AnimationForHidden, void());
};

class MockDialogDeathRecipientListener : public IDialogDeathRecipientListener {
public:
    MOCK_CONST_METHOD0(OnDialogDeathRecipient, void());
};

class MockScreenshotListener : public IScreenshotListener {
public:
    MOCK_METHOD0(OnScreenshot, void());
};

class MockScreenshotAppEventListener : public IScreenshotAppEventListener {
public:
    MOCK_METHOD1(OnScreenshotAppEvent, void(ScreenshotEventType type));
};

class MockDialogTargetTouchListener : public IDialogTargetTouchListener {
public:
    MOCK_CONST_METHOD0(OnDialogTargetTouch, void());
};

class MockWindowChangeListener : public IWindowChangeListener {
public:
    MOCK_METHOD3(OnSizeChange,
                 void(Rect rect, WindowSizeChangeReason reason, const std::shared_ptr<RSTransaction>& rsTransaction));
    MOCK_METHOD2(OnModeChange, void(WindowMode mode, bool hasDeco));
    MOCK_METHOD1(NotifyTransformChange, void(const Transform& transform));
};

class MockAvoidAreaChangedListener : public IAvoidAreaChangedListener {
public:
    MOCK_METHOD3(OnAvoidAreaChanged, void(const AvoidArea avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info));
};

class MockDisplayMoveListener : public IDisplayMoveListener {
public:
    MOCK_METHOD2(OnDisplayMove, void(DisplayId from, DisplayId to));
};

class MockInputEventConsumer : public IInputEventConsumer {
public:
    MOCK_CONST_METHOD1(OnInputEvent, bool(const std::shared_ptr<MMI::KeyEvent>&));
    MOCK_CONST_METHOD1(OnInputEvent, bool(const std::shared_ptr<MMI::PointerEvent>&));
    MOCK_CONST_METHOD1(OnInputEvent, bool(const std::shared_ptr<MMI::AxisEvent>&));
};

class MockKeyEvent : public MMI::KeyEvent {
public:
    MockKeyEvent() : MMI::KeyEvent(0) {}
};

class MockPointerEvent : public MMI::PointerEvent {
public:
    MockPointerEvent() : MMI::PointerEvent(0) {}
};

class WindowImplTest3 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static inline std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};
void WindowImplTest3::SetUpTestCase() {}

void WindowImplTest3::TearDownTestCase() {}

void WindowImplTest3::SetUp() {}

void WindowImplTest3::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: RegisterAnimationTransitionController
 * @tc.desc: RegisterAnimationTransitionController test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, RegisterAnimationTransitionController, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    sptr<IAnimationTransitionController> listener;
    ASSERT_EQ(nullptr, listener);
    window->RegisterAnimationTransitionController(listener);
    listener = sptr<MockAnimationTransitionController>::MakeSptr();
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, SetNextFrameLayoutCallback(_));
    window->RegisterAnimationTransitionController(listener);
    EXPECT_CALL(*content, SetNextFrameLayoutCallback(_));
    window->RegisterAnimationTransitionController(listener);
    window->property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
    EXPECT_CALL(*content, SetNextFrameLayoutCallback(_));
    window->RegisterAnimationTransitionController(listener);
}

/**
 * @tc.name: RegisterDialogDeathRecipientListener
 * @tc.desc: RegisterDialogDeathRecipientListener | NotifyDestroy test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, RegisterDialogDeathRecipientListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockDialogDeathRecipientListener> listener;
    ASSERT_EQ(nullptr, listener);
    window->RegisterDialogDeathRecipientListener(sptr<IDialogDeathRecipientListener>(listener));
    listener = sptr<MockDialogDeathRecipientListener>::MakeSptr();
    window->RegisterDialogDeathRecipientListener(sptr<IDialogDeathRecipientListener>(listener));
    EXPECT_CALL(*listener, OnDialogDeathRecipient());
    window->NotifyDestroy();
    window->UnregisterDialogDeathRecipientListener(sptr<IDialogDeathRecipientListener>(listener));
    window->NotifyDestroy();
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: NotifyScreenshot test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyScreenshot, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockScreenshotListener> listener;
    window->screenshotListeners_[window->GetWindowId()].push_back(sptr<IScreenshotListener>(listener));
    listener = sptr<MockScreenshotListener>::MakeSptr();
    window->screenshotListeners_[window->GetWindowId()].push_back(sptr<IScreenshotListener>(listener));
    EXPECT_CALL(*listener, OnScreenshot()).Times(1);
    window->NotifyScreenshot();
    window->screenshotListeners_[window->GetWindowId()].clear();
}

/**
 * @tc.name: NotifyScreenshotAppEvent
 * @tc.desc: NotifyScreenshotAppEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyScreenshotAppEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;

    sptr<MockScreenshotAppEventListener> listener;
    window->screenshotAppEventListeners_[window->GetWindowId()].push_back(sptr<IScreenshotAppEventListener>(listener));
    listener = sptr<MockScreenshotAppEventListener>::MakeSptr();
    window->screenshotAppEventListeners_[window->GetWindowId()].push_back(sptr<IScreenshotAppEventListener>(listener));
    EXPECT_CALL(*listener, OnScreenshotAppEvent(_)).Times(1);
    auto ret = window->NotifyScreenshotAppEvent(type);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->screenshotAppEventListeners_[window->GetWindowId()].clear();
}

/**
 * @tc.name: NotifyTouchDialogTarget
 * @tc.desc: NotifyTouchDialogTarget test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyTouchDialogTarget, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockDialogTargetTouchListener> listener;
    window->dialogTargetTouchListeners_[window->GetWindowId()].push_back(sptr<IDialogTargetTouchListener>(listener));
    listener = sptr<MockDialogTargetTouchListener>::MakeSptr();
    window->dialogTargetTouchListeners_[window->GetWindowId()].push_back(sptr<IDialogTargetTouchListener>(listener));
    EXPECT_CALL(*listener, OnDialogTargetTouch());
    EXPECT_CALL(m->Mock(), ProcessPointDown(_, _));
    window->NotifyTouchDialogTarget();
    window->dialogTargetTouchListeners_[window->GetWindowId()].clear();
}

/**
 * @tc.name: NotifySizeChange
 * @tc.desc: NotifySizeChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifySizeChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockWindowChangeListener> listener;
    window->windowChangeListeners_[window->GetWindowId()].push_back(sptr<IWindowChangeListener>(listener));
    listener = sptr<MockWindowChangeListener>::MakeSptr();
    window->windowChangeListeners_[window->GetWindowId()].push_back(sptr<IWindowChangeListener>(listener));
    EXPECT_CALL(*listener, OnSizeChange(_, _, _));
    Rect rect;
    window->NotifySizeChange(rect, WindowSizeChangeReason::UNDEFINED);
    window->windowChangeListeners_[window->GetWindowId()].clear();
}

/**
 * @tc.name: NotifyModeChange
 * @tc.desc: NotifyModeChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyModeChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockWindowChangeListener> listener;
    window->windowChangeListeners_[window->GetWindowId()].push_back(sptr<IWindowChangeListener>(listener));
    listener = sptr<MockWindowChangeListener>::MakeSptr();
    window->windowChangeListeners_[window->GetWindowId()].push_back(sptr<IWindowChangeListener>(listener));
    EXPECT_CALL(*listener, OnModeChange(_, _));
    window->NotifyModeChange(WindowMode::WINDOW_MODE_UNDEFINED);
    window->windowChangeListeners_[window->GetWindowId()].clear();
}

/**
 * @tc.name: NotifyAvoidAreaChange
 * @tc.desc: NotifyAvoidAreaChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyAvoidAreaChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockAvoidAreaChangedListener> listener;
    window->avoidAreaChangeListeners_[window->GetWindowId()].push_back(sptr<IAvoidAreaChangedListener>(listener));
    listener = sptr<MockAvoidAreaChangedListener>::MakeSptr();
    window->avoidAreaChangeListeners_[window->GetWindowId()].push_back(sptr<IAvoidAreaChangedListener>(listener));
    EXPECT_CALL(*listener, OnAvoidAreaChanged(_, _, _));
    sptr<AvoidArea> avoidArea = sptr<AvoidArea>::MakeSptr();
    window->NotifyAvoidAreaChange(avoidArea, AvoidAreaType::TYPE_CUTOUT);
    window->avoidAreaChangeListeners_[window->GetWindowId()].clear();
}

/**
 * @tc.name: NotifyDisplayMoveChange
 * @tc.desc: NotifyDisplayMoveChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyDisplayMoveChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<MockDisplayMoveListener> listener;
    window->displayMoveListeners_.push_back(sptr<IDisplayMoveListener>(listener));
    listener = sptr<MockDisplayMoveListener>::MakeSptr();
    window->displayMoveListeners_.push_back(sptr<IDisplayMoveListener>(listener));
    EXPECT_CALL(*listener, OnDisplayMove(_, _));
    window->NotifyDisplayMoveChange(DisplayId{}, DisplayId{});
}

/**
 * @tc.name: SetAceAbilityHandler
 * @tc.desc: SetAceAbilityHandler test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetAceAbilityHandler, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<IAceAbilityHandler> handler;
    ASSERT_EQ(nullptr, handler);
    window->SetAceAbilityHandler(handler);
    handler = sptr<MockAceAbilityHandler>::MakeSptr();
    window->SetAceAbilityHandler(handler);
}

/**
 * @tc.name: HandleBackKeyPressedEvent
 * @tc.desc: HandleBackKeyPressedEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, HandleBackKeyPressedEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, window->GetType());
    std::shared_ptr<MMI::KeyEvent> keyEvent;
    window->HandleBackKeyPressedEvent(keyEvent);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, ProcessBackPressed()).WillOnce(Return(false));
    window->HandleBackKeyPressedEvent(keyEvent);

    window->inputEventConsumer_.reset(new MockInputEventConsumer);
    EXPECT_CALL(*reinterpret_cast<MockInputEventConsumer*>(window->inputEventConsumer_.get()), OnInputEvent(keyEvent))
        .WillOnce(Return(true));
    window->HandleBackKeyPressedEvent(keyEvent);
}

/**
 * @tc.name: ConsumeKeyEvent
 * @tc.desc: ConsumeKeyEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, ConsumeKeyEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::shared_ptr<MMI::KeyEvent> keyEvent = std::make_shared<MockKeyEvent>();
    EXPECT_CALL(m->Mock(), DispatchKeyEvent(_, _));
    window->ConsumeKeyEvent(keyEvent);
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, ProcessKeyEvent(_, _));
    window->ConsumeKeyEvent(keyEvent);

    window->inputEventConsumer_.reset(new MockInputEventConsumer);
    EXPECT_CALL(*reinterpret_cast<MockInputEventConsumer*>(window->inputEventConsumer_.get()), OnInputEvent(keyEvent));
    window->ConsumeKeyEvent(keyEvent);

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    window->inputEventConsumer_ = nullptr;
    EXPECT_CALL(*content, ProcessBackPressed());
    window->ConsumeKeyEvent(keyEvent);
}

/**
 * @tc.name: ConsumePointerEvent
 * @tc.desc: ConsumePointerEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, ConsumePointerEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_LAUNCHER_RECENT);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    Rect rect{ 0, 0, 10u, 10u };
    window->property_->SetWindowRect(rect);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = std::make_shared<MockPointerEvent>();
    MMI::PointerEvent::PointerItem item;
    pointerEvent->SetPointerId(0);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    window->ConsumePointerEvent(pointerEvent);

    item.SetPointerId(0);
    item.SetDisplayX(15); // 15 : position x
    item.SetDisplayY(15); // 15 : position y
    pointerEvent->AddPointerItem(item);
    window->ConsumePointerEvent(pointerEvent);

    item.SetDisplayX(5); // 5 : position x
    item.SetDisplayY(5); // 5 : position y
    pointerEvent->UpdatePointerItem(0, item);
    EXPECT_CALL(m->Mock(), ProcessPointDown(_, _));
    window->ConsumePointerEvent(pointerEvent);
}

/**
 * @tc.name: HandleModeChangeHotZones
 * @tc.desc: HandleModeChangeHotZones test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, HandleModeChangeHotZones, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->HandleModeChangeHotZones(0, 0);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_CALL(m->Mock(), GetModeChangeHotZones(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    window->HandleModeChangeHotZones(0, 0);
    EXPECT_CALL(m->Mock(), GetModeChangeHotZones(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    window->HandleModeChangeHotZones(0, 0);
}

/**
 * @tc.name: UpdatePointerEventForStretchableWindow
 * @tc.desc: UpdatePointerEventForStretchableWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdatePointerEventForStretchableWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->property_->SetWindowRect(Rect{ 0, 0, 10, 10 });
    window->property_->SetOriginRect(Rect{ 0, 0, 100, 100 });
    std::shared_ptr<MMI::PointerEvent> pointerEvent = std::make_shared<MockPointerEvent>();
    MMI::PointerEvent::PointerItem item;
    ASSERT_FALSE(pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), item));
    window->UpdatePointerEventForStretchableWindow(pointerEvent);
    pointerEvent->SetPointerId(0);
    item.SetPointerId(0);
    item.SetDisplayX(5); // 5 : position x
    item.SetDisplayY(5); // 5 : position y
    pointerEvent->AddPointerItem(item);
    window->UpdatePointerEventForStretchableWindow(pointerEvent);
    ASSERT_TRUE(pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), item));
    ASSERT_EQ(50, item.GetDisplayX());
    ASSERT_EQ(50, item.GetDisplayY());
}

/**
 * @tc.name: MoveDrag
 * @tc.desc: StartMove | ReadyToMoveOrDragWindow |  EndMoveOrDragWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, MoveDrag, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveDrag");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    window->RestoreSplitWindowMode(0u);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    window->Show();
    window->moveDragProperty_->startDragFlag_ = false;
    window->moveDragProperty_->pointEventStarted_ = true;
    window->StartMove();
    window->moveDragProperty_->pointEventStarted_ = false;
    window->StartMove();

    std::shared_ptr<MMI::PointerEvent> pointerEvent = std::make_shared<MockPointerEvent>();
    MMI::PointerEvent::PointerItem item;
    pointerEvent->SetTargetDisplayId(0);
    item.SetDisplayX(10000);
    item.SetDisplayY(10000);

    window->moveDragProperty_->pointEventStarted_ = true;
    window->ReadyToMoveOrDragWindow(pointerEvent, item);
    window->moveDragProperty_->startMoveFlag_ = true;
    window->moveDragProperty_->startDragFlag_ = true;
    EXPECT_CALL(m->Mock(), ProcessPointUp(_)).Times(2);
    window->EndMoveOrDragWindow(
        uint32_t(), uint32_t(), window->moveDragProperty_->startPointerId_, window->moveDragProperty_->sourceType_);

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: TransferPointerEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, TransferPointerEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = std::make_shared<MockPointerEvent>();
    window->windowSystemConfig_.isStretchable_ = true;
    window->TransferPointerEvent(pointerEvent);
    window->windowSystemConfig_.isStretchable_ = false;
    window->TransferPointerEvent(pointerEvent);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, ProcessPointerEvent(_));
    window->TransferPointerEvent(pointerEvent);

    window->inputEventConsumer_.reset(new MockInputEventConsumer);
    EXPECT_CALL(*reinterpret_cast<MockInputEventConsumer*>(window->inputEventConsumer_.get()),
                OnInputEvent(pointerEvent));
    window->TransferPointerEvent(pointerEvent);
}

/**
 * @tc.name: UpdateConfiguration
 * @tc.desc: UpdateConfiguration test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateConfiguration, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfiguration");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->subWindowMap_.clear();
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    window->UpdateConfiguration(configuration);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->UpdateConfiguration(configuration);

    window->subWindowMap_[window->GetWindowId()].push_back(nullptr);
    window->UpdateConfiguration(configuration);

    option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowName("subwindow");
    sptr<WindowImpl> subWindow = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_NE(subWindow, nullptr);
    subWindow->property_->SetWindowId(8);
    window->subWindowMap_[window->GetWindowId()].push_back(subWindow);
    window->UpdateConfiguration(configuration);
    window->subWindowMap_.clear();
}

/**
 * @tc.name: UpdateConfigurationForSpecified
 * @tc.desc: UpdateConfigurationForSpecified test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateConfigurationForSpecified, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfigurationForSpecified");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->subWindowMap_.clear();
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    window->UpdateConfigurationForSpecified(configuration, resourceManager);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->UpdateConfigurationForSpecified(configuration, resourceManager);

    window->subWindowMap_[window->GetWindowId()].push_back(nullptr);
    window->UpdateConfigurationForSpecified(configuration, resourceManager);

    option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowName("subwindow");
    sptr<WindowImpl> subWindow = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_NE(subWindow, nullptr);
    subWindow->property_->SetWindowId(8);
    window->subWindowMap_[window->GetWindowId()].push_back(subWindow);
    window->UpdateConfigurationForSpecified(configuration, resourceManager);
    window->subWindowMap_.clear();
}

/**
 * @tc.name: UpdateWindowState
 * @tc.desc: UpdateWindowState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateWindowState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowState");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    window->RestoreSplitWindowMode(0u);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    window->UpdateWindowState(WindowState::STATE_FROZEN);
    window->UpdateWindowState(WindowState::STATE_UNFROZEN);
    window->UpdateWindowState(WindowState::STATE_SHOWN);
    EXPECT_CALL(m->Mock(), RemoveWindow(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    window->UpdateWindowState(WindowState::STATE_HIDDEN);
    window->UpdateWindowState(WindowState::STATE_INITIAL);
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RestoreSplitWindowMode
 * @tc.desc: RestoreSplitWindowMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, RestoreSplitWindowMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RestoreSplitWindowMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    window->RestoreSplitWindowMode(0u);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->RestoreSplitWindowMode(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    window->RestoreSplitWindowMode(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_SECONDARY));
    window->RestoreSplitWindowMode(static_cast<uint32_t>(WindowMode::WINDOW_MODE_UNDEFINED));
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsFocused
 * @tc.desc: IsFocused test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, IsFocused, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsFocused");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    window->UpdateFocusStatus(false);
    ASSERT_FALSE(window->IsFocused());
    window->UpdateFocusStatus(true);
    ASSERT_TRUE(window->IsFocused());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateSubWindowStateAndNotify
 * @tc.desc: update subwindow state
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateSubWindowStateAndNotify, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("main");
    sptr<WindowImpl> mainWindow = sptr<WindowImpl>::MakeSptr(option);

    ASSERT_EQ(WMError::WM_OK, mainWindow->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(WmErrorCode::WM_OK, mainWindow->UpdateSubWindowStateAndNotify(mainWindow->GetWindowId()));

    option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("sub");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowImpl> subWindow = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, subWindow->Create(mainWindow->GetWindowId()));

    // main window hide
    mainWindow->state_ = WindowState::STATE_HIDDEN;
    subWindow->state_ = WindowState::STATE_SHOWN;
    subWindow->subWindowState_ = WindowState::STATE_SHOWN;
    mainWindow->UpdateSubWindowStateAndNotify(mainWindow->GetWindowId());
    ASSERT_EQ(subWindow->subWindowState_, WindowState::STATE_HIDDEN);

    // main window show
    mainWindow->state_ = WindowState::STATE_SHOWN;
    subWindow->state_ = WindowState::STATE_SHOWN;
    subWindow->subWindowState_ = WindowState::STATE_HIDDEN;
    mainWindow->UpdateSubWindowStateAndNotify(mainWindow->GetWindowId());
    ASSERT_EQ(subWindow->subWindowState_, WindowState::STATE_SHOWN);

    subWindow->state_ = WindowState::STATE_HIDDEN;
    mainWindow->UpdateSubWindowStateAndNotify(mainWindow->GetWindowId());
    ASSERT_EQ(subWindow->subWindowState_, WindowState::STATE_HIDDEN);

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, mainWindow->Destroy());
}

/**
 * @tc.name: UpdateWindowStateWhenHide
 * @tc.desc: UpdateWindowStateWhenHide test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateWindowStateWhenHide, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("main");
    sptr<WindowImpl> mainWindow = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, mainWindow->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(WmErrorCode::WM_OK, mainWindow->UpdateWindowStateWhenHide());

    option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("sub");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowImpl> subWindow = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, subWindow->Create(mainWindow->GetWindowId()));
    subWindow->subWindowState_ = WindowState::STATE_SHOWN;
    subWindow->UpdateWindowStateWhenHide();
    ASSERT_EQ(subWindow->subWindowState_, WindowState::STATE_HIDDEN);
    subWindow->subWindowState_ = WindowState::STATE_HIDDEN;
    subWindow->UpdateWindowStateWhenHide();
    ASSERT_EQ(subWindow->subWindowState_, WindowState::STATE_HIDDEN);

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, mainWindow->Destroy());
    ASSERT_EQ(WMError::WM_OK, subWindow->Destroy());
}

/**
 * @tc.name: UpdateWindowStateWhenShow
 * @tc.desc: UpdateWindowStateWhenShow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateWindowStateWhenShow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("main");
    sptr<WindowImpl> mainWindow = sptr<WindowImpl>::MakeSptr(option);
    if (mainWindow->Create(INVALID_WINDOW_ID) != WMError::WM_OK) {
        ASSERT_NE(WMError::WM_OK, mainWindow->Create(INVALID_WINDOW_ID));
    }

    ASSERT_EQ(WmErrorCode::WM_OK, mainWindow->UpdateWindowStateWhenShow());

    option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("sub");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowImpl> subWindow = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, subWindow->Create(mainWindow->GetWindowId()));
    ASSERT_EQ(WmErrorCode::WM_OK, subWindow->UpdateWindowStateWhenShow());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, mainWindow->Destroy());
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: RaiseToAppTop test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, RaiseToAppTop, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->parentId_ = INVALID_WINDOW_ID;
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->RaiseToAppTop());

    window->SetWindowState(WindowState::STATE_CREATED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT, window->RaiseToAppTop());

    window->property_->parentId_ = 100000;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->RaiseToAppTop());

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->RaiseToAppTop());

    window->state_ = WindowState::STATE_SHOWN;
    EXPECT_CALL(m->Mock(), RaiseToAppTop(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->RaiseToAppTop());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateDecorEnable
 * @tc.desc: UpdateDecorEnable test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateDecorEnable, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->property_->mode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.decorWindowModeSupportType_ = WINDOW_MODE_SUPPORT_FLOATING;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    sptr<MockWindowChangeListener> listener = sptr<MockWindowChangeListener>::MakeSptr();
    window->RegisterWindowChangeListener(sptr<IWindowChangeListener>(listener));

    EXPECT_CALL(*content, UpdateWindowMode(_, _));
    EXPECT_CALL(*listener, OnModeChange(_, _));
    window->UpdateDecorEnable(true);
    ASSERT_TRUE(window->IsDecorEnable());
    ASSERT_FALSE(window->property_->GetDecorEnable());
    window->windowSystemConfig_.decorWindowModeSupportType_ = WINDOW_MODE_SUPPORT_ALL;
    window->UpdateDecorEnable();
    ASSERT_TRUE(window->IsDecorEnable());
    ASSERT_TRUE(window->property_->GetDecorEnable());
    window->property_->type_ = WindowType::WINDOW_TYPE_FLOAT;
    window->UpdateDecorEnable();
    ASSERT_FALSE(window->IsDecorEnable());
    ASSERT_FALSE(window->property_->GetDecorEnable());
    window->UnregisterWindowChangeListener(sptr<IWindowChangeListener>(listener));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Find01
 * @tc.desc: Find one exit window
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, Find01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Find01");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(nullptr, WindowImpl::Find("Find01"));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Find02
 * @tc.desc: Find one no exit window
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, Find02, TestSize.Level1)
{
    ASSERT_EQ(nullptr, WindowImpl::Find("Find02"));
}

/**
 * @tc.name: Find03
 * @tc.desc: Find window with empty name
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, Find03, TestSize.Level1)
{
    ASSERT_EQ(nullptr, WindowImpl::Find(""));
}

/**
 * @tc.name: FindWindowById02
 * @tc.desc: Find one top window
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, FindWindowById02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FindWindowById02");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));

    ASSERT_NE(nullptr, window->FindWindowById(window->property_->GetWindowId()));

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: FindWindowById03
 * @tc.desc: Find one no exit window
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, FindWindowById03, TestSize.Level1)
{
    ASSERT_EQ(nullptr, WindowImpl::FindWindowById(0));
}

/**
 * @tc.name: GetTopWindowWithId02
 * @tc.desc: Not get topwindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetTopWindowWithId02, TestSize.Level1)
{
    ASSERT_EQ(nullptr, WindowImpl::GetTopWindowWithId(0));
}

/**
 * @tc.name: UpdateConfigurationForAll01
 * @tc.desc: UpdateConfigurationForAll01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateConfigurationForAll01, TestSize.Level1)
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
 * @tc.name: UpdateConfigurationForAll02
 * @tc.desc: UpdateConfigurationForAll02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, UpdateConfigurationForAll02, TestSize.Level1)
{
    std::vector<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowContexts;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    window->UpdateConfigurationForAll(configuration, ignoreWindowContexts);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("UpdateConfigurationForAll02");
    sptr<WindowImpl> subWindow = sptr<WindowImpl>::MakeSptr(subWindowOption);
    uint32_t windowId = 1001;
    string winName = "test";
    subWindow = nullptr;
    WindowImpl::windowMap_.insert(std::make_pair(winName, std::pair<uint32_t, sptr<Window>>(windowId, subWindow)));
    window->UpdateConfigurationForAll(configuration, ignoreWindowContexts);

    subWindow = sptr<WindowImpl>::MakeSptr(subWindowOption);
    window->UpdateConfigurationForAll(configuration, ignoreWindowContexts);

    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    ignoreWindowContexts.push_back(abilityContext);
    window->context_ = abilityContext;
    window->UpdateConfigurationForAll(configuration, ignoreWindowContexts);
    WindowImpl::windowMap_.erase(winName);
}

/**
 * @tc.name: GetSurfaceNode01
 * @tc.desc: GetSurfaceNode01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetSurfaceNode01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_NE(nullptr, sptr<Window>(window)->GetSurfaceNode());
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetRequestRect
 * @tc.desc: get subwindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetRequestRect, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    Rect a{ 0, 0, 0, 0 };
    ASSERT_EQ(a, window->GetRequestRect());
}

/**
 * @tc.name: GetAlpha
 * @tc.desc: GetAlpha Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetAlpha, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(1.0f, window->GetAlpha());
}

/**
 * @tc.name: GetWindowState
 * @tc.desc: GetWindowState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetWindowState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WindowState::STATE_INITIAL, window->GetWindowState());
}

/**
 * @tc.name: SetFocusable03
 * @tc.desc: SetFocusable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetFocusable03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    WMError err = window->SetFocusable(false);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, err);
}

/**
 * @tc.name: SetWindowType05
 * @tc.desc: SetWindowType Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetWindowType05, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowType05");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(WMError::WM_OK, window->SetWindowType(WindowType::APP_WINDOW_BASE));
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAlpha01
 * @tc.desc: SetAlpha Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetAlpha01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAlpha01");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_EQ(WMError::WM_OK, window->SetAlpha(1.0f));
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: CreateWindowImpl
 * @tc.desc: CreateSurfaceNode with different type
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, CreateWindowImpl, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateSurfaceNode");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("CreateSurfaceNode1");
    option1->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option1->SetWindowType(WindowType::WINDOW_TYPE_BOOT_ANIMATION);
    option1->SetWindowRect({ 1, 1, 1, 1 });
    sptr<WindowImpl> window1 = sptr<WindowImpl>::MakeSptr(option1);
    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("CreateSurfaceNode2");
    option2->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option2->SetWindowType(WindowType::WINDOW_TYPE_POINTER);
    option2->SetWindowRect({ 1, 1, 1, 1 });
    sptr<WindowImpl> window2 = sptr<WindowImpl>::MakeSptr(option2);

    sptr<WindowOption> option3 = sptr<WindowOption>::MakeSptr();
    option3->SetWindowName("CreateSurfaceNode3");
    option3->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option3->SetWindowType(WindowType::WINDOW_TYPE_FREEZE_DISPLAY);
    option3->SetWindowRect({ 1, 1, 1, 1 });
    sptr<WindowImpl> window3 = sptr<WindowImpl>::MakeSptr(option3);

    sptr<WindowOption> option4 = sptr<WindowOption>::MakeSptr();
    option4->SetWindowName("CreateSurfaceNode4");
    option4->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option4->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    option4->SetWindowRect({ 1, 1, 1, 1 });
    sptr<WindowImpl> window4 = sptr<WindowImpl>::MakeSptr(option4);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    ASSERT_EQ(WMError::WM_OK, window1->Destroy());
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
    ASSERT_EQ(WMError::WM_OK, window3->Destroy());
    ASSERT_EQ(WMError::WM_OK, window4->Destroy());
}

/**
 * @tc.name: Create
 * @tc.desc: Create
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, Create, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateSurfaceNode5");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);

    WMError res = window->Create(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_OK, res);

    sptr<WindowImpl> window1 = sptr<WindowImpl>::MakeSptr(option); // the same name
    res = window1->Create(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, res);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    ASSERT_EQ(WMError::WM_OK, window1->Destroy());
}

/**
 * @tc.name: Create1
 * @tc.desc: Create WindowCreateCheck
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, Create1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowCreateCheck");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    WMError res = window->Create(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_OK, res);

    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    sptr<WindowImpl> window1 = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_NE(window1, nullptr);
    res = window1->Create(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, res);

    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowImpl> window2 = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_NE(window2, nullptr);
    res = window1->Create(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, res);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    ASSERT_EQ(WMError::WM_OK, window1->Destroy());
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: Create2
 * @tc.desc: Create WindowCreateCheck
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, Create2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowCreateCheck1");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    WMError res = window->Create(1);
    ASSERT_EQ(WMError::WM_OK, res);

    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window1 = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_NE(window1, nullptr);
    res = window1->Create(1);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, res);

    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    sptr<WindowImpl> window2 = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_NE(window2, nullptr);
    res = window2->Create(1);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, res);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    ASSERT_EQ(WMError::WM_OK, window1->Destroy());
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: GetTopWindowWithId
 * @tc.desc: CreateSurfaceNode with different type
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetTopWindowWithId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopWindowWithId");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    sptr<Window> topWindow = window->GetTopWindowWithId(INVALID_WINDOW_ID);
    ASSERT_EQ(topWindow, nullptr);

    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    bool interactive = false;
    window->NotifyForegroundInteractiveStatus(interactive);
    window->SetWindowState(WindowState::STATE_DESTROYED);
    interactive = true;
    window->NotifyForegroundInteractiveStatus(interactive);
    ASSERT_EQ(WindowState::STATE_DESTROYED, window->GetWindowState());
}

/**
 * @tc.name: GetTopWindowWithContext
 * @tc.desc: GetTopWindowWithContexttest01
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetTopWindowWithContext01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopWindowWithContext01");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::shared_ptr<AbilityRuntime::Context> context;
    ASSERT_EQ(nullptr, sptr<Window>(window)->GetTopWindowWithContext(context));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetSubWindow
 * @tc.desc: GetSubWindowtest02
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetSubWindowtext02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindowtest02");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    auto subWindowVec = sptr<Window>(window)->GetSubWindow(window->GetWindowId());
    if (subWindowVec.size() == 1) {
        ASSERT_EQ(1, subWindowVec.size());
    }
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetFloatingMaximize
 * @tc.desc: SetFloatingMaximize test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetFloatingMaximize, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFloatingMaximize");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowFlags(0));
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->UpdateWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFloatingMaximize(true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAspectRatio
 * @tc.desc: SetAspectRatio Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetAspectRatio, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAspectRatio");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->SetAspectRatio(1.1));
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_OK, window->SetAspectRatio(1.1));
}

/**
 * @tc.name: SetAspectRatio02
 * @tc.desc: SetAspectRatio Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetAspectRatio02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAspectRatio02");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetAspectRatio(0.0));
}

/**
 * @tc.name: MapDialogWindowToAppIfNeeded
 * @tc.desc: MapDialogWindowToAppIfNeededtest
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, MapDialogWindowToAppIfNeededtest, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MapDialogWindowToAppIfNeededtest");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    option->SetWindowRect({ 1, 1, 1, 1 });
    option->SetBundleName("OK");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->MapDialogWindowToAppIfNeeded();
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetConfigurationFromAbilityInfo
 * @tc.desc: GetConfigurationFromAbilityInfotest
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetConfigurationFromAbilityInfotest, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetConfigurationFromAbilityInfotest");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->MapDialogWindowToAppIfNeeded();
    window->GetConfigurationFromAbilityInfo();
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetSnapshotSkip
 * @tc.desc: SetSnapshotSkip test
 * @tc.type: FUNC
 * @tc.require: issueI5MYNX
 */
HWTEST_F(WindowImplTest3, SetSnapshotSkip, TestSize.Level1)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetSnapshotSkip");
    sptr<WindowImpl> window = new WindowImpl(option);
    ASSERT_NE(nullptr, window);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetSnapshotSkip(true));
}

/**
 * @tc.name: MaximizeFloating
 * @tc.desc: MaximizeFloating test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, MaximizeFloating, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MaximizeFloating");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    window->MaximizeFloating();
    ASSERT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, window->GetWindowMode());
    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    window->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    window->Show();
    window->MaximizeFloating();
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());
    EXPECT_CALL(m->Mock(), RemoveWindow(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    window->Hide();
    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetTopWindowWithId03
 * @tc.desc: GetTopWindowWithId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetTopWindowWithId03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopWindowWithId03");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    uint32_t mainWinId = 0;
    uint32_t windowId = 1;
    string winName = "test";
    WindowImpl::windowMap_.insert(std::make_pair(winName, std::pair<uint32_t, sptr<Window>>(windowId, window)));

    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _)).WillRepeatedly(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ASSERT_EQ(nullptr, window->GetTopWindowWithId(mainWinId));

    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(windowId), Return(WMError::WM_OK)));
    ASSERT_NE(nullptr, window->GetTopWindowWithId(mainWinId));
    uint32_t topWinId = 1;
    ASSERT_EQ(WindowImpl::FindWindowById(topWinId), window->GetTopWindowWithId(mainWinId));

    uint32_t tempWindowId = 3;
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(tempWindowId), Return(WMError::WM_OK)));
    ASSERT_EQ(nullptr, window->GetTopWindowWithId(mainWinId));

    WindowImpl::windowMap_.erase(winName);
}

/**
 * @tc.name: GetTopWindowWithContext02
 * @tc.desc: GetTopWindowWithContext test
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetTopWindowWithContext02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopWindowWithContext02");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::shared_ptr<AbilityRuntime::Context> context;

    ASSERT_EQ(nullptr, window->GetTopWindowWithContext(context));
    uint32_t topWinId = INVALID_WINDOW_ID;
    ASSERT_EQ(WindowImpl::FindWindowById(topWinId), window->GetTopWindowWithContext(context));
}

/**
 * @tc.name: GetSubWindow03
 * @tc.desc: GetSubWindowtest
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, GetSubWindow03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindow03");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    uint32_t parentId = 0;

    ASSERT_EQ(std::vector<sptr<Window>>(), window->GetSubWindow(parentId));
}

/**
 * @tc.name: SetNeedDefaultAnimation
 * @tc.desc: SetNeedDefaultAnimation
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplTest3, SetNeedDefaultAnimation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetNeedDefaultAnimation");
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    bool defaultAnimation = true;
    window->SetNeedDefaultAnimation(defaultAnimation);
    EXPECT_EQ(true, window->needDefaultAnimation_);
    window->SetNeedDefaultAnimation(false);
    EXPECT_EQ(false, window->needDefaultAnimation_);
    EXPECT_EQ(WMError::WM_OK, window->SetTextFieldAvoidInfo(2.0, 3.0));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
