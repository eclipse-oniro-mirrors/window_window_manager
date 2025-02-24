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
static const std::string PHY_ROTATION_OFFSET = system::GetParameter("const.window.phyrotation.offset", "0");
static const std::string INVALID_DEVICE = "-1";
static const std::string PORTRAIT_DEVICE = "0";
static const std::string DEFAULT_OFFSET = "0";
static const std::string SINGLE_DISPLAY = "1";
static const std::string DUAL_DISPLAY = "2";
static const std::string SINGLE_POCKET_DISPLAY = "4";
static const std::string SECONDARY_FOLD_DISPLAY = "6";
static const size_t THIRD_ANGLE = 2;
}
class FoldScreenStateInternel {
public:
    static std::string getFoldType()
    {
        if (!IsValidFoldType(g_foldScreenType)) {
            return INVALID_DEVICE;
        }
        std::vector<std::string> foldTypes = StringSplit(g_foldScreenType, ',');
        if (foldTypes.empty()) {
            return INVALID_DEVICE;
        }
        return foldTypes[0];
    }

    static bool IsFoldScreenDevice()
    {
        std::string foldType = getFoldType();
        return foldType != INVALID_DEVICE && foldType != PORTRAIT_DEVICE;
    }

    // is two logic screen device
    static bool IsDualDisplayFoldDevice()
    {
        return getFoldType() == DUAL_DISPLAY;
    }

    // only one logic screen device
    static bool IsSingleDisplayFoldDevice()
    {
        // ALTB ccm property conflict with the chip, waiting for chip conflict resolution
        return !IsDualDisplayFoldDevice() && !IsSingleDisplayPocketFoldDevice() && !IsSecondaryDisplayFoldDevice();
    }

    static bool IsSingleDisplayPocketFoldDevice()
    {
        if (!IsValidFoldType(g_foldScreenType)) {
            return false;
        }
        std::vector<std::string> foldTypes = StringSplit(g_foldScreenType, ',');
        if (foldTypes.empty()) {
            return false;
        }
        return foldTypes[0] == SINGLE_POCKET_DISPLAY;
    }

    static std::vector<std::string> GetPhyRotationOffset()
    {
        static std::vector<std::string> phyOffsets;
        if (phyOffsets.empty()) {
            std::vector<std::string> elems = StringSplit(PHY_ROTATION_OFFSET, ';');
            for (auto& num : elems) {
                if (IsNumber(num)) {
                    phyOffsets.push_back(num);
                } else {
                    phyOffsets.push_back(DEFAULT_OFFSET);
                }
            }
        }
        return phyOffsets;
    }

    static bool IsNumber(const std::string& str)
    {
        int32_t length = static_cast<int32_t>(str.size());
        if (length == 0) {
            return false;
        }
        for (int32_t i = 0; i < length; i++) {
            if (str.at(i) < '0' || str.at(i) > '9') {
                return false;
            }
        }
        return true;
    }

    static bool IsOuterScreen(FoldDisplayMode foldDisplayMode)
    {
        if (IsDualDisplayFoldDevice()) {
            return foldDisplayMode == FoldDisplayMode::SUB;
        }
        if (IsSingleDisplayFoldDevice() || IsSingleDisplayPocketFoldDevice()) {
            return foldDisplayMode == FoldDisplayMode::MAIN;
        }
        return false;
    }

    static bool IsSecondaryDisplayFoldDevice()
    {
        return getFoldType() == SECONDARY_FOLD_DISPLAY;
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

    template<typename T>
    static std::string TransVec2Str(const std::vector<T> &vec, const std::string &name)
    {
        std::stringstream strs;
        for (uint32_t i = 0; i < vec.size(); i++) {
            auto str = vec[i];
            strs << name;
            if (i == 0) {
                strs << "_bc";
            } else if (i == 1) {
                strs << "_ab";
            } else if (i == THIRD_ANGLE) {
                strs << "_ab_anti";
            }
            strs << ": ";
            strs << std::to_string(str) << " ";
        }
        return strs.str();
    }
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_FOLD_SCREEN_STATE_INTERNEL_H