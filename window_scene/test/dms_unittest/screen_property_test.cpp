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

#include "screen_property.h"
#include <gtest/gtest.h>
#include "fold_screen_state_internel.h"

// using namespace FRAME_TRACE;
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class ScreenPropertyTest : public testing::Test {
  public:
    ScreenPropertyTest() {}
    ~ScreenPropertyTest() {}
};

namespace {
/**
 * @tc.name: SetScreenDensityProperties
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetScreenDensityProperties, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    float screenDpi = 5.0;
    property->SetScreenDensityProperties(screenDpi);
    ASSERT_EQ(property->GetVirtualPixelRatio(), screenDpi);
    ASSERT_EQ(property->GetDefaultDensity(), screenDpi);
    ASSERT_EQ(property->GetDensityInCurResolution(), screenDpi);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenRotation end";
}

/**
 * @tc.name: SetScreenRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetScreenRotation, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    Rotation rotation = Rotation::ROTATION_0;
    property->SetScreenRotation(rotation);
    float initialRotationValue = property->GetRotation();
    ASSERT_FLOAT_EQ(initialRotationValue, 0.0f);

    rotation = Rotation::ROTATION_90;
    property->SetScreenRotation(rotation);
    initialRotationValue = property->GetRotation();
    ASSERT_FLOAT_EQ(initialRotationValue, 90.0f);

    rotation = Rotation::ROTATION_180;
    property->SetScreenRotation(rotation);
    initialRotationValue = property->GetRotation();
    ASSERT_FLOAT_EQ(initialRotationValue, 180.0f);

    rotation = Rotation::ROTATION_270;
    property->SetScreenRotation(rotation);
    initialRotationValue = property->GetRotation();
    ASSERT_FLOAT_EQ(initialRotationValue, 270.0f);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenRotation end";
}

/**
 * @tc.name: SetRotationAndScreenRotationOnly001
 * @tc.desc: SetRotationAndScreenRotationOnly001 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetRotationAndScreenRotationOnly001, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetRotationAndScreenRotationOnly001 start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    Rotation ret = Rotation::ROTATION_0;
    Rotation rotation = Rotation::ROTATION_0;
    property->SetRotationAndScreenRotationOnly(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_90;
    property->SetRotationAndScreenRotationOnly(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_180;
    property->SetRotationAndScreenRotationOnly(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_270;
    property->SetRotationAndScreenRotationOnly(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetRotationAndScreenRotationOnly001 end";
}

/**
 * @tc.name: UpdateDeviceRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, UpdateDeviceRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: UpdateDeviceRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    Rotation ret = Rotation::ROTATION_0;
    Rotation rotation = Rotation::ROTATION_0;
    property->UpdateDeviceRotation(rotation);
    ret = property->GetDeviceRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_90;
    property->UpdateDeviceRotation(rotation);
    ret = property->GetDeviceRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_180;
    property->UpdateDeviceRotation(rotation);
    ret = property->GetDeviceRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_270;
    property->UpdateDeviceRotation(rotation);
    ret = property->GetDeviceRotation();
    ASSERT_EQ(ret, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: UpdateDeviceRotation end";
}

/**
 * @tc.name: SetDeviceOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetDeviceOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetDeviceOrientation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    DisplayOrientation ret = DisplayOrientation::PORTRAIT;
    DisplayOrientation displayOrientation = DisplayOrientation::PORTRAIT;
    property->SetDeviceOrientation(displayOrientation);
    ret = property->GetDeviceOrientation();
    ASSERT_EQ(ret, displayOrientation);

    displayOrientation = DisplayOrientation::LANDSCAPE;
    property->SetDeviceOrientation(displayOrientation);
    ret = property->GetDeviceOrientation();
    ASSERT_EQ(ret, displayOrientation);

    displayOrientation = DisplayOrientation::PORTRAIT_INVERTED;
    property->SetDeviceOrientation(displayOrientation);
    ret = property->GetDeviceOrientation();
    ASSERT_EQ(ret, displayOrientation);

    displayOrientation = DisplayOrientation::LANDSCAPE_INVERTED;
    property->SetDeviceOrientation(displayOrientation);
    ret = property->GetDeviceOrientation();
    ASSERT_EQ(ret, displayOrientation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetDeviceOrientation end";
}

/**
 * @tc.name: UpdateScreenRotation
 * @tc.desc: UpdateScreenRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, UpdateScreenRotation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: UpdateScreenRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    Rotation ret = Rotation::ROTATION_0;
    Rotation rotation = Rotation::ROTATION_0;
    property->UpdateScreenRotation(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_90;
    property->UpdateScreenRotation(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_180;
    property->UpdateScreenRotation(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);

    rotation = Rotation::ROTATION_270;
    property->UpdateScreenRotation(rotation);
    ret = property->GetScreenRotation();
    ASSERT_EQ(ret, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: UpdateScreenRotation end";
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOrientation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    Orientation ret = Orientation::BEGIN;
    Orientation orientation = Orientation::SENSOR;
    property->SetOrientation(orientation);
    ret = property->orientation_;
    ASSERT_EQ(ret, orientation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOrientation end";
}

/**
 * @tc.name: GetOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetOrientation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    Orientation ret = Orientation::BEGIN;
    Orientation orientation = Orientation::SENSOR;
    property->SetOrientation(orientation);
    ret = property->GetOrientation();
    ASSERT_EQ(ret, orientation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetOrientation end";
}

/**
 * @tc.name: UpdateVirtualPixelRatio
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, UpdateVirtualPixelRatio, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: UpdateVirtualPixelRatio start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;

    property->UpdateVirtualPixelRatio(bounds);
    ASSERT_EQ(property->GetVirtualPixelRatio(), 3.5f);

    bounds.rect_.height_ = 1111;
    property->UpdateVirtualPixelRatio(bounds);
    ASSERT_EQ(property->GetVirtualPixelRatio(), 1.5f);

    bounds.rect_.width_ = 1111;
    bounds.rect_.height_ = 2772;
    property->UpdateVirtualPixelRatio(bounds);
    ASSERT_EQ(property->GetVirtualPixelRatio(), 1.5f);

    bounds.rect_.width_ = 1111;
    bounds.rect_.height_ = 1111;
    property->UpdateVirtualPixelRatio(bounds);
    ASSERT_EQ(property->GetVirtualPixelRatio(), 1.5f);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: UpdateVirtualPixelRatio end";
}

/**
 * @tc.name: SetBounds
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetBounds, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetBounds start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    int64_t ret = 0;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;

    uint32_t phyWidth = UINT32_MAX;
    property->SetPhyWidth(phyWidth);
    uint32_t phyHeight = UINT32_MAX;
    property->SetPhyHeight(phyHeight);
    property->SetBounds(bounds);
    ASSERT_EQ(property->GetBounds().rect_.width_, 1344);
    ASSERT_EQ(property->GetBounds().rect_.height_, 2772);

    bounds.rect_.width_ = 2772;
    bounds.rect_.height_ = 1344;

    uint32_t phyWidth1 = 2772;
    property->SetPhyWidth(phyWidth1);
    uint32_t phyHeight1 = 1344;
    property->SetPhyHeight(phyHeight1);
    property->SetBounds(bounds);
    ASSERT_EQ(property->GetBounds().rect_.width_, 2772);
    ASSERT_EQ(property->GetBounds().rect_.height_, 1344);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetBounds end";
}

/**
 * @tc.name: CalculateXYDpi
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, CalculateXYDpi, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: CalculateXYDpi start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    RRect Phybounds;
    Phybounds.rect_.width_ = 2772;
    Phybounds.rect_.height_ = 1344;
    property->SetPhyBounds(Phybounds);

    uint32_t phyWidth = 0;
    uint32_t phyHeight = 0;
    property->CalculateXYDpi(phyWidth, phyHeight);
    EXPECT_FLOAT_EQ(property->GetXDpi(), 0);
    EXPECT_FLOAT_EQ(property->GetYDpi(), 0);

    phyWidth = 1;
    phyHeight = 1;
    property->CalculateXYDpi(phyWidth, phyHeight);
    EXPECT_FLOAT_EQ(property->GetXDpi(), 70408.797);
    EXPECT_FLOAT_EQ(property->GetYDpi(), 34137.6);

    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: CalculateXYDpi end";
}

/**
 * @tc.name: SetOffsetX
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetOffsetX, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOffsetX start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    int32_t offsetX = 0;
    property->SetOffsetX(offsetX);
    int32_t ret = property->GetOffsetX();
    ASSERT_EQ(ret, offsetX);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOffsetX end";
}

/**
 * @tc.name: SetOffsetY
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetOffsetY, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOffsetY start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    int32_t offsetY = 0;
    property->SetOffsetY(offsetY);
    int32_t ret = property->GetOffsetY();
    ASSERT_EQ(ret, offsetY);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOffsetY end";
}

/**
 * @tc.name: SetOffset
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetOffset, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOffset start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    property->SetOffset(offsetX, offsetY);
    int32_t ret_x = property->GetOffsetX();
    int32_t ret_y = property->GetOffsetY();
    ASSERT_EQ(ret_x, offsetX);
    ASSERT_EQ(ret_y, offsetY);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetOffset end";
}

/**
 * @tc.name: SetScreenRequestedOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetScreenRequestedOrientation, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenRequestedOrientation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    Orientation orientation = Orientation::UNSPECIFIED;
    property->SetScreenRequestedOrientation(orientation);
    Orientation ret = property->GetScreenRequestedOrientation();
    ASSERT_EQ(ret, orientation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenRequestedOrientation end";
}

/**
 * @tc.name: GetPhyHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetPhyHeight, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPhyHeight start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    uint32_t phyHeight = 1;
    property->SetPhyHeight(phyHeight);
    int32_t ret = property->GetPhyHeight();
    ASSERT_EQ(ret, phyHeight);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPhyHeight end";
}

/**
 * @tc.name: SetRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetRotation, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float rotation = 2.0f;
    property->SetRotation(rotation);
    ASSERT_EQ(property->rotation_, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetRotation end";
}

/**
 * @tc.name: GetRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetRotation, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float rotation = property->GetRotation();
    ASSERT_EQ(property->rotation_, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetRotation end";
}

/**
 * @tc.name: SetPhysicalRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetPhysicalRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPhysicalRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float rotation = 2.0f;
    property->SetPhysicalRotation(rotation);
    ASSERT_EQ(property->physicalRotation_, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPhysicalRotation end";
}

/**
 * @tc.name: GetPhysicalRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetPhysicalRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPhysicalRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float rotation = property->GetPhysicalRotation();
    ASSERT_EQ(property->physicalRotation_, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPhysicalRotation end";
}

/**
 * @tc.name: SetScreenComponentRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetScreenComponentRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenComponentRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float rotation = 2.0f;
    property->SetScreenComponentRotation(rotation);
    ASSERT_EQ(property->screenComponentRotation_, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenComponentRotation end";
}

/**
 * @tc.name: GetScreenComponentRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetScreenComponentRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetScreenComponentRotation start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float rotation = property->GetScreenComponentRotation();
    ASSERT_EQ(property->screenComponentRotation_, rotation);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetScreenComponentRotation end";
}

/**
 * @tc.name: GetBounds
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetBounds, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetBounds start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    RRect bounds = property->GetBounds();
    ASSERT_EQ(property->bounds_, bounds);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetBounds end";
}

/**
 * @tc.name: SetPhyBounds
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetPhyBounds, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPhyBounds start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    RRect phyBounds;
    phyBounds.rect_.width_ = 1344;
    phyBounds.rect_.height_ = 2772;
    property->SetPhyBounds(phyBounds);
    ASSERT_EQ(property->phyBounds_, phyBounds);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPhyBounds end";
}

/**
 * @tc.name: GetPhyBounds
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetPhyBounds, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPhyBounds start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    RRect phyBounds = property->GetPhyBounds();
    ASSERT_EQ(property->phyBounds_, phyBounds);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPhyBounds end";
}

/**
 * @tc.name: GetDensity
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetDensity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetDensity start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float virtualPixelRatio = 1.0f;
    ASSERT_EQ(property->virtualPixelRatio_, virtualPixelRatio);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetDensity end";
}

/**
 * @tc.name: GetDefaultDensity
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetDefaultDensity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetDefaultDensity start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float defaultDensity = 1.0f;
    ASSERT_EQ(property->GetDefaultDensity(), defaultDensity);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetDefaultDensity end";
}

/**
 * @tc.name: SetDefaultDensity
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetDefaultDensity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetDefaultDensity start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float defaultDensity = 1.0f;
    property->SetDefaultDensity(defaultDensity);
    ASSERT_EQ(property->defaultDensity_, defaultDensity);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetDefaultDensity end";
}

/**
 * @tc.name: GetDensityInCurResolution
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetDensityInCurResolution, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetDensityInCurResolution start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float densityInCurResolution = 1.0f;
    ASSERT_EQ(property->GetDensityInCurResolution(), densityInCurResolution);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetDensityInCurResolution end";
}

/**
 * @tc.name: SetDensityInCurResolution
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetDensityInCurResolution, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetDensityInCurResolution start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float densityInCurResolution = 1.0f;
    property->SetDensityInCurResolution(densityInCurResolution);
    ASSERT_EQ(property->densityInCurResolution_, densityInCurResolution);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetDensityInCurResolution end";
}

/**
 * @tc.name: GetPropertyChangeReason
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetPropertyChangeReason, TestSize.Level1)
{
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    ScreenPropertyChangeReason propertyChangeReason = ScreenPropertyChangeReason::SCREEN_MODE_CHANGE;
    ScreenPropertyChangeReason propertyChangeReason_copy = property->GetPropertyChangeReason();
    property->SetPropertyChangeReason(propertyChangeReason);
    ASSERT_EQ(propertyChangeReason, property->GetPropertyChangeReason());
    property->SetPropertyChangeReason(propertyChangeReason_copy);
}

/**
 * @tc.name: CalcDefaultDisplayOrientation
 * @tc.desc: bounds_.rect_.width_ > bounds_.rect_.height_
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, CalcDefaultDisplayOrientation, TestSize.Level1)
{
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    RRect bounds_temp = property->GetBounds();
    RRect bounds;
    bounds.rect_.width_ = 2772;
    bounds.rect_.height_ = 1344;
    property->SetBounds(bounds);
    property->CalcDefaultDisplayOrientation();
    ASSERT_EQ(DisplayOrientation::LANDSCAPE, property->GetDisplayOrientation());
    property->SetBounds(bounds_temp);
}

/**
 * @tc.name: SetStartX
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetStartX, TestSize.Level1)
{
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t ret = 100;
    uint32_t ret_copy = property->GetStartX();
    property->SetStartX(ret);
    ASSERT_EQ(ret, property->GetStartX());
    property->SetStartX(ret_copy);
}

/**
 * @tc.name: SetStartY
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetStartY, TestSize.Level1)
{
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t ret = 100;
    uint32_t ret_copy = property->GetStartY();
    property->SetStartY(ret);
    ASSERT_EQ(ret, property->GetStartY());
    property->SetStartY(ret_copy);
}

/**
 * @tc.name: SetStartPosition
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetStartPosition, TestSize.Level1)
{
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t ret_x = 100;
    uint32_t ret_y = 200;
    uint32_t retx_copy = property->GetStartX();
    uint32_t rety_copy = property->GetStartY();
    property->SetStartPosition(ret_x, ret_y);
    ASSERT_EQ(100, property->GetStartX());
    ASSERT_EQ(200, property->GetStartY());
    property->SetStartPosition(retx_copy, rety_copy);
}

/**
 * @tc.name: SetScreenShape
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetScreenShape, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenShape start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    ScreenShape screenshape = ScreenShape::ROUND;
    property->SetScreenShape(screenshape);
    ASSERT_EQ(property->screenShape_, screenshape);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetScreenShape end";
}

/**
 * @tc.name: GetScreenShape
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetScreenShape, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetScreenShape start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    ScreenShape screenshape = ScreenShape::ROUND;
    property->SetScreenShape(screenshape);
    ASSERT_EQ(property->GetScreenShape(), screenshape);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetScreenShape end";
}

/**
 * @tc.name: SetPointerActiveWidth
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetPointerActiveWidth, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPointerActiveWidth start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t pointerActiveWidth = 123;
    property->SetPointerActiveWidth(pointerActiveWidth);
    ASSERT_EQ(property->pointerActiveWidth_, pointerActiveWidth);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPointerActiveWidth end";
}
 
/**
 * @tc.name: GetPointerActiveWidth
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetPointerActiveWidth, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPointerActiveWidth start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t pointerActiveWidth = 123;
    property->SetPointerActiveWidth(pointerActiveWidth);
    ASSERT_EQ(property->GetPointerActiveWidth(), pointerActiveWidth);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPointerActiveWidth end";
}
 
/**
 * @tc.name: SetPointerActiveHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetPointerActiveHeight, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPointerActiveHeight start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t pointerActiveHeight = 321;
    property->SetPointerActiveHeight(pointerActiveHeight);
    ASSERT_EQ(property->pointerActiveHeight_, pointerActiveHeight);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetPointerActiveHeight end";
}
 
/**
 * @tc.name: GetPointerActiveHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetPointerActiveHeight, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPointerActiveHeight start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    uint32_t pointerActiveHeight = 321;
    property->SetPointerActiveHeight(pointerActiveHeight);
    ASSERT_EQ(property->GetPointerActiveHeight(), pointerActiveHeight);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetPointerActiveHeight end";
}

/**
* @tc.name: SetX
* @tc.desc: normal function
* @tc.type: FUNC
*/
HWTEST_F(ScreenPropertyTest, SetX, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetX start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    int32_t ret = -1000;
    int32_t ret_copy = property->GetX();
    property->SetX(ret);
    ASSERT_EQ(ret, property->GetX());
    property->SetX(ret_copy);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetX end";
}

/**
* @tc.name: SetY
* @tc.desc: normal function
* @tc.type: FUNC
*/
HWTEST_F(ScreenPropertyTest, SetY, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetY start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    int32_t ret = -1000;
    int32_t ret_copy = property->GetY();
    property->SetY(ret);
    ASSERT_EQ(ret, property->GetY());
    property->SetY(ret_copy);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetY end";
}

/**
* @tc.name: SetXYPosition
* @tc.desc: normal function
* @tc.type: FUNC
*/
HWTEST_F(ScreenPropertyTest, SetXYPosition, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetXYPosition start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    int32_t ret_x = 1000;
    int32_t ret_y = 2000;
    int32_t retx_copy = property->GetX();
    int32_t rety_copy = property->GetY();
    property->SetXYPosition(ret_x, ret_y);
    ASSERT_EQ(1000, property->GetX());
    ASSERT_EQ(2000, property->GetY());
    property->SetXYPosition(retx_copy, rety_copy);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetXYPosition end";
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: SetVirtualPixelRatio test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetVirtualPixelRatio, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetVirtualPixelRatio start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float pixelRatio = 1.0f;
    float pixelCopy = property->GetVirtualPixelRatio();
    property->SetVirtualPixelRatio(pixelRatio);
    ASSERT_EQ(property->virtualPixelRatio_, pixelRatio);
    property->SetVirtualPixelRatio(pixelCopy);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetVirtualPixelRatio end";
}
 
/**
 * @tc.name: GetVirtualPixelRatio
 * @tc.desc: GetVirtualPixelRatio test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, GetVirtualPixelRatio, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetVirtualPixelRatio start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    float pixelRatio = 1.0f;
    float pixelCopy = property->GetVirtualPixelRatio();
    property->SetVirtualPixelRatio(pixelRatio);
    ASSERT_EQ(property->GetVirtualPixelRatio(), pixelRatio);
    property->SetVirtualPixelRatio(pixelCopy);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetVirtualPixelRatio end";
}

/**
 * @tc.name: SetRotationAndScreenRotationOnly
 * @tc.desc: SetRotationAndScreenRotationOnly test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetRotationAndScreenRotationOnly, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetRotationAndScreenRotationOnly start";
    std::shared_ptr<ScreenProperty> property = std::make_shared<ScreenProperty>();
    ASSERT_NE(property, nullptr);
    property->SetRotationAndScreenRotationOnly(Rotation::ROTATION_0);
    ASSERT_EQ(property->GetScreenRotation(), Rotation::ROTATION_0);
    
    property->SetRotationAndScreenRotationOnly(Rotation::ROTATION_90);
    ASSERT_EQ(property->GetScreenRotation(), Rotation::ROTATION_90);

    property->SetRotationAndScreenRotationOnly(Rotation::ROTATION_180);
    ASSERT_EQ(property->GetScreenRotation(), Rotation::ROTATION_180);

    property->SetRotationAndScreenRotationOnly(Rotation::ROTATION_270);
    ASSERT_EQ(property->GetScreenRotation(), Rotation::ROTATION_270);
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetRotationAndScreenRotationOnly end";
}

/**
 * @tc.name: SetPhysicalTouchBounds
 * @tc.desc: SetPhysicalTouchBounds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetPhysicalTouchBounds, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP();
    }
    std::shared_ptr<ScreenProperty> property = std::make_shared<ScreenProperty>();
    property->physicalTouchBounds_.rect_.width_ = 100;
    property->physicalTouchBounds_.rect_.height_ = 200;
    property->rotation_ = 0.0f;
    property->SetPhysicalTouchBounds(Rotation::ROTATION_0);
    EXPECT_EQ(property->physicalTouchBounds_.rect_.width_, 200);
    EXPECT_EQ(property->physicalTouchBounds_.rect_.height_, 100);
 
    property->rotation_ = 90.0f;
    property->SetPhysicalTouchBounds(Rotation::ROTATION_0);
    EXPECT_EQ(property->physicalTouchBounds_.rect_.width_, 100);
    EXPECT_EQ(property->physicalTouchBounds_.rect_.height_, 200);
 
    property->rotation_ = 270.0f;
    property->SetPhysicalTouchBounds(Rotation::ROTATION_0);
    EXPECT_EQ(property->physicalTouchBounds_.rect_.width_, 100);
    EXPECT_EQ(property->physicalTouchBounds_.rect_.height_, 200);
}

/**
 * @tc.name: SetInputOffset
 * @tc.desc: SetInputOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPropertyTest, SetInputOffset, TestSize.Level1)
{
    std::shared_ptr<ScreenProperty> property = std::make_shared<ScreenProperty>();
    property->SetInputOffset(0, 0);
    EXPECT_EQ(property->GetInputOffsetX(), 0);
    EXPECT_EQ(property->GetInputOffsetY(), 0);
    property->SetInputOffset(-1, -2);
    EXPECT_EQ(property->GetInputOffsetX(), -1);
    EXPECT_EQ(property->GetInputOffsetY(), -2);
    property->SetInputOffset(1, 2);
    EXPECT_EQ(property->GetInputOffsetX(), 1);
    EXPECT_EQ(property->GetInputOffsetY(), 2);
}

/**
@tc.name : SetIsDestroyDisplay
@tc.desc : SetIsDestroyDisplay
@tc.type: FUNC
*/
HWTEST_F(ScreenPropertyTest, SetIsDestroyDisplay, TestSize.Level1) {
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetIsDestroyDisplay start";
    ScreenProperty* property = new(std::nothrow) ScreenProperty();
    ASSERT_NE(property, nullptr);
    property->SetIsDestroyDisplay(true);
    EXPECT_TRUE(property->isDestroyDisplay_);
    delete property;
    GTEST_LOG_(INFO) << "ScreenPropertyTest: SetIsDestroyDisplay end";
}

/**
@tc.name : GetIsDestroyDisplay
@tc.desc : GetIsDestroyDisplay
@tc.type: FUNC
*/
HWTEST_F(ScreenPropertyTest, GetIsDestroyDisplay, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetIsDestroyDisplay start";

    ScreenProperty* property1 = new (std::nothrow) ScreenProperty();
    ASSERT_NE(property1, nullptr);
    EXPECT_FALSE(property1->GetIsDestroyDisplay());
    delete property1;

    ScreenProperty* property2 = new (std::nothrow) ScreenProperty();
    ASSERT_NE(property2, nullptr);
    property2->SetIsDestroyDisplay(true);
    EXPECT_TRUE(property2->GetIsDestroyDisplay());
    delete property2;

    ScreenProperty* property3 = new (std::nothrow) ScreenProperty();
    ASSERT_NE(property3, nullptr);
    property3->SetIsDestroyDisplay(false);
    EXPECT_FALSE(property3->GetIsDestroyDisplay());
    delete property3;

    GTEST_LOG_(INFO) << "ScreenPropertyTest: GetIsDestroyDisplay end";
}

HWTEST_F(ScreenPropertyTest, SetPhysicalTouchBoundsDirectly, TestSize.Level1)
{
    std::shared_ptr<ScreenProperty> property = std::make_shared<ScreenProperty>();

    RRect zeroBounds;
    zeroBounds.rect_.width_ = 0;
    zeroBounds.rect_.height_ = 0;

    property->SetPhysicalTouchBoundsDirectly(zeroBounds);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        EXPECT_EQ(property->physicalTouchBounds_.rect_.width_, 0);
        EXPECT_EQ(property->physicalTouchBounds_.rect_.height_, 0);
    }

    RRect negativeBounds;
    negativeBounds.rect_.width_ = -100;
    negativeBounds.rect_.height_ = -200;

    property->SetPhysicalTouchBoundsDirectly(negativeBounds);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        EXPECT_EQ(property->physicalTouchBounds_.rect_.width_, -100);
        EXPECT_EQ(property->physicalTouchBounds_.rect_.height_, -200);
    }

    RRect normalBounds;
    normalBounds.rect_.width_ = 1920;
    normalBounds.rect_.height_ = 1080;

    property->SetPhysicalTouchBoundsDirectly(normalBounds);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        EXPECT_EQ(property->physicalTouchBounds_.rect_.width_, 1920);
        EXPECT_EQ(property->physicalTouchBounds_.rect_.height_, 1080);
    }
}
} // namespace
} // namespace Rosen
} // namespace OHOS
