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

#include <gtest/gtest.h>

#include <libxml/globals.h>
#include <libxml/xmlstring.h>

#include "display_manager_config.h"
#include "display_manager_config.cpp"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class DisplayManagerConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DisplayManagerConfigTest::SetUpTestCase()
{
}

void DisplayManagerConfigTest::TearDownTestCase()
{
}

void DisplayManagerConfigTest::SetUp()
{
}

void DisplayManagerConfigTest::TearDown()
{
}

namespace {
/**
 * @tc.name: Split
 * @tc.desc: test function : Split
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, Split, TestSize.Level1)
{
    std::string str = "stringPatStr";
    std::string pattern = "pattern";
    std::vector<std::string> result(DisplayManagerConfig::Split(str, pattern));
    EXPECT_EQ(result[0], "stringPatStr");
    result.clear();
    std::string str02 = "stringPatternStr";
    std::vector<std::string> result02(DisplayManagerConfig::Split(str02, pattern));
    EXPECT_EQ(result02[0], "stringPatternStr");
}

/**
 * @tc.name: LoadConfigXml
 * @tc.desc: test function : LoadConfigXml
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, LoadConfigXml, TestSize.Level1)
{
    bool result = DisplayManagerConfig::LoadConfigXml();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ReadIntNumbersConfigInfo
 * @tc.desc: test function : ReadIntNumbersConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, ReadIntNumbersConfigInfo, TestSize.Level1)
{
    DisplayManagerConfig::enableConfig_.clear();

    const char* xmlContent = "1 2 3 4";
    xmlNodePtr validNode = xmlNewNode(NULL, BAD_CAST"TestNode");
    xmlNodeSetContent(validNode, BAD_CAST xmlContent);
    DisplayManagerConfig::ReadIntNumbersConfigInfo(validNode);

    const char* invalidContent = "1 2 abc 4";
    xmlNodePtr invalidNode = xmlNewNode(NULL, BAD_CAST"TestNode");
    xmlNodeSetContent(invalidNode, BAD_CAST invalidContent);
    DisplayManagerConfig::ReadIntNumbersConfigInfo(invalidNode);

    EXPECT_LE(DisplayManagerConfig::enableConfig_.size(), 0);
    xmlFreeNode(validNode);
    xmlFreeNode(invalidNode);
}

/**
 * @tc.name: IsNumber
 * @tc.desc: test function : IsNumber
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, IsNumber, TestSize.Level1)
{
    bool result = DisplayManagerConfig::IsNumber("123");
    ASSERT_EQ(true, result);
    result = DisplayManagerConfig::IsNumber("a123");
    ASSERT_EQ(false, result);
}

/**
 * @tc.name: GetConfigPath
 * @tc.desc: test function : GetConfigPath
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, GetConfigPath, TestSize.Level1)
{
    auto result = DisplayManagerConfig::GetConfigPath("");
    ASSERT_STRNE("/system/", result.c_str());

    result = DisplayManagerConfig::GetConfigPath("a.xml");
    ASSERT_STREQ("/system/a.xml", result.c_str());
}

/**
 * @tc.name: ReadEnableConfigInfo
 * @tc.desc: test function : ReadEnableConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, ReadEnableConfigInfo, TestSize.Level1)
{
    DisplayManagerConfig::enableConfig_.clear();

    auto configFilePath = DisplayManagerConfig::GetConfigPath("etc/window/resources/display_manager_config.xml");
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
        if (!DisplayManagerConfig::IsValidNode(*curNodePtr)) {
            continue;
        }

        auto nodeName = curNodePtr->name;
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("isWaterfallDisplay"))) {
            DisplayManagerConfig::ReadEnableConfigInfo(curNodePtr);
            readCount++;
            continue;
        }

        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("dpi"))) {
            DisplayManagerConfig::ReadEnableConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
    }

    ASSERT_LE(DisplayManagerConfig::enableConfig_.size(), readCount);

    DisplayManagerConfig::DumpConfig();
    xmlFreeDoc(docPtr);
}

/**
 * @tc.name: ReadStringConfigInfo
 * @tc.desc: test function : ReadStringConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerConfigTest, ReadStringConfigInfo, TestSize.Level1)
{
    DisplayManagerConfig::enableConfig_.clear();

    auto configFilePath = DisplayManagerConfig::GetConfigPath("etc/window/resources/display_manager_config.xml");
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
        if (!DisplayManagerConfig::IsValidNode(*curNodePtr)) {
            continue;
        }

        auto nodeName = curNodePtr->name;
        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("defaultDisplayCutoutPath"))) {
            DisplayManagerConfig::ReadStringConfigInfo(curNodePtr);
            readCount++;
            continue;
        }

        if (!xmlStrcmp(nodeName, reinterpret_cast<const xmlChar*>("dpi"))) {
            DisplayManagerConfig::ReadStringConfigInfo(curNodePtr);
            readCount++;
            continue;
        }
    }

    ASSERT_LE(DisplayManagerConfig::stringConfig_.size(), readCount);
    DisplayManagerConfig::DumpConfig();
    xmlFreeDoc(docPtr);
}

}
} // namespace Rosen
} // namespace OHOS