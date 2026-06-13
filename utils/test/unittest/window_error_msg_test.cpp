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

#include "window_error_msg.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowErrorMsgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowErrorMsgTest::SetUpTestCase() {}

void WindowErrorMsgTest::TearDownTestCase() {}

void WindowErrorMsgTest::SetUp() {}

void WindowErrorMsgTest::TearDown() {}

namespace {
/**
 * @tc.name: GetWmErrorMsg
 * @tc.desc: Test native WMError message mapping.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, GetWmErrorMsg, TestSize.Level1)
{
    EXPECT_EQ("ok", WindowErrorMsg::GetErrorMsg(WMError::WM_OK));
    EXPECT_EQ("invalid param", WindowErrorMsg::GetErrorMsg(WMError::WM_ERROR_INVALID_PARAM));
    EXPECT_EQ("ipc failed", WindowErrorMsg::GetErrorMsg(WMError::WM_ERROR_IPC_FAILED));
}

/**
 * @tc.name: GetWmErrorCodeMsg
 * @tc.desc: Test API-facing WmErrorCode short and full message mapping.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, GetWmErrorCodeMsg, TestSize.Level1)
{
    EXPECT_EQ("Ok.", WindowErrorMsg::GetShortErrorMsg(WmErrorCode::WM_OK));
    EXPECT_EQ("Ok.", WindowErrorMsg::GetFullErrorMsg(WmErrorCode::WM_OK));
    EXPECT_EQ("Parameter error.", WindowErrorMsg::GetShortErrorMsg(WmErrorCode::WM_ERROR_INVALID_PARAM));
    EXPECT_NE(std::string::npos,
        WindowErrorMsg::GetFullErrorMsg(WmErrorCode::WM_ERROR_INVALID_PARAM).find("Possible causes"));
}

/**
 * @tc.name: BuildWmErrorMsg
 * @tc.desc: Test WmErrorCode message building with default and custom messages.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, BuildWmErrorMsg, TestSize.Level1)
{
    EXPECT_NE(std::string::npos,
        WindowErrorMsg::BuildErrorMsg(WmErrorCode::WM_ERROR_INVALID_PARAM, "").find("Possible causes"));
    EXPECT_EQ("Parameter error.width is invalid",
        WindowErrorMsg::BuildErrorMsg(WmErrorCode::WM_ERROR_INVALID_PARAM, "width is invalid"));
}

/**
 * @tc.name: GetDmErrorMsg
 * @tc.desc: Test native DMError message mapping.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, GetDmErrorMsg, TestSize.Level1)
{
    EXPECT_EQ("ok", WindowErrorMsg::GetErrorMsg(DMError::DM_OK));
    EXPECT_EQ("invalid param", WindowErrorMsg::GetErrorMsg(DMError::DM_ERROR_INVALID_PARAM));
    EXPECT_EQ("ipc failed", WindowErrorMsg::GetErrorMsg(DMError::DM_ERROR_IPC_FAILED));
}

/**
 * @tc.name: GetDmErrorCodeMsg
 * @tc.desc: Test API-facing DmErrorCode short and full message mapping.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, GetDmErrorCodeMsg, TestSize.Level1)
{
    EXPECT_EQ("Ok.", WindowErrorMsg::GetShortErrorMsg(DmErrorCode::DM_OK));
    EXPECT_EQ("Ok.", WindowErrorMsg::GetFullErrorMsg(DmErrorCode::DM_OK));
    EXPECT_EQ("Parameter error.", WindowErrorMsg::GetShortErrorMsg(DmErrorCode::DM_ERROR_INVALID_PARAM));
    EXPECT_NE(std::string::npos,
        WindowErrorMsg::GetFullErrorMsg(DmErrorCode::DM_ERROR_INVALID_PARAM).find("Possible causes"));
}

/**
 * @tc.name: BuildDmErrorMsg
 * @tc.desc: Test DmErrorCode message building with default and custom messages.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, BuildDmErrorMsg, TestSize.Level1)
{
    EXPECT_NE(std::string::npos,
        WindowErrorMsg::BuildErrorMsg(DmErrorCode::DM_ERROR_INVALID_PARAM, "").find("Possible causes"));
    EXPECT_EQ("Parameter error.displayId is invalid",
        WindowErrorMsg::BuildErrorMsg(DmErrorCode::DM_ERROR_INVALID_PARAM, "displayId is invalid"));
}

/**
 * @tc.name: UnknownErrorMsg
 * @tc.desc: Test unknown error values return empty messages.
 * @tc.type: FUNC
 */
HWTEST_F(WindowErrorMsgTest, UnknownErrorMsg, TestSize.Level1)
{
    constexpr int32_t UNKNOWN_ERROR = -999;

    EXPECT_EQ("", WindowErrorMsg::GetErrorMsg(static_cast<WMError>(UNKNOWN_ERROR)));
    EXPECT_EQ("", WindowErrorMsg::GetShortErrorMsg(static_cast<WmErrorCode>(UNKNOWN_ERROR)));
    EXPECT_EQ("", WindowErrorMsg::GetFullErrorMsg(static_cast<WmErrorCode>(UNKNOWN_ERROR)));
    EXPECT_EQ("", WindowErrorMsg::BuildErrorMsg(static_cast<WmErrorCode>(UNKNOWN_ERROR), "custom message"));

    EXPECT_EQ("", WindowErrorMsg::GetErrorMsg(static_cast<DMError>(UNKNOWN_ERROR)));
    EXPECT_EQ("", WindowErrorMsg::GetShortErrorMsg(static_cast<DmErrorCode>(UNKNOWN_ERROR)));
    EXPECT_EQ("", WindowErrorMsg::GetFullErrorMsg(static_cast<DmErrorCode>(UNKNOWN_ERROR)));
    EXPECT_EQ("", WindowErrorMsg::BuildErrorMsg(static_cast<DmErrorCode>(UNKNOWN_ERROR), "custom message"));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
