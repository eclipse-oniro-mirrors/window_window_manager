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

#include <unordered_map>

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
/**
 * @brief Error message templates used to build API-facing error messages.
 */
struct ErrorMsgInfo {
    /**
     * @brief A brief summary of the error.
     */
    const char* shortMessage;

    /**
     * @brief A detailed description of the error, including possible causes.
     */
    const char* fullMessage;
};

// === Universal Error Codes ===
/* 0 Success */
constexpr ErrorMsgInfo OK_MSG = { "Ok.", "Ok." };

/* 201 Permission Denied */
constexpr ErrorMsgInfo NO_PERMISSION_MSG = {
    "Permission verification failed.",
    "Permission verification failed. The application does not have the permission required to call the API."
};

/* 202 Permission Verification Failed for Calling a System API */
constexpr ErrorMsgInfo NOT_SYSTEM_APP_MSG = {
    "Permission verification failed.",
    "Permission verification failed. A non-system application calls a system API."
};

/* 401 Parameter Check Failed */
constexpr ErrorMsgInfo INVALID_PARAM_MSG = {
    "Parameter error.",
    "Parameter error. Possible causes: "
    "1. A mandatory parameter is not passed; "
    "2. A parameter type is incorrect (Type Error); "
    "3. The number of parameters is incorrect (Argument Count Error); "
    "4. A null parameter is incorrect (Null Argument Error); "
    "5. A parameter format is incorrect (Format Error); "
    "6. A value range is incorrect (Value Range Error)."
};

/* 801 API Not Supported */
constexpr ErrorMsgInfo DEVICE_NOT_SUPPORT_MSG = {
    "Capability not supported.",
    "Capability not supported. Failed to call the API due to limited device capabilities."
};

// === Window Error Codes ===
const std::unordered_map<WMError, const char*> WM_ERROR_TO_MSG_MAP{
    { WMError::WM_OK, "ok" },
    { WMError::WM_DO_NOTHING, "do nothing" },
    { WMError::WM_ERROR_NO_MEM, "no mem" },
    { WMError::WM_ERROR_DESTROYED_OBJECT, "destroyed object" },
    { WMError::WM_ERROR_INVALID_WINDOW, "invalid window" },
    { WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, "invalid window mode or size" },
    { WMError::WM_ERROR_INVALID_OPERATION, "invalid operation" },
    { WMError::WM_ERROR_INVALID_PERMISSION, "invalid permission" },
    { WMError::WM_ERROR_NOT_SYSTEM_APP, "not system app" },
    { WMError::WM_ERROR_NO_REMOTE_ANIMATION, "no remote animation" },
    { WMError::WM_ERROR_INVALID_DISPLAY, "invalid display" },
    { WMError::WM_ERROR_INVALID_PARENT, "invalid parent" },
    { WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS, "invalid operation in the current window status" },
    { WMError::WM_ERROR_REPEAT_OPERATION, "repeat operation" },
    { WMError::WM_ERROR_INVALID_SESSION, "invalid session" },
    { WMError::WM_ERROR_INVALID_CALLING, "invalid calling" },
    { WMError::WM_ERROR_SYSTEM_ABNORMALLY, "system abnormally" },
    { WMError::WM_ERROR_DEVICE_NOT_SUPPORT, "device not support" },
    { WMError::WM_ERROR_NEED_REPORT_BASE, "need report base" },
    { WMError::WM_ERROR_NULLPTR, "nullptr" },
    { WMError::WM_ERROR_INVALID_TYPE, "invalid type" },
    { WMError::WM_ERROR_INVALID_PARAM, "invalid param" },
    { WMError::WM_ERROR_SAMGR, "samgr" },
    { WMError::WM_ERROR_IPC_FAILED, "ipc failed" },
    { WMError::WM_ERROR_NEED_REPORT_END, "need report end" },
    { WMError::WM_ERROR_START_ABILITY_FAILED, "start ability failed" },
    { WMError::WM_ERROR_PIP_DESTROY_FAILED, "pip destroy failed" },
    { WMError::WM_ERROR_PIP_STATE_ABNORMALLY, "pip state abnormally" },
    { WMError::WM_ERROR_PIP_CREATE_FAILED, "pip create failed" },
    { WMError::WM_ERROR_PIP_INTERNAL_ERROR, "pip internal error" },
    { WMError::WM_ERROR_PIP_REPEAT_OPERATION, "pip repeat operation" },
    { WMError::WM_ERROR_ILLEGAL_PARAM, "parameter validation error" },
    { WMError::WM_ERROR_UI_EFFECT_ERROR, "Incorrect filter calling" },
    { WMError::WM_ERROR_TIMEOUT, "API call timed out" },
    { WMError::WM_ERROR_FB_PARAM_INVALID, "wrong parameters for operating floating ball" },
    { WMError::WM_ERROR_FB_CREATE_FAILED, "failed to create the floating ball window" },
    { WMError::WM_ERROR_FB_REPEAT_CONTROLLER, "failed to start multiple floating ball windows" },
    { WMError::WM_ERROR_FB_REPEAT_OPERATION, "repeated floating ball operation" },
    { WMError::WM_ERROR_FB_INTERNAL_ERROR, "floating ball internal error" },
    { WMError::WM_ERROR_FB_STATE_ABNORMALLY, "the floating ball window state is abnormal" },
    { WMError::WM_ERROR_FB_INVALID_STATE, "the floating ball state does not support this operation" },
    { WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED, "failed to restore the main window" },
    { WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED,
      "when updating the floating ball, the template type cannot be changed" },
    { WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,
      "updating static template-based floating ball is not supported" },
    { WMError::WM_ERROR_INVALID_WINDOW_TYPE, "invalid window type" },
    { WMError::WM_ERROR_FV_REPEAT_OPERATION, "repeated float view operation" },
    { WMError::WM_ERROR_FV_INVALID_STATE, "the float view state does not support this operation" },
    { WMError::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED, "failed to restore main window" },
    { WMError::WM_ERROR_FV_START_FAILED, "failed to start float view" },
    { WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS, "float window conflicts with other window" },
    { WMError::WM_ERROR_FORBID_SUBWINDOW, "forbid subwindow" },
};

const std::unordered_map<WmErrorCode, ErrorMsgInfo> WM_ERROR_CODE_TO_MSG_MAP{
    { WmErrorCode::WM_OK, OK_MSG },
    { WmErrorCode::WM_ERROR_NO_PERMISSION, NO_PERMISSION_MSG },
    { WmErrorCode::WM_ERROR_NOT_SYSTEM_APP, NOT_SYSTEM_APP_MSG },
    { WmErrorCode::WM_ERROR_INVALID_PARAM, INVALID_PARAM_MSG },
    { WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, DEVICE_NOT_SUPPORT_MSG },
    { WmErrorCode::WM_ERROR_REPEAT_OPERATION,
      { "Repeated operation.",
        "Repeated operation. Possible causes: "
        "1. The window has been created; "
        "2. The window is in the current state." } },
    { WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
      { "This window state is abnormal.",
        // Keep the original error message unchanged for compatibility and
        // existing XTS cases. Ideally, the detailed message should be:
        // "This window state is abnormal. Possible causes:
        //  1. The window to operate is not created or has been destroyed;
        //  2. Internal task error."
        "This window state is abnormal. Possible causes: The window is not created or destroyed." } },
    { WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
      { "This window manager service works abnormally.",
        "This window manager service works abnormally. Possible causes: "
        "The internal services of the window are not started normally." } },
    { WmErrorCode::WM_ERROR_INVALID_CALLING,
      { "Unauthorized operation.",
        "Unauthorized operation. Possible causes: "
        "1. The window object of another process is operated; "
        "2. The window type is not supported." } },
    { WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
      { "This window stage is abnormal.",
        "This window stage is abnormal. Possible causes: "
        "The window stage is not created or has been destroyed." } },
    { WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,
      { "This window context is abnormal.",
        "This window context is abnormal. Possible causes: "
        "The window context has been destroyed when being operated." } },
    { WmErrorCode::WM_ERROR_START_ABILITY_FAILED,
      { "Failed to start the ability.",
        "Failed to start the ability. Possible causes: "
        "Incorrect parameters are passed into the API used by the WindowExtensionAbility to start the application." } },
    { WmErrorCode::WM_ERROR_INVALID_DISPLAY,
      { "The display device is abnormal.",
        "The display device is abnormal. Possible causes: "
        "1. The display device is not ready; "
        "2. The display device is removed; "
        "3. The display device is damaged." } },
    { WmErrorCode::WM_ERROR_INVALID_PARENT,
      { "The parent window is invalid.",
        "The parent window is invalid. Possible causes: "
        "1. No parent window is bound; "
        "2. The parent window bound is abnormal. For example, the parent window has been destroyed." } },
    { WmErrorCode::WM_ERROR_INVALID_OP_IN_CUR_STATUS,
      { "The operation in the current window status is invalid.",
        "The operation in the current window status is invalid. Possible causes: "
        "1. Perform the move operation in the full-screen or split-screen window; "
        "2. Perform the resize operation in the full-screen or split-screen window." } },
    { WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED,
      { "Failed to destroy the PiP window.",
        "Failed to destroy the PiP window. Possible causes: "
        "The pointer to the PiP window is null." } },
    { WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY,
      { "The PiP window state is abnormal.", "The PiP window state is abnormal." } },
    { WmErrorCode::WM_ERROR_PIP_CREATE_FAILED,
      { "Failed to create the PiP window.",
        "Failed to create the PiP window. Possible causes: "
        "1. Incorrect parameters are passed in to create the PiP window; "
        "2. Attempt to start PiP in a non-full-screen window." } },
    { WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR,
      { "PiP internal error.",
        "PiP internal error. Possible causes: "
        "1. The window on which the PiP feature depends is abnormal or empty; "
        "2. The PiP controller is abnormal." } },
    { WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION,
      { "Repeated PiP operation.",
        "Repeated PiP operation. Possible causes: The PiP window has been started or closed." } },
    { WmErrorCode::WM_ERROR_ILLEGAL_PARAM,
      { "Parameter validation error.",
        "Parameter validation error. Possible causes: "
        "1. The parameter value is out of range; "
        "2. The parameter length exceeds the allowed limits; "
        "3. The parameter format is incorrect." } },
    { WmErrorCode::WM_ERROR_UI_EFFECT_ERROR,
      { "Incorrect ui effect calling.",
        "Incorrect ui effect calling. Possible causes: "
        "Must use setBackgroundFilter before animateToBackgroundFilter." } },
    { WmErrorCode::WM_ERROR_TIMEOUT,
      { "API call timed out.",
        "API call timed out. Possible causes: "
        "The wait time for a synchronous API call exceeds the upper limit." } },
    { WmErrorCode::WM_ERROR_FB_PARAM_INVALID,
      { "Wrong parameters for operating the floating ball.",
        "Wrong parameters for operating the floating ball. Possible causes: "
        "1. The parameter value is out of range; "
        "2. The parameter length exceeds the allowed limits; "
        "3. The parameter format is incorrect; "
        "4. A mandatory parameter is not passed." } },
    { WmErrorCode::WM_ERROR_FB_CREATE_FAILED,
      { "Failed to create the floating ball window.",
        "Failed to create the floating ball window. Possible causes: "
        "1. Incorrect parameters are passed in to start the floating ball; "
        "2. Attempt to start the floating ball on an unsupported device; "
        "3. Attempt to start the floating ball when the application is in the background." } },
    { WmErrorCode::WM_ERROR_FB_REPEAT_CONTROLLER,
      { "Failed to start multiple floating ball windows.", "Failed to start multiple floating ball windows." } },
    { WmErrorCode::WM_ERROR_FB_REPEAT_OPERATION,
      { "Repeated floating ball operation.",
        "Repeated floating ball operation. Possible causes: "
        "1. Attempt to start the floating ball while it is already running; "
        "2. Attempt to stop the floating ball after it has already stopped; "
        "3. Attempt to register the floating ball callback multiple times." } },
    { WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
      { "Floating ball internal error.",
        "Floating ball internal error. Possible causes: "
        "1. The window on which the floating ball depends is abnormal or empty; "
        "2. The floating ball controller is abnormal or empty." } },
    { WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY,
      { "The floating ball window state is abnormal.",
        "The floating ball window state is abnormal. Possible causes: "
        "The floating ball window may not have been created or may have been destroyed." } },
    { WmErrorCode::WM_ERROR_FB_INVALID_STATE,
      { "The floating ball state does not support this operation.",
        "The floating ball state does not support this operation. Possible causes: "
        "1. Attempt to update the floating ball when it is not active; "
        "2. Attempt to query window information when the floating ball is not active; "
        "3. Attempt to launch an application window when the floating ball is not active; "
        "4. Attempt to start the floating ball before the stop process is complete." } },
    { WmErrorCode::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED,
      { "Failed to restore the main window.",
        "Failed to restore the main window. Possible causes: "
        "1. Incorrect parameters are passed; "
        "2. The application does not have the ohos.permission.AUTO_RESTORE_MAIN_WINDOW "
        "permission and the floating ball is not tapped before the application window is launched; "
        "3. Attempt to launch a window that belongs to another application." } },
    { WmErrorCode::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED,
      { "When updating the floating ball, the template type cannot be changed.",
        "When updating the floating ball, the template type cannot be changed. Possible causes: "
        "The template type used during the update is inconsistent with the one used during creation." } },
    { WmErrorCode::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,
      { "Updating static template-based floating balls is not supported.",
        "Updating static template-based floating balls is not supported." } },
    { WmErrorCode::WM_ERROR_INVALID_WINDOW_TYPE, { "This window type is invalid.", "This window type is invalid." } },
    { WmErrorCode::WM_ERROR_FV_REPEAT_OPERATION,
      { "Repeated operations on the float view.",
        "Repeated operations on the float view. Possible causes: "
        "1. The float view is being started or has been started, and is started again; "
        "2. The float view is being stopped or has been stopped, and is stopped again; "
        "3. The float view callback is registered repeatedly." } },
    { WmErrorCode::WM_ERROR_FV_INVALID_STATE,
      { "The floatView state does not support this operation.",
        "The floatView state does not support this operation. Possible causes: "
        "1. The float view has been started but has not been stopped, and an operation "
        "that requires the float view be in stopped state (such as binding or unbinding) is performed; "
        "2. The float view has not been started, and an operation that requires the float view be in "
        "started state (such as stopping, restoring the main window, or obtaining window attributes) is performed; "
        "3. The float view is in stopping state, and a startup operation is performed." } },
    { WmErrorCode::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED,
      { "Failed to restore the main window.",
        "Failed to restore the main window. Possible causes: "
        "1. The user has never clicked the float view; "
        "2. The float view is not displayed in the foreground; "
        "3. The main window is in the PAUSED lifecycle state; "
        "4. The main window is in the multitasking screen." } },
    { WmErrorCode::WM_ERROR_FV_START_FAILED,
      { "Failed to start float view.",
        "Failed to start float view. Possible causes: "
        "1. Multiple float views are started for the same application; "
        "2. When the float view is started, the main window associated "
        "with the context is not displayed in the foreground." } },
    { WmErrorCode::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS,
      { "This operation conflicts with other floating windows.",
        "This operation conflicts with other floating windows. Possible causes: "
        "The floating ball window or PiP window has been started in the application." } },
    { WmErrorCode::WM_ERROR_FORBID_SUBWINDOW,
      { "Creating a subwindow is not allowed in the current context.",
        "Creating a subwindow is not allowed in the current context. Possible cause: "
        "An AgentUIExtensionAbility cannot create a subwindow." } },
};

// ===  Display Error Codes ===
const std::unordered_map<DMError, const char*> DM_ERROR_TO_MSG_MAP{
    { DMError::DM_ERROR_UNKNOWN, "unknown" },
    { DMError::DM_OK, "ok" },
    { DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED, "init dms proxy locked" },
    { DMError::DM_ERROR_IPC_FAILED, "ipc failed" },
    { DMError::DM_ERROR_REMOTE_CREATE_FAILED, "remote create failed" },
    { DMError::DM_ERROR_NULLPTR, "nullptr" },
    { DMError::DM_ERROR_INVALID_PARAM, "invalid param" },
    { DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED, "write interface token failed" },
    { DMError::DM_ERROR_DEATH_RECIPIENT, "death recipient" },
    { DMError::DM_ERROR_INVALID_MODE_ID, "invalid mode id" },
    { DMError::DM_ERROR_WRITE_DATA_FAILED, "write data failed" },
    { DMError::DM_ERROR_RENDER_SERVICE_FAILED, "render service failed" },
    { DMError::DM_ERROR_UNREGISTER_AGENT_FAILED, "unregister agent failed" },
    { DMError::DM_ERROR_INVALID_CALLING, "invalid calling" },
    { DMError::DM_ERROR_INVALID_PERMISSION, "invalid permission" },
    { DMError::DM_ERROR_NOT_SYSTEM_APP, "not system app" },
    { DMError::DM_ERROR_DISPLAY_MODE_SWITCH_PENDING, "display mode switch pending" },
    { DMError::DM_ERROR_DEVICE_NOT_SUPPORT, "device not support" },
    { DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING, "not support coordinate when wired casting" },
    { DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRLESS_CASTING, "not support coordinate when wireless casting" },
    { DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_RECORDING, "not support coordinate when recording" },
    { DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_TENTMODE, "not support coordinate when tent mode" },
    { DMError::DM_ERROR_ILLEGAL_PARAM, "parameter validation error" },
};

const std::unordered_map<DmErrorCode, ErrorMsgInfo> DM_ERROR_CODE_TO_MSG_MAP{
    { DmErrorCode::DM_OK, OK_MSG },
    { DmErrorCode::DM_ERROR_NO_PERMISSION, NO_PERMISSION_MSG },
    { DmErrorCode::DM_ERROR_NOT_SYSTEM_APP, NOT_SYSTEM_APP_MSG },
    { DmErrorCode::DM_ERROR_INVALID_PARAM, INVALID_PARAM_MSG },
    { DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT, DEVICE_NOT_SUPPORT_MSG },
    { DmErrorCode::DM_ERROR_INVALID_SCREEN,
      { "Invalid display or screen.",
        "Invalid display or screen. Possible causes: "
        "1. The virtual screen has not been created; "
        "2. The virtual screen has been destroyed." } },
    { DmErrorCode::DM_ERROR_INVALID_CALLING,
      { "Unauthorized operation.",
        "Unauthorized operation. Possible cause: "
        "The creator and destroyer of the virtual screen are different." } },
    { DmErrorCode::DM_ERROR_SYSTEM_INNORMAL,
      { "This display manager service works abnormally.",
        "This display manager service works abnormally. Possible causes: "
        "1. The display manager service is not started normally; "
        "2. The bottom-layer graphics synthesis and rendering are abnormal." } },
    { DmErrorCode::DM_ERROR_ILLEGAL_PARAM,
      { "Parameter error.", "Parameter error. Possible causes: The parameter value is out of range." } },
};

/**
 * @brief Finds API-facing WM error message templates by error code.
 *
 * @return Pointer to the error message template if found; nullptr otherwise.
 */
const ErrorMsgInfo* FindErrorMsgInfo(WmErrorCode errorCode)
{
    auto iter = WM_ERROR_CODE_TO_MSG_MAP.find(errorCode);
    if (iter == WM_ERROR_CODE_TO_MSG_MAP.end()) {
        TLOGW(WmsLogTag::DEFAULT,
              "No corresponding error message for this WmErrorCode: %d",
              static_cast<int32_t>(errorCode));
        return nullptr;
    }
    return &iter->second;
}

/**
 * @brief Finds API-facing DM error message templates by error code.
 *
 * @return Pointer to the error message template if found; nullptr otherwise.
 */
const ErrorMsgInfo* FindErrorMsgInfo(DmErrorCode errorCode)
{
    auto iter = DM_ERROR_CODE_TO_MSG_MAP.find(errorCode);
    if (iter == DM_ERROR_CODE_TO_MSG_MAP.end()) {
        TLOGW(WmsLogTag::DEFAULT,
              "No corresponding error message for this DmErrorCode: %d",
              static_cast<int32_t>(errorCode));
        return nullptr;
    }
    return &iter->second;
}
} // namespace

std::string WindowErrorMsg::GetErrorMsg(WMError error)
{
    auto iter = WM_ERROR_TO_MSG_MAP.find(error);
    if (iter == WM_ERROR_TO_MSG_MAP.end()) {
        TLOGW(WmsLogTag::DEFAULT,
              "No corresponding error message for this WMError: %d",
              static_cast<int32_t>(error));
        return "";
    }
    return iter->second;
}

std::string WindowErrorMsg::GetShortErrorMsg(WmErrorCode errorCode)
{
    auto info = FindErrorMsgInfo(errorCode);
    return info == nullptr ? "" : info->shortMessage;
}

std::string WindowErrorMsg::GetFullErrorMsg(WmErrorCode errorCode)
{
    auto info = FindErrorMsgInfo(errorCode);
    return info == nullptr ? "" : info->fullMessage;
}

std::string WindowErrorMsg::BuildErrorMsg(WmErrorCode errorCode, const std::string& customMessage)
{
    auto info = FindErrorMsgInfo(errorCode);
    if (info == nullptr) {
        return "";
    }
    // Preserve the original message format for compatibility.
    // Historically, shortMessage and customMessage are concatenated without
    // a whitespace, and existing XTS cases rely on this behavior.
    return customMessage.empty() ? info->fullMessage : std::string(info->shortMessage) + customMessage;
}

std::string WindowErrorMsg::GetErrorMsg(DMError error)
{
    auto iter = DM_ERROR_TO_MSG_MAP.find(error);
    if (iter == DM_ERROR_TO_MSG_MAP.end()) {
        TLOGW(WmsLogTag::DEFAULT,
              "No corresponding error message for this DMError: %d",
              static_cast<int32_t>(error));
        return "";
    }
    return iter->second;
}

std::string WindowErrorMsg::GetShortErrorMsg(DmErrorCode errorCode)
{
    auto info = FindErrorMsgInfo(errorCode);
    return info == nullptr ? "" : info->shortMessage;
}

std::string WindowErrorMsg::GetFullErrorMsg(DmErrorCode errorCode)
{
    auto info = FindErrorMsgInfo(errorCode);
    return info == nullptr ? "" : info->fullMessage;
}

std::string WindowErrorMsg::BuildErrorMsg(DmErrorCode errorCode, const std::string& customMessage)
{
    auto info = FindErrorMsgInfo(errorCode);
    if (info == nullptr) {
        return "";
    }
    // Preserve the original message format for compatibility.
    // Historically, shortMessage and customMessage are concatenated without
    // a whitespace, and existing XTS cases rely on this behavior.
    return customMessage.empty() ? info->fullMessage : std::string(info->shortMessage) + customMessage;
}
} // namespace OHOS::Rosen
