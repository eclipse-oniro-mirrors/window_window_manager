/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WM_STRING_UTIL_H
#define WM_STRING_UTIL_H

#include <charconv>
#include <string>

#include "dm_common.h"

namespace OHOS {
namespace Rosen {

class StringUtil {
public:
    static std::string Trim(std::string s);

    template<typename T>
    static constexpr bool CanToString()
    {
        if (std::is_same<T, uint64_t>::value ||
            std::is_same<T, int64_t>::value ||
            std::is_same<T, uint32_t>::value ||
            std::is_same<T, int32_t>::value ||
            std::is_same<T, bool>::value) {
            return true;
        }
        return false;
    }

    template<typename T>
    static bool VectorToString(const std::vector<T>& vec, std::string& str)
    {
        if (!CanToString<T>()) {
            str.clear();
            return false;
        }
        std::stringstream oss;
        oss << "[ ";
        for (auto item : vec) {
            oss <<  item << " ";
        }
        oss << "]";
        str = oss.str();
        return true;
    }

    static inline bool ConvertStringToInt32(const std::string& str, int32_t& num)
    {
        auto res = std::from_chars(str.data(), str.data() + str.size(), num);
        if (res.ec != std::errc()) {
            return false;
        }
        return true;
    }

    static inline bool ConvertStringToBool(const std::string& str)
    {
        std::string tmp = str;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        if (tmp == "true" || tmp == "1") {
            return true;
        }
        return false;
    }
};
} // Rosen
} // OHOS
#endif // WM_BUNDLE_MGR_UTIL_H