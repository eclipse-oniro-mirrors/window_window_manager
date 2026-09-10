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

namespace OHOS::Rosen {

namespace {

constexpr const char* MSG_DEVICE_NOT_SUPPORT_SHORT = "Capability not supported.";
constexpr const char* MSG_NO_PERMISSION_SHORT = "Permission verification failed.";
constexpr const char* MSG_SYSTEM_ABNORMAL_SHORT = "This window manager service works abnormally.";
constexpr const char* MSG_WINDOW_STATE_ABNORMAL_SHORT = "This window state is abnormal.";

constexpr const char* CAUSE_DEVICE_NOT_SUPPORT = "Failed to call the API due to limited device capabilities.";
constexpr const char* CAUSE_NO_PERMISSION = "The application does not have the permission required to call the API.";
constexpr const char* CAUSE_IPC_ERROR = "Internal IPC error.";

inline std::string BuildFullMsg(const std::string& shortMsg, const std::string& causes)
{
    return std::string(shortMsg) + " Possible causes: " + causes;
}

inline std::string BuildFullMsgSimple(const std::string& shortMsg, const std::string& cause)
{
    return std::string(shortMsg) + " Possible causes: " + cause;
}

} // namespace

const std::unordered_map<WmErrorCode, ApiErrorMsgInfo> FloatWindowErrorMsg::GLOBAL_DEFAULT_MSGS = {
    { WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
      { MSG_DEVICE_NOT_SUPPORT_SHORT, BuildFullMsg(MSG_DEVICE_NOT_SUPPORT_SHORT, CAUSE_DEVICE_NOT_SUPPORT) } },
    { WmErrorCode::WM_ERROR_NO_PERMISSION,
      { MSG_NO_PERMISSION_SHORT, BuildFullMsg(MSG_NO_PERMISSION_SHORT, CAUSE_NO_PERMISSION) } },
    { WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
      { MSG_SYSTEM_ABNORMAL_SHORT, BuildFullMsgSimple(MSG_SYSTEM_ABNORMAL_SHORT, CAUSE_IPC_ERROR) } },
};

const std::map<FloatWindowModule, std::unordered_map<WmErrorCode, ApiErrorMsgInfo>>
    FloatWindowErrorMsg::MODULE_DEFAULT_MSGS = {
        { FloatWindowModule::PIP,
          {
              { WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED, { "Failed to destroy the PiP window.", "" } },
              { WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY, { "The PiP window state is abnormal.", "" } },
              { WmErrorCode::WM_ERROR_PIP_CREATE_FAILED, { "Failed to create the PiP window.", "" } },
              { WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR, { "PiP internal error.", "" } },
              { WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION, { "Repeated PiP operation.", "" } },
              { WmErrorCode::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS,
                { "This operation conflicts with other floating windows.", "" } },
          } },
        { FloatWindowModule::FLOATING_BALL,
          {
              { WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY, { "The floating ball window state is abnormal.", "" } },
              { WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, { "Floating ball internal error.", "" } },
              { WmErrorCode::WM_ERROR_FB_PARAM_INVALID, { "Wrong parameters for operating the floating ball.", "" } },
              { WmErrorCode::WM_ERROR_FB_INVALID_STATE,
                { "The floating ball state does not support this operation.", "" } },
              { WmErrorCode::WM_ERROR_FB_CREATE_FAILED, { "Failed to create the floating ball window.", "" } },
              { WmErrorCode::WM_ERROR_FB_REPEAT_CONTROLLER, { "The floating ball controller already exists.", "" } },
              { WmErrorCode::WM_ERROR_FB_REPEAT_OPERATION, { "Repeated floating ball operation.", "" } },
              { WmErrorCode::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED, { "Failed to restore the main window.", "" } },
              { WmErrorCode::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED,
                { "Updating the template type is not allowed.", "" } },
              { WmErrorCode::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,
                { "Updating the static template is not allowed.", "" } },
          } },
        { FloatWindowModule::FLOAT_VIEW,
          {
              { WmErrorCode::WM_ERROR_STATE_ABNORMALLY, { MSG_WINDOW_STATE_ABNORMAL_SHORT, "" } },
              { WmErrorCode::WM_ERROR_INVALID_PARAM, { "Parameter error.", "" } },
              { WmErrorCode::WM_ERROR_FV_REPEAT_OPERATION, { "Repeated operations on the float view.", "" } },
              { WmErrorCode::WM_ERROR_FV_INVALID_STATE,
                { "The float view state does not support this operation.", "" } },
              { WmErrorCode::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED, { "Failed to restore the main window.", "" } },
              { WmErrorCode::WM_ERROR_FV_START_FAILED, { "Failed to start the float view.", "" } },
          } },
    };

std::string FloatWindowErrorMsg::BuildErrorMsg(const std::string& module, const std::string& methodName,
                                               const std::string& shortMsg, const std::string& customMessage)
{
    std::string prefix = "[" + module + "][" + methodName + "]msg: ";
    std::string result;
    result += prefix;
    if (!shortMsg.empty()) {
        result += shortMsg;
        result += " ";
    }
    if (!customMessage.empty()) {
        result += customMessage;
    }
    return result;
}

std::string FloatWindowErrorMsg::GetMsgFromMap(const std::unordered_map<WmErrorCode, ApiErrorMsgInfo>& moduleMap,
                                               WmErrorCode errorCode,
                                               const std::string& customMessage)
{
    auto codeIter = moduleMap.find(errorCode);
    if (codeIter != moduleMap.end()) {
        return codeIter->second.shortMessage;
    }

    auto globalIter = GLOBAL_DEFAULT_MSGS.find(errorCode);
    if (globalIter != GLOBAL_DEFAULT_MSGS.end()) {
        return globalIter->second.shortMessage;
    }

    return "No corresponding error message";
}

std::string FloatWindowErrorMsg::GetModuleName(FloatWindowModule module)
{
    switch (module) {
        case FloatWindowModule::PIP:
            return "PiPWindow";
        case FloatWindowModule::FLOATING_BALL:
            return "FBWindow";
        case FloatWindowModule::FLOAT_VIEW:
            return "FloatView";
        default:
            return "";
    }
}

std::string FloatWindowErrorMsg::GetApiErrorMsg(FloatWindowModule module, const std::string& methodName,
                                                WmErrorCode errorCode, const std::string& customMessage)
{
    std::string shortMsg {};
    auto iter = MODULE_DEFAULT_MSGS.find(module);
    if (iter != MODULE_DEFAULT_MSGS.end()) {
        shortMsg = GetMsgFromMap(iter->second, errorCode, customMessage);
    }
    std::string moduleName = GetModuleName(module);
    return BuildErrorMsg(moduleName, methodName, shortMsg, customMessage);
}

} // namespace OHOS::Rosen