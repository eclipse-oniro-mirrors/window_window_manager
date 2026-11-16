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

#include "window_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void WindowHelperTest::SetUpTestCase() {}

void WindowHelperTest::TearDownTestCase() {}

void WindowHelperTest::SetUp() {}

void WindowHelperTest::TearDown() {}

namespace {
/**
 * @tc.name: WindowTypeWindowMode
 * @tc.desc: window type/mode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, WindowTypeWindowMode, TestSize.Level1)
{
    ASSERT_EQ(true,
              WindowHelper::IsMainFullScreenWindow(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
                                                   WindowMode::WINDOW_MODE_FULLSCREEN));
    ASSERT_EQ(false,
              WindowHelper::IsMainFullScreenWindow(WindowType::WINDOW_TYPE_APP_SUB_WINDOW,
                                                   WindowMode::WINDOW_MODE_FULLSCREEN));
    ASSERT_EQ(false,
              WindowHelper::IsMainFullScreenWindow(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
                                                   WindowMode::WINDOW_MODE_SPLIT_PRIMARY));

    ASSERT_EQ(true, WindowHelper::IsFloatingWindow(WindowMode::WINDOW_MODE_FLOATING));
    ASSERT_EQ(false, WindowHelper::IsFloatingWindow(WindowMode::WINDOW_MODE_FULLSCREEN));

    ASSERT_EQ(true, WindowHelper::IsFullScreenWindow(WindowMode::WINDOW_MODE_FULLSCREEN));
    ASSERT_EQ(false, WindowHelper::IsFullScreenWindow(WindowMode::WINDOW_MODE_FLOATING));

    ASSERT_EQ(true, WindowHelper::IsDialogWindow(WindowType::WINDOW_TYPE_DIALOG));
    ASSERT_EQ(false, WindowHelper::IsDialogWindow(WindowType::WINDOW_TYPE_APP_SUB_WINDOW));
}

/**
 * @tc.name: WindowModeSupport
 * @tc.desc: window mode supported test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, WindowModeSupport, TestSize.Level1)
{
    ASSERT_EQ(true,
              WindowHelper::IsWindowModeSupported(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL,
                                                  WindowMode::WINDOW_MODE_FULLSCREEN));
    ASSERT_EQ(true,
              WindowHelper::IsWindowModeSupported(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL,
                                                  WindowMode::WINDOW_MODE_FLOATING));
    ASSERT_EQ(true,
              WindowHelper::IsWindowModeSupported(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL,
                                                  WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    ASSERT_EQ(true,
              WindowHelper::IsWindowModeSupported(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL,
                                                  WindowMode::WINDOW_MODE_SPLIT_SECONDARY));
    ASSERT_EQ(
        true,
        WindowHelper::IsWindowModeSupported(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL, WindowMode::WINDOW_MODE_PIP));
    ASSERT_EQ(false,
              WindowHelper::IsWindowModeSupported(WindowModeSupport::WINDOW_MODE_SUPPORT_ALL,
                                                  WindowMode::WINDOW_MODE_UNDEFINED));
}

/**
 * @tc.name: WindowRect
 * @tc.desc: rect test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, WindowRect, TestSize.Level1)
{
    Rect rect0 = { 0, 0, 0, 0 };
    ASSERT_EQ(true, WindowHelper::IsEmptyRect(rect0));

    Rect rect1 = { 0, 0, 1, 1 };
    ASSERT_EQ(false, WindowHelper::IsEmptyRect(rect1));
}

/**
 * @tc.name: WindowStringUtil
 * @tc.desc: string test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, WindowStringUtil, TestSize.Level1)
{
    ASSERT_EQ(true, WindowHelper::IsNumber("123"));
    ASSERT_EQ(false, WindowHelper::IsNumber("1a3"));
    ASSERT_EQ(false, WindowHelper::IsNumber(""));

    ASSERT_EQ(true, WindowHelper::IsFloatingNumber("1.23"));
    ASSERT_EQ(true, WindowHelper::IsFloatingNumber(".123"));
    ASSERT_EQ(false, WindowHelper::IsFloatingNumber("1a3"));
    ASSERT_EQ(false, WindowHelper::IsFloatingNumber("123.."));

    std::vector<std::string> vec = WindowHelper::Split("123a123a123", "a");
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].compare("123")) {
            ASSERT_TRUE(false);
        }
    }
    ASSERT_TRUE(true);
}

/**
 * @tc.name: CalculateOriginPosition
 * @tc.desc: CalculateOriginPosition test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, CalculateOriginPosition, TestSize.Level1)
{
    Rect rect1{ 0, 0, 10, 10 }, rect2{ 100, 100, 200, 200 };
    PointInfo point = WindowHelper::CalculateOriginPosition(rect1, rect2, PointInfo{ 200, 200 });
    PointInfo expectPoint{ 5, 5 };
    ASSERT_EQ(true, point.x == expectPoint.x);
    ASSERT_EQ(true, point.y == expectPoint.y);

    Transform transform;
    transform.scaleX_ = 0.66f;
    transform.scaleY_ = 1.5f;
    transform.rotationY_ = 30;
    transform.translateX_ = 100;
    transform.translateY_ = 200;
    transform.translateZ_ = 50;
    Rect rect{ 50, 50, 240, 320 };
    TransformHelper::Vector3 pivotPos = { rect.posX_ + transform.pivotX_ * rect.width_,
                                          rect.posY_ + transform.pivotY_ * rect.height_,
                                          0 };
    TransformHelper::Matrix4 mat = TransformHelper::CreateTranslation(-pivotPos);
    mat *= WindowHelper::ComputeWorldTransformMat4(transform);
    mat *= TransformHelper::CreateTranslation(pivotPos);

    TransformHelper::Vector3 expectOriginPoint(0, 0, 0);
    TransformHelper::Vector3 tranformedPoint = TransformHelper::Transform(expectOriginPoint, mat);
    PointInfo actialOriginPoint = WindowHelper::CalculateOriginPosition(
        mat, { static_cast<int32_t>(tranformedPoint.x_), static_cast<int32_t>(tranformedPoint.y_) });
    const float errorRange = 2.f;
    ASSERT_LT(std::abs(expectOriginPoint.x_ - actialOriginPoint.x), errorRange);
    ASSERT_LT(std::abs(expectOriginPoint.y_ - actialOriginPoint.y), errorRange);
}

/**
 * @tc.name: TransformRect
 * @tc.desc: TransformRect test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, TransformRect, TestSize.Level1)
{
    Transform transform;
    Rect rect{ 0, 0, 10, 20 };
    transform.scaleX_ = transform.scaleY_ = 2.0f;
    TransformHelper::Vector3 pivotPos = { rect.posX_ + transform.pivotX_ * rect.width_,
                                          rect.posY_ + transform.pivotY_ * rect.height_,
                                          0 };
    TransformHelper::Matrix4 mat = TransformHelper::CreateTranslation(-pivotPos);
    mat *= WindowHelper::ComputeWorldTransformMat4(transform);
    mat *= TransformHelper::CreateTranslation(pivotPos);
    Rect transformRect = WindowHelper::TransformRect(mat, rect);
    ASSERT_EQ(rect.width_ * transform.scaleX_, transformRect.width_);
    ASSERT_EQ(rect.height_ * transform.scaleY_, transformRect.height_);
}

/**
 * @tc.name: CalculateHotZoneScale
 * @tc.desc: CalculateHotZoneScale test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, CalculateHotZoneScale, TestSize.Level1)
{
    Transform transform;
    transform.scaleX_ = 0.66f;
    transform.scaleY_ = 1.5f;
    transform.pivotX_ = transform.pivotY_ = 0.5f;
    Rect rect{ -1, -2, 2, 4 };
    TransformHelper::Vector3 pivotPos = { rect.posX_ + transform.pivotX_ * rect.width_,
                                          rect.posY_ + transform.pivotY_ * rect.height_,
                                          0 };
    TransformHelper::Matrix4 mat = TransformHelper::CreateTranslation(-pivotPos);
    mat *= WindowHelper::ComputeWorldTransformMat4(transform);
    mat *= TransformHelper::CreateTranslation(pivotPos);

    const float errorRange = 0.01f;
    TransformHelper::Vector2 hotZoneScale = WindowHelper::CalculateHotZoneScale(mat);
    ASSERT_LT(std::abs(transform.scaleX_ - hotZoneScale.x_), errorRange);
    ASSERT_LT(std::abs(transform.scaleY_ - hotZoneScale.y_), errorRange);
}

/**
 * @tc.name: WindowType
 * @tc.desc: Window Type test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, WindowType, TestSize.Level1)
{
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW));
    ASSERT_EQ(true, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_APP_SUB_WINDOW));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_FLOAT));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_FLOAT_NAVIGATION));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_FLOAT_NAVIGATION));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_DYNAMIC));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_DYNAMIC));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_MAGNIFICATION));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_MAGNIFICATION));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_MAGNIFICATION_MENU));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_MAGNIFICATION_MENU));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_SELECTION));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_SELECTION));
    ASSERT_EQ(false, WindowHelper::IsSystemSubWindow(WindowType::WINDOW_TYPE_ACCESSIBILITY_CALL_HELPER));
    ASSERT_EQ(true, WindowHelper::IsSystemWindow(WindowType::WINDOW_TYPE_ACCESSIBILITY_CALL_HELPER));
}

/**
 * @tc.name: GetTransformFromWorldMat4
 * @tc.desc: GetTransformFromWorldMat4 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, GetTransformFromWorldMat4, TestSize.Level1)
{
    Transform transform1;
    transform1.scaleX_ = 0.66f;
    transform1.scaleY_ = 1.5f;
    transform1.translateX_ = 12.f;
    transform1.translateY_ = 45.f;
    Rect rect1{ 0, 0, 300, 400 };
    TransformHelper::Vector3 pivotPos1 = { rect1.posX_ + transform1.pivotX_ * rect1.width_,
                                           rect1.posY_ + transform1.pivotY_ * rect1.height_,
                                           0 };
    TransformHelper::Matrix4 mat1 = TransformHelper::CreateTranslation(-pivotPos1);
    mat1 *= WindowHelper::ComputeWorldTransformMat4(transform1);
    mat1 *= TransformHelper::CreateTranslation(pivotPos1);

    Rect rect2 = WindowHelper::TransformRect(mat1, rect1);
    Transform transform2;
    WindowHelper::GetTransformFromWorldMat4(mat1, rect2, transform2);
    TransformHelper::Vector3 pivotPos2 = { rect2.posX_ + transform2.pivotX_ * rect2.width_,
                                           rect2.posY_ + transform2.pivotY_ * rect2.height_,
                                           0 };
    TransformHelper::Matrix4 mat2 = TransformHelper::CreateTranslation(-pivotPos2);
    mat2 *= WindowHelper::ComputeWorldTransformMat4(transform2);
    mat2 *= TransformHelper::CreateTranslation(pivotPos2);
    for (int i = 0; i < TransformHelper::Matrix4::MAT_SIZE; i++) {
        for (int j = 0; j < TransformHelper::Matrix4::MAT_SIZE; j++) {
            ASSERT_EQ(true, MathHelper::NearZero(mat1.mat_[i][j] - mat2.mat_[i][j]));
        }
    }
}

/**
 * @tc.name: CheckButtonStyleValid
 * @tc.desc: CheckButtonStyleValid test
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, CheckButtonStyleValid, TestSize.Level1)
{
    DecorButtonStyle style;
    // checkout the side of buttonBackgroundSize
    style.buttonBackgroundSize = MIN_BUTTON_BACKGROUND_SIZE - 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonBackgroundSize = MAX_BUTTON_BACKGROUND_SIZE + 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonBackgroundSize = MIN_BUTTON_BACKGROUND_SIZE;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonBackgroundSize = MAX_BUTTON_BACKGROUND_SIZE;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));

    // checkout the side of closeButtonRightMargin
    style.closeButtonRightMargin = MIN_CLOSE_BUTTON_RIGHT_MARGIN - 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.closeButtonRightMargin = MAX_CLOSE_BUTTON_RIGHT_MARGIN + 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.closeButtonRightMargin = MIN_CLOSE_BUTTON_RIGHT_MARGIN;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));
    style.closeButtonRightMargin = MAX_CLOSE_BUTTON_RIGHT_MARGIN;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));

    // checkout the side of spacingBetweenButtons
    style.spacingBetweenButtons = MIN_SPACING_BETWEEN_BUTTONS - 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.spacingBetweenButtons = MAX_SPACING_BETWEEN_BUTTONS + 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.spacingBetweenButtons = MIN_SPACING_BETWEEN_BUTTONS;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));
    style.spacingBetweenButtons = MAX_SPACING_BETWEEN_BUTTONS;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));

    // checkout the side of buttonIconSize
    style.buttonIconSize = MIN_BUTTON_ICON_SIZE - 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonIconSize = MAX_BUTTON_ICON_SIZE + 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonIconSize = MIN_BUTTON_ICON_SIZE;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonIconSize = MAX_BUTTON_ICON_SIZE;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));

    // checkout the side of buttonBackgroundCornerRadius
    style.buttonBackgroundCornerRadius = MIN_BUTTON_BACKGROUND_CORNER_RADIUS - 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonBackgroundCornerRadius = MAX_BUTTON_BACKGROUND_CORNER_RADIUS + 1;
    EXPECT_FALSE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonBackgroundCornerRadius = MIN_BUTTON_BACKGROUND_CORNER_RADIUS;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));
    style.buttonBackgroundCornerRadius = MAX_BUTTON_BACKGROUND_CORNER_RADIUS;
    EXPECT_TRUE(WindowHelper::CheckButtonStyleValid(style));
}

/**
 * @tc.name: SplitStringByDelimiter_TestInput
 * @tc.desc: SplitStringByDelimiter test inputStr
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, SplitStringByDelimiter_TestInput, TestSize.Level1)
{
    std::string inputStr = "";
    std::string delimiter = ",";
    std::unordered_set<std::string> container;
    WindowHelper::SplitStringByDelimiter(inputStr, delimiter, container);
    EXPECT_EQ(0, container.size());
}

/**
 * @tc.name: SplitStringByDelimiter_TestDelimiter
 * @tc.desc: SplitStringByDelimiter test delimiter
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, SplitStringByDelimiter_TestDelimiter, TestSize.Level1)
{
    std::string inputStr = "a,b,c,d";
    std::string delimiter = "";
    std::unordered_set<std::string> container;
    WindowHelper::SplitStringByDelimiter(inputStr, delimiter, container);
    EXPECT_EQ(1, container.count(inputStr));
    EXPECT_EQ(1, container.size());
}

/**
 * @tc.name: SplitStringByDelimiter_TestOther
 * @tc.desc: SplitStringByDelimiter test other
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, SplitStringByDelimiter_TestOther, TestSize.Level1)
{
    std::string inputStr = "a,b,c,d";
    std::string delimiter = ",";
    std::unordered_set<std::string> container;
    WindowHelper::SplitStringByDelimiter(inputStr, delimiter, container);
    EXPECT_EQ(0, container.count("e"));
    EXPECT_EQ(1, container.count("a"));
}

/**
 * @tc.name: ConvertSupportTypeToSupportModes
 * @tc.desc: ConvertSupportTypeToSupportModes
 * @tc.type: FUNC
 */
HWTEST_F(WindowHelperTest, ConvertSupportTypeToSupportModes, TestSize.Level1)
{
    auto supportModes = WindowHelper::ConvertSupportTypeToSupportModes(
        WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING);
    EXPECT_EQ(supportModes.size(), 1);
    EXPECT_EQ(supportModes[0], AppExecFwk::SupportWindowMode::FLOATING);
    supportModes = WindowHelper::ConvertSupportTypeToSupportModes(
        WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY | WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY);
    EXPECT_EQ(supportModes.size(), 1);
    EXPECT_EQ(supportModes[0], AppExecFwk::SupportWindowMode::SPLIT);
    supportModes = WindowHelper::ConvertSupportTypeToSupportModes(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    EXPECT_EQ(supportModes.size(), 1);
    EXPECT_EQ(supportModes[0], AppExecFwk::SupportWindowMode::FULLSCREEN);
}
} // namespace
} // namespace Rosen
} // namespace OHOS