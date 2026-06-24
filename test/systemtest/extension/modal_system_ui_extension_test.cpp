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
#include "mock_message_parcel.h"
#include "modal_system_ui_extension.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ModalSystemUiExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ModalSystemUiExtensionTest::SetUpTestCase() {}

void ModalSystemUiExtensionTest::TearDownTestCase() {}

void ModalSystemUiExtensionTest::SetUp() {}

void ModalSystemUiExtensionTest::TearDown() {}

namespace {
/**
 * @tc.name: ModalSystemUiExtensionConnection01
 * @tc.desc: connect modal system ui_extension
 * @tc.type: FUNC
 */
HWTEST_F(ModalSystemUiExtensionTest, ModalSystemUiExtensionConnection01, TestSize.Level1)
{
    auto connection = new (std::nothrow) ModalSystemUiExtension();
    ASSERT_NE(connection, nullptr);
    OHOS::AAFwk::Want want;
    ASSERT_FALSE(connection->CreateModalUIExtension(want));
    delete connection;
}

/**
 * @tc.name: ToString
 * @tc.desc: ToString
 * @tc.type: FUNC
 */
HWTEST_F(ModalSystemUiExtensionTest, ToString, TestSize.Level1)
{
    AAFwk::WantParams wantParams;
    std::string ret = ModalSystemUiExtension::ToString(wantParams);
    ASSERT_EQ("{}", ret);
}

/**
 * @tc.name: DialogAbilityConnectionOnAbilityConnectDone
 * @tc.desc: DialogAbilityConnectionOnAbilityConnectDone
 * @tc.type: FUNC
 */
HWTEST_F(ModalSystemUiExtensionTest, DialogAbilityConnectionOnAbilityConnectDone, TestSize.Level1)
{
    AAFwk::Want want;
    auto connection = sptr<ModalSystemUiExtension::DialogAbilityConnection>::MakeSptr(want);
    ASSERT_NE(connection, nullptr);
    AppExecFwk::ElementName element;

    connection->OnAbilityConnectDone(element, nullptr, 0);

    auto remoteObject = sptr<MockIRemoteObject>::MakeSptr();
    connection->OnAbilityConnectDone(element, remoteObject, 0);

    remoteObject->sendRequestResult_ = 1;
    connection->OnAbilityConnectDone(element, remoteObject, 0);
    static constexpr uint32_t WAIT_TIME_MICROSECONDS = 5200000;
    usleep(WAIT_TIME_MICROSECONDS);
}

/**
 * @tc.name: DialogAbilityConnectionOnAbilityDisconnectDone
 * @tc.desc: DialogAbilityConnectionOnAbilityDisconnectDone
 * @tc.type: FUNC
 */
HWTEST_F(ModalSystemUiExtensionTest, DialogAbilityConnectionOnAbilityDisconnectDone, TestSize.Level1)
{
    AAFwk::Want want;
    auto connection = sptr<ModalSystemUiExtension::DialogAbilityConnection>::MakeSptr(want);
    ASSERT_NE(connection, nullptr);
    AppExecFwk::ElementName element;
    connection->OnAbilityDisconnectDone(element, 0);
}

/**
 * @tc.name: DialogAbilityConnectionSendWant
 * @tc.desc: DialogAbilityConnectionSendWant
 * @tc.type: FUNC
 */
HWTEST_F(ModalSystemUiExtensionTest, DialogAbilityConnectionSendWant, TestSize.Level1)
{
    AAFwk::Want want;
    auto connection = sptr<ModalSystemUiExtension::DialogAbilityConnection>::MakeSptr(want);
    auto remoteObject = sptr<MockIRemoteObject>::MakeSptr();

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_FALSE(connection->SendWant(remoteObject));
    MockMessageParcel::ClearAllErrorFlag();

    MockMessageParcel::SetWriteString16ErrorFlag(true);
    EXPECT_FALSE(connection->SendWant(remoteObject));
    MockMessageParcel::ClearAllErrorFlag();

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_FALSE(connection->SendWant(remoteObject));
    MockMessageParcel::ClearAllErrorFlag();

    EXPECT_TRUE(connection->SendWant(remoteObject));

    remoteObject->sendRequestResult_ = 1;
    EXPECT_FALSE(connection->SendWant(remoteObject));
}
} // namespace
} // namespace Rosen
} // namespace OHOS