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

#include "display_manager.h"
#include "display_manager_adapter.h"
#include "display_manager_proxy.h"
#include "scene_board_judgement.h"
#include "screen_manager.h"
#include "window_scene.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
class DisplayManagerAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DisplayManagerAdapterTest::SetUpTestCase() {}

void DisplayManagerAdapterTest::TearDownTestCase() {}

void DisplayManagerAdapterTest::SetUp() {}

void DisplayManagerAdapterTest::TearDown() {}

namespace {
static constexpr DisplayId DEFAULT_DISPLAY = 1ULL;
static const int32_t PIXELMAP_SIZE = 2;
static const int32_t SDR_PIXELMAP = 0;
static const int32_t HDR_PIXELMAP = 1;
/**
 * @tc.name: GetDisplayInfo
 * @tc.desc: test nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetDisplayInfo, TestSize.Level1)
{
    sptr<DisplayInfo> info = SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayInfo(DISPLAY_ID_INVALID);
    ASSERT_EQ(info, nullptr);
}

/**
 * @tc.name: GetCutoutInfo
 * @tc.desc: test nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetCutoutInfo, TestSize.Level1)
{
    sptr<CutoutInfo> info =
        SingletonContainer::Get<DisplayManagerAdapter>().GetCutoutInfo(DISPLAY_ID_INVALID, 0, 0, Rotation::ROTATION_0);
    ASSERT_EQ(info, nullptr);
}

/**
 * @tc.name: GetScreenSupportedColorGamuts
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenSupportedColorGamuts, TestSize.Level1)
{
    std::vector<ScreenColorGamut> colorGamuts;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenSupportedColorGamuts(0, colorGamuts);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: SetScreenColorGamut
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenColorGamut, TestSize.Level1)
{
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenColorGamut(0, -1);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    } else {
        ASSERT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
    }
}

/**
 * @tc.name: GetScreenColorGamut
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenColorGamut, TestSize.Level1)
{
    ScreenColorGamut colorGamut;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenColorGamut(0, colorGamut);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetScreenGamutMap
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenGamutMap, TestSize.Level1)
{
    ScreenGamutMap gamutMap;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenGamutMap(0, gamutMap);
    ASSERT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: SetScreenGamutMap
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenGamutMap, TestSize.Level1)
{
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenGamutMap(0, GAMUT_MAP_CONSTANT);
    ASSERT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: SetScreenColorTransform
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenColorTransform, TestSize.Level1)
{
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenColorTransform(0);
    ASSERT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name: SetFreeze
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetFreeze, TestSize.Level1)
{
    std::vector<DisplayId> displayIds;
    bool ret = SingletonContainer::Get<DisplayManagerAdapter>().SetFreeze(displayIds, false);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.name: GetScreenGroupInfoById
 * @tc.desc: test nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenGroupInfoById, TestSize.Level1)
{
    auto info = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenGroupInfoById(SCREEN_ID_INVALID);
    ASSERT_EQ(info, nullptr);
}

/**
 * @tc.name: GetScreenInfo
 * @tc.desc: test nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenInfo, TestSize.Level1)
{
    sptr<ScreenInfo> info = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenInfo(SCREEN_ID_INVALID);
    ASSERT_EQ(info, nullptr);
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: test nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, OnRemoteDied, TestSize.Level1)
{
    sptr<IRemoteObject::DeathRecipient> dmsDeath_ = nullptr;
    dmsDeath_ = new DMSDeathRecipient(SingletonContainer::Get<ScreenManagerAdapter>());
    dmsDeath_->OnRemoteDied(nullptr);
    EXPECT_NE(nullptr, dmsDeath_);
}

/**
 * @tc.name: OnRemoteDied01
 * @tc.desc: test nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, OnRemoteDied01, TestSize.Level1)
{
    sptr<IRemoteObject::DeathRecipient> dmsDeath =
        new (std::nothrow) DMSDeathRecipient(SingletonContainer::Get<ScreenManagerAdapter>());
    SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy();
    sptr<IRemoteObject> remoteObject;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        remoteObject = SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
    } else {
        remoteObject = SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_->AsObject();
    }
    wptr<IRemoteObject> wptrDeath = remoteObject;
    dmsDeath->OnRemoteDied(wptrDeath);
    EXPECT_NE(nullptr, dmsDeath);
}

/**
 * @tc.name: Clear
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, Clear, TestSize.Level1)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy();
        SingletonContainer::Get<ScreenManagerAdapter>().Clear();
        ASSERT_FALSE(SingletonContainer::Get<ScreenManagerAdapter>().isProxyValid_);
    } else {
        SingletonContainer::Get<DisplayManagerAdapter>().InitDMSProxy();
        SingletonContainer::Get<DisplayManagerAdapter>().Clear();
        ASSERT_FALSE(SingletonContainer::Get<DisplayManagerAdapter>().isProxyValid_);
    }
}

/**
 * @tc.name: Clear01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, Clear01, TestSize.Level1)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy();
        SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
        SingletonContainer::Get<ScreenManagerAdapter>().Clear();
        ASSERT_FALSE(SingletonContainer::Get<ScreenManagerAdapter>().isProxyValid_);
    } else {
        SingletonContainer::Get<DisplayManagerAdapter>().InitDMSProxy();
        SingletonContainer::Get<DisplayManagerAdapter>().displayManagerServiceProxy_ = nullptr;
        SingletonContainer::Get<DisplayManagerAdapter>().Clear();
        ASSERT_FALSE(SingletonContainer::Get<DisplayManagerAdapter>().isProxyValid_);
    }
}

/**
 * @tc.name: DisableMirror
 * @tc.desc: DisableMirror test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, DisableMirror, TestSize.Level1)
{
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().DisableMirror(false);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, DMError::DM_ERROR_INVALID_PERMISSION);
    } else {
        ASSERT_EQ(ret, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetPixelFormat
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetPixelFormat, TestSize.Level1)
{
    GraphicPixelFormat pixelFormat = GraphicPixelFormat::GRAPHIC_PIXEL_FMT_CLUT8;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetPixelFormat(0, pixelFormat);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: SetPixelFormat
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetPixelFormat, TestSize.Level1)
{
    GraphicPixelFormat pixelFormat = GraphicPixelFormat::GRAPHIC_PIXEL_FMT_CLUT8;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetPixelFormat(0, pixelFormat);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetSupportedHDRFormats, TestSize.Level1)
{
    std::vector<ScreenHDRFormat> hdrFormats;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetSupportedHDRFormats(0, hdrFormats);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetScreenHDRFormat
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenHDRFormat, TestSize.Level1)
{
    ScreenHDRFormat hdrFormat = ScreenHDRFormat::NOT_SUPPORT_HDR;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenHDRFormat(0, hdrFormat);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: SetScreenHDRFormat
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenHDRFormat, TestSize.Level1)
{
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenHDRFormat(0, 0);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetSupportedColorSpaces
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetSupportedColorSpaces, TestSize.Level1)
{
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetSupportedColorSpaces(0, colorSpaces);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetScreenColorSpace
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetScreenColorSpace, TestSize.Level1)
{
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType::GRAPHIC_CM_COLORSPACE_NONE;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenColorSpace(0, colorSpace);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: SetScreenColorSpace
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenColorSpace, TestSize.Level1)
{
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType::GRAPHIC_CM_COLORSPACE_NONE;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenColorSpace(0, colorSpace);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    }
}

/**
 * @tc.name: DestroyVirtualScreen
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, DestroyVirtualScreen, TestSize.Level1)
{
    VirtualScreenOption defaultOption = { "virtualScreen01", 480, 320, 2.0, nullptr, 0 };
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().DestroyVirtualScreen(id);
    ASSERT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name: SetVirtualMirrorScreenCanvasRotation
 * @tc.desc: test SetVirtualMirrorScreenCanvasRotation
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetVirtualMirrorScreenCanvasRotation, TestSize.Level1)
{
    bool canvasRotation = false;
    DMError ret =
        SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualMirrorScreenCanvasRotation(0, canvasRotation);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, DMError::DM_OK);
    } else {
        ASSERT_EQ(ret, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    }
}

/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: test SetScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = false;
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenRotationLocked(isLocked);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, DMError::DM_OK);
    } else {
        ASSERT_EQ(ret, DMError::DM_OK);
    }
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: test IsScreenRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, IsScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = false;
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().IsScreenRotationLocked(isLocked);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, DMError::DM_OK);
    } else {
        ASSERT_EQ(ret, DMError::DM_OK);
    }
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: test SetSpecifiedScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetSpecifiedScreenPower, TestSize.Level1)
{
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    bool ret = SingletonContainer::Get<ScreenManagerAdapter>().SetSpecifiedScreenPower(0, state, reason);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetOrientation, TestSize.Level1)
{
    Orientation orientation = Orientation::BEGIN;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetOrientation(0, orientation, false);
    ASSERT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name: WakeUpBegin
 * @tc.desc: test WakeUpBegin
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, WakeUpBegin, TestSize.Level1)
{
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    bool ret = SingletonContainer::Get<DisplayManagerAdapter>().WakeUpBegin(reason);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: WakeUpEnd
 * @tc.desc: test WakeUpEnd
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, WakeUpEnd, TestSize.Level1)
{
    bool ret = SingletonContainer::Get<DisplayManagerAdapter>().WakeUpEnd();
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: SuspendBegin
 * @tc.desc: test SuspendBegin
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SuspendBegin, TestSize.Level1)
{
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    bool ret = SingletonContainer::Get<DisplayManagerAdapter>().SuspendBegin(reason);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SuspendEnd
 * @tc.desc: test SuspendEnd
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SuspendEnd, TestSize.Level1)
{
    bool ret = SingletonContainer::Get<DisplayManagerAdapter>().SuspendEnd();
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, MakeMirror, TestSize.Level1)
{
    std::vector<ScreenId> mirrorScreenId;
    ScreenId screenGroupId;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirror(0, mirrorScreenId, screenGroupId);
    ASSERT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: StopMirror
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, StopMirror, TestSize.Level1)
{
    std::vector<ScreenId> mirrorScreenIds;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().StopMirror(mirrorScreenIds);
    ASSERT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, HasPrivateWindow, TestSize.Level1)
{
    bool hasPrivateWindow = false;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().HasPrivateWindow(0, hasPrivateWindow);
    ASSERT_EQ(DMError::DM_OK, err);
}

/**
 * @tc.name: AddSurfaceNodeToDisplay
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, AddSurfaceNodeToDisplay, TestSize.Level1)
{
    RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = std::make_shared<RSSurfaceNode>(rsSurfaceNodeConfig, true, 0);
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().AddSurfaceNodeToDisplay(0, surfaceNode);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(DMError::DM_ERROR_NULLPTR, err);
    } else {
        ASSERT_EQ(DMError::DM_OK, err);
    }
}

/**
 * @tc.name: RemoveSurfaceNodeFromDisplay
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, RemoveSurfaceNodeFromDisplay, TestSize.Level1)
{
    RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = std::make_shared<RSSurfaceNode>(rsSurfaceNodeConfig, true, 0);
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().RemoveSurfaceNodeFromDisplay(0, surfaceNode);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(DMError::DM_ERROR_NULLPTR, err);
    } else {
        ASSERT_EQ(DMError::DM_OK, err);
    }
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, MakeExpand, TestSize.Level1)
{
    std::vector<ScreenId> screenId;
    std::vector<Point> startPoint;
    ScreenId screenGroupId;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().MakeExpand(screenId, startPoint, screenGroupId);
    ASSERT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: StopExpand
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, StopExpand, TestSize.Level1)
{
    std::vector<ScreenId> expandScreenIds;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().StopExpand(expandScreenIds);
    ASSERT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name: RemoveVirtualScreenFromGroup
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, RemoveVirtualScreenFromGroup, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    int resultValue = 0;
    std::function<void()> func = [&]() {
        SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenFromGroup(screens);
        resultValue = 1;
    };
    func();
    ASSERT_NE(resultValue, 0);
}

/**
 * @tc.name: SetScreenActiveMode
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenActiveMode, TestSize.Level1)
{
    VirtualScreenOption defaultOption = {"virtualScreen02", 480, 320, 2.0, nullptr, 0};
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    ASSERT_NE(id, SCREEN_ID_INVALID) << "Failed to create virtual screen";
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenActiveMode(id, 100);
    EXPECT_EQ(err, DMError::DM_ERROR_RENDER_SERVICE_FAILED)
        << "SetScreenActiveMode should return DM_ERROR_NULLPTR when proxy is not initialized.";
    SingletonContainer::Get<ScreenManagerAdapter>().DestroyVirtualScreen(id);
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetVirtualPixelRatio, TestSize.Level1)
{
    VirtualScreenOption defaultOption = { "virtualScreen03", 480, 320, 2.0, nullptr, 0 };
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualPixelRatio(id, 0);
    ASSERT_EQ(err, DMError::DM_OK);
    SingletonContainer::Get<ScreenManagerAdapter>().DestroyVirtualScreen(id);
}

/**
 * @tc.name: SetResolution
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetResolution, TestSize.Level1)
{
    VirtualScreenOption defaultOption = { "virtualScreen04", 480, 320, 2.0, nullptr, 0 };
    ScreenId id = ScreenManager::GetInstance().CreateVirtualScreen(defaultOption);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetResolution(id, 70, 100, 1);
    ASSERT_EQ(err, DMError::DM_ERROR_IPC_FAILED);
    SingletonContainer::Get<ScreenManagerAdapter>().DestroyVirtualScreen(id);
}

/**
 * @tc.name: ResizeVirtualScreen
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, ResizeVirtualScreen, TestSize.Level1)
{
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().ResizeVirtualScreen(0, 70, 100);
    ASSERT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name: MakeUniqueScreen
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, MakeUniqueScreen, TestSize.Level1)
{
    std::vector<ScreenId> screenIds;
    std::vector<DisplayId> displayIds;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().MakeUniqueScreen(screenIds, displayIds,
        UniqueScreenRotationOptions());
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
    }
}

/**
 * @tc.name: GetAvailableArea
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetAvailableArea, TestSize.Level1)
{
    DMRect area{};
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetAvailableArea(0, area);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetAllDisplayPhysicalResolution
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetAllDisplayPhysicalResolution, TestSize.Level1)
{
    std::vector<DisplayPhysicalResolution> allSize =
        SingletonContainer::Get<DisplayManagerAdapter>().GetAllDisplayPhysicalResolution();
    ASSERT_FALSE(allSize.empty());
}

/**
 * @tc.name: SetDisplayScale
 * @tc.desc: SetDisplayScale test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetDisplayScale, TestSize.Level1)
{
    auto& displayManagerAdapter = SingletonContainer::Get<DisplayManagerAdapter>();
    constexpr float scaleX = 1.0f;
    constexpr float scaleY = 1.0f;
    constexpr float pivotX = 0.5f;
    constexpr float pivotY = 0.5f;
    sptr<DisplayInfo> displayInfo = displayManagerAdapter.GetDefaultDisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    ScreenId screenId = displayInfo->GetScreenId();
    displayManagerAdapter.SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
}

/**
 * @tc.name: ForceSetFoldStatusAndLock
 * @tc.desc: ForceSetFoldStatusAndLock test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, ForceSetFoldStatusAndLock, TestSize.Level1)
{
    sptr<IScreenSessionManager> screenSessionManagerServiceProxyTmp =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError ret = SingletonContainer::Get<DisplayManagerAdapter>().ForceSetFoldStatusAndLock(FoldStatus::FOLDED);
    EXPECT_EQ(ret, DMError::DM_OK);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxyTmp;
}

/**
 * @tc.name: SetScreenSkipProtectedWindow
 * @tc.desc: SetScreenSkipProtectedWindow test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenSkipProtectedWindow, TestSize.Level1)
{
    const std::vector<ScreenId> screenIds = { 1001, 1002 };
    bool isEnable = true;
    auto result = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenSkipProtectedWindow(screenIds, isEnable);
    ASSERT_EQ(result, DMError::DM_OK);
}

/**
 * @tc.name: GetDisplayCapability
 * @tc.desc: GetDisplayCapability test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetDisplayCapability, TestSize.Level1)
{
    std::string capabilitInfo;
    auto result = SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayCapability(capabilitInfo);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(result, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetVisibleAreaDisplayInfoById
 * @tc.desc: Test GetVisibleAreaDisplayInfoById function with invalid displayId.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetVisibleAreaDisplayInfoById, TestSize.Level1)
{
    sptr<DisplayInfo> info =
        SingletonContainer::Get<DisplayManagerAdapter>().GetVisibleAreaDisplayInfoById(DISPLAY_ID_INVALID);
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name: GetExpandAvailableArea
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetExpandAvailableArea, TestSize.Level1)
{
    DMRect area;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetExpandAvailableArea(0, area);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        EXPECT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: SetVirtualScreenAutoRotation
 * @tc.desc: SetVirtualScreenAutoRotation test invalid ID
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetVirtualScreenAutoRotation, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1111;
    bool enable = false;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenAutoRotation(screenId, enable);
    EXPECT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetVirtualScreenAutoRotation02
 * @tc.desc: SetVirtualScreenAutoRotation test main screen ID
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetVirtualScreenAutoRotation02, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 0;
    bool enable = false;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenAutoRotation(screenId, enable);
    EXPECT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetDisplayHDRSnapshotWithOption001
 * @tc.desc: GetDisplayHDRSnapshotWithOption test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetDisplayHDRSnapshotWithOption001, TestSize.Level1)
{
    CaptureOption captureOption;
    DmErrorCode errorCode;

    sptr<IScreenSessionManager> screenSessionManagerServiceProxyTmp =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayHDRSnapshotWithOption(captureOption, errorCode);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxyTmp;
    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}

/**
 * @tc.name: GetDisplayHDRSnapshot001
 * @tc.desc: GetDisplayHDRSnapshot test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetDisplayHDRSnapshot001, TestSize.Level1)
{
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;

    sptr<IScreenSessionManager> screenSessionManagerServiceProxyTmp =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayHDRSnapshot(
            validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxyTmp;

    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}

/**
 * @tc.name: GetDisplayHDRSnapshot002
 * @tc.desc: GetDisplayHDRSnapshot test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetDisplayHDRSnapshot002, TestSize.Level1)
{
    DisplayId validDisplayId = DEFAULT_DISPLAY;
    DmErrorCode errorCode;
    bool isUseDma = false;
    bool isCaptureFullOfScreen = false;

    sptr<IScreenSessionManager> screenSessionManagerServiceProxyTmp =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    std::vector<std::shared_ptr<Media::PixelMap>> result =
        SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayHDRSnapshot(
            validDisplayId, errorCode, isUseDma, isCaptureFullOfScreen);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxyTmp;

    EXPECT_EQ(result.size(), PIXELMAP_SIZE);
    EXPECT_EQ(result[SDR_PIXELMAP], nullptr);
    EXPECT_EQ(result[HDR_PIXELMAP], nullptr);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch
 * @tc.desc: SetScreenPrivacyWindowTagSwitch test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetScreenPrivacyWindowTagSwitch, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 0;
    std::vector<std::string> privacyWindowTag{ "test1", "test2" };
    DMError res = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenPrivacyWindowTagSwitch(
        screenId, privacyWindowTag, true);
    EXPECT_EQ(res, DMError::DM_OK);

    sptr<IScreenSessionManager> screenSessionManagerServiceProxyTmp =
        SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    res = SingletonContainer::Get<ScreenManagerAdapter>().SetScreenPrivacyWindowTagSwitch(
        screenId, privacyWindowTag, true);
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxyTmp;
    EXPECT_EQ(res, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: SetVirtualScreenAsDefault
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetVirtualScreenAsDefault, TestSize.Level1)
{
    ScreenId screenId = 0;
    bool res = SingletonContainer::Get<DisplayManagerAdapter>().SetVirtualScreenAsDefault(screenId);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: GetLiveCreaseRegion
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetLiveCreaseRegion, TestSize.Level1)
{
    FoldCreaseRegion region;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetLiveCreaseRegion(region);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        EXPECT_EQ(err, DMError::DM_OK);
    }
}

/**
 * @tc.name: GetBrightnessInfo
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetBrightnessInfo, TestSize.Level1)
{
    FoldCreaseRegion region;
    ScreenBrightnessInfo brightnessInfo;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetBrightnessInfo(0, brightnessInfo);
    EXPECT_EQ(err, DMError::DM_OK);
    EXPECT_NE(brightnessInfo.currentHeadroom, 0);
    EXPECT_NE(brightnessInfo.maxHeadroom, 0);
    EXPECT_NE(brightnessInfo.sdrNits, 0);
}

/**
 * @tc.name: GetBrightnessInfo02
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetBrightnessInfo02, TestSize.Level1)
{
    FoldCreaseRegion region;
    ScreenBrightnessInfo brightnessInfo;
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetBrightnessInfo(0, brightnessInfo);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    EXPECT_NE(brightnessInfo.currentHeadroom, 0);
    EXPECT_NE(brightnessInfo.maxHeadroom, 0);
    EXPECT_NE(brightnessInfo.sdrNits, 0);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: GetRoundedCorner
 * @tc.desc: test GetRoundedCorner
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetRoundedCorner, TestSize.Level1)
{
    std::vector<RoundedCorner> roundedCorner;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetRoundedCorner(roundedCorner, 0, 1100, 2400);
    EXPECT_EQ(err, DMError::DM_OK);
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    err = SingletonContainer::Get<DisplayManagerAdapter>().GetRoundedCorner(roundedCorner, 0, 1100, 2400);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: GetSupportsInput
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetSupportsInput, TestSize.Level1)
{
    bool supportInput;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetSupportsInput(0, supportInput);
    EXPECT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name:GetSupportsInput02
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetSupportsInput02, TestSize.Level1)
{
    bool supportInput;
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetSupportsInput(0, supportInput);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: SetSupportsInput
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetSupportsInput, TestSize.Level1)
{
    bool supportInput = false;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().SetSupportsInput(0, supportInput);
    EXPECT_EQ(err, DMError::DM_OK);
    bool supportsInputGet = true;
    err = SingletonContainer::Get<DisplayManagerAdapter>().GetSupportsInput(0, supportsInputGet);
    EXPECT_EQ(err, DMError::DM_OK);
    EXPECT_EQ(supportsInputGet, false);
}

/**
 * @tc.name:SetSupportsInput02
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, SetSupportsInput02, TestSize.Level1)
{
    bool supportInput = false;
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().SetSupportsInput(0, supportInput);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetBundleName, TestSize.Level1)
{
    std::string bundleName = "";
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetBundleName(0, bundleName);
    EXPECT_EQ(err, DMError::DM_OK);
}

/**
 * @tc.name:GetBundleName02
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, GetBundleNamee02, TestSize.Level1)
{
    std::string bundleName = "";
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().GetBundleName(0, bundleName);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, RegisterDisplayManagerAgent, TestSize.Level1)
{
    DisplayManagerAgentType displayManagerAgentType = DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER;
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err =
        SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(nullptr, displayManagerAgentType);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: UnregisterDisplayManagerAgent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, UnregisterDisplayManagerAgent, TestSize.Level1)
{
    DisplayManagerAgentType displayManagerAgentType = DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER;
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
        nullptr, displayManagerAgentType);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<DisplayManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: AddVirtualScreenWhiteList_proxyIsNull
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, AddVirtualScreenWhiteList_proxyIsNull, TestSize.Level1)
{
    ScreenId screenId = 1;
    std::vector<uint64_t> missionIds = {5, 10};
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenWhiteList(screenId, missionIds);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: AddVirtualScreenWhiteList_proxyNotNull
 * @tc.desc: test screenSessionManagerServiceProxy_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, AddVirtualScreenWhiteList_proxyNotNull, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 1;
    std::vector<uint64_t> missionIds = {5, 10};
    SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenWhiteList(screenId, missionIds);
    EXPECT_TRUE(g_errLog.find("Device not support") == std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RemoveVirtualScreenWhiteList_proxyIsNull
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, RemoveVirtualScreenWhiteList_proxyIsNull, TestSize.Level1)
{
    ScreenId screenId = 1;
    std::vector<uint64_t> missionIds = {5, 10};
    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenWhiteList(screenId, missionIds);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: RemoveVirtualScreenWhiteList_proxyNotNull
 * @tc.desc: test screenSessionManagerServiceProxy_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTest, RemoveVirtualScreenWhiteList_proxyNotNull, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 1;
    std::vector<uint64_t> missionIds = {5, 10};
    SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenWhiteList(screenId, missionIds);
    EXPECT_TRUE(g_errLog.find("Device not support") == std::string::npos);
    LOG_SetCallback(nullptr);
}
} // namespace
} // namespace OHOS::Rosen
