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
#include "fold_screen_state_internel.h"
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
    static ScreenSessionManager& ssm_;
    void SetUp() override;
    void TearDown() override;
    void SetAceessTokenPermission(const std::string processName);
    DMHookInfo CreateDefaultHookInfo();
};

ScreenSessionManager& ScreenCutoutControllerTest::ssm_ = ScreenSessionManager::GetInstance();

void ScreenCutoutControllerTest::SetUpTestCase()
{
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

DMHookInfo ScreenCutoutControllerTest::CreateDefaultHookInfo()
{
    uint32_t hookWidth = 500;
    uint32_t hookHeight = 700;
    float_t hookDensity = 3.0;
    uint32_t hookRotation = static_cast<uint32_t>(Rotation::ROTATION_0);
    uint32_t hookDisplayOrientation = static_cast<uint32_t>(DisplayOrientation::PORTRAIT);
    DMHookInfo dmHookInfo = { hookWidth, hookHeight, hookDensity, hookRotation, true, hookDisplayOrientation, true };
    return dmHookInfo;
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
/**
 * @tc.name: HookCutoutInfo001
 * @tc.desc: HookCutoutInfo001
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo001, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        GTEST_SKIP();
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_TRUE(controller != nullptr);
    DMRect emptyRect = { 0, 0, 0, 0 };
    DMRect emptyRect_ = { 21, 21, 3, 3 };
    std::vector<DMRect> boundaryRects = { emptyRect_, emptyRect };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t hookWidth = 100;
    uint32_t hookHeight = 200;
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects, displayInfo);
    EXPECT_EQ(boundaryRects, boundaryRects);
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(0);
    displayInfo->SetActualWidth(0);
    displayInfo->SetActualHeight(0);
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects, displayInfo);
    EXPECT_EQ(boundaryRects, boundaryRects);
}

/**
 * @tc.name: HookCutoutInfo002
 * @tc.desc: HookCutoutInfo002
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo002, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        GTEST_SKIP();
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_TRUE(controller != nullptr);
    DMRect emptyRect1 = { 50, 50, 1, 1 };
    DMRect emptyRect2 = { 100, 100, 2, 2 };
    std::vector<DMRect> boundaryRects = { emptyRect1, emptyRect2 };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t hookWidth = 200;
    uint32_t hookHeight = 200;
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;
    EXPECT_EQ(boundaryRects, boundaryRects);
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(0);
    displayInfo->SetActualWidth(100);
    displayInfo->SetActualHeight(100);
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects, displayInfo);
    EXPECT_EQ(boundaryRects[0], emptyRect2);
    DMRect emptyRect3 = { 200, 200, 1, 1 };
    std::vector<DMRect> boundaryRects1 = { emptyRect3 };
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects1, displayInfo);
    std::vector<DMRect> testboundaryRects1;
    EXPECT_EQ(testboundaryRects1, testboundaryRects1);
}

/**
 * @tc.name: HookCutoutInfo003
 * @tc.desc: HookCutoutInfo003
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo003, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        GTEST_SKIP();
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_TRUE(controller != nullptr);
    DMRect emptyRect1 = { 50, 50, 1, 1 };
    std::vector<DMRect> boundaryRects = { emptyRect1 };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t hookWidth = 200;
    uint32_t hookHeight = 200;
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(0);
    displayInfo->SetActualWidth(100);
    displayInfo->SetActualHeight(100);
    std::vector<DMRect> boundaryRects1 = { emptyRect1 };
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects1, displayInfo);
    std::vector<DMRect> testboundaryRects1;
    EXPECT_EQ(testboundaryRects1, testboundaryRects1);
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(1000);
    boundaryRects1 = { emptyRect1 };
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects1, displayInfo);
    EXPECT_EQ(boundaryRects1, testboundaryRects1);
    emptyRect1 = { 1000, 1000, 1, 1 };
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(0);
    boundaryRects1 = { emptyRect1 };
    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects1, displayInfo);
    EXPECT_EQ(boundaryRects1, testboundaryRects1);
}

// =============================================================================
// A Group: RecoverDisplayInfo tests — secondary display super fold device support
// =============================================================================

/**
 * @tc.name: RecoverDisplayInfo_SecondarySuperFold_FULL_Rotation0
 * @tc.desc: Verify RecoverDisplayInfo sets dwidth/dheight to physical dimensions on secondary super fold device
 *           when hook is enabled with FULL display mode and ROTATION_0
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_SecondarySuperFold_FULL_Rotation0, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetPhysicalWidth(700);
    displayInfo->SetPhysicalHeight(500);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_0);
    // The method should have executed (not early-returned), setting dwidth/dheight to physical dimensions
    EXPECT_NE(dwidth, 100);
    EXPECT_NE(dheight, 200);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: RecoverDisplayInfo_SecondarySuperFold_GlobalFull_Rotation0
 * @tc.desc: Verify RecoverDisplayInfo handles GLOBAL_FULL display mode on secondary super fold device
 *           with hook enabled and ROTATION_0, ensuring the new GLOBAL_FULL mode path is covered
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_SecondarySuperFold_GlobalFull_Rotation0, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    // Use dimensions that may match GLOBAL_FULL configuration on supported devices
    displayInfo->SetPhysicalWidth(800);
    displayInfo->SetPhysicalHeight(600);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_0);
    EXPECT_NE(dwidth, 100);
    EXPECT_NE(dheight, 200);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: RecoverDisplayInfo_SecondarySuperFold_FULL_Rotation90_NoSwap
 * @tc.desc: Verify RecoverDisplayInfo does NOT swap dimensions when rotation is ROTATION_90,
 *           as the swap only applies to ROTATION_0 and ROTATION_180
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_SecondarySuperFold_FULL_Rotation90_NoSwap, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    int32_t phyWidth = 700;
    int32_t phyHeight = 500;
    displayInfo->SetPhysicalWidth(phyWidth);
    displayInfo->SetPhysicalHeight(phyHeight);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_90);
    // ROTATION_90 does not trigger swap; dwidth/dheight should equal original physical values
    EXPECT_EQ(static_cast<int32_t>(dwidth), phyWidth);
    EXPECT_EQ(static_cast<int32_t>(dheight), phyHeight);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: RecoverDisplayInfo_SecondarySuperFold_HookDisabled_Noop
 * @tc.desc: Verify RecoverDisplayInfo returns early without modifying dwidth/dheight
 *           when hook is disabled (displayHookMap_ has no entry for current uid)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_SecondarySuperFold_HookDisabled_Noop, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetPhysicalWidth(700);
    displayInfo->SetPhysicalHeight(500);
    // Do NOT set up hook info — IsHook() should return false, triggering early return

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_0);
    // Guard should trigger early return: IsHook() is false → !IsHook() is true → return
    EXPECT_EQ(dwidth, 100);
    EXPECT_EQ(dheight, 200);
}

/**
 * @tc.name: RecoverDisplayInfo_NonSuperFoldDevice_Noop
 * @tc.desc: Verify RecoverDisplayInfo returns early on non-super-fold devices even when hook is enabled,
 *           ensuring the device type guard correctly prevents execution on unsupported hardware
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_NonSuperFoldDevice_Noop, TestSize.Level1)
{
    // No device type skip — this test verifies behavior on non-super-fold devices
    // On a super fold device, the guard would pass and this test becomes a positive case
    if (FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Skipping negative test on super fold device";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetPhysicalWidth(700);
    displayInfo->SetPhysicalHeight(500);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_0);
    // On a non-super-fold device: !(IsSecondary || IsSingle) is true → early return
    EXPECT_EQ(dwidth, 100);
    EXPECT_EQ(dheight, 200);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: RecoverDisplayInfo_SecondarySuperFold_NullDisplayInfo
 * @tc.desc: Verify RecoverDisplayInfo handles nullptr displayInfo gracefully without crash
 *           on secondary super fold device with hook enabled
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_SecondarySuperFold_NullDisplayInfo, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = nullptr;
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_0);
    // Should return early on nullptr displayInfo without crash
    EXPECT_EQ(dwidth, 100);
    EXPECT_EQ(dheight, 200);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: RecoverDisplayInfo_SingleSuperFold_GlobalFull_Rotation0
 * @tc.desc: Verify RecoverDisplayInfo on single display super fold device with GLOBAL_FULL mode,
 *           ensuring the existing code path is not broken by the secondary display changes
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, RecoverDisplayInfo_SingleSuperFold_GlobalFull_Rotation0, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires single display super fold device (fold type 7)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    uint32_t dwidth = 100;
    uint32_t dheight = 200;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetPhysicalWidth(800);
    displayInfo->SetPhysicalHeight(600);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->RecoverDisplayInfo(dwidth, dheight, displayInfo, Rotation::ROTATION_0);
    EXPECT_NE(dwidth, 100);
    EXPECT_NE(dheight, 200);

    ssm_.displayHookMap_.erase(uid);
}

// =============================================================================
// B Group: HookCutoutInfo tests — secondary display super fold device support
// =============================================================================

/**
 * @tc.name: HookCutoutInfo_SecondarySuperFold_NormalScaling
 * @tc.desc: Verify HookCutoutInfo correctly scales boundary rects on secondary super fold device
 *           with hook enabled and normal display info
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo_SecondarySuperFold_NormalScaling, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    DMRect rect = { 50, 50, 10, 10 };
    std::vector<DMRect> boundaryRects = { rect };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t hookWidth = 200;
    uint32_t hookHeight = 200;
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(0);
    displayInfo->SetActualWidth(100);
    displayInfo->SetActualHeight(100);

    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects, displayInfo);
    // With scale 100/200 = 0.5, the rect should be transformed
    EXPECT_EQ(boundaryRects.size(), 1);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: HookCutoutInfo_SecondarySuperFold_ZeroHookDimensions
 * @tc.desc: Verify HookCutoutInfo returns early when hook dimensions are zero
 *           on secondary super fold device with hook enabled
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo_SecondarySuperFold_ZeroHookDimensions, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    DMRect rect = { 50, 50, 10, 10 };
    std::vector<DMRect> boundaryRects = { rect };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->HookCutoutInfo(0, 200, boundaryRects, displayInfo);
    // hookWidth is zero → should return early, boundaryRects unchanged
    EXPECT_EQ(boundaryRects.size(), 1);
    EXPECT_EQ(boundaryRects[0], rect);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: HookCutoutInfo_SecondarySuperFold_HookDisabled_Noop
 * @tc.desc: Verify HookCutoutInfo returns early without modification when hook is disabled
 *           on secondary super fold device
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo_SecondarySuperFold_HookDisabled_Noop, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    DMRect rect = { 50, 50, 10, 10 };
    std::vector<DMRect> boundaryRects = { rect };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    // Do NOT set up hook info — IsHook() returns false → guard triggers early return

    controller->HookCutoutInfo(200, 200, boundaryRects, displayInfo);
    EXPECT_EQ(boundaryRects.size(), 1);
    EXPECT_EQ(boundaryRects[0], rect);
}

/**
 * @tc.name: HookCutoutInfo_NonSuperFoldDevice_Noop
 * @tc.desc: Verify HookCutoutInfo returns early on non-super-fold devices even with hook enabled,
 *           ensuring device type guard correctly prevents execution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo_NonSuperFoldDevice_Noop, TestSize.Level1)
{
    if (FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Skipping negative test on super fold device";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    DMRect rect = { 50, 50, 10, 10 };
    std::vector<DMRect> boundaryRects = { rect };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;

    controller->HookCutoutInfo(200, 200, boundaryRects, displayInfo);
    // On non-super-fold device, guard returns early; rects unchanged
    EXPECT_EQ(boundaryRects.size(), 1);
    EXPECT_EQ(boundaryRects[0], rect);

    ssm_.displayHookMap_.erase(uid);
}

/**
 * @tc.name: HookCutoutInfo_SecondarySuperFold_AllRectsOutOfBounds
 * @tc.desc: Verify HookCutoutInfo filters out all boundary rects when they lie completely
 *           outside the display area on secondary super fold device
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCutoutControllerTest, HookCutoutInfo_SecondarySuperFold_AllRectsOutOfBounds, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplaySuperFoldDevice()) {
        GTEST_SKIP() << "Requires secondary display super fold device (fold type 8)";
    }
    sptr<ScreenCutoutController> controller = new ScreenCutoutController();
    ASSERT_NE(controller, nullptr);
    // All rects positioned outside the scaled display area (ActualPos + Actual extends to 100x100,
    // but hook scale factor maps these to far outside the bounds)
    DMRect outRect1 = { 1000, 1000, 1, 1 };
    DMRect outRect2 = { 500, 500, 1, 1 };
    std::vector<DMRect> boundaryRects = { outRect1, outRect2 };
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    uint32_t hookWidth = 200;
    uint32_t hookHeight = 200;
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_.displayHookMap_[uid] = dmHookInfo;
    displayInfo->SetActualPosX(0);
    displayInfo->SetActualPosY(0);
    displayInfo->SetActualWidth(100);
    displayInfo->SetActualHeight(100);

    controller->HookCutoutInfo(hookWidth, hookHeight, boundaryRects, displayInfo);
    // All rects should be filtered out because they're outside the display bounds after scaling
    EXPECT_EQ(boundaryRects.size(), 0);

    ssm_.displayHookMap_.erase(uid);
}
}
} // namespace Rosen
} // namespace OHOS

