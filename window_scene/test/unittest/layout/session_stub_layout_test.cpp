/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <ipc_types.h>
#include <pointer_event.h>

#include "ability_start_setting.h"
#include "iremote_object_mocker.h"
#include "mock/mock_session_stub.h"
#include "parcel/accessibility_event_info_parcel.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "want.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
}

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}
class SessionStubLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SessionStub> session_ = nullptr;
};

void SessionStubLayoutTest::SetUpTestCase() {}

void SessionStubLayoutTest::TearDownTestCase() {}

void SessionStubLayoutTest::SetUp()
{
    session_ = sptr<SessionStubMocker>::MakeSptr();
}

void SessionStubLayoutTest::TearDown()
{
    session_ = nullptr;
}

namespace {
/**
 * @tc.name: HandleUpdateRectChangeListenerRegistered008
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubLayoutTest, HandleUpdateRectChangeListenerRegistered008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto res = session_->HandleUpdateRectChangeListenerRegistered(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUpdateSessionRect
 * @tc.desc: test for HandleUpdateSessionRect
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubLayoutTest, HandleUpdateSessionRect, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(10);
    data.WriteUint32(20);
    data.WriteUint32(30);
    data.WriteUint32(40);
    data.WriteUint32(0);
    data.WriteBool(true);
    data.WriteBool(true);
    auto res = session_->HandleUpdateSessionRect(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUpdateClientRect01
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleUpdateClientRect01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = session_->HandleUpdateClientRect(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    data.WriteInt32(100);
    data.WriteInt32(100);
    data.WriteInt32(800);
    data.WriteInt32(800);
    res = session_->HandleUpdateClientRect(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetSystemEnableDrag_TestReadBool
 * @tc.desc: Check whether the enableDrag is read successfully.
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleSetSystemEnableDrag_TestReadBool, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = session_->HandleSetSystemEnableDrag(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);

    data.WriteBool(true);
    res = session_->HandleSetSystemEnableDrag(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}
/**
 * @tc.name: HandleNotifyAttachedWindowsLimitsChanged01
 * @tc.desc: Test HandleNotifyAttachedWindowsLimitsChanged with valid data
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleNotifyAttachedWindowsLimitsChanged01, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    MessageParcel data;
    MessageParcel reply;

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    newLimits.Marshalling(data);

    auto res = session->HandleNotifyAttachedWindowsLimitsChanged(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleNotifyAttachedWindowsLimitsChanged02
 * @tc.desc: Test HandleNotifyAttachedWindowsLimitsChanged with read limits failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleNotifyAttachedWindowsLimitsChanged02, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    MessageParcel data;
    MessageParcel reply;
    // Don't write limits - will fail to read

    auto res = session->HandleNotifyAttachedWindowsLimitsChanged(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleNotifyAttachedWindowsLimitsChanged03
 * @tc.desc: Test HandleNotifyAttachedWindowsLimitsChanged with height only
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleNotifyAttachedWindowsLimitsChanged03, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    MessageParcel data;
    MessageParcel reply;

    WindowLimits newLimits = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    newLimits.Marshalling(data);

    auto res = session->HandleNotifyAttachedWindowsLimitsChanged(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleNotifyAttachedWindowsLimitsChanged04
 * @tc.desc: Test HandleNotifyAttachedWindowsLimitsChanged with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleNotifyAttachedWindowsLimitsChanged04, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    MessageParcel data;
    MessageParcel reply;

    WindowLimits newLimits = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    newLimits.Marshalling(data);

    auto res = session->HandleNotifyAttachedWindowsLimitsChanged(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleNotifyAttachedWindowsLimitsChanged05
 * @tc.desc: Test HandleNotifyAttachedWindowsLimitsChanged with invalid pixelUnit value
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLayoutTest, HandleNotifyAttachedWindowsLimitsChanged05, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    MessageParcel data;
    MessageParcel reply;

    // Write WindowLimits data manually with invalid pixelUnit value
    data.WriteUint32(1600); // maxWidth_
    data.WriteUint32(800);  // maxHeight_
    data.WriteUint32(100);  // minWidth_
    data.WriteUint32(200);  // minHeight_
    data.WriteFloat(0.0f);  // maxRatio_
    data.WriteFloat(0.0f);  // minRatio_
    data.WriteFloat(0.0f);  // vpRatio_
    data.WriteUint32(999);  // Invalid pixelUnit (valid values are 0=PX, 1=VP)

    // WindowLimits::Unmarshalling will fail due to invalid pixelUnit
    auto res = session->HandleNotifyAttachedWindowsLimitsChanged(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}
} // namespace
} // namespace Rosen
} // namespace OHOS