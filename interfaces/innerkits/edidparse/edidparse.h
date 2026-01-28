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

#ifndef OHOS_ROSEN_EDIDPARSE_H
#define OHOS_ROSEN_EDIDPARSE_H
#include <vector>
#include <string>
 
#define COLOR_CHARACTER_MAX 10
 
typedef struct baseEdid {
    std::string modelName;
    std::string manufacturerName;
    std::string displayProductName;
    std::string asciiText;
    std::string displayProductSerialNumber;
    uint8_t edid_minor;
    uint8_t weekOfManufactureOrModelYearFlag;
    uint16_t yearOfManufactureOrModelYear;
    uint16_t productCode;
    uint32_t serialNumber;
    int32_t hScreenSize; // Horizontal Screen Size in cm.
    int32_t vScreenSize;
    uint8_t bitsPerPrimaryColor;
    uint8_t videoInputType;
    uint8_t gamma;
    uint8_t feature;
    uint8_t colorCharacteristics[COLOR_CHARACTER_MAX];
    uint8_t extensionFlag;
    uint8_t checkSum;
} BaseEdid;
 
// Making API functions visible to callers
extern "C" {
/**
 * @brief  Obtain the edid parse result based on the input edid information.
 *
 * @param  edid - (input) orginal edid value
 * @param  outEdid - (output) the dp infomation parsed through edid.
 *
 * @return Integer indicating whether the parse is successful, 0: Succeeded, -1: Failed
 */
int ParseBaseEdid(const uint8_t* edidData, const uint32_t edidSize, BaseEdid* outEdid);
}
#endif