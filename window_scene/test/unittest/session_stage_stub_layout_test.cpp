/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "session/container/include/zidl/session_stage_stub.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ipc_types.h>
#include <iremote_stub.h>
#include <message_option.h>
#include <message_parcel.h>

#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "window_manager.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "ws_common.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionStageStubLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SessionStageStub> sessionStageStub_ = sptr<SessionStageMocker>::MakeSptr();
};

void SessionStageStubLayoutTest::SetUpTestCase() {}

void SessionStageStubLayoutTest::TearDownTestCase() {}

void SessionStageStubLayoutTest::SetUp() {}

void SessionStageStubLayoutTest::TearDown() {}

namespace {

/**
 * @tc.name: HandleUpdateAttachedWindowLimits01
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with valid data
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 3001;
    data.WriteInt32(sourcePersistentId);

    WindowLimits attachedLimits = { 200, 1000, 300, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    attachedLimits.Marshalling(data);

    data.WriteBool(true);  // isIntersectedHeightLimit
    data.WriteBool(true);  // isIntersectedWidthLimit

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits02
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with read sourceId failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    // Don't write sourcePersistentId - will fail to read

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits03
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits03, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 3003;
    data.WriteInt32(sourcePersistentId);

    WindowLimits attachedLimits = { 50, 500, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    attachedLimits.Marshalling(data);

    data.WriteBool(false);  // isIntersectedHeightLimit
    data.WriteBool(true);   // isIntersectedWidthLimit

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits04
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with height only
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits04, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 3004;
    data.WriteInt32(sourcePersistentId);

    WindowLimits attachedLimits = { 150, 900, 250, 1800, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    attachedLimits.Marshalling(data);

    data.WriteBool(true);   // isIntersectedHeightLimit
    data.WriteBool(false);  // isIntersectedWidthLimit

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleRemoveAttachedWindowLimits01
 * @tc.desc: Test HandleRemoveAttachedWindowLimits with valid sourceId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleRemoveAttachedWindowLimits01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 4001;
    data.WriteInt32(sourcePersistentId);

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleRemoveAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleRemoveAttachedWindowLimits02
 * @tc.desc: Test HandleRemoveAttachedWindowLimits with read sourceId failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleRemoveAttachedWindowLimits02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    // Don't write sourcePersistentId - will fail to read

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleRemoveAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits05
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with Unmarshalling failed (null WindowLimits)
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits05, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    // Write sourcePersistentId but don't write WindowLimits
    int32_t sourcePersistentId = 3005;
    data.WriteInt32(sourcePersistentId);
    // WindowLimits::Unmarshalling will fail and return nullptr

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits06
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with read isIntersectedHeightLimit failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits06, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 3006;
    data.WriteInt32(sourcePersistentId);

    WindowLimits attachedLimits = { 200, 1000, 300, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    attachedLimits.Marshalling(data);

    // Don't write isIntersectedHeightLimit - will fail to read

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits07
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with read isIntersectedWidthLimit failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits07, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 3007;
    data.WriteInt32(sourcePersistentId);

    WindowLimits attachedLimits = { 200, 1000, 300, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    attachedLimits.Marshalling(data);

    data.WriteBool(true);  // isIntersectedHeightLimit
    // Don't write isIntersectedWidthLimit - will fail to read

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleUpdateAttachedWindowLimits08
 * @tc.desc: Test HandleUpdateAttachedWindowLimits with invalid pixelUnit value
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleUpdateAttachedWindowLimits08, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 3008;
    data.WriteInt32(sourcePersistentId);

    // Write WindowLimits data manually with invalid pixelUnit value
    data.WriteUint32(200);  // maxWidth_
    data.WriteUint32(1000); // maxHeight_
    data.WriteUint32(300);  // minWidth_
    data.WriteUint32(2000); // minHeight_
    data.WriteFloat(0.0f);  // maxRatio_
    data.WriteFloat(0.0f);  // minRatio_
    data.WriteFloat(0.0f);  // vpRatio_
    data.WriteUint32(999);  // Invalid pixelUnit (valid values are 0=PX, 1=VP)

    // WindowLimits::Unmarshalling will fail due to invalid pixelUnit
    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateAttachedWindowLimits(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild01
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with valid data
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    // Write limits list: 2 entries
    data.WriteUint32(2);
    // Entry 1: parent limits
    data.WriteInt32(100);  // sourceId
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    limits1.Marshalling(data);
    // Entry 2: sub window limits
    data.WriteInt32(200);  // sourceId
    WindowLimits limits2 = { 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    limits2.Marshalling(data);

    // Write options list: 2 entries
    data.WriteUint32(2);
    data.WriteInt32(100);  // sourceId
    data.WriteBool(true);  // isIntersectedHeightLimit
    data.WriteBool(true);  // isIntersectedWidthLimit
    data.WriteInt32(200);  // sourceId
    data.WriteBool(true);  // isIntersectedHeightLimit
    data.WriteBool(false); // isIntersectedWidthLimit

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild02
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with empty lists
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(0);  // empty limits list
    data.WriteUint32(0);  // empty options list

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild03
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with read limitsCount failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild03, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    // Don't write anything - will fail to read limitsCount

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild04
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with read sourceId failed in limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild04, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);  // 1 entry in limits list
    // Don't write sourceId - will fail

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild05
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with WindowLimits Unmarshalling failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild05, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);  // 1 entry in limits list
    data.WriteInt32(300); // sourceId
    // Don't write WindowLimits data - Unmarshalling will return nullptr

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild06
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with read optionsCount failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild06, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    // Write valid limits list
    data.WriteUint32(1);
    data.WriteInt32(400);
    WindowLimits limits = { 1600, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    limits.Marshalling(data);
    // Don't write optionsCount - will fail

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild07
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with read opt sourceId failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild07, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    // Write valid limits list
    data.WriteUint32(1);
    data.WriteInt32(500);
    WindowLimits limits = { 1700, 900, 120, 220, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    limits.Marshalling(data);

    // Write options list with missing sourceId
    data.WriteUint32(1);
    // Don't write sourceId

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild08
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with read isIntersectedHeightLimit failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild08, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    // Write valid limits list
    data.WriteUint32(1);
    data.WriteInt32(600);
    WindowLimits limits = { 1800, 950, 130, 230, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    limits.Marshalling(data);

    // Write options list with missing heightLimit
    data.WriteUint32(1);
    data.WriteInt32(600);
    // Don't write isIntersectedHeightLimit

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleSyncAllAttachedLimitsToChild09
 * @tc.desc: Test HandleSyncAllAttachedLimitsToChild with read isIntersectedWidthLimit failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSyncAllAttachedLimitsToChild09, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    // Write valid limits list
    data.WriteUint32(1);
    data.WriteInt32(700);
    WindowLimits limits = { 1900, 980, 140, 240, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    limits.Marshalling(data);

    // Write options list with missing widthLimit
    data.WriteUint32(1);
    data.WriteInt32(700);
    data.WriteBool(true); // isIntersectedHeightLimit
    // Don't write isIntersectedWidthLimit

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSyncAllAttachedLimitsToChild(data, reply));
}

/**
 * @tc.name: HandleNotifyRebindAttachAfterParentChange01
 * @tc.desc: Test HandleNotifyRebindAttachAfterParentChange with valid newParentWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleNotifyRebindAttachAfterParentChange01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    int32_t newParentWindowId = 100;
    data.WriteInt32(newParentWindowId);

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleNotifyRebindAttachAfterParentChange(data, reply));
}

/**
 * @tc.name: HandleNotifyRebindAttachAfterParentChange02
 * @tc.desc: Test HandleNotifyRebindAttachAfterParentChange with read newParentWindowId failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleNotifyRebindAttachAfterParentChange02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    // Don't write newParentWindowId - will fail to read

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleNotifyRebindAttachAfterParentChange(data, reply));
}

/**
 * @tc.name: HandleSetDragActivated01
 * @tc.desc: Test HandleSetDragActivated with valid bitmap
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSetDragActivated01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;

    uint32_t bitmap = 0xFFFFFFFF;
    data.WriteUint32(bitmap);

    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleSetDragActivated(data, reply));
}

/**
 * @tc.name: HandleSetDragActivated02
 * @tc.desc: Test HandleSetDragActivated with ReadUint32 failed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubLayoutTest, HandleSetDragActivated02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    // Don't write bitmap - will fail to read

    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleSetDragActivated(data, reply));
}

} // namespace
} // namespace Rosen
} // namespace OHOS
