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

#include "iremote_object_mocker.h"
#include "mock/mock_message_parcel.h"
#include "zidl/screen_session_manager_client_proxy.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg += msg;
    }
}
class ScreenSessionManagerClientProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObjectMocker> iRemoteObjectMocker_;
    sptr<ScreenSessionManagerClientProxy> screenSessionManagerClientProxy_;
};

void ScreenSessionManagerClientProxyTest::SetUpTestCase()
{
}

void ScreenSessionManagerClientProxyTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void ScreenSessionManagerClientProxyTest::SetUp()
{
    iRemoteObjectMocker_ = new IRemoteObjectMocker();
    screenSessionManagerClientProxy_ = new ScreenSessionManagerClientProxy(iRemoteObjectMocker_);
}

void ScreenSessionManagerClientProxyTest::TearDown()
{
    screenSessionManagerClientProxy_ = nullptr;
}

/**
 * @tc.name: OnScreenConnectionChanged
 * @tc.desc: OnScreenConnectionChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnScreenConnectionChanged, TestSize.Level1)
{
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
    };
    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnScreenConnectionChanged(option, screenEvent);
}

/**
 * @tc.name: SwitchUserCallback
 * @tc.desc: SwitchUserCallback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, SwitchUserCallback, TestSize.Level1)
{
    std::vector<int32_t> oldScbPids {0, 1};
    int32_t currentScbPid = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->SwitchUserCallback(oldScbPids, currentScbPid);
}

/**
 * @tc.name: OnScreenExtendChanged
 * @tc.desc: OnScreenExtendChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnScreenExtendChanged, TestSize.Level1)
{
    ScreenId mainScreenId = 0;
    ScreenId extendScreenId = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnScreenExtendChanged(mainScreenId, extendScreenId);
}

/**
 * @tc.name: OnPropertyChanged
 * @tc.desc: OnPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnPropertyChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenProperty property;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnPropertyChanged(screenId, property, reason);
}

/**
 * @tc.name: OnSuperFoldStatusChanged
 * @tc.desc: OnSuperFoldStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnSuperFoldStatusChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    SuperFoldStatus superFoldStatus = SuperFoldStatus::UNKNOWN;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnSuperFoldStatusChanged(screenId, superFoldStatus);
}

/**
 * @tc.name: OnPowerStatusChanged
 * @tc.desc: OnPowerStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnPowerStatusChanged, TestSize.Level1)
{
    DisplayPowerEvent event = DisplayPowerEvent::DISPLAY_ON;
    EventStatus status = EventStatus::BEGIN;
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnPowerStatusChanged(event, status, reason);
}

/**
 * @tc.name: OnSensorRotationChanged
 * @tc.desc: OnSensorRotationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnSensorRotationChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    float sensorRotation = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnSensorRotationChanged(screenId, sensorRotation, false);
}

/**
 * @tc.name: OnHoverStatusChanged
 * @tc.desc: OnHoverStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnHoverStatusChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    int32_t hoverStatus = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnHoverStatusChanged(screenId, hoverStatus);
}

/**
 * @tc.name: OnScreenOrientationChanged
 * @tc.desc: OnScreenOrientationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnScreenOrientationChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    float screenOrientation = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnScreenOrientationChanged(screenId, screenOrientation);
}

/**
 * @tc.name: OnScreenRotationLockedChanged
 * @tc.desc: OnScreenRotationLockedChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnScreenRotationLockedChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    bool isLocked = false;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnScreenRotationLockedChanged(screenId, isLocked);
}

/**
 * @tc.name: OnDisplayStateChanged
 * @tc.desc: OnDisplayStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnDisplayStateChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap {
        { displayInfo->GetDisplayId(), displayInfo },
    };
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnDisplayStateChanged(screenId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: OnGetSurfaceNodeIdsFromMissionIdsChanged
 * @tc.desc: OnGetSurfaceNodeIdsFromMissionIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnGetSurfaceNodeIdsFromMissionIdsChanged, TestSize.Level1)
{
    std::vector<uint64_t> missionIds = {0, 1};
    std::vector<uint64_t> surfaceNodeIds;
    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnGetSurfaceNodeIdsFromMissionIdsChanged(missionIds, surfaceNodeIds);
}

/**
 * @tc.name: OnUpdateFoldDisplayMode
 * @tc.desc: OnUpdateFoldDisplayMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnUpdateFoldDisplayMode, TestSize.Level1)
{
    FoldDisplayMode displayMode = FoldDisplayMode::MAIN;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: OnScreenshot
 * @tc.desc: OnScreenshot test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnScreenshot, TestSize.Level1)
{
    ScreenId screenId = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnScreenshot(screenId);
}

/**
 * @tc.name: OnImmersiveStateChanged
 * @tc.desc: OnImmersiveStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnImmersiveStateChanged, TestSize.Level1)
{
    bool immersive = false;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnImmersiveStateChanged(0u, immersive);
}

/**
 * @tc.name: SetDisplayNodeScreenId
 * @tc.desc: SetDisplayNodeScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, SetDisplayNodeScreenId, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenId displayNodeScreenId = 0;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
}

/**
 * @tc.name: SetVirtualPixelRatioSystem
 * @tc.desc: SetVirtualPixelRatioSystem test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, SetVirtualPixelRatioSystem, TestSize.Level1)
{
    ScreenId screenId = 0;
    float virtualPixelRatio = 1.0f;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
}

/**
 * @tc.name: OnFoldStatusChangedReportUE
 * @tc.desc: OnFoldStatusChangedReportUE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnFoldStatusChangedReportUE, TestSize.Level1)
{
    std::vector<std::string> screenFoldInfo = {"0", "0", "0", "0"};

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnFoldStatusChangedReportUE(screenFoldInfo);
}

/**
 * @tc.name: ScreenCaptureNotify
 * @tc.desc: ScreenCaptureNotify test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, ScreenCaptureNotify, TestSize.Level1)
{
    ScreenId screenId = 0;
    int32_t uid = 0;
    std::string clientName = "test";

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->ScreenCaptureNotify(screenId, uid, clientName);
}

/**
 * @tc.name: OnCreateScreenSessionOnly
 * @tc.desc: OnCreateScreenSessionOnly test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnCreateScreenSessionOnly, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenId rsId = 0;
    std::string name = "test";
    bool isExtend = false;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnCreateScreenSessionOnly(screenId, rsId, name, isExtend);
}

/**
 * @tc.name: OnExtendDisplayNodeChange
 * @tc.desc: OnExtendDisplayNodeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnExtendDisplayNodeChange, TestSize.Level1)
{
    ScreenId firstId = 0;
    ScreenId secondId = 12;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnExtendDisplayNodeChange(firstId, secondId);
}

/**
 * @tc.name: OnMainDisplayNodeChange
 * @tc.desc: OnMainDisplayNodeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnMainDisplayNodeChange, TestSize.Level1)
{
    ScreenId mainScreenId = 0;
    ScreenId extendScreenId = 12;
    ScreenId extendRSId = 12;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnMainDisplayNodeChange(mainScreenId, extendScreenId, extendRSId);
}

/**
 * @tc.name: SetScreenCombination
 * @tc.desc: SetScreenCombination test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, SetScreenCombination, TestSize.Level1)
{
    ScreenId mainScreenId = 0;
    ScreenId extendScreenId = 12;
    ScreenCombination combination = ScreenCombination::SCREEN_ALONE;

    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->SetScreenCombination(mainScreenId, extendScreenId, combination);
}

/**
 * @tc.name: OnDumperClientScreenSessions
 * @tc.desc: OnDumperClientScreenSessions test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnDumperClientScreenSessions, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    screenSessionManagerClientProxy_->OnDumperClientScreenSessions();
}

/**
 * @tc.name: OnBeforeScreenPropertyChanged
 * @tc.desc: OnBeforeScreenPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnBeforeScreenPropertyChanged, TestSize.Level1)
{
    screenSessionManagerClientProxy_->OnBeforeScreenPropertyChanged(FoldStatus::UNKNOWN);
    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
}

/**
 * @tc.name: OnSetSurfaceNodeIdsChanged
 * @tc.desc: OnSetSurfaceNodeIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnSetSurfaceNodeIdsChanged, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    DisplayId displayId = 10;
    std::vector<uint64_t> surfaceNodeIds = { 100, 101 };
    auto ssmProxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    ssmProxy->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    ssmProxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(nullptr, ssmProxy);
    ssmProxy->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ssmProxy->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ssmProxy->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ssmProxy->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnVirtualScreenDisconnected
 * @tc.desc: OnVirtualScreenDisconnected test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnVirtualScreenDisconnected, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    DisplayId displayId = 10;
    auto ssmProxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    ssmProxy->OnVirtualScreenDisconnected(displayId);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    ssmProxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(nullptr, ssmProxy);
    ssmProxy->OnVirtualScreenDisconnected(displayId);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ssmProxy->OnVirtualScreenDisconnected(displayId);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ssmProxy->OnVirtualScreenDisconnected(displayId);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ssmProxy->OnVirtualScreenDisconnected(displayId);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnTentModeChange
 * @tc.desc: OnTentModeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnTentModeChange, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    DisplayId displayId = 10;
    auto ssmProxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    ssmProxy->OnTentModeChange(TentMode::UNKNOWN);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    ssmProxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(nullptr, ssmProxy);
    ssmProxy->OnTentModeChange(TentMode::UNKNOWN);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ssmProxy->OnTentModeChange(TentMode::UNKNOWN);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ssmProxy->OnTentModeChange(TentMode::UNKNOWN);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ssmProxy->OnTentModeChange(TentMode::UNKNOWN);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnScreenConnectionChangedMock
 * @tc.desc: OnScreenConnectionChangedMock
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnScreenConnectionChangedMock, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClientProxy_ != nullptr);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
        .rotationCorrectionMap_ = rotationCorrectionMap,
    };
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    // remote == nullptr
    auto proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    proxy->OnScreenConnectionChanged(option, screenEvent);
    // SendRequest failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->OnScreenConnectionChanged(option, screenEvent);
    remoteMocker->SetRequestResult(ERR_NONE);
    // Pass all
    proxy->OnScreenConnectionChanged(option, screenEvent);
    // write param failed
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    screenSessionManagerClientProxy_->OnScreenConnectionChanged(option, screenEvent);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: ScreenConnectWriteParam
 * @tc.desc: ScreenConnectWriteParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, ScreenConnectWriteParam, TestSize.Level1)
{
    MessageParcel data;
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    rotationCorrectionMap.insert({FoldDisplayMode::MAIN, 3});
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
        .rotationCorrectionMap_ = rotationCorrectionMap,
        .isRotationLocked_ = true,
        .rotation_ = 1,
        .rotationOrientationMap_ = {{0, 0}, {1, 1}, {2, 2}, {3, 3}}
    };
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);

    bool ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_TRUE(ret);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_FALSE(ret);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_FALSE(ret);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_FALSE(ret);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_FALSE(ret);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_FALSE(ret);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = proxy->ScreenConnectWriteParam(option, screenEvent, data);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: OnAnimationFinish
 * @tc.desc: OnAnimationFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, OnAnimationFinish, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    auto proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    proxy->OnAnimationFinish();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    proxy->OnAnimationFinish();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->OnAnimationFinish();
    remoteMocker->SetRequestResult(ERR_NONE);
    proxy->OnAnimationFinish();
    MockMessageParcel::ClearAllErrorFlag();
}

/**
* @tc.name: OnSensorRotationChanged02
* @tc.desc: OnSensorRotationChanged02 test
* @tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientProxyTest, OnSensorRotationChanged02, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ScreenId screenId = 666;
    float sensorRotation = 90.f;
    bool isSwitchUser = false;

    MockMessageParcel::ClearAllErrorFlag();
    auto proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    proxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    EXPECT_TRUE(logMsg.find("remote is nullptr") != std::string::npos);
    logMsg.clear();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    proxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    logMsg.clear();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    proxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    EXPECT_TRUE(logMsg.find("Write screenId failed") != std::string::npos);
    logMsg.clear();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    proxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    EXPECT_TRUE(logMsg.find("Write isSwitchUser failed") != std::string::npos);
    logMsg.clear();

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);
    logMsg.clear();

    remoteMocker->SetRequestResult(ERR_NONE);
    proxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    EXPECT_FALSE(logMsg.find("SendRequest failed") != std::string::npos);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}
 
/**
 * @tc.name: SetInternalClipToBounds
 * @tc.desc: SetInternalClipToBounds test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientProxyTest, SetInternalClipToBounds, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ScreenId screenId = 0;
    bool clipToBounds = true;

    MockMessageParcel::ClearAllErrorFlag();
    auto proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(nullptr);
    proxy->SetInternalClipToBounds(screenId, clipToBounds);
    EXPECT_TRUE(logMsg.find("remote is nullptr") != std::string::npos);
    logMsg.clear();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<ScreenSessionManagerClientProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    proxy->SetInternalClipToBounds(screenId, clipToBounds);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    logMsg.clear();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    proxy->SetInternalClipToBounds(screenId, clipToBounds);
    EXPECT_TRUE(logMsg.find("Write screenId failed") != std::string::npos);
    logMsg.clear();
 
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    proxy->SetInternalClipToBounds(screenId, clipToBounds);
    EXPECT_TRUE(logMsg.find("Write clipToBounds failed") != std::string::npos);
    logMsg.clear();

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->SetInternalClipToBounds(screenId, clipToBounds);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);
    remoteMocker->SetRequestResult(ERR_NONE);
    proxy->SetInternalClipToBounds(screenId, clipToBounds);
    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
}
} // namespace Rosen
} // namespace OHOS
