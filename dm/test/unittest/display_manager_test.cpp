/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#include "display_manager.h"
#include "display_manager_proxy.h"
#include "window.h"
#include "dm_common.h"

#include "mock_display_manager_adapter.h"
#include "singleton_mocker.h"
#include "display_manager.cpp"
#include "window_scene.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<DisplayManagerAdapter, MockDisplayManagerAdapter>;
class DmMockScreenshotListener : public DisplayManager::IScreenshotListener {
public:
    void OnScreenshot(const ScreenshotInfo info) override {}
};
class DmMockDisplayListener : public DisplayManager::IDisplayListener {
public:
    void OnCreate(DisplayId) override {}
    void OnDestroy(DisplayId) override {}
    void OnChange(DisplayId) override {}
};
class DisplayManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void DisplayManagerTest::SetUpTestCase()
{
}

void DisplayManagerTest::TearDownTestCase()
{
}

void DisplayManagerTest::SetUp()
{
}

void DisplayManagerTest::TearDown()
{
}

namespace {
/**
 * @tc.name: Freeze01
 * @tc.desc: success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Freeze01, Function | SmallTest | Level1)
{
    std::vector<DisplayId> displayIds;
    displayIds.push_back(0);
    bool ret = DisplayManager::GetInstance().Freeze(displayIds);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.name: Freeze02
 * @tc.desc: test Freeze displayIds exceed the maximum
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Freeze02, Function | SmallTest | Level1)
{
    std::vector<DisplayId> displayIds;
    for (uint32_t i = 0; i < 33; i++) { // MAX_DISPLAY_SIZE + 1
        displayIds.push_back(i);
    }
    bool ret = DisplayManager::GetInstance().Freeze(displayIds);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.name: Freeze03
 * @tc.desc: test Freeze displayIds empty
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Freeze03, Function | SmallTest | Level1)
{
    std::vector<DisplayId> displayIds;
    bool ret = DisplayManager::GetInstance().Freeze(displayIds);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.name: Unfreeze01
 * @tc.desc: success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Unfreeze01, Function | SmallTest | Level1)
{
    std::vector<DisplayId> displayIds;
    displayIds.push_back(0);
    bool ret = DisplayManager::GetInstance().Unfreeze(displayIds);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.name: Unfreeze02
 * @tc.desc: test Freeze displayIds exceed the maximum
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Unfreeze02, Function | SmallTest | Level1)
{
    std::vector<DisplayId> displayIds;
    for (uint32_t i = 0; i < 33; i++) { // MAX_DISPLAY_SIZE + 1
        displayIds.push_back(i);
    }

    bool ret = DisplayManager::GetInstance().Unfreeze(displayIds);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: Unfreeze03
 * @tc.desc: test Freeze displayIds empty
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Unfreeze03, Function | SmallTest | Level1)
{
    std::vector<DisplayId> displayIds;
    bool ret = DisplayManager::GetInstance().Unfreeze(displayIds);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: RegisterScreenshotListener01
 * @tc.desc: test RegisterScreenshotListener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterScreenshotListener01, Function | SmallTest | Level1)
{
    DMError ret = DisplayManager::GetInstance().RegisterScreenshotListener(nullptr);
    ASSERT_FALSE(DMError::DM_OK == ret);
}

/**
 * @tc.name: RegisterScreenshotListener02
 * @tc.desc: test RegisterScreenshotListener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterScreenshotListener02, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_NULLPTR));
    sptr<DisplayManager::IScreenshotListener> listener = new DmMockScreenshotListener();
    DMError ret = DisplayManager::GetInstance().RegisterScreenshotListener(listener);
    ASSERT_FALSE(DMError::DM_OK == ret);
}

/**
 * @tc.name: UnregisterScreenshotListener01
 * @tc.desc: test UnregisterScreenshotListener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterScreenshotListener01, Function | SmallTest | Level1)
{
    DMError ret = DisplayManager::GetInstance().UnregisterScreenshotListener(nullptr);
    ASSERT_FALSE(DMError::DM_OK == ret);
}

/**
 * @tc.name: OnDisplayCreate01
 * @tc.desc: OnDisplayCreate
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, OnDisplayCreate01, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    DisplayManager::GetInstance().RegisterDisplayListener(listener);
    auto displayManagerListener = DisplayManager::GetInstance().pImpl_->displayManagerListener_;
    ASSERT_NE(displayManagerListener, nullptr);
    displayManagerListener->OnDisplayCreate(nullptr);
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    displayInfo->SetDisplayId(DISPLAY_ID_INVALID);
    displayManagerListener->OnDisplayCreate(displayInfo);
    displayInfo->SetDisplayId(0);
    displayManagerListener->OnDisplayCreate(displayInfo);
    ASSERT_NE(displayManagerListener->pImpl_, nullptr);
    displayManagerListener->pImpl_ = nullptr;
    displayManagerListener->OnDisplayCreate(displayInfo);
    DisplayManager::GetInstance().pImpl_->displayManagerListener_ = nullptr;
}

/**
 * @tc.name: CheckRectValid
 * @tc.desc: CheckRectValid all
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, CheckRectValid, Function | SmallTest | Level1)
{
    int32_t oriHeight = 500;
    int32_t oriWidth = 500;
    Media::Rect rect = {.left = 1, .top = 1, .width = 1, .height = 1};
    bool ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_TRUE(ret);
    rect.left = -1;
    ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    rect.left = 1;
    rect.top = -1;
    ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    rect.top = 1;
    rect.width = -1;
    ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    rect.width = 1;
    rect.height = -1;
    ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    rect.width = 500;
    rect.height = 1;
    ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    rect.width = 1;
    rect.height = 500;
    ret = DisplayManager::GetInstance().pImpl_->CheckRectValid(rect, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: CheckSizeValid
 * @tc.desc: CheckSizeValid all
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, CheckSizeValid, Function | SmallTest | Level1)
{
    int32_t oriHeight = 500;
    int32_t oriWidth = 500;
    Media::Size size = {.width = 1, .height = 1};
    bool ret = DisplayManager::GetInstance().pImpl_->CheckSizeValid(size, oriHeight, oriWidth);
    ASSERT_TRUE(ret);
    size.width = -1;
    ret = DisplayManager::GetInstance().pImpl_->CheckSizeValid(size, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    size.width = 1;
    size.height = -1;
    ret = DisplayManager::GetInstance().pImpl_->CheckSizeValid(size, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    size.width = DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT + 1;
    size.height = 1;
    ret = DisplayManager::GetInstance().pImpl_->CheckSizeValid(size, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
    size.width = DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT;
    size.height = DisplayManager::MAX_RESOLUTION_SIZE_SCREENSHOT + 1;
    ret = DisplayManager::GetInstance().pImpl_->CheckSizeValid(size, oriHeight, oriWidth);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: ImplGetDefaultDisplay01
 * @tc.desc: Impl GetDefaultDisplay nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplGetDefaultDisplay01, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDefaultDisplayInfo()).Times(1).WillOnce(Return(nullptr));
    sptr<Display> display = DisplayManager::GetInstance().pImpl_->GetDefaultDisplay();
    ASSERT_EQ(display, nullptr);
}

/**
 * @tc.name: GetDisplayByScreen
 * @tc.desc: for interface coverage & check GetDisplayByScreen
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayByScreen, Function | SmallTest | Level1)
{
    auto& displayManager = DisplayManager::GetInstance();
    sptr<Display> display = displayManager.GetDisplayByScreen(SCREEN_ID_INVALID);
    ASSERT_EQ(display, nullptr);

    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    displayInfo->SetDisplayId(DISPLAY_ID_INVALID);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDisplayInfoByScreenId(_)).Times(1).WillOnce(Return(displayInfo));
    display = displayManager.GetDisplayByScreen(1);
    ASSERT_EQ(display, nullptr);
}

/**
 * @tc.name: ImplGetDefaultDisplaySync
 * @tc.desc: Impl GetDefaultDisplaySync nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplGetDefaultDisplaySync, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDefaultDisplayInfo()).Times(6).WillRepeatedly(Return(nullptr));
    sptr<Display> display = DisplayManager::GetInstance().GetDefaultDisplaySync();
    ASSERT_EQ(display, nullptr);
}

/**
 * @tc.name: GetScreenBrightness
 * @tc.desc: GetScreenBrightness fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenBrightness, Function | SmallTest | Level1)
{
    uint64_t screenId = 2;
    auto ret = DisplayManager::GetInstance().GetScreenBrightness(screenId);
    ASSERT_FALSE(ret == 1);
}

/**
 * @tc.name: GetDisplayById
 * @tc.desc: GetDisplayById fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayById, Function | SmallTest | Level1)
{
    DisplayId displayId = -1;
    g_dmIsDestroyed = true;
    auto ret = DisplayManager::GetInstance().GetDisplayById(displayId);
    ASSERT_EQ(ret, nullptr);
}

/**
 * @tc.name: RegisterPrivateWindowListener
 * @tc.desc: RegisterPrivateWindowListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterPrivateWindowListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IPrivateWindowListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterPrivateWindowListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterPrivateWindowListener
 * @tc.desc: UnregisterPrivateWindowListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterPrivateWindowListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IPrivateWindowListener> listener = nullptr;
    sptr<DisplayManager::Impl> impl_;
    auto ret = DisplayManager::GetInstance().UnregisterPrivateWindowListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: ImplUnregisterPrivateWindowListener
 * @tc.desc: ImplUnregisterPrivateWindowListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterPrivateWindowListener, Function | SmallTest | Level1)
{
    std::recursive_mutex mutex;
    DisplayManager::Impl impl(mutex);
    sptr<DisplayManager::IPrivateWindowListener> listener;
    auto ret = impl.UnregisterPrivateWindowListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: RegisterPrivateWindowListChangeListener
 * @tc.desc: RegisterPrivateWindowListChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterPrivateWindowListChangeListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IPrivateWindowListChangeListener();
    ret = DisplayManager::GetInstance().RegisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterPrivateWindowListChangeListener(listener));
    listener.clear();
}

/**
 * @tc.name: UnregisterPrivateWindowListChangeListener
 * @tc.desc: UnregisterPrivateWindowListChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterPrivateWindowListChangeListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener = nullptr;
    auto ret = DisplayManager::GetInstance().UnregisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IPrivateWindowListChangeListener();
    ret = DisplayManager::GetInstance().UnregisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterPrivateWindowListChangeListener(listener));
    listener.clear();
}

/**
 * @tc.name: ImplRegisterPrivateWindowListChangeListener
 * @tc.desc: ImplRegisterPrivateWindowListChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplRegisterPrivateWindowListChangeListener, Function | SmallTest | Level1)
{
    std::recursive_mutex mutex;
    sptr<DisplayManager::Impl> impl_;
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener;
    DisplayManager::GetInstance().pImpl_->privateWindowListChangeListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl::DisplayManagerPrivateWindowListAgent> privateWindowListChangeListenerAgent =
        new DisplayManager::Impl::DisplayManagerPrivateWindowListAgent(impl_);
    auto ret = DisplayManager::GetInstance().pImpl_->RegisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            privateWindowListChangeListenerAgent,
            DisplayManagerAgentType::PRIVATE_WINDOW_LIST_LISTENER));
    listener = nullptr;
    privateWindowListChangeListenerAgent.clear();
}

/**
 * @tc.name: ImplUnregisterPrivateWindowListChangeListener
 * @tc.desc: ImplUnregisterPrivateWindowListChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterPrivateWindowListChangeListener, Function | SmallTest | Level1)
{
    std::recursive_mutex mutex;
    DisplayManager::Impl impl(mutex);
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener;
    auto ret = impl.UnregisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: ImplUnregisterFoldStatusListener
 * @tc.desc: ImplUnregisterFoldStatusListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterFoldStatusListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IFoldStatusListener> listener;
    auto ret = DisplayManager::GetInstance().pImpl_->UnregisterFoldStatusListener(listener);
    ASSERT_NE(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: RegisterFoldStatusListener
 * @tc.desc: RegisterFoldStatusListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterFoldStatusListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IFoldStatusListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterFoldStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IFoldStatusListener();
    ret = DisplayManager::GetInstance().RegisterFoldStatusListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterFoldStatusListener(listener));
    listener.clear();
}

/**
 * @tc.name: ImplRegisterFoldStatusListener
 * @tc.desc: ImplRegisterFoldStatusListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplRegisterFoldStatusListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IFoldStatusListener> listener;
    sptr<DisplayManager::Impl> impl_;
    DisplayManager::GetInstance().pImpl_->foldStatusListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl::DisplayManagerFoldStatusAgent> foldStatusListenerAgent =
        new DisplayManager::Impl::DisplayManagerFoldStatusAgent(impl_);
    auto ret = DisplayManager::GetInstance().pImpl_->RegisterFoldStatusListener(listener);
    ASSERT_EQ(ret, SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            foldStatusListenerAgent,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER));
    listener = nullptr;
    foldStatusListenerAgent.clear();
}

/**
 * @tc.name: UnregisterFoldStatusListener
 * @tc.desc: UnregisterFoldStatusListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldStatusListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IFoldStatusListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterFoldStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IFoldStatusListener();
    ret = DisplayManager::GetInstance().UnregisterFoldStatusListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterFoldStatusListener(listener));
    listener.clear();
}

/**
 * @tc.name: RegisterDisplayModeListener
 * @tc.desc: RegisterDisplayModeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterDisplayModeListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayModeListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterDisplayModeListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IDisplayModeListener();
    ret = DisplayManager::GetInstance().RegisterDisplayModeListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterDisplayModeListener(listener));
    listener.clear();
}

/**
 * @tc.name: ImplRegisterDisplayModeListener
 * @tc.desc: ImplRegisterDisplayModeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplRegisterDisplayModeListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayModeListener> listener;
    DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl> impl_;
    sptr<DisplayManager::Impl::DisplayManagerDisplayModeAgent> displayModeListenerAgent =
        new DisplayManager::Impl::DisplayManagerDisplayModeAgent(impl_);
    auto ret = DisplayManager::GetInstance().pImpl_->RegisterDisplayModeListener(listener);
    ASSERT_EQ(ret, SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            displayModeListenerAgent,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER));
    listener.clear();
    displayModeListenerAgent.clear();
}

/**
 * @tc.name: UnregisterDisplayModeListener
 * @tc.desc: UnregisterDisplayModeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterDisplayModeListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayModeListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterDisplayModeListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IDisplayModeListener();
    ret = DisplayManager::GetInstance().UnregisterDisplayModeListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterDisplayModeListener(listener));
    listener.clear();
}

/**
 * @tc.name: ImplUnregisterDisplayModeListener
 * @tc.desc: ImplUnregisterDisplayModeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterDisplayModeListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayModeListener> listener;
    auto ret = DisplayManager::GetInstance().pImpl_->UnregisterDisplayModeListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: OnDisplayCreate02
 * @tc.desc: OnDisplayCreate
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, OnDisplayCreate02, Function | SmallTest | Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    DisplayManager::GetInstance().RegisterDisplayListener(listener);
    auto displayManagerListener = DisplayManager::GetInstance().pImpl_->displayManagerListener_;
    ASSERT_NE(displayManagerListener, nullptr);

    sptr<DisplayInfo> displayInfo = nullptr;
    displayManagerListener->OnDisplayCreate(displayInfo);
    ASSERT_NE(displayManagerListener->pImpl_, nullptr);
}

/**
 * @tc.name: RegisterDisplayListener
 * @tc.desc: RegisterDisplayListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterDisplayListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    bool hasPrivate = true;
    DisplayManager::GetInstance().pImpl_->NotifyPrivateWindowStateChanged(hasPrivate);
    auto ret = DisplayManager::GetInstance().RegisterDisplayListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: UnregisterDisplayListener
 * @tc.desc: UnregisterDisplayListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterDisplayListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    sptr<DisplayManager::Impl> impl_;
    auto ret = DisplayManager::GetInstance().UnregisterDisplayListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: HasPrivateWindow fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindow, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    bool hasPrivateWindow = false;
    auto ret = DisplayManager::GetInstance().HasPrivateWindow(0, hasPrivateWindow);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: HasPrivateWindow02
 * @tc.desc: HasPrivateWindow fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindow02, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    sptr<DisplayManager::Impl> impl_;
    bool hasPrivateWindow = false;
    auto ret = DisplayManager::GetInstance().pImpl_->HasPrivateWindow(0, hasPrivateWindow);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: IsFoldable
 * @tc.desc: IsFoldable all
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, IsFoldable, Function | SmallTest | Level1)
{
    bool ret = DisplayManager::GetInstance().IsFoldable();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: IsFoldable
 * @tc.desc: IsFoldable fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, IsFoldable01, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    FoldDisplayMode mode = FoldDisplayMode{0};
    DisplayManager::GetInstance().SetFoldDisplayMode(mode);
    DisplayManager::GetInstance().SetFoldStatusLocked(false);
    sptr<DisplayManager::Impl> impl_;
    auto ret = DisplayManager::GetInstance().pImpl_->IsFoldable();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: AddSurfaceNodeToDisplay
 * @tc.desc: AddSurfaceNodeToDisplay fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, AddSurfaceNodeToDisplay, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    std::shared_ptr<class RSSurfaceNode> surfaceNode;
    auto ret = DisplayManager::GetInstance().AddSurfaceNodeToDisplay(0, surfaceNode);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(ret, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: RemoveSurfaceNodeFromDisplay
 * @tc.desc: RemoveSurfaceNodeFromDisplay fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RemoveSurfaceNodeFromDisplay, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    std::shared_ptr<class RSSurfaceNode> surfaceNode;
    DisplayManager::GetInstance().OnRemoteDied();
    auto ret = DisplayManager::GetInstance().RemoveSurfaceNodeFromDisplay(0, surfaceNode);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(ret, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: RegisterFoldAngleListener01
 * @tc.desc: RegisterFoldAngleListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterFoldAngleListener01, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IFoldAngleListener> listener = nullptr;
    auto ret = DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IFoldAngleListener();
    ret = DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterFoldAngleListener(listener));
    listener.clear();
}

/**
 * @tc.name: UnregisterFoldAngleListener01
 * @tc.desc: UnregisterFoldAngleListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener01, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IFoldAngleListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IFoldAngleListener();
    ret = DisplayManager::GetInstance().UnregisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterFoldAngleListener(listener));
    listener.clear();
}

/**
 * @tc.name: RegisterCaptureStatusListener01
 * @tc.desc: RegisterCaptureStatusListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterCaptureStatusListener01, Function | SmallTest | Level1)
{
    sptr<DisplayManager::ICaptureStatusListener> listener = nullptr;
    auto ret = DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::ICaptureStatusListener();
    ret = DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterCaptureStatusListener(listener));
    listener.clear();
}

/**
 * @tc.name: UnregisterCaptureStatusListener01
 * @tc.desc: UnregisterCaptureStatusListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener01, Function | SmallTest | Level1)
{
    sptr<DisplayManager::ICaptureStatusListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::ICaptureStatusListener();
    ret = DisplayManager::GetInstance().UnregisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterCaptureStatusListener(listener));
    listener.clear();
}

/**
 * @tc.name: RegisterDisplayUpdateListener01
 * @tc.desc: RegisterDisplayUpdateListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterDisplayUpdateListener01, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayUpdateListener> listener = nullptr;
    auto ret = DisplayManager::GetInstance().RegisterDisplayUpdateListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IDisplayUpdateListener();
    ret = DisplayManager::GetInstance().RegisterDisplayUpdateListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterDisplayUpdateListener(listener));
    listener.clear();
}

/**
 * @tc.name: IsCaptured01
 * @tc.desc: IsCaptured01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, IsCaptured01, Function | SmallTest | Level1)
{
    auto ret = DisplayManager::GetInstance().IsCaptured();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: isinsideof
 * @tc.desc: isinside0f fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, isinsideof, Function | SmallTest | Level1)
{
    DMRect rect = {2, 2, 2, 2};
    DMRect rect1 = {2, 2, 2, 2};
    ASSERT_EQ(rect.IsInsideOf(rect1), true);
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: GetAllDisplayPhysicalResolution test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetAllDisplayPhysicalResolution, Function | SmallTest | Level1)
{
    std::vector<DisplayPhysicalResolution> allSize = DisplayManager::GetInstance().GetAllDisplayPhysicalResolution();
    ASSERT_TRUE(!allSize.empty());
}

/**
 * @tc.name: ClearDisplayStateCallback
 * @tc.desc: ClearDisplayStateCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayStateCallback, Function | SmallTest | Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearDisplayStateCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayStateAgent_ == nullptr);
}

/**
 * @tc.name: ClearFoldStatusCallback
 * @tc.desc: ClearFoldStatusCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearFoldStatusCallback, Function | SmallTest | Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearFoldStatusCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->foldStatusListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearFoldAngleCallback
 * @tc.desc: ClearFoldAngleCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearFoldAngleCallback, Function | SmallTest | Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearFoldAngleCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->foldAngleListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearCaptureStatusCallback
 * @tc.desc: ClearCaptureStatusCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearCaptureStatusCallback, Function | SmallTest | Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearCaptureStatusCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->captureStatusListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearDisplayModeCallback
 * @tc.desc: ClearDisplayModeCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayModeCallback, Function | SmallTest | Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearDisplayModeCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ == nullptr);
}

/**
 * @tc.name: GetDisplayByScreenId
 * @tc.desc: GetDisplayByScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayByScreenId, Function | SmallTest | Level1)
{
    ScreenId screenId = -1;
    auto ret = DisplayManager::GetInstance().pImpl_->GetDisplayByScreenId(screenId);
    ASSERT_TRUE(ret == nullptr);
}

/**
 * @tc.name: UnregisterDisplayUpdateListener
 * @tc.desc: UnregisterDisplayUpdateListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterDisplayUpdateListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IDisplayUpdateListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterDisplayUpdateListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IDisplayUpdateListener();
    ret = DisplayManager::GetInstance().UnregisterDisplayUpdateListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterDisplayUpdateListener(listener));
    listener.clear();
}

/**
 * @tc.name: RegisterAvailableAreaListener
 * @tc.desc: RegisterAvailableAreaListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterAvailableAreaListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IAvailableAreaListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterAvailableAreaListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IAvailableAreaListener();
    ret = DisplayManager::GetInstance().RegisterAvailableAreaListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterAvailableAreaListener(listener));
    listener.clear();
}

/**
 * @tc.name: UnregisterAvailableAreaListener
 * @tc.desc: UnregisterAvailableAreaListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener, Function | SmallTest | Level1)
{
    sptr<DisplayManager::IAvailableAreaListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterAvailableAreaListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IAvailableAreaListener();
    ret = DisplayManager::GetInstance().UnregisterAvailableAreaListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterAvailableAreaListener(listener));
    listener.clear();
}

/**
 * @tc.name: GetDisplayInfoSrting
 * @tc.desc: GetDisplayInfoSrting fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayInfoSrting, Function | SmallTest | Level1)
{
    sptr<DisplayInfo> displayInfo = nullptr;
    auto ret =DisplayManager::GetInstance().pImpl_->GetDisplayInfoSrting(displayInfo);
    ASSERT_EQ(displayInfo, nullptr);
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, OnRemoteDied, Function | SmallTest | Level1)
{
    g_dmIsDestroyed = true;
    DisplayManager::GetInstance().OnRemoteDied();
    ASSERT_EQ(g_dmIsDestroyed, true);
}

/**
 * @tc.name: SetDisplayScale
 * @tc.desc: SetDisplayScale test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, SetDisplayScale, Function | SmallTest | Level1)
{
    DisplayManager& displayManager = DisplayManager::GetInstance();
    ASSERT_NE(displayManager.pImpl_, nullptr);
    const float scaleX = 1.0f;
    const float scaleY = 1.0f;
    const float pivotX = 0.5f;
    const float pivotY = 0.5f;
    sptr<Display> display = displayManager.GetDefaultDisplay();
    ASSERT_NE(display, nullptr);
    ScreenId screenId = display->GetScreenId();
    displayManager.SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
}
}
} // namespace Rosen
} // namespace OHOS