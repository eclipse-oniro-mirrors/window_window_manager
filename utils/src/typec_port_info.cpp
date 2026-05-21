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

#include "typec_port_info.h"

#include <dirent.h>
#include <fstream>
#include <string>
#include <sys/types.h>

namespace OHOS::Rosen {
static std::vector<std::string> GetDirList()
{
    DIR* dir = opendir("sys/class/thermal");
    if (dir == nullptr) {
        return {};
    }

    std::vector<std::string> allPath;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("thermal_zone") == 0) {
            allPath.emplace_back("sys/class/thermal/" + name);
        }
    }
    closedir(dir);
    return allPath;
}

static bool IsStringNumeric(const char* buffer)
{
    if (buffer == nullptr || *buffer == '\0') {
        return false;
    }
    auto begin = buffer;
    if (*begin == '-') {
        begin++;
        if (*begin == '\0') {
            return false;
        }
    }

    while (*begin) {
        if (!std::isdigit(*begin)) {
            return false;
        }
        begin++;
    }
    return true;
}

static bool GetTypeCFileNode(const std::vector<std::string>& allDirName, std::string& typeCMappingPath)
{
    for (const auto& dir : allDirName) {
        std::string absoluteTypePath = dir + "/type";
        char realPathResult[PATH_MAX] = { 0 };
        if (!realpath(absoluteTypePath.c_str(), realPathResult)) {
            return false;
        }

        std::ifstream srcFile(realPathResult, std::ifstream::in);
        if (!srcFile.is_open()) {
            continue;
        }
        constexpr int bufferSize = 20;
        char buffer[bufferSize] = {0};
        srcFile.getline(buffer, bufferSize);
        if (strcmp(buffer, "usb_port") == 0) {
            typeCMappingPath = dir + "/temp";
            return true;
        }
    }
    return false;
}

bool TypeCPortInfo::GetTypeCThermal(int32_t& thermal)
{
    static std::string typeCMappingPath;
    if (typeCMappingPath.empty()) {
        std::vector<std::string> allDirName = GetDirList();
        if (allDirName.empty()) {
            return false;
        }
        if (!GetTypeCFileNode(allDirName, typeCMappingPath)) {
            return false;
        }
    }

    char realPathResult[PATH_MAX] = { 0 };
    if (!realpath(typeCMappingPath.c_str(), realPathResult)) {
        return false;
    }

    std::ifstream typeCThermalFile(realPathResult, std::ifstream::in);
    if (!typeCThermalFile.is_open()) {
        return false;
    }
    constexpr int bufferSize = 20;
    char buffer[bufferSize] = {0};
    typeCThermalFile.getline(buffer, bufferSize);
    if (!IsStringNumeric(buffer)) {
        return false;
    }
    thermal = std::stoi(buffer);
    return true;
}
} // namespace OHOS::Rosen