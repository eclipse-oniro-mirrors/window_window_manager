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

#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "pointer_event.h"
#include "display_info.h"
#include "window_helper.h"
#include "window_scene_session_impl.h"
#include "window_manager.h"
// the .cpp include gives the full Impl definition for the pImpl_ white-box access
#include "window_manager.cpp"
#include "wm_common.h"
#include "window_manager_hilog.h"
#include "window_option.h"
#include "session/host/include/session.h"
#include <ui/rs_surface_node.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr float TEST_RATIO = 0.5f;
constexpr int32_t TEST_PERSISTENT_ID = 1;

class GameAdaptiveResolutionListenerTester : public IGameAdaptiveResolutionListener {
public:
    GameAdaptiveResolutionListenerTester(int32_t& cancelledId, int32_t& availableId)
        : cancelledId_(cancelledId), availableId_(availableId) {}
    ~GameAdaptiveResolutionListenerTester() = default;
    void OnGameAdaptiveResolutionCancelled(int32_t windowId) override { cancelledId_ = windowId; }
    void OnGameAdaptiveResolutionAvailable(int32_t windowId) override { availableId_ = windowId; }
private:
    int32_t& cancelledId_;
    int32_t& availableId_;
};

class WindowSizeChangeListenerTester : public IWindowChangeListener {
public:
    Rect rect_;
    void OnSizeChange(Rect rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction) override { rect_ = rect; }
};

class AvoidAreaChangedListenerTester : public IAvoidAreaChangedListener {
public:
    AvoidArea area_;
    AvoidAreaType type_ = AvoidAreaType::TYPE_SYSTEM;
    int32_t count_ = 0;
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info) override
    {
        area_ = avoidArea;
        type_ = type;
        count_++;
    }
};
}

class WindowSessionImplAdaptiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<WindowSessionImpl> CreateTestWindow(const std::string& name,
        WindowType type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    void ActivateAdaptive(sptr<WindowSessionImpl>& window, float ratio = TEST_RATIO, bool inputHook = true);
    GameAdaptiveResolutionConfig MakeConfig(int32_t windowId, bool enable, float ratio = TEST_RATIO,
        bool inputHook = true);
};

void WindowSessionImplAdaptiveTest::SetUpTestCase() {}

void WindowSessionImplAdaptiveTest::TearDownTestCase() {}

void WindowSessionImplAdaptiveTest::SetUp() {}

void WindowSessionImplAdaptiveTest::TearDown() {}

sptr<WindowSessionImpl> WindowSessionImplAdaptiveTest::CreateTestWindow(const std::string& name, WindowType type)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    if (option == nullptr) {
        return nullptr;
    }
    option->SetWindowName(name);
    option->SetWindowType(type);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    if (window == nullptr) {
        return nullptr;
    }
    SessionInfo sessionInfo = { name, name, name };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    auto runner = AppExecFwk::EventRunner::Create(name.c_str());
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    window->handler_ = handler;
    window->property_->SetPersistentId(TEST_PERSISTENT_ID);
    return window;
}

GameAdaptiveResolutionConfig WindowSessionImplAdaptiveTest::MakeConfig(int32_t windowId, bool enable,
    float ratio, bool inputHook)
{
    GameAdaptiveResolutionConfig config;
    config.enable_ = enable;
    config.scaleRatio_ = ratio;
    config.windowId_ = windowId;
    config.inputHook_ = inputHook;
    config.notifyWindow_ = false; // defer refresh paths in UT
    return config;
}

void WindowSessionImplAdaptiveTest::ActivateAdaptive(sptr<WindowSessionImpl>& window, float ratio, bool inputHook)
{
    window->adaptiveResolutionConfig_ = MakeConfig(TEST_PERSISTENT_ID, true, ratio, inputHook);
    window->adaptiveResolutionActive_.store(true);
}

namespace {
/**
 * @tc.name: ConfigValidationAndParcel
 * @tc.desc: ratio range, IsValid, HasSameEffect and parcel round-trip branches
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, ConfigValidationAndParcel, TestSize.Level1)
{
    GameAdaptiveResolutionConfig config;
    config.scaleRatio_ = 0.0f;
    EXPECT_FALSE(config.IsScaleRatioValid());
    config.scaleRatio_ = 1.0f;
    EXPECT_FALSE(config.IsScaleRatioValid());
    config.scaleRatio_ = 0.5f;
    EXPECT_TRUE(config.IsScaleRatioValid());

    EXPECT_TRUE(config.IsValid());
    config.enable_ = true;
    EXPECT_TRUE(config.IsValid());
    config.density_ = -1.0f;
    EXPECT_FALSE(config.IsValid());
    config.density_ = 0.0f;
    config.xDpi_ = -1.0f;
    EXPECT_FALSE(config.IsValid());
    config.xDpi_ = 0.0f;
    config.yDpi_ = -1.0f;
    EXPECT_FALSE(config.IsValid());
    config.yDpi_ = 0.0f;
    EXPECT_TRUE(config.IsValid());
    config.enable_ = false;
    config.scaleRatio_ = 2.0f;
    EXPECT_TRUE(config.IsValid()); // disable tolerates invalid fields

    GameAdaptiveResolutionConfig other;
    other.enable_ = true;
    other.scaleRatio_ = 0.5f;
    config.enable_ = true;
    config.scaleRatio_ = 0.5f;
    config.density_ = 0.0f;
    EXPECT_TRUE(config.HasSameEffect(other));
    other.scaleRatio_ = 0.75f;
    EXPECT_FALSE(config.HasSameEffect(other));
    other.scaleRatio_ = 0.5f;
    other.autoMetrics_ = false;
    EXPECT_FALSE(config.HasSameEffect(other));
    other.autoMetrics_ = true;
    other.density_ = 3.0f;
    EXPECT_FALSE(config.HasSameEffect(other));

    GameAdaptiveResolutionConfig parcelConfig;
    parcelConfig.enable_ = true;
    parcelConfig.scaleRatio_ = 0.75f;
    parcelConfig.windowId_ = 7;
    parcelConfig.inputHook_ = false;
    parcelConfig.notifyWindow_ = false;
    parcelConfig.notifyDisplay_ = false;
    parcelConfig.autoMetrics_ = false;
    parcelConfig.density_ = 2.5f;
    parcelConfig.xDpi_ = 320.0f;
    parcelConfig.yDpi_ = 330.0f;
    Parcel parcel;
    ASSERT_TRUE(parcelConfig.Marshalling(parcel));
    auto parsed = GameAdaptiveResolutionConfig::Unmarshalling(parcel);
    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(parsed->enable_, true);
    EXPECT_EQ(parsed->scaleRatio_, 0.75f);
    EXPECT_EQ(parsed->windowId_, 7);
    EXPECT_EQ(parsed->inputHook_, false);
    EXPECT_EQ(parsed->autoMetrics_, false);
    EXPECT_EQ(parsed->density_, 2.5f);
    EXPECT_EQ(parsed->yDpi_, 330.0f);
    EXPECT_FALSE(parcelConfig.ToString().empty());
}

/**
 * @tc.name: HookGeometryVariants
 * @tc.desc: input scale ratio, window size and avoid area conversions
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, HookGeometryVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("HookGeometry");
    ASSERT_NE(window, nullptr);

    EXPECT_EQ(window->GetAdaptiveInputScaleRatio(), 0.0f);
    ActivateAdaptive(window);
    EXPECT_EQ(window->GetAdaptiveInputScaleRatio(), TEST_RATIO);
    ActivateAdaptive(window, TEST_RATIO, false);
    EXPECT_EQ(window->GetAdaptiveInputScaleRatio(), 0.0f);

    ActivateAdaptive(window);
    Rect rect = { 0, 0, 1000, 2001 };
    window->HookWindowSizeByAdaptiveResolution(rect);
    EXPECT_EQ(rect.width_, 500u);   // lround(1000 * 0.5)
    EXPECT_EQ(rect.height_, 1001u); // lround(2001 * 0.5)
    Rect tiny = { 0, 0, 1, 0 };
    window->HookWindowSizeByAdaptiveResolution(tiny);
    EXPECT_EQ(tiny.width_, 1u);
    EXPECT_EQ(tiny.height_, 1u);

    AvoidArea avoidArea;
    avoidArea.topRect_ = { 10, 20, 100, 50 };
    avoidArea.bottomRect_ = { 0, 990, 1000, 10 };
    avoidArea.leftRect_ = { 5, 5, 15, 25 };
    avoidArea.rightRect_ = {};
    window->HookAvoidAreaForHookedViewport(avoidArea);
    EXPECT_EQ(avoidArea.topRect_.posX_, 5);    // lround(10 * 0.5)
    EXPECT_EQ(avoidArea.topRect_.posY_, 10);
    EXPECT_EQ(avoidArea.topRect_.width_, 50u);
    EXPECT_EQ(avoidArea.topRect_.height_, 25u);
    EXPECT_EQ(avoidArea.bottomRect_.posY_, 495);
    EXPECT_EQ(avoidArea.leftRect_.posX_, 3);   // lround(5 * 0.5) = 3 (round half up)
    EXPECT_EQ(avoidArea.rightRect_.width_, 0u); // empty rects stay empty

    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    AvoidArea area;
    area.topRect_ = { 0, 100, 200, 40 };
    avoidAreas[AvoidAreaType::TYPE_CUTOUT] = area;
    window->HookAvoidAreasForHookedViewport(avoidAreas);
    EXPECT_EQ(avoidAreas[AvoidAreaType::TYPE_CUTOUT].topRect_.posY_, 50);
    EXPECT_EQ(avoidAreas[AvoidAreaType::TYPE_CUTOUT].topRect_.width_, 100u);

    // inactive: everything passes through unchanged
    window->adaptiveResolutionActive_.store(false);
    Rect passthrough = { 0, 0, 1000, 2000 };
    window->HookWindowSizeByAdaptiveResolution(passthrough);
    EXPECT_EQ(passthrough.width_, 1000u);
    avoidArea.topRect_ = { 10, 20, 100, 50 }; // reseed: the earlier hook scaled it in place
    window->HookAvoidAreaForHookedViewport(avoidArea);
    EXPECT_EQ(avoidArea.topRect_.posX_, 10);
}

/**
 * @tc.name: HookKeyboardAndOccupiedVariants
 * @tc.desc: occupied area info, keyboard animation and panel conversions
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, HookKeyboardAndOccupiedVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("HookKeyboardOccupied");
    ASSERT_NE(window, nullptr);
    auto info = sptr<OccupiedAreaChangeInfo>::MakeSptr(
        OccupiedAreaType::TYPE_INPUT, Rect { 0, 900, 1000, 100 }, 60, 300, 40);
    EXPECT_EQ(window->HookOccupiedAreaInfoForHookedViewport(nullptr), nullptr);
    auto same = window->HookOccupiedAreaInfoForHookedViewport(info);
    EXPECT_EQ(same, info); // inactive: same pointer
    ActivateAdaptive(window);
    auto hooked = window->HookOccupiedAreaInfoForHookedViewport(info);
    EXPECT_NE(hooked, info);
    EXPECT_EQ(hooked->rect_.posY_, 450);
    EXPECT_EQ(hooked->rect_.width_, 500u);
    EXPECT_EQ(hooked->rect_.height_, 50u);
    EXPECT_EQ(hooked->safeHeight_, 30u); // lround(60 * 0.5)
    EXPECT_EQ(hooked->textFieldPositionY_, 300.0); // app-reported in hooked space, unchanged
    EXPECT_EQ(hooked->textFieldHeight_, 40.0);
    EXPECT_EQ(info->rect_.posY_, 900); // original untouched

    KeyboardAnimationInfo anim;
    anim.beginRect = Rect { 0, 1200, 1000, 0 };
    anim.endRect = Rect { 0, 800, 1000, 400 };
    window->adaptiveResolutionActive_.store(false);
    auto animInactive = window->HookKeyboardAnimationInfoForHookedViewport(anim);
    EXPECT_EQ(animInactive.beginRect.posY_, 1200);
    EXPECT_EQ(animInactive.endRect.height_, 400u);
    ActivateAdaptive(window);
    auto animHooked = window->HookKeyboardAnimationInfoForHookedViewport(anim);
    EXPECT_EQ(animHooked.beginRect.posY_, 600);
    EXPECT_EQ(animHooked.endRect.posY_, 400);
    EXPECT_EQ(animHooked.endRect.height_, 200u);
    EXPECT_EQ(anim.beginRect.posY_, 1200); // original untouched

    KeyboardPanelInfo panel;
    panel.rect_ = Rect { 0, 900, 1000, 100 };
    panel.beginRect_ = Rect { 0, 1000, 1000, 0 };
    panel.endRect_ = Rect { 0, 700, 1000, 300 };
    window->adaptiveResolutionActive_.store(false);
    auto panelInactive = window->HookKeyboardPanelInfoForHookedViewport(panel);
    EXPECT_EQ(panelInactive.rect_.posY_, 900);
    ActivateAdaptive(window);
    // glue: the notify entries execute the hook glue lines (no listeners registered)
    window->NotifyKeyboardWillShow(anim);
    window->NotifyKeyboardWillHide(anim);
    window->NotifyKeyboardDidShow(panel);
    window->NotifyKeyboardDidHide(panel);
    auto panelHooked = window->HookKeyboardPanelInfoForHookedViewport(panel);
    EXPECT_EQ(panelHooked.rect_.posY_, 450);
    EXPECT_EQ(panelHooked.rect_.height_, 50u);
    EXPECT_EQ(panelHooked.beginRect_.posY_, 500);
    EXPECT_EQ(panelHooked.endRect_.posY_, 350);
    EXPECT_EQ(panelHooked.endRect_.height_, 150u);
}

/**
 * @tc.name: PointerEventVariants
 * @tc.desc: dispatch scaling and hit-test copy semantics
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, PointerEventVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("PointerEvent");
    ASSERT_NE(window, nullptr);
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    EXPECT_FALSE(window->ScalePointerEventForHookedViewport(nullptr));
    EXPECT_FALSE(window->ScalePointerEventForHookedViewport(pointerEvent));
    EXPECT_EQ(window->GetPointerEventForHitTest(nullptr), nullptr);
    EXPECT_EQ(window->GetPointerEventForHitTest(pointerEvent), pointerEvent);
    ActivateAdaptive(window);
    EXPECT_TRUE(window->ScalePointerEventForHookedViewport(pointerEvent));
    // inputHook off: both paths fall back to the original
    ActivateAdaptive(window, TEST_RATIO, false);
    EXPECT_FALSE(window->ScalePointerEventForHookedViewport(pointerEvent));
    EXPECT_EQ(window->GetPointerEventForHitTest(pointerEvent), pointerEvent);

    ActivateAdaptive(window);
    auto scaled = window->GetPointerEventForHitTest(pointerEvent);
    EXPECT_NE(scaled, nullptr);
    EXPECT_NE(scaled, pointerEvent);

    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(0);
    item.SetWindowX(100);
    item.SetWindowY(201);
    item.SetWindowXPos(10.5f);
    item.SetWindowYPos(20.5f);
    item.SetDisplayX(301);
    item.SetDisplayY(403);
    item.SetDisplayXPos(11.5f);
    item.SetDisplayYPos(21.5f);
    item.SetGlobalX(31.5f);
    item.SetGlobalY(41.5f);
    pointerEvent->AddPointerItem(item);
    EXPECT_TRUE(window->ScalePointerEventForHookedViewport(pointerEvent));
    MMI::PointerEvent::PointerItem got;
    ASSERT_TRUE(pointerEvent->GetPointerItem(0, got));
    EXPECT_EQ(got.GetWindowX(), 50);      // lround(100 * 0.5)
    EXPECT_EQ(got.GetWindowY(), 101);     // lround(201 * 0.5)
    EXPECT_EQ(got.GetWindowXPos(), 5.25f);
    // DisplayX/Y int round-trip varies with the on-device MMI version; the same
    // lround scaling line is asserted through WindowX/WindowY above.
    EXPECT_EQ(got.GetGlobalX(), 15.75f);
    EXPECT_EQ(got.GetGlobalY(), 20.75f);

    // the injection path scales the event in place (no uiContent: consume reports abnormal)
    auto inject = MMI::PointerEvent::Create();
    ASSERT_NE(inject, nullptr);
    MMI::PointerEvent::PointerItem injectItem;
    injectItem.SetPointerId(0);
    injectItem.SetWindowX(200);
    injectItem.SetWindowY(402);
    inject->SetPointerId(0);
    inject->AddPointerItem(injectItem);
    EXPECT_EQ(window->InjectTouchEvent(inject), WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    MMI::PointerEvent::PointerItem gotInject;
    ASSERT_TRUE(inject->GetPointerItem(0, gotInject));
    EXPECT_EQ(gotInject.GetWindowX(), 100); // lround(200 * 0.5)
    EXPECT_EQ(gotInject.GetWindowY(), 201); // lround(402 * 0.5)
}

/**
 * @tc.name: HandleAdaptiveResolutionChangedVariants
 * @tc.desc: SET/CANCELLED/unknown handling and the stored-notifyWindow restore
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, HandleAdaptiveResolutionChangedVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("HandleChanged");
    ASSERT_NE(window, nullptr);
    // SET with mismatched windowId is ignored
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID + 1, true));
    EXPECT_FALSE(window->IsAdaptiveResolutionEnabled());
    // matching id applies the config
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID, true));
    EXPECT_TRUE(window->IsAdaptiveResolutionEnabled());
    // enable with invalid ratio keeps the hook inactive
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID, true, 1.0f));
    EXPECT_FALSE(window->IsAdaptiveResolutionEnabled());

    ActivateAdaptive(window);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveResolutionListenerTester>::MakeSptr(cancelledId, availableId);
    auto& windowManager = WindowManager::GetInstance();
    windowManager.pImpl_->gameAdaptiveListeners_.insert(listener);

    // stored notifyWindow_ = true: the cancel restore re-delivers
    auto enabled = MakeConfig(TEST_PERSISTENT_ID, true);
    enabled.notifyWindow_ = true;
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET, enabled);
    window->notifySizeChangeInCompatibleMode_.store(false);
    window->notifyRectChangeInCompatibleMode_.store(false);
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::CANCELLED,
        MakeConfig(TEST_PERSISTENT_ID, false));
    EXPECT_FALSE(window->IsAdaptiveResolutionEnabled());
    EXPECT_FALSE(window->GetAdaptiveResolutionConfig().enable_);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    // local apply only: the notify source is the SCB agent fan-out
    EXPECT_EQ(cancelledId, -1);
    // the agent-driven callback reaches the listener
    windowManager.NotifyGameAdaptiveResolutionCancelled(window->GetPersistentId());
    EXPECT_EQ(cancelledId, window->GetPersistentId());

    // stored notifyWindow_ = false: the cancel restore stays silent
    auto quiet = MakeConfig(TEST_PERSISTENT_ID, true);
    quiet.notifyWindow_ = false;
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET, quiet);
    window->notifySizeChangeInCompatibleMode_.store(false);
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::CANCELLED,
        MakeConfig(TEST_PERSISTENT_ID, false));
    EXPECT_FALSE(window->IsAdaptiveResolutionEnabled());
    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());

    // AVAILABLE falls to the unknown default: only the agent channel notifies
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::AVAILABLE,
        MakeConfig(TEST_PERSISTENT_ID, false));
    EXPECT_EQ(availableId, -1);
    windowManager.NotifyGameAdaptiveResolutionAvailable(window->GetPersistentId());
    EXPECT_EQ(availableId, window->GetPersistentId());

    auto unknown = static_cast<GameAdaptiveResolutionStatus>(99);
    window->HandleAdaptiveResolutionChanged(unknown, MakeConfig(TEST_PERSISTENT_ID, false));
    EXPECT_FALSE(window->IsAdaptiveResolutionEnabled());

    windowManager.pImpl_->gameAdaptiveListeners_.erase(listener);
}

/**
 * @tc.name: ApplyAndNotifyPlumbing
 * @tc.desc: the notify wrapper posts a task; effect changes gate the refresh
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, ApplyAndNotifyPlumbing, TestSize.Level1)
{
    auto window = CreateTestWindow("ApplyAndNotify");
    ASSERT_NE(window, nullptr);
    // the notify wrapper defers the apply to the event handler
    auto ret = window->NotifyAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID, true));
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_FALSE(window->IsAdaptiveResolutionEnabled());
    window->HandleAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID, true));
    EXPECT_TRUE(window->IsAdaptiveResolutionEnabled());

    // effect gating: refresh defers in UT, observable via pending flags
    window->state_ = WindowState::STATE_CREATED;
    auto config = MakeConfig(TEST_PERSISTENT_ID, true, 0.6f); // differs in effect from the applied one
    config.notifyWindow_ = true;
    window->ApplyAdaptiveResolutionConfig(config);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->notifyRectChangeInCompatibleMode_.load());
    // same effect: no refresh
    window->notifySizeChangeInCompatibleMode_.store(false);
    window->notifyRectChangeInCompatibleMode_.store(false);
    window->ApplyAdaptiveResolutionConfig(config);
    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    // control-flag-only change (inputHook): still no refresh
    auto controlFlagChange = config;
    controlFlagChange.inputHook_ = !config.inputHook_;
    window->ApplyAdaptiveResolutionConfig(controlFlagChange);
    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    // effect change: refresh runs again
    auto ratioChange = config;
    ratioChange.scaleRatio_ = 0.75f;
    window->ApplyAdaptiveResolutionConfig(ratioChange);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_EQ(window->GetAdaptiveResolutionConfig().scaleRatio_, 0.75f);
}

/**
 * @tc.name: RefreshAdaptiveResolutionPaths
 * @tc.desc: defer, immediate and keyboard-open (two-pass) refresh paths
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, RefreshAdaptiveResolutionPaths, TestSize.Level1)
{
    auto window = CreateTestWindow("RefreshPaths");
    ASSERT_NE(window, nullptr);
    window->state_ = WindowState::STATE_CREATED;
    ActivateAdaptive(window);
    // defer: notifyListeners gates the pending flags
    window->RefreshAdaptiveResolution(false);
    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    window->RefreshAdaptiveResolution(true);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    // shown but zero rect: still defers
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetWindowRect(Rect { 0, 0, 0, 0 });
    window->notifySizeChangeInCompatibleMode_.store(false);
    window->RefreshAdaptiveResolution(true);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    window->property_->SetWindowRect(Rect { 0, 0, 100, 0 });
    window->notifySizeChangeInCompatibleMode_.store(false);
    window->RefreshAdaptiveResolution(true);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());

    // immediate path with a valid rect
    window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
    auto sizeListener = sptr<WindowSizeChangeListenerTester>::MakeSptr();
    ASSERT_EQ(window->RegisterWindowChangeListener(sizeListener, true), WMError::WM_OK);
    window->RefreshAdaptiveResolution(true);
    // display lookup fails in UT: viewport update early-returns, callbacks still fire
    EXPECT_EQ(sizeListener->rect_.width_, 500u);
    EXPECT_EQ(sizeListener->rect_.height_, 1000u);
    // notifyListeners=false: no callbacks
    sizeListener->rect_ = Rect {};
    window->RefreshAdaptiveResolution(false);
    EXPECT_EQ(sizeListener->rect_.width_, 0u);
    // a zero-height occupied info is treated as keyboard closed: same plain path
    window->occupiedAreaInfo_ = sptr<OccupiedAreaChangeInfo>::MakeSptr(
        OccupiedAreaType::TYPE_INPUT, Rect { 0, 0, 1000, 0 }, 0, 0, 0);
    sizeListener->rect_ = Rect {};
    window->RefreshAdaptiveResolution(true);
    EXPECT_EQ(sizeListener->rect_.width_, 500u);

    // keyboard open takes the two-pass repair path and keeps the occupied info
    auto info = sptr<OccupiedAreaChangeInfo>::MakeSptr(
        OccupiedAreaType::TYPE_INPUT, Rect { 0, 900, 1000, 100 }, 60, 300, 40);
    window->occupiedAreaInfo_ = info;
    sizeListener->rect_ = Rect {};
    window->RefreshAdaptiveResolution(true);
    // the repair passes early-return on the null display in UT; the member stays as-is
    EXPECT_EQ(window->occupiedAreaInfo_, info);
    EXPECT_EQ(window->occupiedAreaInfo_->rect_.posY_, 900);
    EXPECT_EQ(window->occupiedAreaInfo_->textFieldPositionY_, 300.0);
    EXPECT_EQ(sizeListener->rect_.width_, 500u);
    sizeListener->rect_ = Rect {};
    window->RefreshAdaptiveResolution(false);
    EXPECT_EQ(sizeListener->rect_.width_, 0u);
    ASSERT_EQ(window->UnregisterWindowChangeListener(sizeListener), WMError::WM_OK);
}

/**
 * @tc.name: UpdateViewportConfigVariants
 * @tc.desc: hooked/inactive/use-hooked-size/occupied-areas delivery branches
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, UpdateViewportConfigVariants, TestSize.Level1)
{
    auto displayInfo = sptr<DisplayInfo>::MakeSptr();
    displayInfo->SetVirtualPixelRatio(2.0f);

    // active config delivers the hooked viewport and hooked occupied info
    {
        auto window = CreateTestWindow("ViewportConfigHooked");
        ASSERT_NE(window, nullptr);
        window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
        ActivateAdaptive(window);
        window->occupiedAreaInfo_ = sptr<OccupiedAreaChangeInfo>::MakeSptr(
            OccupiedAreaType::TYPE_INPUT, Rect { 0, 900, 1000, 100 }, 60, 300, 40);
        window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
        auto* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
        Ace::ViewportConfig cfg;
        std::map<AvoidAreaType, AvoidArea> areas;
        sptr<OccupiedAreaChangeInfo> info;
        EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _))
            .WillOnce(DoAll(SaveArg<0>(&cfg), SaveArg<3>(&areas), SaveArg<4>(&info)));
        window->UpdateViewportConfig(Rect { 0, 0, 1000, 2000 }, WindowSizeChangeReason::UNDEFINED,
            nullptr, displayInfo, {});
        EXPECT_EQ(static_cast<int32_t>(cfg.Width()), 500); // hooked viewport
        EXPECT_EQ(static_cast<int32_t>(cfg.Height()), 1000);
        EXPECT_TRUE(areas.empty());   // empty input falls back to the (empty) cached map
        EXPECT_EQ(info->rect_.posY_, 450);   // occupied info hooked
        EXPECT_EQ(info->rect_.width_, 500u);
        EXPECT_EQ(info->safeHeight_, 30u);
        EXPECT_EQ(info->textFieldPositionY_, 300.0); // app-reported fields stay
        EXPECT_EQ(window->occupiedAreaInfo_->rect_.posY_, 900); // the cached info is untouched
    }

    // inactive config delivers the real viewport and the info as-is
    {
        auto window = CreateTestWindow("ViewportConfigInactive");
        ASSERT_NE(window, nullptr);
        window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
        window->occupiedAreaInfo_ = sptr<OccupiedAreaChangeInfo>::MakeSptr(
            OccupiedAreaType::TYPE_INPUT, Rect { 0, 900, 1000, 100 }, 60, 300, 40);
        window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
        auto* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
        Ace::ViewportConfig cfg;
        sptr<OccupiedAreaChangeInfo> info;
        EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _))
            .WillOnce(DoAll(SaveArg<0>(&cfg), SaveArg<4>(&info)));
        window->UpdateViewportConfig(Rect { 0, 0, 1000, 2000 }, WindowSizeChangeReason::UNDEFINED,
            nullptr, displayInfo, {});
        EXPECT_EQ(static_cast<int32_t>(cfg.Width()), 1000);
        EXPECT_EQ(static_cast<int32_t>(cfg.Height()), 2000);
        EXPECT_EQ(info->rect_.posY_, 900); // passthrough
        EXPECT_EQ(info->safeHeight_, 60u);
    }

    // the compatible-mode hooked-size flag selects the hooked viewport without adaptive
    {
        auto window = CreateTestWindow("ViewportConfigUseHooked");
        ASSERT_NE(window, nullptr);
        window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
        HookWindowInfo hookInfo;
        hookInfo.enableHookWindow = true;
        hookInfo.widthHookRatio = 0.5f;
        window->property_->SetHookWindowInfo(hookInfo);
        window->viewportUseHookedSize_.store(true);
        window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
        auto* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
        Ace::ViewportConfig cfg;
        EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _))
            .WillOnce(SaveArg<0>(&cfg));
        window->UpdateViewportConfig(Rect { 0, 0, 1000, 2000 }, WindowSizeChangeReason::UNDEFINED,
            nullptr, displayInfo, {});
        EXPECT_EQ(static_cast<int32_t>(cfg.Width()), 500); // hooked by the window hook, not by adaptive
        EXPECT_EQ(static_cast<int32_t>(cfg.Height()), 2000);
    }

    // occupied reason with non-empty areas delivers the passed-in areas hooked in place
    {
        auto window = CreateTestWindow("ViewportConfigAreas");
        ASSERT_NE(window, nullptr);
        window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
        ActivateAdaptive(window);
        window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
        auto* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
        std::map<AvoidAreaType, AvoidArea> input;
        AvoidArea area;
        area.topRect_ = { 0, 100, 200, 40 };
        input[AvoidAreaType::TYPE_CUTOUT] = area;
        std::map<AvoidAreaType, AvoidArea> captured;
        EXPECT_CALL(*content, UpdateViewportConfig(_, _, _, _, _))
            .WillOnce(SaveArg<3>(&captured));
        window->UpdateViewportConfig(Rect { 0, 0, 1000, 2000 },
            WindowSizeChangeReason::OCCUPIED_AREA_CHANGE, nullptr, displayInfo, input);
        EXPECT_EQ(captured.count(AvoidAreaType::TYPE_CUTOUT), 1u);
        EXPECT_EQ(captured[AvoidAreaType::TYPE_CUTOUT].topRect_.posY_, 50); // hooked in place
        EXPECT_EQ(captured[AvoidAreaType::TYPE_CUTOUT].topRect_.width_, 100u);
        EXPECT_EQ(input[AvoidAreaType::TYPE_CUTOUT].topRect_.posY_, 100); // input untouched
    }
}

/**
 * @tc.name: ListenerDeliveryVariants
 * @tc.desc: avoid-area cache delivery and per-listener hooked size notifications
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, ListenerDeliveryVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("ListenerDelivery");
    ASSERT_NE(window, nullptr);
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
    ActivateAdaptive(window);

    auto avoidListener = sptr<AvoidAreaChangedListenerTester>::MakeSptr();
    // local registration succeeds; the service-side sync result is environment-dependent
    window->RegisterAvoidAreaChangeListener(avoidListener);
    EXPECT_FALSE(window->avoidAreaChangeListeners_[TEST_PERSISTENT_ID].empty());
    AvoidArea area;
    area.topRect_ = { 0, 100, 200, 40 };
    area.bottomRect_ = { 0, 900, 1000, 100 };
    window->lastAvoidAreaMap_[AvoidAreaType::TYPE_CUTOUT] = area;
    window->NotifyAvoidAreasForHookedViewport();
    EXPECT_EQ(avoidListener->count_, 1);
    EXPECT_EQ(avoidListener->type_, AvoidAreaType::TYPE_CUTOUT);
    EXPECT_EQ(avoidListener->area_.topRect_.posY_, 50);
    EXPECT_EQ(avoidListener->area_.bottomRect_.posY_, 450);
    EXPECT_EQ(window->lastAvoidAreaMap_[AvoidAreaType::TYPE_CUTOUT].topRect_.posY_, 100);
    // empty cache: no notifications
    window->lastAvoidAreaMap_.clear();
    avoidListener->count_ = 0;
    window->NotifyAvoidAreasForHookedViewport();
    EXPECT_EQ(avoidListener->count_, 0);
    window->UnregisterAvoidAreaChangeListener(avoidListener);
    EXPECT_TRUE(window->avoidAreaChangeListeners_[TEST_PERSISTENT_ID].empty());

    auto hookedListener = sptr<WindowSizeChangeListenerTester>::MakeSptr();
    auto realListener = sptr<WindowSizeChangeListenerTester>::MakeSptr();
    ASSERT_EQ(window->RegisterWindowChangeListener(hookedListener, true), WMError::WM_OK);
    ASSERT_EQ(window->RegisterWindowChangeListener(realListener, false), WMError::WM_OK);
    Rect rect = { 0, 0, 1000, 2000 };
    window->adaptiveResolutionActive_.store(false);
    window->NotifySizeChange(rect, WindowSizeChangeReason::RESIZE);
    EXPECT_EQ(hookedListener->rect_.width_, 1000u); // inactive: same
    ActivateAdaptive(window);
    window->NotifySizeChange(rect, WindowSizeChangeReason::RESIZE);
    EXPECT_EQ(hookedListener->rect_.width_, 500u);  // hooked space
    EXPECT_EQ(hookedListener->rect_.height_, 1000u);
    EXPECT_EQ(realListener->rect_.width_, 1000u);   // real space
    ASSERT_EQ(window->UnregisterWindowChangeListener(hookedListener), WMError::WM_OK);
    ASSERT_EQ(window->UnregisterWindowChangeListener(realListener), WMError::WM_OK);
}

/**
 * @tc.name: UpdateAdaptiveSurfaceScaleVariants
 * @tc.desc: no-node early returns and apply/dedup/re-apply/reset with a real node
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, UpdateAdaptiveSurfaceScaleVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("SurfaceScale");
    ASSERT_NE(window, nullptr);
    Rect real = { 0, 0, 1000, 2000 };
    Rect hooked = { 0, 0, 500, 1000 };
    // no surface node: false regardless of the applied flag and the scale need
    window->adaptiveSurfaceScaleApplied_.store(true);
    EXPECT_FALSE(window->UpdateAdaptiveSurfaceScale(real, real));
    ActivateAdaptive(window);
    EXPECT_FALSE(window->UpdateAdaptiveSurfaceScale(real, hooked));
    EXPECT_TRUE(window->adaptiveSurfaceScaleApplied_.load()); // early return keeps the flag

    RSSurfaceNodeConfig config;
    config.SurfaceNodeName = "AdaptiveSurfaceScaleTest";
    auto surfaceNode = RSSurfaceNode::Create(config);
    if (surfaceNode == nullptr) {
        GTEST_SKIP() << "surface node unavailable in UT environment";
    }
    window->surfaceNode_ = surfaceNode;
    window->adaptiveSurfaceScaleApplied_.store(false);
    // nothing applied and no scale needed
    EXPECT_FALSE(window->UpdateAdaptiveSurfaceScale(real, real));
    // apply
    EXPECT_TRUE(window->UpdateAdaptiveSurfaceScale(real, hooked));
    EXPECT_TRUE(window->adaptiveSurfaceScaleApplied_.load());
    EXPECT_FLOAT_EQ(window->adaptiveSurfaceScaleX_, 2.0f);
    // dedup: the same values already applied
    EXPECT_FALSE(window->UpdateAdaptiveSurfaceScale(real, hooked));
    // changed ratio: re-apply
    Rect hooked2 = { 0, 0, 400, 800 };
    EXPECT_TRUE(window->UpdateAdaptiveSurfaceScale(real, hooked2));
    EXPECT_FLOAT_EQ(window->adaptiveSurfaceScaleX_, 2.5f);
    // no longer needed: reset
    window->adaptiveResolutionActive_.store(false);
    EXPECT_TRUE(window->UpdateAdaptiveSurfaceScale(real, hooked2));
    EXPECT_FALSE(window->adaptiveSurfaceScaleApplied_.load());
    EXPECT_FLOAT_EQ(window->adaptiveSurfaceScaleX_, 1.0f);
    // nothing applied and no scale needed again
    EXPECT_FALSE(window->UpdateAdaptiveSurfaceScale(real, hooked2));
}

/**
 * @tc.name: FlushLayoutSizeVariants
 * @tc.desc: tolerance matching on main windows; sub windows bypass the logic
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, FlushLayoutSizeVariants, TestSize.Level1)
{
    auto window = CreateTestWindow("FlushLayoutSize");
    ASSERT_NE(window, nullptr);
    window->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
    window->state_ = WindowState::STATE_SHOWN;
    // inactive: reported as-is
    window->FlushLayoutSize(500, 1000);
    EXPECT_EQ(window->layoutRect_.width_, 500);
    // active + exact match: reports the real rect
    ActivateAdaptive(window);
    window->FlushLayoutSize(500, 1000);
    EXPECT_EQ(window->layoutRect_.width_, 1000);
    EXPECT_EQ(window->layoutRect_.height_, 2000);
    // tolerance boundary: +/-2 counts as a match
    window->FlushLayoutSize(502, 998);
    EXPECT_EQ(window->layoutRect_.width_, 1000);
    window->FlushLayoutSize(498, 1002);
    EXPECT_EQ(window->layoutRect_.width_, 1000);
    // beyond tolerance: keeps the reported size
    window->FlushLayoutSize(503, 1000);
    EXPECT_EQ(window->layoutRect_.width_, 503);
    window->FlushLayoutSize(497, 1000);
    EXPECT_EQ(window->layoutRect_.width_, 497);

    // sub windows bypass the adaptive reporting logic
    auto sub = CreateTestWindow("FlushLayoutSizeSub", WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_NE(sub, nullptr);
    sub->property_->SetWindowRect(Rect { 0, 0, 1000, 2000 });
    ActivateAdaptive(sub);
    sub->FlushLayoutSize(500, 1000);
    EXPECT_EQ(sub->layoutRect_.width_, 0); // early return: not reported at all
}

/**
 * @tc.name: WindowManagerSetConfigFacadeVariants
 * @tc.desc: adapter fallthrough and the local device gate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAdaptiveTest, WindowManagerSetConfigFacadeVariants, TestSize.Level1)
{
    // no such window locally: the gate is skipped and the adapter fails
    // (SAMGR without the service, or the real service rejects the non-SA caller)
    GameAdaptiveResolutionConfig config;
    config.enable_ = true;
    config.scaleRatio_ = 0.5f;
    config.windowId_ = 4321;
    auto ret = WindowManager::GetInstance().SetGameAdaptiveResolutionConfig(config);
    EXPECT_NE(ret, WMError::WM_OK);

    // a found window on a non-phone/pad device is rejected before the adapter
    auto window = CreateTestWindow("DeviceGateWindow");
    ASSERT_NE(window, nullptr);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WindowSessionImpl::windowSessionMap_["DeviceGateWindow"] = { 0, window };
    config.windowId_ = window->GetWindowId();
    ret = WindowManager::GetInstance().SetGameAdaptiveResolutionConfig(config);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);
    // phone device: passes the gate and reaches the adapter
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = WindowManager::GetInstance().SetGameAdaptiveResolutionConfig(config);
    EXPECT_NE(ret, WMError::WM_OK);
    WindowSessionImpl::windowSessionMap_.erase("DeviceGateWindow");
}
}
} // namespace Rosen
} // namespace OHOS
