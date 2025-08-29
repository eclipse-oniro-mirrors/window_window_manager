/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "display_manager.cpp"
#include "display_manager.h"
#include "display_manager_proxy.h"
#include "dm_common.h"
#include "mock_display_manager_adapter.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "window_scene.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<DisplayManagerAdapter, MockDisplayManagerAdapter>;
using MockerManager = SingletonMocker<DisplayManager, MockDisplayManger>;
class DmMockScreenshotListener : public DisplayManager::IScreenshotListener {
public:
    void OnScreenshot(const ScreenshotInfo info) override {}
};
class DmMockPrivateWindowListener : public DisplayManager::IPrivateWindowListener {
public:
    void OnPrivateWindow([[maybe_unused]]bool) override {}
};
class DmMockFoldAngleListener : public DisplayManager::IFoldAngleListener {
public:
    void OnFoldAngleChanged([[maybe_unused]]std::vector<float>) override {}
};
class DmMockCaptureStatusListener : public DisplayManager::ICaptureStatusListener {
public:
    void  OnCaptureStatusChanged([[maybe_unused]]bool) override {}
};
class DmMockDisplayListener : public DisplayManager::IDisplayListener {
public:
    void OnCreate(DisplayId) override {}
    void OnDestroy(DisplayId) override {}
    void OnChange(DisplayId) override {}
};
class DmMockDisplayPowerEventListener : public IDisplayPowerEventListener {
public:
    void OnDisplayPowerEvent(DisplayPowerEvent, EventStatus) override {}
};

class DisplayManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
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
static constexpr DisplayId DEFAULT_DISPLAY = 1ULL;
static const int32_t EXECUTION_TIMES = 1;
static const int32_t PIXELMAP_SIZE = 2;
static const int32_t SDR_PIXELMAP = 0;
static const int32_t HDR_PIXELMAP = 1;

/**
 * @tc.name: Freeze01
 * @tc.desc: success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Freeze01, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, Freeze02, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, Freeze03, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, Unfreeze01, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, Unfreeze02, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, Unfreeze03, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterScreenshotListener01, TestSize.Level1)
{
    DMError ret = DisplayManager::GetInstance().RegisterScreenshotListener(nullptr);
    ASSERT_FALSE(DMError::DM_OK == ret);
}

/**
 * @tc.name: RegisterScreenshotListener02
 * @tc.desc: test RegisterScreenshotListener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterScreenshotListener02, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterScreenshotListener01, TestSize.Level1)
{
    DMError ret = DisplayManager::GetInstance().UnregisterScreenshotListener(nullptr);
    ASSERT_FALSE(DMError::DM_OK == ret);
}

/**
 * @tc.name: UnregisterScreenshotListener02
 * @tc.desc: test UnregisterScreenshotListener with null listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterScreenshotListener02, TestSize.Level1)
{
    sptr<DisplayManager::IScreenshotListener> listener = new DmMockScreenshotListener();
    DMError ret = DisplayManager::GetInstance().UnregisterScreenshotListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: OnDisplayCreate01
 * @tc.desc: OnDisplayCreate
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, OnDisplayCreate01, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, CheckRectValid, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, CheckSizeValid, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplGetDefaultDisplay01, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, GetDisplayByScreen, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplGetDefaultDisplaySync, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, GetScreenBrightness, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, GetDisplayById, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterPrivateWindowListener01, TestSize.Level1)
{
    sptr<DisplayManager::IPrivateWindowListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterPrivateWindowListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: RegisterPrivateWindowListener
 * @tc.desc: RegisterPrivateWindowListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterPrivateWindowListener02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_IPC_FAILED));
    sptr<DisplayManager::IPrivateWindowListener> listener = new DmMockPrivateWindowListener();
    auto ret = DisplayManager::GetInstance().RegisterPrivateWindowListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: UnregisterPrivateWindowListener
 * @tc.desc: UnregisterPrivateWindowListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterPrivateWindowListener, TestSize.Level1)
{
    sptr<DisplayManager::IPrivateWindowListener> listener = nullptr;
    auto ret = DisplayManager::GetInstance().UnregisterPrivateWindowListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: ImplUnregisterPrivateWindowListener
 * @tc.desc: ImplUnregisterPrivateWindowListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterPrivateWindowListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterPrivateWindowListChangeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterPrivateWindowListChangeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplRegisterPrivateWindowListChangeListener01, TestSize.Level1)
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
 * @tc.name: ImplRegisterPrivateWindowListChangeListener
 * @tc.desc: ImplRegisterPrivateWindowListChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplRegisterPrivateWindowListChangeListener02, TestSize.Level1)
{
    std::recursive_mutex mutex;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_IPC_FAILED));
    sptr<DisplayManager::Impl> impl_;
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener;
    DisplayManager::GetInstance().pImpl_->privateWindowListChangeListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl::DisplayManagerPrivateWindowListAgent> privateWindowListChangeListenerAgent =
        new DisplayManager::Impl::DisplayManagerPrivateWindowListAgent(impl_);
    auto ret = DisplayManager::GetInstance().pImpl_->RegisterPrivateWindowListChangeListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    listener = nullptr;
    privateWindowListChangeListenerAgent.clear();
}

/**
 * @tc.name: ImplUnregisterPrivateWindowListChangeListener
 * @tc.desc: ImplUnregisterPrivateWindowListChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterPrivateWindowListChangeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplUnregisterFoldStatusListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterFoldStatusListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplRegisterFoldStatusListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterFoldStatusListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterDisplayModeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplRegisterDisplayModeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterDisplayModeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, ImplUnregisterDisplayModeListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, OnDisplayCreate02, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterDisplayListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterDisplayListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, HasPrivateWindow, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, HasPrivateWindow02, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, IsFoldable, TestSize.Level1)
{
    bool ret = DisplayManager::GetInstance().IsFoldable();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: IsFoldable
 * @tc.desc: IsFoldable fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, IsFoldable01, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, AddSurfaceNodeToDisplay, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RemoveSurfaceNodeFromDisplay, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterFoldAngleListener01, TestSize.Level1)
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
 * @tc.name: RegisterFoldAngleListener02
 * @tc.desc: RegisterFoldAngleListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterFoldAngleListener02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_IPC_FAILED));
    DisplayManager::GetInstance().pImpl_->foldAngleListenerAgent_ = nullptr;
    sptr<DisplayManager::IFoldAngleListener> listener = new DmMockFoldAngleListener();
    auto ret = DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    listener.clear();
}

/**
 * @tc.name: UnregisterFoldAngleListener01
 * @tc.desc: UnregisterFoldAngleListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener01, TestSize.Level1)
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
 * @tc.name: UnregisterFoldAngleListener02
 * @tc.desc: UnregisterFoldAngleListener02 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener02, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->foldAngleListeners_.empty());
    sptr<DisplayManager::IFoldAngleListener> listener = new DisplayManager::IFoldAngleListener();
    DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: UnregisterFoldAngleListener03
 * @tc.desc: UnregisterFoldAngleListener03 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener03, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->foldAngleListeners_.empty());
    DisplayManager::GetInstance().pImpl_->foldAngleListeners_.clear();
    sptr<DisplayManager::IFoldAngleListener> listener = new DisplayManager::IFoldAngleListener();
    DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: RegisterCaptureStatusListener01
 * @tc.desc: RegisterCaptureStatusListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterCaptureStatusListener01, TestSize.Level1)
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
 * @tc.name: RegisterCaptureStatusListener02
 * @tc.desc: RegisterCaptureStatusListener02 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterCaptureStatusListener02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_IPC_FAILED));
    DisplayManager::GetInstance().pImpl_->captureStatusListenerAgent_ = nullptr;
    sptr<DisplayManager::ICaptureStatusListener> listener = new DisplayManager::ICaptureStatusListener();
    auto ret = DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    listener.clear();
}

/**
 * @tc.name: UnregisterCaptureStatusListener01
 * @tc.desc: UnregisterCaptureStatusListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener01, TestSize.Level1)
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
 * @tc.name: UnregisterCaptureStatusListener02
 * @tc.desc: UnregisterCaptureStatusListener02 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener02, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->captureStatusListeners_.empty());
    sptr<DisplayManager::ICaptureStatusListener> listener = new DisplayManager::ICaptureStatusListener();
    DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: UnregisterCaptureStatusListener03
 * @tc.desc: UnregisterCaptureStatusListener03 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener03, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->captureStatusListeners_.empty());
    DisplayManager::GetInstance().pImpl_->captureStatusListeners_.clear();
    sptr<DisplayManager::ICaptureStatusListener> listener = new DisplayManager::ICaptureStatusListener();
    DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: RegisterDisplayUpdateListener01
 * @tc.desc: RegisterDisplayUpdateListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterDisplayUpdateListener01, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, IsCaptured01, TestSize.Level1)
{
    auto ret = DisplayManager::GetInstance().IsCaptured();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: isinsideof
 * @tc.desc: isinside0f fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, isinsideof, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, GetAllDisplayPhysicalResolution, TestSize.Level1)
{
    std::vector<DisplayPhysicalResolution> allSize = DisplayManager::GetInstance().GetAllDisplayPhysicalResolution();
    ASSERT_TRUE(!allSize.empty());
}

/**
 * @tc.name: ClearDisplayStateCallback
 * @tc.desc: ClearDisplayStateCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayStateCallback, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearDisplayStateCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayStateAgent_ == nullptr);
}

/**
 * @tc.name: ClearFoldStatusCallback
 * @tc.desc: ClearFoldStatusCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearFoldStatusCallback, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearFoldStatusCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->foldStatusListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearFoldAngleCallback
 * @tc.desc: ClearFoldAngleCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearFoldAngleCallback, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearFoldAngleCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->foldAngleListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearCaptureStatusCallback
 * @tc.desc: ClearCaptureStatusCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearCaptureStatusCallback, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearCaptureStatusCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->captureStatusListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearDisplayModeCallback
 * @tc.desc: ClearDisplayModeCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayModeCallback01, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearDisplayModeCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearDisplayModeCallback
 * @tc.desc: ClearDisplayModeCallback test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayModeCallback02, TestSize.Level1)
{
    sptr<DisplayManager::IDisplayModeListener> listener = new DisplayManager::IDisplayModeListener();
    DisplayManager::GetInstance().RegisterDisplayModeListener(listener);
    DisplayManager::GetInstance().pImpl_->ClearDisplayModeCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ == nullptr);
}

/**
 * @tc.name: GetDisplayByScreenId
 * @tc.desc: GetDisplayByScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayByScreenId, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterDisplayUpdateListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, RegisterAvailableAreaListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, GetDisplayInfoSrting, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, OnRemoteDied, TestSize.Level1)
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
HWTEST_F(DisplayManagerTest, SetDisplayScale, TestSize.Level1)
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

/**
 * @tc.name: Clear
 * @tc.desc: Clear test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Clear01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    DisplayManager::GetInstance().pImpl_->displayManagerListener_ = nullptr;
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    DisplayManager::GetInstance().RegisterDisplayListener(listener);
    auto displayManagerListener = DisplayManager::GetInstance().pImpl_->displayManagerListener_;
    ASSERT_NE(displayManagerListener, nullptr);
    DisplayManager::GetInstance().pImpl_->Clear();
    ASSERT_EQ(DisplayManager::GetInstance().pImpl_->displayManagerListener_, nullptr);
}

/**
 * @tc.name: Clear
 * @tc.desc: Clear test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Clear02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_ = nullptr;
    sptr<IDisplayPowerEventListener> listener = new DmMockDisplayPowerEventListener();
    DisplayManager::GetInstance().RegisterDisplayPowerEventListener(listener);
    auto powerEventListenerAgent = DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_;
    ASSERT_NE(powerEventListenerAgent, nullptr);
    DisplayManager::GetInstance().pImpl_->Clear();
    ASSERT_EQ(DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_, nullptr);
}

/**
 * @tc.name: NotifyCaptureStatusChanged
 * @tc.desc: NotifyCaptureStatusChanged fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, NotifyCaptureStatusChanged, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl> impl_;
    sptr<DisplayManager::Impl::DisplayManagerDisplayModeAgent> displayModeListenerAgent =
        new DisplayManager::Impl::DisplayManagerDisplayModeAgent(impl_);
    bool isCapture = true;
    DisplayManager::GetInstance().pImpl_->NotifyCaptureStatusChanged(isCapture);
    ASSERT_EQ(DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_, nullptr);
}

/**
 * @tc.name: NotifyCaptureStatusChanged01
 * @tc.desc: NotifyCaptureStatusChanged01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, NotifyCaptureStatusChanged01, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl> impl_;
    sptr<DisplayManager::Impl::DisplayManagerDisplayModeAgent> displayModeListenerAgent =
        new DisplayManager::Impl::DisplayManagerDisplayModeAgent(impl_);
    bool isCapture = true;
    DisplayManager::GetInstance().pImpl_->NotifyCaptureStatusChanged(isCapture);
    ASSERT_EQ(DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_, nullptr);
}

/**
 * @tc.name: RegisterFoldAngleListener03
 * @tc.desc: RegisterFoldAngleListener03 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterFoldAngleListener03, TestSize.Level1)
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
 * @tc.name: RegisterFoldAngleListener04
 * @tc.desc: RegisterFoldAngleListener04 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterFoldAngleListener04, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_IPC_FAILED));
    DisplayManager::GetInstance().pImpl_->foldAngleListenerAgent_ = nullptr;
    sptr<DisplayManager::IFoldAngleListener> listener = new DmMockFoldAngleListener();
    auto ret = DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    listener.clear();
}

/**
 * @tc.name: UnregisterFoldAngleListener04
 * @tc.desc: UnregisterFoldAngleListener04 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener04, TestSize.Level1)
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
 * @tc.name: UnregisterFoldAngleListener05
 * @tc.desc: UnregisterFoldAngleListener05 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener05, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->foldAngleListeners_.empty());
    sptr<DisplayManager::IFoldAngleListener> listener = new DisplayManager::IFoldAngleListener();
    DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: UnregisterFoldAngleListener06
 * @tc.desc: UnregisterFoldAngleListener06 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterFoldAngleListener06, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->foldAngleListeners_.empty());
    DisplayManager::GetInstance().pImpl_->foldAngleListeners_.clear();
    sptr<DisplayManager::IFoldAngleListener> listener = new DisplayManager::IFoldAngleListener();
    DisplayManager::GetInstance().RegisterFoldAngleListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterFoldAngleListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: RegisterCaptureStatusListener03
 * @tc.desc: RegisterCaptureStatusListener03 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterCaptureStatusListener03, TestSize.Level1)
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
 * @tc.name: RegisterCaptureStatusListener04
 * @tc.desc: RegisterCaptureStatusListener04 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterCaptureStatusListener04, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_ERROR_IPC_FAILED));
    DisplayManager::GetInstance().pImpl_->captureStatusListenerAgent_ = nullptr;
    sptr<DisplayManager::ICaptureStatusListener> listener = new DisplayManager::ICaptureStatusListener();
    auto ret = DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    listener.clear();
}

/**
 * @tc.name: UnregisterCaptureStatusListener04
 * @tc.desc: UnregisterCaptureStatusListener04 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener04, TestSize.Level1)
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
 * @tc.name: UnregisterCaptureStatusListener05
 * @tc.desc: UnregisterCaptureStatusListener05 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener05, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->captureStatusListeners_.empty());
    sptr<DisplayManager::ICaptureStatusListener> listener = new DisplayManager::ICaptureStatusListener();
    DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: UnregisterCaptureStatusListener06
 * @tc.desc: UnregisterCaptureStatusListener06 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterCaptureStatusListener06, TestSize.Level1)
{
    ASSERT_FALSE(DisplayManager::GetInstance().pImpl_->captureStatusListeners_.empty());
    DisplayManager::GetInstance().pImpl_->captureStatusListeners_.clear();
    sptr<DisplayManager::ICaptureStatusListener> listener = new DisplayManager::ICaptureStatusListener();
    DisplayManager::GetInstance().RegisterCaptureStatusListener(listener);
    auto ret = DisplayManager::GetInstance().UnregisterCaptureStatusListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: RegisterDisplayUpdateListener02
 * @tc.desc: RegisterDisplayUpdateListener02 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterDisplayUpdateListener02, TestSize.Level1)
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
 * @tc.name: IsCaptured02
 * @tc.desc: IsCaptured02 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, IsCaptured02, TestSize.Level1)
{
    auto ret = DisplayManager::GetInstance().IsCaptured();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: isinsideof01
 * @tc.desc: isinsideof01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, isinsideof01, TestSize.Level1)
{
    DMRect rect = {2, 2, 2, 2};
    DMRect rect1 = {2, 2, 2, 2};
    ASSERT_EQ(rect.IsInsideOf(rect1), true);
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution01
 * @tc.desc: GetAllDisplayPhysicalResolution01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetAllDisplayPhysicalResolution01, TestSize.Level1)
{
    std::vector<DisplayPhysicalResolution> allSize = DisplayManager::GetInstance().GetAllDisplayPhysicalResolution();
    ASSERT_TRUE(!allSize.empty());
}

/**
 * @tc.name: ClearDisplayStateCallback01
 * @tc.desc: ClearDisplayStateCallback01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayStateCallback01, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearDisplayStateCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayStateAgent_ == nullptr);
}

/**
 * @tc.name: ClearFoldStatusCallback01
 * @tc.desc: ClearFoldStatusCallback01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearFoldStatusCallback01, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearFoldStatusCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->foldStatusListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearFoldAngleCallback01
 * @tc.desc: ClearFoldAngleCallback01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearFoldAngleCallback01, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearFoldAngleCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->foldAngleListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearCaptureStatusCallback01
 * @tc.desc: ClearCaptureStatusCallback01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearCaptureStatusCallback01, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearCaptureStatusCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->captureStatusListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearDisplayModeCallback03
 * @tc.desc: ClearDisplayModeCallback03 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayModeCallback03, TestSize.Level1)
{
    DisplayManager::GetInstance().pImpl_->ClearDisplayModeCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ == nullptr);
}

/**
 * @tc.name: ClearDisplayModeCallback04
 * @tc.desc: ClearDisplayModeCallback04 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ClearDisplayModeCallback04, TestSize.Level1)
{
    sptr<DisplayManager::IDisplayModeListener> listener = new DisplayManager::IDisplayModeListener();
    DisplayManager::GetInstance().RegisterDisplayModeListener(listener);
    DisplayManager::GetInstance().pImpl_->ClearDisplayModeCallback();
    ASSERT_TRUE(DisplayManager::GetInstance().pImpl_->displayModeListenerAgent_ == nullptr);
}

/**
 * @tc.name: GetDisplayByScreenId01
 * @tc.desc: GetDisplayByScreenId01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayByScreenId01, TestSize.Level1)
{
    ScreenId screenId = -1;
    auto ret = DisplayManager::GetInstance().pImpl_->GetDisplayByScreenId(screenId);
    ASSERT_TRUE(ret == nullptr);
}

/**
 * @tc.name: UnregisterDisplayUpdateListener01
 * @tc.desc: UnregisterDisplayUpdateListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterDisplayUpdateListener01, TestSize.Level1)
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
 * @tc.name: RegisterAvailableAreaListener01
 * @tc.desc: RegisterAvailableAreaListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterAvailableAreaListener01, TestSize.Level1)
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
 * @tc.name: UnregisterAvailableAreaListener01
 * @tc.desc: UnregisterAvailableAreaListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener01, TestSize.Level1)
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
 * @tc.name: GetDisplayInfoSrting01
 * @tc.desc: GetDisplayInfoSrting01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetDisplayInfoSrting01, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = nullptr;
    auto ret =DisplayManager::GetInstance().pImpl_->GetDisplayInfoSrting(displayInfo);
    ASSERT_EQ(displayInfo, nullptr);
}

/**
 * @tc.name: OnRemoteDied01
 * @tc.desc: OnRemoteDied01 fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, OnRemoteDied01, TestSize.Level1)
{
    g_dmIsDestroyed = true;
    DisplayManager::GetInstance().OnRemoteDied();
    ASSERT_EQ(g_dmIsDestroyed, true);
}

/**
 * @tc.name: SetDisplayScale01
 * @tc.desc: SetDisplayScale01 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, SetDisplayScale01, TestSize.Level1)
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

/**
 * @tc.name: Clear03
 * @tc.desc: Clear03 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Clear03, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    DisplayManager::GetInstance().pImpl_->displayManagerListener_ = nullptr;
    sptr<DisplayManager::IDisplayListener> listener = new DmMockDisplayListener();
    DisplayManager::GetInstance().RegisterDisplayListener(listener);
    auto displayManagerListener = DisplayManager::GetInstance().pImpl_->displayManagerListener_;
    ASSERT_NE(displayManagerListener, nullptr);
    DisplayManager::GetInstance().pImpl_->Clear();
    ASSERT_EQ(DisplayManager::GetInstance().pImpl_->displayManagerListener_, nullptr);
}

/**
 * @tc.name: Clear04
 * @tc.desc: Clear04 test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, Clear04, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m->Mock(), UnregisterDisplayManagerAgent(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_ = nullptr;
    sptr<IDisplayPowerEventListener> listener = new DmMockDisplayPowerEventListener();
    DisplayManager::GetInstance().RegisterDisplayPowerEventListener(listener);
    auto powerEventListenerAgent = DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_;
    ASSERT_NE(powerEventListenerAgent, nullptr);
    DisplayManager::GetInstance().pImpl_->Clear();
    ASSERT_EQ(DisplayManager::GetInstance().pImpl_->powerEventListenerAgent_, nullptr);
}

/**
 * @tc.name: GetScreenCapture
 * @tc.desc: GetScreenCapture test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenCapture, TestSize.Level1)
{
    CaptureOption captureOption;
    sptr<Display> display = DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_NE(display, nullptr);
    captureOption.displayId_ = display->GetId();
    DmErrorCode errCode;
    std::shared_ptr<Media::PixelMap> pixelMap = DisplayManager::GetInstance().GetScreenCapture(captureOption,
        &errCode);
}

/**
 * @tc.name: GetPrimaryDisplaySync
 * @tc.desc: GetPrimaryDisplaySync test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetPrimaryDisplaySync, TestSize.Level1)
{
    sptr<Display> display = DisplayManager::GetInstance().GetPrimaryDisplaySync();
    ASSERT_NE(display, nullptr);
}

/**
 * @tc.name: RegisterScreenMagneticStateListener
 * @tc.desc: RegisterScreenMagneticStateListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterScreenMagneticStateListener, TestSize.Level1)
{
    sptr<DisplayManager::IScreenMagneticStateListener> listener;
    auto ret = DisplayManager::GetInstance().RegisterScreenMagneticStateListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IScreenMagneticStateListener();
    ret = DisplayManager::GetInstance().RegisterScreenMagneticStateListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->RegisterScreenMagneticStateListener(listener));
    listener.clear();
}

/**
 * @tc.name: ImplRegisterScreenMagneticStateListener
 * @tc.desc: ImplRegisterScreenMagneticStateListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplRegisterScreenMagneticStateListener, TestSize.Level1)
{
    sptr<DisplayManager::IScreenMagneticStateListener> listener;
    DisplayManager::GetInstance().pImpl_->screenMagneticStateListenerAgent_ = nullptr;
    sptr<DisplayManager::Impl> impl_;
    sptr<DisplayManager::Impl::DisplayManagerScreenMagneticStateAgent> screenMagneticStateListenerAgent =
        new DisplayManager::Impl::DisplayManagerScreenMagneticStateAgent(impl_);
    auto ret = DisplayManager::GetInstance().pImpl_->RegisterScreenMagneticStateListener(listener);
    ASSERT_EQ(ret, SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            screenMagneticStateListenerAgent,
            DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER));
    listener.clear();
    screenMagneticStateListenerAgent.clear();
}

/**
 * @tc.name: UnregisterScreenMagneticStateListener
 * @tc.desc: UnregisterScreenMagneticStateListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterScreenMagneticStateListener, TestSize.Level1)
{
    sptr<DisplayManager::IScreenMagneticStateListener> listener;
    auto ret = DisplayManager::GetInstance().UnregisterScreenMagneticStateListener(listener);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    listener = new DisplayManager::IScreenMagneticStateListener();
    ret = DisplayManager::GetInstance().UnregisterScreenMagneticStateListener(listener);
    ASSERT_EQ(ret, DisplayManager::GetInstance().pImpl_->UnregisterScreenMagneticStateListener(listener));
    listener.clear();
}

/**
 * @tc.name: ImplUnregisterScreenMagneticStateListener
 * @tc.desc: ImplUnregisterScreenMagneticStateListener fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ImplUnregisterScreenMagneticStateListener, TestSize.Level1)
{
    sptr<DisplayManager::IScreenMagneticStateListener> listener;
    auto ret = DisplayManager::GetInstance().pImpl_->UnregisterScreenMagneticStateListener(listener);
    ASSERT_EQ(ret, DMError::DM_OK);
    listener.clear();
}

/**
 * @tc.name: GetCutoutInfoWithRotation
 * @tc.desc: GetCutoutInfoWithRotation with right rotation
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetCutoutInfoWithRotation, Function | SmallTest | Level2)
{
    Rotation rotation = Rotation::ROTATION_0;
    sptr<CutoutInfo> info = DisplayManager::GetInstance().GetCutoutInfoWithRotation(rotation);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(nullptr, info);
    }
}

/**
 * @tc.name: GetVisibleAreaDisplayInfoById_ShouldReturnNull_WhenDisplayInfoIsNull
 * @tc.desc: Test GetVisibleAreaDisplayInfoById function when the returned DisplayInfo is
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetVisibleAreaDisplayInfoById_ShouldReturnNull_WhenDisplayInfoIsNull, TestSize.Level1)
{
    DisplayId displayId = 123;
    g_dmIsDestroyed = true;

    auto result = DisplayManagerAdapter::GetInstance().GetVisibleAreaDisplayInfoById(displayId);

    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSnapshotByPicker_LockFailed_Test
 * @tc.desc: Test GetSnapshotByPicker when try_lock fails
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetSnapshotByPicker_LockFailed_Test, TestSize.Level1)
{
    DisplayManager displayManager;
    Media::Rect rect;
    DmErrorCode errorCode;

    std::unique_lock<std::mutex> lock(snapBypickerMutex, std::defer_lock);

    auto result = displayManager.GetSnapshotByPicker(rect, &errorCode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSnapshotByPicker_GetSnapshotFailed_Test
 * @tc.desc: Test GetSnapshotByPicker when GetSnapshotByPicker returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetSnapshotByPicker_GetSnapshotFailed_Test, TestSize.Level1)
{
    DisplayManager displayManager;
    Media::Rect rect;
    DmErrorCode errorCode;

    std::shared_ptr<Media::PixelMap> screenShot =
        SingletonContainer::Get<DisplayManagerAdapter>().GetSnapshotByPicker(rect, &errorCode);

    auto result = displayManager.GetSnapshotByPicker(rect, &errorCode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSnapshotByPicker_InvalidSize_Test
 * @tc.desc: Test GetSnapshotByPicker when rect has invalid size
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetSnapshotByPicker_InvalidSize_Test, TestSize.Level1)
{
    DisplayManager displayManager;
    Media::Rect rect;
    rect.width = 0;
    rect.height = 0;
    DmErrorCode errorCode;

    auto result = displayManager.GetSnapshotByPicker(rect, &errorCode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSnapshotByPicker_CreatePixelMapFailed_Test
 * @tc.desc: Test GetSnapshotByPicker when Media::PixelMap::Create returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetSnapshotByPicker_CreatePixelMapFailed_Test, TestSize.Level1)
{
    DisplayManager displayManager;
    Media::Rect rect;
    DmErrorCode errorCode;
    auto pixelMap = nullptr;

    auto result = displayManager.GetSnapshotByPicker(rect, &errorCode);
    EXPECT_EQ(result, pixelMap);
}

/**
 * @tc.name: GetScreenshotreturnsnullptr
 * @tc.desc: GetScreenshot returns nullptr fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenshotreturnsnullptr, TestSize.Level1)
{
    DisplayManager displayManager;
    SnapShotConfig config;
    DmErrorCode errorCode;
    bool isUseDma = false;

    auto result = displayManager.GetScreenshotwithConfig(config, &errorCode, isUseDma);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetScreenHdrshot_ShouldReturnNullptrVector_WhenSnapshotFails
 * @tc.desc: Test that GetScreenHDRshot returns a vector with two nullptrs when the snapshot fails.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHdrshot_ShouldReturnNullptrVector_WhenSnapshotFails, TestSize.Level1)
{
    DisplayManager displayManager;
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;
 
    // Mock the behavior of DisplayManagerAdapter to return a vector with size != PIXMAP_VECTOR_SIZE
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDisplayHDRSnapshot(_, _, _, _)).Times(EXECUTION_TIMES).WillOnce(
        Return(std::vector<std::shared_ptr<Media::PixelMap>> { nullptr }));
 
    std::vector<std::shared_ptr<Media::PixelMap>> result = displayManager.GetScreenHDRshot(
        validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
 
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}
 
/**
 * @tc.name: GetScreenHdrshot_ShouldReturnSnapshotVector_WhenSnapshotSizeIsTwo
 * @tc.desc: Test that GetScreenHDRshot returns a vector with two nullptrs when snapshotSize is two.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHdrshot_ShouldReturnSnapshotVector_WhenSnapshotSizeIsTwo, TestSize.Level1)
{
    DisplayManager displayManager;
    DisplayManagerAdapter displayManagerAdapter;
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;
 
    // Mock the GetDisplayHDRSnapshot function to return a vector with size equal to 2
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDisplayHDRSnapshot(_, _, _, _)).Times(EXECUTION_TIMES).WillOnce(
        Return(std::vector<std::shared_ptr<Media::PixelMap>> { nullptr, nullptr }));
 
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        displayManager.GetScreenHDRshot(validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
 
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}
 
/**
 * @tc.name: GetScreenHdrshot_GetDisplayHDRSnapshot
 * @tc.desc: Test that GetScreenHDRshot returns a vector when snapshotSize is two.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHdrshot_GetDisplayHDRSnapshot, TestSize.Level1)
{
    DisplayManager displayManager;
    DisplayManagerAdapter displayManagerAdapter;
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;
 
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        displayManager.GetScreenHDRshot(validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
 
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}
 
/**
 * @tc.name: GetScreenHdrshot_GetDisplayHDRSnapshot001
 * @tc.desc: Test that GetScreenHDRshot returns a vector when snapshotSize is two.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHdrshot_GetDisplayHDRSnapshot001, TestSize.Level1)
{
    DisplayManager displayManager;
    DisplayManagerAdapter displayManagerAdapter;
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;
 
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        displayManager.GetScreenHDRshot(validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
 
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}
 
/**
 * @tc.name: GetScreenHdrshot_GetDisplayHDRSnapshot002
 * @tc.desc: Test that GetScreenHDRshot returns a vector when snapshotSize is two.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHdrshot_GetDisplayHDRSnapshot002, TestSize.Level1)
{
    DisplayManager displayManager;
    DisplayManagerAdapter displayManagerAdapter;
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;
 
    sptr<IScreenSessionManager> screenSessionManagerServiceProxyTmp =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        displayManager.GetScreenHDRshot(validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxyTmp;
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}

/**
 * @tc.name: InvalidimageRect
 * @tc.desc: Invalid imageRect fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, InvalidimageRect, TestSize.Level1)
{
    DisplayManager displayManager;
    SnapShotConfig config;
    config.imageRect_.left = -1;
    DmErrorCode errorCode;
    bool isUseDma = false;

    auto result = displayManager.GetScreenshotwithConfig(config, &errorCode, isUseDma);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: InvalidimageSize
 * @tc.desc: Invalid imageSize fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, InvalidimageSize, TestSize.Level1)
{
    DisplayManager displayManager;
    SnapShotConfig config;
    config.imageSize_.width = 0;
    DmErrorCode errorCode;
    bool isUseDma = false;

    auto result = displayManager.GetScreenshotwithConfig(config, &errorCode, isUseDma);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: Media::PixelMap::Create returns nullptr
 * @tc.desc: Media::PixelMap::Create returns nullptr fun
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, PixelMapCreateReturnsNullptr, TestSize.Level1)
{
    DisplayManager displayManager;
    SnapShotConfig config;
    DmErrorCode errorCode;
    bool isUseDma = false;

    auto result = displayManager.GetScreenshotwithConfig(config, &errorCode, isUseDma);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetCallingAbilityDisplayId_shouldReturnInvalid_WhenListIsEmpty
 * @tc.desc: GetCallingAbilityDisplayId function when displayIdList_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetCallingAbilityDisplayId_shouldReturnInvalid_WhenListIsEmpty, TestSize.Level1)
{
    DisplayManager displayManager;
    displayManager.displayIdList_.clear();
    EXPECT_EQ(displayManager.GetCallingAbilityDisplayId(), DISPLAY_ID_INVALID);
}

/**
 * @tc.name: GetCallingAbilityDisplayId_shouldReturnInvalid_WhenListContainsOneId
 * @tc.desc: Test GetCallingAbilityDisplayId function when displayIdList_ contains one id.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetCallingAbilityDisplayId_shouldReturnInvalid_WhenListContainsOneId, TestSize.Level1)
{
    DisplayManager displayManager;
    displayManager.displayIdList_.clear();
    ASSERT_TRUE(SingletonContainer::Get<DisplayManagerAdapter>().InitDMSProxy());
    sptr<IRemoteObject> obj;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_, nullptr);
        obj = SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
    } else {
        ASSERT_NE(SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_, nullptr);
        obj = SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_->AsObject();
    }
    sptr<IRemoteObject> weakPtr = obj;
    DisplayId displayId = DISPLAY_ID_INVALID;
    displayManager.displayIdList_.emplace_back(weakPtr, displayId);
    EXPECT_EQ(displayManager.GetCallingAbilityDisplayId(), DISPLAY_ID_INVALID);
}

/**
 * @tc.name: GetCallingAbilityDisplayId_shouldReturnInvalid_WhenListContainsMultipleDifferentIds
 * @tc.desc: Test GetCallingAbilityDisplayId function when displayIdList_ multiple different ids.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetCallingAbilityDisplayId_shouldReturnInvalid_WhenListContainsMultipleDifferentIds,
    TestSize.Level1)
{
    DisplayManager displayManager;
    displayManager.displayIdList_.clear();

    ASSERT_TRUE(SingletonContainer::Get<DisplayManagerAdapter>().InitDMSProxy());
    sptr<IRemoteObject> obj;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_, nullptr);
        obj = SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
    } else {
        ASSERT_NE(SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_, nullptr);
        obj = SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_->AsObject();
    }
    sptr<IRemoteObject> weakPtr = obj;
    DisplayId displayId = DISPLAY_ID_INVALID;
    displayManager.displayIdList_.emplace_back(weakPtr, displayId);

    ASSERT_TRUE(SingletonContainer::Get<DisplayManagerAdapter>().InitDMSProxy());
    sptr<IRemoteObject> obj_01;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_, nullptr);
        obj_01 = SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
    } else {
        ASSERT_NE(SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_, nullptr);
        obj_01 = SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_->AsObject();
    }
    sptr<IRemoteObject> weakPtr_01 = obj_01;
    DisplayId displayId_01 = 2;
    displayManager.displayIdList_.emplace_back(weakPtr_01, displayId_01);

    EXPECT_EQ(displayManager.GetCallingAbilityDisplayId(), DISPLAY_ID_INVALID);
}

/**
 * @tc.name: ShouldReturnUNKNOWN
 * @tc.desc: GetFoldDisplayMode returns UNKNOWN
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ShouldNotReturnGLOBAL_FULL, TestSize.Level1)
{
    DisplayManager displayManager;
    DisplayManagerAdapter displayManagerAdapter;
    SingletonContainer::Get<DisplayManagerAdapter>().InitDMSProxy();
    displayManagerAdapter.SetFoldDisplayMode(FoldDisplayMode::GLOBAL_FULL);

    auto result = displayManager.GetFoldDisplayModeForExternal();

    EXPECT_NE(result, OHOS::Rosen::FoldDisplayMode::GLOBAL_FULL);
}

/**
 * @tc.name: RegisterAvailableAreaListener001
 * @tc.desc: RegisterAvailableAreaListener001 with nullptr listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterAvailableAreaListener001, TestSize.Level1)
{
    DisplayManager dm;
    auto result = dm.RegisterAvailableAreaListener(nullptr, 1);
    EXPECT_EQ(result, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: RegisterAvailableAreaListener002
 * @tc.desc: RegisterAvailableAreaListener002 with non-null listener
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, RegisterAvailableAreaListener002, TestSize.Level1)
{
    DisplayManager dm;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    auto result = dm.RegisterAvailableAreaListener(listener, 1);
    EXPECT_EQ(result, DMError::DM_OK);
}

/**
 * @tc.name: ShouldRegisterListenerWhenAgentIsNull
 * @tc.desc: ShouldRegisterListenerWhenAgentIsNull
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ShouldRegisterListenerWhenAgentIsNull, TestSize.Level1)
{
    DisplayManager dm;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    DisplayId displayId = 1;

    DMError result = dm.RegisterAvailableAreaListener(listener, displayId);

    EXPECT_EQ(result, DMError::DM_OK);
}

/**
 * @tc.name: UnregisterAvailableAreaListener001
 * @tc.desc: UnregisterAvailableAreaListener001
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener001, TestSize.Level1)
{
    DisplayManager dm;
    auto result = dm.UnregisterAvailableAreaListener(nullptr, 1);
    EXPECT_EQ(result, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterAvailableAreaListener002
 * @tc.desc: UnregisterAvailableAreaListener002
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener002, TestSize.Level1)
{
    DisplayManager dm;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    DisplayId displayId = 1;

    EXPECT_EQ(dm.UnregisterAvailableAreaListener(listener, displayId), DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterAvailableAreaListener003
 * @tc.desc: UnregisterAvailableAreaListener003
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener003, TestSize.Level1)
{
    DisplayManager dm;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    auto result = dm.UnregisterAvailableAreaListener(nullptr, 1);
    EXPECT_EQ(result, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterAvailableAreaListener004
 * @tc.desc: UnregisterAvailableAreaListener004
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener004, TestSize.Level1)
{
    DisplayManager displayManager;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    DisplayId displayId = 100;

    DMError result = displayManager.UnregisterAvailableAreaListener(listener, displayId);

    EXPECT_EQ(result, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterAvailableAreaListener005
 * @tc.desc: UnregisterAvailableAreaListener005
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener005, TestSize.Level1)
{
    DisplayManager displayManager;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    DisplayId displayId = 1;

    std::set<sptr<DisplayManager::IAvailableAreaListener>> mySet;
    mySet.insert(listener);
    displayManager.pImpl_->availableAreaListenersMap_.insert({displayId, mySet});
    DMError result = displayManager.UnregisterAvailableAreaListener(listener, displayId);

    EXPECT_EQ(result, DMError::DM_OK);
}

/**
 * @tc.name: UnregisterAvailableAreaListener006
 * @tc.desc: UnregisterAvailableAreaListener006
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, UnregisterAvailableAreaListener006, TestSize.Level1)
{
    DisplayManager displayManager;
    sptr<DisplayManager::IAvailableAreaListener> listener;
    listener = new DisplayManager::IAvailableAreaListener();
    DisplayId displayId = 1;

    std::set<sptr<DisplayManager::IAvailableAreaListener>> mySet;
    displayManager.pImpl_->availableAreaListenersMap_.insert({displayId, mySet});
    DMError result = displayManager.UnregisterAvailableAreaListener(listener, displayId);

    EXPECT_EQ(result, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetScreenshotWithOption_ShouldReturnNull_WhenDisplayIdInvalid
 * @tc.desc: Test GetScreenshotWithOption function when display is idInvalid
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenshotWithOption_ShouldReturnNull_WhenDisplayIdInvalid, TestSize.Level1)
{
    CaptureOption captureOption;
    captureOption.displayId_ = DISPLAY_ID_INVALID;
    DmErrorCode errorCode;

    std::shared_ptr<Media::PixelMap> result =
        DisplayManager::GetInstance().GetScreenshotWithOption(captureOption, &errorCode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetScreenshotWithOption_ShouldReturnNull_WhenGetSnapshotFailed
 * @tc.desc: Test GetScreenshotWithOption function when GetDisplaySnapshotWithOption returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenshotWithOption_ShouldReturnNull_WhenGetSnapshotFailed, TestSize.Level1)
{
    CaptureOption captureOption;
    captureOption.displayId_ = 1;
    DmErrorCode errorCode;

    std::shared_ptr<Media::PixelMap> result =
        DisplayManager::GetInstance().GetScreenshotWithOption(captureOption, &errorCode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: ShouldReturnNullptrWhenScreenshotCaptureFailes
 * @tc.desc: Test screenshot capture failure
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ShouldReturnNullptrWhenScreenshotCaptureFailes, TestSize.Level1)
{
    CaptureOption captureOption;
    Media::Rect rect;
    Media::Size size;
    int rotation = 0;
    DmErrorCode errorCode;

    auto result =
        DisplayManager::GetInstance().GetScreenshotWithOption(captureOption, rect, size, rotation, &errorCode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetScreenHDRshotWithOption_ShouldReturnNull_WhenDisplayIdInvalid
 * @tc.desc: Test GetScreenHDRshotWithOption function when display is idInvalid
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHDRshotWithOption_ShouldReturnNull_WhenDisplayIdInvalid, TestSize.Level1)
{
    CaptureOption captureOption;
    captureOption.displayId_ = DISPLAY_ID_INVALID;
    DmErrorCode errorCode;
 
    std::vector<std::shared_ptr<Media::PixelMap>> pixRVec= { nullptr, nullptr };
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        DisplayManager::GetInstance().GetScreenHDRshotWithOption(captureOption, errorCode);
    EXPECT_EQ(result, pixRVec);
}
 
/**
 * @tc.name: GetScreenHDRshotWithOption_ShouldReturnNull_WhenGetSnapshotFailed
 * @tc.desc: Test GetScreenHDRshotWithOption function when GetDisplaySnapshotWithOption returns { nullptr, nullptr }.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHDRshotWithOption_ShouldReturnNull_WhenGetSnapshotFailed, TestSize.Level1)
{
    CaptureOption captureOption;
    captureOption.displayId_ = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
 
    std::vector<std::shared_ptr<Media::PixelMap>> pixRVec= { nullptr, nullptr };
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        DisplayManager::GetInstance().GetScreenHDRshotWithOption(captureOption, errorCode);
    EXPECT_EQ(result, pixRVec);
}
 
/**
 * @tc.name: GetScreenHDRshotWithOption_WhenDisplayIdIsValidButSnapshotFails
 * @tc.desc: Test that the function returns a vector with two nullptrs when displayId is valid but snapshot fails.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetScreenHDRshotWithOption_WhenDisplayIdIsValidButSnapshotFails, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    CaptureOption captureOption;
    captureOption.displayId_ = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
 
    std::unique_ptr<MockerManager> m = std::make_unique<MockerManager>();
 
    EXPECT_CALL(m->Mock(), GetDisplayHDRSnapshotWithOption(_, _)).Times(1).WillOnce(
        Return(std::vector<std::shared_ptr<Media::PixelMap>> { nullptr, nullptr }));
 
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        DisplayManager::GetInstance().GetScreenHDRshotWithOption(captureOption, errorCode);
 
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}

/**
 * @tc.name: GetPrimaryDisplaySystemDpi
 * @tc.desc: Test GetPrimaryDisplaySystemDpi
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, GetPrimaryDisplaySystemDpi, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    float dpi = DisplayManager::GetInstance().GetPrimaryDisplaySystemDpi();
    EXPECT_EQ(dpi, displayInfo->GetDensityInCurResolution());
}

/**
 * @tc.name: ConvertRelativeCoordinateToGlobalNullDisplay
 * @tc.desc: Test convert relative coordinate to global coordinate failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertRelativeCoordinateToGlobalNullDisplay, TestSize.Level1)
{
    RelativePosition relativePosition = {100, {1, 2}};
    Position globalPosition;
    DMError errorCode = DisplayManager::GetInstance().ConvertRelativeCoordinateToGlobal(relativePosition,
        globalPosition);
    EXPECT_EQ(errorCode, DMError::DM_ERROR_ILLEGAL_PARAM);
}

/**
 * @tc.name: ConvertRelativeCoordinateToGlobalSuccess
 * @tc.desc: Test convert relative coordinate to global coordinate success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertRelativeCoordinateToGlobalSuccess, TestSize.Level1)
{
    RelativePosition relativePosition = {0, {10, 20}};
    Position globalPosition;
    DMError errorCode = DisplayManager::GetInstance().ConvertRelativeCoordinateToGlobal(relativePosition,
        globalPosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(globalPosition.x, 10);
    EXPECT_EQ(globalPosition.y, 20);
}

/**
 * @tc.name: ConvertGlobalCoordinateToRelativeOutDisplay
 * @tc.desc: Test convert relative coordinate to global coordinate failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertGlobalCoordinateToRelativeOutDisplay, TestSize.Level1)
{
    RelativePosition relativePosition;
    Position globalPosition = {10000, 20000};
    DMError errorCode = DisplayManager::GetInstance().ConvertGlobalCoordinateToRelative(globalPosition,
        relativePosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(relativePosition.displayId, 0);
    EXPECT_EQ(relativePosition.position.x, 10000);
    EXPECT_EQ(relativePosition.position.y, 20000);
}

/**
 * @tc.name: ConvertGlobalCoordinateToRelativeIndisplay
 * @tc.desc: Test convert relative coordinate to global coordinate success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertGlobalCoordinateToRelativeIndisplay, TestSize.Level1)
{
    RelativePosition relativePosition;
    Position globalPosition = {10, 20};
    DMError errorCode = DisplayManager::GetInstance().ConvertGlobalCoordinateToRelative(globalPosition,
        relativePosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(relativePosition.displayId, 0);
    EXPECT_EQ(relativePosition.position.x, 10);
    EXPECT_EQ(relativePosition.position.y, 20);
}

/**
 * @tc.name: ConvertGlobalCoordinateToRelativeNullDisplay
 * @tc.desc: Test convert relative coordinate to global coordinate while display is null.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertGlobalCoordinateToRelativeNullDisplay, TestSize.Level1)
{
    RelativePosition relativePosition;
    Position globalPosition = {10000, 20000};
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetAllDisplayIds()).Times(1).WillOnce(Return(std::vector<DisplayId>{-1}));
    DMError errorCode = DisplayManager::GetInstance().ConvertGlobalCoordinateToRelative(globalPosition,
        relativePosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(relativePosition.displayId, 0);
    EXPECT_EQ(relativePosition.position.x, 10000);
    EXPECT_EQ(relativePosition.position.y, 20000);
}

/**
 * @tc.name: ConvertGlobalCoordinateToRelativeWithDisplayIdInvalidDisplay
 * @tc.desc: Test convert relative coordinate to global coordinate failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertGlobalCoordinateToRelativeWithDisplayIdInvalidDisplay, TestSize.Level1)
{
    RelativePosition relativePosition;
    Position globalPosition = {10, 20};
    DisplayId displayId = 100;
    DMError errorCode = DisplayManager::GetInstance().ConvertGlobalCoordinateToRelativeWithDisplayId(globalPosition,
        displayId, relativePosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(relativePosition.displayId, 0);
    EXPECT_EQ(relativePosition.position.x, 10);
    EXPECT_EQ(relativePosition.position.y, 20);
}

/**
 * @tc.name: ConvertGlobalCoordinateToRelativeWithDisplayIdOutRange
 * @tc.desc: Test convert relative coordinate to global coordinate failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertGlobalCoordinateToRelativeWithDisplayIdOutRange, TestSize.Level1)
{
    RelativePosition relativePosition;
    Position globalPosition = {-10000, -20000};
    DisplayId displayId = 0;
    DMError errorCode = DisplayManager::GetInstance().ConvertGlobalCoordinateToRelativeWithDisplayId(globalPosition,
        displayId, relativePosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(relativePosition.displayId, 0);
    EXPECT_EQ(relativePosition.position.x, -10000);
    EXPECT_EQ(relativePosition.position.y, -20000);
}

/**
 * @tc.name: ConvertGlobalCoordinateToRelativeWithDisplayIdInRange
 * @tc.desc: Test convert relative coordinate to global coordinate success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, ConvertGlobalCoordinateToRelativeWithDisplayIdInRange, TestSize.Level1)
{
    RelativePosition relativePosition;
    Position globalPosition = {10, 20};
    DisplayId displayId = 0;
    DMError errorCode = DisplayManager::GetInstance().ConvertGlobalCoordinateToRelativeWithDisplayId(globalPosition,
        displayId, relativePosition);
    EXPECT_EQ(errorCode, DMError::DM_OK);
    EXPECT_EQ(relativePosition.displayId, 0);
    EXPECT_EQ(relativePosition.position.x, 10);
    EXPECT_EQ(relativePosition.position.y, 20);
}

/**
 * @tc.name: SetVirtualScreenAsDefault
 * @tc.desc: Test SetVirtualScreenAsDefault
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, SetVirtualScreenAsDefault, TestSize.Level1)
{
#define SCREENLESS_ENABLE
    uint64_t screenId = 0;
    bool res = SingletonContainer::Get<DisplayManager>().SetVirtualScreenAsDefault(screenId);
    EXPECT_FALSE(res);
#undef SCREENLESS_ENABLE
}
}
} // namespace Rosen
} // namespace OHOS
