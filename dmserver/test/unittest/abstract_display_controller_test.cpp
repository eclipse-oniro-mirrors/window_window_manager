/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "abstract_display.h"
#include "abstract_display_controller.h"
#include "display_cutout_controller.h"
#include "screen.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class AbstractDisplayControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    void InitScreen();
    std::string name_ = "abstract_display_controller_test";
    sptr<SupportedScreenModes> modesInfo_;
    std::recursive_mutex mutex_;
    sptr<AbstractScreenController> absScreenController_ = nullptr;
    sptr<AbstractDisplayController> absDisplayController_ = nullptr;
    sptr<AbstractScreen> absScreen_ = nullptr;
    ScreenId defaultScreenId_ = 0;
    DisplayId defaultDisplayId_ = 0;
    sptr<AbstractDisplay> absDisplay_ = nullptr;
    sptr<DisplayCutoutController> displayCutoutController_ = nullptr;
};

void AbstractDisplayControllerTest::SetUpTestCase()
{
}

void AbstractDisplayControllerTest::TearDownTestCase()
{
}

void AbstractDisplayControllerTest::SetUp()
{
    absDisplayController_ = new AbstractDisplayController(mutex_, [](DisplayId, sptr<DisplayInfo>,
        const std::map<DisplayId, sptr<DisplayInfo>>&, DisplayStateChangeType) {});
    ASSERT_NE(nullptr, absDisplayController_);
    absScreenController_ = new AbstractScreenController(mutex_);
    ASSERT_NE(nullptr, absScreenController_);
    absDisplayController_->Init(absScreenController_);

    ScreenId id = absScreenController_->GetDefaultAbstractScreenId();
    defaultScreenId_ = id;

    absScreen_ = absScreenController_->GetAbstractScreen(defaultScreenId_);
    ASSERT_NE(nullptr, absScreen_);

    InitScreen();
    absDisplay_ = absDisplayController_->GetAbstractDisplayByScreen(defaultScreenId_);
    ASSERT_NE(nullptr, absDisplay_);
    defaultDisplayId_ = absDisplay_->GetId();
    displayCutoutController_ = new DisplayCutoutController();
    DisplayId displayid = 1;
    absDisplayController_->abstractDisplayMap_.insert(std::make_pair(displayid, absDisplay_));
}

void AbstractDisplayControllerTest::TearDown()
{
    absScreenController_->ProcessScreenDisconnected(defaultScreenId_);
    absScreenController_ = nullptr;
    absDisplayController_ = nullptr;
    displayCutoutController_ = nullptr;
}

void AbstractDisplayControllerTest::InitScreen()
{
    modesInfo_ = new SupportedScreenModes();
    modesInfo_->width_ = 200; // 200 is test width
    modesInfo_->height_ = 200; // 200 is test height
    modesInfo_->refreshRate_ = 60; // 60 is test data
    absScreen_->modes_[0] = modesInfo_;
}

namespace {
/**
 * @tc.name: OnAbstractScreenConnectAndDisConnect01
 * @tc.desc: OnAbstractScreenConnectAndDisConnect with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, OnAbstractScreenConnectAndDisConnect01, TestSize.Level1)
{
    sptr<AbstractScreen> absScreen = nullptr;
    absDisplayController_->OnAbstractScreenConnect(absScreen);
    absDisplayController_->OnAbstractScreenDisconnect(absScreen);
    absScreen_->groupDmsId_ = SCREEN_ID_INVALID;
    EXPECT_EQ(nullptr, absScreen_->GetGroup());
    absDisplayController_->OnAbstractScreenConnect(absScreen_);
    absDisplayController_->OnAbstractScreenDisconnect(absScreen_);
}

/**
 * @tc.name: OnAbstractScreenConnectAndDisConnect02
 * @tc.desc: OnAbstractScreenConnectAndDisConnect02 with different ScreenCombination
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, OnAbstractScreenConnectAndDisConnect02, TestSize.Level1)
{
    auto group = absScreen_->GetGroup();
    EXPECT_NE(nullptr, group);
    group->screenMap_.insert(std::make_pair(100, absScreen_)); // 100 is test data
    group->combination_ = ScreenCombination::SCREEN_MIRROR;
    absDisplayController_->OnAbstractScreenConnect(absScreen_);
    absDisplayController_->OnAbstractScreenDisconnect(absScreen_);
}

/**
 * @tc.name: OnAbstractScreenConnectAndDisConnect03
 * @tc.desc: OnAbstractScreenConnectAndDisConnect03 with different ScreenCombination
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, OnAbstractScreenConnectAndDisConnect03, TestSize.Level1)
{
    auto group = absScreen_->GetGroup();
    EXPECT_NE(nullptr, group);
    group->screenMap_.insert(std::make_pair(100, absScreen_)); // 100 is test data
    group->combination_ = ScreenCombination::SCREEN_EXPAND;
    absDisplayController_->OnAbstractScreenConnect(absScreen_);
    absDisplayController_->OnAbstractScreenDisconnect(absScreen_);

    group->combination_ = static_cast<ScreenCombination>(100); // 100 is test data
    absDisplayController_->OnAbstractScreenConnect(absScreen_);
    absDisplayController_->OnAbstractScreenDisconnect(absScreen_);
}

/**
 * @tc.name: ProcessNormalScreenDisconnected01
 * @tc.desc: ProcessNormalScreenDisconnected01 failed
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessNormalScreenDisconnected01, TestSize.Level1)
{
    sptr<AbstractScreen> absScreen = nullptr;
    sptr<AbstractScreenGroup> screenGroup = nullptr;
    auto displayId = absDisplayController_->ProcessNormalScreenDisconnected(absScreen, absScreen_->GetGroup(),
        absDisplay_);
    EXPECT_EQ(DISPLAY_ID_INVALID, displayId);

    displayId = absDisplayController_->ProcessNormalScreenDisconnected(absScreen_, screenGroup, absDisplay_);
    EXPECT_EQ(DISPLAY_ID_INVALID, displayId);

    absDisplayController_->abstractDisplayMap_.clear();
    displayId = absDisplayController_->ProcessNormalScreenDisconnected(absScreen_, screenGroup, absDisplay_);
    EXPECT_EQ(DISPLAY_ID_INVALID, displayId);
}

/**
 * @tc.name: ProcessExpandScreenDisconnected01
 * @tc.desc: ProcessExpandScreenDisconnected01 failed
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessExpandScreenDisconnected01, TestSize.Level1)
{
    sptr<AbstractScreen> absScreen = nullptr;
    sptr<AbstractScreenGroup> screenGroup = nullptr;
    auto displayId = absDisplayController_->ProcessExpandScreenDisconnected(absScreen, absScreen_->GetGroup(),
        absDisplay_);
    EXPECT_EQ(DISPLAY_ID_INVALID, displayId);

    displayId = absDisplayController_->ProcessExpandScreenDisconnected(absScreen_, screenGroup, absDisplay_);
    EXPECT_EQ(DISPLAY_ID_INVALID, displayId);

    absDisplayController_->abstractDisplayMap_.clear();
    displayId = absDisplayController_->ProcessExpandScreenDisconnected(absScreen_, screenGroup, absDisplay_);
    EXPECT_EQ(DISPLAY_ID_INVALID, displayId);
}

/**
 * @tc.name: OnAbstractScreenChange01
 * @tc.desc: OnAbstractScreenChange01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, OnAbstractScreenChange01, TestSize.Level1)
{
    sptr<AbstractScreen> absScreen = nullptr;
    absDisplayController_->OnAbstractScreenChange(absScreen, DisplayChangeEvent::UNKNOWN);
    EXPECT_NE(nullptr, absScreen_);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::UNKNOWN);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::UPDATE_ORIENTATION);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::UPDATE_ORIENTATION_FROM_WINDOW);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::UPDATE_ROTATION);
    absDisplayController_->OnAbstractScreenChange(absScreen_, DisplayChangeEvent::UPDATE_ROTATION_FROM_WINDOW);
}

/**
 * @tc.name: ProcessDisplayRotationChange01
 * @tc.desc: ProcessDisplayRotationChange01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessDisplayRotationChange01, TestSize.Level1)
{
    sptr<AbstractScreen> absScreen = new AbstractScreen(absScreenController_, name_, 1, 1);
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen));
    absDisplayController_->ProcessDisplayRotationChange(absScreen, DisplayStateChangeType::UPDATE_ROTATION);

    auto display = absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_);
    EXPECT_NE(nullptr, display);
    display->rotation_ = absScreen_->rotation_;
    EXPECT_EQ(false, display->RequestRotation(absScreen->rotation_));
    absDisplayController_->ProcessDisplayRotationChange(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);
    display->rotation_ = Rotation::ROTATION_270;
    EXPECT_EQ(true, display->RequestRotation(absScreen_->rotation_));
    absDisplayController_->ProcessDisplayRotationChange(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);
}

/**
 * @tc.name: ProcessDisplayCompression01
 * @tc.desc: ProcessDisplayCompression01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessDisplayCompression01, TestSize.Level1)
{
    bool isWaterfallDisplayOrigin = DisplayCutoutController::IsWaterfallDisplay();
    bool isCompressionEnableOrigin =
        DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal();
    uint32_t testSizeOrigin = DisplayCutoutController::GetWaterfallAreaCompressionSizeWhenHorizontal();
    DisplayCutoutController::SetIsWaterfallDisplay(true);
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(true);
    EXPECT_EQ(true, DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal());
    DisplayCutoutController::SetWaterfallAreaCompressionSizeWhenHorizontal(20); // 20 is test size
    uint32_t sizeInVp = DisplayCutoutController::GetWaterfallAreaCompressionSizeWhenHorizontal();
    EXPECT_EQ(20, sizeInVp);
    auto mode = absScreen_->GetActiveScreenMode();
    EXPECT_NE(nullptr, mode);
    mode->height_ = 60; // 60 is test data
    absDisplayController_->ProcessDisplayCompression(absScreen_);

    mode->width_ = 60; // 60 is test data
    absDisplayController_->ProcessDisplayCompression(absScreen_);

    mode->height_ = 100; // 100 is test data
    absDisplayController_->ProcessDisplayCompression(absScreen_);

    mode->width_ = 100; // 100 is test data
    absDisplayController_->ProcessDisplayCompression(absScreen_);
    
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    displayInfo->rotation_ = static_cast<Rotation>(Rotation::ROTATION_90);

    auto oriIdx = absScreen_->activeIdx_;
    absScreen_->activeIdx_ = -1;
    absDisplayController_->ProcessDisplayCompression(absScreen_);
    absScreen_->activeIdx_ = oriIdx;

    DisplayCutoutController::SetIsWaterfallDisplay(isWaterfallDisplayOrigin);
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(isCompressionEnableOrigin);
    DisplayCutoutController::SetWaterfallAreaCompressionSizeWhenHorizontal(testSizeOrigin);
}

/**
 * @tc.name: GetAbstractDisplayByAbsScreen01
 * @tc.desc: GetAbstractDisplayByAbsScreen01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetAbstractDisplayByAbsScreen01, TestSize.Level1)
{
    EXPECT_NE(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_));

    auto oriId = absScreen_->groupDmsId_;
    absScreen_->groupDmsId_ = SCREEN_ID_INVALID;
    sptr<AbstractScreenGroup> group = absScreen_->GetGroup();
    EXPECT_EQ(nullptr, group);
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_));
    absScreen_->groupDmsId_ = oriId;

    group = absScreen_->GetGroup();
    EXPECT_NE(nullptr, group);
    absDisplayController_->abstractDisplayMap_.clear();
    group->combination_ = ScreenCombination::SCREEN_ALONE;
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_));

    group->combination_ = ScreenCombination::SCREEN_EXPAND;
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_));

    group->combination_ = ScreenCombination::SCREEN_MIRROR;
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_));

    group->combination_ = static_cast<ScreenCombination>(100); // 100 is test data
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen_));
}

/**
 * @tc.name: ProcessDisplayUpdateOrientation01
 * @tc.desc: ProcessDisplayUpdateOrientation01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessDisplayUpdateOrientation01, TestSize.Level1)
{
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);

    auto oriId = absScreen_->groupDmsId_;
    absScreen_->groupDmsId_ = SCREEN_ID_INVALID;
    sptr<AbstractScreenGroup> group = absScreen_->GetGroup();
    EXPECT_EQ(nullptr, group);
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);
    absScreen_->groupDmsId_ = oriId;

    group = absScreen_->GetGroup();
    EXPECT_NE(nullptr, group);
    absDisplayController_->abstractDisplayMap_.clear();
    group->combination_ = ScreenCombination::SCREEN_ALONE;
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);

    group->combination_ = ScreenCombination::SCREEN_EXPAND;
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);

    group->combination_ = ScreenCombination::SCREEN_MIRROR;
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);

    group->combination_ = static_cast<ScreenCombination>(100); // 100 is test data
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen_, DisplayStateChangeType::UPDATE_ROTATION);
    
    sptr<AbstractScreen> absScreen = new AbstractScreen(absScreenController_, name_, 1, 1);
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen));
    absDisplayController_->ProcessDisplayUpdateOrientation(absScreen, DisplayStateChangeType::UPDATE_ROTATION);
}

/**
 * @tc.name: ProcessDisplaySizeChange01
 * @tc.desc: ProcessDisplaySizeChange01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessDisplaySizeChange01, TestSize.Level1)
{
    absDisplayController_->ProcessDisplaySizeChange(absScreen_);
    auto display = absDisplayController_->abstractDisplayMap_.at(defaultDisplayId_);
    EXPECT_NE(nullptr, display);
    display->screenId_ = SCREEN_ID_INVALID;
    absDisplayController_->ProcessDisplaySizeChange(absScreen_);

    absDisplayController_->abstractDisplayMap_[defaultDisplayId_] = nullptr;
    absDisplayController_->ProcessDisplaySizeChange(absScreen_);

    absDisplayController_->abstractDisplayMap_.clear();
    absDisplayController_->ProcessDisplaySizeChange(absScreen_);

    SetUp();
    sptr<AbstractScreen> absScreen = new AbstractScreen(absScreenController_, name_, 1, 1);
    EXPECT_EQ(nullptr, absDisplayController_->GetAbstractDisplayByAbsScreen(absScreen));
    absDisplayController_->ProcessDisplaySizeChange(absScreen);

    auto oriIdx = absScreen_->activeIdx_;
    absScreen_->activeIdx_ = -1;
    auto mode = absScreen_->GetActiveScreenMode();
    EXPECT_EQ(nullptr, mode);
    absDisplayController_->ProcessDisplaySizeChange(absScreen_);
    absScreen_->activeIdx_  = oriIdx;
}

/**
 * @tc.name: UpdateDisplaySize01
 * @tc.desc: UpdateDisplaySize01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, UpdateDisplaySize01, TestSize.Level1)
{
    EXPECT_EQ(false, absDisplayController_->UpdateDisplaySize(nullptr, modesInfo_));

    EXPECT_EQ(false, absDisplayController_->UpdateDisplaySize(absDisplay_, nullptr));

    EXPECT_EQ(false, absDisplayController_->UpdateDisplaySize(nullptr, nullptr));

    sptr<SupportedScreenModes> info = new SupportedScreenModes();

    info->height_ = absDisplay_->GetHeight();
    info->width_ = absDisplay_->GetWidth();
    EXPECT_EQ(false, absDisplayController_->UpdateDisplaySize(absDisplay_, info));

    info->width_ = absDisplay_->GetWidth();
    info->height_ = 200; // 200 is test height
    EXPECT_EQ(true, absDisplayController_->UpdateDisplaySize(absDisplay_, info));

    info->width_ = 200; // 200 is test height
    info->height_ = absDisplay_->GetHeight();
    EXPECT_EQ(true, absDisplayController_->UpdateDisplaySize(absDisplay_, info));

    info->width_ = 100; // 100 is test width
    info->height_ = 100; // 100 is test height
    EXPECT_EQ(true, absDisplayController_->UpdateDisplaySize(absDisplay_, info));
}

/**
 * @tc.name: ProcessVirtualPixelRatioChange01
 * @tc.desc: ProcessVirtualPixelRatioChange01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, ProcessVirtualPixelRatioChange01, TestSize.Level1)
{
    auto display = absDisplayController_->abstractDisplayMap_.at(defaultDisplayId_);
    EXPECT_NE(nullptr, display);
    display->screenId_ = SCREEN_ID_INVALID;
    absDisplayController_->ProcessVirtualPixelRatioChange(absScreen_);

    absDisplayController_->abstractDisplayMap_.clear();
    absDisplayController_->ProcessVirtualPixelRatioChange(absScreen_);
}

/**
 * @tc.name: BindAloneScreenLocked01
 * @tc.desc: BindAloneScreenLocked01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, BindAloneScreenLocked01, TestSize.Level1)
{
    absDisplayController_->BindAloneScreenLocked(nullptr);

    sptr<AbstractScreen> absScreen = new AbstractScreen(absScreenController_, name_, 1, 1);
    EXPECT_NE(nullptr, absScreen);
    absDisplayController_->BindAloneScreenLocked(absScreen);

    absDisplayController_->dummyDisplay_ = absDisplay_;
    sptr<SupportedScreenModes> info = absScreen_->GetActiveScreenMode();
    info->height_ = absDisplay_->GetHeight();
    info->width_ = absDisplay_->GetWidth();
    bool updateFlag = static_cast<uint32_t>(absDisplayController_->dummyDisplay_->GetHeight()) == info->height_
            && static_cast<uint32_t>(absDisplayController_->dummyDisplay_->GetWidth()) == info->width_;
    EXPECT_NE(nullptr, info);
    EXPECT_EQ(true, updateFlag);
    absDisplayController_->BindAloneScreenLocked(absScreen_);

    absDisplay_->SetWidth(100); // 100 is test size
    absDisplay_->SetHeight(100); // 100 is test size
    absDisplayController_->dummyDisplay_ = absDisplay_;
    EXPECT_NE(nullptr, info);
    updateFlag = static_cast<uint32_t>(absDisplayController_->dummyDisplay_->GetHeight()) == info->height_
            && static_cast<uint32_t>(absDisplayController_->dummyDisplay_->GetWidth()) == info->width_;
    EXPECT_EQ(false, updateFlag);
    absDisplayController_->BindAloneScreenLocked(absScreen_);

    auto oriIdx = absScreen_->activeIdx_;
    absScreen_->activeIdx_ = -1;
    info = absScreen_->GetActiveScreenMode();
    EXPECT_EQ(nullptr, info);
    absDisplayController_->BindAloneScreenLocked(absScreen_);
    absScreen_->activeIdx_ = oriIdx;
}

/**
 * @tc.name: AddScreenToExpandLocked01
 * @tc.desc: AddScreenToExpandLocked01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, AddScreenToExpandLocked01, TestSize.Level1)
{
    absDisplayController_->AddScreenToExpandLocked(nullptr);

    sptr<AbstractScreen> absScreen = new AbstractScreen(absScreenController_, name_, 1, 1);
    EXPECT_NE(nullptr, absScreen);
    absDisplayController_->AddScreenToExpandLocked(absScreen);

    absDisplayController_->AddScreenToExpandLocked(absScreen_);

    absDisplayController_->abstractDisplayMap_.clear();
    absDisplayController_->AddScreenToExpandLocked(absScreen_);

    absDisplayController_->abstractDisplayMap_.clear();
    absScreen_->type_ = ScreenType::VIRTUAL;
    absDisplayController_->AddScreenToExpandLocked(absScreen_);
}

/**
 * @tc.name: SetFreeze01
 * @tc.desc: SetFreeze01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, SetFreeze01, TestSize.Level1)
{
    absDisplayController_->abstractDisplayMap_.clear();
    std::vector<DisplayId> displayIds(1, 0);
    auto iter = absDisplayController_->abstractDisplayMap_.find(0);
    EXPECT_EQ(true, iter == absDisplayController_->abstractDisplayMap_.end());
    absDisplayController_->SetFreeze(displayIds, false);
}

/**
 * @tc.name: GetAllDisplayInfoOfGroup01
 * @tc.desc: GetAllDisplayInfoOfGroup01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetAllDisplayInfoOfGroup01, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    EXPECT_NE(nullptr, displayInfo);
    displayInfo->SetScreenGroupId(100); // 100 is test size
    auto displayInfoMap = absDisplayController_->GetAllDisplayInfoOfGroup(displayInfo);
    EXPECT_EQ(0, displayInfoMap.size());
}

/**
 * @tc.name: GetDefaultDisplayId01
 * @tc.desc: GetDefaultDisplayId01
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetDefaultDisplayId01, TestSize.Level1)
{
    auto absDisplayController = new AbstractDisplayController(mutex_, [](DisplayId, sptr<DisplayInfo>,
        const std::map<DisplayId, sptr<DisplayInfo>>&, DisplayStateChangeType) {});
    EXPECT_NE(nullptr, absDisplayController);
    absDisplayController->abstractScreenController_ = new AbstractScreenController(mutex_);
    EXPECT_NE(nullptr, absDisplayController->abstractScreenController_);
    ScreenId defaultScreenId = absDisplayController->abstractScreenController_->GetDefaultAbstractScreenId();
    sptr<AbstractDisplay> defaultDisplay = absDisplayController->GetAbstractDisplayByScreen(defaultScreenId);
    EXPECT_EQ(nullptr, defaultDisplay);
    EXPECT_EQ(DISPLAY_ID_INVALID, absDisplayController->GetDefaultDisplayId());
}

/**
 * @tc.name: GetAbstractDisplayByScreen
 * @tc.desc: GetAbstractDisplayByScreen
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetAbstractDisplayByScreen, TestSize.Level1)
{
    ScreenId screenId = SCREEN_ID_INVALID;
    auto ret = absDisplayController_->GetAbstractDisplayByScreen(screenId);
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name: GetScreenSnapshot
 * @tc.desc: GetScreenSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetScreenSnapshot01, TestSize.Level1)
{
    ScreenId screenId = SCREEN_ID_INVALID;
    auto ret = absDisplayController_->GetScreenSnapshot(screenId);
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name: GetScreenSnapshot
 * @tc.desc: GetScreenSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetScreenSnapshot02, TestSize.Level1)
{
    ScreenId screenId = 2;
    auto ret = absDisplayController_->GetScreenSnapshot(screenId);
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name: GetScreenSnapshot
 * @tc.desc: GetScreenSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, GetScreenSnapshot03, TestSize.Level1)
{
    ScreenId screenId = 1;
    auto ret = absDisplayController_->GetScreenSnapshot(screenId);
    ASSERT_EQ(nullptr, ret);
}

/**
 * @tc.name: FeatureTest
 * @tc.desc: SetScreenOffsetInner
 * @tc.type: FUNC
 */
HWTEST_F(AbstractDisplayControllerTest, FeatureTest, TestSize.Level1)
{
    ScreenId screenId = 0;
    auto ret = absDisplayController_->SetScreenOffsetInner(screenId);
    EXPECT_TRUE(ret);
    screenId = 6;
    ret = absDisplayController_->SetScreenOffsetInner(screenId);
    EXPECT_FALSE(ret);
}
}
} // namespace Rosen
} // namespace OHOS
