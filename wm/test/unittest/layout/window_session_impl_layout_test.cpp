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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "ability_context_impl.h"
#include "accessibility_event_info.h"
#include "color_parser.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "window_helper.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSessionImplLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
};

void WindowSessionImplLayoutTest::SetUpTestCase() {}

void WindowSessionImplLayoutTest::TearDownTestCase() {}

void WindowSessionImplLayoutTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSessionImplLayoutTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {
sptr<WindowSessionImpl> GetTestWindowImpl(const std::string& name)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { name, name, name };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    return window;
}

template <typename TListener, typename MockListener> std::vector<sptr<TListener>> GetListenerList()
{
    std::vector<sptr<TListener>> listeners;
    sptr<TListener> listener = sptr<TListener>::MakeSptr();
    listeners.insert(listeners.begin(), listener);
    return listeners;
}

/**
 * @tc.name: UpdateRect01
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, UpdateRect01, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: UpdateRect01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateRect01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.height_ = 50;
    rect.width_ = 50;

    Rect rectW; // GetRect().IsUninitializedRect is false
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 200; // rectW - rect > 50
    rectW.width_ = 200;  // rectW - rect > 50

    window->property_->SetWindowRect(rectW);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);

    rectW.height_ = 50;
    window->property_->SetWindowRect(rectW);
    res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);

    rectW.height_ = 200;
    rectW.width_ = 50;
    window->property_->SetWindowRect(rectW);
    res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
    Rect nowRect = window->property_->GetWindowRect();
    EXPECT_EQ(nowRect.posX_, rect.posX_);
    EXPECT_EQ(nowRect.posY_, rect.posY_);
    EXPECT_EQ(nowRect.width_, rect.width_);
    EXPECT_EQ(nowRect.height_, rect.height_);
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: UpdateRect01 end";
}

/**
 * @tc.name: UpdateRect02
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, UpdateRect02, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: UpdateRect02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateRect02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.height_ = 0;
    rect.width_ = 0;

    Rect rectW; // GetRect().IsUninitializedRect is true
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 0; // rectW - rect > 50
    rectW.width_ = 0;  // rectW - rect > 50

    window->property_->SetWindowRect(rectW);
    SizeChangeReason reason = SizeChangeReason::ROTATION;
    WSError res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);

    rect.height_ = 50;
    rect.width_ = 50;
    rectW.height_ = 50;
    rectW.width_ = 50;
    window->property_->SetWindowRect(rectW);
    res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
    Rect nowRect = window->property_->GetWindowRect();
    EXPECT_EQ(nowRect.posX_, rect.posX_);
    EXPECT_EQ(nowRect.posY_, rect.posY_);
    EXPECT_EQ(nowRect.width_, rect.width_);
    EXPECT_EQ(nowRect.height_, rect.height_);
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: UpdateRect02 end";
}

/**
 * @tc.name: SetResizeByDragEnabled01
 * @tc.desc: SetResizeByDragEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, SetResizeByDragEnabled01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetResizeByDragEnabled01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SetResizeByDragEnabled02
 * @tc.desc: SetResizeByDragEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, SetResizeByDragEnabled02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetResizeByDragEnabled02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    WMError retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_OK);
    ASSERT_EQ(true, window->property_->GetDragEnabled());
}

/**
 * @tc.name: SetResizeByDragEnabled03
 * @tc.desc: SetResizeByDragEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, SetResizeByDragEnabled03, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetResizeByDragEnabled03");
    option->SetSubWindowDecorEnable(true);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    WMError retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_OK);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_OK);

    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_TYPE);
}

/**
 * @tc.name: UpdateViewportConfig
 * @tc.desc: UpdateViewportConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, UpdateViewportConfig, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: UpdateViewportConfig start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    Rect rectW; // GetRect().IsUninitializedRect is true
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 0; // rectW - rect > 50
    rectW.width_ = 0;  // rectW - rect > 50

    window->virtualPixelRatio_ = -1.0;
    window->useUniqueDensity_ = true;
    WindowSizeChangeReason reason = WindowSizeChangeReason::UNDEFINED;
    window->UpdateViewportConfig(rectW, reason);
    ASSERT_EQ(window->virtualPixelRatio_, -1.0);

    window->virtualPixelRatio_ = -2.0;
    DisplayId displayId = 1;
    window->property_->SetDisplayId(displayId);
    window->UpdateViewportConfig(rectW, reason);
    ASSERT_EQ(window->virtualPixelRatio_, -2.0);

    displayId = 0;
    rectW.height_ = 500;
    rectW.width_ = 500;
    window->useUniqueDensity_ = false;
    window->property_->SetDisplayId(displayId);
    window->UpdateViewportConfig(rectW, reason);
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: UpdateViewportConfig end";
}

/**
 * @tc.name: UpdateViewportConfig01
 * @tc.desc: UpdateViewportConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, UpdateViewportConfig01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateViewportConfig01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Rect rectW;
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 0;
    rectW.width_ = 0;
    WindowSizeChangeReason reason = WindowSizeChangeReason::UNDEFINED;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    window->UpdateViewportConfig(rectW, reason, nullptr, displayInfo);
    rectW.width_ = 10;
    rectW.height_ = 0;
    window->UpdateViewportConfig(rectW, reason, nullptr, displayInfo);
    rectW.width_ = 10;
    rectW.height_ = 10;
    window->UpdateViewportConfig(rectW, reason, nullptr, displayInfo);
    ASSERT_NE(window, nullptr);
}

/**
 * @tc.name: NotifySingleHandTransformChange_TestUIContent
 * @tc.desc: NotifySingleHandTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifySingleHandTransformChange_TestUIContent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifySingleHandTransformChange_TestUIContent start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySingleHandTransformChange_TestUIContent");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2025);
    std::string url = "";
    window->SetUIContentInner(
        url, nullptr, nullptr, WindowSetUIContentType::DEFAULT, BackupAndRestoreType::NONE, nullptr);
    SingleHandTransform testTransform;
    testTransform.posX = 100;
    window->NotifySingleHandTransformChange(testTransform);
    ASSERT_EQ(testTransform.posX, window->singleHandTransform_.posX);
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifySingleHandTransformChange_TestUIContent end";
}

/**
 * @tc.name: NotifyTransformChange_TestUIContent
 * @tc.desc: NotifyTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyTransformChange_TestUIContent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyTransformChange_TestUIContent start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyTransformChange_TestUIContent");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2025);

    Transform testTransform;
    window->uiContent_ = nullptr;
    window->SetNeedRenotifyTransform(true);
    window->NotifyTransformChange(testTransform);
    ASSERT_EQ(true, window->IsNeedRenotifyTransform());

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->SetNeedRenotifyTransform(true);
    window->NotifyTransformChange(testTransform);
    ASSERT_EQ(false, window->IsNeedRenotifyTransform());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyTransformChange_TestUIContent end";
}

/**
 * @tc.name: NotifyWindowStatusDidChange
 * @tc.desc: NotifyWindowStatusDidChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyWindowStatusDidChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyWindowStatusDidChange start";
    auto window = GetTestWindowImpl("NotifyWindowStatusDidChange");
    auto listeners = GetListenerList<IWindowStatusDidChangeListener, MockWindowStatusDidChangeListener>();
    EXPECT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->windowStatusDidChangeListeners_.insert({ window->GetPersistentId(), listeners });
    window->NotifyWindowStatusDidChange(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyWindowStatusDidChange end";
}

/**
 * @tc.name: NotifyWindowStatusDidChange_Test_Undefined
 * @tc.desc: NotifyWindowStatusDidChange test UNDEFINED
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyWindowStatusDidChange_Test_Undefined, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyWindowStatusDidChange_Test_Undefined start";
    auto window = GetTestWindowImpl("NotifyWindowStatusDidChange");
    auto listeners = GetListenerList<IWindowStatusDidChangeListener, MockWindowStatusDidChangeListener>();
    EXPECT_NE(listeners.size(), 0);
    window->windowStatusDidChangeListeners_.insert({ window->GetPersistentId(), listeners });
    window->lastStatusWhenNotifyWindowStatusDidChange_.store(WindowStatus::WINDOW_STATUS_FULLSCREEN);
    window->NotifyWindowStatusDidChange(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(window->lastStatusWhenNotifyWindowStatusDidChange_, WindowStatus::WINDOW_STATUS_UNDEFINED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyWindowStatusDidChange_Test_Undefined end";
}

/**
 * @tc.name: NotifyParentWindowSizeChange
 * @tc.desc: NotifyParentWindowSizeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyParentWindowSizeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyParentWindowSizeChange start";
    auto window = GetTestWindowImpl("NotifyParentWindowSizeChange");
    auto listeners = GetListenerList<IParentWindowSizeChangeListener, MockParentWindowSizeChangeListener>();
    EXPECT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->parentWindowSizeChangeListeners_.insert({ window->GetPersistentId(), listeners });
    Rect rect = { 1, 2, 3, 4};
    window->NotifyParentWindowSizeChange(rect);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyParentWindowStatusChange end";
}

/**
 * @tc.name: NotifyParentWindowStatusChange
 * @tc.desc: NotifyParentWindowStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyParentWindowStatusChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyParentWindowStatusChange start";
    auto window = GetTestWindowImpl("NotifyParentWindowStatusChange");
    auto listeners = GetListenerList<IParentWindowStatusChangeListener, MockParentWindowStatusChangeListener>();
    EXPECT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->parentWindowStatusChangeListeners_.insert({ window->GetPersistentId(), listeners });
    window->NotifyParentWindowStatusChange(WindowMode::WINDOW_MODE_FLOATING, MaximizeMode::MODE_AVOID_SYSTEM_BAR, true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyParentWindowStatusChange end";
}

/**
 * @tc.name: NotifyParentWindowStatusChange_Test_Undefined
 * @tc.desc: NotifyParentWindowStatusChange test UNDEFINED
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyParentWindowStatusChange_Test_Undefined, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyWindowStatusDidChange_Test_Undefined start";
    auto window = GetTestWindowImpl("NotifyWindowStatusDidChange");
    auto listeners = GetListenerList<IParentWindowStatusChangeListener, MockParentWindowStatusChangeListener>();
    EXPECT_NE(listeners.size(), 0);
    window->parentWindowStatusChangelisteners_.insert({ window->GetPersistentId(), listeners });
    window->lastStatusWhenNotifyParentStatusChange_.store(WindowStatus::WINDOW_STATUS_FULLSCREEN);
    window->NotifyParentWindowStatusChange(WindowMode::WINDOW_MODE_UNDEFINED, MaximizeMode::MODE_FULL_FILL, true);
    EXPECT_EQ(window->lastStatusWhenNotifyParentStatusChange_, WindowStatus::WINDOW_STATUS_UNDEFINED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyWindowStatusDidChange_Test_Undefined end";
}

/**
 * @tc.name: NotifyAfterUIContentReady
 * @tc.desc: NotifyAfterUIContentReady
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyAfterUIContentReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyAfterUIContentReady start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyAfterUIContentReady");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2025);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->SetNeedRenotifyTransform(false);
    window->NotifyAfterUIContentReady();
    ASSERT_EQ(false, window->IsNeedRenotifyTransform());
    window->SetNeedRenotifyTransform(false);
    window->NotifyAfterUIContentReady();
    ASSERT_EQ(false, window->IsNeedRenotifyTransform());
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyAfterUIContentReady end";
}

/**
 * @tc.name: NotifyFirstValidLayoutUpdate
 * @tc.desc: NotifyFirstValidLayoutUpdate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, NotifyFirstValidLayoutUpdate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyFirstValidLayoutUpdate start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyFirstValidLayoutUpdate");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2025);

    Rect preRect = { 0, 0, 0, 0 };
    Rect newRect = { 0, 0, 0, 0 };
    window->isFirstValidLayoutUpdate_ = true;
    window->NotifyFirstValidLayoutUpdate(preRect, newRect);
    EXPECT_EQ(window->isFirstValidLayoutUpdate_, true);

    preRect = { 0, 0, 1000, 1000 };
    window->NotifyFirstValidLayoutUpdate(preRect, newRect);
    EXPECT_EQ(window->isFirstValidLayoutUpdate_, true);


    newRect = { 0, 0, 1000, 1000 };
    window->NotifyFirstValidLayoutUpdate(preRect, newRect);
    EXPECT_EQ(window->isFirstValidLayoutUpdate_, false);

    window->NotifyFirstValidLayoutUpdate(preRect, newRect);
    EXPECT_EQ(window->isFirstValidLayoutUpdate_, false);
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: NotifyFirstValidLayoutUpdate end";
}

/**
 * @tc.name: HookWindowSizeByHookWindowInfo
 * @tc.desc: HookWindowSizeByHookWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, HookWindowSizeByHookWindowInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: HookWindowSizeByHookWindowInfo start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HookWindowSizeByHookWindowInfo");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2025);

    // Case 1: not enable hookWindow
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = false;
    hookWindowInfo.widthHookRatio = 0.5f;
    window->SetAppHookWindowInfo(hookWindowInfo);
    const uint32_t defaultSize = 800;
    Rect rect = { 0, 0, defaultSize, defaultSize };
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);

    // Case 2: not main window
    hookWindowInfo.enableHookWindow = true;
    window->SetAppHookWindowInfo(hookWindowInfo);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);

    // Case 3: default window size hook ratio
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    hookWindowInfo.widthHookRatio = HookWindowInfo::DEFAULT_WINDOW_SIZE_HOOK_RATIO;
    window->SetAppHookWindowInfo(hookWindowInfo);
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);

    window->isFullScreenInForceSplit_.store(true);
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);
    window->isFullScreenInForceSplit_.store(false);

    // Case 4: success
    hookWindowInfo.widthHookRatio = 0.5f;
    window->SetAppHookWindowInfo(hookWindowInfo);
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_NE(rect.width_, defaultSize);

    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: HookWindowSizeByHookWindowInfo end";
}

HWTEST_F(WindowSessionImplLayoutTest, HookWindowSizeByDrawableRectHook, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: HookWindowSizeByDrawableRectHook start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HookWindowSizeByDrawableRectHook");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2026);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    const uint32_t defaultSize = 800;
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;

    // Case 1: drawableRectHook=false -> do not call hook
    hookWindowInfo.drawableRectHook = false;
    window->SetAppHookWindowInfo(hookWindowInfo);
    Rect drawableRect = { 0, 0, defaultSize, defaultSize };
    if (window->GetProperty()->GetHookWindowInfo().drawableRectHook) {
        window->HookWindowSizeByHookWindowInfo(drawableRect);
    }
    EXPECT_EQ(drawableRect.width_, defaultSize);

    // Case 2: drawableRectHook=true -> call hook, applied
    hookWindowInfo.drawableRectHook = true;
    window->SetAppHookWindowInfo(hookWindowInfo);
    drawableRect = { 0, 0, defaultSize, defaultSize };
    if (window->GetProperty()->GetHookWindowInfo().drawableRectHook) {
        window->HookWindowSizeByHookWindowInfo(drawableRect);
    }
    EXPECT_NE(drawableRect.width_, defaultSize);

    // Case 3: normal rect always hooks regardless of drawableRectHook
    hookWindowInfo.drawableRectHook = false;
    window->SetAppHookWindowInfo(hookWindowInfo);
    Rect normalRect = { 0, 0, defaultSize, defaultSize };
    window->HookWindowSizeByHookWindowInfo(normalRect);
    EXPECT_NE(normalRect.width_, defaultSize);

    GTEST_LOG_(INFO) << "WindowSessionImplLayoutTest: HookWindowSizeByDrawableRectHook end";
}

/**
 * @tc.name: ClearParentWindowListeners01
 * @tc.desc: ClearParentWindowListeners removes registered parent window listeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, ClearParentWindowListeners01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ClearParentWindowListeners01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    int32_t persistentId = 9999;
    window->property_->SetPersistentId(persistentId);
    ASSERT_NE(window, nullptr);

    // Register a parent window size change listener
    sptr<MockParentWindowSizeChangeListener> sizeListener = sptr<MockParentWindowSizeChangeListener>::MakeSptr();
    window->RegisterParentWindowSizeChangeListener(sizeListener);

    // Register a parent window status change listener
    sptr<MockParentWindowStatusChangeListener> statusListener = sptr<MockParentWindowStatusChangeListener>::MakeSptr();
    window->RegisterParentWindowStatusChangeListener(statusListener);

    // Clear listeners
    window->ClearParentWindowListeners(persistentId);

    // Verify listeners are cleared by checking the map entry is removed
    EXPECT_EQ(WindowSessionImpl::parentWindowSizeChangeListeners_.count(persistentId), 0u);
    EXPECT_EQ(WindowSessionImpl::parentWindowStatusChangeListeners_.count(persistentId), 0u);
}

/**
 * @tc.name: ClearParentWindowListeners02
 * @tc.desc: ClearParentWindowListeners with no registered listeners → no crash
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplLayoutTest, ClearParentWindowListeners02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ClearParentWindowListeners02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    int32_t persistentId = 8888;
    window->property_->SetPersistentId(persistentId);
    ASSERT_NE(window, nullptr);

    // No listeners registered, should not crash
    window->ClearParentWindowListeners(persistentId);
    EXPECT_EQ(WindowSessionImpl::parentWindowSizeChangeListeners_.count(persistentId), 0u);
}
}
} // namespace Rosen
} // namespace OHOS
