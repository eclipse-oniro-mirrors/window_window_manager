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

#include "screen_session.h"
#include <gtest/gtest.h>
#include "screen_session_manager/include/screen_session_manager.h"
#include "scene_board_judgement.h"

// using namespace FRAME_TRACE;
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class MockScreenChangeListener : public IScreenChangeListener {
public:
    void OnConnect(ScreenId screenId) override {}
    void OnDisconnect(ScreenId screenId) override {}
    void OnPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
        ScreenId screenId) override {}
    void OnPowerStatusChange(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason) override {}
    void OnSensorRotationChange(float sensorRotation, ScreenId screenId) override {}
    void OnScreenOrientationChange(float screenOrientation, ScreenId screenId) override {}
    void OnScreenRotationLockedChange(bool isLocked, ScreenId screenId) override {}
    void OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId) override {}
    void OnHoverStatusChange(int32_t hoverStatus, ScreenId screenId) override {}
    void OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName,
        uint32_t tokenId, const std::vector<std::string>& permissions) override {}
    void OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus) override {}
};
class ScreenSessionTest : public testing::Test {
  public:
    ScreenSessionTest() {}
    ~ScreenSessionTest() {}
};

namespace {

/**
 * @tc.name: create ScreenSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenSession, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    EXPECT_NE(nullptr, screenSession);
}

/**
 * @tc.name: create ScreenSession02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenSession02, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
}

/**
 * @tc.name: create ScreenSession03
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenSession03, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    EXPECT_NE(nullptr, screenSession);
}

/**
 * @tc.name: create ScreenSession03
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenSession04, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    EXPECT_NE(nullptr, screenSession);
}

/**
 * @tc.name: create ScreenSession05
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenSession05, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::INVALID);
    EXPECT_NE(nullptr, screenSession);
}

/**
 * @tc.name: CreateDisplayNode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CreateDisplayNode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: CreateDisplayNode start";
    Rosen::RSDisplayNodeConfig rsConfig;
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    rsConfig.isMirrored = true;
    rsConfig.screenId = 101;
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    screenSession->CreateDisplayNode(rsConfig);
    GTEST_LOG_(INFO) << "ScreenSessionTest: CreateDisplayNode end";
}

/**
 * @tc.name: SetMirrorScreenType
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetMirrorScreenType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMirrorScreenType start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    Rosen::RSDisplayNodeConfig rsConfig;
    rsConfig.isMirrored = true;
    rsConfig.screenId = 101;
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    MirrorScreenType mirrorType = MirrorScreenType::VIRTUAL_MIRROR;
    screenSession->SetMirrorScreenType(mirrorType);
    GTEST_LOG_(INFO) << "SetMirrorScreenType end";
}

/**
 * @tc.name: GetMirrorScreenType
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetMirrorScreenType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMirrorScreenType start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    MirrorScreenType mirrorType = MirrorScreenType::VIRTUAL_MIRROR;
    screenSession->SetMirrorScreenType(mirrorType);
    MirrorScreenType res = screenSession->GetMirrorScreenType();
    ASSERT_EQ(mirrorType, res);
    GTEST_LOG_(INFO) << "GetMirrorScreenType end";
}

/**
 * @tc.name: SetDefaultDeviceRotationOffset
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetDefaultDeviceRotationOffset, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDefaultDeviceRotationOffset start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    uint32_t defaultRotationOffset = 0;
    screenSession->SetDefaultDeviceRotationOffset(defaultRotationOffset);
    GTEST_LOG_(INFO) << "SetDefaultDeviceRotationOffset end";
}

/**
 * @tc.name: UpdateDisplayState
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateDisplayState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateDisplayState start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DisplayState displayState = DisplayState::OFF;
    screenSession->UpdateDisplayState(displayState);
    GTEST_LOG_(INFO) << "UpdateDisplayState end";
}

/**
 * @tc.name: UpdateRefreshRate
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRefreshRate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateRefreshRate start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    uint32_t refreshRate = 2;
    screenSession->UpdateRefreshRate(refreshRate);
    GTEST_LOG_(INFO) << "UpdateRefreshRate end";
}

/**
 * @tc.name: GetRefreshRate
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetRefreshRate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRefreshRate start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    uint32_t refreshRate = 2;
    screenSession->UpdateRefreshRate(refreshRate);
    auto res = screenSession->GetRefreshRate();
    ASSERT_EQ(refreshRate, res);
    GTEST_LOG_(INFO) << "GetRefreshRate end";
}

/**
 * @tc.name: UpdatePropertyByResolution
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByResolution, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePropertyByResolution start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    uint32_t width = 1000;
    uint32_t height = 1500;
    screenSession->UpdatePropertyByResolution(width, height);
    GTEST_LOG_(INFO) << "UpdatePropertyByResolution end";
}

/**
 * @tc.name: HandleSensorRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, HandleSensorRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleSensorRotation start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    float sensorRotation = 0.0f;
    screenSession->HandleSensorRotation(sensorRotation);
    GTEST_LOG_(INFO) << "HandleSensorRotation end";
}

/**
 * @tc.name: ConvertIntToRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ConvertIntToRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertIntToRotation start";
    sptr<ScreenSession> session = new(std::nothrow) ScreenSession();
    int rotation = 0;
    auto res = session->ConvertIntToRotation(rotation);
    ASSERT_EQ(res, Rotation::ROTATION_0);
    rotation = 90;
    res = session->ConvertIntToRotation(rotation);
    ASSERT_EQ(res, Rotation::ROTATION_90);
    rotation = 180;
    res = session->ConvertIntToRotation(rotation);
    ASSERT_EQ(res, Rotation::ROTATION_180);
    rotation = 270;
    res = session->ConvertIntToRotation(rotation);
    ASSERT_EQ(res, Rotation::ROTATION_270);
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertIntToRotation end";
}

/**
 * @tc.name: SetVirtualScreenFlag
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetVirtualScreenFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetVirtualScreenFlag start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    VirtualScreenFlag screenFlag = VirtualScreenFlag::CAST;
    screenSession->SetVirtualScreenFlag(screenFlag);
    GTEST_LOG_(INFO) << "SetVirtualScreenFlag end";
}

/**
 * @tc.name: GetVirtualScreenFlag
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetVirtualScreenFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetVirtualScreenFlag start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    VirtualScreenFlag screenFlag = VirtualScreenFlag::CAST;
    screenSession->SetVirtualScreenFlag(screenFlag);
    auto res = screenSession->GetVirtualScreenFlag();
    ASSERT_EQ(screenFlag, res);
    GTEST_LOG_(INFO) << "GetVirtualScreenFlag end";
}

/**
 * @tc.name: SetPhysicalRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetPhysicalRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPhysicalRotation start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    int rotation = 0;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::UNKNOWN;
    screenSession->SetPhysicalRotation(rotation, foldDisplayMode);
    GTEST_LOG_(INFO) << "SetPhysicalRotation end";
}

/**
 * @tc.name: SetScreenComponentRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenComponentRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetScreenComponentRotation start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    int rotation = 0;
    screenSession->SetScreenComponentRotation(rotation);
    GTEST_LOG_(INFO) << "SetScreenComponentRotation end";
}

/**
 * @tc.name: UpdateToInputManager
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateToInputManager, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateToInputManager start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    RRect bounds;
    int rotation = 90;
    int deviceRotation = 90;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::FULL;
    screenSession->UpdateToInputManager(bounds, rotation, deviceRotation, foldDisplayMode);
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    rotation = 0;
    foldDisplayMode = FoldDisplayMode::MAIN;
    screenSession->UpdateToInputManager(bounds, rotation, deviceRotation, foldDisplayMode);
    GTEST_LOG_(INFO) << "UpdateToInputManager end";
}

/**
 * @tc.name: UpdatePropertyAfterRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyAfterRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePropertyAfterRotation start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    int rotation = 90;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::MAIN;
    screenSession->UpdatePropertyAfterRotation(bounds, rotation, foldDisplayMode);
    GTEST_LOG_(INFO) << "UpdatePropertyAfterRotation end";
}

/**
 * @tc.name: ReportNotifyModeChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ReportNotifyModeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportNotifyModeChange start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DisplayOrientation displayOrientation = DisplayOrientation::LANDSCAPE;
    screenSession->ReportNotifyModeChange(displayOrientation);
    displayOrientation = DisplayOrientation::PORTRAIT;
    screenSession->ReportNotifyModeChange(displayOrientation);
    displayOrientation = DisplayOrientation::PORTRAIT_INVERTED;
    screenSession->ReportNotifyModeChange(displayOrientation);
    GTEST_LOG_(INFO) << "ReportNotifyModeChange end";
}

/**
 * @tc.name: SuperFoldStatusChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SuperFoldStatusChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SuperFoldStatusChange start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    ScreenId screenId = 0;
    SuperFoldStatus superFoldStatus = SuperFoldStatus::UNKNOWN;
    screenSession->SuperFoldStatusChange(screenId, superFoldStatus);
    superFoldStatus = SuperFoldStatus::EXPANDED;
    screenSession->SuperFoldStatusChange(screenId, superFoldStatus);
    superFoldStatus = SuperFoldStatus::FOLDED;
    screenSession->SuperFoldStatusChange(screenId, superFoldStatus);
    superFoldStatus = SuperFoldStatus::KEYBOARD;
    screenSession->SuperFoldStatusChange(screenId, superFoldStatus);
    superFoldStatus = SuperFoldStatus::HALF_FOLDED;
    screenSession->SuperFoldStatusChange(screenId, superFoldStatus);
    GTEST_LOG_(INFO) << "SuperFoldStatusChange end";
}

/**
 * @tc.name: UpdateRotationAfterBoot01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRotationAfterBoot01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateRotationAfterBoot start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    bool foldToExpand = true;
    screenSession->UpdateRotationAfterBoot(foldToExpand);
    GTEST_LOG_(INFO) << "UpdateRotationAfterBoot end";
}

/**
 * @tc.name: UpdateRotationAfterBoot02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRotationAfterBoot02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateRotationAfterBoot start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    bool foldToExpand = false;
    screenSession->UpdateRotationAfterBoot(foldToExpand);
    GTEST_LOG_(INFO) << "UpdateRotationAfterBoot end";
}

/**
 * @tc.name: UpdateValidRotationToScb
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateValidRotationToScb, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateValidRotationToScb start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    screenSession->UpdateValidRotationToScb();
    GTEST_LOG_(INFO) << "UpdateValidRotationToScb end";
}

/**
 * @tc.name: SetScreenSceneDpiChangeListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenSceneDpiChangeListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetScreenSceneDpiChangeListener start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    SetScreenSceneDpiFunc func = nullptr;
    screenSession->SetScreenSceneDpiChangeListener(func);
    GTEST_LOG_(INFO) << "SetScreenSceneDpiChangeListener end";
}

/**
 * @tc.name: SetScreenSceneDestroyListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenSceneDestroyListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetScreenSceneDestroyListener start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DestroyScreenSceneFunc func = nullptr;
    screenSession->SetScreenSceneDestroyListener(func);
    GTEST_LOG_(INFO) << "SetScreenSceneDestroyListener end";
}

/**
 * @tc.name: SetScreenSceneDpi
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenSceneDpi, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetScreenSceneDpi start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    float density = 1.5f;
    SetScreenSceneDpiFunc func = nullptr;
    screenSession->SetScreenSceneDpiChangeListener(func);
    screenSession->SetScreenSceneDpi(density);
    SetScreenSceneDpiFunc func2 = [](float density) {
            EXPECT_TRUE(true);
        };
    screenSession->SetScreenSceneDpiChangeListener(func2);
    screenSession->SetScreenSceneDpi(density);
    GTEST_LOG_(INFO) << "SetScreenSceneDpi end";
}

/**
 * @tc.name: DestroyScreenScene
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, DestroyScreenScene, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyScreenScene start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DestroyScreenSceneFunc func = nullptr;
    screenSession->SetScreenSceneDestroyListener(func);
    screenSession->DestroyScreenScene();
    DestroyScreenSceneFunc func2 = []() {
            EXPECT_TRUE(true);
        };
    screenSession->SetScreenSceneDestroyListener(func2);
    screenSession->DestroyScreenScene();
    GTEST_LOG_(INFO) << "DestroyScreenScene end";
}

/**
 * @tc.name: SetDensityInCurResolution
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetDensityInCurResolution, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDensityInCurResolution start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    float densityInCurResolution = 1.5f;
    screenSession->SetDensityInCurResolution(densityInCurResolution);
    GTEST_LOG_(INFO) << "SetDensityInCurResolution end";
}

/**
 * @tc.name: GetSourceMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSourceMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode start";
    sptr<ScreenSession> session = new(std::nothrow) ScreenSession();
    session->screenId_ = session->defaultScreenId_;
    ScreenSourceMode mode = session->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MAIN);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_ALONE);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXPAND);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_EXTEND);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MIRROR);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_UNIQUE);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_UNIQUE);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode end";
}

/**
 * @tc.name: GetSourceMode02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSourceMode02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode02 start";
    sptr<ScreenSession> session = new(std::nothrow) ScreenSession();
    session->screenId_ = session->defaultScreenId_;
    ScreenSourceMode mode = session->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MAIN);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);

    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MAIN);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_EXTEND);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode02 end";
}

/**
 * @tc.name: GetPixelFormat
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetPixelFormat, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPixelFormat start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    GraphicPixelFormat pixelFormat;
    auto res = screenSession->GetPixelFormat(pixelFormat);
    ASSERT_EQ(res, DMError::DM_OK);
    GTEST_LOG_(INFO) << "GetPixelFormat end";
}

/**
 * @tc.name: SetPixelFormat
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetPixelFormat, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPixelFormat start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    GraphicPixelFormat pixelFormat = GraphicPixelFormat{GRAPHIC_PIXEL_FMT_CLUT8};
    auto res = screenSession->SetPixelFormat(pixelFormat);
    ASSERT_EQ(res, DMError::DM_OK);
    GTEST_LOG_(INFO) << "SetPixelFormat end";
}

/**
 * @tc.name: GetSupportedHDRFormats
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSupportedHDRFormats, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSupportedHDRFormats start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    std::vector<ScreenHDRFormat> hdrFormats;
    auto res = screenSession->GetSupportedHDRFormats(hdrFormats);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "GetSupportedHDRFormats end";
}

/**
 * @tc.name: GetScreenHDRFormat
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenHDRFormat, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetScreenHDRFormat start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    ScreenHDRFormat hdrFormat;
    auto res = screenSession->GetScreenHDRFormat(hdrFormat);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "GetScreenHDRFormat end";
}

/**
 * @tc.name: SetScreenHDRFormat
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenHDRFormat, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetScreenHDRFormat start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    int32_t modeIdx = 0;
    auto res = screenSession->SetScreenHDRFormat(modeIdx);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
        modeIdx = -1;
        res = screenSession->SetScreenHDRFormat(modeIdx);
        ASSERT_EQ(res, DMError::DM_ERROR_INVALID_PARAM);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "SetScreenHDRFormat end";
}

/**
 * @tc.name: GetSupportedColorSpaces
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSupportedColorSpaces, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSupportedColorSpaces start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    std::vector<GraphicCM_ColorSpaceType> colorSpaces;
    auto res = screenSession->GetSupportedColorSpaces(colorSpaces);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "GetSupportedColorSpaces end";
}

/**
 * @tc.name: GetScreenColorSpace
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenColorSpace, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetScreenColorSpace start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    GraphicCM_ColorSpaceType colorSpace;
    auto res = screenSession->GetScreenColorSpace(colorSpace);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "GetScreenColorSpace end";
}

/**
 * @tc.name: SetScreenColorSpace
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenColorSpace, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetScreenColorSpace start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType{GRAPHIC_CM_COLORSPACE_NONE};
    screenSession->SetScreenColorSpace(colorSpace);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "SetScreenColorSpace end";
}

/**
 * @tc.name: SetPrivateSessionForeground
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetPrivateSessionForeground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPrivateSessionForeground start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    bool hasPrivate = true;
    screenSession->SetPrivateSessionForeground(hasPrivate);
    auto res = screenSession->HasPrivateSessionForeground();
    ASSERT_EQ(res, hasPrivate);
    GTEST_LOG_(INFO) << "SetPrivateSessionForeground end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
