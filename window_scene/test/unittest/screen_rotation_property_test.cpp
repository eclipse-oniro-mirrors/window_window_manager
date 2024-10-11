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
#include <chrono>

#include <gtest/gtest.h>

#include "window_manager_hilog.h"
#include "screen_session_manager.h"

#include "session_manager/include/screen_rotation_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class ScreenRotationPropertyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenRotationPropertyTest::SetUpTestCase()
{
}

void ScreenRotationPropertyTest::TearDownTestCase()
{
}

void ScreenRotationPropertyTest::SetUp()
{
}

void ScreenRotationPropertyTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: HandleSensorEventInput
 * @tc.desc: test function : HandleSensorEventInput
 * @tc.type: FUNC
 */
HWTEST_F(ScreenRotationPropertyTest, HandleSensorEventInput, Function | SmallTest | Level1)
{
    ScreenRotationProperty::HandleSensorEventInput(DeviceRotation::INVALID);
    ScreenRotationProperty::HandleSensorEventInput(DeviceRotation::ROTATION_PORTRAIT);
    ScreenRotationProperty::HandleSensorEventInput(DeviceRotation::ROTATION_LANDSCAPE);
}

/**
 * @tc.name: ConvertDeviceToFloat
 * @tc.desc: test function : ConvertDeviceToFloat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenRotationPropertyTest, ConvertDeviceToFloat, Function | SmallTest | Level1)
{
    float ret;
    ret = ScreenRotationProperty::ConvertDeviceToFloat(DeviceRotation::INVALID);
    ASSERT_EQ(ret, -1.0f);

    ret = ScreenRotationProperty::ConvertDeviceToFloat(DeviceRotation::ROTATION_PORTRAIT);
    ASSERT_EQ(ret, 0.0f);

    ret = ScreenRotationProperty::ConvertDeviceToFloat(DeviceRotation::ROTATION_LANDSCAPE);
    ASSERT_EQ(ret, 90.0f);

    ret = ScreenRotationProperty::ConvertDeviceToFloat(DeviceRotation::ROTATION_PORTRAIT_INVERTED);
    ASSERT_EQ(ret, 180.0f);

    ret = ScreenRotationProperty::ConvertDeviceToFloat(DeviceRotation::ROTATION_LANDSCAPE_INVERTED);
    ASSERT_EQ(ret, 270.0f);
}

}

}
}