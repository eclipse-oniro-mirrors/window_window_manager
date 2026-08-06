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

namespace OHOS {
namespace Rosen {

namespace {
    constexpr const char* const FOCUS_API_NAMES[] = {
        "[window][setFocusable]msg: ",
        "[window][setWindowFocusable]msg: ",
        "[window][setSubWindowModal]msg: ",
        "[window][setTopmost]msg: ",
        "[window][setWindowTopmost]msg: ",
        "[window][raiseToAppTop]msg: ",
        "[window][setSubWindowZLevel]msg: ",
        "[window][getSubWindowZLevel]msg: ",
        "[window][setWindowModal]msg: ",
        "[window][setRaiseByClickEnabled]msg: ",
        "[window][setMainWindowRaiseByClickEnabled]msg: ",
        "[window][raiseAboveTarget]msg: ",
        "[window][raiseMainWindowAboveTarget]msg: ",
        "[window][setExclusivelyHighlighted]msg: ",
        "[window][setWindowDelayRaiseOnDrag]msg: ",
        "[window][shiftAppWindowFocus]msg: ",
        "[window][getTopWindow]msg: ",
        "[window][isFocused]msg: ",
        "[window][isWindowHighlighted]msg: ",
    };

    constexpr size_t FOCUS_API_COUNT = sizeof(FOCUS_API_NAMES) / sizeof(FOCUS_API_NAMES[0]);

    constexpr const char* const ERROR_NOT_CREATED = "The window is not created or destroyed.";
    constexpr const char* const ERROR_ONLY_SUB = "Invalid window type. Only subwindows are supported.";
    constexpr const char* const ERROR_ONLY_MAIN = "Invalid window type. Only main windows are supported.";
    constexpr const char* const ERROR_NO_PARENT = "The parent window does not exist or has been destroyed.";
    constexpr const char* const ERROR_NOT_SHOWN = "The window is not shown.";
    constexpr const char* const ERROR_ONLY_NON_MODAL = "Invalid window type. Only non-modal subwindows are supported.";
    constexpr const char* const ERROR_SUB_OR_DIALOG = "Invalid window type. Only subwindows are supported.";
    constexpr const char* const ERROR_TYPE_NOT_SUPPORT = "Invalid window type. Only app windows are supported.";
    constexpr const char* const ERROR_SOURCE_NOT_FOCUSED = "The source window is not focused.";
    constexpr const char* const ERROR_NOT_SAME_APP = "The two windows are not from the same process.";
    constexpr const char* const ERROR_TARGET_ALREADY_FOCUSED = "The target window is already focused.";
    constexpr const char* const ERROR_CANNOT_FIND_TOP = "Cannot find top window with the given context or window id.";
    constexpr const char* const ERROR_MAIN_OR_TARGET_INVALID = "Invalid window type. Only main windows are supported, and target must not be modal or topmost.";
    constexpr const char* const ERROR_SUB_OR_TARGET_NOT_SHOWN = "The window or target window is not shown.";
    constexpr const char* const ERROR_SCREEN_NOT_ALLOW_FOCUSED = "The screen of the window is not allowed to be focused.";
}

std::string WindowFocusErrorMsgHelper::GetErrorMsg(WindowFocusApiType apiType, WMError error,
    const std::string& customErrMsg)
{
    auto errorIt = WM_JS_TO_ERROR_CODE_MAP.find(error);
    if (errorIt == WM_JS_TO_ERROR_CODE_MAP.end()) {
        return std::string(" ") + GetApiName(apiType) + "failed.";
    }

    WmErrorCode wmErrorCode = errorIt->second;
    const char* apiName = GetApiName(apiType);

    if (IsFixedGeneralError(error)) {
        return std::string(" ") + apiName + WindowErrorMsg::GetFullErrorMsg(wmErrorCode);
    }

    const char* specificMsg = customErrMsg.empty() ? GetSpecificErrorMsg(apiType, error) : "";
    const char* detail = !customErrMsg.empty() ? customErrMsg.c_str() : specificMsg;

    if (detail == nullptr || strlen(detail) == 0) {
        return std::string(" ") + apiName + "failed.";
    }
    return std::string(" ") + apiName + detail;
}

const char* WindowFocusErrorMsgHelper::GetApiName(WindowFocusApiType apiType)
{
    uint32_t index = static_cast<uint32_t>(apiType);
    return (index < FOCUS_API_COUNT) ? FOCUS_API_NAMES[index] : "[window]msg: ";
}

const char* WindowFocusErrorMsgHelper::GetSpecificErrorMsg(WindowFocusApiType apiType, WMError error)
{
    if (IsFixedGeneralError(error)) {
        return "";
    }

    if (error == WMError::WM_ERROR_INVALID_WINDOW || error == WMError::WM_ERROR_INVALID_SESSION) {
        return ERROR_NOT_CREATED;
    }

    switch (apiType) {
        case WindowFocusApiType::SET_FOCUSABLE:
        case WindowFocusApiType::SET_WINDOW_FOCUSABLE:
            if (error == WMError::WM_ERROR_NULLPTR || error == WMError::WM_DO_NOTHING) {
                return ERROR_NOT_CREATED;
            }
            if (error == WMError::WM_ERROR_INVALID_OPERATION) return ERROR_SCREEN_NOT_ALLOW_FOCUSED;
            break;
        case WindowFocusApiType::SET_SUB_WINDOW_MODAL:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_SUB;
            if (error == WMError::WM_ERROR_NULLPTR || error == WMError::WM_DO_NOTHING) return ERROR_NOT_CREATED;
            break;
        case WindowFocusApiType::SET_TOPMOST:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_MAIN;
            if (error == WMError::WM_ERROR_NULLPTR || error == WMError::WM_DO_NOTHING) return ERROR_NOT_CREATED;
            break;
        case WindowFocusApiType::SET_WINDOW_TOPMOST:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_MAIN;
            if (error == WMError::WM_ERROR_NULLPTR || error == WMError::WM_DO_NOTHING) return ERROR_NOT_CREATED;
            break;
        case WindowFocusApiType::RAISE_TO_APP_TOP:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_SUB;
            if (error == WMError::WM_ERROR_INVALID_PARENT) return ERROR_NO_PARENT;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            if (error == WMError::WM_DO_NOTHING) return ERROR_NOT_SHOWN;
            break;
        case WindowFocusApiType::SET_SUB_WINDOW_Z_LEVEL:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_NON_MODAL;
            if (error == WMError::WM_ERROR_INVALID_PARENT) return ERROR_NO_PARENT;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            break;
        case WindowFocusApiType::GET_SUB_WINDOW_Z_LEVEL:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_SUB_OR_DIALOG;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            break;
        case WindowFocusApiType::SET_WINDOW_MODAL:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_MAIN;
            break;
        case WindowFocusApiType::SET_RAISE_BY_CLICK_ENABLED:
            if (error == WMError::WM_ERROR_INVALID_PARENT) return ERROR_NO_PARENT;
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_SUB;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            if (error == WMError::WM_DO_NOTHING) return ERROR_NOT_SHOWN;
            break;
        case WindowFocusApiType::SET_MAIN_WINDOW_RAISE_BY_CLICK_ENABLED:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_MAIN;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            if (error == WMError::WM_DO_NOTHING) return ERROR_NOT_SHOWN;
            break;
        case WindowFocusApiType::RAISE_ABOVE_TARGET:
            if (error == WMError::WM_ERROR_INVALID_PARENT) return ERROR_NO_PARENT;
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_SUB;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            if (error == WMError::WM_DO_NOTHING) return ERROR_SUB_OR_TARGET_NOT_SHOWN;
            break;
        case WindowFocusApiType::RAISE_MAIN_WINDOW_ABOVE_TARGET:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_MAIN_OR_TARGET_INVALID;
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_NOT_CREATED;
            break;
        case WindowFocusApiType::SET_EXCLUSIVELY_HIGHLIGHTED:
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_ONLY_NON_MODAL;
            break;
        case WindowFocusApiType::SET_WINDOW_DELAY_RAISE_ENABLED:
            if (error == WMError::WM_ERROR_INVALID_TYPE) return ERROR_TYPE_NOT_SUPPORT;
            break;
        case WindowFocusApiType::SHIFT_APP_WINDOW_FOCUS:
            if (error == WMError::WM_ERROR_INVALID_OPERATION) return ERROR_SOURCE_NOT_FOCUSED;
            if (error == WMError::WM_ERROR_INVALID_CALLING) return ERROR_NOT_SAME_APP;
            if (error == WMError::WM_DO_NOTHING) return ERROR_TARGET_ALREADY_FOCUSED;
            break;
        case WindowFocusApiType::GET_TOP_WINDOW:
            if (error == WMError::WM_ERROR_NULLPTR) return ERROR_CANNOT_FIND_TOP;
            break;
        case WindowFocusApiType::IS_FOCUSED:
            if (error == WMError::WM_ERROR_INVALID_WINDOW || error == WMError::WM_ERROR_NULLPTR) {
                return ERROR_NOT_CREATED;
            }
            break;
        case WindowFocusApiType::IS_WINDOW_HIGHLIGHTED:
            if (error == WMError::WM_ERROR_INVALID_WINDOW || error == WMError::WM_ERROR_NULLPTR) {
                return ERROR_NOT_CREATED;
            }
            break;
    }

    return "";
}

} // namespace Rosen
} // namespace OHOS