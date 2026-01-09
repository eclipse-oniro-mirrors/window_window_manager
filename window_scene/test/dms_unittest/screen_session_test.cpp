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
#include <gmock/gmock.h>

#include "fold_screen_state_internel.h"
#include "scene_board_judgement.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "window_manager_hilog.h"

// using namespace FRAME_TRACE;
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
    g_errLog = msg;
    }
    void MyLogCallbackWithAllLog(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog += msg;
    }
}

class MockScreenChangeListener : public IScreenChangeListener {
public:
    MOCK_METHOD(void, OnConnect, (ScreenId screenId), (override));
    MOCK_METHOD(void, OnDisconnect, (ScreenId screenId), (override));
    MOCK_METHOD(void, OnPropertyChange,
        (const ScreenProperty& newProperty, ScreenPropertyChangeReason reason, ScreenId screenId), (override));
    MOCK_METHOD(void, OnPowerStatusChange,
        (DisplayPowerEvent event, EventStatus status, PowerStateChangeReason reason), (override));
    MOCK_METHOD(void, OnSensorRotationChange,
        (float sensorRotation, ScreenId screenId, bool isSwitchUser), (override));
    MOCK_METHOD(void, OnScreenOrientationChange, (float screenOrientation, ScreenId screenId), (override));
    MOCK_METHOD(void, OnScreenRotationLockedChange, (bool isLocked, ScreenId screenId), (override));
    MOCK_METHOD(void, OnScreenExtendChange, (ScreenId mainScreenId, ScreenId extendScreenId), (override));
    MOCK_METHOD(void, OnHoverStatusChange, (int32_t hoverStatus, bool needRotate, ScreenId screenId), (override));
    MOCK_METHOD(void, OnScreenCaptureNotify,
        (ScreenId mainScreenId, int32_t uid, const std::string& clientName), (override));
    MOCK_METHOD(void, OnCameraBackSelfieChange, (bool isCameraBackSelfie, ScreenId screenId), (override));
    MOCK_METHOD(void, OnSuperFoldStatusChange, (ScreenId screenId, SuperFoldStatus superFoldStatus), (override));
    MOCK_METHOD(void, OnSecondaryReflexionChange, (ScreenId screenId, bool isSecondaryReflexion), (override));
    MOCK_METHOD(void, OnExtendScreenConnectStatusChange,
        (ScreenId screenId, ExtendScreenConnectStatus extendScreenConnectStatus), (override));
    MOCK_METHOD(void, OnBeforeScreenPropertyChange, (FoldStatus foldStatus), (override));
    MOCK_METHOD(void, OnScreenModeChange, (ScreenModeChangeEvent screenModeChangeEvent), (override));
};

class ScreenSessionTest : public testing::Test {
  public:
    void TearDown() override;
    ScreenSessionTest() {}
    ~ScreenSessionTest() {}
};

void ScreenSessionTest::TearDown()
{
    usleep(SLEEP_TIME_IN_US);
}

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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::INVALID);
    EXPECT_NE(nullptr, screenSession);
}

/**
 * @tc.name: CreateDisplayNode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CreateDisplayNode, TestSize.Level0)
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
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
HWTEST_F(ScreenSessionTest, SetVirtualScreenFlag, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetVirtualScreenFlag start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
HWTEST_F(ScreenSessionTest, GetVirtualScreenFlag, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetVirtualScreenFlag start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    int rotation = 0;
    screenSession->SetPhysicalRotation(rotation);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
HWTEST_F(ScreenSessionTest, GetSourceMode, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->screenId_ = session->defaultScreenId_;
    ScreenSourceMode mode = session->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MAIN);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_EXTEND);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    mode = screenSession->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MAIN);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode end";
}

/**
 * @tc.name: GetSourceMode02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSourceMode02, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode02 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->screenId_ = session->defaultScreenId_;
    ScreenSourceMode mode = session->GetSourceMode();
    ASSERT_EQ(mode, ScreenSourceMode::SCREEN_MAIN);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);

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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    GTEST_LOG_(INFO) << "GetSupportedHDRFormats start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    std::vector<ScreenHDRFormat> hdrFormats;
    auto res = screenSession->GetSupportedHDRFormats(hdrFormats);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "GetSupportedHDRFormats end";
#endif
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    GTEST_LOG_(INFO) << "SetScreenHDRFormat start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
#endif
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
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
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    GraphicCM_ColorSpaceType colorSpace = GraphicCM_ColorSpaceType{GRAPHIC_CM_COLORSPACE_NONE};
    screenSession->SetScreenColorSpace(colorSpace);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "SetScreenColorSpace end";
}

/**
 * @tc.name: GetScreenCombination
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenCombination, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetScreenCombination start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    ScreenCombination combination { ScreenCombination::SCREEN_ALONE };
    screenSession->SetScreenCombination(combination);
    auto res = screenSession->GetScreenCombination();
    ASSERT_EQ(res, combination);
    GTEST_LOG_(INFO) << "GetScreenCombination end";
}

/**
 * @tc.name: Resize001
 * @tc.desc: Resize001 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, Resize001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Resize001 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(nullptr, screenSession);
    uint32_t width = 1000;
    uint32_t height = 1000;
    screenSession->Resize(width, height);
    GTEST_LOG_(INFO) << "Resize001 end";
}

/**
 * @tc.name: Resize002
 * @tc.desc: Resize002 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, Resize002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Resize002 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(nullptr, screenSession);
    uint32_t width = 1000;
    uint32_t height = 1000;
    screenSession->Resize(width, height, false);
    GTEST_LOG_(INFO) << "Resize002 end";
}

/**
 * @tc.name: UpdateAvailableArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateAvailableArea, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UpdateAvailableArea start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DMRect area = screenSession->GetAvailableArea();
    auto res = screenSession->UpdateAvailableArea(area);
    ASSERT_EQ(res, false);
    area = {2, 2, 2, 2};
    res = screenSession->UpdateAvailableArea(area);
    ASSERT_EQ(res, true);
    GTEST_LOG_(INFO) << "UpdateAvailableArea end";
}

/**
 * @tc.name: SetAvailableArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetAvailableArea, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAvailableArea start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DMRect area = {2, 2, 2, 2};
    screenSession->SetAvailableArea(area);
    GTEST_LOG_(INFO) << "SetAvailableArea end";
}

/**
 * @tc.name: GetAvailableArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetAvailableArea, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAvailableArea start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DMRect area = {2, 2, 2, 2};
    screenSession->SetAvailableArea(area);
    auto res = screenSession->GetAvailableArea();
    ASSERT_EQ(res, area);
    GTEST_LOG_(INFO) << "GetAvailableArea end";
}

/**
 * @tc.name: SetFoldScreen
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetFoldScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFoldScreen start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    bool isFold = false;
    screenSession->SetFoldScreen(isFold);
    GTEST_LOG_(INFO) << "SetFoldScreen end";
}

/**
 * @tc.name: SetHdrFormats
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetHdrFormats, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetHdrFormats start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    std::vector<uint32_t> hdrFormats = { 0, 0, 0, 0 };
    screenSession->SetHdrFormats(std::move(hdrFormats));
    GTEST_LOG_(INFO) << "SetHdrFormats end";
}

/**
 * @tc.name: SetColorSpaces
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetColorSpaces, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetColorSpaces start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    std::vector<uint32_t> colorSpaces = { 0, 0, 0, 0 };
    screenSession->SetColorSpaces(std::move(colorSpaces));
    GTEST_LOG_(INFO) << "SetColorSpaces end";
}

/**
 * @tc.name: UpdatePropertyByActiveModeChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByActiveModeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePropertyByActiveModeChange start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->modes_.clear();
    sptr<SupportedScreenModes> info = new(std::nothrow) SupportedScreenModes();
    ASSERT_NE(info, nullptr);
    info->id_ = 0;
    info->width_ = 1920;
    info->height_ = 1080;
    info->refreshRate_ = 60;

    session->modes_.push_back(info);
    session->SetActiveId(-1);
    session->UpdatePropertyByActiveModeChange();
    EXPECT_TRUE(g_errLog.find("mode is null") != std::string::npos);
    session->SetActiveId(0);
    session->UpdatePropertyByActiveModeChange();
    EXPECT_TRUE(g_errLog.find("active mode bounds") != std::string::npos);
}

/**
 * @tc.name: UpdatePropertyByScreenMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByScreenMode01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    RSScreenModeInfo screenMode;
    session->UpdatePropertyByScreenMode(screenMode);
    EXPECT_TRUE(g_errLog.find("Invalid RSScreenModeInfo") != std::string::npos);
    g_errLog.clear();
}

/**
 * @tc.name: UpdatePropertyByScreenMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByScreenMode02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    RSScreenModeInfo screenMode;
    screenMode.SetScreenHeight(2440);
    screenMode.SetScreenWidth(2240);
    screenMode.SetScreenRefreshRate(90);
    screenMode.SetScreenModeId(0);
    session->UpdatePropertyByScreenMode(screenMode);
    ASSERT_EQ(session->GetScreenProperty().GetBounds().rect_.width_, 2240);
    ASSERT_EQ(session->GetScreenProperty().GetBounds().rect_.height_, 2440);
}

/**
 * @tc.name: SetSupportedRefreshRate
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetSupportedRefreshRate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetSupportedRefreshRate start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    std::vector<uint32_t> supportedRefreshRate = { 0, 0, 0, 0 };
    std::vector<uint32_t> supportedRefreshRate_cmp = { 0, 0, 0, 0 };
    screenSession->SetSupportedRefreshRate(std::move(supportedRefreshRate));
    EXPECT_EQ(screenSession->supportedRefreshRate_, supportedRefreshRate_cmp);
    GTEST_LOG_(INFO) << "SetSupportedRefreshRate end";
}

/**
 * @tc.name: SetDisplayNodeScreenId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetDisplayNodeScreenId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetDisplayNodeScreenId start";
    ScreenId screenId = 0;
    ScreenId rsId = 1;
    std::string name = "OpenHarmony";
    ScreenProperty property;
    RSDisplayNodeConfig displayNodeConfig;
    std::shared_ptr<RSDisplayNode> displayNode = RSDisplayNode::Create(displayNodeConfig);
    EXPECT_NE(nullptr, displayNode);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, rsId, name, property, displayNode);
    session->SetDisplayNodeScreenId(screenId);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetDisplayNodeScreenId end";
}

/**
 * @tc.name: UnregisterScreenChangeListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UnregisterScreenChangeListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UnregisterScreenChangeListener start";
    IScreenChangeListener* screenChangeListener = nullptr;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->UnregisterScreenChangeListener(screenChangeListener);
    EXPECT_FALSE(g_errLog.find("Failed to unregister screen change listener, listener is null!") != std::string::npos);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UnregisterScreenChangeListener end";
}

/**
 * @tc.name: UnregisterScreenChangeListener02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UnregisterScreenChangeListener02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UnregisterScreenChangeListener02 start";
    IScreenChangeListener* screenChangeListener = new ScreenSessionManager();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->UnregisterScreenChangeListener(screenChangeListener);
    auto it = std::find(session->screenChangeListenerList_.begin(),
        session->screenChangeListenerList_.end(), screenChangeListener);
    bool isStillRegistered = (it != session->screenChangeListenerList_.end());
    ASSERT_FALSE(isStillRegistered);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UnregisterScreenChangeListener02 end";
}

/**
 * @tc.name: ConvertToDisplayInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ConvertToDisplayInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertToDisplayInfo start";
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    EXPECT_NE(displayInfo, nullptr);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session->ConvertToDisplayInfo());

    sptr<ScreenInfo> info = sptr<ScreenInfo>::MakeSptr();
    EXPECT_NE(info, nullptr);
    EXPECT_NE(nullptr, session->ConvertToScreenInfo());
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertToDisplayInfo end";
}

/**
 * @tc.name: SetMirrorScreenRegion
 * @tc.desc: SetMirrorScreenRegion test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetMirrorScreenRegion, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetMirrorScreenRegion start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    DMRect area = {2, 2, 2, 2};
    ScreenId screenId = 0;
    session->SetMirrorScreenRegion(screenId, area);
    std::pair<ScreenId, DMRect> mode1 = session->GetMirrorScreenRegion();
    EXPECT_EQ(screenId, mode1.first);
    EXPECT_EQ(area, mode1.second);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetMirrorScreenRegion end";
}

/**
 * @tc.name: GetScreenSupportedColorGamuts
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenSupportedColorGamuts, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenSupportedColorGamuts start";
    std::vector<ScreenColorGamut> colorGamuts;
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);
    DMError ret = screenSession->GetScreenSupportedColorGamuts(colorGamuts);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, DMError::DM_OK);
    } else {
        ASSERT_NE(ret, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenSupportedColorGamuts end";
}

/**
 * @tc.name: GetActiveScreenMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetActiveScreenMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetActiveScreenMode start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->activeIdx_ = -1;
    sptr<SupportedScreenModes> mode1 = session->GetActiveScreenMode();
    session->GetActiveScreenMode();
    ASSERT_EQ(mode1, nullptr);

    session->activeIdx_ = 100;
    sptr<SupportedScreenModes> mode2 = session->GetActiveScreenMode();
    session->GetActiveScreenMode();
    ASSERT_EQ(mode2, nullptr);

    GTEST_LOG_(INFO) << "ScreenSessionTest: GetActiveScreenMode end";
}

/**
 * @tc.name: SetScreenCombination
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenCombination, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenCombination start";
    ScreenId screenId = 2024;
    ScreenProperty property;
    NodeId nodeId = 0;
    ScreenId defaultScreenId = 0;
    ScreenCombination combination { ScreenCombination::SCREEN_ALONE };
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, property, nodeId, defaultScreenId);
    session->SetScreenCombination(combination);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(combination, session->GetScreenCombination());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenCombination end";
}

/**
 * @tc.name: GetScreenColorGamut
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenColorGamut, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenColorGamut start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);
    ScreenColorGamut colorGamut;
    DMError res = screenSession->GetScreenColorGamut(colorGamut);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, DMError::DM_OK);
    } else {
        ASSERT_NE(res, DMError::DM_OK);
    }
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenColorGamut end";
#endif
}

/**
 * @tc.name: SetScreenColorGamut01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenColorGamut01, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenColorGamut start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    int32_t colorGamut = 1;
    DMError res = session->SetScreenColorGamut(colorGamut);
    ASSERT_EQ(res, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenColorGamut end";
#endif
}

/**
 * @tc.name: SetScreenColorGamut02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenColorGamut02, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenColorGamut start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    int32_t colorGamut = -1;
    DMError res = session->SetScreenColorGamut(colorGamut);
    EXPECT_NE(res, DMError::DM_OK);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenColorGamut end";
#endif
}

/**
 * @tc.name: GetScreenGamutMap
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenGamutMap, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenGamutMap start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    ScreenGamutMap gamutMap;
    DMError res = session->GetScreenGamutMap(gamutMap);
    ASSERT_EQ(res, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenGamutMap end";
#endif
}

/**
 * @tc.name: SetScreenGamutMap
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenGamutMap, TestSize.Level1)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenGamutMap start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    ScreenGamutMap gamutMap = GAMUT_MAP_CONSTANT;
    DMError res = session->SetScreenGamutMap(gamutMap);
    ASSERT_EQ(res, DMError::DM_ERROR_RENDER_SERVICE_FAILED);

    gamutMap = GAMUT_MAP_HDR_EXTENSION;
    res = session->SetScreenGamutMap(gamutMap);
    ASSERT_EQ(res, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenGamutMap end";
#endif
}

/**
 * @tc.name: InitRSDisplayNode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, InitRSDisplayNode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: InitRSDisplayNode start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RSDisplayNodeConfig config;
    Point startPoint;
    sessionGroup.InitRSDisplayNode(config, startPoint);
    SUCCEED();
    GTEST_LOG_(INFO) << "ScreenSessionTest: InitRSDisplayNode end";
}

/**
 * @tc.name: GetRSDisplayNodeConfig
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetRSDisplayNodeConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetRSDisplayNodeConfig start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenSession> session0 = nullptr;
    RSDisplayNodeConfig config = {1};
    sptr<ScreenSession> defaultScreenSession;
    bool res = sessionGroup.GetRSDisplayNodeConfig(session0, config, defaultScreenSession);
    ASSERT_EQ(res, false);

    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    sessionGroup.combination_ = ScreenCombination::SCREEN_ALONE;
    res = sessionGroup.GetRSDisplayNodeConfig(session, config, defaultScreenSession);
    ASSERT_EQ(res, true);

    sessionGroup.combination_ = ScreenCombination::SCREEN_EXPAND;
    res = sessionGroup.GetRSDisplayNodeConfig(session, config, defaultScreenSession);
    ASSERT_EQ(res, true);

    sessionGroup.combination_ = ScreenCombination::SCREEN_MIRROR;
    res = sessionGroup.GetRSDisplayNodeConfig(session, config, defaultScreenSession);
    ASSERT_EQ(res, true);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetRSDisplayNodeConfig end";
}

/**
 * @tc.name: AddChild
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, AddChild, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: AddChild start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenSession> session0 = nullptr;
    Point startPoint(0, 0);
    sptr<ScreenSession> defaultScreenSession;
    bool res = sessionGroup.AddChild(session0, startPoint, defaultScreenSession);
    ASSERT_EQ(res, false);

    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    res = sessionGroup.AddChild(session, startPoint, defaultScreenSession, false, {Rotation::ROTATION_0, true});
    ASSERT_EQ(res, true);
    GTEST_LOG_(INFO) << "ScreenSessionTest: AddChild end";
}

/**
 * @tc.name: AddChildren01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, AddChildren01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: AddChildren start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    std::vector<sptr<ScreenSession>> smsScreens;
    std::vector<Point> startPoints;
    bool res = sessionGroup.AddChildren(smsScreens, startPoints);
    ASSERT_EQ(res, true);
    GTEST_LOG_(INFO) << "ScreenSessionTest: AddChildren end";
}

/**
 * @tc.name: AddChildren02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, AddChildren02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: AddChildren start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    std::vector<sptr<ScreenSession>> smsScreens(2);
    std::vector<Point> startPoints(1);
    bool res = sessionGroup.AddChildren(smsScreens, startPoints);
    ASSERT_EQ(res, false);
    GTEST_LOG_(INFO) << "ScreenSessionTest: AddChildren end";
}

/**
 * @tc.name: RemoveChild
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, RemoveChild, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: RemoveChild start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenSession> session0 = nullptr;
    bool res = sessionGroup.RemoveChild(session0);
    ASSERT_EQ(res, false);

    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    res = sessionGroup.RemoveChild(session);
    ASSERT_EQ(res, false);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetRSDisplayNodeConfig end";
}

/**
 * @tc.name: GetChildPosition
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetChildPosition, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetChildPosition start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    ScreenId screenId = 1;
    Point res = sessionGroup.GetChildPosition(screenId);
    ASSERT_EQ(res.posX_, 0);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetChildPosition end";
}

/**
 * @tc.name: ConvertToScreenGroupInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ConvertToScreenGroupInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertToScreenGroupInfo start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenGroupInfo> res = sessionGroup.ConvertToScreenGroupInfo();
    ASSERT_NE(res, nullptr);
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertToScreenGroupInfo end";
}

/**
 * @tc.name: RegisterScreenChangeListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, RegisterScreenChangeListener01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: RegisterScreenChangeListener start";
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->RegisterScreenChangeListener(screenChangeListener);
    auto it = std::find(session->screenChangeListenerList_.begin(),
        session->screenChangeListenerList_.end(), screenChangeListener);
    bool isRegistered = (it != session->screenChangeListenerList_.end());
    ASSERT_TRUE(isRegistered);
    GTEST_LOG_(INFO) << "ScreenSessionTest: RegisterScreenChangeListener end";
}

/**
 * @tc.name: RegisterScreenChangeListener
 * @tc.desc: Repeat to register
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, RegisterScreenChangeListener02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: RegisterScreenChangeListener start";
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    IScreenChangeListener* screenChangeListener1 = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->RegisterScreenChangeListener(screenChangeListener);
    session->RegisterScreenChangeListener(screenChangeListener1);
    ASSERT_FALSE(session->isFold_);
}

/**
 * @tc.name: RegisterScreenChangeListener
 * @tc.desc: screenState_ == ScreenState::CONNECTION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, RegisterScreenChangeListener03, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_EQ(session->screenState_, ScreenState::INIT);
    session->screenState_ = ScreenState::CONNECTION;
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    session->RegisterScreenChangeListener(screenChangeListener);
    ASSERT_FALSE(session->isFold_);
    session->screenState_ = ScreenState::INIT;
}

/**
 * @tc.name: Connect
 * @tc.desc: Connect
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, Connect, TestSize.Level1)
{
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->RegisterScreenChangeListener(screenChangeListener);
    session->Connect();
    ASSERT_FALSE(session->isFold_);
}

/**
 * @tc.name: UpdatePropertyByActiveMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByActiveMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdatePropertyByActiveMode start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty originalProperty = session->GetScreenProperty();
    auto screenBounds = originalProperty.GetBounds();
    session->UpdatePropertyByActiveMode();
    ScreenProperty updatedProperty = session->GetScreenProperty();
    auto screenBounds1 = updatedProperty.GetBounds();
    ASSERT_EQ(screenBounds, screenBounds1);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdatePropertyByActiveMode end";
}

/**
 * @tc.name: Disconnect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, Disconnect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: Disconnect start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->Disconnect();
    ASSERT_FALSE(session->isFold_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: Disconnect end";
}

/**
 * @tc.name: Disconnect
 * @tc.desc: !listener
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, Disconnect02, TestSize.Level1)
{
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    IScreenChangeListener* screenChangeListener1 = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    sptr<ScreenSession> session1 = sptr<ScreenSession>::MakeSptr();
    session->RegisterScreenChangeListener(screenChangeListener);
    session1->RegisterScreenChangeListener(screenChangeListener1);
    session1->Connect();
    session1->Disconnect();
    ASSERT_FALSE(session->isFold_);
}

/**
 * @tc.name: SensorRotationChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SensorRotationChange01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SensorRotationChange start";
    Rotation sensorRotation = Rotation::ROTATION_0;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SensorRotationChange(sensorRotation);
    ASSERT_FALSE(session->isFold_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SensorRotationChange end";
}

/**
 * @tc.name: SensorRotationChange
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SensorRotationChange02, TestSize.Level1)
{
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->RegisterScreenChangeListener(screenChangeListener);
    Rotation sensorRotation = Rotation::ROTATION_90;
    session->SensorRotationChange(sensorRotation);
    ASSERT_FALSE(session->isFold_);
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetOrientation start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Orientation orientation = Orientation::UNSPECIFIED;
    session->SetOrientation(orientation);
    Orientation res = session->GetOrientation();
    ASSERT_EQ(res, orientation);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetOrientation end";
}

/**
 * @tc.name: SetScreenRequestedOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenRequestedOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenRequestedOrientation start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Orientation orientation = Orientation::UNSPECIFIED;
    session->SetScreenRequestedOrientation(orientation);
    Orientation res = session->GetScreenRequestedOrientation();
    ASSERT_EQ(res, orientation);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenRequestedOrientation end";
}

/**
 * @tc.name: SetUpdateToInputManagerCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetUpdateToInputManagerCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetUpdateToInputManagerCallback start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    auto func = [session](float x) {
        session->SetVirtualPixelRatio(x);
    };
    session->SetUpdateToInputManagerCallback(func);
    EXPECT_NE(nullptr, &func);

    ScreenProperty screenPropert = session->GetScreenProperty();
    EXPECT_NE(nullptr, &screenPropert);
    session->SetVirtualPixelRatio(3.14);

    ScreenType screenType { ScreenType::REAL };
    session->SetScreenType(screenType);

    GTEST_LOG_(INFO) << "ScreenSessionTest: SetUpdateToInputManagerCallbackend";
}

/**
 * @tc.name: SetScreenRotationLocked
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenRotationLocked, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenRotationLocked start";
    bool isLocked = true;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetScreenRotationLocked(isLocked);
    bool res = session->IsScreenRotationLocked();
    ASSERT_EQ(res, isLocked);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenRotationLocked end";
}

/**
 * @tc.name: SetScreenRotationLockedFromJs
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenRotationLockedFromJs, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenRotationLockedFromJs start";
    bool isLocked = true;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetScreenRotationLockedFromJs(isLocked);
    bool res = session->IsScreenRotationLocked();
    ASSERT_EQ(res, isLocked);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenRotationLockedFromJs end";
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, IsScreenRotationLocked, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: IsScreenRotationLocked start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    bool res = session->IsScreenRotationLocked();
    ASSERT_EQ(res, session->isScreenLocked_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: IsScreenRotationLocked end";
}

/**
 * @tc.name: GetScreenRequestedOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenRequestedOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenRequestedOrientation start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Orientation res = session->GetScreenRequestedOrientation();
    ASSERT_EQ(res, session->property_.GetScreenRequestedOrientation());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenRequestedOrientation end";
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetVirtualPixelRatio, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetVirtualPixelRatio start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    float virtualPixelRatio = 1;
    session->SetVirtualPixelRatio(virtualPixelRatio);
    float res = session->property_.GetVirtualPixelRatio();
    ASSERT_EQ(res, virtualPixelRatio);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetVirtualPixelRatio end";
}

/**
 * @tc.name: screen_session_test001
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test001 start";
    IScreenChangeListener* screenChangeListener = nullptr;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->screenState_ = ScreenState::CONNECTION;
    session->RegisterScreenChangeListener(screenChangeListener);
    EXPECT_FALSE(g_errLog.find("Failed to register screen change listener, listener is null!") != std::string::npos);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test001 end";
}

/**
 * @tc.name: screen_session_test002
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test002 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenId res = session->GetScreenId();
    ASSERT_EQ(res, session->screenId_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test002 end";
}

/**
 * @tc.name: screen_session_test003
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test003 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    DMError res = session->SetScreenColorTransform();
    ASSERT_EQ(res, DMError::DM_OK);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test003 end";
}

/**
 * @tc.name: screen_session_test004
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test004 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty newProperty;
    ScreenProperty screenProperty;
    session->UpdatePropertyByFoldControl(newProperty, FoldDisplayMode::MAIN, true);
    screenProperty = session->GetScreenProperty();
    ASSERT_EQ(screenProperty.GetDisplayOrientation(), DisplayOrientation::UNKNOWN);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test004 end";
}

/**
 * @tc.name: screen_session_test005
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test005 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty newProperty;
    session->PropertyChange(newProperty, ScreenPropertyChangeReason::UNDEFINED);
    GTEST_SKIP();
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test005 end";
}

/**
 * @tc.name: screen_session_test006
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test006 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Rotation sensorRotation = Rotation::ROTATION_90;
    float res = session->ConvertRotationToFloat(sensorRotation);
    ASSERT_EQ(res, 90.f);
    sensorRotation = Rotation::ROTATION_180;
    res = session->ConvertRotationToFloat(sensorRotation);
    ASSERT_EQ(res, 180.f);
    sensorRotation = Rotation::ROTATION_270;
    res = session->ConvertRotationToFloat(sensorRotation);
    ASSERT_EQ(res, 270.f);
    sensorRotation = Rotation::ROTATION_0;
    res = session->ConvertRotationToFloat(sensorRotation);
    ASSERT_EQ(res, 0.f);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test006 end";
}

/**
 * @tc.name: screen_session_test007
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test007 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Orientation orientation = Orientation::UNSPECIFIED;
    int res = 0;
    session->ScreenOrientationChange(orientation, FoldDisplayMode::UNKNOWN, false);
    EXPECT_FALSE(g_errLog.find("set orientation from napi") != std::string::npos);
    LOG_SetCallback(nullptr);
    g_errLog.clear();
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test007 end";
}

/**
 * @tc.name: screen_session_test009
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test009 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Rotation rotation = Rotation::ROTATION_90;
    session->SetRotation(rotation);
    Rotation res = session->GetRotation();
    ASSERT_EQ(res, rotation);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test009 end";
}

/**
 * @tc.name: screen_session_test010
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test010 start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    ScreenId childScreen = 1;
    bool res = sessionGroup.HasChild(childScreen);
    ASSERT_EQ(res, false);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test010 end";
}

/**
 * @tc.name: screen_session_test011
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test011 start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    std::vector<sptr<ScreenSession>> res = sessionGroup.GetChildren();
    ASSERT_EQ(res.empty(), true);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test011 end";
}

/**
 * @tc.name: screen_session_test012
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, screen_session_test012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test012 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    RectF rect = RectF(0, 0, 0, 0);
    ScreenProperty screenProperty;
    uint32_t offsetY = 0;
    session->SetDisplayBoundary(rect, offsetY);
    ASSERT_EQ(screenProperty.GetOffsetY(), offsetY);
    GTEST_LOG_(INFO) << "ScreenSessionTest: screen_session_test012 end";
}

/**
 * @tc.name: GetName
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetName start";
    std::string name { "UNKNOWN" };
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_EQ(name, session->GetName());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetName end";
}

/**
 * @tc.name: SetName
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetName start";
    std::string name { "UNKNOWN" };
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetName(name);
    ASSERT_EQ(name, session->GetName());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetName end";
}

/**
 * @tc.name: GetScreenSnapshot
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenSnapshot, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenSnapshot start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->displayNode_ = nullptr;
    auto pixelmap = session->GetScreenSnapshot(1.0, 1.0);
    EXPECT_EQ(pixelmap, nullptr);

    ScreenProperty newScreenProperty;
    session = sptr<ScreenSession>::MakeSptr(0, newScreenProperty, 0);
    pixelmap = session->GetScreenSnapshot(1.0, 1.0);
    EXPECT_EQ(pixelmap, nullptr);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenSnapshot end";
}

/**
 * @tc.name: GetRSScreenId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetRSScreenId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetRSScreenId start";
    ScreenProperty property;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr("OpenHarmony", 1, 100, 0);
    EXPECT_EQ(100, session->GetRSScreenId());

    RSDisplayNodeConfig displayNodeConfig;
    std::shared_ptr<RSDisplayNode> displayNode = RSDisplayNode::Create(displayNodeConfig);
    session = sptr<ScreenSession>::MakeSptr(1, 100, "OpenHarmony",
        property, displayNode);
    EXPECT_NE(nullptr, session->GetDisplayNode());
    session->ReleaseDisplayNode();
    EXPECT_EQ(nullptr, session->GetDisplayNode());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetRSScreenId end";
}

/**
 * @tc.name: CalcRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcRotation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty property;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    property.SetBounds(bounds);
    property.UpdateDeviceRotation(Rotation::ROTATION_0);
    session->SetScreenProperty(property);
    Orientation orientation = Orientation::VERTICAL;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::UNKNOWN;
    auto res = session->CalcRotation(orientation, foldDisplayMode);
    EXPECT_EQ(Rotation::ROTATION_0, res);
    property.UpdateDeviceRotation(Rotation::ROTATION_90);
    session->SetScreenProperty(property);
    res = session->CalcRotation(orientation, foldDisplayMode);
    EXPECT_EQ(Rotation::ROTATION_0, res);
}

/**
 * @tc.name: CalcBoundsInRotationZero
 * @tc.desc: normal CalcBoundsInRotationZero
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcBoundsInRotationZero, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty property;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    property.SetBounds(bounds);
    property.UpdateDeviceRotation(Rotation::ROTATION_0);
    session->SetScreenProperty(property);
    auto res = session->CalcBoundsInRotationZero(FoldDisplayMode::MAIN);
    EXPECT_EQ(res.rect_.width_, 1344);

    property.UpdateDeviceRotation(Rotation::ROTATION_90);
    session->SetScreenProperty(property);
    res = session->CalcBoundsInRotationZero(FoldDisplayMode::MAIN);
    EXPECT_EQ(res.rect_.width_, 2772);
}

/**
 * @tc.name: CalcBoundsByRotation
 * @tc.desc: normal CalcBoundsByRotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcBoundsByRotation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty property;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    property.SetBounds(bounds);
    property.UpdateDeviceRotation(Rotation::ROTATION_0);
    session->SetScreenProperty(property);
    Rotation rotation = Rotation::ROTATION_0;
    auto res = session->CalcBoundsByRotation(rotation);
    EXPECT_EQ(res.rect_.width_, 1344);
    rotation = Rotation::ROTATION_90;
    res = session->CalcBoundsByRotation(rotation);
    EXPECT_EQ(res.rect_.width_, 2772);

    property.UpdateDeviceRotation(Rotation::ROTATION_90);
    session->SetScreenProperty(property);
    rotation = Rotation::ROTATION_0;
    res = session->CalcBoundsByRotation(rotation);
    EXPECT_EQ(res.rect_.width_, 2772);
    rotation = Rotation::ROTATION_90;
    res = session->CalcBoundsByRotation(rotation);
    EXPECT_EQ(res.rect_.width_, 1344);
}

/**
 * @tc.name: IsVertical
 * @tc.desc: IsVertical
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, IsVertical, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_TRUE(session->IsVertical(Rotation::ROTATION_0));
    EXPECT_FALSE(session->IsVertical(Rotation::ROTATION_90));
    EXPECT_TRUE(session->IsVertical(Rotation::ROTATION_180));
    EXPECT_FALSE(session->IsVertical(Rotation::ROTATION_270));
}

/**
 * @tc.name: CalcOrientationToDisplayOrientation
 * @tc.desc: CalcOrientationToDisplayOrientation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcOrientationToDisplayOrientation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    DisplayOrientation orientation = session->CalcOrientationToDisplayOrientation(Orientation::UNSPECIFIED);
    EXPECT_EQ(orientation, DisplayOrientation::UNKNOWN);
    orientation = session->CalcOrientationToDisplayOrientation(Orientation::VERTICAL);
    EXPECT_EQ(orientation, DisplayOrientation::PORTRAIT);
    orientation = session->CalcOrientationToDisplayOrientation(Orientation::HORIZONTAL);
    EXPECT_EQ(orientation, DisplayOrientation::LANDSCAPE);
    orientation = session->CalcOrientationToDisplayOrientation(Orientation::REVERSE_VERTICAL);
    EXPECT_EQ(orientation, DisplayOrientation::PORTRAIT_INVERTED);
    orientation = session->CalcOrientationToDisplayOrientation(Orientation::REVERSE_HORIZONTAL);
    EXPECT_EQ(orientation, DisplayOrientation::LANDSCAPE_INVERTED);
}

/**
 * @tc.name: CalcRotationByDeviceOrientation
 * @tc.desc: CalcRotationByDeviceOrientation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcRotationByDeviceOrientation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    DisplayOrientation displayOrientation = DisplayOrientation::PORTRAIT;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::GLOBAL_FULL;
    RRect boundsInRotationZero;
    boundsInRotationZero.rect_.width_ = 1344;
    boundsInRotationZero.rect_.height_ = 2772;
    Rotation rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_90);
    foldDisplayMode = FoldDisplayMode::UNKNOWN;
    rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
    boundsInRotationZero.rect_.width_ = 2772;
    boundsInRotationZero.rect_.height_ = 1344;
    rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_270);
    foldDisplayMode = FoldDisplayMode::MAIN;
    displayOrientation = DisplayOrientation::PORTRAIT;
    rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
    displayOrientation = DisplayOrientation::LANDSCAPE;
    rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_90);
    displayOrientation = DisplayOrientation::PORTRAIT_INVERTED;
    rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_180);
    displayOrientation = DisplayOrientation::LANDSCAPE_INVERTED;
    rotation = session->CalcRotationByDeviceOrientation(displayOrientation,
        foldDisplayMode, boundsInRotationZero);
    EXPECT_EQ(rotation, Rotation::ROTATION_270);
}

/**
 * @tc.name: CalcDisplayOrientationToOrientation
 * @tc.desc: CalcDisplayOrientationToOrientation Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDisplayOrientationToOrientation, TestSize.Level2)
{
    ScreenId screenId = 0;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    Orientation orientation = screenSession->CalcDisplayOrientationToOrientation(DisplayOrientation::UNKNOWN);
    EXPECT_EQ(orientation, Orientation::UNSPECIFIED);
    orientation = screenSession->CalcDisplayOrientationToOrientation(DisplayOrientation::PORTRAIT);
    EXPECT_EQ(orientation, Orientation::VERTICAL);
    orientation = screenSession->CalcDisplayOrientationToOrientation(DisplayOrientation::LANDSCAPE);
    EXPECT_EQ(orientation, Orientation::HORIZONTAL);
    orientation = screenSession->CalcDisplayOrientationToOrientation(DisplayOrientation::PORTRAIT_INVERTED);
    EXPECT_EQ(orientation, Orientation::REVERSE_VERTICAL);
    orientation = screenSession->CalcDisplayOrientationToOrientation(DisplayOrientation::LANDSCAPE_INVERTED);
    EXPECT_EQ(orientation, Orientation::REVERSE_HORIZONTAL);
}

/**
 * @tc.name: CalcDisplayOrientation01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDisplayOrientation01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: CalcDisplayOrientation start";
    Rotation rotation { Rotation::ROTATION_0 };
    FoldDisplayMode foldDisplayMode { FoldDisplayMode::COORDINATION };
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->activeIdx_ = -1;
    auto res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::LANDSCAPE, res);

    sptr<SupportedScreenModes> supportedScreenModes = new SupportedScreenModes;
    supportedScreenModes->width_ = 40;
    supportedScreenModes->height_ = 20;
    session->modes_ = { supportedScreenModes };
    session->activeIdx_ = 0;

    rotation = Rotation::ROTATION_0;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::LANDSCAPE, res);

    rotation = Rotation::ROTATION_90;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::PORTRAIT, res);

    rotation = Rotation::ROTATION_180;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::LANDSCAPE_INVERTED, res);

    rotation = Rotation::ROTATION_270;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::PORTRAIT_INVERTED, res);

    GTEST_LOG_(INFO) << "ScreenSessionTest: CalcDisplayOrientation end";
}

/**
 * @tc.name: CalcDisplayOrientation02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDisplayOrientation02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: CalcDisplayOrientation start";
    Rotation rotation { Rotation::ROTATION_0 };
    FoldDisplayMode foldDisplayMode { FoldDisplayMode::UNKNOWN };
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->activeIdx_ = -1;
    auto res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::LANDSCAPE, res);

    sptr<SupportedScreenModes> supportedScreenModes = new SupportedScreenModes;
    supportedScreenModes->width_ = 40;
    supportedScreenModes->height_ = 20;
    session->modes_ = { supportedScreenModes };
    session->activeIdx_ = 0;

    rotation = Rotation::ROTATION_0;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::LANDSCAPE, res);

    rotation = Rotation::ROTATION_90;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::PORTRAIT, res);

    rotation = Rotation::ROTATION_180;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::LANDSCAPE_INVERTED, res);

    rotation = Rotation::ROTATION_270;
    res = session->CalcDisplayOrientation(rotation, foldDisplayMode);
    EXPECT_EQ(DisplayOrientation::PORTRAIT_INVERTED, res);

    GTEST_LOG_(INFO) << "ScreenSessionTest: CalcDisplayOrientation end";
}

/**
 * @tc.name: FillScreenInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, FillScreenInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: FillScreenInfo start";
    ScreenProperty property;
    property.SetVirtualPixelRatio(3.1415927);
    sptr<ScreenSession> session = new(std::nothrow) ScreenSession(2024, property, 0);
    sptr<ScreenInfo> info = nullptr;
    session->FillScreenInfo(info);

    sptr<SupportedScreenModes> supportedScreenModes = new SupportedScreenModes;
    session->modes_ = { supportedScreenModes };
    supportedScreenModes->width_ = 40;
    supportedScreenModes->height_ = 20;
    info = new ScreenInfo;
    session->FillScreenInfo(info);
    EXPECT_NE(nullptr, info);
    GTEST_LOG_(INFO) << "ScreenSessionTest: FillScreenInfo end";
}

/**
 * @tc.name: PropertyChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, PropertyChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: PropertyChange start";
    int res = 0;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ScreenProperty newProperty;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::CHANGE_MODE;
    session->PropertyChange(newProperty, reason);
    ASSERT_EQ(res, 0);
    GTEST_LOG_(INFO) << "ScreenSessionTest: PropertyChange end";
}

/**
 * @tc.name: PowerStatusChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, PowerStatusChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: PowerStatusChange start";
    int res = 0;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    DisplayPowerEvent event = DisplayPowerEvent::DISPLAY_ON;
    EventStatus status = EventStatus::BEGIN;
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    session->PowerStatusChange(event, status, reason);
    ASSERT_EQ(res, 0);
    GTEST_LOG_(INFO) << "ScreenSessionTest: PowerStatusChange end";
}

/**
 * @tc.name: SetScreenScale
 * @tc.desc: SetScreenScale test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenScale, TestSize.Level1)
{
    ScreenSession session;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float translateX = 0.0f;
    float translateY = 0.0f;
    session.SetScreenScale(scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    EXPECT_EQ(session.property_.GetScaleX(), scaleX);
    EXPECT_EQ(session.property_.GetScaleY(), scaleY);
    EXPECT_EQ(session.property_.GetPivotX(), pivotX);
    EXPECT_EQ(session.property_.GetPivotY(), pivotY);
    EXPECT_EQ(session.property_.GetTranslateX(), translateX);
    EXPECT_EQ(session.property_.GetTranslateY(), translateY);
}

/**
 * @tc.name: HoverStatusChange01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, HoverStatusChange01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HoverStatusChange start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    int32_t HoverStatus = 0;
    screenSession->HoverStatusChange(HoverStatus);
    GTEST_LOG_(INFO) << "HoverStatusChange end";
}

/**
 * @tc.name: HoverStatusChange02
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, HoverStatusChange02, TestSize.Level1)
{
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);
    session->RegisterScreenChangeListener(screenChangeListener);
    int32_t hoverStatus = 0;
    session->HoverStatusChange(hoverStatus);
}

/**
 * @tc.name: HandleHoverStatusChange01
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, HandleHoverStatusChange01, TestSize.Level1)
{
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);
    session->RegisterScreenChangeListener(screenChangeListener);
    int32_t hoverStatus = 0;
    session->HandleHoverStatusChange(hoverStatus);
}

/**
 * @tc.name: ScreenCaptureNotify
 * @tc.desc: ScreenCaptureNotify test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenCaptureNotify, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    ScreenId screenId = 0;
    int32_t uid = 0;
    std::string clientName = "test";
    session->ScreenCaptureNotify(screenId, uid, clientName);
}

/**
 * @tc.name: GetIsInternal01
 * @tc.desc: Test when isInternal is true, GetIsInternal should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsInternal01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsInternal(true);
    ASSERT_EQ(session->GetIsInternal(), true);
}

/**
 * @tc.name: GetIsInternal02
 * @tc.desc: Test when isInternal is false, GetIsInternal should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsInternal02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsInternal(false);
    ASSERT_EQ(session->GetIsInternal(), false);
}

/**
 * @tc.name: GetIsFakeInUse01
 * @tc.desc: Test when isFakeInUse is true, GetIsFakeInUse should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsFakeInUse01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsFakeInUse(true);
    ASSERT_EQ(session->GetIsFakeInUse(), true);
}

/**
 * @tc.name: GetIsFakeInUse02
 * @tc.desc: Test when isFakeInUse is false, GetIsFakeInUse should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsFakeInUse02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsFakeInUse(false);
    ASSERT_EQ(session->GetIsFakeInUse(), false);
}

/**
 * @tc.name: GetIsRealScreen01
 * @tc.desc: Test when isReal is true, GetIsRealScreen should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsRealScreen01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsRealScreen(true);
    ASSERT_EQ(session->GetIsRealScreen(), true);
}

/**
 * @tc.name: GetIsRealScreen02
 * @tc.desc: Test when isReal is false, GetIsRealScreen should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsRealScreen02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsRealScreen(false);
    ASSERT_EQ(session->GetIsRealScreen(), false);
}

/**
 * @tc.name: GetIsPcUse01
 * @tc.desc: Test when isPcUse is true, GetIsPcUse should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsPcUse01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsPcUse(true);
    ASSERT_EQ(session->GetIsPcUse(), true);
}

/**
 * @tc.name: GetIsPcUse02
 * @tc.desc: Test when isPcUse is false, GetIsPcUse should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsPcUse02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsPcUse(false);
    ASSERT_EQ(session->GetIsPcUse(), false);
}

/**
 * @tc.name: SetIsExtendVirtual001
 * @tc.desc: Test when isPcUse is true, GetIsPcUse should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsExtendVirtual001, TestSize.Level1)
{
    sptr<ScreenSession> session = new ScreenSession();
    ASSERT_NE(session, nullptr);
    session->SetIsExtendVirtual(true);
    EXPECT_EQ(session->GetIsExtendVirtual(), true);
}

/**
 * @tc.name: SetIsExtendVirtual002
 * @tc.desc: Test when isPcUse is false, GetIsPcUse should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsExtendVirtual002, TestSize.Level1)
{
    sptr<ScreenSession> session = new ScreenSession();
    ASSERT_NE(session, nullptr);
    session->SetIsExtendVirtual(false);
    EXPECT_EQ(session->GetIsExtendVirtual(), false);
}

/**
 * @tc.name: GetIsBScreenHalf01
 * @tc.desc: Test when isBScreenHalf is true, GetIsBScreenHalf should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsBScreenHalf01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsBScreenHalf(true);
    ASSERT_EQ(session->GetIsBScreenHalf(), true);
}

/**
 * @tc.name: GetIsBScreenHalf02
 * @tc.desc: Test when isBScreenHalf is false, GetIsBScreenHalf should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsBScreenHalf02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsBScreenHalf(false);
    ASSERT_EQ(session->GetIsBScreenHalf(), false);
}

/**
 * @tc.name: CalcDeviceOrientation01
 * @tc.desc: Test when rotation is ROTATION_0 then CalcDeviceOrientation returns PORTRAIT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientation01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    DisplayOrientation result = session->CalcDeviceOrientation(Rotation::ROTATION_0, FoldDisplayMode::FULL);
    ASSERT_EQ(result, DisplayOrientation::PORTRAIT);
}

/**
 * @tc.name: CalcDeviceOrientation02
 * @tc.desc: Test when rotation is ROTATION_90 then CalcDeviceOrientation returns LANDSCAPE
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientation02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    DisplayOrientation result = session->CalcDeviceOrientation(Rotation::ROTATION_90, FoldDisplayMode::FULL);
    ASSERT_EQ(result, DisplayOrientation::LANDSCAPE);
}

/**
 * @tc.name: CalcDeviceOrientation03
 * @tc.desc: Test when rotation is ROTATION_180 then CalcDeviceOrientation returns PORTRAIT_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientation03, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    DisplayOrientation result = session->CalcDeviceOrientation(Rotation::ROTATION_180, FoldDisplayMode::FULL);
    ASSERT_EQ(result, DisplayOrientation::PORTRAIT_INVERTED);
}

/**
 * @tc.name: CalcDeviceOrientation04
 * @tc.desc: Test when rotation is ROTATION_270 then CalcDeviceOrientation returns LANDSCAPE_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientation04, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    DisplayOrientation result = session->CalcDeviceOrientation(Rotation::ROTATION_270, FoldDisplayMode::FULL);
    ASSERT_EQ(result, DisplayOrientation::LANDSCAPE_INVERTED);
}

/**
 * @tc.name: CalcDeviceOrientation05
 * @tc.desc: Test when rotation is unknown then CalcDeviceOrientation returns UNKNOWN
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientation05, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    DisplayOrientation result = session->CalcDeviceOrientation(static_cast<Rotation>(100),
        FoldDisplayMode::FULL);
    ASSERT_EQ(result, DisplayOrientation::UNKNOWN);
}

/**
 * @tc.name: CalcDeviceOrientationWithBounds01
 * @tc.desc: Test when rotation is ROTATION_0 then CalcDeviceOrientationWithBounds returns PORTRAIT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    auto result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_0, FoldDisplayMode::FULL, bounds);
    ASSERT_EQ(result, DisplayOrientation::PORTRAIT);
}
 
/**
 * @tc.name: CalcDeviceOrientationWithBounds02
 * @tc.desc: Test when rotation is ROTATION_90 then CalcDeviceOrientationWithBounds returns LANDSCAPE
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    auto result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_90, FoldDisplayMode::FULL, bounds);
    ASSERT_EQ(result, DisplayOrientation::LANDSCAPE);
}
 
/**
 * @tc.name: CalcDeviceOrientationWithBounds03
 * @tc.desc: Test when rotation is ROTATION_180 then CalcDeviceOrientationWithBounds returns PORTRAIT_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds03, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    auto result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_180, FoldDisplayMode::FULL, bounds);
    ASSERT_EQ(result, DisplayOrientation::PORTRAIT_INVERTED);
}
 
/**
 * @tc.name: CalcDeviceOrientationWithBounds04
 * @tc.desc: Test when rotation is ROTATION_270 then CalcDeviceOrientationWithBounds returns LANDSCAPE_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds04, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    auto result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_270, FoldDisplayMode::FULL, bounds);
    ASSERT_EQ(result, DisplayOrientation::LANDSCAPE_INVERTED);
}
 
/**
 * @tc.name: CalcDeviceOrientationWithBounds05
 * @tc.desc: Test when rotation is unknown then CalcDeviceOrientationWithBounds returns UNKNOWN
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds05, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    auto result = session->CalcDeviceOrientationWithBounds(static_cast<Rotation>(100), FoldDisplayMode::FULL, bounds);
    ASSERT_EQ(result, DisplayOrientation::UNKNOWN);
}
 
/**
 * @tc.name: CalcDeviceOrientationWithBounds06
 * @tc.desc: Test when foldDisplayMode is GLOBAL_FULL
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds06, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    auto result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_0, FoldDisplayMode::GLOBAL_FULL, bounds);
    ASSERT_EQ(result, DisplayOrientation::LANDSCAPE_INVERTED);
}
 
/**
 * @tc.name: CalcDeviceOrientationWithBounds07
 * @tc.desc: Test CalcDeviceOrientationWithBounds
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcDeviceOrientationWithBounds07, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    RRect bounds;
    bounds.rect_.width_ = 2772;
    bounds.rect_.height_ = 1344;
    auto result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_0, FoldDisplayMode::UNKNOWN, bounds);
    EXPECT_EQ(result, DisplayOrientation::LANDSCAPE);
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    result = session->CalcDeviceOrientationWithBounds(Rotation::ROTATION_90, FoldDisplayMode::UNKNOWN, bounds);
    EXPECT_EQ(result, DisplayOrientation::PORTRAIT_INVERTED);
}

/**
 * @tc.name: GetIsPhysicalMirrorSwitch01
 * @tc.desc: Test when isPhysicalMirrorSwitch is true, GetIsPhysicalMirrorSwitch should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsPhysicalMirrorSwitch01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsPhysicalMirrorSwitch(true);
    ASSERT_EQ(session->GetIsPhysicalMirrorSwitch(), true);
}

/**
 * @tc.name: GetIsPhysicalMirrorSwitch02
 * @tc.desc: Test when isPhysicalMirrorSwitch is false, GetIsPhysicalMirrorSwitch should return false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsPhysicalMirrorSwitch02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsPhysicalMirrorSwitch(false);
    ASSERT_EQ(session->GetIsPhysicalMirrorSwitch(), false);
}
/**
 * @tc.name: GetDisplaySourceMode01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetDisplaySourceMode01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSourceMode start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->screenId_ = session->defaultScreenId_;
    DisplaySourceMode mode = session->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::MAIN);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    mode = screenSession->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::NONE);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXPAND);
    mode = screenSession->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::EXTEND);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    mode = screenSession->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::MIRROR);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_UNIQUE);
    mode = screenSession->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::ALONE);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDisplaySourceMode01 end";
}

/**
* @tc.name: GetDisplaySourceMode02
* @tc.desc: normal function
* @tc.type: FUNC
*/
HWTEST_F(ScreenSessionTest, GetDisplaySourceMode02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDisplaySourceMode02 start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->screenId_ = session->defaultScreenId_;
    DisplaySourceMode mode = session->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::MAIN);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);

    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    mode = screenSession->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::MAIN);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    mode = screenSession->GetDisplaySourceMode();
    ASSERT_EQ(mode, DisplaySourceMode::EXTEND);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDisplaySourceMode02 end";
}

/**
 * @tc.name: SetPointerActiveWidth
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetPointerActiveWidth, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetPointerActiveWidth start";
    uint32_t pointerActiveWidth = 123;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetPointerActiveWidth(pointerActiveWidth);
    ASSERT_EQ(pointerActiveWidth, session->property_.GetPointerActiveWidth());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetPointerActiveWidth end";
}

/**
 * @tc.name: GetPointerActiveWidth
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetPointerActiveWidth, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetPointerActiveWidth start";
    uint32_t pointerActiveWidth = 123;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetPointerActiveWidth(pointerActiveWidth);
    ASSERT_EQ(pointerActiveWidth, session->GetPointerActiveWidth());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetPointerActiveWidth end";
}

/**
 * @tc.name: SetPointerActiveHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetPointerActiveHeight, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetPointerActiveHeight start";
    uint32_t pointerActiveHeight = 321;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetPointerActiveHeight(pointerActiveHeight);
    ASSERT_EQ(pointerActiveHeight, session->property_.GetPointerActiveHeight());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetPointerActiveHeight end";
}

/**
 * @tc.name: GetPointerActiveHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetPointerActiveHeight, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetPointerActiveHeight start";
    uint32_t pointerActiveHeight = 321;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    session->SetPointerActiveHeight(pointerActiveHeight);
    ASSERT_EQ(pointerActiveHeight, session->GetPointerActiveHeight());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetPointerActiveHeight end";
}

/**
 * @tc.name: UpdateExpandAvailableArea
 * @tc.desc: UpdateExpandAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateExpandAvailableArea, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateExpandAvailableArea start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);
    DMRect area = screenSession->GetExpandAvailableArea();
    auto res = screenSession->UpdateExpandAvailableArea(area);
    ASSERT_EQ(res, false);
    area = {200, 200, 200, 200};
    res = screenSession->UpdateExpandAvailableArea(area);
    ASSERT_EQ(res, true);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateExpandAvailableArea end";
}

/**
 * @tc.name: UpdateDisplayNodeRotation
 * @tc.desc: UpdateDisplayNodeRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateDisplayNodeRotation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateDisplayNodeRotation start";
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(screenSession, nullptr);
    screenSession->UpdateDisplayNodeRotation(1);
    ASSERT_EQ(screenSession->isExtended_, false);

    Rosen::RSDisplayNodeConfig rsConfig;
    rsConfig.isMirrored = true;
    rsConfig.screenId = 101;
    screenSession->CreateDisplayNode(rsConfig);
    screenSession->UpdateDisplayNodeRotation(1);
    ASSERT_EQ(screenSession->isExtended_, false);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateDisplayNodeRotation end";
}

/**
 * @tc.name: UpdateExpandAvailableArea01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateExpandAvailableArea01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExpandAvailableArea start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    DMRect area = screenSession->GetAvailableArea();
    auto res = screenSession->UpdateExpandAvailableArea(area);
    ASSERT_EQ(res, false);
    area = {2, 2, 2, 2};
    res = screenSession->UpdateExpandAvailableArea(area);
    ASSERT_EQ(res, true);
    GTEST_LOG_(INFO) << "UpdateExpandAvailableArea end";
}

/**
 * @tc.name: ScreenExtendChange01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenExtendChange01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenExtendChange start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    ScreenId mainScreenId = 0;
    ScreenId extendScreenId = 1;
    screenSession->ScreenExtendChange(mainScreenId, extendScreenId);
    GTEST_LOG_(INFO) << "UpdateExpandAvailableArea end";
}

/**
 * @tc.name: ScreenExtendChange02
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenExtendChange02, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    IScreenChangeListener* screenChangeListener = new MockScreenChangeListener();
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);
    session->RegisterScreenChangeListener(screenChangeListener);
    ScreenId mainScreenId = 0;
    ScreenId extendScreenId = 1;
    session->ScreenExtendChange(mainScreenId, extendScreenId);
    EXPECT_FALSE(g_errLog.find("screenChangeListener is null.") != std::string::npos);
}

/**
 * @tc.name: SuperFoldStatusChange01
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SuperFoldStatusChange01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);
    ScreenId mainScreenId = 0;
    SuperFoldStatus superFoldStatus = SuperFoldStatus::UNKNOWN;
    session->SuperFoldStatusChange(mainScreenId, superFoldStatus);
    EXPECT_EQ(true, session->screenChangeListenerList_.empty());
    EXPECT_TRUE(g_errLog.find("screenChangeListenerList is empty.") != std::string::npos);
}

/**
 * @tc.name: ExtendScreenConnectStatusChange
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ExtendScreenConnectStatusChange, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);
    ScreenId mainScreenId = 0;
    ExtendScreenConnectStatus extendScreenConnectStatus = ExtendScreenConnectStatus::UNKNOWN;
    session->ExtendScreenConnectStatusChange(mainScreenId, extendScreenConnectStatus);
    EXPECT_EQ(true, session->screenChangeListenerList_.empty());
    EXPECT_TRUE(g_errLog.find("screenChangeListenerList is empty.") != std::string::npos);
}

/**
 * @tc.name: SecondaryReflexionChange
 * @tc.desc: run in for
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SecondaryReflexionChange, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);
    ScreenId mainScreenId = 0;
    bool isSecondaryReflexion = true;
    session->SecondaryReflexionChange(mainScreenId, isSecondaryReflexion);
    EXPECT_EQ(true, session->screenChangeListenerList_.empty());
    EXPECT_TRUE(g_errLog.find("screenChangeListenerList is empty.") != std::string::npos);
}

/**
 * @tc.name: UpdatePropertyOnly
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyOnly, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePropertyOnly start";
    LOG_SetCallback(MyLogCallback);
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    EXPECT_NE(nullptr, screenSession);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    int rotation = 90;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::MAIN;
    screenSession->UpdatePropertyOnly(bounds, rotation, foldDisplayMode);
    EXPECT_FALSE(g_errLog.find("bounds:[%{public}f %{public}f %{public}f %{public}f],\
        rotation:%{public}d, displayOrientation:%{public}u") != std::string::npos);
    GTEST_LOG_(INFO) << "UpdatePropertyOnly end";
}

/**
 * @tc.name: ReuseDisplayNode
 * @tc.desc: ReuseDisplayNode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ReuseDisplayNode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: ReuseDisplayNode start";
    Rosen::RSDisplayNodeConfig rsConfig;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    screenSession->SetDisplayNode(nullptr);
    screenSession->ReuseDisplayNode(rsConfig);

    rsConfig.isMirrored = true;
    rsConfig.screenId = 101;
    std::shared_ptr<RSDisplayNode> displayNode = RSDisplayNode::Create(rsConfig);
    screenSession->SetDisplayNode(displayNode);
    screenSession->ReuseDisplayNode(rsConfig);
    GTEST_LOG_(INFO) << "ScreenSessionTest: ReuseDisplayNode end";
}

/**
 * @tc.name: GetScreenModes
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenModes, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenModes start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    auto modes_ = session->GetScreenModes();
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenModes end";
}

/**
 * @tc.name: SetScreenModes
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenModes, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenModes start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    sptr<SupportedScreenModes> supportedScreenModes = new SupportedScreenModes();
    session->SetScreenModes({supportedScreenModes});
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenModes end";
}

/**
 * @tc.name: ConvertToRealDisplayInfo
 * @tc.desc: ConvertToRealDisplayInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ConvertToRealDisplayInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertToRealDisplayInfo start";
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(screenSession->ConvertToRealDisplayInfo(), nullptr);
    GTEST_LOG_(INFO) << "ScreenSessionTest: ConvertToRealDisplayInfo end";
}

/**
 * @tc.name: UpdateVirtualPixelRatio
 * @tc.desc: UpdateVirtualPixelRatio test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateVirtualPixelRatio, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateVirtualPixelRatio start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    EXPECT_NE(nullptr, screenSession);
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    screenSession->UpdateVirtualPixelRatio(bounds);
    float curVirtualPixelRatio = screenSession->property_.GetVirtualPixelRatio();
    ASSERT_EQ(curVirtualPixelRatio, 3.5f);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateVirtualPixelRatio end";
}

/**
 * @tc.name: SetInnerName
 * @tc.desc: SetInnerName test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetInnerName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetInnerName start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    EXPECT_NE(nullptr, screenSession);
    std::string innerName = "OpenHarmony";
    screenSession->SetInnerName(innerName);
    EXPECT_EQ(innerName, screenSession->GetInnerName());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetInnerName end";
}

/**
 * @tc.name: SetFakeScreenSession
 * @tc.desc: SetFakeScreenSession test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetFakeScreenSession, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetFakeScreenSession start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionConfig fakeConfig = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> fakeScreenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    screenSession->SetFakeScreenSession(fakeScreenSession);
    ASSERT_EQ(screenSession->GetFakeScreenSession(), fakeScreenSession);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetFakeScreenSession end";
}

/**
 * @tc.name: GetScreenShape
 * @tc.desc: GetScreenShape test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenShape, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenShape start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    screenSession->property_.SetScreenShape(ScreenShape::RECTANGLE);
    ASSERT_EQ(screenSession->GetScreenShape(), ScreenShape::RECTANGLE);
    screenSession->property_.SetScreenShape(ScreenShape::ROUND);
    ASSERT_EQ(screenSession->GetScreenShape(), ScreenShape::ROUND);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenShape end";
}

/**
 * @tc.name: SetSerialNumber
 * @tc.desc: SetSerialNumber test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetSerialNumber, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetSerialNumber start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    std::string serialNumber = "OpenHarmony";
    screenSession->SetSerialNumber(serialNumber);
    ASSERT_EQ(screenSession->GetSerialNumber(), serialNumber);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetSerialNumber end";
}

/**
 * @tc.name: UpdatePropertyByFakeBounds
 * @tc.desc: UpdatePropertyByFakeBounds test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByFakeBounds, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdatePropertyByFakeBounds start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    EXPECT_NE(nullptr, screenSession);
    uint32_t width = 1000;
    uint32_t height = 1500;
    screenSession->UpdatePropertyByFakeBounds(width, height);
    auto screenFakeBounds = screenSession->property_.GetFakeBounds();
    ASSERT_EQ(screenFakeBounds.rect_.width_, width);
    ASSERT_EQ(screenFakeBounds.rect_.height_, height);
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdatePropertyByFakeBounds end";
}

/**
 * @tc.name: GetValidSensorRotation
 * @tc.desc: GetValidSensorRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetValidSensorRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetValidSensorRotation start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    EXPECT_NE(nullptr, screenSession);
    Rotation sensorRotation = Rotation::ROTATION_0;
    screenSession->SensorRotationChange(sensorRotation);
    ASSERT_EQ(0, screenSession->GetValidSensorRotation());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetValidSensorRotation end";
}

/**
 * @tc.name: EnableMirrorScreenRegion
 * @tc.desc: EnableMirrorScreenRegion test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, EnableMirrorScreenRegion, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(nullptr, screenSession);
    screenSession->SetIsEnableRegionRotation(true);
    screenSession->EnableMirrorScreenRegion();
    EXPECT_EQ(screenSession->GetIsEnableRegionRotation(), true);
    screenSession->SetIsPhysicalMirrorSwitch(true);
    screenSession->EnableMirrorScreenRegion();
    EXPECT_EQ(screenSession->GetIsPhysicalMirrorSwitch(), true);
}

/**
 * @tc.name: SetIsExtend
 * @tc.desc: SetIsExtend
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsExtend01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsExtend(true);
    EXPECT_EQ(session->GetIsExtend(), true);
}

/**
 * @tc.name: SetIsExtend
 * @tc.desc: SetIsExtend
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsExtend02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsExtend(false);
    EXPECT_EQ(session->GetIsExtend(), false);
}

/**
 * @tc.name: SetIsCurrentInUse
 * @tc.desc: SetIsCurrentInUse
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsCurrentInUse01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsCurrentInUse(true);
    EXPECT_EQ(session->GetIsCurrentInUse(), true);
}

/**
 * @tc.name: SetIsCurrentInUse
 * @tc.desc: SetIsCurrentInUse
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsCurrentInUse02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsCurrentInUse(false);
    EXPECT_EQ(session->GetIsCurrentInUse(), false);
}

/**
 * @tc.name: SetValidHeight
 * @tc.desc: SetValidHeight
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetValidHeight, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    uint32_t validHeight = 100;
    session->SetValidHeight(validHeight);
    EXPECT_EQ(session->GetValidHeight(), validHeight);
}

/**
 * @tc.name: SetValidWidth
 * @tc.desc: SetValidWidth
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetValidWidth, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    uint32_t validWidth = 100;
    session->SetValidWidth(validWidth);
    EXPECT_EQ(session->GetValidWidth(), validWidth);
}

/**
 * @tc.name: UpdateRotationOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRotationOrientation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    int rotation = 0;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::MAIN;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    session->UpdateRotationOrientation(rotation, foldDisplayMode, bounds);
    ScreenProperty screenProperty = session->GetScreenProperty();
    EXPECT_EQ(screenProperty.deviceRotation_, Rotation::ROTATION_0);
}

/**
 * @tc.name: UpdatePropertyByFakeInUse
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByFakeInUse01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->UpdatePropertyByFakeInUse(false);
    EXPECT_EQ(session->property_.GetIsFakeInUse(), false);
}

/**
 * @tc.name: UpdatePropertyByFakeInUse
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByFakeInUse02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->UpdatePropertyByFakeInUse(true);
    EXPECT_EQ(session->property_.GetIsFakeInUse(), true);
}

/**
 * @tc.name: SetRotationAndScreenRotationOnly
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetRotationAndScreenRotationOnly, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    bool enableRotation = (system::GetParameter("persist.window.rotation.enabled", "1") == "1");
    if (enableRotation) {
        session->SetRotationAndScreenRotationOnly(Rotation::ROTATION_0);
        EXPECT_EQ(session->property_.GetRotation(), 0.0f);
        session->SetRotationAndScreenRotationOnly(Rotation::ROTATION_90);
        EXPECT_EQ(session->property_.GetRotation(), 90.0f);
        session->SetRotationAndScreenRotationOnly(Rotation::ROTATION_180);
        EXPECT_EQ(session->property_.GetRotation(), 180.0f);
        session->SetRotationAndScreenRotationOnly(Rotation::ROTATION_270);
        EXPECT_EQ(session->property_.GetRotation(), 270.0f);
    }
}

/**
 * @tc.name: SetTouchEnabledFromJs
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetTouchEnabledFromJs01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetTouchEnabledFromJs(false);
    EXPECT_EQ(session->IsTouchEnabled(), false);
}

/**
 * @tc.name: SetTouchEnabledFromJs
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetTouchEnabledFromJs02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetTouchEnabledFromJs(true);
    EXPECT_EQ(session->IsTouchEnabled(), true);
}

/**
 * @tc.name: SetIsEnableCanvasRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsEnableCanvasRotation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsEnableCanvasRotation(false);
    EXPECT_EQ(session->GetIsEnableCanvasRotation(), false);
}

/**
 * @tc.name: GetIsEnableCanvasRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsEnableCanvasRotation, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetIsEnableCanvasRotation(true);
    EXPECT_EQ(session->GetIsEnableCanvasRotation(), true);
}

/**
 * @tc.name: GetIsExtend
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsExtend, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetIsExtend start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetIsExtend(true);
    EXPECT_EQ(screenSession->GetIsExtend(), true);
    GTEST_LOG_(INFO) << "GetIsExtend end";
}

/**
 * @tc.name: SetIsInternal
 * @tc.desc: Test when isInternal is true, GetIsInternal should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsInternal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIsInternal start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    session->SetIsInternal(true);
    EXPECT_EQ(session->GetIsInternal(), true);
    GTEST_LOG_(INFO) << "SetIsInternal end";
}

/**
 * @tc.name: GetIsCurrentInUse
 * @tc.desc: GetIsCurrentInUse
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetIsCurrentInUse, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIsCurrentInUse start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    session->SetIsCurrentInUse(true);
    EXPECT_EQ(session->GetIsCurrentInUse(), true);
    GTEST_LOG_(INFO) << "GetIsCurrentInUse end";
}

/**
 * @tc.name: SetIsFakeInUse
 * @tc.desc: Test when isFakeInUse is true, GetIsFakeInUse should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsFakeInUse, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIsFakeInUse start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    session->SetIsFakeInUse(true);
    EXPECT_EQ(session->GetIsFakeInUse(), true);
    GTEST_LOG_(INFO) << "SetIsFakeInUse end";
}

/**
 * @tc.name: SetIsRealScreen
 * @tc.desc: Test when isReal is true, GetIsRealScreen should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsRealScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIsRealScreen start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    session->SetIsRealScreen(true);
    EXPECT_EQ(session->GetIsRealScreen(), true);
    GTEST_LOG_(INFO) << "SetIsRealScreen end";
}

/**
 * @tc.name: SetIsPcUse
 * @tc.desc: Test when isPcUse is true, GetIsPcUse should return true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsPcUse, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIsRealScreen start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    session->SetIsPcUse(true);
    EXPECT_EQ(session->GetIsPcUse(), true);
    GTEST_LOG_(INFO) << "SetIsRealScreen end";
}

/**
 * @tc.name: GetValidHeight
 * @tc.desc: GetValidHeight
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetValidHeight, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetValidHeight start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    uint32_t validHeight = 100;
    session->SetValidHeight(validHeight);
    EXPECT_EQ(session->GetValidHeight(), validHeight);
    GTEST_LOG_(INFO) << "GetValidHeight end";
}

/**
 * @tc.name: GetValidWidth
 * @tc.desc: GetValidWidth
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetValidWidth, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetValidWidth start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    uint32_t validWidth = 100;
    session->SetValidWidth(validWidth);
    EXPECT_EQ(session->GetValidWidth(), validWidth);
    GTEST_LOG_(INFO) << "GetValidWidth end";
}

/**
 * @tc.name: GetInnerName
 * @tc.desc: GetInnerName test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetInnerName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetInnerName start";
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(nullptr, screenSession);

    std::string innerName = "OpenHarmony";
    screenSession->SetInnerName(innerName);
    EXPECT_EQ(innerName, screenSession->GetInnerName());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetInnerName end";
}


/**
 * @tc.name: screen_session_test002
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenId start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    ScreenId res = session->GetScreenId();
    EXPECT_EQ(res, session->screenId_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetScreenId end";
}

/**
 * @tc.name: SetUpdateScreenPivotCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetUpdateScreenPivotCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetUpdateScreenPivotCallback start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    auto func = [session](float x, float y) {
        session->GetScreenSnapshot(x, y);
    };

    session->SetUpdateScreenPivotCallback(func);
    EXPECT_NE(nullptr, session->updateScreenPivotCallback_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetUpdateScreenPivotCallback end";
}


/**
 * @tc.name: UpdateTouchBoundsAndOffset
 * @tc.desc: UpdateTouchBoundsAndOffset test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateTouchBoundsAndOffset, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateTouchBoundsAndOffset start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    screenSession->UpdateTouchBoundsAndOffset(FoldDisplayMode::UNKNOWN);
    EXPECT_EQ(0, screenSession->property_.GetInputOffsetY());
    GTEST_LOG_(INFO) << "ScreenSessionTest: UpdateTouchBoundsAndOffset end";
}

/**
 * @tc.name: GetOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetOrientation start";
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    Orientation orientation = Orientation::UNSPECIFIED;
    session->SetOrientation(orientation);
    Orientation res = session->GetOrientation();
    ASSERT_EQ(res, orientation);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetOrientation end";
}

/**
 * @tc.name: GetOrientation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetDensityInCurResolution, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDensityInCurResolution start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    float density = 1.5f;
    screenSession->SetDensityInCurResolution(density);
    float res = screenSession->GetDensityInCurResolution();
    EXPECT_EQ(1.5f, res);
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDensityInCurResolution end";
}

/**
 * @tc.name: SetDefaultDensity
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetDefaultDensity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetDefaultDensity start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    float density = 1.5f;
    screenSession->SetDefaultDensity(density);
    float res = screenSession->property_.GetDefaultDensity();
    EXPECT_EQ(1.5f, res);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetDefaultDensity end";
}

/**
 * @tc.name: GetChildrenPosition
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetChildrenPosition, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetChildrenPosition start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenSession> session0 = nullptr;
    Point startPoint(0, 0);
    sptr<ScreenSession> defaultScreenSession;
    bool res = sessionGroup.AddChild(session0, startPoint, defaultScreenSession);
    EXPECT_EQ(res, false);

    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    res = sessionGroup.AddChild(session, startPoint, defaultScreenSession);
    EXPECT_EQ(res, true);
    EXPECT_NE(0, sessionGroup.GetChildrenPosition().size());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetChildrenPosition end";
}

/**
 * @tc.name: GetChildCount
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetChildCount, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetChildCount start";
    ScreenSessionGroup sessionGroup(1, 1, "create", ScreenCombination::SCREEN_ALONE);
    sptr<ScreenSession> session0 = nullptr;
    Point startPoint(0, 0);
    sptr<ScreenSession> defaultScreenSession;
    bool res = sessionGroup.AddChild(session0, startPoint, defaultScreenSession);
    EXPECT_EQ(res, false);

    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    res = sessionGroup.AddChild(session, startPoint, defaultScreenSession);
    EXPECT_EQ(res, true);
    EXPECT_NE(0, sessionGroup.GetChildCount());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetChildCount end";
}

/**
 * @tc.name: SetForceCloseHdr
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetForceCloseHdr, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetForceCloseHdr start";
    LOG_SetCallback(MyLogCallback);
    ScreenProperty newScreenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(0, newScreenProperty, 0);
    ASSERT_NE(session, nullptr);
    session->SetForceCloseHdr(true);
    session->SetForceCloseHdr(false);
    EXPECT_FALSE(g_errLog.find("Start get screen status.") != std::string::npos);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetForceCloseHdr end";
}

/**
 * @tc.name: SetForceCloseHdr01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetForceCloseHdr01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetForceCloseHdr start";
    LOG_SetCallback(MyLogCallback);
    ScreenProperty newScreenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(0, newScreenProperty, 0);
    ASSERT_NE(session, nullptr);
    session->SetForceCloseHdr(false);
    session->SetForceCloseHdr(false);
    EXPECT_TRUE(g_errLog.find("lastCloseHdrStatus_ and isForceCloseHdr are the same.") != std::string::npos);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetForceCloseHdr end";
}

/**
 * @tc.name: IsWidthHeightMatch01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, IsWidthHeightMatch01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: IsWidthHeightMatch01 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    float width = 1.5f;
    float height = 1.5f;
    float targetWidth = 1.5f;
    float targetHeight = 1.5f;
    EXPECT_EQ(true, screenSession->IsWidthHeightMatch(width, height, targetWidth, targetHeight));
    GTEST_LOG_(INFO) << "ScreenSessionTest: IsWidthHeightMatch01 end";
}

/**
 * @tc.name: IsWidthHeightMatch02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, IsWidthHeightMatch02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: IsWidthHeightMatch02 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    float width = 1.0f;
    float height = 2.0f;
    float targetWidth = 3.0f;
    float targetHeight = 4.0f;
    EXPECT_EQ(false, screenSession->IsWidthHeightMatch(width, height, targetWidth, targetHeight));
    GTEST_LOG_(INFO) << "ScreenSessionTest: IsWidthHeightMatch02 end";
}

/**
 * @tc.name: SetScreenSnapshotRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenSnapshotRect, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenSnapshotRect start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    RSSurfaceCaptureConfig config1 = {
        .scaleX = scaleX,
        .scaleY = scaleY,
        .useDma = true,
    };
    screenSession->SetScreenSnapshotRect(config1);
    EXPECT_TRUE(g_errLog.find("no need to set screen snapshot rect") != std::string::npos);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetScreenSnapshotRect end";
}

/**
 * @tc.name: SetStartPosition
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetStartPosition, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetStartPosition start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    screenSession->SetStartPosition(scaleX, scaleY);
    EXPECT_EQ(scaleX, screenSession->property_.scaleX_);
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetStartPosition end";
}

/**
 * @tc.name: ScreenModeChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenModeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: ScreenModeChange start";
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);

    session->screenChangeListenerList_.clear();
    session->ScreenModeChange(ScreenModeChangeEvent::UNKNOWN);
    EXPECT_TRUE(g_errLog.find("screenChangeListenerList is empty.") != std::string::npos);
    GTEST_LOG_(INFO) << "ScreenSessionTest: ScreenModeChange end";
}

/**
 * @tc.name: GetSensorRotation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSensorRotation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSensorRotation start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);

    Rotation sensorRotation = Rotation::ROTATION_0;
    screenSession->SensorRotationChange(sensorRotation);
    EXPECT_EQ(0, screenSession->GetSensorRotation());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetSensorRotation end";
}

/**
 * @tc.name: GetDisplayId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetDisplayId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDisplayId start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetScreenId(1);
    EXPECT_EQ(1, screenSession->GetDisplayId());
    GTEST_LOG_(INFO) << "ScreenSessionTest: GetDisplayId end";
}

/**
 * @tc.name: SetDisplayNode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetDisplayNode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetDisplayNode start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    RSDisplayNodeConfig displayNodeConfig;
    std::shared_ptr<RSDisplayNode> displayNode = RSDisplayNode::Create(displayNodeConfig);
    EXPECT_NE(nullptr, displayNode);

    screenSession->SetDisplayNode(displayNode);
    EXPECT_NE(nullptr, screenSession->GetDisplayNode());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetDisplayNode end";
}

/**
 * @tc.name: SetRSScreenId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetRSScreenId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetRSScreenId start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetRSScreenId(1);
    EXPECT_EQ(1, screenSession->GetRSScreenId());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetRSScreenId end";
}

/**
 * @tc.name: SetActiveId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetActiveId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetActiveId start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetActiveId(1);
    EXPECT_EQ(1, screenSession->GetActiveId());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetActiveId end";
}

/**
 * @tc.name: SetIsAvailableAreaNeedNotify01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsAvailableAreaNeedNotify01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsAvailableAreaNeedNotify01 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetIsAvailableAreaNeedNotify(false);
    EXPECT_EQ(false, screenSession->GetIsAvailableAreaNeedNotify());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsAvailableAreaNeedNotify01 end";
}


/**
 * @tc.name: SetIsAvailableAreaNeedNotify02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsAvailableAreaNeedNotify02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsAvailableAreaNeedNotify02 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetIsAvailableAreaNeedNotify(true);
    EXPECT_EQ(true, screenSession->GetIsAvailableAreaNeedNotify());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsAvailableAreaNeedNotify02 end";
}

/**
 * @tc.name: SetIsEnableCanvasRotation01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsEnableCanvasRotation01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsEnableCanvasRotation01 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetIsEnableCanvasRotation(false);
    EXPECT_EQ(false, screenSession->GetIsEnableCanvasRotation());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsEnableCanvasRotation01 end";
}

/**
 * @tc.name: SetIsEnableCanvasRotation01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetIsEnableCanvasRotation02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsEnableCanvasRotation02 start";
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetIsEnableCanvasRotation(true);
    EXPECT_EQ(true, screenSession->GetIsEnableCanvasRotation());
    GTEST_LOG_(INFO) << "ScreenSessionTest: SetIsEnableCanvasRotation02 end";
}

/**
 * @tc.name: SetSecurity01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetSecurity01, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetSecurity(true);
    EXPECT_EQ(session->isSecurity_, true);
}

/**
 * @tc.name: SetSecurity02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetSecurity02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    session->SetSecurity(false);
    EXPECT_EQ(session->isSecurity_, false);
}

/**
 * @tc.name: SetHorizontalRotation01
 * @tc.desc: SetHorizontalRotation01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetHorizontalRotation01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    EXPECT_NE(nullptr, session);

    session->displayNode_ = nullptr;
    session->SetHorizontalRotation();
    EXPECT_EQ(session->property_.GetDeviceRotation(), Rotation::ROTATION_270);
    EXPECT_TRUE(g_errLog.find("displayNode is null, no need to set displayNode.") != std::string::npos);
}

/**
 * @tc.name: SetHorizontalRotation02
 * @tc.desc: SetHorizontalRotation02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetHorizontalRotation02, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);

    RSDisplayNodeConfig displayNodeConfig;
    std::shared_ptr<RSDisplayNode> displayNode = RSDisplayNode::Create(displayNodeConfig);
    EXPECT_NE(nullptr, displayNode);

    session->SetDisplayNode(displayNode);
    session->SetHorizontalRotation();
    EXPECT_EQ(session->property_.GetDeviceRotation(), Rotation::ROTATION_270);
    EXPECT_FALSE(g_errLog.find("displayNode is null, no need to set displayNode.") != std::string::npos);
}

/**
 * @tc.name: SetDisplayGroupId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetDisplayGroupId, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetDisplayGroupId(0);
    EXPECT_EQ(0, screenSession->GetDisplayGroupId());
    screenSession->SetMainDisplayIdOfGroup(0);
    EXPECT_EQ(0, screenSession->GetMainDisplayIdOfGroup());
}

/**
 * @tc.name: SetScreenArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetScreenArea, TestSize.Level1)
{
    ScreenSessionConfig config = {
        .screenId = 100,
        .rsId = 101,
        .name = "OpenHarmony",
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    ASSERT_NE(screenSession, nullptr);

    screenSession->SetScreenAreaOffsetX(100);
    screenSession->SetScreenAreaOffsetY(200);
    screenSession->SetScreenAreaWidth(300);
    screenSession->SetScreenAreaHeight(400);
    EXPECT_EQ(100, screenSession->GetScreenAreaOffsetX());
    EXPECT_EQ(200, screenSession->GetScreenAreaOffsetY());
    EXPECT_EQ(300, screenSession->GetScreenAreaWidth());
    EXPECT_EQ(400, screenSession->GetScreenAreaHeight());
}

/**
 * @tc.name: FreezeScreen
 * @tc.desc: FreezeScreen Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, FreezeScreen, TestSize.Level2)
{
    LOG_SetCallback(MyLogCallback);
    g_errLog.clear();
    ScreenId screenId = 0;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    screenSession->displayNode_ = nullptr;
    bool isFreeze = false;
    screenSession->FreezeScreen(isFreeze);
    EXPECT_TRUE(g_errLog.find("displayNode is null") != std::string::npos);

    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> displayNode = std::make_shared<RSDisplayNode>(config);
    screenSession->SetDisplayNode(displayNode);
    screenSession->FreezeScreen(isFreeze);
    g_errLog.clear();
}

/**
 * @tc.name: GetRotationCorrection
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetRotationCorrection, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    Rotation rotation = session->GetRotationCorrection(FoldDisplayMode::MAIN);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    session->SetRotationCorrectionMap(rotationCorrectionMap);
    rotation = session->GetRotationCorrection(FoldDisplayMode::MAIN);
    EXPECT_EQ(rotation, Rotation::ROTATION_270);
    rotation = session->GetRotationCorrection(FoldDisplayMode::FULL);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
}
 
/**
 * @tc.name: RemoveRotationCorrection
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, RemoveRotationCorrection, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    Rotation rotation = Rotation::ROTATION_270;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    session->SetRotationCorrectionMap(rotationCorrectionMap);
    session->RemoveRotationCorrection(rotation, FoldDisplayMode::MAIN);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
 
    rotation = Rotation::ROTATION_270;
    session->RemoveRotationCorrection(rotation, FoldDisplayMode::FULL);
    EXPECT_EQ(rotation, Rotation::ROTATION_270);
 
    rotation = static_cast<Rotation>(100);
    session->RemoveRotationCorrection(rotation, FoldDisplayMode::FULL);
}
 
/**
 * @tc.name: SetRotationCorrectionMap
 * @tc.desc: test set and get
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetRotationCorrectionMap, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    session->SetRotationCorrectionMap(rotationCorrectionMap);
    auto correctionMap = session->GetRotationCorrectionMap();
    EXPECT_EQ(correctionMap[FoldDisplayMode::MAIN], 3);
}

/**
 * @tc.name: GetScreenSnapshotWithAllWindows01
 * @tc.desc: GetScreenSnapshotWithAllWindows01 Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenSnapshotWithAllWindows01, TestSize.Level2)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    screenSession->displayNode_ = nullptr;
    float scaleX = 1.0;
    float scaleY = 1.0;
    bool isNeedCheckDrmAndSurfaceLock = true;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    pixelMap = screenSession->GetScreenSnapshotWithAllWindows(scaleX, scaleY, isNeedCheckDrmAndSurfaceLock);
    EXPECT_EQ(pixelMap, nullptr);
}

/**
 * @tc.name: GetScreenSnapshotWithAllWindows02
 * @tc.desc: GetScreenSnapshotWithAllWindows02 Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetScreenSnapshotWithAllWindows02, TestSize.Level2)
{
    ScreenId screenId = 100;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    Rosen::RSDisplayNodeConfig config = { .screenId = 100 };
    std::shared_ptr<RSDisplayNode> displayNode = Rosen::RSDisplayNode::Create(config);
    screenSession->SetDisplayNode(displayNode);
    float scaleX = 1.0;
    float scaleY = 1.0;
    bool isNeedCheckDrmAndSurfaceLock = true;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    pixelMap = screenSession->GetScreenSnapshotWithAllWindows(scaleX, scaleY, isNeedCheckDrmAndSurfaceLock);
    EXPECT_EQ(pixelMap, nullptr);
}

/**
 * @tc.name: UpdatePropertyByFoldControl
 * @tc.desc: UpdatePropertyByFoldControl Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdatePropertyByFoldControl, TestSize.Level2)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        GTEST_SKIP() << "Skip test when device is not secondaryDisplayFoldDevice.";
    }
    ScreenId screenId = 100;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ScreenProperty newProperty;
    screenSession->GetScreenProperty().UpdateScreenRotation(Rotation::ROTATION_0);
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    screenSession->SetRotationCorrectionMap(rotationCorrectionMap);
    screenSession->UpdatePropertyByFoldControl(newProperty, FoldDisplayMode::MAIN, true);
    screenProperty = screenSession->GetScreenProperty();
    EXPECT_EQ(screenProperty.GetDisplayOrientation(), DisplayOrientation::LANDSCAPE_INVERTED);
    screenSession->UpdatePropertyByFoldControl(newProperty, FoldDisplayMode::MAIN, false);
    screenProperty = screenSession->GetScreenProperty();
    EXPECT_EQ(screenProperty.GetDisplayOrientation(), DisplayOrientation::PORTRAIT);
}

/**
 * @tc.name: AddRotationCorrection
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, AddRotationCorrection, TestSize.Level1)
{
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(session, nullptr);
    Rotation rotation = Rotation::ROTATION_0;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    session->SetRotationCorrectionMap(rotationCorrectionMap);
    session->AddRotationCorrection(rotation, FoldDisplayMode::MAIN);
    EXPECT_EQ(rotation, Rotation::ROTATION_270);
 
    rotation = Rotation::ROTATION_0;
    session->AddRotationCorrection(rotation, FoldDisplayMode::FULL);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
 
    rotation = static_cast<Rotation>(100);
    session->AddRotationCorrection(rotation, FoldDisplayMode::FULL);
}

/**
 * @tc.name: TestPropertyChangeAllCases
 * @tc.desc: Verify PropertyChange behavior for multiple scenarios
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, TestPropertyChangeAllCases, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);

    // Case 1: reason = VIRTUAL_PIXEL_RATIO_CHANGE -> no listener call
    {
        auto listener = std::make_unique<MockScreenChangeListener>();
        EXPECT_CALL(*listener, OnPropertyChange(_, _, _)).Times(0);

        session->screenChangeListenerList_.push_back(listener.get());
        session->PropertyChange(screenProperty, ScreenPropertyChangeReason::VIRTUAL_PIXEL_RATIO_CHANGE);
        session->screenChangeListenerList_.clear();
    }

    // Case 2: listener list is empty -> early return
    {
        session->PropertyChange(screenProperty, ScreenPropertyChangeReason::UNDEFINED);
        SUCCEED();
    }

    // Case 3: listener list contains nullptr -> skip safely
    {
        auto reason = ScreenPropertyChangeReason::UNDEFINED;
        auto listener = std::make_unique<MockScreenChangeListener>();
        EXPECT_CALL(*listener, OnPropertyChange(_, reason, screenId)).Times(1);

        session->screenChangeListenerList_.push_back(nullptr);
        session->screenChangeListenerList_.push_back(listener.get());
        session->PropertyChange(screenProperty, reason);
        session->screenChangeListenerList_.clear();
    }

    // Case 4: normal case -> all valid listeners should be called
    {
        auto reason = ScreenPropertyChangeReason::UNDEFINED;
        auto listener1 = std::make_unique<MockScreenChangeListener>();
        auto listener2 = std::make_unique<MockScreenChangeListener>();
        EXPECT_CALL(*listener1, OnPropertyChange(_, reason, screenId)).Times(1);
        EXPECT_CALL(*listener2, OnPropertyChange(_, reason, screenId)).Times(1);

        session->screenChangeListenerList_.push_back(listener1.get());
        session->screenChangeListenerList_.push_back(listener2.get());
        session->PropertyChange(screenProperty, reason);
        session->screenChangeListenerList_.clear();
    }
}

/**
 * @tc.name  : CalcRotationSystemInner
 * @tc.desc  : CalcRotationSystemInner
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, CalcRotationSystemInner, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    Orientation orientation = Orientation::UNSPECIFIED;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::UNKNOWN;
    Rotation rotation = screenSession->CalcRotationSystemInner(orientation, foldDisplayMode);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
 
    sptr<SupportedScreenModes> supportedScreenMode = sptr<SupportedScreenModes>::MakeSptr();
    supportedScreenMode->id_ = 1;
    supportedScreenMode->width_ = 1000;
    supportedScreenMode->height_ = 2000;
    supportedScreenMode->refreshRate_ = 90;
    screenSession->modes_.emplace_back(supportedScreenMode);
    screenSession->activeIdx_ = 0;
    rotation = screenSession->CalcRotationSystemInner(orientation, foldDisplayMode);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
 
    orientation = Orientation::VERTICAL;
    rotation = screenSession->CalcRotationSystemInner(orientation, foldDisplayMode);
    EXPECT_EQ(rotation, Rotation::ROTATION_0);
 
    orientation = Orientation::HORIZONTAL;
    rotation = screenSession->CalcRotationSystemInner(orientation, foldDisplayMode);
    EXPECT_EQ(rotation, Rotation::ROTATION_90);
 
    orientation = Orientation::REVERSE_VERTICAL;
    rotation = screenSession->CalcRotationSystemInner(orientation, foldDisplayMode);
    EXPECT_EQ(rotation, Rotation::ROTATION_180);
 
    orientation = Orientation::REVERSE_HORIZONTAL;
    rotation = screenSession->CalcRotationSystemInner(orientation, foldDisplayMode);
    EXPECT_EQ(rotation, Rotation::ROTATION_270);
}
 
/**
 * @tc.name  : ScreenOrientationChange
 * @tc.desc  : ScreenOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, ScreenOrientationChange, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr();
    Orientation orientation = Orientation::UNSPECIFIED;
    session->ScreenOrientationChange(orientation, FoldDisplayMode::UNKNOWN, true);
    EXPECT_TRUE(g_errLog.find("set orientation from napi") != std::string::npos);
    LOG_SetCallback(nullptr);
    g_errLog.clear();
}

/**
 * @tc.name  : GetSupportsFocus
 * @tc.desc  : GetSupportsFocus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSupportsFocus, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    auto supportInput = session->GetSupportsFocus();
    EXPECT_EQ(supportInput, true);
}

/**
 * @tc.name  : SetSupportsFocus
 * @tc.desc  : SetSupportsFocus
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetSupportsFocus, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    session->SetSupportsFocus(false);
    auto supportInput = session->GetSupportsFocus();
    EXPECT_EQ(supportInput, false);

    session->SetSupportsFocus(true);
    supportInput = session->GetSupportsFocus();
    EXPECT_EQ(supportInput, true);
}

/**
 * @tc.name  : GetSupportsInput
 * @tc.desc  : GetSupportsInput
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, GetSupportsInput, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    auto supportInput = session->GetSupportsInput();
    EXPECT_EQ(supportInput, true);
}

/**
 * @tc.name  : SetSupportsInput
 * @tc.desc  : SetSupportsInput
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetSupportsInput, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    session->SetSupportsInput(false);
    auto supportInput = session->GetSupportsInput();
    EXPECT_EQ(supportInput, false);

    session->SetSupportsInput(true);
    supportInput = session->GetSupportsInput();
    EXPECT_EQ(supportInput, true);
}

/**
 * @tc.name  : SetBundleName
 * @tc.desc  : SetBundleName
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetBundleName, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    session->SetBundleName("test");
    auto bundleName = session->GetBundleName();
    EXPECT_EQ(bundleName, "test");
}

/**
 * @tc.name  : UpdateRotationOrientationMap01
 * @tc.desc  : UpdateRotationOrientationMap01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRotationOrientationMap01, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);

    UniqueScreenRotationOptions rotationOptions;
    int32_t rotation = static_cast<int32_t>(Rotation::ROTATION_0);
    int32_t orientation = static_cast<int32_t>(DisplayOrientation::LANDSCAPE);

    bool result = session->UpdateRotationOrientationMap(rotationOptions, rotation, orientation);
    std::map<int32_t, int32_t> rotationOrientationMap = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
    EXPECT_EQ(result, true);
    EXPECT_EQ(rotationOptions.rotationOrientationMap_, rotationOrientationMap);
}

/**
 * @tc.name  : UpdateRotationOrientationMap02
 * @tc.desc  : UpdateRotationOrientationMap02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRotationOrientationMap02, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);

    UniqueScreenRotationOptions rotationOptions;
    int32_t rotation = ROTATION_UNSET;
    int32_t orientation = static_cast<int32_t>(DisplayOrientation::LANDSCAPE);

    bool result = session->UpdateRotationOrientationMap(rotationOptions, rotation, orientation);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : UpdateRotationOrientationMap03
 * @tc.desc  : UpdateRotationOrientationMap03
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateRotationOrientationMap03, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    constexpr int32_t INVALID_ORIENTATION = 4;

    UniqueScreenRotationOptions rotationOptions;
    int32_t rotation = static_cast<int32_t>(Rotation::ROTATION_0);
    int32_t orientation = INVALID_ORIENTATION;

    bool result = session->UpdateRotationOrientationMap(rotationOptions, rotation, orientation);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : SetAndGetCurrentRotationCorrection
 * @tc.desc  : SetSupportsInput
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, SetAndGetCurrentRotationCorrection, TestSize.Level1)
{
    ScreenId screenId = 10000;
    ScreenProperty screenProperty;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    session->SetCurrentRotationCorrection(Rotation::ROTATION_270);
    EXPECT_EQ(session->GetCurrentRotationCorrection(), Rotation::ROTATION_270);
}

/**
 * @tc.name  : HandleResolutionEffectPropertyChange
 * @tc.desc  : test HandleResolutionEffectPropertyChange Different rsid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, HandleResolutionEffectPropertyChange1, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    LOG_SetCallback(MyLogCallback);
    g_errLog.clear();
    ScreenProperty property1, property2;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(0, property1, 0);
    property1.SetRsId(1);
    session->HandleResolutionEffectPropertyChange(property1, property2);
    EXPECT_TRUE(g_errLog.find("no need handle") != std::string::npos);
    g_errLog.clear();

    property1.SetRsId(0);
    session->HandleResolutionEffectPropertyChange(property1, property2);
    EXPECT_FALSE(g_errLog.find("no need handle") != std::string::npos);

    LOG_SetCallback(nullptr);
    g_errLog.clear();
}

/**
 * @tc.name  : HandleResolutionEffectPropertyChange
 * @tc.desc  : test HandleResolutionEffectPropertyChange Different rotation
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, HandleResolutionEffectPropertyChange2, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    ScreenProperty property1, property2;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(0, property1, 0);
    property1.SetRsId(0);
    property1.SetScreenRotation(Rotation::ROTATION_0);
    auto bounds = property2.GetBounds();
    bounds.rect_.width_ = 1;
    bounds.rect_.height_ = 2;
    property2.SetBounds(bounds);
    session->HandleResolutionEffectPropertyChange(property1, property2);
    EXPECT_EQ(property1.GetBounds().rect_.width_, 1);
    EXPECT_EQ(property1.GetBounds().rect_.height_, 2);
}


/**
 * @tc.name  : UpdateScbScreenPropertyForSuperFlod
 * @tc.desc  : test UpdateScbScreenPropertyForSuperFlod
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionTest, UpdateScbScreenPropertyForSuperFlod, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    LOG_SetCallback(MyLogCallbackWithAllLog);
    g_errLog.clear();
    ScreenProperty property;
    sptr<ScreenSession> session = sptr<ScreenSession>::MakeSptr(0, property, 0);

    property.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::KEYBOARD_ON);
    session->UpdateScbScreenPropertyForSuperFlod(property);
    EXPECT_TRUE(g_errLog.find("handle keyboard on and keyboard succ") != std::string::npos);
    g_errLog.clear();

    property.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::KEYBOARD_OFF);
    session->UpdateScbScreenPropertyForSuperFlod(property);
    EXPECT_TRUE(g_errLog.find("handle keyboard on and keyboard succ") != std::string::npos);
    g_errLog.clear();

    property.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON);
    session->UpdateScbScreenPropertyForSuperFlod(property);
    EXPECT_TRUE(g_errLog.find("handle system keyboard on and system keyboard succ") != std::string::npos);
    g_errLog.clear();

    property.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF);
    session->UpdateScbScreenPropertyForSuperFlod(property);
    EXPECT_TRUE(g_errLog.find("handle system keyboard on and system keyboard succ") != std::string::npos);
    g_errLog.clear();
    LOG_SetCallback(nullptr);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
