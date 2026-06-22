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
#include "window_session_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class CompatibleModePropertyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CompatibleModePropertyTest::SetUpTestCase() {}

void CompatibleModePropertyTest::TearDownTestCase() {}

namespace {
/**
 * @tc.name: SetIsAdaptToImmersive
 * @tc.desc: SetIsAdaptToImmersive
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToImmersive, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToImmersive(true);
    EXPECT_EQ(property->isAdaptToImmersive_, true);
    property->SetIsAdaptToImmersive(false);
    EXPECT_EQ(property->isAdaptToImmersive_, false);
}

/**
 * @tc.name: IsAdaptToImmersive
 * @tc.desc: IsAdaptToImmersive
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToImmersive, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToImmersive(), false);
    property->isAdaptToImmersive_ = true;
    EXPECT_EQ(property->IsAdaptToImmersive(), true);
}

/**
 * @tc.name: SetIsAdaptToEventMapping
 * @tc.desc: SetIsAdaptToEventMapping
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToEventMapping, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToEventMapping(true);
    EXPECT_EQ(property->isAdaptToEventMapping_, true);
    property->SetIsAdaptToEventMapping(false);
    EXPECT_EQ(property->isAdaptToEventMapping_, false);
}

/**
 * @tc.name: IsAdaptToEventMapping
 * @tc.desc: IsAdaptToEventMapping
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToEventMapping, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToEventMapping(), false);
    property->isAdaptToEventMapping_ = true;
    EXPECT_EQ(property->IsAdaptToEventMapping(), true);
}

/**
 * @tc.name: SetIsAdaptToProportionalScale
 * @tc.desc: SetIsAdaptToProportionalScale
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToProportionalScale, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToProportionalScale(true);
    EXPECT_EQ(property->isAdaptToProportionalScale_, true);
    property->SetIsAdaptToProportionalScale(false);
    EXPECT_EQ(property->isAdaptToProportionalScale_, false);
}

/**
 * @tc.name: IsAdaptToProportionalScale
 * @tc.desc: IsAdaptToProportionalScale
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToProportionalScale, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToProportionalScale(), false);
    property->isAdaptToProportionalScale_ = true;
    EXPECT_EQ(property->IsAdaptToProportionalScale(), true);
}

/**
 * @tc.name: SetIsAdaptToBackButton
 * @tc.desc: SetIsAdaptToBackButton
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToBackButton, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToBackButton(true);
    EXPECT_EQ(property->isAdaptToBackButton_, true);
    property->SetIsAdaptToBackButton(false);
    EXPECT_EQ(property->isAdaptToBackButton_, false);
}

/**
 * @tc.name: IsAdaptToBackButton
 * @tc.desc: IsAdaptToBackButton
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToBackButton, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToBackButton(), false);
    property->isAdaptToBackButton_ = true;
    EXPECT_EQ(property->IsAdaptToBackButton(), true);
}

/**
 * @tc.name: SetIsAdaptToDragScale
 * @tc.desc: SetIsAdaptToDragScale
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToDragScale, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToDragScale(true);
    EXPECT_EQ(property->IsAdaptToDragScale(), true);
    property->SetIsAdaptToDragScale(false);
    EXPECT_EQ(property->IsAdaptToDragScale(), false);
}

/**
 * @tc.name: IsAdaptToDragScale
 * @tc.desc: IsAdaptToDragScale
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToDragScale, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToDragScale(), false);
    property->SetIsAdaptToDragScale(true);
    EXPECT_EQ(property->IsAdaptToDragScale(), true);
}

/**
 * @tc.name: SetDisableDragResize
 * @tc.desc: SetDisableDragResize
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetDisableDragResize, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetDisableDragResize(true);
    EXPECT_EQ(property->disableDragResize_, true);
    property->SetDisableDragResize(false);
    EXPECT_EQ(property->disableDragResize_, false);
}

/**
 * @tc.name: IsDragResizeDisabled
 * @tc.desc: IsDragResizeDisabled
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsDragResizeDisabled, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsDragResizeDisabled(), false);
    property->disableDragResize_ = true;
    EXPECT_EQ(property->IsDragResizeDisabled(), true);
}

/**
 * @tc.name: SetDisableResizeWithDpi
 * @tc.desc: SetDisableResizeWithDpi
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetDisableResizeWithDpi, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetDisableResizeWithDpi(true);
    EXPECT_EQ(property->disableResizeWithDpi_, true);
    property->SetDisableResizeWithDpi(false);
    EXPECT_EQ(property->disableResizeWithDpi_, false);
}

/**
 * @tc.name: IsResizeWithDpiDisabled
 * @tc.desc: IsResizeWithDpiDisabled
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsResizeWithDpiDisabled, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsResizeWithDpiDisabled(), false);
    property->disableResizeWithDpi_ = true;
    EXPECT_EQ(property->IsResizeWithDpiDisabled(), true);
}

/**
 * @tc.name: SetDisableFullScreen
 * @tc.desc: SetDisableFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetDisableFullScreen, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetDisableFullScreen(true);
    EXPECT_EQ(property->disableFullScreen_, true);
    property->SetDisableFullScreen(false);
    EXPECT_EQ(property->disableFullScreen_, false);
}

/**
 * @tc.name: IsFullScreenDisabled
 * @tc.desc: IsFullScreenDisabled
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsFullScreenDisabled, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsFullScreenDisabled(), false);
    property->disableFullScreen_ = true;
    EXPECT_EQ(property->IsFullScreenDisabled(), true);
}

/**
 * @tc.name: SetDisableWindowLimit
 * @tc.desc: SetDisableWindowLimit
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetDisableWindowLimit, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetDisableWindowLimit(true);
    EXPECT_EQ(property->disableWindowLimit_, true);
    property->SetDisableWindowLimit(false);
    EXPECT_EQ(property->disableWindowLimit_, false);
}

/**
 * @tc.name: IsWindowLimitDisabled
 * @tc.desc: IsWindowLimitDisabled
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsWindowLimitDisabled, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsWindowLimitDisabled(), false);
    property->disableWindowLimit_ = true;
    EXPECT_EQ(property->IsWindowLimitDisabled(), true);
}

/**
 * @tc.name: SetDisableDecorFullscreen
 * @tc.desc: SetDisableDecorFullscreen
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetDisableDecorFullscreen, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetDisableDecorFullscreen(true);
    EXPECT_EQ(property->disableDecorFullscreen_, true);
    property->SetDisableDecorFullscreen(false);
    EXPECT_EQ(property->disableDecorFullscreen_, false);
}
 
/**
 * @tc.name: IsDecorFullscreenDisabled
 * @tc.desc: IsDecorFullscreenDisabled
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsDecorFullscreenDisabled, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsDecorFullscreenDisabled(), false);
    property->disableDecorFullscreen_ = true;
    EXPECT_EQ(property->IsDecorFullscreenDisabled(), true);
}

/**
 * @tc.name: SetIsAdaptToSimulationScale
 * @tc.desc: SetIsAdaptToSimulationScale
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToSimulationScale, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToSimulationScale(true);
    EXPECT_EQ(property->isAdaptToSimulationScale_, true);
    property->SetIsAdaptToSimulationScale(false);
    EXPECT_EQ(property->isAdaptToSimulationScale_, false);
}

/**
 * @tc.name: IsAdaptToSimulationScale
 * @tc.desc: IsAdaptToSimulationScale
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToSimulationScale, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToSimulationScale(), false);
    property->isAdaptToSimulationScale_ = true;
    EXPECT_EQ(property->IsAdaptToSimulationScale(), true);
}

/**
 * @tc.name: SetIsFullScreenStart
 * @tc.desc: SetIsFullScreenStart
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsFullScreenStart, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsFullScreenStart(true);
    EXPECT_EQ(property->isFullScreenStart_, true);
    property->SetIsFullScreenStart(false);
    EXPECT_EQ(property->isFullScreenStart_, false);
}

/**
 * @tc.name: IsFullScreenStart
 * @tc.desc: IsFullScreenStart
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsFullScreenStart, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsFullScreenStart(), false);
    property->isFullScreenStart_ = true;
    EXPECT_EQ(property->IsFullScreenStart(), true);
}

/**
 * @tc.name: SetIsSupportRotateFullScreen
 * @tc.desc: SetIsSupportRotateFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsSupportRotateFullScreen, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsSupportRotateFullScreen(true);
    EXPECT_EQ(property->isSupportRotateFullScreen_, true);
    property->SetIsSupportRotateFullScreen(false);
    EXPECT_EQ(property->isSupportRotateFullScreen_, false);
}

/**
 * @tc.name: IsSupportRotateFullScreen
 * @tc.desc: IsSupportRotateFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsSupportRotateFullScreen, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsSupportRotateFullScreen(), false);
    property->isSupportRotateFullScreen_ = true;
    EXPECT_EQ(property->IsSupportRotateFullScreen(), true);
}

/**
 * @tc.name: SetIsAdaptToSubWindow
 * @tc.desc: SetIsAdaptToSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, SetIsAdaptToSubWindow, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->SetIsAdaptToSubWindow(true);
    EXPECT_EQ(property->isAdaptToSubWindow_, true);
    property->SetIsAdaptToSubWindow(false);
    EXPECT_EQ(property->isAdaptToSubWindow_, false);
}

/**
 * @tc.name: IsAdaptToSubWindow
 * @tc.desc: IsAdaptToSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModePropertyTest, IsAdaptToSubWindow, TestSize.Level1)
{
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_EQ(property->IsAdaptToSubWindow(), false);
    property->isAdaptToSubWindow_ = true;
    EXPECT_EQ(property->IsAdaptToSubWindow(), true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
