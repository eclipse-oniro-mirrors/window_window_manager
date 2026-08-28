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

#ifndef WINDOW_WINDOW_MANAGER_UTILS_INCLUDE_WINDOW_FOCUS_ERROR_MSG_HELPER_H
#define WINDOW_WINDOW_MANAGER_UTILS_INCLUDE_WINDOW_FOCUS_ERROR_MSG_HELPER_H

#include <string>
#include "wm_common.h"
#include "window_error_msg.h"

namespace OHOS {
namespace Rosen {

enum class WindowFocusApiType : uint32_t {
    SET_FOCUSABLE,
    SET_WINDOW_FOCUSABLE,
    SET_SUB_WINDOW_MODAL,
    SET_TOPMOST,
    SET_WINDOW_TOPMOST,
    RAISE_TO_APP_TOP,
    SET_SUB_WINDOW_Z_LEVEL,
    GET_SUB_WINDOW_Z_LEVEL,
    SET_WINDOW_MODAL,
    SET_RAISE_BY_CLICK_ENABLED,
    SET_MAIN_WINDOW_RAISE_BY_CLICK_ENABLED,
    RAISE_ABOVE_TARGET,
    RAISE_MAIN_WINDOW_ABOVE_TARGET,
    SET_EXCLUSIVELY_HIGHLIGHTED,
    SET_WINDOW_DELAY_RAISE_ENABLED,
    SHIFT_APP_WINDOW_FOCUS,
    GET_TOP_WINDOW,
    IS_FOCUSED,
    IS_WINDOW_HIGHLIGHTED,
};

class WindowFocusErrorMsgHelper {
public:
    static std::string GetErrorMsg(WindowFocusApiType apiType, WMError error, const std::string& customErrMsg = "");

private:
    static bool IsFixedGeneralError(WMError error)
    {
        return error == WMError::WM_ERROR_DEVICE_NOT_SUPPORT ||
               error == WMError::WM_ERROR_INVALID_PERMISSION ||
               error == WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    static const char* GetApiName(WindowFocusApiType apiType);
    static const char* GetSpecificErrorMsg(WindowFocusApiType apiType, WMError error);
    static const char* GetFocusableErrorMsg(WMError error);
    static const char* GetSubWindowModalErrorMsg(WMError error);
    static const char* GetTopmostErrorMsg(WMError error);
    static const char* GetRaiseToAppTopErrorMsg(WMError error);
    static const char* GetSubWindowZLevelErrorMsg(WMError error);
    static const char* GetSubWindowZLevelQueryErrorMsg(WMError error);
    static const char* GetWindowModalErrorMsg(WMError error);
    static const char* GetRaiseByClickErrorMsg(WMError error);
    static const char* GetMainWindowRaiseByClickErrorMsg(WMError error);
    static const char* GetRaiseAboveTargetErrorMsg(WMError error);
    static const char* GetRaiseMainWindowAboveTargetErrorMsg(WMError error);
    static const char* GetExclusivelyHighlightedErrorMsg(WMError error);
    static const char* GetWindowDelayRaiseErrorMsg(WMError error);
    static const char* GetShiftAppWindowFocusErrorMsg(WMError error);
    static const char* GetTopWindowErrorMsg(WMError error);
    static const char* GetFocusQueryErrorMsg(WMError error);
};

} // namespace Rosen
} // namespace OHOS

#endif // WINDOW_WINDOW_MANAGER_UTILS_INCLUDE_WINDOW_FOCUS_ERROR_MSG_HELPER_H