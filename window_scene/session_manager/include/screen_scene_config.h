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

#ifndef OHOS_ROSEN_SCREEN_SCENE_CONFIG_H
#define OHOS_ROSEN_SCREEN_SCENE_CONFIG_H


#include <refbase.h>

#include "cutout_info.h"
#include "xml_config_base.h"
#include "libxml/parser.h"

namespace OHOS::Rosen {
class ScreenSceneConfig : public RefBase, public XmlConfigBase {
public:
    ScreenSceneConfig() = delete;
    ~ScreenSceneConfig() = default;

    static bool LoadConfigXml();
    static const std::map<std::string, bool>& GetEnableConfig();
    static const std::map<std::string, std::vector<int>>& GetIntNumbersConfig();
    static const std::map<std::string, std::string>& GetStringConfig();
    static const std::map<std::string, std::vector<std::string>>& GetStringListConfig();
    static void DumpConfig();
    static std::vector<DMRect> GetCutoutBoundaryRect(uint64_t displayId);
    static std::vector<DMRect> GetSubCutoutBoundaryRect();
    static void SetCutoutSvgPath(uint64_t displayId, const std::string& svgPath);
    static void SetSubCutoutSvgPath(const std::string& svgPath);
    static bool IsWaterfallDisplay();
    static void SetCurvedCompressionAreaInLandscape();
    static std::vector<int> GetCurvedScreenBoundaryConfig();
    static uint32_t GetCurvedCompressionAreaInLandscape();
    static bool IsSupportRotateWithSensor();
    static std::string GetExternalScreenDefaultMode();
    static std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalConfig();
    static bool IsSupportCapture();
    static std::map<FoldDisplayMode, ScrollableParam> GetAllScrollableParam();

private:
    static std::map<int32_t, std::string> xmlNodeMap_;
    static std::map<std::string, bool> enableConfig_;
    static std::map<std::string, std::vector<int>> intNumbersConfig_;
    static std::map<std::string, std::string> stringConfig_;
    static std::map<uint64_t, std::vector<DMRect>> cutoutBoundaryRectMap_;
    static std::map<std::string, std::vector<std::string>> stringListConfig_;
    static std::vector<DMRect> subCutoutBoundaryRect_;
    static bool isWaterfallDisplay_;
    static bool isScreenCompressionEnableInLandscape_;
    static uint32_t curvedAreaInLandscape_;
    static std::vector<DisplayPhysicalResolution> displayPhysicalResolution_;
    static bool isSupportCapture_;
    static std::map<FoldDisplayMode, ScrollableParam> scrollableParams_;

    static bool IsValidNode(const xmlNode& currNode);
    static void ReadEnableConfigInfo(const xmlNodePtr& currNode);
    static void ReadIntNumbersConfigInfo(const xmlNodePtr& currNode);
    static void ReadStringConfigInfo(const xmlNodePtr& currNode);
    static void ReadStringListConfigInfo(const xmlNodePtr& currNode, std::string name);
    static std::string GetConfigPath(const std::string& configFileName);
    static void ParseNodeConfig(const xmlNodePtr& currNode);

    static std::vector<std::string> Split(std::string str, std::string pattern);
    static bool IsNumber(std::string str);
    static DMRect CalcCutoutBoundaryRect(const std::string svgPath);
    static void ReadPhysicalDisplayConfigInfo(const xmlNodePtr& currNode);
    static void ReadScrollableParam(const xmlNodePtr& currNode);
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_SCREEN_SCENE_CONFIG_H
