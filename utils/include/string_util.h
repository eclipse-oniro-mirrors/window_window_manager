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
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <set>
#include <sstream>
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

    static inline bool ConvertStringToFloat(const std::string& str, float& num)
    {
        char* end = nullptr;
        errno = 0;
        num = std::strtof(str.c_str(), &end);
        if (end == str.c_str() || errno == ERANGE || !std::isfinite(num)) {
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

    /**
     * @brief Join a set of values into a delimiter-separated string.
     *
     * @tparam T Value type. Must support operator<< for std::ostream.
     * @param values Set of values to join.
     * @param delimiter Delimiter used between values.
     * @return Joined string representation of the value set.
     */
    template<typename T>
    static std::string JoinValueSet(const std::set<T>& values, char delimiter = ' ');

    /**
     * @brief Parse a delimiter-separated string into a value set.
     *
     * Empty tokens and invalid values are ignored.
     *
     * @tparam T Value type. Must support operator>> for std::istream.
     * @param str Input string to parse.
     * @param delimiter Delimiter used to split values.
     * @return Parsed value set.
     */
    template<typename T>
    static std::set<T> ParseValueSet(const std::string& str, char delimiter = ' ');
};

template<typename T>
std::string StringUtil::JoinValueSet(const std::set<T>& values, char delimiter)
{
    std::ostringstream oss;

    for (auto iter = values.begin(); iter != values.end(); ++iter) {
        if (iter != values.begin()) {
            oss << delimiter;
        }
        oss << *iter;
    }

    return oss.str();
}

template<typename T>
std::set<T> StringUtil::ParseValueSet(const std::string& str, char delimiter)
{
    std::istringstream stream(str);
    std::set<T> values;
    std::string token;

    while (std::getline(stream, token, delimiter)) {
        if (token.empty()) {
            continue;
        }

        std::istringstream tokenStream(token);

        T value {};
        tokenStream >> value;

        tokenStream >> std::ws;
        if (!tokenStream.eof()) {
            continue;
        }

        values.insert(value);
    }

    return values;
}
} // namespace Rosen
} // namespace OHOS
#endif // WM_STRING_UTIL_H
