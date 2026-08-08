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

#include "window_focus_error_msg_helper.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowFocusErrorMsgHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowFocusErrorMsgHelperTest::SetUpTestCase() {}

void WindowFocusErrorMsgHelperTest::TearDownTestCase() {}

void WindowFocusErrorMsgHelperTest::SetUp() {}

void WindowFocusErrorMsgHelperTest::TearDown() {}

namespace {
HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgBasic, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(0, msg.find(" "));
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
    EXPECT_NE(std::string::npos, msg.find("msg:"));
    EXPECT_NE(std::string::npos, msg.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgFormat, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_NULLPTR);

    EXPECT_EQ(0, msg.find(" "));

    size_t apiPos = msg.find("[window][setWindowFocusable]");
    size_t msgPos = msg.find("msg:");
    size_t specificPos = msg.find("The window is not created or destroyed.");

    EXPECT_NE(std::string::npos, apiPos);
    EXPECT_NE(std::string::npos, msgPos);
    EXPECT_NE(std::string::npos, specificPos);
    EXPECT_LT(apiPos, msgPos);
    EXPECT_LT(msgPos, specificPos);
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgGeneralOnly, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_INVALID_WINDOW);

    EXPECT_EQ(0, msg.find(" "));
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
    EXPECT_NE(std::string::npos, msg.find("msg:"));
    EXPECT_NE(std::string::npos, msg.find("not created or destroyed"));

    size_t apiPos = msg.find("[window][setWindowFocusable]");
    size_t msgPos = msg.find("msg:");
    size_t specificPos = msg.find("The window is not created or destroyed.");

    EXPECT_LT(apiPos, msgPos);
    EXPECT_LT(msgPos, specificPos);
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgGeneralOnlyNoDetail, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_IPC_FAILED);

    EXPECT_EQ(0, msg.find(" "));
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
    EXPECT_NE(std::string::npos, msg.find("msg:"));

    size_t apiPos = msg.find("[window][setWindowFocusable]");
    EXPECT_NE(std::string::npos, apiPos);

    EXPECT_EQ(std::string::npos, msg.find("The window is not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgWithCustomMsg, TestSize.Level1)
{
    std::string customMsg = "session creation failed due to memory limit";
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_NULLPTR,
        customMsg);

    EXPECT_EQ(0, msg.find(" "));
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
    EXPECT_NE(std::string::npos, msg.find("msg:"));
    EXPECT_NE(std::string::npos, msg.find(customMsg));
    EXPECT_EQ(std::string::npos, msg.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgGeneralWithCustom, TestSize.Level1)
{
    std::string customMsg = "window creation timeout";
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_IPC_FAILED,
        customMsg);

    EXPECT_EQ(0, msg.find(" "));
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
    EXPECT_NE(std::string::npos, msg.find("msg:"));
    EXPECT_NE(std::string::npos, msg.find(customMsg));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgFixedGeneralError, TestSize.Level1)
{
    std::string customMsg = "this should not appear";

    std::string msg801 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_DEVICE_NOT_SUPPORT,
        customMsg);
    EXPECT_NE(std::string::npos, msg801.find("Capability not supported"));
    EXPECT_EQ(std::string::npos, msg801.find(customMsg));

    std::string msg201 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_INVALID_PERMISSION,
        customMsg);
    EXPECT_NE(std::string::npos, msg201.find("Permission verification failed"));
    EXPECT_EQ(std::string::npos, msg201.find(customMsg));

    std::string msg202 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_NOT_SYSTEM_APP,
        customMsg);
    EXPECT_NE(std::string::npos, msg202.find("non-system application"));
    EXPECT_EQ(std::string::npos, msg202.find(customMsg));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgUnknownError, TestSize.Level1)
{
    constexpr int32_t unknownError = -999;
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        static_cast<WMError>(unknownError));
    EXPECT_NE(std::string::npos, msg.find("failed"));
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgUnknownWithCustom, TestSize.Level1)
{
    constexpr int32_t unknownError = -999;
    std::string customMsg = "custom error detail";
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        static_cast<WMError>(unknownError),
        customMsg);
    EXPECT_NE(std::string::npos, msg.find("setWindowFocusable"));
    EXPECT_NE(std::string::npos, msg.find(customMsg));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgEmptyCustomMsg, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_NULLPTR);
    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE,
        WMError::WM_ERROR_NULLPTR,
        "");
    EXPECT_EQ(msg1, msg2);
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgDifferentApiTypes, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_MODAL,
        WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("setSubWindowModal"));
    EXPECT_NE(std::string::npos, msg1.find("Only sub windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_TOPMOST,
        WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg2.find("setWindowTopmost"));
    EXPECT_NE(std::string::npos, msg2.find("Only main windows are supported"));

    EXPECT_NE(msg1, msg2);
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgUnknownApiType, TestSize.Level1)
{
    constexpr WindowFocusApiType unknownApi = static_cast<WindowFocusApiType>(999);
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        unknownApi,
        WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg.find("[window]"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetFocusableAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg1.find("not created or destroyed"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE, WMError::WM_ERROR_INVALID_OPERATION);
    EXPECT_NE(std::string::npos, msg3.find("not allowed to be focused"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetWindowFocusableAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_FOCUSABLE, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg1.find("not created or destroyed"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_FOCUSABLE, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_FOCUSABLE, WMError::WM_ERROR_INVALID_OPERATION);
    EXPECT_NE(std::string::npos, msg3.find("not allowed to be focused"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetSubWindowModalAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_MODAL, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("Only sub windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_MODAL, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_MODAL, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetTopmostAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_TOPMOST, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("Only main windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_TOPMOST, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_TOPMOST, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetWindowTopmostAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_TOPMOST, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("Only main windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_TOPMOST, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_TOPMOST, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgRaiseToAppTopAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_TO_APP_TOP, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("Only sub windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_TO_APP_TOP, WMError::WM_ERROR_INVALID_PARENT);
    EXPECT_NE(std::string::npos, msg2.find("parent window does not exist"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_TO_APP_TOP, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));

    std::string msg4 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_TO_APP_TOP, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg4.find("not shown"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetSubWindowZLevelAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_Z_LEVEL, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("Only non-modal sub windows"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_Z_LEVEL, WMError::WM_ERROR_INVALID_PARENT);
    EXPECT_NE(std::string::npos, msg2.find("parent window does not exist"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_Z_LEVEL, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgGetSubWindowZLevelAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::GET_SUB_WINDOW_Z_LEVEL, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("sub windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::GET_SUB_WINDOW_Z_LEVEL, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetWindowModalAllErrors, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_MODAL, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg.find("Only main windows are supported"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetRaiseByClickEnabledAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_RAISE_BY_CLICK_ENABLED, WMError::WM_ERROR_INVALID_PARENT);
    EXPECT_NE(std::string::npos, msg1.find("parent window does not exist"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_RAISE_BY_CLICK_ENABLED, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg2.find("Only sub windows are supported"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_RAISE_BY_CLICK_ENABLED, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));

    std::string msg4 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_RAISE_BY_CLICK_ENABLED, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg4.find("not shown"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetMainWindowRaiseByClickEnabledAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_MAIN_WINDOW_RAISE_BY_CLICK_ENABLED, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("Only main windows are supported"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_MAIN_WINDOW_RAISE_BY_CLICK_ENABLED, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_MAIN_WINDOW_RAISE_BY_CLICK_ENABLED, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg3.find("not shown"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgRaiseAboveTargetAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_ABOVE_TARGET, WMError::WM_ERROR_INVALID_PARENT);
    EXPECT_NE(std::string::npos, msg1.find("parent window does not exist"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_ABOVE_TARGET, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg2.find("Only sub windows are supported"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_ABOVE_TARGET, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg3.find("not created or destroyed"));

    std::string msg4 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_ABOVE_TARGET, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg4.find("or target window is not shown"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgRaiseMainWindowAboveTargetAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_MAIN_WINDOW_ABOVE_TARGET, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg1.find("target must not be modal or topmost"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_MAIN_WINDOW_ABOVE_TARGET, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetExclusivelyHighlightedAllErrors, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_EXCLUSIVELY_HIGHLIGHTED, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg.find("Only non-modal sub windows"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgSetWindowDelayRaiseEnabledAllErrors, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_WINDOW_DELAY_RAISE_ENABLED, WMError::WM_ERROR_INVALID_TYPE);
    EXPECT_NE(std::string::npos, msg.find("Only app windows are supported"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgShiftAppWindowFocusAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SHIFT_APP_WINDOW_FOCUS, WMError::WM_ERROR_INVALID_OPERATION);
    EXPECT_NE(std::string::npos, msg1.find("source window is not focused"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SHIFT_APP_WINDOW_FOCUS, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_NE(std::string::npos, msg2.find("not from the same process"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SHIFT_APP_WINDOW_FOCUS, WMError::WM_DO_NOTHING);
    EXPECT_NE(std::string::npos, msg3.find("already focused"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgGetTopWindowAllErrors, TestSize.Level1)
{
    std::string msg = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::GET_TOP_WINDOW, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg.find("Cannot find top window"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgIsFocusedAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::IS_FOCUSED, WMError::WM_ERROR_INVALID_WINDOW);
    EXPECT_NE(std::string::npos, msg1.find("not created or destroyed"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::IS_FOCUSED, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgIsWindowHighlightedAllErrors, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::IS_WINDOW_HIGHLIGHTED, WMError::WM_ERROR_INVALID_WINDOW);
    EXPECT_NE(std::string::npos, msg1.find("not created or destroyed"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::IS_WINDOW_HIGHLIGHTED, WMError::WM_ERROR_NULLPTR);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgInvalidWindowAndSession, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE, WMError::WM_ERROR_INVALID_WINDOW);
    EXPECT_NE(std::string::npos, msg1.find("not created or destroyed"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE, WMError::WM_ERROR_INVALID_SESSION);
    EXPECT_NE(std::string::npos, msg2.find("not created or destroyed"));
}

HWTEST_F(WindowFocusErrorMsgHelperTest, GetErrorMsgAllApiTypesNoMatch, TestSize.Level1)
{
    std::string msg1 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_FOCUSABLE, WMError::WM_ERROR_IPC_FAILED);
    EXPECT_NE(std::string::npos, msg1.find("failed"));

    std::string msg2 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::SET_SUB_WINDOW_MODAL, WMError::WM_ERROR_IPC_FAILED);
    EXPECT_NE(std::string::npos, msg2.find("failed"));

    std::string msg3 = WindowFocusErrorMsgHelper::GetErrorMsg(
        WindowFocusApiType::RAISE_TO_APP_TOP, WMError::WM_ERROR_IPC_FAILED);
    EXPECT_NE(std::string::npos, msg3.find("failed"));
}
} // namespace
} // namespace Rosen
} // namespace OHOS