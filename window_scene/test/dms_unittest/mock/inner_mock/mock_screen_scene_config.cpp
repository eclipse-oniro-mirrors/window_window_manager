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

#include "screen_scene_config.h"

namespace OHOS::Rosen {
namespace {
constexpr uint32_t NO_EARTERFALL_DISPLAY_COMPRESSION_SIZE = 0;
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
    HALL_SIZE,
    WAIT_COORDINATION_READY_MAX_TIME,
};
}  // namespace

std::map<std::string, bool> ScreenSceneConfig::enableConfig_;
std::map<std::string, std::vector<int>> ScreenSceneConfig::intNumbersConfig_;
std::map<std::string, std::string> ScreenSceneConfig::stringConfig_;
std::map<std::string, std::vector<std::string>> ScreenSceneConfig::stringListConfig_;
std::map<uint64_t, std::vector<DMRect>> ScreenSceneConfig::defaultCutoutBoundaryRectMap_;
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
    {HALL_SIZE, "hallSize"},
    {WAIT_COORDINATION_READY_MAX_TIME, "waitCoordinationReadyMaxTime"}};

std::vector<std::string> ScreenSceneConfig::Split(std::string str, std::string pattern)
{
    return {};
}

bool ScreenSceneConfig::IsNumber(std::string str)
{
    return true;
}

std::string ScreenSceneConfig::GetConfigPath(const std::string& configFileName)
{
    return "";
}

bool ScreenSceneConfig::LoadConfigXml()
{
    return true;
}

void ScreenSceneConfig::ParseNodeConfig(const xmlNodePtr& currNode) {}

uint64_t ScreenSceneConfig::ParseStrToUll(const std::string& contentStr)
{
    return 0;
}

void ScreenSceneConfig::ParseDisplaysConfig(const xmlNodePtr& currNode) {}

bool ScreenSceneConfig::ParseFlagsConfig(const xmlNodePtr& flagsNode, DisplayFlag& outFlag)
{
    return true;
}

bool ScreenSceneConfig::IsValidNode(const xmlNode& currNode)
{
    return true;
}

void ScreenSceneConfig::ReadIntNumbersConfigInfo(const xmlNodePtr& currNode) {}

// LCOV_EXCL_START
void ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(const xmlNodePtr& currNode) {}
// LCOV_EXCL_STOP

std::vector<DisplayPhysicalResolution> ScreenSceneConfig::GetAllDisplayPhysicalConfig()
{
    return displayPhysicalResolution_;
}

// LCOV_EXCL_START
FoldDisplayMode ScreenSceneConfig::GetFoldDisplayMode(uint32_t width, uint32_t height)
{
    return FoldDisplayMode::UNKNOWN;
}

void ScreenSceneConfig::ReadScrollableParam(const xmlNodePtr& currNode) {}

std::map<FoldDisplayMode, ScrollableParam> ScreenSceneConfig::GetAllScrollableParam()
{
    return scrollableParams_;
}
// LCOV_EXCL_STOP

void ScreenSceneConfig::ReadEnableConfigInfo(const xmlNodePtr& currNode) {}

void ScreenSceneConfig::ReadStringConfigInfo(const xmlNodePtr& currNode) {}

// LCOV_EXCL_START
void ScreenSceneConfig::ReadStringListConfigInfo(const xmlNodePtr& rootNode, std::string name) {}
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

void ScreenSceneConfig::DumpConfig() {}

void ScreenSceneConfig::DumpDisplaysConfigs() {}

// LCOV_EXCL_START
void ScreenSceneConfig::SetCutoutSvgPath(uint64_t displayId, const std::string& svgPath)
{
    if (svgPath.empty()) {
        return;
    }
    defaultCutoutBoundaryRectMap_.clear();
    defaultCutoutBoundaryRectMap_[displayId].emplace_back(CalcCutoutBoundaryRect(svgPath));
    cutoutBoundaryRectMap_.clear();
    cutoutBoundaryRectMap_[displayId].emplace_back(CalcCutoutBoundaryRect(svgPath));
}

void ScreenSceneConfig::UpdateCutoutBoundRect(uint64_t displayId, float rogRatio)
{
    if (defaultCutoutBoundaryRectMap_.count(displayId) == 0 || cutoutBoundaryRectMap_.count(displayId) == 0) {
        return;
    }
    for (uint64_t i = 0; i < cutoutBoundaryRectMap_[displayId].size(); i++) {
        cutoutBoundaryRectMap_[displayId][i].posX_ = defaultCutoutBoundaryRectMap_[displayId][i].posX_ * rogRatio;
        cutoutBoundaryRectMap_[displayId][i].posY_ = defaultCutoutBoundaryRectMap_[displayId][i].posY_ * rogRatio;
        cutoutBoundaryRectMap_[displayId][i].width_ = defaultCutoutBoundaryRectMap_[displayId][i].width_ * rogRatio;
        cutoutBoundaryRectMap_[displayId][i].height_ = defaultCutoutBoundaryRectMap_[displayId][i].height_ * rogRatio;
    }
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

void ScreenSceneConfig::SetCurvedCompressionAreaInLandscape() {}

std::vector<int> ScreenSceneConfig::GetCurvedScreenBoundaryConfig()
{
    return intNumbersConfig_[xmlNodeMap_[CURVED_SCREEN_BOUNDARY]];
}

uint32_t ScreenSceneConfig::GetCurvedCompressionAreaInLandscape()
{
    if (!isWaterfallDisplay_ || !isScreenCompressionEnableInLandscape_) {
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
    return default_value;
}

}  // namespace OHOS::Rosen