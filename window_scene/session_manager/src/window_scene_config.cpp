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

#include "window_scene_config.h"

#include "config_policy_utils.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSceneConfig"};
}

WindowSceneConfig::ConfigItem WindowSceneConfig::config_;
const std::map<std::string, WindowSceneConfig::ValueType> WindowSceneConfig::configItemTypeMap_ = {
    { "maxAppWindowNumber",                           WindowSceneConfig::ValueType::INTS },
    { "modeChangeHotZones",                           WindowSceneConfig::ValueType::INTS },
    { "duration",                                     WindowSceneConfig::ValueType::INTS },
    { "defaultWindowMode",                            WindowSceneConfig::ValueType::INTS },
    { "dragFrameGravity",                             WindowSceneConfig::ValueType::INTS },
    { "floatingBottomPosY",                           WindowSceneConfig::ValueType::INTS },
    { "defaultFloatingWindow",                        WindowSceneConfig::ValueType::INTS },
    { "maxMainFloatingWindowNumber",                  WindowSceneConfig::ValueType::INTS },
    { "maxFloatingWindowSize",                        WindowSceneConfig::ValueType::INTS },
    { "defaultMaximizeMode",                          WindowSceneConfig::ValueType::INTS },
    { "miniWidth",                                    WindowSceneConfig::ValueType::INTS },
    { "miniHeight",                                   WindowSceneConfig::ValueType::INTS },
    { "showInLandscapeMode",                          WindowSceneConfig::ValueType::INTS },
    { "singleHandCompatibleMode",                     WindowSceneConfig::ValueType::MAP },
    { "mainWindowSizeLimits",                         WindowSceneConfig::ValueType::MAP },
    { "subWindowSizeLimits",                          WindowSceneConfig::ValueType::MAP },
    { "dialogWindowSizeLimits",                       WindowSceneConfig::ValueType::MAP },
    { "windowAnimation",                              WindowSceneConfig::ValueType::MAP },
    { "keyboardAnimation",                            WindowSceneConfig::ValueType::MAP },
    { "animationIn",                                  WindowSceneConfig::ValueType::MAP },
    { "animationOut",                                 WindowSceneConfig::ValueType::MAP },
    { "timing",                                       WindowSceneConfig::ValueType::MAP },
    { "windowEffect",                                 WindowSceneConfig::ValueType::MAP },
    { "appWindows",                                   WindowSceneConfig::ValueType::MAP },
    { "cornerRadius",                                 WindowSceneConfig::ValueType::MAP },
    { "shadow",                                       WindowSceneConfig::ValueType::MAP },
    { "shadowDark",                                   WindowSceneConfig::ValueType::MAP },
    { "focused",                                      WindowSceneConfig::ValueType::MAP },
    { "unfocused",                                    WindowSceneConfig::ValueType::MAP },
    { "decor",                                        WindowSceneConfig::ValueType::MAP },
    { "startWindowTransitionAnimation",               WindowSceneConfig::ValueType::MAP },
    { "systemUIStatusBar",                            WindowSceneConfig::ValueType::MAP },
    { "curve",                                        WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "splitRatios",                                  WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "exitSplitRatios",                              WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "scale",                                        WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "opacity",                                      WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "opacityStart",                                 WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "opacityEnd",                                   WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "elevation",                                    WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "alpha",                                        WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "singleHandScale",                              WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "heightChangeRatio",                            WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "widthChangeRatio",                             WindowSceneConfig::ValueType::POSITIVE_FLOATS },
    { "rotation",                                     WindowSceneConfig::ValueType::FLOATS },
    { "translate",                                    WindowSceneConfig::ValueType::FLOATS },
    { "offsetX",                                      WindowSceneConfig::ValueType::FLOATS },
    { "offsetY",                                      WindowSceneConfig::ValueType::FLOATS },
    { "radius",                                       WindowSceneConfig::ValueType::FLOATS },
    { "snapshotScale",                                WindowSceneConfig::ValueType::FLOATS },
    { "fullScreen",                                   WindowSceneConfig::ValueType::STRING },
    { "split",                                        WindowSceneConfig::ValueType::STRING },
    { "float",                                        WindowSceneConfig::ValueType::STRING },
    { "color",                                        WindowSceneConfig::ValueType::STRING },
    { "immersiveStatusBarBgColor",                    WindowSceneConfig::ValueType::STRING },
    { "immersiveStatusBarContentColor",               WindowSceneConfig::ValueType::STRING },
    { "supportedMode",                                WindowSceneConfig::ValueType::STRINGS },
    { "appsWithDeduplicatedWindowStatus",             WindowSceneConfig::ValueType::STRINGS },
    { "minimizeByOther",                              WindowSceneConfig::ValueType::UNDIFINED },
    { "stretchable",                                  WindowSceneConfig::ValueType::UNDIFINED },
    { "remoteAnimation",                              WindowSceneConfig::ValueType::UNDIFINED },
    { "configMainFloatingWindowAbove",                WindowSceneConfig::ValueType::UNDIFINED },
    { "backgroundswitch",                             WindowSceneConfig::ValueType::INTS },
    { "freeMultiWindow",                              WindowSceneConfig::ValueType::MAP },
    { "uiType",                                       WindowSceneConfig::ValueType::STRING },
    { "backgroundScreenLock",                         WindowSceneConfig::ValueType::STRING },
    { "rotationMode",                                 WindowSceneConfig::ValueType::STRING },
    { "immersive",                                    WindowSceneConfig::ValueType::MAP },
    { "inDesktopStatusBarConfig",                     WindowSceneConfig::ValueType::MAP },
    { "inSplitStatusBarConfig",                       WindowSceneConfig::ValueType::MAP },
    { "upDownSplit",                                  WindowSceneConfig::ValueType::MAP },
    { "leftRightSplit",                               WindowSceneConfig::ValueType::MAP },
    { "showHide",                                     WindowSceneConfig::ValueType::STRING },
    { "backgroundColor",                              WindowSceneConfig::ValueType::STRING },
    { "contentColor",                                 WindowSceneConfig::ValueType::STRING },
    { "supportTypeFloatWindow",                       WindowSceneConfig::ValueType::STRING },
    { "maxMidSceneNum",                               WindowSceneConfig::ValueType::INTS },
    { "defaultDragResizeType",                        WindowSceneConfig::ValueType::INTS },
    // ========================= Window Layout Config =========================
    { "windowLayout",                                 WindowSceneConfig::ValueType::MAP },
    { "moveDrag",                                     WindowSceneConfig::ValueType::MAP },
    { "moveResample",                                 WindowSceneConfig::ValueType::MAP },
    { "resampleFpsRange",                             WindowSceneConfig::ValueType::INTS },
    // ========================================================================
};

std::vector<std::string> WindowSceneConfig::SplitNodeContent(const xmlNodePtr& node, const std::string& pattern)
{
    xmlChar* content = xmlNodeGetContent(node);
    if (content == nullptr) {
        WLOGFE("read xml node error: nodeName:(%{public}s)", node->name);
        return std::vector<std::string>();
    }

    std::string contentStr = reinterpret_cast<const char*>(content);
    xmlFree(content);
    if (contentStr.size() == 0) {
        return std::vector<std::string>();
    }
    return WindowHelper::Split(contentStr, pattern);
}

std::string WindowSceneConfig::GetConfigPath(const std::string& configFileName)
{
    char buf[PATH_MAX + 1];
    char* configPath = GetOneCfgFile(configFileName.c_str(), buf, PATH_MAX + 1);
    char tmpPath[PATH_MAX + 1] = { 0 };
    if (!configPath || strlen(configPath) == 0 || strlen(configPath) > PATH_MAX || !realpath(configPath, tmpPath)) {
        WLOGI("can not get customization config file");
        return "/system/" + configFileName;
    }
    return std::string(tmpPath);
}

void WindowSceneConfig::ReadConfig(const xmlNodePtr& rootPtr, std::map<std::string, ConfigItem>& mapValue)
{
    for (xmlNodePtr curNodePtr = rootPtr->xmlChildrenNode; curNodePtr != nullptr; curNodePtr = curNodePtr->next) {
        if (!IsValidNode(*curNodePtr)) {
            WLOGFE("[WmConfig]: invalid node!");
            continue;
        }
        std::string nodeName = reinterpret_cast<const char*>(curNodePtr->name);
        if (configItemTypeMap_.count(nodeName)) {
            std::map<std::string, ConfigItem> p = ReadProperty(curNodePtr);
            if (p.size() > 0) {
                mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetProperty(p);
            }
            switch (configItemTypeMap_.at(nodeName)) {
                case ValueType::INTS: {
                    std::vector<int> v = ReadIntNumbersConfigInfo(curNodePtr);
                    mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetValue(v);
                    break;
                }
                case ValueType::POSITIVE_FLOATS: {
                    std::vector<float> v = ReadFloatNumbersConfigInfo(curNodePtr, false);
                    mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetValue(v);
                    break;
                }
                case ValueType::FLOATS: {
                    std::vector<float> v = ReadFloatNumbersConfigInfo(curNodePtr, true);
                    mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetValue(v);
                    break;
                }
                case ValueType::MAP: {
                    std::map<std::string, ConfigItem> v;
                    ReadConfig(curNodePtr, v);
                    mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetValue(v);
                    break;
                }
                case ValueType::STRING: {
                    std::string v = ReadStringConfigInfo(curNodePtr);
                    mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetValue(v);
                    break;
                }
                case ValueType::STRINGS: {
                    std::vector<std::string> v = ReadStringsConfigInfo(curNodePtr);
                    mapValue[reinterpret_cast<const char*>(curNodePtr->name)].SetValue(v);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

bool WindowSceneConfig::LoadConfigXml()
{
    auto configFilePath = GetConfigPath("etc/window/resources/window_manager_config.xml");
    xmlDocPtr docPtr = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        docPtr = xmlReadFile(configFilePath.c_str(), nullptr, XML_PARSE_NOBLANKS);
    }
    WLOGI("filePath: %{public}s", configFilePath.c_str());
    if (docPtr == nullptr) {
        WLOGFE("load xml error!");
        return false;
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        WLOGFE("get root element failed!");
        xmlFreeDoc(docPtr);
        return false;
    }

    std::map<std::string, ConfigItem> configMap;
    config_.SetValue(configMap);
    ReadConfig(rootPtr, *config_.mapValue_);

    xmlFreeDoc(docPtr);
    return true;
}

bool WindowSceneConfig::IsValidNode(const xmlNode& currNode)
{
    if (currNode.name == nullptr || currNode.type == XML_COMMENT_NODE) {
        return false;
    }
    return true;
}

std::map<std::string, XmlConfigBase::ConfigItem> WindowSceneConfig::ReadProperty(const xmlNodePtr& currNode)
{
    std::map<std::string, ConfigItem> property;
    xmlChar* prop = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("enable"));
    if (prop != nullptr) {
        if (!xmlStrcmp(prop, reinterpret_cast<const xmlChar*>("true"))) {
            property["enable"].SetValue(true);
        } else if (!xmlStrcmp(prop, reinterpret_cast<const xmlChar*>("false"))) {
            property["enable"].SetValue(false);
        }
        xmlFree(prop);
    }

    prop = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name"));
    if (prop != nullptr) {
        property["name"].SetValue(std::string(reinterpret_cast<const char*>(prop)));
        xmlFree(prop);
    }

    return property;
}

std::vector<int> WindowSceneConfig::ReadIntNumbersConfigInfo(const xmlNodePtr& currNode)
{
    std::vector<int> intsValue;
    auto numbers = SplitNodeContent(currNode);
    for (auto& num : numbers) {
        if (!WindowHelper::IsNumber(num)) {
            WLOGFE("read int number error: nodeName:(%{public}s)", currNode->name);
            return {};
        }
        intsValue.push_back(std::stoi(num));
    }
    return intsValue;
}

std::vector<std::string> WindowSceneConfig::ReadStringsConfigInfo(const xmlNodePtr& currNode)
{
    return SplitNodeContent(currNode);
}

std::vector<float> WindowSceneConfig::ReadFloatNumbersConfigInfo(const xmlNodePtr& currNode, bool allowNeg)
{
    std::vector<float> floatsValue;
    auto numbers = SplitNodeContent(currNode);
    for (auto& num : numbers) {
        if (!WindowHelper::IsFloatingNumber(num, allowNeg)) {
            WLOGFE("read float number error: nodeName:(%{public}s)", currNode->name);
            return {};
        }
        floatsValue.push_back(std::stof(num));
    }
    return floatsValue;
}

std::string WindowSceneConfig::ReadStringConfigInfo(const xmlNodePtr& currNode)
{
    std::string stringValue;
    xmlChar* context = xmlNodeGetContent(currNode);
    if (context == nullptr) {
        WLOGFE("read xml node error: nodeName:(%{public}s)", currNode->name);
        return {};
    }

    stringValue = std::string(reinterpret_cast<const char*>(context));
    xmlFree(context);
    return stringValue;
}

void WindowSceneConfig::DumpConfig(const std::map<std::string, ConfigItem>& config)
{
    for (auto& conf : config) {
        WLOGI("%{public}s", conf.first.c_str());
        std::map<std::string, ConfigItem> propMap;
        if (conf.second.property_) {
            propMap = *conf.second.property_;
        }
        for (auto prop : propMap) {
            switch (prop.second.type_) {
                case ValueType::BOOL:
                    WLOGI("Prop: %{public}s %{public}u", prop.first.c_str(), prop.second.boolValue_);
                    break;
                case ValueType::STRING:
                    WLOGI("Prop: %{public}s %{public}s", prop.first.c_str(),
                        prop.second.stringValue_.c_str());
                    break;
                default:
                    break;
            }
        }
        switch (conf.second.type_) {
            case ValueType::MAP:
                if (conf.second.mapValue_) {
                    DumpConfig(*conf.second.mapValue_);
                }
                break;
            case ValueType::BOOL:
                WLOGI("%{public}u", conf.second.boolValue_);
                break;
            case ValueType::STRING:
                WLOGI("%{public}s", conf.second.stringValue_.c_str());
                break;
            case ValueType::INTS:
                for (auto& num : *conf.second.intsValue_) {
                    WLOGI("Num: %{public}d", num);
                }
                break;
            case ValueType::FLOATS:
                for (auto& num : *conf.second.floatsValue_) {
                    WLOGI("Num: %{public}f", num);
                }
                break;
            default:
                break;
        }
    }
}

} // namespace Rosen
} // namespace OHOS
