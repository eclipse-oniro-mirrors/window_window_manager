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

#include "shell_command.h"

#include <sstream>

namespace OHOS {
namespace Rosen {
namespace {
const std::string CREATE_MAP_FAILED_MSG = "failed to create command map.\n";
const std::string EXEC_COMMAND_FAILED_MSG = "error: failed to execute your command.\n";
}  // namespace

ShellCommand::ShellCommand(int argc, char* argv[], const std::string& name)
{
    argc_ = argc;
    argv_ = argv;
    name_ = name;

    if (argc < MIN_ARGUMENT_NUMBER || argc > MAX_ARGUMENT_NUMBER) {
        cmd_ = "help";
        return;
    }
    if (argv == nullptr) {
        return;
    }
    cmd_ = argv[1];
    for (int i = 2; i < argc; i++) {
        argList_.push_back(argv[i]);
    }
}

ShellCommand::~ShellCommand()
{}

int32_t ShellCommand::OnCommand()
{
    int32_t result = ERR_OK;

    auto respond = commandMap_[cmd_];
    if (respond == nullptr) {
        resultReceiver_.append(GetCommandErrorMsg());
        respond = commandMap_["help"];
    }

    if (init() == ERR_OK) {
        respond();
    } else {
        result = ERR_INVALID_VALUE;
    }

    return result;
}

std::string ShellCommand::ExecCommand()
{
    if (CreateCommandMap() != ERR_OK) {
        resultReceiver_.append(CREATE_MAP_FAILED_MSG);
    }

    if (OnCommand() != ERR_OK) {
        resultReceiver_.assign(EXEC_COMMAND_FAILED_MSG);
    }

    return resultReceiver_;
}

std::string ShellCommand::GetCommandErrorMsg() const
{
    std::ostringstream os;
    os << name_ << ": '" << cmd_ << "' is not a valid "
       << name_ << " command. See '" << name_ << " help'." << '\n';
    return os.str();
}

}  // namespace Rosen
}  // namespace OHOS