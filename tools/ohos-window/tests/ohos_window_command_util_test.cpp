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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <getopt.h>

#include "ohos_window_command.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Rosen;

namespace {
constexpr int ARGC_BELOW_MIN = 1;
constexpr int ARGC_SUBCOMMAND_ONLY = 2;
constexpr int ARGC_HELP_OPTION = 3;
constexpr int ARGC_OPTION_VALUE = 4;
constexpr int ARGC_EXTRA_ARG = 5;
constexpr int OPTIND_DEFAULT = 1;
constexpr int OPTIND_OFFSET = 10;

class MockClawWindowShellCommand : public ClawWindowShellCommand {
public:
    explicit MockClawWindowShellCommand(int argc, char* argv[]) : ClawWindowShellCommand(argc, argv) {}
    void SetProxy(const sptr<ISceneSessionManagerLite>& proxy)
    {
        proxy_ = proxy;
    }
    void SetInitResult(int32_t result)
    {
        initResult_ = result;
    }

protected:
    sptr<ISceneSessionManagerLite> GetSceneSessionManagerLiteProxy() override
    {
        return proxy_;
    }
    int32_t init() override
    {
        return initResult_;
    }

private:
    sptr<ISceneSessionManagerLite> proxy_ = nullptr;
    int32_t initResult_ = ERR_OK;
};
} // namespace

class OhosWindowCommandUtilTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void TearDown() override {}
};

TEST_F(OhosWindowCommandUtilTest, CreateCommandMap_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.CreateCommandMap(), ERR_OK);
    EXPECT_NE(cmd.commandMap_.find("--help"), cmd.commandMap_.end());
    EXPECT_NE(cmd.commandMap_.find("help"), cmd.commandMap_.end());
    EXPECT_NE(cmd.commandMap_.find("restore-session"), cmd.commandMap_.end());
}

TEST_F(OhosWindowCommandUtilTest, CreateErrorInfoMap_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("")
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.CreateErrorInfoMap(), ERR_OK);
    EXPECT_NE(cmd.errorInfoMap_.find(static_cast<int32_t>(WSError::WS_ERROR_INVALID_PERMISSION)),
        cmd.errorInfoMap_.end());
    EXPECT_NE(cmd.errorInfoMap_.find(static_cast<int32_t>(WSError::WS_ERROR_DEVICE_NOT_SUPPORT)),
        cmd.errorInfoMap_.end());
    EXPECT_NE(cmd.errorInfoMap_.find(static_cast<int32_t>(WSError::WS_ERROR_INVALID_PARAM)),
        cmd.errorInfoMap_.end());
    EXPECT_NE(cmd.errorInfoMap_.find(static_cast<int32_t>(WSError::WS_ERROR_IPC_FAILED)),
        cmd.errorInfoMap_.end());
    EXPECT_NE(cmd.errorInfoMap_.find(static_cast<int32_t>(WSError::WS_ERROR_INVALID_OPERATION)),
        cmd.errorInfoMap_.end());
}

TEST_F(OhosWindowCommandUtilTest, GetErrorInfoFromCode_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    auto info = cmd.GetErrorInfoFromCode(static_cast<int32_t>(WSError::WS_ERROR_INVALID_PARAM));
    EXPECT_EQ(info.code, "ERR_INVALID_INPUT");
}

TEST_F(OhosWindowCommandUtilTest, GetErrorInfoFromCode_0200)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    auto info = cmd.GetErrorInfoFromCode(999999);
    EXPECT_TRUE(info.code.empty());
}

TEST_F(OhosWindowCommandUtilTest, GetErrorInfoFromCode_0300)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    auto info = cmd.GetErrorInfoFromCode(static_cast<int32_t>(WSError::WS_ERROR_INVALID_OPERATION));
    EXPECT_EQ(info.code, "ERR_INVALID_OPERATION");
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_Help_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("--help"),
        const_cast<char*>(""),
    };
    int argc = ARGC_HELP_OPTION;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_OK);
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_MissingId_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_INVALID_VALUE);
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_InvalidId_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("--persistentId"),
        const_cast<char*>("abc"),
        const_cast<char*>(""),
    };
    int argc = ARGC_OPTION_VALUE;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_INVALID_VALUE);
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_InvalidId_0200)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("--persistentId"),
        const_cast<char*>("90.1"),
        const_cast<char*>(""),
    };
    int argc = ARGC_OPTION_VALUE;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_INVALID_VALUE);
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_UnexpectedArg_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("--persistentId"),
        const_cast<char*>("90"),
        const_cast<char*>("0"),
        const_cast<char*>(""),
    };
    int argc = ARGC_EXTRA_ARG;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_INVALID_VALUE);
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_NullProxy_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("--persistentId"),
        const_cast<char*>("100"),
        const_cast<char*>(""),
    };
    int argc = ARGC_OPTION_VALUE;
    MockClawWindowShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    cmd.SetProxy(nullptr);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_INVALID_VALUE);
}

TEST_F(OhosWindowCommandUtilTest, ShellCommand_Construct_ArgcTooSmall_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>(""),
    };
    int argc = ARGC_BELOW_MIN;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.cmd_, "help");
}

TEST_F(OhosWindowCommandUtilTest, ShellCommand_Construct_ArgvNull_0100)
{
    ClawWindowShellCommand cmd(ARGC_SUBCOMMAND_ONLY, nullptr);
    EXPECT_TRUE(cmd.cmd_.empty());
}

TEST_F(OhosWindowCommandUtilTest, OnCommand_UnknownCommand_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("foo"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    cmd.CreateCommandMap();
    EXPECT_EQ(cmd.OnCommand(), ERR_OK);
    EXPECT_FALSE(cmd.resultReceiver_.empty());
}

TEST_F(OhosWindowCommandUtilTest, OnCommand_InitFail_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    MockClawWindowShellCommand cmd(argc, argv);
    cmd.CreateCommandMap();
    cmd.SetInitResult(ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.OnCommand(), ERR_INVALID_VALUE);
}

TEST_F(OhosWindowCommandUtilTest, ExecCommand_OnCommandFail_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    MockClawWindowShellCommand cmd(argc, argv);
    cmd.SetInitResult(ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

TEST_F(OhosWindowCommandUtilTest, GetUnknownOptionMsg_OptindOutOfRange_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    std::string unknownOption;
    optind = argc + OPTIND_OFFSET;
    EXPECT_TRUE(cmd.GetUnknownOptionMsg(unknownOption).empty());
}

TEST_F(OhosWindowCommandUtilTest, GetUnknownOptionMsg_Normal_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    std::string unknownOption;
    optind = OPTIND_DEFAULT;
    EXPECT_FALSE(cmd.GetUnknownOptionMsg(unknownOption).empty());
}

TEST_F(OhosWindowCommandUtilTest, RunAsHelpCommand_Help_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("--help"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsHelpCommand(), ERR_OK);
}

TEST_F(OhosWindowCommandUtilTest, RunAsHelpCommand_InvalidCommand_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("foo"),
        const_cast<char*>(""),
    };
    int argc = ARGC_SUBCOMMAND_ONLY;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsHelpCommand(), ERR_OK);
}

TEST_F(OhosWindowCommandUtilTest, RunAsRestoreSession_NegativeId_0100)
{
    char* argv[] = {
        const_cast<char*>("ohos-window"),
        const_cast<char*>("restore-session"),
        const_cast<char*>("--persistentId"),
        const_cast<char*>("-90"),
        const_cast<char*>(""),
    };
    int argc = ARGC_OPTION_VALUE;
    ClawWindowShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsRestoreSession(), ERR_INVALID_VALUE);
}