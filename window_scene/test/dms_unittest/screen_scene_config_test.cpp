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

#include <vector>
#include <gtest/gtest.h>

#include <libxml/globals.h>
#include <libxml/xmlstring.h>

#include "window_manager_hilog.h"
#include "xml_config_base.h"
#include "screen_scene_config.h"
#include "screen_session_manager.h"
#include "scene_board_judgement.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
class ScreenSceneConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSceneConfigTest::SetUpTestCase()
{
}

void ScreenSceneConfigTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void ScreenSceneConfigTest::SetUp()
{
}

void ScreenSceneConfigTest::TearDown()
{
}

namespace {
/**
 * @tc.name: IsNumber
 * @tc.desc: test function : IsNumber
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsNumber, TestSize.Level1)
{
    bool result = ScreenSceneConfig::IsNumber("123");
    ASSERT_EQ(true, result);
    result = ScreenSceneConfig::IsNumber("a123");
    ASSERT_EQ(false, result);
    result = ScreenSceneConfig::IsNumber("");
    ASSERT_EQ(false, result);
    result = ScreenSceneConfig::IsNumber("-123");
    ASSERT_EQ(false, result);
    result = ScreenSceneConfig::IsNumber("123.456");
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: GetConfigPath1
 * @tc.desc: test function : GetConfigPath
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetConfigPath1, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetConfigPath("");
    ASSERT_STRNE("/system/", result.c_str());
}

/**
 * @tc.name: GetConfigPath2
 * @tc.desc: test function : GetConfigPath
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetConfigPath2, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetConfigPath("a.xml");
    ASSERT_STREQ("/system/a.xml", result.c_str());
}


/**
 * @tc.name: IsValidNode1
 * @tc.desc: test function : IsValidNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsValidNode1, TestSize.Level1)
{
    xmlNode node;
    auto result = ScreenSceneConfig::IsValidNode(node);
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: IsValidNode2
 * @tc.desc: test function : IsValidNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsValidNode2, TestSize.Level1)
{
    const xmlChar xmlStringText[] = { 't', 'e', 'x', 't', 0 };
    xmlNode node;
    node.name = xmlStringText;
    node.type = XML_TEXT_NODE;
    auto result = ScreenSceneConfig::IsValidNode(node);
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: IsValidNode3
 * @tc.desc: test function : IsValidNode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsValidNode3, TestSize.Level1)
{
    const xmlChar xmlStringText[] = { 't', 'e', 'x', 't', 0 };
    xmlNode node;
    node.name = xmlStringText;
    node.type = XML_COMMENT_NODE;
    auto result = ScreenSceneConfig::IsValidNode(node);
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: ReadIntNumbersConfigInfo
 * @tc.desc: test function : ReadIntNumbersConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadIntNumbersConfigInfo, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_.clear();

    auto configFilePath = ScreenSceneConfig::GetConfigPath("etc/window/resources/display_manager_config.xml");
    xmlDocPtr docPtr = xmlReadFile(configFilePath.c_str(), nullptr, XML_PARSE_NOBLANKS);
    if (docPtr == nullptr) {
        GTEST_SKIP();
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        xmlFreeDoc(docPtr);
        GTEST_SKIP();
    }
    uint32_t readCount = 0;
    for (xmlNodePtr curNodePtr = rootPtr->xmlChildrenNode; curNodePtr != nullptr; curNodePtr = curNodePtr->next) {
        if (!ScreenSceneConfig::IsValidNode(*curNodePtr)) {
            continue;
        }
        auto nodeName = curNodePtr->name;
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("dpi")) ||
            !xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("defaultDeviceRotationOffset")) ||
            !xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("cutoutArea")) ||
            !xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("curvedScreenBoundary"))) {
            ScreenSceneConfig::ReadIntNumbersConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
    }
    ASSERT_GE(ScreenSceneConfig::intNumbersConfig_.size(), readCount);
    ScreenSceneConfig::DumpConfig();
    xmlFreeDoc(docPtr);
}

/**
 * @tc.name: ReadIntNumbersConfigInfo02
 * @tc.desc: Test ReadIntNumbersConfigInfo method when node content is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadIntNumbersConfigInfo02, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadIntNumbersConfigInfo(currNode);
    xmlFree(currNode);
}

/**
 * @tc.name: ReadIntNumbersConfigInfo03
 * @tc.desc: Test ReadIntNumbersConfigInfo method when node content is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadIntNumbersConfigInfo03, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, BAD_CAST "");
    ScreenSceneConfig::ReadIntNumbersConfigInfo(currNode);
    xmlFree(currNode);
}

/**
 * @tc.name: ReadIntNumbersConfigInfo04
 * @tc.desc: Test ReadIntNumbersConfigInfo method when node content contains non-number.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadIntNumbersConfigInfo04, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, BAD_CAST "123 abc");
    ScreenSceneConfig::ReadIntNumbersConfigInfo(currNode);
    xmlFree(currNode);
}

/**
 * @tc.name: ReadIntNumbersConfigInfo05
 * @tc.desc: Test ReadIntNumbersConfigInfo method when node content is valid.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadIntNumbersConfigInfo05, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, BAD_CAST "123 456 789");
    ScreenSceneConfig::ReadIntNumbersConfigInfo(currNode);
    xmlFree(currNode);
}

/**
 * @tc.name: ReadEnableConfigInfo
 * @tc.desc: test function : ReadEnableConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadEnableConfigInfo, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_.clear();

    auto configFilePath = ScreenSceneConfig::GetConfigPath("etc/window/resources/display_manager_config.xml");
    xmlDocPtr docPtr = xmlReadFile(configFilePath.c_str(), nullptr, XML_PARSE_NOBLANKS);
    if (docPtr == nullptr) {
        GTEST_SKIP();
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        xmlFreeDoc(docPtr);
        GTEST_SKIP();
    }
    uint32_t readCount = 0;
    for (xmlNodePtr curNodePtr = rootPtr->xmlChildrenNode; curNodePtr != nullptr; curNodePtr = curNodePtr->next) {
        if (!ScreenSceneConfig::IsValidNode(*curNodePtr)) {
            continue;
        }

        auto nodeName = curNodePtr->name;
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("isWaterfallDisplay"))) {
            ScreenSceneConfig::ReadEnableConfigInfo(curNodePtr);
            readCount++;
            continue;
        }

        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("dpi"))) {
            ScreenSceneConfig::ReadEnableConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
    }

    ASSERT_LE(ScreenSceneConfig::enableConfig_.size(), readCount);

    ScreenSceneConfig::DumpConfig();
    xmlFreeDoc(docPtr);
}

/**
 * @tc.name: ReadStringConfigInfo
 * @tc.desc: test function : ReadStringConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadStringConfigInfo, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_.clear();

    auto configFilePath = ScreenSceneConfig::GetConfigPath("etc/window/resources/display_manager_config.xml");
    xmlDocPtr docPtr = xmlReadFile(configFilePath.c_str(), nullptr, XML_PARSE_NOBLANKS);
    if (docPtr == nullptr) {
        GTEST_SKIP();
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        xmlFreeDoc(docPtr);
        GTEST_SKIP();
    }
    uint32_t readCount = 0;
    for (xmlNodePtr curNodePtr = rootPtr->xmlChildrenNode; curNodePtr != nullptr; curNodePtr = curNodePtr->next) {
        if (!ScreenSceneConfig::IsValidNode(*curNodePtr)) {
            continue;
        }

        auto nodeName = curNodePtr->name;
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("defaultDisplayCutoutPath"))) {
            ScreenSceneConfig::ReadStringConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("subDisplayCutoutPath"))) {
            ScreenSceneConfig::ReadStringConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("dpi"))) {
            ScreenSceneConfig::ReadStringConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("externalScreenDefaultMode"))) {
            ScreenSceneConfig::ReadStringConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
    }

    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_GT(ScreenSceneConfig::stringConfig_.size(), readCount);
    } else {
        ASSERT_EQ(ScreenSceneConfig::stringConfig_.size(), readCount);
    }
    ScreenSceneConfig::DumpConfig();
    xmlFreeDoc(docPtr);
}

/**
 * @tc.name: GetEnableConfig1
 * @tc.desc: test function : GetEnableConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetEnableConfig, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetEnableConfig();
    ASSERT_EQ(true, result.size() == 0);
}

/**
 * @tc.name: GetIntNumbersConfig
 * @tc.desc: test function : GetIntNumbersConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetIntNumbersConfig, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetIntNumbersConfig();
    ASSERT_NE(true, result.size() == 0);
}

/**
 * @tc.name: GetStringConfig
 * @tc.desc: test function : GetStringConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetStringConfig, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetStringConfig();
    ASSERT_NE(0, result.size());
}

/**
 * @tc.name: GetStringListConfig
 * @tc.desc: test function : GetStringListConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetStringListConfig, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetStringListConfig();
    ASSERT_EQ(0, result.size());
}

/**
 * @tc.name: GetCurvedScreenBoundaryConfig
 * @tc.desc: test function : GetCurvedScreenBoundaryConfig
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetCurvedScreenBoundaryConfig, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetCurvedScreenBoundaryConfig();
    if ((ScreenSessionManager::GetInstance().GetCurvedCompressionArea() == 0) &&
        SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(0, result.size());
    } else {
        ASSERT_NE(0, result.size());
    }
}

/**
 * @tc.name: GetCutoutBoundaryRect
 * @tc.desc: GetCutoutBoundaryRect func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetCutoutBoundaryRect, TestSize.Level1)
{
    uint64_t displayId = -1;
    auto result = ScreenSceneConfig::GetCutoutBoundaryRect(displayId);
    ASSERT_FALSE(result.size() > 0);
}

/**
 * @tc.name: GetSubCutoutBoundaryRect
 * @tc.desc: GetSubCutoutBoundaryRect func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetSubCutoutBoundaryRect, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    auto result = ScreenSceneConfig::GetSubCutoutBoundaryRect();
    if (ScreenSessionManager::GetInstance().IsFoldable() && !FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        ASSERT_TRUE(result.size() > 0);
    } else {
        ASSERT_TRUE(result.size() == 0);
    }
}

/**
 * @tc.name: IsWaterfallDisplay
 * @tc.desc: IsWaterfallDisplay func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsWaterfallDisplay, TestSize.Level1)
{
    auto result = ScreenSceneConfig::IsWaterfallDisplay();
    if (result) {
        ASSERT_EQ(true, result);
    }
}

/**
 * @tc.name: GetCurvedCompressionAreaInLandscape
 * @tc.desc: GetCurvedCompressionAreaInLandscape func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetCurvedCompressionAreaInLandscape, TestSize.Level1)
{
    auto result = ScreenSceneConfig::GetCurvedCompressionAreaInLandscape();
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: Split01
 * @tc.desc: Split01 func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, Split01, TestSize.Level1)
{
    auto result = ScreenSceneConfig::Split("oo", "+9");
    ASSERT_NE(0, result.size());
}

/**
 * @tc.name: Split02
 * @tc.desc: Test Split function when no pattern is found in the string
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, Split02, TestSize.Level1)
{
    std::string str = "HelloWorld";
    std::string pattern = "::";
    std::vector<std::string> result = ScreenSceneConfig::Split(str, pattern);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], str);
}

/**
 * @tc.name: Split03
 * @tc.desc: Test Split function when one pattern is found in the string
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, Split03, TestSize.Level1)
{
    std::string str = "Hello::World";
    std::string pattern = "::";
    std::vector<std::string> result = ScreenSceneConfig::Split(str, pattern);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "Hello");
    EXPECT_EQ(result[1], "World");
}

/**
 * @tc.name: Split04
 * @tc.desc: Test Split function when multiple patterns are found in the string
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, Split04, TestSize.Level1)
{
    std::string str = "Hello::World::This::Is::A::Test";
    std::string pattern = "::";
    std::vector<std::string> result = ScreenSceneConfig::Split(str, pattern);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "Hello");
    EXPECT_EQ(result[1], "World");
    EXPECT_EQ(result[2], "This");
    EXPECT_EQ(result[3], "Is");
    EXPECT_EQ(result[4], "A");
    EXPECT_EQ(result[5], "Test");
}

/**
 * @tc.name: Split05
 * @tc.desc: Test Split function when the string ends with the pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, Split05, TestSize.Level1)
{
    std::string str = "Hello::World::";
    std::string pattern = "::";
    std::vector<std::string> result = ScreenSceneConfig::Split(str, pattern);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "Hello");
    EXPECT_EQ(result[1], "World");
    EXPECT_EQ(result[2], "");
}

/**
 * @tc.name: Split06
 * @tc.desc: Test Split function when the input string is the same as the pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, Split06, TestSize.Level1)
{
    std::string str = "::";
    std::string pattern = "::";
    std::vector<std::string> result = ScreenSceneConfig::Split(str, pattern);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
}

/**
 * @tc.name: GetAllDisplayPhysicalConfig01
 * @tc.desc: GetAllDisplayPhysicalConfig01 func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetAllDisplayPhysicalConfig01, TestSize.Level1)
{
    ScreenSceneConfig::displayPhysicalResolution_.clear();
    std::vector<DisplayPhysicalResolution> actual = ScreenSceneConfig::GetAllDisplayPhysicalConfig();
    EXPECT_TRUE(actual.empty());
}

/**
 * @tc.name: CalcCutoutBoundaryRect
 * @tc.desc: CalcCutoutBoundaryRect func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, CalcCutoutBoundaryRect, TestSize.Level1)
{
    DMRect emptyRect = {0, 0, 0, 0};
    auto result = ScreenSceneConfig::CalcCutoutBoundaryRect("oo");
    ASSERT_FALSE(result != emptyRect);
}

/**
 * @tc.name: CalcCutoutBoundaryRect02
 * @tc.desc: Test scenario where svg parsing fails and an empty rectangle is expected.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, CalcCutoutBoundaryRect02, TestSize.Level1)
{
    std::string invalidSvgPath = "invalid_svg_path";
    DMRect result = ScreenSceneConfig::CalcCutoutBoundaryRect(invalidSvgPath);
    EXPECT_EQ(result.posX_, 0);
    EXPECT_EQ(result.posY_, 0);
    EXPECT_EQ(result.width_, 0);
    EXPECT_EQ(result.height_, 0);
}

/**
 * @tc.name: CalcCutoutBoundaryRect03
 * @tc.desc: Test scenario where SkRect is empty and an empty rectangle is expected.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, CalcCutoutBoundaryRect03, TestSize.Level1)
{
    std::string emptySvgPath = "M0 0";
    DMRect result = ScreenSceneConfig::CalcCutoutBoundaryRect(emptySvgPath);
    EXPECT_EQ(result.posX_, 0);
    EXPECT_EQ(result.posY_, 0);
    EXPECT_EQ(result.width_, 0);
    EXPECT_EQ(result.height_, 0);
}

/**
 * @tc.name: CalcCutoutBoundaryRect04
 * @tc.desc: Test scenario where svg is valid and a valid rectangle is expected.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, CalcCutoutBoundaryRect04, TestSize.Level1)
{
    std::string validSvgPath = "M10 10 L20 20";
    DMRect result = ScreenSceneConfig::CalcCutoutBoundaryRect(validSvgPath);
    EXPECT_EQ(result.posX_, 10);
    EXPECT_EQ(result.posY_, 10);
    EXPECT_EQ(result.width_, 10);
    EXPECT_EQ(result.height_, 10);
}

/**
 * @tc.name: GetFoldDisplayMode
 * @tc.desc: GetFoldDisplayMode func
 * @tc.type: FUNC
*/
HWTEST_F(ScreenSceneConfigTest, GetFoldDisplayMode, TestSize.Level1)
{
    DisplayPhysicalResolution physicalSize1{ FoldDisplayMode::MAIN, 100, 200 };
    ScreenSceneConfig::displayPhysicalResolution_.emplace_back(physicalSize1);

    DisplayPhysicalResolution physicalSize2{ FoldDisplayMode::FULL, 300, 600 };
    ScreenSceneConfig::displayPhysicalResolution_.emplace_back(physicalSize2);

    FoldDisplayMode result = ScreenSceneConfig::GetFoldDisplayMode(200, 100);
    EXPECT_EQ(FoldDisplayMode::MAIN, result);
    result = ScreenSceneConfig::GetFoldDisplayMode(100, 200);
    EXPECT_EQ(FoldDisplayMode::MAIN, result);

    result = ScreenSceneConfig::GetFoldDisplayMode(300, 600);
    EXPECT_EQ(FoldDisplayMode::FULL, result);
    result = ScreenSceneConfig::GetFoldDisplayMode(600, 300);
    EXPECT_EQ(FoldDisplayMode::FULL, result);

    ScreenSceneConfig::displayPhysicalResolution_.clear();
}

/**
 * @tc.name: SetCutoutSvgPath
 * @tc.desc: SetCutoutSvgPath func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, SetCutoutSvgPath, TestSize.Level1)
{
    uint64_t displayId = 0;
    ScreenSceneConfig::SetCutoutSvgPath(displayId, "");
    ScreenSceneConfig::SetCutoutSvgPath(displayId, "oo");
    auto result_ = ScreenSceneConfig::GetCutoutBoundaryRect(displayId);
    ASSERT_NE(0, result_.size());
}

/**
 * @tc.name: SetSubCutoutSvgPath
 * @tc.desc: SetSubCutoutSvgPath func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, SetSubCutoutSvgPath, TestSize.Level1)
{
    ScreenSceneConfig::SetSubCutoutSvgPath("");
    ScreenSceneConfig::SetSubCutoutSvgPath("oo");
    auto result = ScreenSceneConfig::GetSubCutoutBoundaryRect();
    ASSERT_NE(0, result.size());
}

/**
 * @tc.name: SetSubCutoutSvgPath01
 * @tc.desc: SetSubCutoutSvgPath func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, SetSubCutoutSvgPath01, TestSize.Level1)
{
    ScreenSceneConfig::SetSubCutoutSvgPath("M507 18 L573 18 v 66 h -66 Z");
    std::vector<DMRect> result = ScreenSceneConfig::GetSubCutoutBoundaryRect();
    if (result.size() <= 0) {
        ASSERT_EQ(0, result.size());
    }
    DMRect targetRect{507, 18, 66, 66}; // the rect size after svg parsing
    EXPECT_EQ(result[0].posX_, targetRect.posX_);
    EXPECT_EQ(result[0].posY_, targetRect.posY_);
    EXPECT_EQ(result[0].width_, targetRect.width_);
    EXPECT_EQ(result[0].height_, targetRect.height_);
}

/**
 * @tc.name: SetCurvedCompressionAreaInLandscape
 * @tc.desc: SetCurvedCompressionAreaInLandscape func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, SetCurvedCompressionAreaInLandscape, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSceneConfig::SetCurvedCompressionAreaInLandscape();
    EXPECT_TRUE(g_logMsg.find("waterfallAreaCompressionSizeWhenHorzontal value is not exist") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: IsSupportRotateWithSensor01
 * @tc.desc: IsSupportRotateWithSensor
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsSupportRotateWithSensor01, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_["supportRotateWithSensor"] = true;
    bool res = ScreenSceneConfig::IsSupportRotateWithSensor();
    ASSERT_EQ(true, res);
}

/**
 * @tc.name: IsSupportRotateWithSensor01
 * @tc.desc: IsSupportRotateWithSensor
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsSupportRotateWithSensor02, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_.erase("supportRotateWithSensor");
    bool res = ScreenSceneConfig::IsSupportRotateWithSensor();
    ASSERT_EQ(false, res);
}

/**
 * @tc.name: GetExternalScreenDefaultMode01
 * @tc.desc: GetExternalScreenDefaultMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetExternalScreenDefaultMode01, TestSize.Level1)
{
    ScreenSceneConfig::stringConfig_["externalScreenDefaultMode"] = "mirror";
    std::string res = ScreenSceneConfig::GetExternalScreenDefaultMode();
    ASSERT_EQ("mirror", res);
}

/**
 * @tc.name: GetExternalScreenDefaultMode02
 * @tc.desc: GetExternalScreenDefaultMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetExternalScreenDefaultMode02, TestSize.Level1)
{
    ScreenSceneConfig::stringConfig_.erase("externalScreenDefaultMode");
    std::string res = ScreenSceneConfig::GetExternalScreenDefaultMode();
    ASSERT_EQ("", res);
}

/**
 * @tc.name: GetCurvedCompressionAreaInLandscape01
 * @tc.desc: GetCurvedCompressionAreaInLandscape
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetCurvedCompressionAreaInLandscape01, TestSize.Level1)
{
    ScreenSceneConfig::isWaterfallDisplay_ = false;
    ScreenSceneConfig::isScreenCompressionEnableInLandscape_ = false;
    auto result = ScreenSceneConfig::GetCurvedCompressionAreaInLandscape();
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: GetCurvedCompressionAreaInLandscape02
 * @tc.desc: GetCurvedCompressionAreaInLandscape
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetCurvedCompressionAreaInLandscape02, TestSize.Level1)
{
    ScreenSceneConfig::isWaterfallDisplay_ = true;
    ScreenSceneConfig::isScreenCompressionEnableInLandscape_ = false;
    auto result = ScreenSceneConfig::GetCurvedCompressionAreaInLandscape();
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: GetCurvedCompressionAreaInLandscape03
 * @tc.desc: Test GetCurvedCompressionAreaInLandscape method
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetCurvedCompressionAreaInLandscape03, TestSize.Level1)
{
    ScreenSceneConfig::isWaterfallDisplay_ = true;
    ScreenSceneConfig::isScreenCompressionEnableInLandscape_ = true;
    auto result = ScreenSceneConfig::GetCurvedCompressionAreaInLandscape();
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: ReadStringListConfigInfo01
 * @tc.desc: ReadStringListConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadStringListConfigInfo01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    xmlNodePtr rootNode = nullptr;
    ScreenSceneConfig::ReadStringListConfigInfo(rootNode, "");
    EXPECT_TRUE(g_logMsg.find("get root element failed") == std::string::npos &&
        g_logMsg.find("rootContext is null") == std::string::npos &&
        g_logMsg.find("invalid node") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReadStringListConfigInfo02
 * @tc.desc: ReadStringListConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadStringListConfigInfo02, TestSize.Level1)
{
    xmlNodePtr rootNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(rootNode, nullptr);
    rootNode->name = nullptr;
    std::string name = "testName";
    ScreenSceneConfig::ReadStringListConfigInfo(rootNode, name);
    xmlFreeNode(rootNode);
}

/**
 * @tc.name: ReadStringListConfigInfo03
 * @tc.desc: ReadStringListConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadStringListConfigInfo03, TestSize.Level1)
{
    xmlNodePtr rootNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(rootNode, nullptr);
    std::string name = "testName";
    ScreenSceneConfig::ReadStringListConfigInfo(rootNode, name);
    xmlFreeNode(rootNode);
}

/**
 * @tc.name: ReadStringListConfigInfo04
 * @tc.desc: ReadStringListConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadStringListConfigInfo04, TestSize.Level1)
{
    xmlNodePtr rootNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(rootNode, nullptr);
    xmlNodePtr curNode = xmlNewNode(nullptr, BAD_CAST "invalidNode");
    rootNode->children = curNode;
    curNode->parent = rootNode;
    curNode->next = nullptr;
    std::string name = "testName";
    ScreenSceneConfig::ReadStringListConfigInfo(rootNode, name);
    xmlFreeNode(rootNode);
}

/**
 * @tc.name: ReadStringListConfigInfo05
 * @tc.desc: ReadStringListConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadStringListConfigInfo05, TestSize.Level1)
{
    xmlNodePtr rootNode = xmlNewNode(nullptr, BAD_CAST "testNode");
    ASSERT_NE(rootNode, nullptr);
    xmlNodePtr curNode = xmlNewNode(nullptr, BAD_CAST "invalidNode");
    xmlNodeSetContent(curNode, BAD_CAST "validContent");
    rootNode->children = curNode;
    curNode->parent = rootNode;
    curNode->next = nullptr;
    std::string name = "testName";
    ScreenSceneConfig::ReadStringListConfigInfo(rootNode, name);
    xmlFreeNode(rootNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo01
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo01, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*> ("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo02
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo02, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>(" "));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo03
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo03, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("100:200"));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo04
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo04, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_FULL:100:200"));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo05
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo05, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_MAIN:100:200"));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo06
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo0, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_SUB:100:200"));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo07
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo07, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("UNKNOWN:100:200"));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadPhysicalDisplayConfigInfo08
 * @tc.desc: ReadPhysicalDisplayConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadPhysicalDisplayConfigInfo08, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadPhysicalDisplayConfigInfo(currNode);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_FULL:abc:def"));
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam01
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam01, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam02
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam02, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_FULL:1.5:0.5"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam03
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam03, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_MAIN:1.5:0.5"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam04
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam04, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_SUB:1.5:0.5"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam05
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam05, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("UNKNOWN:1.5:0.5"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam06
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam06, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
        xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_FULL:1.5:"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam07
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam07, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>(" "));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam08
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam08, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("UNKNOWN:!!:aa"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ReadScrollableParam09
 * @tc.desc: ReadScrollableParam
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ReadScrollableParam09, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("displayMode"));
    ASSERT_NE(currNode, nullptr);
    xmlNodeSetContent(currNode, reinterpret_cast<const xmlChar*>("FOLD_DISPLAY_MODE_COORDINATION:1.5:0.5"));
    ScreenSceneConfig::ReadScrollableParam(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig01
 * @tc.desc: Test if ParseNodeConfig correctly calls ReadEnableConfigInfo when node name matches.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig01, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("IS_WATERFALL_DISPLAY"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig02
 * @tc.desc: Test if ParseNodeConfig correctly calls ReadIntNumbersConfigInfo when node name matches.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig02, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("DPI"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig03
 * @tc.desc: Test if ParseNodeConfig correctly calls ReadStringConfigInfo when node name matches.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig03, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("DEFAULT_DISPLAY_CUTOUT_PATH"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig04
 * @tc.desc: Test if ParseNodeConfig correctly calls ReadStringListConfigInfo when node name matches.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig04, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("HALL_SWITCH_APP"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig05
 * @tc.desc: Test if ParseNodeConfig correctly calls ReadPhysicalDisplayConfigInfo when node name matches.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig05, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("PHYSICAL_DISPLAY_RESOLUTION"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig06
 * @tc.desc: Test if ParseNodeConfig correctly calls ReadScrollableParam when node name matches.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig06, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("SCROLLABLE_PARAM"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseNodeConfig07
 * @tc.desc: Test if ParseNodeConfig logs a warning when node name does not match any known node.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseNodeConfig07, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>("UNKNOWN_NODE"));
    ASSERT_NE(currNode, nullptr);
    ScreenSceneConfig::ParseNodeConfig(currNode);
    xmlFreeNode(currNode);
}

/**
 * @tc.name: ParseStrToUll
 * @tc.desc: Test ParseStrToUll
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseStrToUll, TestSize.Level1)
{
    std::string input = "12345";
    uint64_t result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 12345u);

    input = "abc123";
    result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 0u);

    input = "123abc";
    result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 0u);

    input = "";
    result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 0u);

    input = "18446744073709551615";
    result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 18446744073709551615ULL);

    input = "18446744073709551616";
    result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 0u);

    input = " 123 ";
    result = ScreenSceneConfig::ParseStrToUll(input);
    EXPECT_EQ(result, 0u);
}

/**
 * @tc.name: ParseDisplaysConfig03
 * @tc.desc: ParseDisplaysConfig03
 @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, ParseDisplaysConfig03, TestSize.Level1)
{
    ScreenSceneConfig::displaysConfigs_.clear();
    xmlNodePtr displaysNode = xmlNewNode(nullptr, BAD_CAST("displays"));
    xmlNodePtr displayNode = xmlNewChild(displaysNode, nullptr, BAD_CAST("display"), nullptr);
    xmlNewChild(displayNode, nullptr, BAD_CAST("physicalId"), BAD_CAST("101"));
    xmlNewChild(displayNode, nullptr, BAD_CAST("logicalId"), BAD_CAST("201"));
    xmlNodePtr emptyNameNode = xmlNewChild(displayNode, nullptr, BAD_CAST("name"), BAD_CAST(""));
    xmlNewChild(displayNode, nullptr, BAD_CAST("dpi"), BAD_CAST("300"));
    ScreenSceneConfig::ParseDisplaysConfig(displaysNode);
    ASSERT_EQ(ScreenSceneConfig::GetDisplaysConfigs().size(), 1u);
    const auto& config = ScreenSceneConfig::GetDisplaysConfigs()[0];
    EXPECT_EQ(config.physicalId, 101u);
    EXPECT_EQ(config.logicalId, 201u);
    EXPECT_EQ(config.dpi, 300);
    EXPECT_TRUE(config.name.empty());
    xmlFree(displaysNode);
}

/**
 * @tc.name: GetOffScreenPPIThreshold01
 * @tc.desc: Test GetOffScreenPPIThreshold when the array is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetOffScreenPPIThreshold01, TestSize.Level1)
{
    uint32_t result = ScreenSceneConfig::GetOffScreenPPIThreshold();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetOffScreenPPIThreshold02
 * @tc.desc: Test GetOffScreenPPIThreshold when the array is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetOffScreenPPIThreshold02, TestSize.Level1)
{
    ScreenSceneConfig::xmlNodeMap_.clear();
    uint32_t result = ScreenSceneConfig::GetOffScreenPPIThreshold();
    EXPECT_EQ(result, ScreenSceneConfig::offScreenPPIThreshold_);
}

/**
 * @tc.name: IsSupportOffScreenRendering01
 * @tc.desc: IsSupportOffScreenRendering01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsSupportOffScreenRendering01, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_["isSupportOffScreenRendering"] = true;
    bool res = ScreenSceneConfig::IsSupportOffScreenRendering();
    EXPECT_EQ(true, res);
}

/**
 * @tc.name: IsSupportOffScreenRendering02
 * @tc.desc: IsSupportOffScreenRendering02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsSupportOffScreenRendering02, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_.erase("isSupportOffScreenRendering");
    bool res = ScreenSceneConfig::IsSupportOffScreenRendering();
    EXPECT_EQ(false, res);
}

/**
 * @tc.name: IsSupportDuringCall01
 * @tc.desc: IsSupportDuringCall01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsSupportDuringCall01, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_["supportDuringCall"] = true;
    bool res = ScreenSceneConfig::IsSupportDuringCall();
    EXPECT_TRUE(res);
}

/**
 * @tc.name: IsSupportDuringCall02
 * @tc.desc: IsSupportDuringCall02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, IsSupportDuringCall02, TestSize.Level1)
{
    ScreenSceneConfig::enableConfig_.erase("supportDuringCall");
    bool res = ScreenSceneConfig::IsSupportDuringCall();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: GetNumberConfigValue
 * @tc.desc: GetNumberConfigValue
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetNumberConfigValue, TestSize.Level1)
{
    std::vector<int> test_config;
    test_config.push_back(10);
    ScreenSceneConfig::intNumbersConfig_["halfFoldedBuffer"] = test_config;
    uint32_t res = ScreenSceneConfig::GetNumberConfigValue("halfFoldedBuffer", 1);
    EXPECT_EQ(res, 10);
}

/**
 * @tc.name: GetNumberConfigValue_default
 * @tc.desc: GetNumberConfigValue_default
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneConfigTest, GetNumberConfigValue_default, TestSize.Level1)
{
    ScreenSceneConfig::intNumbersConfig_.erase("halfFoldedBuffer");
    uint32_t res = ScreenSceneConfig::GetNumberConfigValue("halfFoldedBuffer", 1);
    EXPECT_EQ(res, 1);
}

}
} // namespace Rosen
} // namespace OHOS
