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
constexpr size_t EDID_VIDEO_INPUT_DEFINITION_OFFSET = 0x14;
static constexpr uint8_t COLOR_DEPTH_BPC_CALC_SHIFT = 3;
static constexpr uint8_t COLOR_DEPTH_BPC_BASE_OFFSET = 4;
static constexpr uint8_t VIDEO_INPUT_DIGITAL_FLAG_MASK = 0x80;
static constexpr uint8_t COLOR_BIT_DEPTH_FIELD_MASK = 0x70;
static constexpr uint8_t COLOR_DEPTH_ENCODING_UNDEFINED = 0x00;
static constexpr uint8_t COLOR_DEPTH_ENCODING_RESERVED = 0x70;

// CEA-861 Extension Block Structure Constants
constexpr uint8_t EDID_VERSION_1_3 = 3;
static constexpr uint8_t EDID_NUM_EXT_OFFSET = 126;  // Offset to the extension block count field
static constexpr uint8_t CEA_TAG = 0x02;
static constexpr uint8_t CEA_BLOCK_TAG_OFFSET = 0;      // Block tag identifier offset
static constexpr uint8_t CEA_DTD_START_OFFSET = 2;      // DTD (Detailed Timing Descriptor) start offset
static constexpr uint8_t CEA_DATA_BLOCK_START_OFFSET = 4; // Data block collection start offset

// Data Block Header Parsing Constants (Byte 0: TTTT TLLL)
static constexpr uint8_t DATA_BLOCK_TAG_SHIFT = 5;
static constexpr uint8_t DATA_BLOCK_TAG_MASK = 0x07;   // Upper 3 bits: Tag code
static constexpr uint8_t DATA_BLOCK_LEN_MASK = 0x1F;   // Lower 5 bits: Payload length

// Data Block Tag Types
static constexpr uint8_t VENDOR_SPECIFIC_TAG = 0x03;

// HDMI VSDB (Vendor Specific Data Block) Structure Constants
static constexpr uint32_t HDMI_OUI = 0x000C03;
static constexpr uint8_t HDMI_VSDB_MIN_LEN = 6;        // Minimum valid length: OUI(3) + Physical Address(2) + Flags(1)
static constexpr uint8_t HDMI_VSDB_OUI_OFFSET = 1;     // OUI start offset relative to the data block header
static constexpr uint8_t HDMI_VSDB_FLAGS_OFFSET = 6;   // Deep Color flags byte offset relative to the data block header

// Deep Color Flag Bits (HDMI VSDB Byte 6)
static constexpr uint8_t DC_30bit = 0x10;  // bit 4: 10 bpc
static constexpr uint8_t DC_36bit = 0x20;  // bit 5: 12 bpc
static constexpr uint8_t DC_48bit = 0x40;  // bit 6: 16 bpc
static constexpr uint8_t BPC_8bit = 8;
static constexpr uint8_t BPC_10bit = 10;
static constexpr uint8_t BPC_12bit = 12;
static constexpr uint8_t BPC_16bit = 16;

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

uint8_t ExtractBpcFromFlags(uint8_t flags)
{
    if (flags & DC_48bit) {
        return BPC_16bit;
    }
    if (flags & DC_36bit) {
        return BPC_12bit;
    }
    if (flags & DC_30bit) {
        return BPC_10bit;
    }
    return BPC_8bit;
}

bool TryParseHdmiVsdb(const std::vector<uint8_t>& edid, size_t pos, uint8_t len, uint8_t& bpc)
{
    if (len < HDMI_VSDB_MIN_LEN) {
        return false;
    }
    
    uint32_t oui = edid[pos + HDMI_VSDB_OUI_OFFSET] |
        (static_cast<uint32_t>(edid[pos + HDMI_VSDB_OUI_OFFSET + 1]) << 8) |
        (static_cast<uint32_t>(edid[pos + HDMI_VSDB_OUI_OFFSET + 2]) << 16);
    if (oui == HDMI_OUI) {
        uint8_t flags = edid[pos + HDMI_VSDB_FLAGS_OFFSET];
        WLOGFI("Edid vsdb flag: 0x%{public}x, pos: 0x%{public}zx", flags, pos + HDMI_VSDB_FLAGS_OFFSET);
        bpc = ExtractBpcFromFlags(flags);
        return true;
    }
    return false;
}

bool ParseCeaDataBlocks(const std::vector<uint8_t>& edid, size_t base, uint8_t dtdStart, uint8_t& bpc)
{
    size_t pos = base + CEA_DATA_BLOCK_START_OFFSET;
    while (pos < base + dtdStart) {
        uint8_t tag = (edid[pos] >> DATA_BLOCK_TAG_SHIFT) & DATA_BLOCK_TAG_MASK;
        uint8_t len = edid[pos] & DATA_BLOCK_LEN_MASK;
        if (tag == VENDOR_SPECIFIC_TAG &&
            (pos + HDMI_VSDB_FLAGS_OFFSET) < edid.size() &&
            TryParseHdmiVsdb(edid, pos, len, bpc)) {
            return true;
        }
        pos += 1 + len;
    }
    return false;
}

bool ParseCeaExtensionBlock(const std::vector<uint8_t>& edid, size_t base, uint8_t& bpc)
{
    if (edid[base + CEA_BLOCK_TAG_OFFSET] != CEA_TAG) {
        return false;
    }
    uint8_t dtdStart = edid[base + CEA_DTD_START_OFFSET];
    if (dtdStart < CEA_DATA_BLOCK_START_OFFSET || dtdStart >= EDID_BLOCK_SIZE) {
        return false;
    }
    return ParseCeaDataBlocks(edid, base, dtdStart, bpc);
}

void ParseBpcFromCEABlock(const std::vector<uint8_t>& edid, const uint32_t edidSize, uint8_t& bpc)
{
    bpc = BPC_8bit;  // default

    if (edid.size() < EDID_BLOCK_SIZE || edidSize < EDID_BLOCK_SIZE) {
        return;
    }
    uint8_t numExt = edid[EDID_NUM_EXT_OFFSET];
    size_t totalSize = EDID_BLOCK_SIZE + static_cast<size_t>(numExt) * EDID_BLOCK_SIZE;
    if (numExt == 0 || edidSize < totalSize || edid.size() < totalSize) {
        return;
    }

    for (uint8_t ext = 0; ext < numExt; ++ext) {
        size_t base = EDID_BLOCK_SIZE + ext * EDID_BLOCK_SIZE;

        if (ParseCeaExtensionBlock(edid, base, bpc)) {
            return;
        }
    }
}

void ParseBpc(const std::vector<uint8_t>& edid, const uint32_t edidSize, uint8_t& bpc)
{
    constexpr size_t videoInputDefinitionOffset = EDID_VIDEO_INPUT_DEFINITION_OFFSET;
    if ((edid[videoInputDefinitionOffset] & VIDEO_INPUT_DIGITAL_FLAG_MASK) &&
        (edid[EDID_MINOR_OFFSET] >= BASE_MINOR)) {
        uint8_t colorBitDepth = edid[videoInputDefinitionOffset] & COLOR_BIT_DEPTH_FIELD_MASK;
        if (!(colorBitDepth == COLOR_DEPTH_ENCODING_UNDEFINED || colorBitDepth == COLOR_DEPTH_ENCODING_RESERVED)) {
            bpc = ((colorBitDepth >> COLOR_DEPTH_BPC_CALC_SHIFT) + COLOR_DEPTH_BPC_BASE_OFFSET);
        }
    } else if (edid[EDID_MINOR_OFFSET] == EDID_VERSION_1_3) {
        ParseBpcFromCEABlock(edid, edidSize, bpc);
    }
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
    uint8_t bpc = 0;
    ParseBpc(edid, edidSize, bpc);
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