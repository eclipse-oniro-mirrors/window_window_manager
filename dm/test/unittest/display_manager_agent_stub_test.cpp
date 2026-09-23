/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#include "display_manager_agent_stub.h"
#include "display_manager_agent_default.h"
#include "marshalling_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class DisplayManagerAgentStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<DisplayManagerAgentStub> stub_;
};

void DisplayManagerAgentStubTest::SetUpTestCase()
{
}

void DisplayManagerAgentStubTest::TearDownTestCase()
{
}

void DisplayManagerAgentStubTest::SetUp()
{
    stub_ = new DisplayManagerAgentDefault();
}

void DisplayManagerAgentStubTest::TearDown()
{
}

namespace {
    /**
     * @tc.name: OnRemoteRequest01
     * @tc.desc: TRANS_ID_ON_DISPLAY_CONNECT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest01, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        sptr<DisplayInfo> displayInfo;
        data.WriteParcelable(displayInfo.GetRefPtr());
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_DISPLAY_CONNECT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest02
     * @tc.desc: TRANS_ID_ON_DISPLAY_DISCONNECT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest02, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId displayId = 0;
        data.WriteUint64(displayId);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_DISPLAY_DISCONNECT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest03
     * @tc.desc: TRANS_ID_NOTIFY_DISPLAY_POWER_EVENT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest03, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId displayId = 0;
        data.WriteUint64(displayId);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_NOTIFY_DISPLAY_POWER_EVENT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest04
     * @tc.desc: TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest04, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId displayId = 0;
        data.WriteUint64(displayId);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest05
     * @tc.desc: TRANS_ID_ON_SCREEN_CONNECT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest05, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_SCREEN_CONNECT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest06
     * @tc.desc: TRANS_ID_ON_SCREEN_DISCONNECT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest06, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        ScreenId screenId = 0;
        data.WriteUint64(screenId);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_SCREEN_DISCONNECT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest07
     * @tc.desc: TRANS_ID_ON_SCREEN_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest07, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        ScreenChangeEvent event = ScreenChangeEvent::CHANGE_MODE;
        data.WriteUint32(static_cast<uint32_t>(event));
        sptr<ScreenInfo> screenInfo;
        data.WriteParcelable(screenInfo.GetRefPtr());
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_SCREEN_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest08
     * @tc.desc: TRANS_ID_ON_SCREENGROUP_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest08, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        std::string trigger = "trigger";
        data.WriteString(trigger);
        ScreenGroupChangeEvent event = ScreenGroupChangeEvent::ADD_TO_GROUP;
        data.WriteUint32(static_cast<uint32_t>(event));
        sptr<ScreenInfo> info = new ScreenInfo();
        std::vector<sptr<ScreenInfo>> infos;
        infos.emplace_back(info);
        MarshallingHelper::MarshallingVectorParcelableObj<ScreenInfo>(data, infos);

        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_SCREENGROUP_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest09
     * @tc.desc: TRANS_ID_ON_DISPLAY_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest09, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayChangeEvent event = DisplayChangeEvent::DISPLAY_SIZE_CHANGED;
        data.WriteUint32(static_cast<uint32_t>(event));
        sptr<DisplayInfo> displayInfo;
        data.WriteParcelable(displayInfo.GetRefPtr());
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_DISPLAY_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest10
     * @tc.desc: TRANS_ID_ON_SCREEN_SHOT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest10, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_SCREEN_SHOT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest11
     * @tc.desc: TRANS_ID_ON_PRIVATE_WINDOW
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest11, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        data.WriteBool(true);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_PRIVATE_WINDOW);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest12
     * @tc.desc: TRANS_ID_ON_PRIVATE_WINDOW_LIST
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest12, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId displayId = 0;
        data.WriteUint64(displayId);
        std::vector<std::string> privacyWindowList{"win0", "win1"};
        data.WriteStringVector(privacyWindowList);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_PRIVATE_WINDOW_LIST);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest13
     * @tc.desc: TRANS_ID_ON_FOLD_STATUS_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest13, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        uint32_t foldStatus = 0;
        data.WriteUint32(foldStatus);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_FOLD_STATUS_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest14
     * @tc.desc: TRANS_ID_ON_DISPLAY_CHANGE_INFO_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest14, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayInfo displayInfoSrc;
        displayInfoSrc.SetDisplayId(1);
        displayInfoSrc.Marshalling(data);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_DISPLAY_CHANGE_INFO_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest15
     * @tc.desc: TRANS_ID_ON_DISPLAY_MODE_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest15, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        uint32_t displayMode = 0;
        data.WriteUint32(displayMode);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_DISPLAY_MODE_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest16
     * @tc.desc: TRANS_ID_ON_AVAILABLE_AREA_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest16, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        data.WriteInt32(0);
        data.WriteInt32(0);
        data.WriteUint32(1);
        data.WriteUint32(1);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_AVAILABLE_AREA_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: OnRemoteRequest17
     * @tc.desc: TRANS_ID_ON_BRIGHTNESS_INFO_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest17, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        data.WriteInt32(0);
        data.WriteInt32(0);
        data.WriteUint32(1);
        data.WriteUint32(1);
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_BRIGHTNESS_INFO_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, -1);
    }

    /**
     * @tc.name: OnRemoteRequest18
     * @tc.desc: TRANS_ID_ON_RECORDING_DISPLAY_CHANGED
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, OnRemoteRequest18, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        data.WriteUInt64Vector({0});
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_ON_RECORDING_DISPLAY_CHANGED);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(res, -1);
    }

    /**
     * @tc.name: ProcPrivateWindowList
     * @tc.desc: ProcPrivateWindowList test
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, ProcPrivateWindowList, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId displayId = 0;
        data.WriteUint64(displayId);
        std::vector<std::string> privacyWindowList{"win0", "win1"};
        data.WriteStringVector(privacyWindowList);
        int res = stub_->ProcPrivateWindowList(data);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: NotifyDisplayStateChangedById
     * @tc.desc: TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED_BY_ID
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, NotifyDisplayStateChangedById, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId id = 0;
        DisplayState state = DisplayState::OFF;
        data.WriteUint64(static_cast<uint64_t>(id));
        data.WriteUint32(static_cast<uint32_t>(state));
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED_BY_ID);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: NotifyDisplayStateChangedById_InvalidToken
     * @tc.desc: TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED_BY_ID with invalid token
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, NotifyDisplayStateChangedById_InvalidToken, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(u"InvalidDescriptor");
        DisplayId id = 0;
        DisplayState state = DisplayState::OFF;
        data.WriteUint64(static_cast<uint64_t>(id));
        data.WriteUint32(static_cast<uint32_t>(state));
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED_BY_ID);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, -1);
    }

    /**
     * @tc.name: NotifySpecifiedDisplayPowerEvent_Stub
     * @tc.desc: TRANS_ID_NOTIFY_SPECIFIED_DISPLAY_POWER_EVENT
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, NotifySpecifiedDisplayPowerEvent_Stub, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(DisplayManagerAgentStub::GetDescriptor());
        DisplayId displayId = 0;
        DisplayPowerEvent event = DisplayPowerEvent::WAKE_UP;
        EventStatus status = EventStatus::BEGIN;
        data.WriteUint64(static_cast<uint64_t>(displayId));
        data.WriteUint32(static_cast<uint32_t>(event));
        data.WriteUint32(static_cast<uint32_t>(status));
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_NOTIFY_SPECIFIED_DISPLAY_POWER_EVENT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }

    /**
     * @tc.name: NotifySpecifiedDisplayPowerEvent_Stub_InvalidToken
     * @tc.desc: TRANS_ID_NOTIFY_SPECIFIED_DISPLAY_POWER_EVENT with invalid token
     * @tc.type: FUNC
     */
    HWTEST_F(DisplayManagerAgentStubTest, NotifySpecifiedDisplayPowerEvent_Stub_InvalidToken, TestSize.Level1)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInterfaceToken(u"InvalidDescriptor");
        DisplayId displayId = 0;
        DisplayPowerEvent event = DisplayPowerEvent::WAKE_UP;
        EventStatus status = EventStatus::BEGIN;
        data.WriteUint64(static_cast<uint64_t>(displayId));
        data.WriteUint32(static_cast<uint32_t>(event));
        data.WriteUint32(static_cast<uint32_t>(status));
        uint32_t code = static_cast<uint32_t>(IDisplayManagerAgent::TRANS_ID_NOTIFY_SPECIFIED_DISPLAY_POWER_EVENT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, -1);
    }
}
} // namespace Rosen
} // namespace OHOS