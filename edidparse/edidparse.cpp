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

#include "edidparse.h"
#include "window_manager_hilog.h"

constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, OHOS::Rosen::HILOG_DOMAIN_DISPLAY, "EdidParse"};
constexpr uint32_t EDID_BLOCK_SIZE = 128;
constexpr uint32_t EDID_MAX_BLOCK = 4;
constexpr uint32_t EDID_MAX_SIZE = EDID_BLOCK_SIZE * EDID_MAX_BLOCK;
constexpr uint32_t ONE_NUMBER_OF_BYTES = 8;
constexpr uint32_t TWO_NUMBER_OF_BYTES = 16;
constexpr uint32_t THREE_NUMBER_OF_BYTES = 24;
constexpr uint8_t EDID_VENDOR_START = 0x8;
constexpr uint8_t EDID_VENDOR_END = 0x11;
constexpr size_t BASE_YEAR = 1990;
constexpr size_t SN_SECOND_NUM = 2;
constexpr size_t SN_THIRD_NUM = 3;
constexpr uint8_t BASE_MINOR = 4;
constexpr size_t EDID_MINOR_OFFSET = 0x13;
constexpr size_t MANUFACTURE_WEEK_OFFSET = 0x10;
constexpr size_t MANUFACTURE_YEAR_OFFSET = 0x11;


template <size_t I>
char GetLetter(uint16_t id)
{
    static_assert(I < 3);
    const char letter = 'A' + (static_cast<uint8_t>(id >> ((2 - I) * 5)) & 0b00011111) - 1;
    return letter < 'A' || letter > 'Z' ? '\0' : letter;
}

static std::string GetManufacturerNameFromNum(uint16_t manufacturerId)
{
    std::string manufacturerName;
    constexpr uint32_t secondLetter = 2;
    manufacturerName.push_back(GetLetter<0>(manufacturerId));
    manufacturerName.push_back(GetLetter<1>(manufacturerId));
    manufacturerName.push_back(GetLetter<secondLetter>(manufacturerId));
    return manufacturerName;
}

static bool CheckEdidValid(const std::vector<uint8_t>& edid)
{
    if (edid.empty() || edid.size() % EDID_BLOCK_SIZE != 0) {
        WLOGFE("edid is invalid, size = %{public}u", static_cast<uint32_t>(edid.size()));
        return false;
    }
    const uint8_t magicStr[] = {0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0};
    return std::equal(std::begin(magicStr), std::end(magicStr), edid.begin());
}

static std::string ParseEdidText(const std::vector<uint8_t>& edid)
{
    std::string text(reinterpret_cast<const char*>(edid.data()), edid.size());
    text = text.substr(0, text.find('\n'));
    if (!std::all_of(text.begin(), text.end(), ::isprint)) {
        WLOGFW("edid text is empty");
        return {};
    }
    return text;
}

static void ParseDetailedTiming(const std::vector<uint8_t>& edid, struct baseEdid* outEdid)
{
    constexpr size_t detailedTimingDescriptorStart = 0x36;
    constexpr size_t detailedTimingDescriptorend = 0x6c;
    constexpr size_t eachDetailedTimingDescriptorSize = 0x12;
    constexpr size_t displayProductNameTag = 0xfc;
    constexpr size_t alphanumericDataStringTag = 0xfe;
    constexpr size_t displayProductSerialNumberTag = 0xff;
    constexpr size_t displayDescriptorTagOffset = 3;
    constexpr size_t displayDescriptorTagStart = 5;
    constexpr size_t displayDescriptorTagEnd = 18;
    for (size_t i = detailedTimingDescriptorStart; i <= detailedTimingDescriptorend;
        i += eachDetailedTimingDescriptorSize) {
        const size_t displayDescriptorTag = edid[i + displayDescriptorTagOffset];
        std::vector<uint8_t> descriptorVec;
        descriptorVec.assign(edid.begin() + i + displayDescriptorTagStart, edid.begin() + i + displayDescriptorTagEnd);
        switch (displayDescriptorTag) {
            case displayProductNameTag:
                outEdid->displayProductName = ParseEdidText(descriptorVec);
                break;
            case alphanumericDataStringTag:
                outEdid->asciiText = ParseEdidText(descriptorVec);
                break;
            case displayProductSerialNumberTag:
                outEdid->displayProductSerialNumber = ParseEdidText(descriptorVec);
                break;
            default:
                break;
        }
    }
    outEdid->modelName = outEdid->displayProductName;
    if (outEdid->modelName.empty()) {
        WLOGFW("display product name is empty");
        outEdid->modelName = outEdid->displayProductSerialNumber;
    }
    if (outEdid->modelName.empty()) {
        WLOGFW("display product serial number is empty");
        outEdid->modelName = outEdid->asciiText;
    }
}

static bool CheckParamsValid(const uint8_t* edidData, const uint32_t edidSize, BaseEdid* outEdid)
{
    if (!edidData || !outEdid) {
        WLOGFE("edid is nullptr or outEdid i nullptr");
        return false;
    }
    WLOGFW("edid size is %{public}u", edidSize);
    if (edidSize == 0 || edidSize % EDID_BLOCK_SIZE != 0 || edidSize > EDID_MAX_SIZE) {
        return false;
    }
    return true;
}

extern "C" {
int ParseBaseEdid(const uint8_t* edidData, const uint32_t edidSize, struct baseEdid* outEdid)
{
    if (!CheckParamsValid(edidData, edidSize, outEdid)) {
        return -1;
    }

    std::vector<uint8_t> edid(edidData, edidData + edidSize);
    if (!CheckEdidValid(edid)) {
        return -1;
    }

    uint32_t vendorDataSum = 0;
    for (uint32_t i = EDID_VENDOR_START; i <= EDID_VENDOR_END; i++) {
        vendorDataSum += edid[i];
    }
    WLOGFW("parseBaseEdid vendorDataSum is %{public}u", vendorDataSum);

    // get the edid version
    outEdid->edid_minor = edid[EDID_MINOR_OFFSET];

    // get the manufacturer name
    constexpr size_t manufacturerNameOffset = 0x8;
    outEdid->manufacturerName =
            GetManufacturerNameFromNum(static_cast<uint16_t>((edid[manufacturerNameOffset] << ONE_NUMBER_OF_BYTES) |
            edid[manufacturerNameOffset + 1]));

    // get the product code
    constexpr size_t productIdOffset = 0xa;
    outEdid->productCode = static_cast<uint16_t>(edid[productIdOffset] |
        (edid[productIdOffset + 1] << ONE_NUMBER_OF_BYTES));

    //get the serial number
    constexpr size_t serialNumberOffset = 0xc;

    outEdid->serialNumber = static_cast<uint32_t>(edid[serialNumberOffset] |
        (edid[serialNumberOffset + 1] << ONE_NUMBER_OF_BYTES) |
        (edid[serialNumberOffset + SN_SECOND_NUM] << TWO_NUMBER_OF_BYTES) |
        (edid[serialNumberOffset + SN_THIRD_NUM] << THREE_NUMBER_OF_BYTES));

    // get the manufacture week and year
    outEdid->weekOfManufactureOrModelYearFlag = edid[MANUFACTURE_WEEK_OFFSET];
    outEdid->yearOfManufactureOrModelYear = edid[MANUFACTURE_YEAR_OFFSET] + BASE_YEAR;

    // get the bpc
    constexpr size_t videoInputDefinitionOffset = 0x14;
    uint8_t bpc = 0;
    constexpr uint8_t bpcHighNum = 3;
    constexpr uint8_t bpcNumOffSet = 4;
    if ((edid[videoInputDefinitionOffset] & 0x80) && (outEdid->edid_minor >= BASE_MINOR)) {
        uint8_t colorBitDepth = edid[videoInputDefinitionOffset] & 0x70;
        if (!(colorBitDepth == 0x00 || colorBitDepth == 0x70)) {
            bpc = ((colorBitDepth >> bpcHighNum) + bpcNumOffSet);
        }
    }
    outEdid->bitsPerPrimaryColor = bpc;

    // get the screen size
    constexpr size_t HorizontalSceenSizeOffset = 0x15;
    constexpr size_t VerticalSceenSizeOffset = 0x16;
    if (edid[HorizontalSceenSizeOffset] != 0 && edid[VerticalSceenSizeOffset] != 0) {
        outEdid->hScreenSize = edid[HorizontalSceenSizeOffset];
        outEdid->vScreenSize = edid[VerticalSceenSizeOffset];
    }

    // get the detailed timing
    ParseDetailedTiming(edid, outEdid);
    WLOGFW("parseBaseEdid edid_minor is %{public}u, ScreenSize is %{public}u cm * %{public}u cm, bpc is %{public}u",
        outEdid->edid_minor, outEdid->hScreenSize, outEdid->vScreenSize, outEdid->bitsPerPrimaryColor);
    return 0;
}
}