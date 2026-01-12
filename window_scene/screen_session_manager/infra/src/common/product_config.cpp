/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <charconv>

#include "product_config.h"
#include "dm_common.h"
#include "fold_screen_state_internel.h"

namespace OHOS {
namespace Rosen {
namespace DMS {
static const std::string BOARDLIST_CCM = OHOS::system::GetParameter("const.product.buildin_screenid_list", "0");
WM_IMPLEMENT_SINGLE_INSTANCE(ProductConfig)

bool ProductConfig::IsDualDisplayFoldDevice()
{
    return FoldScreenStateInternel::IsDualDisplayFoldDevice();
}

bool ProductConfig::IsSingleDisplayFoldDevice()
{
    return FoldScreenStateInternel::IsSingleDisplayFoldDevice();
}

bool ProductConfig::IsSingleDisplayPocketFoldDevice()
{
    return FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice();
}

bool ProductConfig::IsSingleDisplaySuperFoldDevice()
{
    return FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice();
}

bool ProductConfig::IsSecondaryDisplayFoldDevice()
{
    return FoldScreenStateInternel::IsSecondaryDisplayFoldDevice();
}

std::vector<std::string> ProductConfig::StringSplit(const std::string& str, char delim)
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

std::vector<uint64_t> ProductConfig::ConvertToUint64Vector(const std::string& inputStr)
{
    std::vector<std::string> strList = StringSplit(inputStr, ',');
    std::vector<uint64_t> result;
    result.reserve(strList.size());

    // transfer string to uint64_t
    for (const auto& str : strList) {
        uint64_t num = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), num);
        if (ec == std::errc() && ptr == str.data() + str.size()) {
            result.push_back(num);
        }
    }
    return result.empty() ? std::vector<uint64_t>{0} : result;
}

bool ProductConfig::IsValidBoardList(const std::string& boardListStr)
{
    const std::regex REG_EXPR(R"(([1-9]\d*|0)(,([1-9]\d*|0))*)");
    return std::regex_match(boardListStr, REG_EXPR);
}

std::vector<uint64_t> ProductConfig::GetBoardList()
{
    std::lock_guard<std::mutex> lock(boardListMutex_);
    if (boardList_.empty()) {
        boardList_ = IsValidBoardList(BOARDLIST_CCM) ? ConvertToUint64Vector(BOARDLIST_CCM) : std::vector<uint64_t>{0};
    }
    return boardList_;
}
}  // namespace DMS
}  // namespace Rosen
}  // namespace OHOS