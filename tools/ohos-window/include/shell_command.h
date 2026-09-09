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

#ifndef OHOS_ROSEN_CLAW_SHELL_COMMAND_H
#define OHOS_ROSEN_CLAW_SHELL_COMMAND_H

#include <cstdint>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "errors.h"

namespace OHOS {
namespace Rosen {

struct WmToolErrorInfo;

class ShellCommand {
public:
    ShellCommand(int argc, char* argv[], std::string name);
    virtual ~ShellCommand();

    int32_t OnCommand();
    std::string ExecCommand();
    std::string GetCommandErrorMsg() const;
    std::string GetUnknownOptionMsg(std::string& unknownOption) const;

    virtual int32_t CreateCommandMap() = 0;
    virtual int32_t init() = 0;

protected:
    static constexpr int MIN_ARGUMENT_NUMBER = 2;
    static constexpr int MAX_ARGUMENT_NUMBER = 4096;

    char** argv_ = nullptr;
    int argc_ = 0;
    std::string resultReceiver_ = "";

    std::string cmd_;
    std::vector<std::string> argList_;

    std::string name_;
    std::map<std::string, std::function<int32_t()>> commandMap_;
};

struct WmToolErrorInfo {
    std::string code;
    std::string message;
    std::string cause;
    std::vector<std::string> solutions;

    std::string SolutionsToString() const
    {
        std::ostringstream oss;
        for (const auto& solution : solutions) {
            oss << solution << "\n";
        }
        return oss.str();
    }
};
}  // namespace Rosen
}  // namespace OHOS

#endif  // OHOS_ROSEN_CLAW_SHELL_COMMAND_H