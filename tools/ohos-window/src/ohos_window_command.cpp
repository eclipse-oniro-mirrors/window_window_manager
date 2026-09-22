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
#include <unordered_set>
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

const size_t RESTORE_WINDOW_PARAM_COUNT = 2;
const size_t HELP_PARAM_COUNT = 1;
const std::string OPTION_WINDOW_ID = "--windowId";
const std::string OPTION_HELP = "--help";
const std::string OPTION_DISPLAY_ID = "--displayId";
const std::string OPTION_FILTER = "--filter";
const std::string OPTION_TYPE = "--type";
const std::string OPTION_VAL_VISIBLE = "visible";
const std::string OPTION_VAL_EXCLUDE_SYSTEM = "excludeSystem";
const std::string OPTION_VAL_FOREGROUND = "foreground";
const std::string OPTION_VAL_UI = "ui";
const std::string OPTION_VAL_DISPLAY = "display";
const std::string OPTION_VAL_LAYOUT = "layout";
const std::string OPTION_VAL_META = "meta";
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
        {"restore-window", [this]() { return this->RunAsRestoreWindow(); }},
        {"list-windows", [this]() { return this->RunAsListWindowInfo(); }},
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

    errorInfoMap_[static_cast<int32_t>(WMError::WM_ERROR_IPC_FAILED)] = {
        "ERR_IPC_FAILED", "IPC communication failed.",
        "IPC request failed.", { IPC_FAILED_SOLUTION } };

    errorInfoMap_[static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION)] = {
        "ERR_NO_PERMISSION", "Operation failed.",
        "Permission verification failed.", { NO_PERMISSION_SOLUTION } };

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

int32_t ClawWindowShellCommand::RunAsRestoreWindow()
{
    if (argList_.size() == RESTORE_WINDOW_PARAM_COUNT && argList_[0] == OPTION_WINDOW_ID) {
        int32_t persistentId = 0;
        std::string idStr = argList_[1];
        auto res = std::from_chars(idStr.c_str(), idStr.c_str() + idStr.size(), persistentId);
        if (res.ec == std::errc() && res.ptr == idStr.c_str() + idStr.size() && persistentId >= 0) {
            return DoRestoreWindow(persistentId);
        }
        WmToolErrorInfo errorInfo = {ERR_INVALID_INPUT, "Invalid input parameters.",
            "The passed parameters are invalid.", {INVALID_PARAM_SOLUTION}};
        PrintError(errorInfo);
        return ERR_INVALID_VALUE;
    }
    if (argList_.size() == HELP_PARAM_COUNT && argList_[0] == OPTION_HELP) {
        std::cout << HELP_MSG_RESTORE_WINDOW << std::endl;
        return ERR_OK;
    }
    WmToolErrorInfo errorInfo = {ERR_INVALID_INPUT,
        "Invalid options or parameters for restore-window command.",
        "Wrong options or Missing parameters or too many parameters.", {HELP_MSG_RESTORE_WINDOW}};
    PrintError(errorInfo);
    return ERR_INVALID_VALUE;
}

int32_t ClawWindowShellCommand::DoRestoreWindow(int32_t persistentId)
{
    auto proxy = GetSceneSessionManagerLiteProxy();
    if (proxy == nullptr) {
        WmToolErrorInfo errorInfo = GetErrorInfoFromCode(static_cast<int32_t>(WSError::WS_ERROR_IPC_FAILED));
        PrintError(errorInfo);
        resultReceiver_ = STRING_RESTORE_WINDOW_NG;
        return ERR_INVALID_VALUE;
    }

    WSErrorResult result = proxy->RestoreSessionToForeground(persistentId);
    if (result.errCode == WSError::WS_OK) {
        resultReceiver_.append(STRING_RESTORE_WINDOW_OK);
        PrintSuccess(resultReceiver_);
        return ERR_OK;
    }

    resultReceiver_.append(STRING_RESTORE_WINDOW_NG);
    WmToolErrorInfo errorInfo = GetErrorInfoFromCode(static_cast<int32_t>(result.errCode));
    if (!result.errMsg.empty()) {
        errorInfo.cause = result.errMsg;
    }
    PrintError(errorInfo);
    return ERR_INVALID_VALUE;
}

int32_t ClawWindowShellCommand::RunAsListWindowInfo()
{
    WindowInfoOption infoOption;
    if (!ParseListWindowInfoOption(infoOption)) {
        return ERR_INVALID_VALUE;
    }
    auto proxy = GetSceneSessionManagerLiteProxy();
    if (proxy == nullptr) {
        WmToolErrorInfo errorInfo = GetErrorInfoFromCode(static_cast<int32_t>(WMError::WM_ERROR_IPC_FAILED));
        PrintError(errorInfo);
        return ERR_INVALID_VALUE;
    }
    std::vector<sptr<WindowInfo>> infos;
    auto ret = proxy->ListWindowInfo(infoOption, infos);
    if (ret != WMError::WM_OK) {
        WmToolErrorInfo errorInfo = GetErrorInfoFromCode(static_cast<int32_t>(ret));
        PrintError(errorInfo);
        return ERR_INVALID_VALUE;
    }
    BuildListWindowInfoResultJson(infos, infoOption.windowInfoTypeOption);
    return ERR_OK;
}

bool ClawWindowShellCommand::ParseListWindowInfoOption(WindowInfoOption& infoOption)
{
    size_t i = 0;
    const size_t nextOptStep = 2;
    std::unordered_set<std::string> expectValueOptions {
        OPTION_WINDOW_ID, OPTION_DISPLAY_ID, OPTION_FILTER, OPTION_TYPE };
    bool needResetTypeOptValue = true;
    while (i < argList_.size()) {
        const auto& opt = argList_[i];
        if (opt == OPTION_HELP) {
            std::cout << HELP_MSG_LIST_WINDOWS << std::endl;
            return false;
        }
        if (expectValueOptions.find(opt) == expectValueOptions.end()) {
            WmToolErrorInfo errorInfo = { ERR_INVALID_INPUT, "Invalid input parameters.",
                "Unsupported option: " + opt, { INVALID_PARAM_SOLUTION, HELP_MSG_LIST_WINDOWS } };
            PrintError(errorInfo);
            return false;
        }
        if (i >= argList_.size() - 1) {
            WmToolErrorInfo errorInfo = { ERR_INVALID_INPUT, "Invalid input parameters.",
                "Missing value for option: " + opt, { INVALID_PARAM_SOLUTION, HELP_MSG_LIST_WINDOWS } };
            PrintError(errorInfo);
            return false;
        }
        if (opt == OPTION_WINDOW_ID) {
            if (!ParseWindowIdOption(argList_[i + 1], infoOption.windowId)) {
                return false;
            }
        } else if (opt == OPTION_DISPLAY_ID) {
            if (!ParseDisplayIdOption(argList_[i + 1], infoOption.displayId)) {
                return false;
            }
        } else if (opt == OPTION_FILTER) {
            if (!ParseWindowInfoFilterOption(argList_[i + 1], infoOption.windowInfoFilterOption)) {
                return false;
            }
        } else if (opt == OPTION_TYPE) {
            if (needResetTypeOptValue) {
                infoOption.windowInfoTypeOption = static_cast<WindowInfoTypeOption>(0);
                needResetTypeOptValue = false;
            }
            if (!ParseWindowInfoTypeOption(argList_[i + 1], infoOption.windowInfoTypeOption)) {
                return false;
            }
        }
        i += nextOptStep;
    }
    return true;
}

bool ClawWindowShellCommand::ParseWindowIdOption(const std::string& idStr, int32_t& windowId)
{
    auto res = std::from_chars(idStr.c_str(), idStr.c_str() + idStr.size(), windowId);
    if (res.ec == std::errc() && res.ptr == idStr.c_str() + idStr.size()) {
        return true;
    }
    WmToolErrorInfo errorInfo = { ERR_INVALID_INPUT, "Invalid input parameters.",
        "Invalid value for " + OPTION_WINDOW_ID + " option: " + idStr, { INVALID_PARAM_SOLUTION } };
    PrintError(errorInfo);
    return false;
}

bool ClawWindowShellCommand::ParseDisplayIdOption(const std::string& idStr, DisplayId& displayId)
{
    auto res = std::from_chars(idStr.c_str(), idStr.c_str() + idStr.size(), displayId);
    if (res.ec == std::errc() && res.ptr == idStr.c_str() + idStr.size()) {
        return true;
    }
    WmToolErrorInfo errorInfo = { ERR_INVALID_INPUT, "Invalid input parameters.",
        "Invalid value for " + OPTION_DISPLAY_ID + " option: " + idStr, { INVALID_PARAM_SOLUTION } };
    PrintError(errorInfo);
    return false;
}

bool ClawWindowShellCommand::ParseWindowInfoFilterOption(const std::string& valStr,
    WindowInfoFilterOption& filterOption)
{
    if (valStr == OPTION_VAL_VISIBLE) {
        filterOption = filterOption | WindowInfoFilterOption::VISIBLE;
    } else if (valStr == OPTION_VAL_EXCLUDE_SYSTEM) {
        filterOption = filterOption | WindowInfoFilterOption::EXCLUDE_SYSTEM;
    } else if (valStr == OPTION_VAL_FOREGROUND) {
        filterOption = filterOption | WindowInfoFilterOption::FOREGROUND;
    } else {
        WmToolErrorInfo errorInfo = { ERR_INVALID_INPUT, "Invalid input parameters.",
            "Invalid value for " + OPTION_FILTER + " option: " + valStr, { INVALID_PARAM_SOLUTION } };
        PrintError(errorInfo);
        return false;
    }
    return true;
}

bool ClawWindowShellCommand::ParseWindowInfoTypeOption(const std::string& valStr, WindowInfoTypeOption& typeOption)
{
    if (valStr == OPTION_VAL_UI) {
        typeOption = typeOption | WindowInfoTypeOption::WINDOW_UI_INFO;
    } else if (valStr == OPTION_VAL_DISPLAY) {
        typeOption = typeOption | WindowInfoTypeOption::WINDOW_DISPLAY_INFO;
    } else if (valStr == OPTION_VAL_LAYOUT) {
        typeOption = typeOption | WindowInfoTypeOption::WINDOW_LAYOUT_INFO;
    } else if (valStr == OPTION_VAL_META) {
        typeOption = typeOption | WindowInfoTypeOption::WINDOW_META_INFO;
    } else {
        WmToolErrorInfo errorInfo = { ERR_INVALID_INPUT, "Invalid input parameters.",
            "Invalid value for " + OPTION_TYPE + " option: " + valStr, { INVALID_PARAM_SOLUTION } };
        PrintError(errorInfo);
        return false;
    }
    return true;
}

void ClawWindowShellCommand::BuildListWindowInfoResultJson(const std::vector<sptr<WindowInfo>>& infos,
    const WindowInfoTypeOption& typeOption)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    json windows = json::array();
    for (const auto& info : infos) {
        if (info == nullptr) {
            continue;
        }
        json window;
        if (IsChosenWindowOption(typeOption, WindowInfoTypeOption::WINDOW_UI_INFO)) {
            window["uiInfo"]["visibilityState"] = static_cast<uint32_t>(info->windowUIInfo.visibilityState);
        }
        if (IsChosenWindowOption(typeOption, WindowInfoTypeOption::WINDOW_DISPLAY_INFO)) {
            window["displayInfo"]["displayId"] = info->windowDisplayInfo.displayId;
        }
        if (IsChosenWindowOption(typeOption, WindowInfoTypeOption::WINDOW_LAYOUT_INFO)) {
            window["layoutInfo"]["rect"] = {
                {"posX", info->windowLayoutInfo.rect.posX_},
                {"posY", info->windowLayoutInfo.rect.posY_},
                {"width", info->windowLayoutInfo.rect.width_},
                {"height", info->windowLayoutInfo.rect.height_},
            };
            window["layoutInfo"]["zOrder"] = info->windowLayoutInfo.zOrder;
            window["layoutInfo"]["windowAlpha"] = info->windowLayoutInfo.windowAlpha;
        }
        if (IsChosenWindowOption(typeOption, WindowInfoTypeOption::WINDOW_META_INFO)) {
            FillWindowMetaInfoJson(info->windowMetaInfo, window);
        }
        windows.push_back(window);
    }
    response["data"]["windows"] = windows;
    std::cout << response.dump() << std::endl;
}

void ClawWindowShellCommand::FillWindowMetaInfoJson(const WindowMetaInfo& metaInfo, nlohmann::json& windowJson)
{
    windowJson["metaInfo"] = {
        {"windowId", metaInfo.windowId},
        {"windowName", metaInfo.windowName},
        {"bundleName", metaInfo.bundleName},
        {"abilityName", metaInfo.abilityName},
        {"appIndex", metaInfo.appIndex},
        {"pid", metaInfo.pid},
        {"windowType", static_cast<uint32_t>(metaInfo.windowType)},
        {"parentWindowId", metaInfo.parentWindowId},
        {"surfaceNodeId", metaInfo.surfaceNodeId},
        {"leashWinSurfaceNodeId", metaInfo.leashWinSurfaceNodeId},
        {"isPrivacyMode", metaInfo.isPrivacyMode},
        {"windowMode", static_cast<uint32_t>(metaInfo.windowMode)},
        {"windowModeInfo", {
            {"windowMode", static_cast<uint32_t>(metaInfo.windowModeInfo.windowMode)},
            {"splitStyle", static_cast<uint32_t>(metaInfo.windowModeInfo.splitStyle)},
            {"splitIndex", metaInfo.windowModeInfo.splitIndex},
        }},
        {"isMidScene", metaInfo.isMidScene},
        {"isFocused", metaInfo.isFocused},
        {"isTouchable", metaInfo.isTouchable},
        {"mainWindowPersistentId", metaInfo.mainWindowPersistentId},
        {"controlAppType", static_cast<uint32_t>(metaInfo.controlAppType)},
    };
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