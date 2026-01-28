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

#ifndef SCREEN_EDID_PARSE_H
#define SCREEN_EDID_PARSE_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <dlfcn.h>
#include <unistd.h>

#include "window_manager_hilog.h"
#include "dm_common.h"
#include "transaction/rs_interfaces.h"

namespace OHOS {
namespace Rosen {
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string EDID_PARSE_SO_PATH = "/system/lib64/libedid_parse.z.so";
#else
const std::string EDID_PARSE_SO_PATH = "/system/lib/libedid_parse.z.so";
#endif

class RSInterfaces;

struct MultiScreenInfo {
    bool isExtendMain;
    bool outerOnly;
    MultiScreenMode multiScreenMode;
    MultiScreenPositionOptions mainScreenOption;
    MultiScreenPositionOptions secondaryScreenOption;
};

struct BaseEdid {
    std::string modelName_;
    std::string manufacturerName_;
    std::string displayProductName_;
    std::string asciiText_;
    std::string displayProductSerialNumber_;
    uint8_t edidMinor_;
    uint8_t weekOfManufactureOrModelYearFlag_;
    uint16_t yearOfManufactureOrModelYear_;
    uint16_t productCode_;
    uint32_t serialNumber_;
    int32_t hScreenSize_;  // Horizontal Screen Size in cm.
    int32_t vScreenSize_;
    uint8_t bitsPerPrimaryColor_;
    uint8_t videoInputType_;
    uint8_t gamma_;
    uint8_t feature_;
    uint8_t colorCharacteristics_[10];
    uint8_t extensionFlag_;
    uint8_t checkSum_;
};

using ParseEdidFunc = int32_t (*)(const uint8_t*, const uint32_t, struct BaseEdid*);

bool LoadEdidPlugin(void);
void UnloadEdidPlugin(void);
bool GetEdid(ScreenId rsScreenId, struct BaseEdid& edid);
int32_t GetEdidCheckCode(const std::vector<uint8_t>& edidData);
}
}
#endif /* SCREEN_EDID_PARSE_H */