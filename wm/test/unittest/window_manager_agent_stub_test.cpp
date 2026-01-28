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
#include "marshalling_helper.h"
#include "window_manager_agent.h"
#include "window_manager_agent_stub.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class WindowManagerAgentStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowManagerAgentStub> stub_;
};

void WindowManagerAgentStubTest::SetUpTestCase() {}

void WindowManagerAgentStubTest::TearDownTestCase() {}

void WindowManagerAgentStubTest::SetUp()
{
    stub_ = new WindowManagerAgent();
}

void WindowManagerAgentStubTest::TearDown() {}

namespace {
/**
 * @tc.name: OnRemoteRequest01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"error.GetDescriptor");

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_FOCUS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_TRANSACTION_FAILED));
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: test TRANS_ID_UPDATE_FOCUS
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    sptr<FocusChangeInfo> focusChangeInfo = new FocusChangeInfo();
    data.WriteParcelable(focusChangeInfo);
    data.WriteRemoteObject(focusChangeInfo->abilityToken_);
    data.WriteBool(false);

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_FOCUS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest03
 * @tc.desc: test TRANS_ID_UPDATE_FOCUS failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_FOCUS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest04
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    sptr<AccessibilityWindowInfo> info = new AccessibilityWindowInfo();
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    infos.emplace_back(info);
    MarshallingHelper::MarshallingVectorParcelableObj<AccessibilityWindowInfo>(data, infos);

    data.WriteUint32(static_cast<uint32_t>(WindowUpdateType::WINDOW_UPDATE_ADDED));

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_STATUS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest06
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_VISIBILITY success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest06, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    sptr<WindowVisibilityInfo> visibilityInfo = new WindowVisibilityInfo();
    std::vector<sptr<WindowVisibilityInfo>> visibilityInfos;
    visibilityInfos.emplace_back(visibilityInfo);
    data.WriteUint32(static_cast<uint32_t>(visibilityInfos.size()));
    for (auto& info : visibilityInfos) {
        data.WriteParcelable(info);
    }

    auto code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_VISIBILITY);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest08
 * @tc.desc: test TRANS_ID_UPDATE_SYSTEM_BAR_PROPS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest08, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    data.WriteUint64(0);

    SystemBarRegionTints tints;
    MarshallingHelper::MarshallingVectorObj<SystemBarRegionTint>(
        data, tints, [](Parcel& parcel, const SystemBarRegionTint& tint) {
            return parcel.WriteUint32(static_cast<uint32_t>(tint.type_)) && parcel.WriteBool(tint.prop_.enable_) &&
                   parcel.WriteUint32(tint.prop_.backgroundColor_) && parcel.WriteUint32(tint.prop_.contentColor_) &&
                   parcel.WriteInt32(tint.region_.posX_) && parcel.WriteInt32(tint.region_.posY_) &&
                   parcel.WriteInt32(tint.region_.width_) && parcel.WriteInt32(tint.region_.height_);
        });

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_SYSTEM_BAR_PROPS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest09
 * @tc.desc: test TRANS_ID_UPDATE_SYSTEM_BAR_PROPS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest09, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WATER_MARK_FLAG);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest10
 * @tc.desc: test TRANS_ID_UPDATE_SYSTEM_BAR_PROPS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest10, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteBool(true);
    uint32_t code =
        static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_GESTURE_NAVIGATION_ENABLED);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest11
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest11, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"error.GetDescriptor");

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_MODE_TYPE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_TRANSACTION_FAILED));
}

/**
 * @tc.name: OnRemoteRequest12
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_MODE_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest12, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteUint8(static_cast<uint8_t>(WindowModeType::WINDOW_MODE_FLOATING));
    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_MODE_TYPE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest13
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_MODE_TYPE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest13, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteUint8(static_cast<uint8_t>(WindowModeType::WINDOW_MODE_FLOATING));
    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_MODE_TYPE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest14, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    VisibleWindowNumInfo oneNum;
    oneNum.displayId = 0;
    oneNum.visibleWindowNum = 3;
    std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
    visibleWindowNumInfo.push_back(oneNum);
    MarshallingHelper::MarshallingVectorObj<VisibleWindowNumInfo>(
        data, visibleWindowNumInfo, [](Parcel& parcel, const VisibleWindowNumInfo& num) {
            return parcel.WriteUint32(num.displayId) && parcel.WriteUint32(num.visibleWindowNum);
        });

    uint32_t code =
        static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_VISIBLE_WINDOW_NUM);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest15
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_DRAWING_STATE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest15, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    uint32_t code =
        static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_DRAWING_STATE);
    ASSERT_NE(stub_, nullptr);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest16
 * @tc.desc: test TRANS_ID_UPDATE_CAMERA_FLOAT
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest16, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteUint8(static_cast<uint8_t>(1));
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_CAMERA_FLOAT);
    ASSERT_NE(stub_, nullptr);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest17
 * @tc.desc: test TRANS_ID_UPDATE_CAMERA_WINDOW_STATUS
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest17, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteUint8(static_cast<uint8_t>(1));
    data.WriteBool(true);
    uint32_t code =
        static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_CAMERA_WINDOW_STATUS);
    ASSERT_NE(stub_, nullptr);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest18
 * @tc.desc: test default
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest18, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteUint8(static_cast<uint8_t>(1));
    uint32_t code = static_cast<uint32_t>(12);
    ASSERT_NE(stub_, nullptr);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest19
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_STYLE_TYPE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest19, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    data.WriteUint8(static_cast<uint8_t>(WindowStyleType::WINDOW_STYLE_DEFAULT));
    uint32_t code =
        static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_STYLE_TYPE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest20
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_FOCUS error
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest20, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());

    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    data.WriteParcelable(focusChangeInfo);
    data.WriteRemoteObject(focusChangeInfo->abilityToken_);

    uint32_t code = static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_UPDATE_FOCUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest21
 * @tc.desc: test TRANS_ID_NOTIFY_WINDOW_PROPERTY_CHANGE success
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest21, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    uint32_t propertyDirtyFlags = 0;
    data.WriteUint32(propertyDirtyFlags);
    data.WriteUint32(0);

    uint32_t code =
        static_cast<uint32_t>(IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_PROPERTY_CHANGE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest22
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_DRAWING_STATE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest22, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_SYSTEM_BAR_PROPERTY_CHANGE);
    EXPECT_NE(stub_, nullptr);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 5);
}


/**
 * @tc.name: OnRemoteRequest23
 * @tc.desc: test TRANS_ID_NOTIFY_WINDOW_SUPPORT_ROTATION_CHANGE success
 */
HWTEST_F(WindowManagerAgentStubTest, OnRemoteRequest23, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    sptr<SupportRotationInfo> supportRotationInfo = sptr<SupportRotationInfo>::MakeSptr();
    data.WriteParcelable(supportRotationInfo);

    uint32_t code = static_cast<uint32_t>(
        IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_SUPPORT_ROTATION_CHANGE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: ReadWindowInfoList01
 * @tc.desc: test ReadWindowInfoList
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfoList01, TestSize.Level1)
{
    MessageParcel data;
    WindowInfoList windowInfoList;

    data.WriteUint32(1);
    EXPECT_FALSE(stub_->ReadWindowInfoList(data, windowInfoList));
    data.WriteUint32(1);
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_ID));
    EXPECT_FALSE(stub_->ReadWindowInfoList(data, windowInfoList));
    data.WriteUint32(0);
    EXPECT_TRUE(stub_->ReadWindowInfoList(data, windowInfoList));
}

/**
 * @tc.name: ReadWindowInfoList02
 * @tc.desc: test ReadWindowInfoList
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfoList02, TestSize.Level1)
{
    MessageParcel data;
    WindowInfoList windowInfoList;

    data.WriteUint32(100000);
    EXPECT_FALSE(stub_->ReadWindowInfoList(data, windowInfoList));
}

/**
 * @tc.name: ReadWindowInfo01
 * @tc.desc: test WINDOW_ID
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo01, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_ID));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_ID));
    data.WriteUint32(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo02
 * @tc.desc: test BUNDLE_NAME
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo02, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::BUNDLE_NAME));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::BUNDLE_NAME));
    data.WriteString("test");
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo03
 * @tc.desc: test ABILITY_NAME
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo03, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::ABILITY_NAME));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::ABILITY_NAME));
    data.WriteString("test");
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo04
 * @tc.desc: test APP_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo04, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::APP_INDEX));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::APP_INDEX));
    data.WriteInt32(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo05
 * @tc.desc: test VISIBILITY_STATE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo05, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::VISIBILITY_STATE));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::VISIBILITY_STATE));
    data.WriteUint32(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo06
 * @tc.desc: test WINDOW_MODE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo06, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_MODE));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_MODE));
    data.WriteUint32(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo07
 * @tc.desc: test DISPLAY_ID
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo07, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::DISPLAY_ID));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::DISPLAY_ID));
    data.WriteUint64(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo08
 * @tc.desc: test WINDOW_RECT
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo08, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_RECT));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_RECT));
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteUint32(0);
    data.WriteUint32(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo09
 * @tc.desc: test FLOATING_SCALE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo09, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::FLOATING_SCALE));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::FLOATING_SCALE));
    data.WriteFloat(0.f);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo10
 * @tc.desc: test MID_SCENE
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo10, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::MID_SCENE));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::MID_SCENE));
    data.WriteBool(true);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}

/**
 * @tc.name: ReadWindowInfo11
 * @tc.desc: test WINDOW_GLOBAL_RECT
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentStubTest, ReadWindowInfo11, TestSize.Level1)
{
    MessageParcel data;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;

    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_GLOBAL_RECT));
    EXPECT_FALSE(stub_->ReadWindowInfo(data, windowInfo));
    data.WriteInt32(static_cast<int32_t>(WindowInfoKey::WINDOW_GLOBAL_RECT));
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteUint32(0);
    data.WriteUint32(0);
    EXPECT_TRUE(stub_->ReadWindowInfo(data, windowInfo));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
