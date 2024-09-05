/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_FOLD_SCREEN_STATE_INTERNEL_H
#define OHOS_ROSEN_FOLD_SCREEN_STATE_INTERNEL_H

#include <sstream>
#include <parameters.h>
#include <regex>

namespace OHOS {
namespace Rosen {
namespace {
    static const std::string g_foldScreenType = system::GetParameter("const.window.foldscreen.type", "0,0,0,0");
    static const  std::string SINGLE_DISPLAY = "1";
    static const std::string DUAL_DISPLAY = "2";
}
class FoldScreenStateInternel {
public:
    static bool IsFoldScreenDevice()
    {
        return g_foldScreenType != "";
    }

    static bool IsDualDisplayFoldDevice()
    {
        if (!IsValidFoldType(g_foldScreenType)) {
            return false;
        }
        std::vector<std::string> foldTypes = StringSplit(g_foldScreenType, ',');
        if (foldTypes.empty()) {
            return false;
        }
        return foldTypes[0] == DUAL_DISPLAY;
    }

    static bool IsSingleDisplayFoldDevice()
    {
        if (!IsValidFoldType(g_foldScreenType)) {
            return false;
        }
        std::vector<std::string> foldTypes = StringSplit(g_foldScreenType, ',');
        if (foldTypes.empty()) {
            return false;
        }
        return foldTypes[0] == SINGLE_DISPLAY;
    }

    static std::vector<std::string> StringSplit(const std::string& str, char delim)
    {
        std::size_t previous = 0;
        std::size_t current = str.find(delim);
        std::vector<std::string> elems;
        while (current != std::string::npos) {
            if (current > previous) {
                elems.push_back(str.substr(previous, current - previous));
            }
            previous = current + 1;
            current = str.find(delim, previous);
        }
        if (previous != str.size()) {
            elems.push_back(str.substr(previous));
        }
        return elems;
    }

    static bool IsValidFoldType(const std::string& foldTypeStr)
    {
        std::regex reg("^([0-9],){3}[0-9]{1}$");
        return std::regex_match(foldTypeStr, reg);
    }
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_FOLD_SCREEN_STATE_INTERNEL_H