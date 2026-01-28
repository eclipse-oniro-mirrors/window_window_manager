/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "interfaces/include/ws_common.h"
#include "screen_scene_config.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "window_manager_agent.h"
#include "session_manager.h"
#include "screen_cutout_controller.h"
#include "zidl/window_manager_agent_interface.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "screen_scene_config.h"
#include "common_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 100000;
}

class ScreenCutoutControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void SetAceessTokenPermission(const std::string processName);
};

void ScreenCutoutControllerTest::SetUpTestCase()
{
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void ScreenCutoutControllerTest::TearDownTestCase()
{
}

void ScreenCutoutControllerTest::SetUp()
{
}

void ScreenCutoutControllerTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: GetCutoutArea
 * @tc.desc: GetCutoutArea func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, GetCutoutArea, TestSize.Level1)
{
    uint32_t width = 1276;
    uint32_t height = 2848;
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();

    std::string svgPath = "M600 44 L676 44 v 76 h -76 Z";
    ScreenSceneConfig::SetSubCutoutSvgPath(svgPath);
    std::vector<DMRect> cutoutRects;
    controller->GetCutoutArea(0, width, height, Rotation::ROTATION_0, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 0);
    controller->GetCutoutArea(0, width, height, Rotation::ROTATION_90, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 0);
    controller->GetCutoutArea(0, width, height, Rotation::ROTATION_180, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 0);
    controller->GetCutoutArea(0, width, height, Rotation::ROTATION_270, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 0);
}

/**
 * @tc.name: CalcCutoutRects
 * @tc.desc: CalcCutoutRects func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, CalcCutoutRects, TestSize.Level1)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    uint32_t width = 2224;
    uint32_t height = 2496;
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();

    std::vector<DMRect> boundaryRects = {};
    boundaryRects.emplace_back(DMRect {2109, 28, 69, 69});

    std::vector<DMRect> cutoutRects;
    controller->CalcCutoutRects(boundaryRects, width, height, Rotation::ROTATION_0, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 1);
    emptyRect = { 2109, 28, 69, 69 };
    EXPECT_EQ(cutoutRects[0], emptyRect);
    
    cutoutRects.clear();
    controller->CalcCutoutRects(boundaryRects, height, width, Rotation::ROTATION_90, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 1);
    emptyRect = { 2399, 2109, 69, 69 };
    EXPECT_EQ(cutoutRects[0], emptyRect);
    
    cutoutRects.clear();
    controller->CalcCutoutRects(boundaryRects, width, height, Rotation::ROTATION_180, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 1);
    emptyRect = { 46, 2399, 69, 69 };
    EXPECT_EQ(cutoutRects[0], emptyRect);
    
    cutoutRects.clear();
    controller->CalcCutoutRects(boundaryRects, height, width, Rotation::ROTATION_270, cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 1);
    emptyRect = { 28, 46, 69, 69 };
    EXPECT_EQ(cutoutRects[0], emptyRect);

    cutoutRects.clear();
    controller->CalcCutoutRects(boundaryRects, height, width, static_cast<Rotation>(5), cutoutRects);
    EXPECT_EQ(cutoutRects.size(), 0);
}

/**
 * @tc.name: GetWaterfallArea
 * @tc.desc: GetWaterfallArea func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, GetWaterfallArea, TestSize.Level1)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    uint32_t width = 1276;
    uint32_t height = 2848;
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();

    ScreenSceneConfig::isWaterfallDisplay_ = true;
    WaterfallDisplayAreaRects waterfallArea;
    controller->GetWaterfallArea(width, height, Rotation::ROTATION_0, waterfallArea);
    EXPECT_EQ(waterfallArea.left, emptyRect);

    ScreenSceneConfig::intNumbersConfig_["curvedScreenBoundary"] = { 0, 0, 0, 0 };
    controller->GetWaterfallArea(width, height, Rotation::ROTATION_0, waterfallArea);
    EXPECT_EQ(waterfallArea.left, emptyRect);
}

/**
 * @tc.name: CalcCutoutRects
 * @tc.desc: CalcCutoutRects func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, CalcWaterfallRects0, TestSize.Level1)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    const std::vector numberVec = { 0, 0, 0, 0 };
    uint32_t width = 1276;
    uint32_t height = 2848;
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();
    WaterfallDisplayAreaRects waterfallArea;

    controller->CalcWaterfallRects(numberVec, width, height, Rotation::ROTATION_0, waterfallArea);
    EXPECT_EQ(waterfallArea.left, emptyRect);
    EXPECT_EQ(waterfallArea.top, emptyRect);
    EXPECT_EQ(waterfallArea.right, emptyRect);
    EXPECT_EQ(waterfallArea.bottom, emptyRect);
}

/**
 * @tc.name: CalcCutoutRects
 * @tc.desc: CalcCutoutRects func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, CalcWaterfallRects1, TestSize.Level1)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    const std::vector numberVec = { 1276, 0, 0, 0 };
    uint32_t width = 1276;
    uint32_t height = 2848;
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();
    WaterfallDisplayAreaRects waterfallArea;

    controller->CalcWaterfallRects(numberVec, width, height, Rotation::ROTATION_0, waterfallArea);
    EXPECT_EQ(waterfallArea.left, emptyRect);
    EXPECT_EQ(waterfallArea.top, emptyRect);
    EXPECT_EQ(waterfallArea.right, emptyRect);
    EXPECT_EQ(waterfallArea.bottom, emptyRect);
}

/**
 * @tc.name: CalcCutoutRects
 * @tc.desc: CalcCutoutRects func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, CalcWaterfallRects2, TestSize.Level1)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    const std::vector numberVec = { 1, 2, 1, 2 };
    uint32_t width = 1276;
    uint32_t height = 2848;
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();
    WaterfallDisplayAreaRects waterfallArea;

    controller->CalcWaterfallRects(numberVec, width, height, Rotation::ROTATION_0, waterfallArea);
    emptyRect = { 0, 0, 1, height };
    EXPECT_EQ(waterfallArea.left, emptyRect);
    emptyRect = { 0, 0, width, 2 };
    EXPECT_EQ(waterfallArea.top, emptyRect);
    emptyRect = { width - 1, 0, 1, height };
    EXPECT_EQ(waterfallArea.right, emptyRect);
    emptyRect = { 0, height - 2, width, 2 };
    EXPECT_EQ(waterfallArea.bottom, emptyRect);

    controller->CalcWaterfallRects(numberVec, width, height, Rotation::ROTATION_180, waterfallArea);
    emptyRect = { 0, 0, 1, height };
    EXPECT_EQ(waterfallArea.left, emptyRect);
    emptyRect = { 0, 0, width, 2 };
    EXPECT_EQ(waterfallArea.top, emptyRect);
    emptyRect = { width - 1, 0, 1, height };
    EXPECT_EQ(waterfallArea.right, emptyRect);
    emptyRect = { 0, height - 2, width, 2 };
    EXPECT_EQ(waterfallArea.bottom, emptyRect);

    std::swap(width, height);
    controller->CalcWaterfallRects(numberVec, width, height, Rotation::ROTATION_90, waterfallArea);
    emptyRect = { 0, 0, 2, height };
    EXPECT_EQ(waterfallArea.left, emptyRect);
    emptyRect = { 0, 0, width, 1 };
    EXPECT_EQ(waterfallArea.top, emptyRect);
    emptyRect = { width - 2, 0, 2, height };
    EXPECT_EQ(waterfallArea.right, emptyRect);
    emptyRect = { 0, height - 1, width, 1 };
    EXPECT_EQ(waterfallArea.bottom, emptyRect);

    controller->CalcWaterfallRects(numberVec, width, height, Rotation::ROTATION_270, waterfallArea);
    emptyRect = { 0, 0, 2, height };
    EXPECT_EQ(waterfallArea.left, emptyRect);
    emptyRect = { 0, 0, width, 1 };
    EXPECT_EQ(waterfallArea.top, emptyRect);
    emptyRect = { width - 2, 0, 2, height };
    EXPECT_EQ(waterfallArea.right, emptyRect);
    emptyRect = { 0, height - 1, width, 1 };
    EXPECT_EQ(waterfallArea.bottom, emptyRect);

    emptyRect = { 0, 0, 0, 0 };
    waterfallArea.left = emptyRect;
    controller->CalcWaterfallRects(numberVec, width, height, static_cast<Rotation>(5), waterfallArea);
    EXPECT_EQ(waterfallArea.left, emptyRect);
}

/**
 * @tc.name: InitRect
 * @tc.desc: InitRect func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, InitRect, TestSize.Level1)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    sptr<ScreenCutoutController> controller = sptr<ScreenCutoutController>::MakeSptr();

    controller->InitRect(100, 100, 0, 100, emptyRect);
    EXPECT_EQ(emptyRect, DMRect::NONE());
    controller->InitRect(100, 100, 100, 0, emptyRect);
    EXPECT_EQ(emptyRect, DMRect::NONE());
    controller->InitRect(100, 100, 100, 100, emptyRect);
    EXPECT_EQ(emptyRect.posX_, 100);
    EXPECT_EQ(emptyRect.posY_, 100);
    EXPECT_EQ(emptyRect.width_, 100);
    EXPECT_EQ(emptyRect.height_, 100);
}

/**
 * @tc.name: CheckBoundaryRectsWithRotation01
 * @tc.desc: ScreenCutoutController check boundary rects
 * @tc.type: FUNC
*/
HWTEST_F(ScreenCutoutControllerTest, CheckBoundaryRectsWithRotation01, TestSize.Level1)
{
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    DMRect emptyRect = {-15, -15, 8, 8};
    DMRect emptyRect_ = {21, 21, 3, 3};
    std::vector<DMRect> boundaryRects = {emptyRect_, emptyRect};
    ScreenProperty screenProperty;
    auto screenBouns = RRect({0, 0, 35, 35}, 0.0f, 0.0f);
    screenProperty.SetBounds(screenBouns);
    ASSERT_TRUE(controller != nullptr);
    controller->CheckBoundaryRectsWithRotation(boundaryRects, 35, 35, Rotation::ROTATION_0);
    ASSERT_EQ(boundaryRects.size(), 1);
    EXPECT_EQ(boundaryRects[0], emptyRect_);
}

/**
 * @tc.name: CheckBoundaryRectsWithRotation02
 * @tc.desc: ScreenCutoutController check boundary rects
 * @tc.type: FUNC
*/
HWTEST_F(ScreenCutoutControllerTest, CheckBoundaryRectsWithRotation02, TestSize.Level1)
{
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    DMRect emptyRect = {1, 1, 7, 7};
    DMRect emptyRect_ = {21, 21, 3, 3};
    std::vector<DMRect> boundaryRects = {emptyRect_, emptyRect};
    ScreenProperty screenProperty;
    auto screenBouns = RRect({0, 0, 35, 35}, 0.0f, 0.0f);
    screenProperty.SetBounds(screenBouns);
    ASSERT_TRUE(controller != nullptr);
    controller->CheckBoundaryRectsWithRotation(boundaryRects, 35, 35, Rotation::ROTATION_0);
    ASSERT_EQ(boundaryRects.size(), 2);
    EXPECT_EQ(boundaryRects[0], emptyRect_);
}

/**
 * @tc.name: CheckBoundaryRectsWithRotation03
 * @tc.desc: ScreenCutoutController check boundary rects
 * @tc.type: FUNC
*/
HWTEST_F(ScreenCutoutControllerTest, CheckBoundaryRectsWithRotation03, TestSize.Level1)
{
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    DMRect emptyRect = {1, 1, 7, 7};
    std::vector<DMRect> boundaryRects = {emptyRect};
    ScreenProperty screenProperty;
    auto screenBouns = RRect({0, 0, 6, 6}, 0.0f, 0.0f);
    screenProperty.SetBounds(screenBouns);
    ASSERT_TRUE(controller != nullptr);
    controller->CheckBoundaryRectsWithRotation(boundaryRects, 6, 6, Rotation::ROTATION_0);
    ASSERT_EQ(boundaryRects.size(), 0);
}

/**
 * @tc.name: CheckBoundaryRectsWithRotation04
 * @tc.desc: ScreenCutoutController check boundary rects
 * @tc.type: FUNC
*/
HWTEST_F(ScreenCutoutControllerTest, CheckBoundaryRectsWithRotation04, TestSize.Level1)
{
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    DMRect emptyRect = {1, 0, 8, 8};
    DMRect emptyRect_ = {21, 21, 3, 3};
    std::vector<DMRect> boundaryRects = {emptyRect_, emptyRect};
    ScreenProperty screenProperty;
    auto screenBouns = RRect({0, 0, 25, 23}, 0.0f, 0.0f);
    screenProperty.SetBounds(screenBouns);
    ASSERT_TRUE(controller != nullptr);
    controller->CheckBoundaryRectsWithRotation(boundaryRects, 25, 23, Rotation::ROTATION_0);
    ASSERT_EQ(boundaryRects.size(), 1);
    EXPECT_EQ(boundaryRects[0], emptyRect);
}

/**
 * @tc.name: CheckBoundaryRectsWithRotation05
 * @tc.desc: ScreenCutoutController check boundary rects
 * @tc.type: FUNC
*/
HWTEST_F(ScreenCutoutControllerTest, CheckBoundaryRectsWithRotation05, TestSize.Level1)
{
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    DMRect emptyRect = {0, 0, 0, 0};
    DMRect emptyRect_ = {21, 21, 3, 3};
    std::vector<DMRect> boundaryRects = {emptyRect_, emptyRect};
    ScreenProperty screenProperty;
    auto screenBouns = RRect({0, 0, 35, 35}, 0.0f, 0.0f);
    screenProperty.SetBounds(screenBouns);
    ASSERT_TRUE(controller != nullptr);
    controller->CheckBoundaryRectsWithRotation(boundaryRects, 35, 35, Rotation::ROTATION_0);
    ASSERT_EQ(boundaryRects.size(), 1);
    EXPECT_EQ(boundaryRects[0], emptyRect_);
}
}
} // namespace Rosen
} // namespace OHOS

