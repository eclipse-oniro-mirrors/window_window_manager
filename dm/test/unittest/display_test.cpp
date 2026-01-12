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
#include "display_manager.h"
#include "display_manager_proxy.h"
#include "mock_display_manager_adapter.h"
#include "singleton_mocker.h"
#include "display_cutout_controller.h"
#include "scene_board_judgement.h"
#include "display.cpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<DisplayManagerAdapter, MockDisplayManagerAdapter>;
using ScreenMocker = SingletonMocker<ScreenManagerAdapter, MockScreenManagerAdapter>;
class DisplayTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    static sptr<Display> defaultDisplay_;
};
sptr<Display> DisplayTest::defaultDisplay_ = nullptr;

void DisplayTest::SetUpTestCase()
{
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    defaultDisplay_ = sptr<Display>::MakeSptr("", displayInfo);
}

void DisplayTest::TearDownTestCase()
{
    defaultDisplay_ = nullptr;
}

void DisplayTest::SetUp()
{
}

void DisplayTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetCutoutInfo01
 * @tc.desc: GetCutoutInfo with valid defaultDisplayId and return success
 * @tc.type: FUNC
 * @tc.require: issueI5K0JP
 */
HWTEST_F(DisplayTest, GetCutoutInfo01, TestSize.Level1)
{
    auto cutoutInfo = defaultDisplay_->GetCutoutInfo();
    ASSERT_EQ(nullptr, cutoutInfo);
}

/**
 * @tc.name: UpdateDisplayInfo01
 * @tc.desc: UpdateDisplayInfo with nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5K0JP
 */
HWTEST_F(DisplayTest, UpdateDisplayInfo01, TestSize.Level1)
{
    auto baseInfo = defaultDisplay_->GetDisplayInfo();
    auto defaultName = baseInfo->GetName();
    auto defaultDpi = baseInfo->GetName();
    defaultDisplay_->UpdateDisplayInfo(nullptr);

    auto changedInfo = defaultDisplay_->GetDisplayInfo();
    ASSERT_EQ(changedInfo->GetName(), defaultName);
}

/**
 * @tc.name: SetWaterfallCompression01
 * @tc.desc: Set waterfall compression related values with valid input.
 * @tc.type: FUNC
 * @tc.require: issueI5P8CI
 */
HWTEST_F(DisplayTest, SetWaterfallCompression01, TestSize.Level1)
{
    DisplayCutoutController::SetIsWaterfallDisplay(true);
    bool isCompressionEnableOrigin =
        DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal();
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(true);
    uint32_t testSize = 20;
    DisplayCutoutController::SetWaterfallAreaCompressionSizeWhenHorizontal(testSize);
    ASSERT_EQ(true, DisplayCutoutController::IsWaterfallDisplay());
    ASSERT_EQ(true, DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal());
    ASSERT_EQ(testSize, DisplayCutoutController::GetWaterfallAreaCompressionSizeWhenHorizontal());
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(isCompressionEnableOrigin);
    ASSERT_EQ(isCompressionEnableOrigin, DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal());
    ASSERT_FALSE(DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal());
    DisplayCutoutController::SetIsWaterfallDisplay(false);
    ASSERT_FALSE(DisplayCutoutController::IsWaterfallDisplay());
}

/**
 * @tc.name: SetWaterfallCompression02
 * @tc.desc: Set waterfall compression related values with invalid input.
 * @tc.type: FUNC
 * @tc.require: issueI5P8CI
 */
HWTEST_F(DisplayTest, SetWaterfallCompression02, TestSize.Level1)
{
    DisplayCutoutController::SetIsWaterfallDisplay(true);
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(true);

    DisplayCutoutController::SetIsWaterfallDisplay(false);
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(true);
    ASSERT_EQ(false, DisplayCutoutController::IsWaterfallAreaCompressionEnableWhenHorizontal());

    uint32_t testSize = 20;
    DisplayCutoutController::SetIsWaterfallDisplay(true);
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(false);
    DisplayCutoutController::SetWaterfallAreaCompressionSizeWhenHorizontal(testSize);
    ASSERT_EQ(0, DisplayCutoutController::GetWaterfallAreaCompressionSizeWhenHorizontal());

    DisplayCutoutController::SetIsWaterfallDisplay(false);
    DisplayCutoutController::SetWaterfallAreaCompressionEnableWhenHorzontal(false);
    DisplayCutoutController::SetWaterfallAreaCompressionSizeWhenHorizontal(testSize);
    ASSERT_EQ(0, DisplayCutoutController::GetWaterfallAreaCompressionSizeWhenHorizontal());
}

/**
 * @tc.name: GetName01
 * @tc.desc: GetName function cover
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetName01, TestSize.Level1)
{
    auto name = defaultDisplay_->GetName();
    ASSERT_TRUE(name.empty());
}

/**
 * @tc.name: GetDpi01
 * @tc.desc: GetDpi function cover
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetDpi01, TestSize.Level1)
{
    auto dpi = defaultDisplay_->GetDpi();

    auto vpr = defaultDisplay_->GetVirtualPixelRatio();
    ASSERT_EQ(vpr * DOT_PER_INCH, dpi);
}

/**
 * @tc.name: HasImmersiveWindow
 * @tc.desc: test HasImmersiveWindow
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, HasImmersiveWindow, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), HasImmersiveWindow(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    bool immersive = false;
    DMError ret = defaultDisplay_->HasImmersiveWindow(immersive);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: GetPhysicalWidth
 * @tc.desc: test GetPhysicalWidth
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetPhysicalWidth, TestSize.Level1)
{
    auto physicalwidth = defaultDisplay_->GetPhysicalWidth();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(physicalwidth, 0);
    } else {
        ASSERT_EQ(physicalwidth, 0);
    }
}

/**
 * @tc.name: GetPhysicalHeight
 * @tc.desc: test GetPhysicalHeight
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetPhysicalHeight, TestSize.Level1)
{
    auto physicalheight = defaultDisplay_->GetPhysicalHeight();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(physicalheight, 0);
    } else {
        ASSERT_EQ(physicalheight, 0);
    }
}

/**
 * @tc.name: GetAvailableArea
 * @tc.desc: test GetAvailableArea
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetAvailableArea, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetAvailableArea(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    DMRect area;
    auto res = defaultDisplay_ ->GetAvailableArea(area);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: test GetSupportedHDRFormats
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetSupportedHDRFormats, TestSize.Level1)
{
    std::unique_ptr<ScreenMocker> m = std::make_unique<ScreenMocker>();
    EXPECT_CALL(m->Mock(), GetSupportedHDRFormats(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    std::vector<uint32_t> hdrFormats;
    auto res = defaultDisplay_ ->GetSupportedHDRFormats(hdrFormats);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetSupportedColorSpaces
 * @tc.desc: test GetSupportedColorSpaces
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetSupportedColorSpaces, TestSize.Level1)
{
    std::unique_ptr<ScreenMocker> m = std::make_unique<ScreenMocker>();
    EXPECT_CALL(m->Mock(), GetSupportedColorSpaces(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    std::vector<uint32_t> colorSpaces;
    auto res = defaultDisplay_ -> GetSupportedColorSpaces(colorSpaces);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetDisplayInfoWithCache01
 * @tc.desc: test GetDisplayInfoWithCache01
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetDisplayInfoWithCache01, TestSize.Level1)
{
    auto baseInfo = defaultDisplay_->GetDisplayInfo();
    auto defaultName = baseInfo->GetName();
    auto defaultDpi = baseInfo->GetName();
    defaultDisplay_->UpdateDisplayInfo(nullptr);

    auto changedInfo = defaultDisplay_->GetDisplayInfoWithCache();
    EXPECT_EQ(changedInfo->GetName(), defaultName);
}

/**
 * @tc.name: GetLiveCreaseRegion
 * @tc.desc: test GetLiveCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetLiveCreaseRegion, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    sptr<DisplayInfo> baseInfo = sptr<DisplayInfo>::MakeSptr();
    sptr<Display> disPlay = sptr<Display>::MakeSptr("", baseInfo);
    FoldCreaseRegion region;
    DMError ret = disPlay->GetLiveCreaseRegion(region);
    EXPECT_EQ(ret, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);

    ScreenId screenId = 0;
    baseInfo->SetScreenId(screenId);
    ret = disPlay->GetLiveCreaseRegion(region);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, DMError::DM_OK);
    } else {
        EXPECT_EQ(ret, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    }
    if (DisplayManager::GetInstance().IsFoldable()) {
        screenId = 5;
        baseInfo->SetScreenId(screenId);
        ret = disPlay->GetLiveCreaseRegion(region);
        EXPECT_EQ(ret, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetRoundedCorner
 * @tc.desc: test GetRoundedCorner
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetRoundedCorner, TestSize.Level1)
{
    std::vector<RoundedCorner> roundedCorner;
    auto ret = defaultDisplay_->GetRoundedCorner(roundedCorner);
    EXPECT_EQ(ret, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: GetOriginRotation
 * @tc.desc: test GetOriginRotation
 * @tc.type: FUNC
 */
HWTEST_F(DisplayTest, GetOriginRotation, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    sptr<DisplayInfo> baseInfo = sptr<DisplayInfo>::MakeSptr();
    sptr<Display> display = sptr<Display>::MakeSptr("", baseInfo);
    Rotation rotation = Rotation::ROTATION_90;
    baseInfo->SetOriginRotation(rotation);
    EXPECT_EQ(display->GetOriginRotation(), rotation);
 
    display->pImpl_->displayInfo_ = nullptr;
    baseInfo->SetOriginRotation(rotation);
    EXPECT_EQ(display->GetOriginRotation(), Rotation::ROTATION_0);
    display->pImpl_ = nullptr;
    baseInfo->SetOriginRotation(rotation);
    EXPECT_EQ(display->GetOriginRotation(), Rotation::ROTATION_0);
}
}
} // namespace Rosen
} // namespace OHOS