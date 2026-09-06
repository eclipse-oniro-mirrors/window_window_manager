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

#ifndef WINDOW_WINDOW_MANAGER_FLOAT_WINDOW_ERROR_MSG_H
#define WINDOW_WINDOW_MANAGER_FLOAT_WINDOW_ERROR_MSG_H

#include <string>
#include <unordered_map>
#include <map>
#include "wm_common.h"

namespace OHOS::Rosen {

enum class FloatWindowModule : int32_t {
    PIP = 0,
    FLOATING_BALL = 1,
    FLOAT_VIEW = 2,
    END = 3,
};

struct ApiErrorMsgInfo {
    std::string shortMessage;
    std::string fullMessage;
};

class FloatWindowErrorMsg {
public:
    static std::string GetApiErrorMsg(FloatWindowModule module, const std::string& methodName,
                                       WmErrorCode errorCode, const std::string& customMessage = "");

private:
    static const std::unordered_map<WmErrorCode, ApiErrorMsgInfo> GLOBAL_DEFAULT_MSGS;
    static const std::map<FloatWindowModule, std::unordered_map<WmErrorCode, ApiErrorMsgInfo>> MODULE_DEFAULT_MSGS;
    
    static std::string BuildErrorMsg(const std::string& module, const std::string& methodName,
                                      const std::string& shortMsg, const std::string& customMessage);
    static std::string GetMsgFromMap(const std::unordered_map<WmErrorCode, ApiErrorMsgInfo>& moduleMap,
                                      WmErrorCode errorCode, const std::string& customMessage);
    static std::string GetModuleName(FloatWindowModule module);
};

} // namespace OHOS::Rosen

#endif // WINDOW_WINDOW_MANAGER_FLOAT_WINDOW_ERROR_MSG_H