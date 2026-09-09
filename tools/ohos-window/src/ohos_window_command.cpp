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

#include "ohos_window_command.h"

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

#include "session_manager_lite.h"

using json = nlohmann::json;

namespace OHOS {
namespace Rosen {
namespace {
const std::string ERR_INVALID_COMMAND = "ERR_INVALID_COMMAND";
const std::string ERR_INVALID_INPUT = "ERR_INVALID_INPUT";

const std::string NO_PERMISSION_SOLUTION =
    "Check that the caller has the required permission.";
const std::string DEVICE_NOT_SUPPORT_SOLUTION =
    "Verify that the device supports this feature.";
const std::string INVALID_PARAM_SOLUTION =
    "Check the passed parameters and ensure they are valid.";
const std::string IPC_FAILED_SOLUTION =
    "Try restarting the device and executing again.";
const std::string INVALID_OPERATION_SOLUTION =
    "Check the current device state and try again.";

const size_t RESTORE_SESSION_PARAM_COUNT = 2;
const size_t HELP_PARAM_COUNT = 1;
const std::string OPTION_WINDOW_ID = "--windowId";
const std::string OPTION_HELP = "--help";
}  // namespace

void PrintSuccess(const std::string& message)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    response["data"]["message"] = message;
    std::cout << response.dump() << std::endl;
}

void PrintError(const WmToolErrorInfo& errorInfo)
{
    json response;
    response["type"] = "result";
    response["status"] = "failed";
    response["errCode"] = errorInfo.code;
    std::string errMessage = errorInfo.message;
    if (errMessage != errorInfo.cause) {
        errMessage = errMessage + " " + errorInfo.cause;
    }
    response["errMsg"] = errMessage;
    response["suggestion"] = errorInfo.SolutionsToString();
    std::cout << response.dump() << std::endl;
}

ClawWindowShellCommand::ClawWindowShellCommand(int argc, char* argv[]) : ShellCommand(argc, argv, TOOL_NAME)
{}

int32_t ClawWindowShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"--help", [this]() { return this->RunAsHelpCommand(); }},
        {"help", [this]() { return this->RunAsHelpCommand(); }},
        {"restore-session", [this]() { return this->RunAsRestoreSession(); }},
    };
    return ERR_OK;
}

int32_t ClawWindowShellCommand::CreateErrorInfoMap()
{
    errorInfoMap_[static_cast<int32_t>(WSError::WS_ERROR_INVALID_PERMISSION)] = {
        "ERR_NO_PERMISSION", "Operation failed.",
        "Permission verification failed.", {NO_PERMISSION_SOLUTION}};

    errorInfoMap_[static_cast<int32_t>(WSError::WS_ERROR_DEVICE_NOT_SUPPORT)] = {
        "ERR_DEVICE_NOT_SUPPORT", "Operation failed.",
        "The device does not support this feature.", {DEVICE_NOT_SUPPORT_SOLUTION}};

    errorInfoMap_[static_cast<int32_t>(WSError::WS_ERROR_INVALID_PARAM)] = {
        "ERR_INVALID_INPUT", "Invalid input parameters.",
        "The passed parameters are invalid.", {INVALID_PARAM_SOLUTION}};

    errorInfoMap_[static_cast<int32_t>(WSError::WS_ERROR_IPC_FAILED)] = {
        "ERR_IPC_FAILED", "IPC communication failed.",
        "IPC request failed.", {IPC_FAILED_SOLUTION}};

    errorInfoMap_[static_cast<int32_t>(WSError::WS_ERROR_INVALID_OPERATION)] = {
        "ERR_INVALID_OPERATION", "Operation failed.",
        "The operation is not allowed in the current state.", {INVALID_OPERATION_SOLUTION}};

    return ERR_OK;
}

int32_t ClawWindowShellCommand::init()
{
    return ERR_OK;
}

int32_t ClawWindowShellCommand::RunAsHelpCommand()
{
    if (cmd_ != "--help" && cmd_ != "help") {
        std::string message = "Invalid command for ohos-window.";
        WmToolErrorInfo errorInfo = {
            ERR_INVALID_COMMAND,
            message,
            message,
            {HELP_MSG},
        };
        PrintError(errorInfo);
    } else {
        std::cout << HELP_MSG << std::endl;
    }
    return ERR_OK;
}

int32_t ClawWindowShellCommand::RunAsRestoreSession()
{
    if (argList_.size() == RESTORE_SESSION_PARAM_COUNT && argList_[0] == OPTION_WINDOW_ID) {
        int32_t persistentId = 0;
        std::string idStr = argList_[1];
        auto res = std::from_chars(idStr.c_str(), idStr.c_str() + idStr.size(), persistentId);
        if (res.ec == std::errc() && res.ptr == idStr.c_str() + idStr.size() && persistentId >= 0) {
            return DoRestoreSession(persistentId);
        }
        WmToolErrorInfo errorInfo = {ERR_INVALID_INPUT, "Invalid input parameters.",
            "The passed parameters are invalid.", {INVALID_PARAM_SOLUTION}};
        PrintError(errorInfo);
        return ERR_INVALID_VALUE;
    }
    if (argList_.size() == HELP_PARAM_COUNT && argList_[0] == OPTION_HELP) {
        std::cout << HELP_MSG_RESTORE_SESSION << std::endl;
        return ERR_OK;
    }
    WmToolErrorInfo errorInfo = {ERR_INVALID_INPUT,
        "Invalid options or parameters for restore-session command.",
        "Wrong options or Missing parameters or too many parameters.", {HELP_MSG_RESTORE_SESSION}};
    PrintError(errorInfo);
    return ERR_INVALID_VALUE;
}

int32_t ClawWindowShellCommand::DoRestoreSession(int32_t persistentId)
{
    auto proxy = GetSceneSessionManagerLiteProxy();
    if (proxy == nullptr) {
        WmToolErrorInfo errorInfo = GetErrorInfoFromCode(static_cast<int32_t>(WSError::WS_ERROR_IPC_FAILED));
        PrintError(errorInfo);
        resultReceiver_ = STRING_RESTORE_SESSION_NG;
        return ERR_INVALID_VALUE;
    }

    WSError ret = proxy->RestoreSessionToForeground(persistentId);
    if (ret == WSError::WS_OK) {
        resultReceiver_.append(STRING_RESTORE_SESSION_OK);
        PrintSuccess(resultReceiver_);
        return ERR_OK;
    }

    resultReceiver_.append(STRING_RESTORE_SESSION_NG);
    WmToolErrorInfo errorInfo = GetErrorInfoFromCode(static_cast<int32_t>(ret));
    PrintError(errorInfo);
    return ERR_INVALID_VALUE;
}

sptr<ISceneSessionManagerLite> ClawWindowShellCommand::GetSceneSessionManagerLiteProxy()
{
    return SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
}

WmToolErrorInfo ClawWindowShellCommand::GetErrorInfoFromCode(const int32_t code)
{
    WmToolErrorInfo result;
    if (errorInfoMap_.find(code) != errorInfoMap_.end()) {
        result = errorInfoMap_.at(code);
    }
    return result;
}
}  // namespace Rosen
}  // namespace OHOS