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

#include "window_session_impl.h"
#include "window_scene_session_impl.h"

#include <gtest/gtest.h>
#include <transaction/rs_transaction.h>

#include "ability_context_impl.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg = msg;
    }
}

namespace OHOS {
namespace Rosen {
class WindowSessionImplTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowSessionImpl> window_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
};

void WindowSessionImplTest2::SetUpTestCase() {}

void WindowSessionImplTest2::TearDownTestCase() {}

void WindowSessionImplTest2::SetUp() {}

void WindowSessionImplTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    if (window_ != nullptr) {
        window_->Destroy();
    }
}

namespace {
sptr<WindowSessionImpl> GetTestWindowImpl(const std::string& name)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    if (option == nullptr) {
        return nullptr;
    }
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    if (window == nullptr) {
        return nullptr;
    }

    SessionInfo sessionInfo = { name, name, name };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    if (session == nullptr) {
        return nullptr;
    }

    window->hostSession_ = session;
    auto runner = AppExecFwk::EventRunner::Create("WindowSessionImpl");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    window->handler_ = handler;
    return window;
}

template <typename TListener, typename MockListener> std::vector<sptr<TListener>> GetListenerList()
{
    std::vector<sptr<TListener>> listeners;
    sptr<TListener> listener = new (std::nothrow) MockListener();
    if (listener == nullptr) {
        return listeners;
    }

    listeners.insert(listeners.begin(), listener);
    return listeners;
}

/**
 * @tc.name: GetTitleButtonVisible
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetTitleButtonVisible, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetTitleButtonVisible");
    ASSERT_NE(window, nullptr);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    bool isMaximizeVisible = false;
    bool isMinimizeVisible = false;
    bool isSplitVisible = false;
    bool isCloseVisible = false;
    window->GetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible);
    ASSERT_FALSE(isSplitVisible);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->windowTitleVisibleFlags_.isSplitVisible = false;
    window->windowTitleVisibleFlags_.isMaximizeVisible = false;
    window->windowTitleVisibleFlags_.isMinimizeVisible = false;
    window->windowTitleVisibleFlags_.isCloseVisible = false;
    window->GetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible);
    ASSERT_TRUE(isSplitVisible);
    ASSERT_TRUE(isMaximizeVisible);
    ASSERT_TRUE(isMinimizeVisible);
    ASSERT_TRUE(isCloseVisible);

    window->windowTitleVisibleFlags_.isSplitVisible = true;
    window->windowTitleVisibleFlags_.isMaximizeVisible = true;
    window->windowTitleVisibleFlags_.isMinimizeVisible = true;
    window->windowTitleVisibleFlags_.isCloseVisible = true;
    window->GetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible);
    ASSERT_TRUE(isSplitVisible);
    ASSERT_TRUE(isMaximizeVisible);
    ASSERT_TRUE(isMinimizeVisible);
    ASSERT_TRUE(isCloseVisible);
}

/**
 * @tc.name: GetSystemSessionConfig
 * @tc.desc: GetSystemSessionConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetSystemSessionConfig, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetSystemSessionConfig");
    ASSERT_NE(window, nullptr);
    window->GetSystemSessionConfig();
    window->Destroy();
}

/**
 * @tc.name: GetColorSpaceFromSurfaceGamut
 * @tc.desc: GetColorSpaceFromSurfaceGamut
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetColorSpaceFromSurfaceGamut, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetColorSpaceFromSurfaceGamut");
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(window->GetColorSpaceFromSurfaceGamut(GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB),
              ColorSpace::COLOR_SPACE_DEFAULT);
    ASSERT_EQ(window->GetColorSpaceFromSurfaceGamut(GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3),
              ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    ASSERT_EQ(window->GetColorSpaceFromSurfaceGamut(GraphicColorGamut::GRAPHIC_COLOR_GAMUT_INVALID),
              ColorSpace::COLOR_SPACE_DEFAULT);
    window->Destroy();
}

/**
 * @tc.name: GetSurfaceGamutFromColorSpace
 * @tc.desc: GetSurfaceGamutFromColorSpace
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetSurfaceGamutFromColorSpace, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetSurfaceGamutFromColorSpace");
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(window->GetSurfaceGamutFromColorSpace(ColorSpace::COLOR_SPACE_DEFAULT),
              GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB);
    ASSERT_EQ(window->GetSurfaceGamutFromColorSpace(ColorSpace::COLOR_SPACE_WIDE_GAMUT),
              GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3);
    ASSERT_EQ(window->GetSurfaceGamutFromColorSpace(ColorSpace(uint32_t(3))),
              GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB);
    window->Destroy();
}

/**
 * @tc.name: Create
 * @tc.desc: Create
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, Create, TestSize.Level1)
{
    auto window = GetTestWindowImpl("Create");
    ASSERT_NE(window, nullptr);
    std::shared_ptr<AbilityRuntime::Context> context;
    sptr<Rosen::ISession> ISession;
    ASSERT_EQ(window->Create(context, ISession), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: Destroy
 * @tc.desc: Destroy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, Destroy, TestSize.Level1)
{
    auto window = GetTestWindowImpl("Destroy");
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(INVALID_SESSION_ID);
    ASSERT_EQ(window->Destroy(true, true), WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_DESTROYED;
    ASSERT_EQ(window->Destroy(true, true), WMError::WM_ERROR_INVALID_WINDOW);

    window->state_ = WindowState::STATE_INITIAL;
    ASSERT_EQ(window->Destroy(true, true), WMError::WM_OK);

    window = GetTestWindowImpl("Destroy");
    ASSERT_NE(window, nullptr);
    window->state_ = WindowState::STATE_INITIAL;
    window->property_->SetPersistentId(1);
    ASSERT_EQ(window->Destroy(true, false), WMError::WM_OK);

    window->hostSession_ = nullptr;
    ASSERT_EQ(window->Destroy(true, true), WMError::WM_ERROR_INVALID_WINDOW);

    window = GetTestWindowImpl("Destroy");
    ASSERT_NE(window, nullptr);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = hostSession;
    ASSERT_EQ(window->Destroy(true, true), WMError::WM_ERROR_INVALID_WINDOW);

    window = GetTestWindowImpl("Destroy");
    ASSERT_NE(window, nullptr);
    window->hostSession_ = hostSession;
    window->state_ = WindowState::STATE_INITIAL;
    window->property_->SetPersistentId(1);
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_EQ(window->Destroy(true, true), WMError::WM_OK);
}

/**
 * @tc.name: GetWindowState
 * @tc.desc: GetWindowState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetWindowState, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetWindowState");
    ASSERT_NE(window, nullptr);
    window->state_ = WindowState::STATE_DESTROYED;
    ASSERT_EQ(window->GetWindowState(), WindowState::STATE_DESTROYED);
    window->Destroy();
}

/**
 * @tc.name: RecoverSessionListener
 * @tc.desc: RecoverSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RecoverSessionListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RecoverSessionListener");
    ASSERT_NE(window, nullptr);
    int32_t id = 1;
    window->property_->SetPersistentId(id);
    window->RecoverSessionListener();

    std::vector<sptr<IAcrossDisplaysChangeListener>> iAcrossDisplaysChangeListener;
    std::vector<sptr<IAvoidAreaChangedListener>> iAvoidAreaChangedListeners;
    std::vector<sptr<ITouchOutsideListener>> iTouchOutsideListeners;
    window->avoidAreaChangeListeners_.insert({ id, iAvoidAreaChangedListeners });
    window->acrossDisplaysChangeListeners_.insert({ id, iAcrossDisplaysChangeListener });
    window->touchOutsideListeners_.insert({ id, iTouchOutsideListeners });
    window->RecoverSessionListener();

    window->avoidAreaChangeListeners_.clear();
    window->acrossDisplaysChangeListeners_.clear();
    window->touchOutsideListeners_.clear();
    sptr<MockAvoidAreaChangedListener> changedListener = sptr<MockAvoidAreaChangedListener>::MakeSptr();
    sptr<MockTouchOutsideListener> touchOutsideListener = sptr<MockTouchOutsideListener>::MakeSptr();
    sptr<MockAcrossDisplaysChangeListener> changedListener2 = sptr<MockAcrossDisplaysChangeListener>::MakeSptr();
    iAvoidAreaChangedListeners.insert(iAvoidAreaChangedListeners.begin(), changedListener);
    iAcrossDisplaysChangeListener.insert(iAcrossDisplaysChangeListener.begin(), changedListener2);
    iTouchOutsideListeners.insert(iTouchOutsideListeners.begin(), touchOutsideListener);
    window->avoidAreaChangeListeners_.insert({ id, iAvoidAreaChangedListeners });
    window->acrossDisplaysChangeListeners_.insert({ id, iAcrossDisplaysChangeListener });
    window->touchOutsideListeners_.insert({ id, iTouchOutsideListeners });
    std::vector<sptr<IOcclusionStateChangedListener>> occlusionStateChangeListeners;
    occlusionStateChangeListeners.push_back(nullptr);
    window->occlusionStateChangeListeners_.clear();
    window->occlusionStateChangeListeners_.insert({ id, occlusionStateChangeListeners });
    window->RecoverSessionListener();
    window->occlusionStateChangeListeners_.clear();
    ASSERT_TRUE(window->avoidAreaChangeListeners_.find(id) != window->avoidAreaChangeListeners_.end() &&
                !window->avoidAreaChangeListeners_[id].empty());
    ASSERT_TRUE(window->touchOutsideListeners_.find(id) != window->touchOutsideListeners_.end() &&
                !window->touchOutsideListeners_[id].empty());
    ASSERT_TRUE(window->acrossDisplaysChangeListeners_.find(id) != window->acrossDisplaysChangeListeners_.end() &&
                !window->acrossDisplaysChangeListeners_[id].empty());
    window->Destroy();
}

/**
 * @tc.name: RegisterOcclusionStateChangeListener
 * @tc.desc: register occlusion state change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterOcclusionStateChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RegisterOcclusionStateChangeListener");
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    window->occlusionStateChangeListeners_.clear();
    EXPECT_NE(window->RegisterOcclusionStateChangeListener(nullptr), WMError::WM_OK);
    sptr<IOcclusionStateChangedListener> listener = sptr<IOcclusionStateChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterOcclusionStateChangeListener(listener), WMError::WM_OK);
    EXPECT_EQ(window->occlusionStateChangeListeners_.size(), 1);
    window->occlusionStateChangeListeners_[window->GetPersistentId()].push_back(nullptr);
    sptr<IOcclusionStateChangedListener> listener2 = sptr<IOcclusionStateChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterOcclusionStateChangeListener(listener2), WMError::WM_OK);
    window->occlusionStateChangeListeners_.clear();
    window->Destroy();
}

/**
 * @tc.name: UnregisterOcclusionStateChangeListener
 * @tc.desc: unregister occlusion state change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UnregisterOcclusionStateChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UnregisterOcclusionStateChangeListener");
    ASSERT_NE(window, nullptr);
    window->occlusionStateChangeListeners_.clear();
    EXPECT_NE(window->UnregisterOcclusionStateChangeListener(nullptr), WMError::WM_OK);
    sptr<IOcclusionStateChangedListener> listener = sptr<IOcclusionStateChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterOcclusionStateChangeListener(listener), WMError::WM_OK);
    sptr<IOcclusionStateChangedListener> listener2 = sptr<IOcclusionStateChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterOcclusionStateChangeListener(listener2), WMError::WM_OK);
    EXPECT_EQ(window->UnregisterOcclusionStateChangeListener(listener), WMError::WM_OK);
    EXPECT_EQ(window->occlusionStateChangeListeners_.size(), 1);
    EXPECT_EQ(window->UnregisterOcclusionStateChangeListener(listener2), WMError::WM_OK);
    window->occlusionStateChangeListeners_.clear();
    EXPECT_EQ(window->occlusionStateChangeListeners_.size(), 0);
    window->Destroy();
}

/**
 * @tc.name: NotifyWindowOcclusionState
 * @tc.desc: unregister occlusion state change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyWindowOcclusionState, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyWindowOcclusionState");
    ASSERT_NE(window, nullptr);
    window->occlusionStateChangeListeners_.clear();
    sptr<IOcclusionStateChangedListener> listener = sptr<IOcclusionStateChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterOcclusionStateChangeListener(listener), WMError::WM_OK);
    EXPECT_EQ(window->occlusionStateChangeListeners_.size(), 1);
    EXPECT_EQ(window->NotifyWindowOcclusionState(WindowVisibilityState::END), WSError::WS_OK);
    EXPECT_EQ(window->lastVisibilityState_, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    EXPECT_EQ(window->NotifyWindowOcclusionState(WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION),
        WSError::WS_OK);
    EXPECT_EQ(window->lastVisibilityState_, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    window->occlusionStateChangeListeners_.clear();
    window->Destroy();
}

/**
 * @tc.name: RegisterFrameMetricsChangeListener
 * @tc.desc: register frame metrics change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterFrameMetricsChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RegisterFrameMetricsChangeListener");
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    window->frameMetricsChangeListeners_.clear();
    window->uiContent_ = nullptr;
    EXPECT_NE(window->RegisterFrameMetricsChangeListener(nullptr), WMError::WM_OK);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_NE(window->RegisterFrameMetricsChangeListener(nullptr), WMError::WM_OK);
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, SetFrameMetricsCallBack(_));
    sptr<IFrameMetricsChangedListener> listener = sptr<IFrameMetricsChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterFrameMetricsChangeListener(listener), WMError::WM_OK);
    EXPECT_EQ(window->frameMetricsChangeListeners_.size(), 1);
    sptr<IFrameMetricsChangedListener> listener2 = sptr<IFrameMetricsChangedListener>::MakeSptr();
    window->frameMetricsChangeListeners_[window->GetPersistentId()].push_back(listener2);
    sptr<IFrameMetricsChangedListener> listener3 = sptr<IFrameMetricsChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterFrameMetricsChangeListener(listener3), WMError::WM_OK);
    window->frameMetricsChangeListeners_.clear();
    window->Destroy();
}

/**
 * @tc.name: UnregisterFrameMetricsChangeListener
 * @tc.desc: unregister frame metrics change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UnregisterFrameMetricsChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UnregisterFrameMetricsChangeListener");
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    window->frameMetricsChangeListeners_.clear();
    window->uiContent_ = nullptr;
    EXPECT_NE(window->RegisterFrameMetricsChangeListener(nullptr), WMError::WM_OK);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_NE(window->UnregisterFrameMetricsChangeListener(nullptr), WMError::WM_OK);
    Ace::UIContentMocker* content = reinterpret_cast<Ace::UIContentMocker*>(window->uiContent_.get());
    EXPECT_CALL(*content, SetFrameMetricsCallBack(_));
    sptr<IFrameMetricsChangedListener> listener = sptr<IFrameMetricsChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterFrameMetricsChangeListener(listener), WMError::WM_OK);
    sptr<IFrameMetricsChangedListener> listener2 = sptr<IFrameMetricsChangedListener>::MakeSptr();
    EXPECT_EQ(window->RegisterFrameMetricsChangeListener(listener2), WMError::WM_OK);
    EXPECT_EQ(window->UnregisterFrameMetricsChangeListener(listener), WMError::WM_OK);
    EXPECT_CALL(*content, SetFrameMetricsCallBack(_));
    EXPECT_EQ(window->UnregisterFrameMetricsChangeListener(listener2), WMError::WM_OK);
    window->frameMetricsChangeListeners_.clear();
    EXPECT_EQ(window->frameMetricsChangeListeners_.size(), 0);
    window->Destroy();
}

/**
 * @tc.name: NotifyFrameMetrics
 * @tc.desc: unregister frame metrics change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyFrameMetrics, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyFrameMetrics");
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    window->frameMetricsChangeListeners_.clear();
    sptr<IFrameMetricsChangedListener> listener = sptr<IFrameMetricsChangedListener>::MakeSptr();
    window->frameMetricsChangeListeners_[window->GetPersistentId()].push_back(listener);
    Ace::FrameMetrics metric;
    window->NotifyFrameMetrics(metric);
    window->frameMetricsChangeListeners_.clear();
    window->Destroy();
}

/**
 * @tc.name: UpdateViewportConfig_KeyFrame
 * @tc.desc: UpdateViewportConfig_KeyFrame
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateViewportConfig_KeyFrame, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UpdateViewportConfig_KeyFrame");
    ASSERT_NE(window, nullptr);
    Rect rect{ 10, 10, 10, 10 };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    KeyFramePolicy keyFramePolicy;
    window->SetStageKeyFramePolicy(keyFramePolicy);
    WindowSizeChangeReason reason = WindowSizeChangeReason::UNDEFINED;
    window->UpdateViewportConfig(rect, reason, nullptr, displayInfo, avoidAreas);
    ASSERT_EQ(window->keyFramePolicy_.stopping_, false);
    reason = WindowSizeChangeReason::DRAG_END;
    window->UpdateViewportConfig(rect, reason, nullptr, displayInfo, avoidAreas);
    ASSERT_EQ(window->keyFramePolicy_.stopping_, false);
    window->Destroy();
}

/**
 * @tc.name: RegisterKeyFrameCallback
 * @tc.desc: RegisterKeyFrameCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterKeyFrameCallback, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RegisterKeyFrameCallback");
    ASSERT_NE(window, nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->RegisterKeyFrameCallback();
    window->uiContent_ = nullptr;
    window->RegisterKeyFrameCallback();
    window->Destroy();
}

/**
 * @tc.name: LinkKeyFrameNode
 * @tc.desc: LinkKeyFrameNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, LinkKeyFrameNode, TestSize.Level1)
{
    auto window = GetTestWindowImpl("LinkKeyFrameNode");
    ASSERT_NE(window, nullptr);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    auto hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->uiContent_ = nullptr;
    window->hostSession_ = nullptr;
    ASSERT_EQ(window->LinkKeyFrameNode(), WSError::WS_ERROR_NULLPTR);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(window->LinkKeyFrameNode(), WSError::WS_ERROR_NULLPTR);
    window->uiContent_ = nullptr;
    window->hostSession_ = hostSession;
    ASSERT_EQ(window->LinkKeyFrameNode(), WSError::WS_ERROR_NULLPTR);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(window->LinkKeyFrameNode(), WSError::WS_OK);
    window->Destroy();
}

/**
 * @tc.name: SetStageKeyFramePolicy
 * @tc.desc: SetStageKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetStageKeyFramePolicy, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SetStageKeyFramePolicy");
    ASSERT_NE(window, nullptr);
    KeyFramePolicy keyFramePolicy;
    ASSERT_EQ(window->SetStageKeyFramePolicy(keyFramePolicy), WSError::WS_OK);
    ASSERT_EQ(window->keyFramePolicy_, keyFramePolicy);
    window->Destroy();
}

/**
 * @tc.name: NotifyUIContentFocusStatus
 * @tc.desc: NotifyUIContentFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyUIContentFocusStatus, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyUIContentFocusStatus");
    ASSERT_NE(window, nullptr);
    window->isFocused_ = false;
    window->NotifyUIContentFocusStatus();

    window->isFocused_ = true;
    window->NotifyUIContentFocusStatus();

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->NotifyUIContentFocusStatus();
    window->Destroy();
}

/**
 * @tc.name: NotifyForegroundFailed
 * @tc.desc: NotifyForegroundFailed
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyForegroundFailed, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyForegroundFailed");
    ASSERT_NE(window, nullptr);
    window->NotifyForegroundFailed(WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: NotifyTransferComponentDataSync
 * @tc.desc: NotifyTransferComponentDataSync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyTransferComponentDataSync, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyTransferComponentDataSync");
    ASSERT_NE(window, nullptr);
    AAFwk::WantParams wantParams;
    AAFwk::WantParams reWantParams;
    ASSERT_EQ(WSErrorCode::WS_OK, window->NotifyTransferComponentDataSync(wantParams, reWantParams));
    window->Destroy();
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateAvoidArea, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UpdateAvoidArea");
    ASSERT_NE(window, nullptr);
    sptr<AvoidArea> avoidArea = sptr<AvoidArea>::MakeSptr();
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    ASSERT_EQ(WSError::WS_OK, window->UpdateAvoidArea(avoidArea, type));
    window->Destroy();
}

/**
 * @tc.name: HandleEscKeyEvent001
 * @tc.desc: HandleEscKeyEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, HandleEscKeyEvent001, TestSize.Level1)
{
    auto window = GetTestWindowImpl("HandleEscKeyEvent001");
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    ASSERT_NE(window->property_, nullptr);
    bool isConsumed = false;
    window->escKeyEventTriggered_ = false;
    window->escKeyHasDown_ = true;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->HandleEscKeyEvent(keyEvent, isConsumed));

    keyEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE);
    EXPECT_EQ(true, keyEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE));
    WMError result = window->HandleEscKeyEvent(keyEvent, isConsumed);
    EXPECT_EQ(result, WMError::WM_OK);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->HandleEscKeyEvent(keyEvent, isConsumed));
    window->Destroy();
}

/**
 * @tc.name: HandleEscKeyEvent002
 * @tc.desc: HandleEscKeyEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, HandleEscKeyEvent002, TestSize.Level1)
{
    auto window = GetTestWindowImpl("HandleEscKeyEvent002");
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    bool isConsumed = false;
    WMError result = window->HandleEscKeyEvent(keyEvent, isConsumed);
    EXPECT_EQ(result, WMError::WM_ERROR_NULLPTR);
    window->Destroy();
}

/**
 * @tc.name: HandleEscKeyEvent003
 * @tc.desc: HandleEscKeyEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, HandleEscKeyEvent003, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleEscKeyEvent003");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = true;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_A);
    EXPECT_EQ(WMError::WM_DO_NOTHING, window->HandleEscKeyEvent(keyEvent, isConsumed));
    isConsumed = false;
    EXPECT_EQ(WMError::WM_DO_NOTHING, window->HandleEscKeyEvent(keyEvent, isConsumed));

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    keyEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE);
    EXPECT_EQ(true, keyEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE));

    WMError result = window->HandleEscKeyEvent(keyEvent, isConsumed);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: HandleEscKeyEvent004
 * @tc.desc: HandleEscKeyEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, HandleEscKeyEvent004, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleEscKeyEvent004");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE);
    EXPECT_EQ(true, keyEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->escKeyEventTriggered_ = false;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->SetImmersiveModeEnabledState(true);
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyEventTriggered_ = true;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));
}

/**
 * @tc.name: HandleEscKeyEvent005
 * @tc.desc: HandleEscKeyEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, HandleEscKeyEvent005, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleEscKeyEvent005");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    keyEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE);
    EXPECT_EQ(true, keyEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE));

    window->escKeyEventTriggered_ = true;
    window->escKeyHasDown_ = true;
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyEventTriggered_ = false;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyHasDown_ = false;
    window->escKeyEventTriggered_ = true;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyEventTriggered_ = false;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyEventTriggered_ = true;
    window->escKeyHasDown_ = true;
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyEventTriggered_ = false;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyHasDown_ = false;
    window->escKeyEventTriggered_ = true;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));

    window->escKeyEventTriggered_ = false;
    EXPECT_EQ(WMError::WM_OK, window->HandleEscKeyEvent(keyEvent, isConsumed));
}

/**
 * @tc.name: DispatchKeyEventCallback
 * @tc.desc: DispatchKeyEventCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, DispatchKeyEventCallback, TestSize.Level1)
{
    auto window = GetTestWindowImpl("DispatchKeyEventCallback");
    ASSERT_NE(window, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_VIRTUAL_MULTITASK);
    bool isConsumed = false;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    ASSERT_FALSE(isConsumed);

    std::shared_ptr<MockInputEventConsumer> inputEventConsumer = std::make_shared<MockInputEventConsumer>();
    window->inputEventConsumer_ = inputEventConsumer;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    window->inputEventConsumer_ = nullptr;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->DispatchKeyEventCallback(keyEvent, isConsumed);

    keyEvent->SetKeyAction(MMI::KeyEvent::KEYCODE_ESCAPE);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    window->Destroy();
}


/**
 * @tc.name: DispatchKeyEventCallback001
 * @tc.desc: DispatchKeyEventCallback test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, DispatchKeyEventCallback001, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DispatchKeyEventCallback001");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    bool isConsumed = false;
    window->uiContent_ = nullptr;
    std::shared_ptr<MockInputEventConsumer> inputEventConsumer = std::make_shared<MockInputEventConsumer>();
    window->inputEventConsumer_ = inputEventConsumer;
    ASSERT_NE(window->inputEventConsumer_, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);

    isConsumed = false;
    window->inputEventConsumer_ = nullptr;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(window->uiContent_, nullptr);

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, true);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, false);
    EXPECT_EQ(window->escKeyEventTriggered_, false);
}

/**
 * @tc.name: DispatchKeyEventCallback002
 * @tc.desc: DispatchKeyEventCallback test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, DispatchKeyEventCallback002, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DispatchKeyEventCallback002");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    window->inputEventConsumer_ = nullptr;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(window->uiContent_, nullptr);

    bool isConsumed = false;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    window->escKeyEventTriggered_ = false;
    window->escKeyHasDown_ = true;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, false);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    window->escKeyEventTriggered_ = false;
    window->escKeyHasDown_ = false;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, false);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    window->escKeyEventTriggered_ = true;
    window->escKeyHasDown_ = false;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, false);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    window->escKeyEventTriggered_ = true;
    window->escKeyHasDown_ = true;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, false);
    EXPECT_EQ(window->escKeyEventTriggered_, false);
}

/**
 * @tc.name: DispatchKeyEventCallback003
 * @tc.desc: DispatchKeyEventCallback test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, DispatchKeyEventCallback003, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DispatchKeyEventCallback003");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    window->inputEventConsumer_ = nullptr;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(window->uiContent_, nullptr);

    bool isConsumed = false;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_ESCAPE);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    window->escKeyEventTriggered_ = false;
    window->escKeyHasDown_ = false;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, true);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    window->escKeyEventTriggered_ = false;
    window->escKeyHasDown_ = true;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, true);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    window->escKeyEventTriggered_ = true;
    window->escKeyHasDown_ = false;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, true);
    EXPECT_EQ(window->escKeyEventTriggered_, false);

    window->escKeyEventTriggered_ = true;
    window->escKeyHasDown_ = true;
    window->DispatchKeyEventCallback(keyEvent, isConsumed);
    EXPECT_EQ(window->escKeyHasDown_, true);
    EXPECT_EQ(window->escKeyEventTriggered_, false);
}

/**
 * @tc.name: HandleBackEvent01
 * @tc.desc: HandleBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, HandleBackEvent01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleBackEvent01");
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);

    windowSession->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(WSError::WS_OK, windowSession->HandleBackEvent());
}

/**
 * @tc.name: IsKeyboardEvent
 * @tc.desc: IsKeyboardEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, IsKeyboardEvent, TestSize.Level1)
{
    auto window = GetTestWindowImpl("IsKeyboardEvent");
    ASSERT_NE(window, nullptr);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_VIRTUAL_MULTITASK);
    ASSERT_FALSE(window->IsKeyboardEvent(keyEvent));

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN);
    ASSERT_TRUE(window->IsKeyboardEvent(keyEvent));

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    ASSERT_TRUE(window->IsKeyboardEvent(keyEvent));

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_FN);
    ASSERT_TRUE(window->IsKeyboardEvent(keyEvent));

    window->Destroy();
}

/**
 * @tc.name: GetVSyncPeriod
 * @tc.desc: GetVSyncPeriod
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetVSyncPeriod, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetVSyncPeriod");
    ASSERT_NE(window, nullptr);

    auto vsyncStation = window->vsyncStation_;
    if (vsyncStation == nullptr) {
        vsyncStation = std::make_shared<VsyncStation>(DisplayId(0));
    }
    window->vsyncStation_ = nullptr;
    ASSERT_EQ(window->GetVSyncPeriod(), 0);

    window->vsyncStation_ = vsyncStation;
    window->GetVSyncPeriod();
    window->Destroy();
}

/**
 * @tc.name: FlushFrameRate
 * @tc.desc: FlushFrameRate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, FlushFrameRate, TestSize.Level1)
{
    auto window = GetTestWindowImpl("FlushFrameRate");
    ASSERT_NE(window, nullptr);

    auto vsyncStation = window->vsyncStation_;
    if (vsyncStation == nullptr) {
        vsyncStation = std::make_shared<VsyncStation>(DisplayId(0));
    }
    window->vsyncStation_ = nullptr;
    window->FlushFrameRate(1, -1);

    window->vsyncStation_ = vsyncStation;
    window->FlushFrameRate(1, -1);
    window->Destroy();
}

/**
 * @tc.name: FindWindowById
 * @tc.desc: FindWindowById
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, FindWindowById, TestSize.Level1)
{
    auto window = GetTestWindowImpl("FindWindowById");
    ASSERT_NE(window, nullptr);
    window->windowSessionMap_.clear();
    ASSERT_EQ(window->FindWindowById(0), nullptr);

    window->windowSessionMap_.insert({ "test1", std::pair<int32_t, sptr<WindowSessionImpl>>(1, window) });
    window->windowSessionMap_.insert({ "test2", std::pair<int32_t, sptr<WindowSessionImpl>>(2, window) });
    ASSERT_EQ(window->FindWindowById(0), nullptr);

    window->windowSessionMap_.insert({ "test0", std::pair<int32_t, sptr<WindowSessionImpl>>(0, window) });
    ASSERT_NE(window->FindWindowById(0), nullptr);
    window->Destroy();
}

/**
 * @tc.name: SetLayoutFullScreenByApiVersion
 * @tc.desc: SetLayoutFullScreenByApiVersion
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetLayoutFullScreenByApiVersion, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SetLayoutFullScreenByApiVersion");
    ASSERT_NE(window, nullptr);
    window->windowSessionMap_.clear();
    ASSERT_EQ(window->SetLayoutFullScreenByApiVersion(true), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: SetSystemBarProperty
 * @tc.desc: SetSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetSystemBarProperty, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SetSystemBarProperty");
    ASSERT_NE(window, nullptr);
    window->windowSessionMap_.clear();
    SystemBarProperty property;
    ASSERT_EQ(window->SetSystemBarProperty(WindowType::APP_MAIN_WINDOW_END, property), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: SetSpecificBarProperty
 * @tc.desc: SetSpecificBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetSpecificBarProperty, TestSize.Level0)
{
    auto window = GetTestWindowImpl("SetSpecificBarProperty");
    ASSERT_NE(window, nullptr);
    window->windowSessionMap_.clear();
    SystemBarProperty property;
    ASSERT_EQ(window->SetSpecificBarProperty(WindowType::APP_MAIN_WINDOW_END, property), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfo
 * @tc.desc: NotifyOccupiedAreaChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyOccupiedAreaChangeInfo, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyOccupiedAreaChangeInfo");
    ASSERT_NE(window, nullptr);

    auto listeners = GetListenerList<IOccupiedAreaChangeListener, MockIOccupiedAreaChangeListener>();
    ASSERT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->occupiedAreaChangeListeners_.insert({ window->GetPersistentId(), listeners });

    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->windowSessionMap_.insert(
        { "test1", std::pair<int32_t, sptr<WindowSessionImpl>>(window->GetPersistentId(), nullptr) });
    window->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});
    window->windowSessionMap_.clear();

    window->windowSessionMap_.insert(
        { "test1", std::pair<int32_t, sptr<WindowSessionImpl>>(window->GetPersistentId(), window) });
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});

    window->handler_ = nullptr;
    window->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});
    window->Destroy();
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfoInner
 * @tc.desc: NotifyOccupiedAreaChangeInfoInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyOccupiedAreaChangeInfoInner, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyOccupiedAreaChangeInfoInner");
    ASSERT_NE(window, nullptr);

    auto listeners = GetListenerList<IOccupiedAreaChangeListener, MockIOccupiedAreaChangeListener>();
    ASSERT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->occupiedAreaChangeListeners_.insert({ window->GetPersistentId(), listeners });

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->NotifyOccupiedAreaChangeInfoInner(info);

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->NotifyOccupiedAreaChangeInfoInner(info);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->NotifyOccupiedAreaChangeInfoInner(info);

    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->NotifyOccupiedAreaChangeInfoInner(info);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->NotifyOccupiedAreaChangeInfoInner(info);

    window->Destroy();
}

/**
 * @tc.name: NotifyWindowStatusChange
 * @tc.desc: NotifyWindowStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyWindowStatusChange, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyWindowStatusChange");
    ASSERT_NE(window, nullptr);

    auto listeners = GetListenerList<IWindowStatusChangeListener, MockWindowStatusChangeListener>();
    ASSERT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->windowStatusChangeListeners_.insert({ window->GetPersistentId(), listeners });

    WindowMode mode = WindowMode::WINDOW_MODE_FLOATING;
    window->state_ = WindowState::STATE_HIDDEN;
    window->property_->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    window->NotifyWindowStatusChange(mode);

    window->state_ = WindowState::STATE_FROZEN;
    window->property_->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    window->NotifyWindowStatusChange(mode);

    mode = WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
    window->NotifyWindowStatusChange(mode);

    mode = WindowMode::WINDOW_MODE_SPLIT_SECONDARY;
    window->NotifyWindowStatusChange(mode);

    mode = WindowMode::WINDOW_MODE_PIP;
    window->NotifyWindowStatusChange(mode);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: NotifyWindowStatusChange02
 * @tc.desc: NotifyWindowStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyWindowStatusChange02, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyWindowStatusChange02");
    ASSERT_NE(window, nullptr);

    auto listeners = GetListenerList<IWindowStatusChangeListener, MockWindowStatusChangeListener>();
    EXPECT_NE(listeners.size(), 0);
    listeners.insert(listeners.begin(), nullptr);
    window->windowStatusChangeListeners_.insert({window->GetPersistentId(), listeners});

    WindowMode mode = WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
    window->state_ = WindowState::STATE_SHOWN;
    window->NotifyWindowStatusChange(mode);
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_SPLITSCREEN, window->lastWindowStatus_);

    window->NotifyWindowStatusChange(mode);
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_SPLITSCREEN, window->lastWindowStatus_);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: UpdatePiPRect
 * @tc.desc: UpdatePiPRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdatePiPRect, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UpdatePiPRect");
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_FROZEN;
    Rect rect;
    window->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RATIO_CHANGE);

    window->hostSession_ = nullptr;
    window->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RATIO_CHANGE);
    window->Destroy();
}

/**
 * @tc.name: NotifyTransformChange
 * @tc.desc: NotifyTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyTransformChange, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyTransformChange");
    ASSERT_NE(window, nullptr);

    Transform transform;
    window->NotifyTransformChange(transform);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->NotifyTransformChange(transform);
    window->Destroy();
}

/**
 * @tc.name: SubmitNoInteractionMonitorTask
 * @tc.desc: SubmitNoInteractionMonitorTask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SubmitNoInteractionMonitorTask, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SubmitNoInteractionMonitorTask");
    ASSERT_NE(window, nullptr);

    IWindowNoInteractionListenerSptr listener = sptr<MockWindowNoInteractionListener>::MakeSptr();
    window->SubmitNoInteractionMonitorTask(window->lastInteractionEventId_.load() + 1, listener);

    window->state_ = WindowState::STATE_SHOWN;
    window->SubmitNoInteractionMonitorTask(window->lastInteractionEventId_.load(), listener);

    window->state_ = WindowState::STATE_FROZEN;
    window->SubmitNoInteractionMonitorTask(window->lastInteractionEventId_.load(), listener);
    window->Destroy();
}

/**
 * @tc.name: RefreshNoInteractionTimeoutMonitor
 * @tc.desc: RefreshNoInteractionTimeoutMonitor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RefreshNoInteractionTimeoutMonitor, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RefreshNoInteractionTimeoutMonitor");
    ASSERT_NE(window, nullptr);
    window->RefreshNoInteractionTimeoutMonitor();
    ASSERT_TRUE(window->windowNoInteractionListeners_[window->GetPersistentId()].empty());
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    sptr<IWindowNoInteractionListener> listener = sptr<MockWindowNoInteractionListener>::MakeSptr();
    ASSERT_EQ(window->RegisterWindowNoInteractionListener(listener), WMError::WM_OK);
    window->RefreshNoInteractionTimeoutMonitor();
    ASSERT_EQ(window->GetPersistentId(), 1);
    ASSERT_FALSE(window->windowNoInteractionListeners_[window->GetPersistentId()].empty());
    window->Destroy();
}

/**
 * @tc.name: IsUserOrientation
 * @tc.desc: IsUserOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, IsUserOrientation, TestSize.Level1)
{
    auto window = GetTestWindowImpl("IsUserOrientation");
    ASSERT_NE(window, nullptr);

    ASSERT_FALSE(window->IsUserOrientation(Orientation::FOLLOW_DESKTOP));
    ASSERT_TRUE(window->IsUserOrientation(Orientation::USER_ROTATION_PORTRAIT));
    ASSERT_TRUE(window->IsUserOrientation(Orientation::USER_ROTATION_LANDSCAPE));
    ASSERT_TRUE(window->IsUserOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED));
    ASSERT_TRUE(window->IsUserOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED));
    window->Destroy();
}

/**
 * @tc.name: WindowSessionCreateCheck
 * @tc.desc: WindowSessionCreateCheck
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, WindowSessionCreateCheck, TestSize.Level1)
{
    auto window = GetTestWindowImpl("WindowSessionCreateCheck");
    ASSERT_NE(window, nullptr);

    int32_t nullWindowTestId = 1001;
    int32_t displayId = 1003;
    int32_t cameraId = 1004;

    window->windowSessionMap_.clear();
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    window->windowSessionMap_.insert(std::make_pair<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>>(
        "nullWindow", std::pair<int32_t, sptr<WindowSessionImpl>>(nullWindowTestId, nullptr)));

    auto displayWindow = GetTestWindowImpl("displayWindow");
    ASSERT_NE(displayWindow, nullptr);
    displayWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_FREEZE_DISPLAY);
    window->windowSessionMap_.insert(std::make_pair<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>>(
        "displayWindow", std::pair<int32_t, sptr<WindowSessionImpl>>(displayId, displayWindow)));
    ASSERT_EQ(window->WindowSessionCreateCheck(), WMError::WM_OK);

    window->windowSessionMap_.clear();
    auto cameraWindow = GetTestWindowImpl("cameraWindow");
    ASSERT_NE(cameraWindow, nullptr);
    cameraWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    window->windowSessionMap_.insert(std::make_pair<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>>(
        "cameraWindow", std::pair<int32_t, sptr<WindowSessionImpl>>(cameraId, cameraWindow)));
    ASSERT_EQ(window->WindowSessionCreateCheck(), WMError::WM_ERROR_REPEAT_OPERATION);
    window->Destroy();
    displayWindow->Destroy();
    cameraWindow->Destroy();
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyForegroundInteractiveStatus");
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    window->NotifyForegroundInteractiveStatus(true);
    window->NotifyForegroundInteractiveStatus(false);

    window->state_ = WindowState::STATE_DESTROYED;
    window->NotifyForegroundInteractiveStatus(true);
    window->state_ = WindowState::STATE_FROZEN;
    window->NotifyForegroundInteractiveStatus(true);
    window->Destroy();
}

/**
 * @tc.name: UpdateDecorEnableToAce
 * @tc.desc: UpdateDecorEnableToAce
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateDecorEnableToAce, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UpdateDecorEnableToAce");
    ASSERT_NE(window, nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    auto listeners = GetListenerList<IWindowChangeListener, MockWindowChangeListener>();
    sptr<MockWindowChangeListener> nullListener;
    listeners.insert(listeners.begin(), nullListener);
    window->windowChangeListeners_.insert({ window->GetPersistentId(), listeners });
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    window->UpdateDecorEnableToAce(false);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateDecorEnableToAce2");
    sptr<WindowSceneSessionImpl> sceneSseeionWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    sceneSseeionWindow->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    sceneSseeionWindow->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    ASSERT_NE(compatibleModeProperty, nullptr);
    compatibleModeProperty->SetDisableDecorFullscreen(true);
    sceneSseeionWindow->property_->SetCompatibleModeProperty(compatibleModeProperty);
    sceneSseeionWindow->UpdateDecorEnableToAce(false);

    window->uiContent_ = nullptr;
    window->UpdateDecorEnableToAce(false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: UpdateDecorEnableToAce02
 * @tc.desc: UpdateDecorEnableToAce02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateDecorEnableToAce02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImpl: UpdateDecorEnableToAce02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateDecorEnableToAce02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->windowMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));

    window->UpdateDecorEnableToAce(true);
    ASSERT_FALSE(window->IsDecorEnable());
}

/**
 * @tc.name: UpdateDecorEnable
 * @tc.desc: UpdateDecorEnable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateDecorEnable, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UpdateDecorEnable");
    ASSERT_NE(window, nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->UpdateDecorEnable(true, WindowMode::WINDOW_MODE_UNDEFINED);

    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    window->UpdateDecorEnable(true, WindowMode::WINDOW_MODE_FULLSCREEN);

    window->uiContent_ = nullptr;
    window->UpdateDecorEnable(true, WindowMode::WINDOW_MODE_FULLSCREEN);
    window->UpdateDecorEnable(false, WindowMode::WINDOW_MODE_FULLSCREEN);
    window->Destroy();
}

/**
 * @tc.name: NotifyModeChange
 * @tc.desc: NotifyModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyModeChange, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyModeChange");
    ASSERT_NE(window, nullptr);
    auto listeners = GetListenerList<IWindowChangeListener, MockWindowChangeListener>();
    sptr<MockWindowChangeListener> nullListener;
    listeners.insert(listeners.begin(), nullListener);
    window->windowChangeListeners_.insert({ window->GetPersistentId(), listeners });

    window->NotifyModeChange(WindowMode::WINDOW_MODE_FULLSCREEN, true);
    window->Destroy();
}

/**
 * @tc.name: GetContentInfo
 * @tc.desc: GetContentInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetContentInfo, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetContentInfo");
    ASSERT_NE(window, nullptr);

    ASSERT_EQ(window->GetContentInfo(BackupAndRestoreType::CONTINUATION), "");
    ASSERT_EQ(window->GetContentInfo(BackupAndRestoreType::APP_RECOVERY), "");
    ASSERT_EQ(window->GetContentInfo(BackupAndRestoreType::NONE), "");
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->GetContentInfo(BackupAndRestoreType::NONE);
    window->Destroy();
}

/**
 * @tc.name: SetDecorHeight
 * @tc.desc: SetDecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetDecorHeight, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SetDecorHeight");
    ASSERT_NE(window, nullptr);
    int32_t height = 50;
    EXPECT_EQ(window->SetDecorHeight(height), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(window->SetDecorHeight(height), WMError::WM_ERROR_NULLPTR);

    window->property_->SetDisplayId(0);
    EXPECT_EQ(window->SetDecorHeight(height), WMError::WM_ERROR_NULLPTR);
    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    window->uiContent_ = std::move(uiContent);
    EXPECT_EQ(window->SetDecorHeight(height), WMError::WM_OK);
    window->property_->SetDisplayId(-1);
    EXPECT_EQ(window->SetDecorHeight(height), WMError::WM_ERROR_NULLPTR);
    window->property_->SetDisplayId(0);
    EXPECT_EQ(window->SetDecorHeight(height), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: GetDecorHeight01
 * @tc.desc: GetDecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetDecorHeight01, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetDecorHeight01");
    ASSERT_NE(window, nullptr);
    int32_t height = -1;
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_ERROR_NULLPTR);

    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalTitleHeight()).WillRepeatedly(Return(-1));
    window->uiContent_ = std::move(uiContent);
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: GetDecorHeight02
 * @tc.desc: GetDecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetDecorHeight02, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetDecorHeight02");
    ASSERT_NE(window, nullptr);
    int32_t height = -1;
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_ERROR_NULLPTR);

    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalTitleHeight()).WillRepeatedly(Return(1));
    window->uiContent_ = std::move(uiContent);
    window->property_->SetDisplayId(-1);
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_ERROR_NULLPTR);
    window->property_->SetDisplayId(0);
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_OK);
    height = 1;
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: GetDecorHeight03
 * @tc.desc: GetDecorHeight version isolation test cases
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetDecorHeight03, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetDecorHeight03");
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    window->property_->SetDisplayId(0);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->useUniqueDensity_ = true;
    window->virtualPixelRatio_ = 1.9f;
    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalTitleHeight()).WillRepeatedly(Return(72));
    window->uiContent_ = std::move(uiContent);
    int32_t decorHeight = 38;
    window->SetDecorHeight(decorHeight);
    EXPECT_EQ(window->decorHeight_, decorHeight);

    window->SetTargetAPIVersion(14);
    int32_t height = -1;
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_OK);
    EXPECT_EQ(height, 37);
    window->SetTargetAPIVersion(18);
    EXPECT_EQ(window->GetDecorHeight(height), WMError::WM_OK);
    EXPECT_EQ(height, 37);
    window->Destroy();
}

/**
 * @tc.name: GetTitleButtonArea01
 * @tc.desc: GetTitleButtonArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetTitleButtonArea01, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetTitleButtonArea01");
    ASSERT_NE(window, nullptr);
    TitleButtonRect titleButtonRect;
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    window->uiContent_ = nullptr;
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_ERROR_NULLPTR);
    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalButtonsRect(testing::_, testing::_)).WillRepeatedly(Return(false));
    window->uiContent_ = std::move(uiContent);
    ASSERT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: GetTitleButtonArea02
 * @tc.desc: GetTitleButtonArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetTitleButtonArea02, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetTitleButtonArea02");
    ASSERT_NE(window, nullptr);
    TitleButtonRect titleButtonRect;
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->property_->SetDisplayId(0);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_ERROR_NULLPTR);
    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalButtonsRect(testing::_, testing::_)).WillRepeatedly(Return(true));
    window->uiContent_ = std::move(uiContent);
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_OK);
    window->property_->SetDisplayId(-1);
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_ERROR_NULLPTR);
    window->property_->SetDisplayId(0);
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: GetTitleButtonArea03
 * @tc.desc: GetTitleButtonArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetTitleButtonArea03, TestSize.Level1)
{
    auto window = GetTestWindowImpl("GetTitleButtonArea03");
    ASSERT_NE(window, nullptr);
    TitleButtonRect titleButtonRect = { 1400, 0, 0, 0 };
    window->property_->SetPersistentId(1);
    window->property_->SetDisplayId(0);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalButtonsRect(testing::_, testing::_)).WillRepeatedly(Return(false));
    window->uiContent_ = std::move(uiContent);
    window->SetTargetAPIVersion(14);
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_OK);
    EXPECT_EQ(titleButtonRect.IsUninitializedRect(), false);

    window->SetTargetAPIVersion(15);
    EXPECT_EQ(window->GetTitleButtonArea(titleButtonRect), WMError::WM_OK);
    EXPECT_EQ(titleButtonRect.IsUninitializedRect(), true);
    window->Destroy();
}

/**
 * @tc.name: RegisterWindowTitleButtonRectChangeListener
 * @tc.desc: RegisterWindowTitleButtonRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterWindowTitleButtonRectChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RegisterWindowTitleButtonRectChangeListener");
    ASSERT_NE(window, nullptr);
    sptr<IWindowTitleButtonRectChangedListener> listener = nullptr;
    auto ret = window->RegisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->RegisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    listener = sptr<MockWindowTitleButtonRectChangedListener>::MakeSptr();
    ret = window->RegisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    listener = sptr<MockWindowTitleButtonRectChangedListener>::MakeSptr();
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ret = window->RegisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: UnregisterWindowTitleButtonRectChangeListener
 * @tc.desc: UnregisterWindowTitleButtonRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UnregisterWindowTitleButtonRectChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("UnregisterWindowTitleButtonRectChangeListener");
    ASSERT_NE(window, nullptr);
    sptr<IWindowTitleButtonRectChangedListener> listener =  nullptr;
    auto ret = window->UnregisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    ret = window->UnregisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    listener = sptr<MockWindowTitleButtonRectChangedListener>::MakeSptr();
    ret = window->UnregisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ret = window->UnregisterWindowTitleButtonRectChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->Destroy();
}

/**
 * @tc.name: NotifyWindowTitleButtonRectChange
 * @tc.desc: NotifyWindowTitleButtonRectChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyWindowTitleButtonRectChange, TestSize.Level1)
{
    auto window = GetTestWindowImpl("NotifyWindowTitleButtonRectChange");
    ASSERT_NE(window, nullptr);
    auto listeners = GetListenerList<IWindowTitleButtonRectChangedListener, MockWindowTitleButtonRectChangedListener>();
    listeners.insert(listeners.begin(), nullptr);
    window->windowTitleButtonRectChangeListeners_.insert({ window->GetPersistentId(), listeners });
    TitleButtonRect titleButtonRect;
    window->NotifyWindowTitleButtonRectChange(titleButtonRect);
    window->Destroy();
}

/**
 * @tc.name: RegisterWindowRectChangeListener
 * @tc.desc: RegisterWindowRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterWindowRectChangeListener, TestSize.Level1)
{
    auto window = GetTestWindowImpl("RegisterWindowRectChangeListener");
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->RegisterWindowRectChangeListener(nullptr));

    sptr<IWindowRectChangeListener> listener = sptr<MockWindowRectChangeListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    ASSERT_EQ(WMError::WM_OK, window->RegisterWindowRectChangeListener(listener));

    window->hostSession_ = nullptr;
    ASSERT_EQ(WMError::WM_OK, window->RegisterWindowRectChangeListener(listener));
    window->Destroy();
}

/**
 * @tc.name: UnregisterWindowRectChangeListener
 * @tc.desc: UnregisterWindowRectChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UnregisterWindowRectChangeListener01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: UnregisterWindowRectChangeListener01 start";
    auto window = GetTestWindowImpl("UnregisterWindowRectChangeListener01");
    ASSERT_NE(window, nullptr);
    window->hostSession_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->RegisterWindowRectChangeListener(nullptr));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->UnregisterWindowRectChangeListener(nullptr));

    sptr<IWindowRectChangeListener> listener = sptr<MockWindowRectChangeListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    ASSERT_EQ(WMError::WM_OK, window->RegisterWindowRectChangeListener(listener));
    ASSERT_EQ(WMError::WM_OK, window->UnregisterWindowRectChangeListener(listener));

    SessionInfo sessionInfo = { "CreateTestBunble", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = hostSession;
    ASSERT_EQ(WMError::WM_OK, window->RegisterWindowRectChangeListener(listener));
    ASSERT_EQ(WMError::WM_OK, window->UnregisterWindowRectChangeListener(listener));
    window->Destroy();
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: UnregisterWindowRectChangeListener01 end";
}

/**
 * @tc.name: GetVirtualPixelRatio
 * @tc.desc: GetVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetVirtualPixelRatio, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetVirtualPixelRatio start";
    auto window = GetTestWindowImpl("GetVirtualPixelRatio");
    ASSERT_NE(nullptr, window);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    float density = 2.0f;
    displayInfo->SetVirtualPixelRatio(density);
    float vpr = window->GetVirtualPixelRatio(displayInfo);
    ASSERT_EQ(density, vpr);

    window->useUniqueDensity_ = true;
    vpr = window->GetVirtualPixelRatio(displayInfo);
    ASSERT_EQ(window->virtualPixelRatio_, vpr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetVirtualPixelRatio end";
}

/**
 * @tc.name: SetDragKeyFramePolicy
 * @tc.desc: SetDragKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetDragKeyFramePolicy, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SetDragKeyFramePolicy");
    ASSERT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = hostSession;
    KeyFramePolicy keyFramePolicy;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_FALSE(window->IsWindowSessionInvalid());

    // other
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    // phone
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    // subWindow
    window->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_ERROR_INVALID_CALLING);
    // mainWindow
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_OK);
    // pad
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_OK);
    // pc
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    // pass
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_OK);
    // invalid id
    window->property_->SetPersistentId(0);
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_ERROR_INVALID_WINDOW);
    // invalid session
    window->property_->SetPersistentId(1);
    window->hostSession_ = nullptr;
    EXPECT_EQ(window->SetDragKeyFramePolicy(keyFramePolicy), WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: NotifyScreenshot01 listener==nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyScreenshot01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyScreenshot01 start";
    class MockIScrenshotListener : public IScreenshotListener {
    public:
        MOCK_METHOD0(OnScreenshot, void());
    };
    window_ = GetTestWindowImpl("NotifyScreenshot01");
    auto listeners = GetListenerList<IScreenshotListener, MockIScrenshotListener>();
    listeners[0] = nullptr;
    ASSERT_EQ(listeners.size(), 1);
    window_->screenshotListeners_.insert({ window_->GetPersistentId(), listeners });
    window_->NotifyScreenshot();
    std::vector<sptr<IScreenshotListener>> screenshotListeners =
        window_->screenshotListeners_[window_->GetPersistentId()];
    ASSERT_NE(std::find(screenshotListeners.begin(), screenshotListeners.end(), nullptr), screenshotListeners.end());
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyScreenshot01 end";
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: NotifyScreenshot02 listener!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyScreenshot02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyScreenshot02 start";
    class ScreenshotListener : public IScreenshotListener {
    public:
        void OnScreenshot()
        {
            GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyScreenshot02 OnScreenshot";
            SUCCEED();
        }
    };
    window_ = GetTestWindowImpl("NotifyScreenshot02");
    sptr<IScreenshotListener> listener = new (std::nothrow) ScreenshotListener();
    window_->RegisterScreenshotListener(listener);
    window_->NotifyScreenshot();
    window_->UnregisterScreenshotListener(listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyScreenshot02 end";
}

/**
 * @tc.name: NotifyTouchDialogTarget
 * @tc.desc: NotifyTouchDialogTarget01 hostSession_==nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyTouchDialogTarget01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget01 start";
    window_ = GetTestWindowImpl("NotifyTouchDialogTarget01");
    int32_t posX = 100;
    int32_t posY = 100;
    window_->hostSession_ = nullptr;
    window_->NotifyTouchDialogTarget(posX, posY);
    sptr<ISession> hostSession = window_->GetHostSession();
    ASSERT_EQ(nullptr, hostSession);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget01 end";
}

/**
 * @tc.name: NotifyTouchDialogTarget
 * @tc.desc: NotifyTouchDialogTarget02 hostSession_!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyTouchDialogTarget02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget02 start";
    window_ = GetTestWindowImpl("NotifyTouchDialogTarget02");
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> hostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window_->hostSession_ = hostSession;
    int32_t posX = 100;
    int32_t posY = 100;
    window_->NotifyTouchDialogTarget(posX, posY);
    sptr<ISession> hostSession1 = window_->GetHostSession();
    ASSERT_NE(nullptr, hostSession1);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget02 end";
}

/**
 * @tc.name: NotifyTouchDialogTarget
 * @tc.desc: NotifyTouchDialogTarget03 hostSession_==nullptr listener==nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyTouchDialogTarget03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget03 start";
    class MockIDialogTargetTouchListener : public IDialogTargetTouchListener {
    public:
        MOCK_CONST_METHOD0(OnDialogTargetTouch, void());
    };
    window_ = GetTestWindowImpl("NotifyTouchDialogTarget03");
    window_->hostSession_ = nullptr;
    auto listeners = GetListenerList<IDialogTargetTouchListener, MockIDialogTargetTouchListener>();
    listeners[0] = nullptr;
    (window_->dialogTargetTouchListener_)[window_->GetPersistentId()] = listeners;
    int32_t posX = 100;
    int32_t posY = 100;
    window_->NotifyTouchDialogTarget(posX, posY);
    std::vector<sptr<IDialogTargetTouchListener>> dialogTargetTouchListeners =
        (window_->dialogTargetTouchListener_)[window_->GetPersistentId()];
    ASSERT_NE(std::find(dialogTargetTouchListeners.begin(), dialogTargetTouchListeners.end(), nullptr),
              dialogTargetTouchListeners.end());
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget03 end";
}

/**
 * @tc.name: NotifyTouchDialogTarget
 * @tc.desc: NotifyTouchDialogTarget04 hostSession_==nullptr listener!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyTouchDialogTarget04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget04 start";
    class MockIDialogTargetTouchListener : public IDialogTargetTouchListener {
    public:
        void OnDialogTargetTouch() const
        {
            GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget04 OnDialogTargetTouch";
            SUCCEED();
        }
    };
    window_ = GetTestWindowImpl("NotifyTouchDialogTarget04");
    window_->hostSession_ = nullptr;
    sptr<IDialogTargetTouchListener> dialogTargetTouchListener = new (std::nothrow) MockIDialogTargetTouchListener();
    window_->RegisterDialogTargetTouchListener(dialogTargetTouchListener);
    int32_t posX = 100;
    int32_t posY = 100;
    window_->NotifyTouchDialogTarget(posX, posY);
    window_->UnregisterDialogTargetTouchListener(dialogTargetTouchListener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyTouchDialogTarget04 end";
}

/**
 * @tc.name: NotifyDisplayMove
 * @tc.desc: NotifyDisplayMove01 listener==nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyDisplayMove01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDisplayMove01 start";
    class MockIDisplayMoveListener : public IDisplayMoveListener {
    public:
        MOCK_METHOD2(OnDisplayMove, void(DisplayId from, DisplayId to));
    };
    window_ = GetTestWindowImpl("NotifyDisplayMove01");
    auto listeners = GetListenerList<IDisplayMoveListener, MockIDisplayMoveListener>();
    listeners[0] = nullptr;
    (window_->displayMoveListeners_)[window_->GetPersistentId()] = listeners;
    int32_t posX = 100;
    int32_t posY = 100;
    window_->NotifyTouchDialogTarget(posX, posY);
    std::vector<sptr<IDisplayMoveListener>> displayMoveListeners =
        (window_->displayMoveListeners_)[window_->GetPersistentId()];
    ASSERT_NE(std::find(displayMoveListeners.begin(), displayMoveListeners.end(), nullptr), displayMoveListeners.end());
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDisplayMove01 end";
}

/**
 * @tc.name: NotifyDisplayMove
 * @tc.desc: NotifyDisplayMove02 listener!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyDisplayMove02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDisplayMove02 start";
    class MockIDisplayMoveListener : public IDisplayMoveListener {
    public:
        void OnDisplayMove(DisplayId from, DisplayId to)
        {
            GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDisplayMove02 OnDisplayMove";
            SUCCEED();
        }
    };
    window_ = GetTestWindowImpl("NotifyDisplayMove02");
    sptr<IDisplayMoveListener> displayMoveListener = sptr<MockIDisplayMoveListener>::MakeSptr();
    EXPECT_EQ(window_->RegisterDisplayMoveListener(displayMoveListener), WMError::WM_OK);
    int32_t posX = 100;
    int32_t posY = 100;
    window_->NotifyTouchDialogTarget(posX, posY);
    EXPECT_EQ(window_->UnregisterDisplayMoveListener(displayMoveListener), WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDisplayMove02 end";
}

/**
 * @tc.name: NotifyDestroy
 * @tc.desc: NotifyDestroy01 listener==nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyDestroy01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDestroy01 start";
    class MockIDialogDeathRecipientListener : public IDialogDeathRecipientListener {
    public:
        MOCK_CONST_METHOD0(OnDialogDeathRecipient, void());
    };
    window_ = GetTestWindowImpl("NotifyDestroy01");
    auto listeners = GetListenerList<IDialogDeathRecipientListener, MockIDialogDeathRecipientListener>();
    listeners[0] = nullptr;
    (window_->dialogDeathRecipientListeners_)[window_->GetPersistentId()] = listeners;
    window_->NotifyDestroy();
    std::vector<sptr<IDialogDeathRecipientListener>> dialogDeathRecipientListeners =
        (window_->dialogDeathRecipientListeners_)[window_->GetPersistentId()];
    ASSERT_NE(std::find(dialogDeathRecipientListeners.begin(), dialogDeathRecipientListeners.end(), nullptr),
              dialogDeathRecipientListeners.end());
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDestroy01 end";
}

/**
 * @tc.name: NotifyDestroy
 * @tc.desc: NotifyDestroy02 listener!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyDestroy02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDestroy02 start";
    class MockIDialogDeathRecipientListener : public IDialogDeathRecipientListener {
    public:
        void OnDialogDeathRecipient() const
        {
            GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDestroy02 OnDialogDeathRecipient";
            SUCCEED();
        }
    };
    window_ = GetTestWindowImpl("NotifyDestroy02");
    sptr<IDialogDeathRecipientListener> listener = new (std::nothrow) MockIDialogDeathRecipientListener();
    window_->RegisterDialogDeathRecipientListener(listener);
    window_->NotifyDestroy();
    window_->UnregisterDialogDeathRecipientListener(listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDestroy02 end";
}

/**
 * @tc.name: RegisterDialogTargetTouchListener
 * @tc.desc: RegisterDialogTargetTouchListener01 listener!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterDialogTargetTouchListener01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: RegisterDialogTargetTouchListener01 start";
    class MockIDialogTargetTouchListener : public IDialogTargetTouchListener {
    public:
        void OnDialogTargetTouch() const {}
    };
    window_ = GetTestWindowImpl("RegisterDialogTargetTouchListener01");
    sptr<IDialogTargetTouchListener> listener = sptr<MockIDialogTargetTouchListener>::MakeSptr();
    WMError res = window_->RegisterDialogTargetTouchListener(listener);
    ASSERT_EQ(WMError::WM_OK, res);
    window_->UnregisterDialogTargetTouchListener(listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: RegisterDialogTargetTouchListener01 end";
}

/**
 * @tc.name: RegisterDialogDeathRecipientListener
 * @tc.desc: RegisterDialogDeathRecipientListener01 listener!=nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterDialogDeathRecipientListener01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: RegisterDialogDeathRecipientListener01 start";
    class MockIDialogDeathRecipientListener : public IDialogDeathRecipientListener {
    public:
        void OnDialogDeathRecipient() const {}
    };
    window_ = GetTestWindowImpl("RegisterDialogDeathRecipientListener01");
    sptr<IDialogDeathRecipientListener> listener = sptr<MockIDialogDeathRecipientListener>::MakeSptr();
    int32_t count = (window_->dialogDeathRecipientListeners_)[window_->GetPersistentId()].size();
    window_->RegisterDialogDeathRecipientListener(listener);
    std::vector<sptr<IDialogDeathRecipientListener>> dialogDeathRecipientListeners =
        (window_->dialogDeathRecipientListeners_)[window_->GetPersistentId()];
    ASSERT_EQ(++count, dialogDeathRecipientListeners.size());
    window_->UnregisterDialogDeathRecipientListener(listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: RegisterDialogDeathRecipientListener01 end";
}

/**
 * @tc.name: RegisterSubWindowCloseListeners
 * @tc.desc: RegisterSubWindowCloseListeners01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterSubWindowCloseListeners01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: RegisterSubWindowCloseListeners01 start";
    class MockISubWindowCloseListener : public ISubWindowCloseListener {
    public:
        void OnSubWindowClose(bool& terminateCloseProcess) {}
    };
    window_ = GetTestWindowImpl("RegisterSubWindowCloseListeners01");
    ASSERT_NE(window_, nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->RegisterSubWindowCloseListeners(nullptr));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window_->UnregisterSubWindowCloseListeners(nullptr));

    sptr<ISubWindowCloseListener> listener = sptr<MockISubWindowCloseListener>::MakeSptr();
    window_->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window_->RegisterSubWindowCloseListeners(listener));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window_->UnregisterSubWindowCloseListeners(listener));

    window_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_OK, window_->RegisterSubWindowCloseListeners(listener));
    ASSERT_EQ(WMError::WM_OK, window_->UnregisterSubWindowCloseListeners(listener));
    window_->Destroy();
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: RegisterSubWindowCloseListeners01 end";
}

/**
 * @tc.name: GetListeners
 * @tc.desc: GetListeners01 IWindowLifeCycle
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetListeners01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetListeners01 start";
    window_ = GetTestWindowImpl("GetListeners01");
    ASSERT_NE(window_, nullptr);
    window_->lifecycleListeners_.clear();
    window_->NotifyWindowAfterFocused();
    ASSERT_TRUE(window_->lifecycleListeners_[window_->GetPersistentId()].empty());
    sptr<IWindowLifeCycle> listener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    window_->RegisterLifeCycleListener(listener);
    window_->NotifyWindowAfterFocused();
    ASSERT_FALSE(window_->lifecycleListeners_[window_->GetPersistentId()].empty());
    window_->Destroy();
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetListeners01 end";
}

/**
 * @tc.name: GetListeners
 * @tc.desc: GetListeners02 IOccupiedAreaChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetListeners02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetListeners02 start";
    window_ = GetTestWindowImpl("GetListeners02");
    ASSERT_NE(window_, nullptr);
    window_->occupiedAreaChangeListeners_.clear();
    sptr<OccupiedAreaChangeInfo> occupiedAreaChangeInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    window_->NotifyOccupiedAreaChangeInfo(occupiedAreaChangeInfo, nullptr, {}, {});
    ASSERT_TRUE(window_->occupiedAreaChangeListeners_[window_->GetPersistentId()].empty());
    sptr<IOccupiedAreaChangeListener> listener = sptr<MockIOccupiedAreaChangeListener>::MakeSptr();
    window_->RegisterOccupiedAreaChangeListener(listener);
    window_->NotifyOccupiedAreaChangeInfo(occupiedAreaChangeInfo, nullptr, {}, {});
    ASSERT_FALSE(window_->occupiedAreaChangeListeners_[window_->GetPersistentId()].empty());
    window_->Destroy();
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetListeners02 end";
}

/**
 * @tc.name: GetListeners
 * @tc.desc: GetListeners03 IKeyboardDidShowListener IKeyboardDidHideListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetListeners03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetListeners03 start";
    window_ = GetTestWindowImpl("GetListeners03");
    ASSERT_NE(window_, nullptr);
    KeyboardPanelInfo keyboardPanelInfo;
    window_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    keyboardPanelInfo.isShowing_ = true;
    window_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    ASSERT_TRUE(window_->keyboardDidShowListeners_[window_->GetPersistentId()].empty());
    ASSERT_TRUE(window_->keyboardDidHideListeners_[window_->GetPersistentId()].empty());
    sptr<IKeyboardDidShowListener> listener = sptr<MockIKeyboardDidShowListener>::MakeSptr();
    window_->RegisterKeyboardDidShowListener(listener);
    keyboardPanelInfo.isShowing_ = true;
    window_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    ASSERT_FALSE(window_->keyboardDidShowListeners_[window_->GetPersistentId()].empty());
    window_->UnregisterKeyboardDidShowListener(listener);

    sptr<IKeyboardDidHideListener> listener1 = sptr<MockIKeyboardDidHideListener>::MakeSptr();
    window_->RegisterKeyboardDidHideListener(listener1);
    keyboardPanelInfo.isShowing_ = false;
    window_->uiContent_ = nullptr;
    window_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    window_->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    ASSERT_FALSE(window_->keyboardDidHideListeners_[window_->GetPersistentId()].empty());
    window_->UnregisterKeyboardDidHideListener(listener1);
    window_->Destroy();
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetListeners03 end";
}

/**
 * @tc.name: GetUIContent
 * @tc.desc: GetUIContent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetUIContent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetUIContent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Ace::UIContent* res = window->GetUIContent();
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(window->Destroy(), WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: NotifySizeChange
 * @tc.desc: NotifySizeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifySizeChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySizeChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    Rect rect;
    sptr<IWindowChangeListener> listener = sptr<MockWindowChangeListener>::MakeSptr();
    window->RegisterWindowChangeListener(listener);
    window->NotifySizeChange(rect, WindowSizeChangeReason::PIP_RATIO_CHANGE);

    sptr<IWindowRectChangeListener> listener1 = sptr<MockWindowRectChangeListener>::MakeSptr();
    window->RegisterWindowRectChangeListener(listener1);
    window->NotifySizeChange(rect, WindowSizeChangeReason::PIP_RATIO_CHANGE);
    window->Destroy(true);
}

/**
 * @tc.name: NotifyUIExtHostWindowRectChangeListeners
 * @tc.desc: NotifyUIExtHostWindowRectChangeListeners test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyUIExtHostWindowRectChangeListeners, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyUIExtHostWindowRectChangeListeners");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_NE(0, window->GetPersistentId());
    window->uiContent_ = nullptr;

    Rect rect;
    WindowSizeChangeReason reason = WindowSizeChangeReason::MOVE;
    window->NotifyUIExtHostWindowRectChangeListeners(rect, reason);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_TRUE(window->rectChangeUIExtListenerIds_.empty());
    window->NotifyUIExtHostWindowRectChangeListeners(rect, reason);
    window->rectChangeUIExtListenerIds_.emplace(111);
    window->NotifyUIExtHostWindowRectChangeListeners(rect, reason);
}

/**
 * @tc.name: AvoidAreaChangeListener
 * @tc.desc: AvoidAreaChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, AvoidAreaChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AvoidAreaChangeListener");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    sptr<IAvoidAreaChangedListener> nullListener = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->UnregisterAvoidAreaChangeListener(nullListener));

    sptr<IAvoidAreaChangedListener> listener = sptr<MockAvoidAreaChangedListener>::MakeSptr();
    window->UnregisterAvoidAreaChangeListener(listener);

    window->RegisterAvoidAreaChangeListener(nullListener);
    window->RegisterAvoidAreaChangeListener(listener);

    sptr<IAvoidAreaChangedListener> listener1 = sptr<MockAvoidAreaChangedListener>::MakeSptr();
    window->RegisterAvoidAreaChangeListener(listener1);

    window->UnregisterAvoidAreaChangeListener(listener);
    window->UnregisterAvoidAreaChangeListener(listener1);
}

/**
 * @tc.name: RegisterScreenshotAppEventListener
 * @tc.desc: RegisterScreenshotAppEventListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterScreenshotAppEventListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterScreenshotAppEventListener");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IScreenshotAppEventListener> listener = nullptr;
    WMError ret = window->RegisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ret = window->RegisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IScreenshotAppEventListener>::MakeSptr();
    std::vector<sptr<IScreenshotAppEventListener>> holder;
    window->screenshotAppEventListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->RegisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    holder = window->screenshotAppEventListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    EXPECT_NE(existsListener, holder.end());

    ret = window->RegisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    holder = window->screenshotAppEventListeners_[window->property_->GetPersistentId()];
    EXPECT_EQ(holder.size(), 1);
}

/**
 * @tc.name: unregisterScreenshotAppEventListener
 * @tc.desc: unregisterScreenshotAppEventListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, unregisterScreenshotAppEventListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("unregisterScreenshotAppEventListener");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IScreenshotAppEventListener> listener = nullptr;
    WMError ret = window->UnregisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ret = window->RegisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IScreenshotAppEventListener>::MakeSptr();
    std::vector<sptr<IScreenshotAppEventListener>> holder;
    window->screenshotAppEventListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->UnregisterScreenshotAppEventListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);

    holder = window->screenshotAppEventListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    EXPECT_EQ(existsListener, holder.end());
}

/**
 * @tc.name: RegisterAcrossDisplaysChangeListener
 * @tc.desc: RegisterAcrossDisplaysChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, RegisterAcrossDisplaysChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterAcrossDisplaysChangeListener");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    sptr<IAcrossDisplaysChangeListener> listener = nullptr;
    auto ret = window->RegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ret = window->RegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IAcrossDisplaysChangeListener>::MakeSptr();
    std::vector<sptr<IAcrossDisplaysChangeListener>> holder;
    window->acrossDisplaysChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->RegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    holder = window->acrossDisplaysChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    EXPECT_NE(existsListener, holder.end());

    ret = window->RegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    holder = window->acrossDisplaysChangeListeners_[window->property_->GetPersistentId()];
    EXPECT_EQ(holder.size(), 1);
}

/**
 * @tc.name: UnRegisterAcrossDisplaysChangeListener
 * @tc.desc: UnRegisterAcrossDisplaysChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UnRegisterAcrossDisplaysChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnRegisterAcrossDisplaysChangeListener01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    sptr<IAcrossDisplaysChangeListener> listener = nullptr;
    auto ret = window->UnRegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ret = window->UnRegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IAcrossDisplaysChangeListener>::MakeSptr();
    std::vector<sptr<IAcrossDisplaysChangeListener>> holder;
    window->acrossDisplaysChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->UnRegisterAcrossDisplaysChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);

    holder = window->acrossDisplaysChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    EXPECT_EQ(existsListener, holder.end());
}

/**
 * @tc.name: NotifyScreenshotAppEvent
 * @tc.desc: NotifyScreenshotAppEvent IScreenshotAppEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyScreenshotAppEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("unregisterScreenshotAppEventListener");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->screenshotAppEventListeners_.clear();
    EXPECT_EQ(0, window->screenshotAppEventListeners_.size());

    sptr<IScreenshotAppEventListener> listeners = sptr<IScreenshotAppEventListener>::MakeSptr();
    std::vector<sptr<IScreenshotAppEventListener>> holder;
    holder.push_back(listeners);
    window->screenshotAppEventListeners_[window->property_->GetPersistentId()] = holder;
    EXPECT_EQ(1, window->screenshotAppEventListeners_.size());
    auto ret = window->NotifyScreenshotAppEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: TouchOutsideListener
 * @tc.desc: TouchOutsideListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, TouchOutsideListener, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("TouchOutsideListener");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    sptr<ITouchOutsideListener> nullListener = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->UnregisterTouchOutsideListener(nullListener));

    sptr<ITouchOutsideListener> listener = new (std::nothrow) MockTouchOutsideListener();
    ASSERT_NE(nullptr, listener);
    window->UnregisterTouchOutsideListener(listener);

    window->RegisterTouchOutsideListener(nullListener);
    window->RegisterTouchOutsideListener(listener);

    sptr<ITouchOutsideListener> listener1 = new (std::nothrow) MockTouchOutsideListener();
    ASSERT_NE(nullptr, listener1);
    window->RegisterTouchOutsideListener(listener1);

    window->UnregisterTouchOutsideListener(listener);
    window->UnregisterTouchOutsideListener(listener1);
}

/**
 * @tc.name: NotifyDialogStateChange
 * @tc.desc: NotifyDialogStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifyDialogStateChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDialogStateChange start";
    window_ = GetTestWindowImpl("NotifyDialogStateChange");
    ASSERT_NE(window_, nullptr);
    ASSERT_EQ(window_->NotifyDialogStateChange(true), WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifyDialogStateChange end";
}

/**
 * @tc.name: SwitchFreeMultiWindow
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SwitchFreeMultiWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: SwitchFreeMultiWindow start";
    window_ = GetTestWindowImpl("SwitchFreeMultiWindow");
    ASSERT_NE(window_, nullptr);
    ASSERT_EQ(window_->SwitchFreeMultiWindow(true), WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: SwitchFreeMultiWindow end";
}

/**
 * @tc.name: UpdateTitleInTargetPos
 * @tc.desc: UpdateTitleInTargetPos
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateTitleInTargetPos, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: UpdateTitleInTargetPos start";
    window_ = GetTestWindowImpl("UpdateTitleInTargetPos");
    ASSERT_NE(window_, nullptr);
    ASSERT_EQ(window_->UpdateTitleInTargetPos(true, 100), WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: UpdateTitleInTargetPos end";
}

/**
 * @tc.name: NotifySessionBackground
 * @tc.desc: NotifySessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, NotifySessionBackground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifySessionBackground start";
    window_ = GetTestWindowImpl("NotifySessionBackground");
    ASSERT_NE(window_, nullptr);
    window_->NotifySessionBackground(true, true, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: NotifySessionBackground end";
}

/**
 * @tc.name: UpdateMaximizeMode
 * @tc.desc: UpdateMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, UpdateMaximizeMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: UpdateMaximizeMode start";
    window_ = GetTestWindowImpl("UpdateMaximizeMode");
    ASSERT_NE(window_, nullptr);
    ASSERT_EQ(window_->UpdateMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR), WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: UpdateMaximizeMode end";
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: DumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, DumpSessionElementInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: DumpSessionElementInfo start";
    window_ = GetTestWindowImpl("DumpSessionElementInfo");
    ASSERT_NE(window_, nullptr);
    std::vector<std::string> params;
    params.push_back("test");
    window_->DumpSessionElementInfo(params);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: DumpSessionElementInfo end";
}

/**
 * @tc.name: GetKeyboardAnimationConfig
 * @tc.desc: GetKeyboardAnimationConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetKeyboardAnimationConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetKeyboardAnimationConfig start";
    window_ = GetTestWindowImpl("GetKeyboardAnimationConfig");
    ASSERT_NE(window_, nullptr);
    window_->GetKeyboardAnimationConfig();
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetKeyboardAnimationConfig end";
}

/**
 * @tc.name: GetSubWindow
 * @tc.desc: GetSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, GetSubWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetSubWindow start";
    window_ = GetTestWindowImpl("GetSubWindow");
    ASSERT_NE(window_, nullptr);
    ASSERT_TRUE(window_->subWindowSessionMap_.empty());
    std::vector<sptr<WindowSessionImpl>> vec;
    vec.push_back(window_);
    int32_t parentId = 111;
    window_->subWindowSessionMap_.insert(std::pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(parentId, vec));
    std::vector<sptr<Window>> subWindows = window_->GetSubWindow(parentId);
    ASSERT_EQ(subWindows.size(), 1);
    GTEST_LOG_(INFO) << "WindowSessionImplTest2: GetSubWindow end";
}

/**
 * @tc.name: SetRestoredRouterStack_0200
 * @tc.desc: basic function test of set or get restored router stack.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, SetRestoredRouterStack_0200, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    std::string routerStack = "stackInfo:{}";
    EXPECT_EQ(window->SetRestoredRouterStack(routerStack), WMError::WM_OK);
    std::string gettedStack = window->GetRestoredRouterStack();
    EXPECT_EQ(gettedStack, routerStack);
    EXPECT_TRUE(window->GetRestoredRouterStack().empty());
}

/**
 * @tc.name: SetUiDvsyncSwitch
 * @tc.desc: SetUiDvsyncSwitch
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetUiDvsyncSwitch, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetUiDvsyncSwitch");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetUiDvsyncSwitch(true);
    window->vsyncStation_ = nullptr;
    window->SetUiDvsyncSwitch(true);
}

/**
 * @tc.name: SetUiDvsyncSwitchSucc
 * @tc.desc: SetUiDvsyncSwitch Test Succ
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetUiDvsyncSwitchSucc, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowName("SetUiDvsyncSwitchSucc");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetUiDvsyncSwitch(true);
    window->SetUiDvsyncSwitch(false);
}

/**
 * @tc.name: SetUiDvsyncSwitchErr
 * @tc.desc: SetUiDvsyncSwitch Test Err
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetUiDvsyncSwitchErr, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowName("SetUiDvsyncSwitchErr");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->vsyncStation_ = nullptr;
    window->SetUiDvsyncSwitch(true);
    window->SetUiDvsyncSwitch(false);
}

/**
 * @tc.name: SetTouchEvent
 * @tc.desc: SetTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetTouchEvent, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetTouchEvent");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetTouchEvent(0);
    window->vsyncStation_ = nullptr;
    window->SetTouchEvent(1);
}

/**
 * @tc.name: SetTouchEventSucc
 * @tc.desc: SetTouchEvent Test Succ
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetTouchEventSucc, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowName("SetTouchEventSucc");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetTouchEvent(0);
    window->SetTouchEvent(1);
}

/**
 * @tc.name: SetTouchEventErr
 * @tc.desc: SetTouchEvent Test Err
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest2, SetTouchEventErr, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowName("SetTouchEventErr");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->vsyncStation_ = nullptr;
    window->SetTouchEvent(0);
    window->SetTouchEvent(1);
}
/**
 * @tc.name: SetRestoredRouterStack_0100
 * @tc.desc: basic function test of set or get restored router stack.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, SetRestoredRouterStack_0100, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    std::string routerStack = "stackInfo:{}";
    EXPECT_EQ(window->SetRestoredRouterStack(routerStack), WMError::WM_OK);
    EXPECT_EQ(window->NapiSetUIContent("info", (napi_env)nullptr, (napi_value)nullptr, BackupAndRestoreType::NONE,
        nullptr, nullptr), WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: RegisterKeyboardWillShowListener
 * @tc.desc: RegisterKeyboardWillShowListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, RegisterKeyboardWillShowListener, TestSize.Level1)
{
    window_ = GetTestWindowImpl("RegisterKeyboardWillShowListener");
    sptr<IKeyboardWillShowListener> listener = sptr<MockIKeyboardWillShowListener>::MakeSptr();
    auto status = window_->RegisterKeyboardWillShowListener(listener);
    EXPECT_EQ(status, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window_->windowSystemConfig_.supportFunctionType_ = SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION;
    status = window_->RegisterKeyboardWillShowListener(listener);
    EXPECT_EQ(status, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterKeyboardWillShowListener
 * @tc.desc: UnregisterKeyboardWillShowListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, UnregisterKeyboardWillShowListener, TestSize.Level1)
{
    window_ = GetTestWindowImpl("RegisterKeyboardWillShowListener");
    sptr<IKeyboardWillShowListener> listener = sptr<MockIKeyboardWillShowListener>::MakeSptr();

    window_->windowSystemConfig_.supportFunctionType_ = SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION;
    auto status = window_->RegisterKeyboardWillShowListener(listener);
    EXPECT_EQ(status, WMError::WM_OK);

    EXPECT_EQ(window_->UnregisterKeyboardWillShowListener(listener), WMError::WM_OK);
}

/**
 * @tc.name: RegisterKeyboardWillHideListener
 * @tc.desc: RegisterKeyboardWillHideListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, RegisterKeyboardWillHideListener, TestSize.Level1)
{
    window_ = GetTestWindowImpl("RegisterKeyboardWillHideListener");
    sptr<IKeyboardWillHideListener> listener = sptr<MockIKeyboardWillHideListener>::MakeSptr();
    auto status = window_->RegisterKeyboardWillHideListener(listener);
    EXPECT_EQ(status, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
 
    window_->windowSystemConfig_.supportFunctionType_ = SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION;
    status = window_->RegisterKeyboardWillHideListener(listener);
    EXPECT_EQ(status, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterKeyboardWillHideListener
 * @tc.desc: UnregisterKeyboardWillHideListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, UnregisterKeyboardWillHideListener, TestSize.Level1)
{
    window_ = GetTestWindowImpl("RegisterKeyboardWillHideListener");
    sptr<IKeyboardWillHideListener> listener = sptr<MockIKeyboardWillHideListener>::MakeSptr();
    window_->windowSystemConfig_.supportFunctionType_ = SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION;
    auto status = window_->RegisterKeyboardWillHideListener(listener);
    EXPECT_EQ(status, WMError::WM_OK);
 
    EXPECT_EQ(window_->UnregisterKeyboardWillHideListener(listener), WMError::WM_OK);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: NotifyKeyboardAnimationWillBegin
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(WindowSessionImplTest2, NotifyKeyboardAnimationWillBegin, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window_ = GetTestWindowImpl("RegisterKeyboardWillHideListener");
    window_->windowSystemConfig_.supportFunctionType_ = SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION;
    sptr<IKeyboardWillShowListener> listener = sptr<MockIKeyboardWillShowListener>::MakeSptr();
    window_->RegisterKeyboardWillShowListener(listener);
    sptr<IKeyboardWillHideListener> listener1 = sptr<MockIKeyboardWillHideListener>::MakeSptr();
    window_->RegisterKeyboardWillHideListener(listener1);
    
    KeyboardAnimationInfo animationInfo;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();
    window_->NotifyKeyboardAnimationWillBegin(animationInfo, nullptr);
    animationInfo.isShow = true;
    window_->NotifyKeyboardAnimationWillBegin(animationInfo, rsTransaction);
    animationInfo.isShow = false;
    window_->NotifyKeyboardAnimationWillBegin(animationInfo, rsTransaction);

    EXPECT_TRUE(logMsg.find("handler is nullptr") == std::string::npos);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
