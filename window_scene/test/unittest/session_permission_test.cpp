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
#include <libxml/globals.h>
#include <libxml/xmlstring.h>
#include "window_scene_config.h"
#include "window_manager_hilog.h"
#include "common/include/session_permission.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionPermissionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SessionPermissionTest::SetUpTestCase() {}

void SessionPermissionTest::TearDownTestCase() {}

void SessionPermissionTest::SetUp() {}

void SessionPermissionTest::TearDown() {}
namespace {

/**
 * @tc.name: IsSystemServiceCalling
 * @tc.desc: test function : IsSystemServiceCalling
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsSystemServiceCalling, TestSize.Level1)
{
    bool needPrintLog = true;
    bool result = SessionPermission::IsSystemServiceCalling(needPrintLog);
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: IsSACalling
 * @tc.desc: test function : IsSACalling
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsSACalling, TestSize.Level1)
{
    bool result = SessionPermission::IsSACalling();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: VerifyCallingPermission
 * @tc.desc: test function : VerifyCallingPermission1
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, VerifyCallingPermission, TestSize.Level1)
{
    const std::string* permissionNode = new string;
    bool result = SessionPermission::VerifyCallingPermission(*permissionNode);
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: VerifyPermissionByCallerToken
 * @tc.desc: test function : VerifyPermissionByCallerToken
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, VerifyPermissionByCallerToken, TestSize.Level1)
{
    const uint32_t callerToken = 1000;
    const std::string* permissionNode = new string;
    bool result = SessionPermission::VerifyPermissionByCallerToken(callerToken, *permissionNode);
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: IsSameBundleNameAsCalling
 * @tc.desc: test function : IsSameBundleNameAsCalling1
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsSameBundleNameAsCalling, TestSize.Level1)
{
    const std::string* bundleName = new string;
    bool result = SessionPermission::IsSameBundleNameAsCalling(*bundleName);
    ASSERT_EQ(false, result);

    const std::string* bundleName2 = new string("test");
    bool result2 = SessionPermission::IsSameBundleNameAsCalling(*bundleName2);
    ASSERT_EQ(false, result2);
}

/**
 * @tc.name: IsSameAppAsCalling
 * @tc.desc: test function : IsSameAppAsCalling
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsSameAppAsCalling, TestSize.Level1)
{
    const std::string appIdentifier = "123";
    const std::string bundleName = "";
    bool result = SessionPermission::IsSameAppAsCalling(bundleName, appIdentifier);
    ASSERT_EQ(false, result);

    const std::string bundleName2 = "test";
    bool result2 = SessionPermission::IsSameAppAsCalling(bundleName2, appIdentifier);
    ASSERT_EQ(false, result2);
}

/**
 * @tc.name: VerifySessionPermission
 * @tc.desc: test function : VerifySessionPermission
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, VerifySessionPermission, TestSize.Level1)
{
    bool result = SessionPermission::VerifySessionPermission();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: JudgeCallerIsAllowedToUseSystemAPI
 * @tc.desc: test function : JudgeCallerIsAllowedToUseSystemAPI
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, JudgeCallerIsAllowedToUseSystemAPI, TestSize.Level1)
{
    bool result = SessionPermission::JudgeCallerIsAllowedToUseSystemAPI();
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: IsShellCall
 * @tc.desc: test function : IsShellCall
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsShellCall, TestSize.Level1)
{
    bool result = SessionPermission::IsShellCall();
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: IsStartByHdcd
 * @tc.desc: test function : IsStartByHdcd
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsStartByHdcd, TestSize.Level1)
{
    bool result = SessionPermission::IsStartByHdcd();
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: IsStartedByInputMethod
 * @tc.desc: test function : IsStartedByInputMethod
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsStartedByInputMethod, TestSize.Level1)
{
    bool result = SessionPermission::IsStartedByInputMethod();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: IsKeyboardCallingProcess
 * @tc.desc: test function : IsKeyboardCallingProcess
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsKeyboardCallingProcess, TestSize.Level1)
{
    bool result = SessionPermission::IsKeyboardCallingProcess(1, 1);
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: IsStartedByUIExtension
 * @tc.desc: test function : IsStartedByUIExtension
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsStartedByUIExtension, TestSize.Level1)
{
    bool result = SessionPermission::IsStartedByUIExtension();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: IsFoundationCall
 * @tc.desc: test function : IsFoundationCall
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, IsFoundationCall, TestSize.Level1)
{
    bool result = SessionPermission::IsFoundationCall();
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: PermissionConstantsTest
 * @tc.desc: test permission constants
 * @tc.type: FUNC
 */
HWTEST_F(SessionPermissionTest, PermissionConstantsTest, TestSize.Level1)
{
    ASSERT_STREQ(PermissionConstants::PERMISSION_FLOAT_VIEW, "ohos.permission.FLOAT_VIEW");
    ASSERT_STREQ(PermissionConstants::PERMISSION_FLOATING_BALL, "ohos.permission.USE_FLOAT_BALL");
    ASSERT_STREQ(PermissionConstants::PERMISSION_FLOATING_BALL_AUTO_RESTORE,
        "ohos.permission.AUTO_RESTORE_MAIN_WINDOW");
}

} // namespace
} // namespace Rosen
} // namespace OHOS