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

#ifndef OHOS_ROSEN_CLAW_WINDOW_COMMAND_H
#define OHOS_ROSEN_CLAW_WINDOW_COMMAND_H

#include <map>
#include <string>
#include <vector>

#include "shell_command.h"
#include "zidl/scene_session_manager_lite_interface.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string TOOL_NAME = "ohos-window";

const std::string HELP_MSG =
    "ohos-window - Window management utility for restoring a main window to foreground\n\n"
    "Usage:\n"
    "  ohos-window [command] [options]\n\n"
    "Parameters:\n"
    "  --help                    Display this help message\n\n"
    "SubCommands:\n"
    "  restore-session           restore a main window to foreground\n\n"
    "Examples:\n"
    "  # Restore a main window to foreground\n"
    "  ohos-window restore-session --windowId 100\n";

const std::string VERSION_MSG = "1.0.0\n";

const std::string HELP_MSG_RESTORE_SESSION =
    "ohos-window restore-session - Restore a main window to foreground\n\n"
    "Usage:\n"
    "  ohos-window restore-session [options]\n\n"
    "Parameters:\n"
    "  --help                                             Display this help message\n"
    "  --windowId <windowId>                       windowId of the session to be restored to foreground\n"
    "Examples:\n"
    "  # Restore a main window to foreground\n"
    "  ohos-window restore-session --windowId 100\n";

const std::string STRING_RESTORE_SESSION_OK = "restore main window to foreground successfully.";
const std::string STRING_RESTORE_SESSION_NG = "error: failed to restore main window to foreground.";

}  // namespace

class ClawWindowShellCommand : public ShellCommand {
public:
    ClawWindowShellCommand(int argc, char* argv[]);
    ~ClawWindowShellCommand() override
    {}

    int32_t CreateErrorInfoMap();
    WmToolErrorInfo GetErrorInfoFromCode(const int32_t code);

protected:
    virtual sptr<ISceneSessionManagerLite> GetSceneSessionManagerLiteProxy();

private:
    int32_t init() override;
    int32_t CreateCommandMap() override;

    int32_t RunAsHelpCommand();
    int32_t RunAsRestoreSession();
    int32_t DoRestoreSession(int32_t persistentId);

    std::map<int32_t, WmToolErrorInfo> errorInfoMap_;
};
}  // namespace Rosen
}  // namespace OHOS

#endif  // OHOS_ROSEN_CLAW_WINDOW_COMMAND_H