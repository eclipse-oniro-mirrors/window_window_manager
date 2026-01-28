/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "screen_manager.h"
#include "screen_manager_utils.h"
#include "mock_display_manager_adapter.h"
#include "singleton_mocker.h"
#include "scene_board_judgement.h"
#include "screen_scene_config.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
using Mocker = SingletonMocker<ScreenManagerAdapter, MockScreenManagerAdapter>;
class ScreenTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    static ScreenId defaultScreenId_;
    static sptr<Screen> screen_;
};
ScreenId ScreenTest::defaultScreenId_ = SCREEN_ID_INVALID;
sptr<Screen> ScreenTest::screen_ = nullptr;
bool g_isPcDevice = ScreenSceneConfig::GetExternalScreenDefaultMode() == "none";

void ScreenTest::SetUpTestCase()
{
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();
    screen_ =  sptr<Screen>::MakeSptr(screenInfo);
    usleep(SLEEP_TIME_IN_US);
}

void ScreenTest::TearDownTestCase()
{
    screen_ = nullptr;
}

void ScreenTest::SetUp()
{
}

void ScreenTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetBasicProperty01
 * @tc.desc: Basic property getter test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetBasicProperty01, TestSize.Level1)
{
    EXPECT_EQ(screen_->GetName().size(), 0);
    EXPECT_EQ(screen_->GetWidth(), 0);
    EXPECT_EQ(screen_->GetHeight(), 0);
    EXPECT_EQ(screen_->GetVirtualWidth(), 0);
    EXPECT_EQ(screen_->GetVirtualHeight(), 0);
    EXPECT_EQ(screen_->GetVirtualPixelRatio(), 1);
    EXPECT_EQ(screen_->IsReal(), false);
    EXPECT_NE(screen_->GetScreenInfo(), nullptr);
}

/**
 * @tc.name: SetScreenActiveMode01
 * @tc.desc: SetScreenActiveMode01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenActiveMode01, TestSize.Level1)
{
    auto modes = screen_->GetSupportedModes();
    ASSERT_EQ(modes.size(), 0u) << "This test only applies when no display modes are supported.";
    EXPECT_EQ(screen_->SetScreenActiveMode(0), DMError::DM_ERROR_INVALID_PARAM);
    EXPECT_EQ(screen_->SetScreenActiveMode(1), DMError::DM_ERROR_INVALID_PARAM);
    EXPECT_EQ(screen_->SetScreenActiveMode(100), DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetScreenActiveMode02
 * @tc.desc: SetScreenActiveMode02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenActiveMode02, TestSize.Level1)
{
    uint32_t invalidModeId = 999999;
    DMError result = screen_->SetScreenActiveMode(invalidModeId);
    ASSERT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetScreenSupportedColorGamuts01
 * @tc.desc: GetScreenSupportedColorGamuts
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetScreenSupportedColorGamuts01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetScreenSupportedColorGamuts(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    std::vector<ScreenColorGamut> colorGamuts;
    auto res = screen_->GetScreenSupportedColorGamuts(colorGamuts);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetScreenColorGamut01
 * @tc.desc: GetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetScreenColorGamut01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetScreenColorGamut(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenColorGamut colorGamut = ScreenColorGamut::COLOR_GAMUT_SRGB;
    auto res = screen_->GetScreenColorGamut(colorGamut);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetScreenColorGamut01
 * @tc.desc: SetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenColorGamut01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetScreenColorGamut(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenColorGamut colorGamut = ScreenColorGamut::COLOR_GAMUT_SRGB;
    auto res = screen_->SetScreenColorGamut(colorGamut);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetScreenGamutMap01
 * @tc.desc: GetScreenGamutMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetScreenGamutMap01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetScreenGamutMap(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenGamutMap gamutMap = ScreenGamutMap::GAMUT_MAP_CONSTANT;
    auto res = screen_->GetScreenGamutMap(gamutMap);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetScreenGamutMap01
 * @tc.desc: SetScreenGamutMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenGamutMap01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetScreenGamutMap(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenGamutMap gamutMap = ScreenGamutMap::GAMUT_MAP_CONSTANT;
    auto res = screen_->SetScreenGamutMap(gamutMap);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetScreenColorTransform01
 * @tc.desc: SetScreenColorTransform
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenColorTransform01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetScreenColorTransform(_)).Times(1).WillOnce(Return(DMError::DM_OK));
    auto res = screen_->SetScreenColorTransform();
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: IsGroup
 * @tc.desc: for interface coverage and check IsGroup
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, IsGroup, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<ScreenInfo> screenInfo = screen_->GetScreenInfo();
    screenInfo->SetIsScreenGroup(true);
    EXPECT_CALL(m->Mock(), GetScreenInfo(_)).Times(1).WillOnce(Return(screenInfo));
    ASSERT_EQ(true, screen_->IsGroup());
    screenInfo->SetIsScreenGroup(false);
    EXPECT_CALL(m->Mock(), GetScreenInfo(_)).Times(1).WillOnce(Return(screenInfo));
    ASSERT_EQ(false, screen_->IsGroup());
}

/**
 * @tc.name: GetParentId
 * @tc.desc: for interface coverage and check GetParentId
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetParentId, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<ScreenInfo> screenInfo = screen_->GetScreenInfo();
    screenInfo->SetParentId(0);
    EXPECT_CALL(m->Mock(), GetScreenInfo(_)).Times(1).WillOnce(Return(screenInfo));
    ASSERT_EQ(0, screen_->GetParentId());
    screenInfo->SetParentId(SCREEN_ID_INVALID);
    EXPECT_CALL(m->Mock(), GetScreenInfo(_)).Times(1).WillOnce(Return(screenInfo));
    ASSERT_EQ(SCREEN_ID_INVALID, screen_->GetParentId());
}

/**
 * @tc.name: GetRotation
 * @tc.desc: for interface coverage and check GetRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetRotation, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<ScreenInfo> screenInfo = screen_->GetScreenInfo();
    screenInfo->SetParentId(0);
    EXPECT_CALL(m->Mock(), GetScreenInfo(_)).Times(1).WillOnce(Return(screenInfo));
    ASSERT_EQ(Rotation::ROTATION_0, screen_->GetRotation());
}

/**
 * @tc.name: GetPixelFormat
 * @tc.desc: GetPixelFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetPixelFormat, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetPixelFormat(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    GraphicPixelFormat pixelFormat = GraphicPixelFormat{GRAPHIC_PIXEL_FMT_CLUT8};
    auto res = screen_->GetPixelFormat(pixelFormat);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetPixelFormat
 * @tc.desc: SetPixelFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetPixelFormat, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetPixelFormat(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    GraphicPixelFormat pixelFormat = GraphicPixelFormat{GRAPHIC_PIXEL_FMT_CLUT8};
    auto res = screen_->SetPixelFormat(pixelFormat);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: GetSupportedHDRFormats
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetSupportedHDRFormats, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetSupportedHDRFormats(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    std::vector<ScreenHDRFormat> hdrFormats;
    auto res = screen_->GetSupportedHDRFormats(hdrFormats);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetSupportedColorSpaces
 * @tc.desc: GetSupportedColorSpaces
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetSupportedColorSpaces, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetSupportedColorSpaces(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    auto res = screen_->GetSupportedColorSpaces(colorSpaces);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetResolution, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetResolution(_, _, _, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    auto res = screen_->SetResolution(0, 0, 1000);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, res);

    res = screen_->SetResolution(1, 1, 100);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetDensityInCurResolution
 * @tc.desc: GetDensityInCurResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetDensityInCurResolution, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetDensityInCurResolution(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    float virtualPixelRatio;
    auto res = screen_->GetDensityInCurResolution(virtualPixelRatio);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetOrientation, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetOrientation(_, _, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    Orientation orientation = Orientation{0};
    auto res = screen_->SetOrientation(orientation);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetScreenHDRFormat
 * @tc.desc: GetScreenHDRFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetScreenHDRFormat, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetScreenHDRFormat(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    ScreenHDRFormat hdrFormat = ScreenHDRFormat{0};
    auto res = screen_->GetScreenHDRFormat(hdrFormat);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetScreenHDRFormat
 * @tc.desc: SetScreenHDRFormat
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenHDRFormat, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetScreenHDRFormat(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    auto res = screen_->SetScreenHDRFormat(0);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: GetScreenColorSpace
 * @tc.desc: GetScreenColorSpace
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetScGetScreenColorSpacereenHDRFormat, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetScreenColorSpace(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType{GRAPHIC_CM_COLORSPACE_NONE};
    auto res = screen_->GetScreenColorSpace(colorSpace);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetScreenColorSpace
 * @tc.desc: SetScreenColorSpace
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenColorSpace, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), SetScreenColorSpace(_, _)).Times(1).WillOnce(Return(DMError::DM_OK));
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType{GRAPHIC_CM_COLORSPACE_NONE};
    auto res = screen_->SetScreenColorSpace(colorSpace);
    ASSERT_EQ(DMError::DM_OK, res);
}

/**
 * @tc.name: SetDensityDpi01
 * @tc.desc: SetDensityDpi
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetDensityDpi, TestSize.Level1)
{
    auto res = screen_->SetDensityDpi(DOT_PER_INCH_MAXIMUM_VALUE + 1);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, res);
    res = screen_->SetDensityDpi(100);
    ASSERT_NE(DMError::DM_ERROR_INVALID_PARAM, res);
}

/**
 * @tc.name: SetDensityDpiSystem01
 * @tc.desc: SetDensityDpiSystem
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetDensityDpiSystem, TestSize.Level1)
{
    auto res = screen_->SetDensityDpiSystem(DOT_PER_INCH_MAXIMUM_VALUE + 1);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, res);

    res = screen_->SetDensityDpiSystem(100);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(DMError::DM_OK, res);
    } else {
        ASSERT_NE(DMError::DM_OK, res);
    }
}

/**
 * @tc.name: GetSerialNumber
 * @tc.desc: GetSerialNumber
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, GetSerialNumber, Function | SmallTest | Level2)
{
    auto res = screen_->GetSerialNumber();
    ASSERT_EQ(res, "");
}

/**
 * @tc.name: SetDefaultDensityDpi01
 * @tc.desc: SetDefaultDensityDpi
 * @tc.type: FUNC
 *
 */
HWTEST_F(ScreenTest, SetDefaultDensityDpi, TestSize.Level1)
{
    auto res = screen_->SetDefaultDensityDpi(DOT_PER_INCH_MAXIMUM_VALUE + 1);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, res);
    res = screen_->SetDefaultDensityDpi(100);
    ASSERT_NE(DMError::DM_ERROR_INVALID_PARAM, res) << "Valid DPI input should pass validation";
}

/**
 * @tc.name: SetScreenOrientation
 * @tc.desc: SetScreenOrientation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenTest, SetScreenOrientation, TestSize.Level1)
{
    Orientation orientation = Orientation::VERTICAL;
    auto res = screen_->SetScreenOrientation(orientation);
    ASSERT_NE(DMError::DM_OK, res);
}
}
} // namespace Rosen
} // namespace OHOS