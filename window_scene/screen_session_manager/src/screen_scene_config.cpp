/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "screen_scene_config.h"

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <libxml/globals.h>
#include <libxml/xmlstring.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "config_policy_utils.h"
#include "fold_screen_state_internel.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/utils/SkParsePath.h"
#include "screen_session_manager.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr uint32_t NO_WATERFALL_DISPLAY_COMPRESSION_SIZE = 0;
constexpr uint32_t DISPLAY_PHYSICAL_SIZE = 2;
constexpr uint32_t SCROLLABLE_PARAM_SIZE = 2;
enum XmlNodeElement {
    DPI = 0,
    SUB_DPI,
    IS_WATERFALL_DISPLAY,
    CURVED_SCREEN_BOUNDARY,
    CURVED_AREA_IN_LANDSCAPE,
    IS_CURVED_COMPRESS_ENABLED,
    IS_CONCURRENT_USER,
    BUILD_IN_DEFAULT_ORIENTATION,
    DEFAULT_DEVICE_ROTATION_OFFSET,
    DEFAULT_DISPLAY_CUTOUT_PATH,
    SUB_DISPLAY_CUTOUT_PATH,
    HALL_SWITCH_APP,
    PACKAGE_NAME,
    ROTATION_POLICY,
    DEFAULT_ROTATION_POLICY,
    SCREEN_SNAPSHOT_BUNDLE_NAME,
    SCREEN_SNAPSHOT_ABILITY_NAME,
    IS_RIGHT_POWER_BUTTON,
    SUPPORT_ROTATE_WITH_SCREEN,
    EXTERNAL_SCREEN_DEFAULT_MODE,
    CAST_BUNDLE_NAME,
    CAST_ABILITY_NAME,
    PHYSICAL_DISPLAY_RESOLUTION,
    SCROLLABLE_PARAM,
    IS_SUPPORT_CAPTURE,
    IS_SUPPORT_OFFSCREEN_RENDERING,
    OFF_SCREEN_PPI_THRESHOLD,
    PC_MODE_DPI,
    SUPPORT_DURING_CALL,
    HALF_FOLDED_MAX_THRESHOLD,
    CLOSE_HALF_FOLDED_MIN_THRESHOLD,
    OPEN_HALF_FOLDED_MIN_THRESHOLD,
    HALF_FOLDED_BUFFER,
    LARGER_BOUNDARY_FOR_THRESHOLD,
    POSTURE_SIZE,
    HALL_SIZE
};
}

std::map<std::string, bool> ScreenSceneConfig::enableConfig_;
std::map<std::string, std::vector<int>> ScreenSceneConfig::intNumbersConfig_;
std::map<std::string, std::string> ScreenSceneConfig::stringConfig_;
std::map<std::string, std::vector<std::string>> ScreenSceneConfig::stringListConfig_;
std::map<uint64_t, std::vector<DMRect>> ScreenSceneConfig::cutoutBoundaryRectMap_;
std::vector<DisplayPhysicalResolution> ScreenSceneConfig::displayPhysicalResolution_;
std::map<FoldDisplayMode, ScrollableParam> ScreenSceneConfig::scrollableParams_;
std::vector<DisplayConfig> ScreenSceneConfig::displaysConfigs_;
std::vector<DMRect> ScreenSceneConfig::subCutoutBoundaryRect_;
bool ScreenSceneConfig::isWaterfallDisplay_ = false;
bool ScreenSceneConfig::isSupportCapture_ = false;
bool ScreenSceneConfig::isScreenCompressionEnableInLandscape_ = false;
bool ScreenSceneConfig::isSupportOffScreenRendering_ = false;
bool ScreenSceneConfig::isConcurrentUser_ = false;
uint32_t ScreenSceneConfig::curvedAreaInLandscape_ = 0;
uint32_t ScreenSceneConfig::offScreenPPIThreshold_ = 0;
std::map<int32_t, std::string> ScreenSceneConfig::xmlNodeMap_ = {
    {DPI, "dpi"},
    {SUB_DPI, "subDpi"},
    {IS_WATERFALL_DISPLAY, "isWaterfallDisplay"},
    {IS_CONCURRENT_USER, "concurrentUser"},
    {CURVED_SCREEN_BOUNDARY, "curvedScreenBoundary"},
    {CURVED_AREA_IN_LANDSCAPE, "waterfallAreaCompressionSizeWhenHorzontal"},
    {IS_CURVED_COMPRESS_ENABLED, "isWaterfallAreaCompressionEnableWhenHorizontal"},
    {BUILD_IN_DEFAULT_ORIENTATION, "buildInDefaultOrientation"},
    {DEFAULT_DEVICE_ROTATION_OFFSET, "defaultDeviceRotationOffset"},
    {DEFAULT_DISPLAY_CUTOUT_PATH, "defaultDisplayCutoutPath"},
    {SUB_DISPLAY_CUTOUT_PATH, "subDisplayCutoutPath"},
    {HALL_SWITCH_APP, "hallSwitchApp"},
    {PACKAGE_NAME, "packageName"},
    {ROTATION_POLICY, "rotationPolicy"},
    {DEFAULT_ROTATION_POLICY, "defaultRotationPolicy"},
    {SCREEN_SNAPSHOT_BUNDLE_NAME, "screenSnapshotBundleName"},
    {SCREEN_SNAPSHOT_ABILITY_NAME, "screenSnapshotAbilityName"},
    {IS_RIGHT_POWER_BUTTON, "isRightPowerButton"},
    {SUPPORT_ROTATE_WITH_SCREEN, "supportRotateWithSensor"},
    {EXTERNAL_SCREEN_DEFAULT_MODE, "externalScreenDefaultMode"},
    {CAST_BUNDLE_NAME, "castBundleName"},
    {CAST_ABILITY_NAME, "castAbilityName"},
    {PHYSICAL_DISPLAY_RESOLUTION, "physicalDisplayResolution"},
    {SCROLLABLE_PARAM, "scrollableParam"},
    {IS_SUPPORT_CAPTURE, "isSupportCapture"},
    {IS_SUPPORT_OFFSCREEN_RENDERING, "isSupportOffScreenRendering"},
    {OFF_SCREEN_PPI_THRESHOLD, "offScreenPPIThreshold"},
    {PC_MODE_DPI, "pcModeDpi"},
    {SUPPORT_DURING_CALL, "supportDuringCall"},
    {HALF_FOLDED_MAX_THRESHOLD, "halfFoldMaxThreshold"},
    {CLOSE_HALF_FOLDED_MIN_THRESHOLD, "closeHalfFoldedMinThreshold"},
    {OPEN_HALF_FOLDED_MIN_THRESHOLD, "openHalfFoldedMinThreshold"},
    {HALF_FOLDED_BUFFER, "halfFoldedBuffer"},
    {LARGER_BOUNDARY_FOR_THRESHOLD, "largerBoundaryForThreshold"},
    {POSTURE_SIZE, "postureSize"},
    {HALL_SIZE, "hallSize"}
};

std::vector<std::string> ScreenSceneConfig::Split(std::string str, std::string pattern)
{
    std::vector<std::string> result;
    str += pattern;
    int32_t length = static_cast<int32_t>(str.size());
    for (int32_t i = 0; i < length; i++) {
        int32_t position = static_cast<int32_t>(str.find(pattern, i));
        if (position < length) {
            std::string tmp = str.substr(i, position - i);
            result.push_back(tmp);
            i = position + static_cast<int32_t>(pattern.size()) - 1;
        }
    }
    return result;
}

bool ScreenSceneConfig::IsNumber(std::string str)
{
    if (str.size() == 0) {
        return false;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(str.size()); i++) {
        if (str.at(i) < '0' || str.at(i) > '9') {
            return false;
        }
    }
    return true;
}

std::string ScreenSceneConfig::GetConfigPath(const std::string& configFileName)
{
    char buf[PATH_MAX + 1];
    char* configPath = GetOneCfgFile(configFileName.c_str(), buf, PATH_MAX + 1);
    char tmpPath[PATH_MAX + 1] = { 0 };
    if (!configPath || strlen(configPath) == 0 || strlen(configPath) > PATH_MAX || !realpath(configPath, tmpPath)) {
        TLOGI(WmsLogTag::DMS, "can not get customization config file");
        return "/system/" + configFileName;
    }
    TLOGI(WmsLogTag::DMS, "get customization config file success");
    return std::string(tmpPath);
}

bool ScreenSceneConfig::LoadConfigXml()
{
    auto configFilePath = GetConfigPath("etc/window/resources/display_manager_config.xml");
    xmlDocPtr docPtr = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        docPtr = xmlReadFile(configFilePath.c_str(), nullptr, XML_PARSE_NOBLANKS);
    }
    TLOGI(WmsLogTag::DMS, "filePath: %{public}s", configFilePath.c_str());
    if (docPtr == nullptr) {
        TLOGE(WmsLogTag::DMS, "load xml error!");
        return false;
    }
    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        TLOGE(WmsLogTag::DMS, "get root element failed!");
        xmlFreeDoc(docPtr);
        return false;
    }
    for (xmlNodePtr curNodePtr = rootPtr->xmlChildrenNode; curNodePtr != nullptr; curNodePtr = curNodePtr->next) {
        if (!IsValidNode(*curNodePtr)) {
            TLOGE(WmsLogTag::DMS, ":invalid node!");
            continue;
        }
        ParseNodeConfig(curNodePtr);
        if (xmlStrcmp(curNodePtr->name, reinterpret_cast<const xmlChar*>("displays")) != 0) {
            ParseNodeConfig(curNodePtr);
        } else {
            TLOGI(WmsLogTag::DMS, "find displays keyword, reading the nested content of XML");
            ParseDisplaysConfig(curNodePtr);
        }
    }
    xmlFreeDoc(docPtr);
    return true;
}

void ScreenSceneConfig::ParseNodeConfig(const xmlNodePtr& currNode)
{
    std::string nodeName(reinterpret_cast<const char*>(currNode->name));
    bool enableConfigCheck = (xmlNodeMap_[IS_WATERFALL_DISPLAY] == nodeName) ||
        (xmlNodeMap_[IS_CURVED_COMPRESS_ENABLED] == nodeName) ||
        (xmlNodeMap_[IS_RIGHT_POWER_BUTTON] == nodeName) ||
        (xmlNodeMap_[IS_SUPPORT_CAPTURE] == nodeName) ||
        (xmlNodeMap_[IS_CONCURRENT_USER] == nodeName) ||
        (xmlNodeMap_[SUPPORT_ROTATE_WITH_SCREEN] == nodeName)||
        (xmlNodeMap_[IS_SUPPORT_OFFSCREEN_RENDERING] == nodeName) ||
        (xmlNodeMap_[SUPPORT_DURING_CALL] == nodeName);
    bool numberConfigCheck = (xmlNodeMap_[DPI] == nodeName) ||
        (xmlNodeMap_[SUB_DPI] == nodeName) ||
        (xmlNodeMap_[CURVED_SCREEN_BOUNDARY] == nodeName) ||
        (xmlNodeMap_[CURVED_AREA_IN_LANDSCAPE] == nodeName) ||
        (xmlNodeMap_[BUILD_IN_DEFAULT_ORIENTATION] == nodeName) ||
        (xmlNodeMap_[DEFAULT_DEVICE_ROTATION_OFFSET] == nodeName) ||
        (xmlNodeMap_[OFF_SCREEN_PPI_THRESHOLD] == nodeName) ||
        (xmlNodeMap_[PC_MODE_DPI] == nodeName) ||
        (xmlNodeMap_[HALF_FOLDED_MAX_THRESHOLD] == nodeName) ||
        (xmlNodeMap_[CLOSE_HALF_FOLDED_MIN_THRESHOLD] == nodeName) ||
        (xmlNodeMap_[OPEN_HALF_FOLDED_MIN_THRESHOLD] == nodeName) ||
        (xmlNodeMap_[HALF_FOLDED_BUFFER] == nodeName) ||
        (xmlNodeMap_[LARGER_BOUNDARY_FOR_THRESHOLD] == nodeName) ||
        (xmlNodeMap_[POSTURE_SIZE] == nodeName) ||
        (xmlNodeMap_[HALL_SIZE] == nodeName);
    bool stringConfigCheck = (xmlNodeMap_[DEFAULT_DISPLAY_CUTOUT_PATH] == nodeName) ||
        (xmlNodeMap_[SUB_DISPLAY_CUTOUT_PATH] == nodeName) ||
        (xmlNodeMap_[ROTATION_POLICY] == nodeName) ||
        (xmlNodeMap_[DEFAULT_ROTATION_POLICY] == nodeName) ||
        (xmlNodeMap_[SCREEN_SNAPSHOT_BUNDLE_NAME] == nodeName) ||
        (xmlNodeMap_[SCREEN_SNAPSHOT_ABILITY_NAME] == nodeName) ||
        (xmlNodeMap_[EXTERNAL_SCREEN_DEFAULT_MODE] == nodeName) ||
        (xmlNodeMap_[CAST_BUNDLE_NAME] == nodeName) ||
        (xmlNodeMap_[CAST_ABILITY_NAME] == nodeName);
    if (enableConfigCheck) {
        ReadEnableConfigInfo(currNode);
    } else if (numberConfigCheck) {
        ReadIntNumbersConfigInfo(currNode);
    } else if (stringConfigCheck) {
        ReadStringConfigInfo(currNode);
    } else if (xmlNodeMap_[HALL_SWITCH_APP] == nodeName) {
        ReadStringListConfigInfo(currNode, nodeName);
    } else if (xmlNodeMap_[PHYSICAL_DISPLAY_RESOLUTION] == nodeName) {
        ReadPhysicalDisplayConfigInfo(currNode);
    } else if (xmlNodeMap_[SCROLLABLE_PARAM] == nodeName) {
        ReadScrollableParam(currNode);
    } else {
        TLOGI(WmsLogTag::DMS, "xml config node name is not match, nodeName:%{public}s", nodeName.c_str());
    }
}

uint64_t ScreenSceneConfig::ParseStrToUll(const std::string& contentStr)
{
    if (contentStr.empty()) {
        TLOGE(WmsLogTag::DMS, "Invalid value: %{public}s", contentStr.c_str());
        return 0;
    }
    uint64_t num;
    auto result = std::from_chars(contentStr.data(), contentStr.data() + contentStr.size(), num);
    if (result.ec == std::errc::invalid_argument) {
        TLOGE(WmsLogTag::DMS, "Invalid value: %{public}s", contentStr.c_str());
        return 0;
    } else if (result.ec == std::errc::result_out_of_range) {
        TLOGE(WmsLogTag::DMS, "Value out of range: %{public}s", contentStr.c_str());
        return 0;
    }
    return num;
}

void ScreenSceneConfig::ParseDisplaysConfig(const xmlNodePtr& currNode)
{
    for (xmlNodePtr displayNode = currNode->xmlChildrenNode; displayNode != nullptr; displayNode = displayNode->next) {
        if (!IsValidNode(*displayNode) ||
            xmlStrcmp(displayNode->name, reinterpret_cast<const xmlChar*>("display")) != 0) {
            continue;
        }
        DisplayConfig config;
        TLOGI(WmsLogTag::DMS, "start parse displays config");
        for (xmlNodePtr fileNode = displayNode->xmlChildrenNode; fileNode != nullptr; fileNode = fileNode->next) {
            if (!IsValidNode(*fileNode)) {
                continue;
            }
            std::string nodeName = reinterpret_cast<const char*>(fileNode->name);
            xmlChar* content = xmlNodeGetContent(fileNode);
            std::string contentStr;
            if (content) {
                contentStr = reinterpret_cast<const char*>(content);
                xmlFree(content);
                content = nullptr;
            }
            if (nodeName == "physicalId") {
                config.physicalId = static_cast<ScreenId>(ParseStrToUll(contentStr));
            } else if (nodeName == "logicalId") {
                config.logicalId = static_cast<ScreenId>(ParseStrToUll(contentStr));
            } else if (nodeName == "name") {
                config.name = contentStr;
            } else if (nodeName == "dpi") {
                config.dpi = atoi(contentStr.c_str());
            } else if (nodeName == "flags") {
                config.hasFlag = ParseFlagsConfig(fileNode, config.flag);
            }
        }
        displaysConfigs_.push_back(config);
    }
}

bool ScreenSceneConfig::ParseFlagsConfig(const xmlNodePtr& flagsNode, DisplayFlag& outFlag)
{
    bool foundFlag = false;
    for (xmlNodePtr flagNode = flagsNode->xmlChildrenNode; flagNode != nullptr; flagNode = flagNode->next) {
        if (!IsValidNode(*flagNode) || xmlStrcmp(flagNode->name, reinterpret_cast<const xmlChar*>("flag")) != 0) {
            continue;
        }
        xmlChar* typeAttr = xmlGetProp(flagNode, reinterpret_cast<const xmlChar*>("type"));
        xmlChar* valueAttr = xmlGetProp(flagNode, reinterpret_cast<const xmlChar*>("value"));
        if (typeAttr && valueAttr) {
            outFlag.type = reinterpret_cast<const char*>(typeAttr);
            outFlag.value = atoi(reinterpret_cast<const char*>(valueAttr));
            foundFlag = true;
            xmlFree(typeAttr);
            typeAttr = nullptr;
            xmlFree(valueAttr);
            valueAttr = nullptr;
            break;
        }
        xmlFree(typeAttr);
        typeAttr = nullptr;
        xmlFree(valueAttr);
        valueAttr = nullptr;
    }
    return foundFlag;
}

bool ScreenSceneConfig::IsValidNode(const xmlNode& currNode)
{
    if (currNode.name == nullptr || currNode.type == XML_COMMENT_NODE) {
        return false;
    }
    return true;
}

void ScreenSceneConfig::ReadIntNumbersConfigInfo(const xmlNodePtr& currNode)
{
    xmlChar* context = xmlNodeGetContent(currNode);
    if (context == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml node error: nodeName:(%{public}s)", currNode->name);
        return;
    }

    std::vector<int> numbersVec;
    std::string numbersStr = reinterpret_cast<const char*>(context);
    if (numbersStr.empty()) {
        xmlFree(context);
        return;
    }
    auto numbers = Split(numbersStr, " ");
    for (auto& num : numbers) {
        if (!IsNumber(num)) {
            TLOGE(WmsLogTag::DMS, "read number error: nodeName:(%{public}s)", currNode->name);
            xmlFree(context);
            return;
        }
        numbersVec.emplace_back(std::stoi(num));
    }

    std::string nodeName = reinterpret_cast<const char *>(currNode->name);
    intNumbersConfig_[nodeName] = numbersVec;
    xmlFree(context);
}

// LCOV_EXCL_START
void ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(const xmlNodePtr& currNode)
{
    xmlChar* displayMode = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("displayMode"));
    if (displayMode == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml node error: nodeName:(%{public}s)", currNode->name);
        return;
    }
    xmlChar* displayModeContext = xmlNodeGetContent(currNode);
    if (displayModeContext == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml nodeName:(%{public}s) context null", currNode->name);
        xmlFree(displayMode);
        return;
    }
    std::string displaySizeStr = reinterpret_cast<const char*>(displayModeContext);
    if (displaySizeStr.empty()) {
        xmlFree(displayModeContext);
        xmlFree(displayMode);
        return;
    }
    auto displaySizeArray = Split(displaySizeStr, ":");
    if (displaySizeArray.size() != DISPLAY_PHYSICAL_SIZE) {
        xmlFree(displayModeContext);
        xmlFree(displayMode);
        return;
    }
    DisplayPhysicalResolution physicalSize;
    if (!xmlStrcmp(displayMode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_FULL"))) {
        physicalSize.foldDisplayMode_ = FoldDisplayMode::FULL;
    } else if (!xmlStrcmp(displayMode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_MAIN"))) {
        physicalSize.foldDisplayMode_ = FoldDisplayMode::MAIN;
    } else if (!xmlStrcmp(displayMode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_SUB"))) {
        physicalSize.foldDisplayMode_ = FoldDisplayMode::SUB;
    } else if (!xmlStrcmp(displayMode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_GLOBAL_FULL"))) {
        physicalSize.foldDisplayMode_ = FoldDisplayMode::GLOBAL_FULL;
    } else {
        physicalSize.foldDisplayMode_ = FoldDisplayMode::UNKNOWN;
    }
    if (IsNumber(displaySizeArray[0]) && IsNumber(displaySizeArray[1])) {
        physicalSize.physicalWidth_ = static_cast<uint32_t>(std::stoi(displaySizeArray[0]));
        physicalSize.physicalHeight_ = static_cast<uint32_t>(std::stoi(displaySizeArray[1]));
    }
    displayPhysicalResolution_.emplace_back(physicalSize);
    xmlFree(displayModeContext);
    xmlFree(displayMode);
}
// LCOV_EXCL_STOP

std::vector<DisplayPhysicalResolution> ScreenSceneConfig::GetAllDisplayPhysicalConfig()
{
    return displayPhysicalResolution_;
}

// LCOV_EXCL_START
FoldDisplayMode ScreenSceneConfig::GetFoldDisplayMode(uint32_t width, uint32_t height)
{
    // Due to incorrect configuration files, custom processing is required.
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        return (width == height) ? FoldDisplayMode::MAIN : FoldDisplayMode::FULL;
    }

    for (const DisplayPhysicalResolution& resolution : displayPhysicalResolution_) {
        if ((resolution.physicalWidth_ == width && resolution.physicalHeight_ == height) ||
            (resolution.physicalWidth_ == height && resolution.physicalHeight_ == width)) {
            return resolution.foldDisplayMode_;
        }
    }
    return FoldDisplayMode::UNKNOWN;
}

void ScreenSceneConfig::ReadScrollableParam(const xmlNodePtr& currNode)
{
    xmlChar* displayModeXml = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("displayMode"));
    if (displayModeXml == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml node error: nodeName:(%{public}s)", currNode->name);
        return;
    }
    xmlChar* displayModeContext = xmlNodeGetContent(currNode);
    if (displayModeContext == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml nodeName:(%{public}s) context null", currNode->name);
        xmlFree(displayModeXml);
        return;
    }
    std::string scrollableParamStr = reinterpret_cast<const char*>(displayModeContext);
    if (scrollableParamStr.empty()) {
        xmlFree(displayModeContext);
        xmlFree(displayModeXml);
        return;
    }
    auto scrollableParamArray = Split(scrollableParamStr, ":");
    if (scrollableParamArray.size() != SCROLLABLE_PARAM_SIZE) {
        xmlFree(displayModeContext);
        xmlFree(displayModeXml);
        return;
    }
    FoldDisplayMode foldDisplayMode;
    if (!xmlStrcmp(displayModeXml, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_FULL"))) {
        foldDisplayMode = FoldDisplayMode::FULL;
    } else if (!xmlStrcmp(displayModeXml, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_MAIN"))) {
        foldDisplayMode = FoldDisplayMode::MAIN;
    } else if (!xmlStrcmp(displayModeXml, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_SUB"))) {
        foldDisplayMode = FoldDisplayMode::SUB;
    } else if (!xmlStrcmp(displayModeXml, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_COORDINATION"))) {
        foldDisplayMode = FoldDisplayMode::COORDINATION;
    } else {
        foldDisplayMode = FoldDisplayMode::UNKNOWN;
    }
    ScrollableParam scrollableParam;
    scrollableParam.velocityScale_ = scrollableParamArray[0];
    scrollableParam.friction_ = scrollableParamArray[1];
    scrollableParams_[foldDisplayMode] = scrollableParam;
    xmlFree(displayModeContext);
    xmlFree(displayModeXml);
}

std::map<FoldDisplayMode, ScrollableParam> ScreenSceneConfig::GetAllScrollableParam()
{
    return scrollableParams_;
}
// LCOV_EXCL_STOP

void ScreenSceneConfig::ReadEnableConfigInfo(const xmlNodePtr& currNode)
{
    xmlChar* enable = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("enable"));
    if (enable == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml node error: nodeName:(%{public}s)", currNode->name);
        return;
    }

    std::string nodeName = reinterpret_cast<const char *>(currNode->name);
    if (!xmlStrcmp(enable, reinterpret_cast<const xmlChar*>("true"))) {
        enableConfig_[nodeName] = true;
        if (xmlNodeMap_[IS_WATERFALL_DISPLAY] == nodeName) {
            isWaterfallDisplay_ = true;
        } else if (xmlNodeMap_[IS_CURVED_COMPRESS_ENABLED] == nodeName) {
            isScreenCompressionEnableInLandscape_ = true;
        } else if (xmlNodeMap_[IS_SUPPORT_CAPTURE] == nodeName) {
            isSupportCapture_ = true;
        } else if (xmlNodeMap_[IS_SUPPORT_OFFSCREEN_RENDERING] == nodeName) {
            isSupportOffScreenRendering_ = true;
        } else if (xmlNodeMap_[IS_CONCURRENT_USER] == nodeName) {
            isConcurrentUser_ = true;
        }
    } else {
        enableConfig_[nodeName] = false;
    }
    xmlFree(enable);
}

void ScreenSceneConfig::ReadStringConfigInfo(const xmlNodePtr& currNode)
{
    xmlChar* context = xmlNodeGetContent(currNode);
    if (context == nullptr) {
        TLOGE(WmsLogTag::DMS, "read xml node error: nodeName:(%{public}s)", currNode->name);
        return;
    }

    std::string inputString = reinterpret_cast<const char*>(context);
    std::string nodeName = reinterpret_cast<const char*>(currNode->name);
    stringConfig_[nodeName] = inputString;
    xmlFree(context);
}

// LCOV_EXCL_START
void ScreenSceneConfig::ReadStringListConfigInfo(const xmlNodePtr& rootNode, std::string name)
{
    if (rootNode == nullptr || rootNode->name == nullptr) {
        TLOGE(WmsLogTag::DMS, "get root element failed!");
        return;
    }
    xmlChar* rootContext = xmlNodeGetContent(rootNode);
    if (rootContext == nullptr) {
        TLOGE(WmsLogTag::DMS, "rootContext is null");
        return;
    }
    std::vector<std::string> stringVec;
    for (xmlNodePtr curNodePtr = rootNode->xmlChildrenNode; curNodePtr != nullptr; curNodePtr = curNodePtr->next) {
        if (!IsValidNode(*curNodePtr)) {
            TLOGE(WmsLogTag::DMS, "invalid node!");
            continue;
        }
        xmlChar* context = xmlNodeGetContent(curNodePtr);
        std::string str = reinterpret_cast<const char*>(context);
        stringVec.emplace_back(str);
        xmlFree(context);
    }
    stringListConfig_[name] = stringVec;
    xmlFree(rootContext);
}
// LCOV_EXCL_STOP

const std::map<std::string, bool>& ScreenSceneConfig::GetEnableConfig()
{
    return enableConfig_;
}

const std::map<std::string, std::vector<int>>& ScreenSceneConfig::GetIntNumbersConfig()
{
    return intNumbersConfig_;
}

const std::map<std::string, std::string>& ScreenSceneConfig::GetStringConfig()
{
    return stringConfig_;
}

const std::map<std::string, std::vector<std::string>>& ScreenSceneConfig::GetStringListConfig()
{
    return stringListConfig_;
}

const std::vector<DisplayConfig>& ScreenSceneConfig::GetDisplaysConfigs()
{
    return displaysConfigs_;
}

void ScreenSceneConfig::DumpConfig()
{
    for (auto& enable : enableConfig_) {
        TLOGI(WmsLogTag::DMS, "Enable: %{public}s %{public}u", enable.first.c_str(), enable.second);
    }
    for (auto& numbers : intNumbersConfig_) {
        TLOGI(WmsLogTag::DMS, "Numbers: %{public}s %{public}zu",
            numbers.first.c_str(), numbers.second.size());
        for (auto& num : numbers.second) {
            TLOGI(WmsLogTag::DMS, "Num: %{public}d", num);
        }
    }
    for (auto& string : stringConfig_) {
        TLOGI(WmsLogTag::DMS, "String: %{public}s", string.first.c_str());
    }
}

void ScreenSceneConfig::DumpDisplaysConfigs()
{
    for (const auto& iter : displaysConfigs_) {
        TLOGI(WmsLogTag::DMS, "name: %{public}s, physicalId: %{public}" PRIu64 ", logicalId: %{public}" PRIu64
              ", dpi: %{public}d", iter.name.c_str(), iter.physicalId, iter.logicalId, iter.dpi);
        if (iter.hasFlag) {
            TLOGI(WmsLogTag::DMS, "flag type: %{public}s, flag value: %{public}d",
                  iter.flag.type.c_str(), iter.flag.value);
        }
    }
}

// LCOV_EXCL_START
void ScreenSceneConfig::SetCutoutSvgPath(uint64_t displayId, const std::string& svgPath)
{
    if (svgPath.empty()) {
        return;
    }
    cutoutBoundaryRectMap_.clear();
    cutoutBoundaryRectMap_[displayId].emplace_back(CalcCutoutBoundaryRect(svgPath));
}

void ScreenSceneConfig::SetSubCutoutSvgPath(const std::string& svgPath)
{
    if (svgPath.empty()) {
        return;
    }
    subCutoutBoundaryRect_.clear();
    subCutoutBoundaryRect_.emplace_back(CalcCutoutBoundaryRect(svgPath));
}

DMRect ScreenSceneConfig::CalcCutoutBoundaryRect(std::string svgPath)
{
    DMRect emptyRect = { 0, 0, 0, 0 };
    SkPath skCutoutSvgPath;
    if (!SkParsePath::FromSVGString(svgPath.c_str(), &skCutoutSvgPath)) {
        TLOGE(WmsLogTag::DMS, "Parse svg string path failed.");
        return emptyRect;
    }
    SkRect skRect = skCutoutSvgPath.computeTightBounds();
    if (skRect.isEmpty()) {
        TLOGW(WmsLogTag::DMS, "Get empty skRect");
        return emptyRect;
    }
    SkIRect skiRect = skRect.roundOut();
    if (skiRect.isEmpty()) {
        TLOGW(WmsLogTag::DMS, "Get empty skiRect");
        return emptyRect;
    }
    int32_t left = static_cast<int32_t>(skiRect.left());
    int32_t top = static_cast<int32_t>(skiRect.top());
    uint32_t width = static_cast<uint32_t>(skiRect.width());
    uint32_t height = static_cast<uint32_t>(skiRect.height());
    TLOGI(WmsLogTag::DMS,
        "calc cutout boundary rect - left: [%{public}d top: %{public}d width: %{public}u height: %{public}u]",
        left, top, width, height);
    DMRect cutoutMinOuterRect = {
        .posX_ = left,
        .posY_ = top,
        .width_ = width,
        .height_ = height
    };
    return cutoutMinOuterRect;
}

std::vector<DMRect> ScreenSceneConfig::GetCutoutBoundaryRect(uint64_t displayId)
{
    if (cutoutBoundaryRectMap_.count(displayId) == 0) {
        return {};
    }
    return cutoutBoundaryRectMap_[displayId];
}

std::vector<DMRect> ScreenSceneConfig::GetSubCutoutBoundaryRect()
{
    return subCutoutBoundaryRect_;
}

bool ScreenSceneConfig::IsWaterfallDisplay()
{
    return isWaterfallDisplay_;
}

bool ScreenSceneConfig::IsSupportCapture()
{
    return isSupportCapture_;
}

void ScreenSceneConfig::SetCurvedCompressionAreaInLandscape()
{
    if (intNumbersConfig_[xmlNodeMap_[CURVED_AREA_IN_LANDSCAPE]].size() > 0) {
        curvedAreaInLandscape_ = static_cast<uint32_t>(intNumbersConfig_[xmlNodeMap_[CURVED_AREA_IN_LANDSCAPE]][0]);
    } else {
        TLOGW(WmsLogTag::DMS, "waterfallAreaCompressionSizeWhenHorzontal value is not exist");
    }
}

std::vector<int> ScreenSceneConfig::GetCurvedScreenBoundaryConfig()
{
    return intNumbersConfig_[xmlNodeMap_[CURVED_SCREEN_BOUNDARY]];
}

uint32_t ScreenSceneConfig::GetCurvedCompressionAreaInLandscape()
{
    if (!isWaterfallDisplay_ || !isScreenCompressionEnableInLandscape_) {
        TLOGW(WmsLogTag::DMS, "not waterfall screen or waterfall compression is not enabled");
        return NO_WATERFALL_DISPLAY_COMPRESSION_SIZE;
    }
    return curvedAreaInLandscape_;
}

bool ScreenSceneConfig::IsSupportRotateWithSensor()
{
    if (enableConfig_.count("supportRotateWithSensor") != 0) {
        return static_cast<bool>(enableConfig_["supportRotateWithSensor"]);
    }
    return false;
}
std::string ScreenSceneConfig::GetExternalScreenDefaultMode()
{
    if (stringConfig_.count("externalScreenDefaultMode") != 0) {
        return static_cast<std::string>(stringConfig_["externalScreenDefaultMode"]);
    }
    return "";
}

uint32_t ScreenSceneConfig::GetOffScreenPPIThreshold()
{
    if (intNumbersConfig_[xmlNodeMap_[OFF_SCREEN_PPI_THRESHOLD]].size() != 0) {
        return static_cast<uint32_t>(intNumbersConfig_[xmlNodeMap_[OFF_SCREEN_PPI_THRESHOLD]][0]);
    }
    return offScreenPPIThreshold_;
}

bool ScreenSceneConfig::IsSupportOffScreenRendering()
{
    if (enableConfig_.count("isSupportOffScreenRendering") != 0) {
        return static_cast<bool>(enableConfig_["isSupportOffScreenRendering"]);
    }
    return false;
}

bool ScreenSceneConfig::IsSupportDuringCall()
{
    if (enableConfig_.count("supportDuringCall") != 0) {
        return static_cast<bool>(enableConfig_["supportDuringCall"]);
    }
    return false;
}

bool ScreenSceneConfig::IsConcurrentUser()
{
    return isConcurrentUser_;
}
// LCOV_EXCL_STOP

uint32_t ScreenSceneConfig::GetNumberConfigValue(const std::string& name, const uint32_t& default_value)
{
    if (intNumbersConfig_.count(name) != 0) {
        return static_cast<uint32_t>(intNumbersConfig_[name][0]);
    }
    TLOGI(WmsLogTag::DMS, "default %{public}s = %{public}u", name.c_str(), default_value);
    return default_value;
}
} // namespace OHOS::Rosen
