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

#include "float_window_error_msg.h"

#include <gtest/gtest.h>
#include <string>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class FloatWindowErrorMsgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FloatWindowErrorMsgTest::SetUpTestCase() {}
void FloatWindowErrorMsgTest::TearDownTestCase() {}
void FloatWindowErrorMsgTest::SetUp() {}
void FloatWindowErrorMsgTest::TearDown() {}

namespace {

HWTEST_F(FloatWindowErrorMsgTest, GlobalDefault_DeviceNotSupport, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "startPiP", WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][startPiP]"));
    EXPECT_NE(std::string::npos, msg.find("Capability not supported"));
}

HWTEST_F(FloatWindowErrorMsgTest, GlobalDefault_NoPermission, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::FLOAT_VIEW,
        "start", WmErrorCode::WM_ERROR_NO_PERMISSION);
    EXPECT_NE(std::string::npos, msg.find("[FloatView][start]"));
    EXPECT_NE(std::string::npos, msg.find("Permission verification failed"));
}

HWTEST_F(FloatWindowErrorMsgTest, GlobalDefault_SystemAbnormally, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "setWindowSize", WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][setWindowSize]"));
    EXPECT_NE(std::string::npos, msg.find("window manager service works abnormally"));
}

HWTEST_F(FloatWindowErrorMsgTest, ModuleDefault_PiP_InternalError, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "updateContentSize", WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR);
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][updateContentSize]"));
    EXPECT_NE(std::string::npos, msg.find("PiP internal error"));
}

HWTEST_F(FloatWindowErrorMsgTest, ModuleDefault_PiP_StateAbnormally, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "startPiP", WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY);
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][startPiP]"));
    EXPECT_NE(std::string::npos, msg.find("PiP window state is abnormal"));
}

HWTEST_F(FloatWindowErrorMsgTest, ModuleDefault_FloatingBall_StateAbnormally, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::FLOATING_BALL,
        "stopFloatingBall", WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY);
    EXPECT_NE(std::string::npos, msg.find("[FBWindow][stopFloatingBall]"));
    EXPECT_NE(std::string::npos, msg.find("floating ball window state is abnormal"));
}

HWTEST_F(FloatWindowErrorMsgTest, ModuleDefault_FloatView_StateAbnormally, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::FLOAT_VIEW,
        "setFloatViewVisibilityInApp", WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    EXPECT_NE(std::string::npos, msg.find("[FloatView][setFloatViewVisibilityInApp]"));
    EXPECT_NE(std::string::npos, msg.find("window state is abnormal"));
}

HWTEST_F(FloatWindowErrorMsgTest, CustomMessage_WithCause, TestSize.Level1)
{
    std::string customMsg = "The PiP controller has been destroyed.";
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP, "startPiP",
        WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY, customMsg);
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][startPiP]"));
    EXPECT_NE(std::string::npos, msg.find(customMsg));
}

HWTEST_F(FloatWindowErrorMsgTest, CustomMessage_Empty, TestSize.Level1)
{
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "startPiP", WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY, "");
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][startPiP]"));
    EXPECT_NE(std::string::npos, msg.find("PiP window state is abnormal"));
}

HWTEST_F(FloatWindowErrorMsgTest, UnknownError_Fallback, TestSize.Level1)
{
    constexpr int32_t unknownError = -99999;
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "startPiP", static_cast<WmErrorCode>(unknownError));
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][startPiP]"));
}

HWTEST_F(FloatWindowErrorMsgTest, CustomMessage_UnknownError, TestSize.Level1)
{
    std::string customMsg = "The PiP controller has been destroyed.";
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::PIP,
        "startPiP", WmErrorCode::WM_ERROR_FORBID_SUBWINDOW, customMsg);
    EXPECT_NE(std::string::npos, msg.find("[PiPWindow][startPiP]"));
    EXPECT_NE(std::string::npos, msg.find(customMsg));
}

HWTEST_F(FloatWindowErrorMsgTest, UnknownModule, TestSize.Level1)
{
    constexpr int32_t unknownError = -99999;
    std::string msg = FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule::END,
        "startPiP", static_cast<WmErrorCode>(unknownError));
    EXPECT_NE(std::string::npos, msg.find("[][startPiP]"));
}

} // namespace
} // namespace Rosen
} // namespace OHOS