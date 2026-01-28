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
#include <transaction/rs_marshalling_helper.h>

#include "screen_session_manager_client.h"
#include "screen_session_manager_client_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ScreenSessionManagerClientStubTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    sptr<ScreenSessionManagerClientStub> screenSessionManagerClientStub_;
};

void ScreenSessionManagerClientStubTest::SetUp()
{
    screenSessionManagerClientStub_ = new ScreenSessionManagerClient();
}

void ScreenSessionManagerClientStubTest::TearDown()
{
    screenSessionManagerClientStub_ = nullptr;
}

/**
 * @tc.name: OnRemoteRequest01
 * @tc.desc: TRANS_ID_ON_SCREEN_CONNECTION_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenEvent event = ScreenEvent::UNKNOWN;
    data.WriteUint8(static_cast<uint8_t>(event));
    ScreenId rsId = 0;
    data.WriteUint64(rsId);
    std::string name = "screen";
    data.WriteString(name);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_CONNECTION_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: TRANS_ID_ON_PROPERTY_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    data.WriteUint32(static_cast<uint32_t>(reason));

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_PROPERTY_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, 0);
}

/**
 * @tc.name: OnRemoteRequest03
 * @tc.desc: TRANS_ID_ON_POWER_STATUS_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    DisplayPowerEvent event = DisplayPowerEvent::DISPLAY_OFF;
    data.WriteUint32(static_cast<uint32_t>(event));
    EventStatus status = EventStatus::BEGIN;
    data.WriteUint32(static_cast<uint32_t>(status));
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    data.WriteUint32(static_cast<uint32_t>(reason));

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_POWER_STATUS_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest04
 * @tc.desc: TRANS_ID_ON_SENSOR_ROTATION_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    float rotation = 90.0f;
    data.WriteFloat(rotation);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SENSOR_ROTATION_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest05
 * @tc.desc: TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    float rotation = 0.0f;
    data.WriteFloat(rotation);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest06
 * @tc.desc: TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest06, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    bool isLocked = false;
    data.WriteBool(isLocked);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest07
 * @tc.desc: TRANS_ID_ON_DISPLAY_STATE_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest07, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    DisplayId screenId = 0;
    data.WriteUint64(screenId);
    sptr<DisplayInfo> displayInfo;
    data.WriteStrongParcelable(displayInfo);
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    for (auto [id, info] : displayInfoMap) {
        data.WriteUint64(id);
        data.WriteStrongParcelable(info);
    }
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    data.WriteUint32(static_cast<uint32_t>(type));
    auto mapSize = static_cast<uint32_t>(displayInfoMap.size());
    data.WriteUint32(mapSize);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_DISPLAY_STATE_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest08
 * @tc.desc: TRANS_ID_ON_SCREEN_SHOT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest08, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    DisplayId screenId = 0;
    data.WriteUint64(screenId);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_SHOT);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest09
 * @tc.desc: TRANS_ID_ON_IMMERSIVE_STATE_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest09, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    data.WriteUint64(0u);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_IMMERSIVE_STATE_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest10
 * @tc.desc: TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest10, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenId displayNodeScreenId = 0;
    data.WriteUint64(displayNodeScreenId);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest11
 * @tc.desc: TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest11, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    std::vector<uint64_t> missionIds = {0, 1};
    data.WriteUInt64Vector(missionIds);
    std::vector<uint64_t> surfaceNodeIds = {0, 1};
    data.WriteUInt64Vector(surfaceNodeIds);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest12
 * @tc.desc: TRANS_ID_SET_FOLD_DISPLAY_MODE
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest12, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    FoldDisplayMode displayMode = FoldDisplayMode::FULL;
    data.WriteUint32(static_cast<uint32_t>(displayMode));

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest13
 * @tc.desc: TRANS_ID_ON_SWITCH_USER_CMD
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest13, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    std::vector<int32_t> oldScbPids = {0, 1};
    data.WriteInt32Vector(oldScbPids);
    int32_t currentScbPid = 0;
    data.WriteInt32(currentScbPid);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SWITCH_USER_CMD);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest14
 * @tc.desc: TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest14, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    float virtualPixelRatio = 0.0f;
    data.WriteFloat(virtualPixelRatio);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest15
 * @tc.desc: TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest15, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    std::vector<std::string> screenFoldInfo{""};
    data.WriteStringVector(screenFoldInfo);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest16
 * @tc.desc: default
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest16, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(20);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest17
 * @tc.desc: TRANS_ID_ON_SCREEN_EXTEND_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest17, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenId mainScreenId = 0;
    data.WriteUint64(mainScreenId);
    ScreenId extendScreenId = 100;
    data.WriteUint64(extendScreenId);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_EXTEND_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest18
 * @tc.desc: TRANS_ID_ON_HOVER_STATUS_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest18, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    int32_t hoverStatus = 1;
    data.WriteInt32(hoverStatus);

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_HOVER_STATUS_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest19
 * @tc.desc: TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest19, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    SuperFoldStatus superFoldStatus = SuperFoldStatus::UNKNOWN;
    data.WriteInt32(static_cast<uint32_t>(superFoldStatus));

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest20
 * @tc.desc: TRANS_ID_ON_SCREEN_MODE_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, OnRemoteRequest20, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenModeChangeEvent screenModeChangeEvent = ScreenModeChangeEvent::UNKNOWN;
    data.WriteInt32(static_cast<uint32_t>(screenModeChangeEvent));

    uint32_t code = static_cast<uint32_t>(
        IScreenSessionManagerClient::ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_MODE_CHANGED);
    int res = screenSessionManagerClientStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: HandleOnScreenConnectionChanged
 * @tc.desc: HandleOnScreenConnectionChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenConnectionChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    data.WriteUint8(static_cast<uint8_t>(screenEvent));
    ScreenId rsId = 0;
    data.WriteUint64(rsId);
    std::string name;
    data.WriteString(name);
    data.WriteBool(false);
    data.WriteUint64(1);
    data.WriteUint32(1);
    data.WriteUint32(2);
    data.WriteBool(true);
    data.WriteInt32(1);
    uint64_t mapSize = 4;
    data.WriteUint64(mapSize);
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(2);
    data.WriteInt32(2);
    data.WriteInt32(3);
    data.WriteInt32(3);

    int ret = screenSessionManagerClientStub_->HandleOnScreenConnectionChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnScreenConnectionChanged04
 * @tc.desc: HandleOnScreenConnectionChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenConnectionChanged04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    ScreenId rsId = 0;
    data.WriteUint64(rsId);
    std::string name = "test";
    data.WriteString(name);
    data.WriteBool(false);
    std::string innerName = "test";
    data.WriteString(innerName);
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    data.WriteUint8(static_cast<uint8_t>(screenEvent));
    uint64_t size = 1;
    data.WriteUint64(size);
    data.WriteUint32(1);
    data.WriteUint32(2);
    data.WriteBool(true);
    data.WriteInt32(1);

    int ret = screenSessionManagerClientStub_->HandleOnScreenConnectionChanged(data, reply);
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name: HandleOnScreenConnectionChanged05
 * @tc.desc: HandleOnScreenConnectionChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenConnectionChanged05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    ScreenId rsId = 0;
    data.WriteUint64(rsId);
    std::string name = "test";
    data.WriteString(name);
    data.WriteBool(false);
    std::string innerName = "test";
    data.WriteString(innerName);
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    data.WriteUint8(static_cast<uint8_t>(screenEvent));
    uint64_t size = 1;
    data.WriteUint64(size);
    data.WriteUint32(1);
    data.WriteUint32(2);
    data.WriteBool(true);
    data.WriteInt32(1);
    data.WriteInt32(3);
    uint64_t mapSize = 5;
    data.WriteUint64(mapSize);
    for (int i = 0; i < mapSize; ++i) {
        data.WriteInt32(i);
        data.WriteInt32(i);
    }
    int ret = screenSessionManagerClientStub_->HandleOnScreenConnectionChanged(data, reply);
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name: HandleOnPropertyChanged
 * @tc.desc: HandleOnPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnPropertyChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenProperty property;
    RSMarshallingHelper::Marshalling(data, property);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    data.WriteUint32(static_cast<uint32_t>(reason));

    int ret = screenSessionManagerClientStub_->HandleOnPropertyChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnSuperFoldStatusChanged
 * @tc.desc: HandleOnSuperFoldStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnSuperFoldStatusChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenProperty property;
    RSMarshallingHelper::Marshalling(data, property);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    data.WriteUint32(static_cast<uint32_t>(reason));

    int ret = screenSessionManagerClientStub_->HandleOnSuperFoldStatusChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnHoverStatusChanged
 * @tc.desc: HandleOnHoverStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnHoverStatusChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    int32_t hoverStatus = 0;
    data.WriteInt32(hoverStatus);

    int ret = screenSessionManagerClientStub_->HandleOnHoverStatusChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnSensorRotationChanged
 * @tc.desc: HandleOnSensorRotationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnSensorRotationChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    float sensorRotation = 0;
    data.WriteFloat(sensorRotation);

    int ret = screenSessionManagerClientStub_->HandleOnSensorRotationChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnScreenExtendChanged
 * @tc.desc: HandleOnScreenExtendChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenExtendChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
  
    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId mainScreenId = 0;
    ScreenId extendScreenId = 100;
    data.WriteUint64(mainScreenId);
    data.WriteUint64(extendScreenId);

    int ret = screenSessionManagerClientStub_->HandleOnScreenExtendChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnScreenOrientationChanged
 * @tc.desc: HandleOnScreenOrientationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenOrientationChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    float screenOrientation = 0;
    data.WriteFloat(screenOrientation);
    data.WriteBool(false);

    int ret = screenSessionManagerClientStub_->HandleOnScreenOrientationChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnScreenRotationLockedChanged
 * @tc.desc: HandleOnScreenRotationLockedChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenRotationLockedChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    bool isLocked = false;
    data.WriteBool(isLocked);

    int ret = screenSessionManagerClientStub_->HandleOnScreenRotationLockedChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnDisplayStateChanged
 * @tc.desc: HandleOnDisplayStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnDisplayStateChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    data.WriteStrongParcelable(displayInfo);
    data.WriteUint32(0);
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    data.WriteUint32(static_cast<uint32_t>(type));

    int ret = screenSessionManagerClientStub_->HandleOnDisplayStateChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnScreenshot
 * @tc.desc: HandleOnScreenshot test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenshot, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);

    int ret = screenSessionManagerClientStub_->HandleOnScreenshot(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnImmersiveStateChanged
 * @tc.desc: HandleOnImmersiveStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnImmersiveStateChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    int ret = screenSessionManagerClientStub_->HandleOnImmersiveStateChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnPowerStatusChanged
 * @tc.desc: HandleOnPowerStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnPowerStatusChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    int ret = screenSessionManagerClientStub_->HandleOnPowerStatusChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnGetSurfaceNodeIdsFromMissionIdsChanged
 * @tc.desc: HandleOnGetSurfaceNodeIdsFromMissionIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest,
    HandleOnGetSurfaceNodeIdsFromMissionIdsChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    int ret = screenSessionManagerClientStub_->HandleOnGetSurfaceNodeIdsFromMissionIdsChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnSetSurfaceNodeIdsChanged
 * @tc.desc: HandleOnSetSurfaceNodeIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnSetSurfaceNodeIdsChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    int ret = screenSessionManagerClientStub_->HandleOnSetSurfaceNodeIdsChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnVirtualScreenDisconnected
 * @tc.desc: HandleOnVirtualScreenDisconnected test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnVirtualScreenDisconnected, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    int ret = screenSessionManagerClientStub_->HandleOnVirtualScreenDisconnected(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnSetDisplayNodeScreenId
 * @tc.desc: HandleOnSetDisplayNodeScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnSetDisplayNodeScreenId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    int ret = screenSessionManagerClientStub_->HandleOnSetDisplayNodeScreenId(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleSwitchUserCallback
 * @tc.desc: HandleSwitchUserCallback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleSwitchUserCallback, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    std::vector<int32_t> oldScbPids {0, 1};
    data.WriteInt32Vector(oldScbPids);
    int32_t currentScbPid = 1;
    data.WriteInt32(currentScbPid);

    int ret = screenSessionManagerClientStub_->HandleSwitchUserCallback(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnUpdateFoldDisplayMode
 * @tc.desc: HandleOnUpdateFoldDisplayMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnUpdateFoldDisplayMode, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::MAIN;
    data.WriteUint32(static_cast<uint32_t>(foldDisplayMode));

    int ret = screenSessionManagerClientStub_->HandleOnUpdateFoldDisplayMode(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleSetVirtualPixelRatioSystem
 * @tc.desc: HandleSetVirtualPixelRatioSystem test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleSetVirtualPixelRatioSystem, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    float virtualPixelRatio = 1.0f;
    data.WriteFloat(virtualPixelRatio);

    int ret = screenSessionManagerClientStub_->HandleSetVirtualPixelRatioSystem(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnFoldStatusChangedReportUE
 * @tc.desc: HandleOnFoldStatusChangedReportUE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnFoldStatusChangedReportUE, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    std::vector<std::string> screenFoldInfo = {"0", "0", "0", "0"};
    data.WriteStringVector(screenFoldInfo);

    int ret = screenSessionManagerClientStub_->HandleOnFoldStatusChangedReportUE(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleScreenCaptureNotify
 * @tc.desc: HandleScreenCaptureNotify test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleScreenCaptureNotify, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    data.WriteInt32(0);
    data.WriteString("test");
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    int ret = screenSessionManagerClientStub_->HandleScreenCaptureNotify(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnExtendDisplayNodeChange
 * @tc.desc: HandleOnExtendDisplayNodeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnExtendDisplayNodeChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenId mainScreenId = 50;
    ScreenId extendScreenId = 51;
    data.WriteUint64(mainScreenId);
    data.WriteUint64(extendScreenId);
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleOnExtendDisplayNodeChange(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnDumperClientScreenSessions
 * @tc.desc: HandleOnDumperClientScreenSessions test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnDumperClientScreenSessions, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleOnDumperClientScreenSessions(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnBeforeScreenPropertyChange01
 * @tc.desc: HandleOnBeforeScreenPropertyChange test err
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnBeforeScreenPropertyChange01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t errFoldStatus = 123;
    data.WriteUint32(errFoldStatus);
    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleOnBeforeScreenPropertyChange(data, reply);
    EXPECT_EQ(ret, 22);
}

/**
 * @tc.name: HandleOnBeforeScreenPropertyChange02
 * @tc.desc: HandleOnBeforeScreenPropertyChange test normal
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnBeforeScreenPropertyChange02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    FoldStatus foldStatus = FoldStatus::FOLDED;
    data.WriteUint32(static_cast<uint32_t>(foldStatus));
    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleOnBeforeScreenPropertyChange(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleSyncScreenCombination
 * @tc.desc: HandleSyncScreenCombination test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleSyncScreenCombination, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenId mainScreenId = 50;
    ScreenId extendScreenId = 51;
    ScreenCombination extendCombination = ScreenCombination::SCREEN_ALONE;
    data.WriteUint64(mainScreenId);
    data.WriteUint64(extendScreenId);
    data.WriteUint32(static_cast<u_int32_t>(extendCombination));
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleSyncScreenCombination(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnScreenModeChanged
 * @tc.desc: HandleOnScreenModeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnScreenModeChanged, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());

    ScreenModeChangeEvent screenModeChangeEvent = ScreenModeChangeEvent::UNKNOWN;
    data.WriteUint32(static_cast<u_int32_t>(screenModeChangeEvent));
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleOnScreenModeChanged(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleOnAnimationFinish
 * @tc.desc: HandleOnAnimationFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleOnAnimationFinish, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    ASSERT_TRUE(screenSessionManagerClientStub_ != nullptr);
    auto ret = screenSessionManagerClientStub_->HandleOnAnimationFinish(data, reply);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: HandleSetInternalClipToBounds
 * @tc.desc: HandleSetInternalClipToBounds test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientStubTest, HandleSetInternalClipToBounds, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(ScreenSessionManagerClientStub::GetDescriptor());
    data.WriteUint64(0);
    data.WriteBool(true);
    int ret = screenSessionManagerClientStub_->HandleSetInternalClipToBounds(data, reply);
    EXPECT_EQ(ret, 0);
}
} // namespace Rosen
} // namespace OHOS
