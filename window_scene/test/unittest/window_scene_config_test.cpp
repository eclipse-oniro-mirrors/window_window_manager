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

#include <gtest/gtest.h>
#include <libxml/globals.h>
#include <libxml/xmlstring.h>
#include "window_scene_config.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using ConfigItem = WindowSceneConfig::ConfigItem;
const std::string XML_STR = R"(<?xml version='1.0' encoding="utf-8"?>
<!--
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
 -->
<Configs>
    <!--decor enable is true means app main window show decoration-->
    <decor enable="false"></decor>
    <!--max number of main window that could be shown on display-->
    <maxAppWindowNumber>100</maxAppWindowNumber>
    <!--minimizeByOther enable is true means fullscreen window will be minmized by other fullscreen window-->
    <minimizeByOther enable="true"></minimizeByOther>
    <!--window mdoe change hot zones config, fullscreen primary secondary-->
    <modeChangeHotZones>50 50 50</modeChangeHotZones>
    <!--stretchable enable is true means all window be stretchable-->
    <stretchable enable="false"></stretchable>
    <!--exit split screen mode ratio config-->
    <exitSplitRatios>0.1 0.9</exitSplitRatios>
    <!--split screen ratios config-->
    <splitRatios>0.5 0.33 0.67</splitRatios>
    <!--default window mode config-->
    <defaultWindowMode>1</defaultWindowMode>
    <!--window animation config-->
    <windowAnimation>
        <timing>
            <!--duration of animation when add/remove window, unit is ms-->
            <duration>350</duration>
            <!--timing curve of animation, config it as below:
            name=ease, easeIn, easeOut, easeInOut, default, linear,
            spring, interactiveSpring, cubic(float float float float)-->
            <curve name="easeOut"></curve>
        </timing>
        <!--scaleX and scaleY of animation start state-->
        <scale>0.7 0.7</scale>
        <!--rotation of animation start state, 4 numbers is axis and angle-->
        <rotation>0 0 1 0</rotation>
        <!--translateX and translateY of animation start state-->
        <translate>0 0</translate>
        <!--opacity of animation start state-->
        <opacity>0</opacity>
    </windowAnimation>
    <!--keyboard animation config-->
    <keyboardAnimation>
        <animationIn>
            <timing>
                <!--duration of animation when add keyboard, unit is ms-->
                <duration>500</duration>
                <!--friction curve-->
                <curve name="cubic">0.2 0.0 0.2 1.0</curve>
            </timing>
        </animationIn>
        <animationOut>
            <timing>
                <!--duration of animation when remove keyboard, unit is ms-->
                <duration>300</duration>
                <!--friction curve-->
                <curve name="cubic">0.2 0.0 0.2 1.0</curve>
            </timing>
        </animationOut>
    </keyboardAnimation>
    <!--enable/disable remote animation-->
    <remoteAnimation enable="true"></remoteAnimation>
    <!--window effect config-->
    <windowEffect>
        <appWindows>
            <cornerRadius>
                <!--off: no corner, defaultCornerRadiusXS: 4vp, defaultCornerRadiusS: 8vp-->
                <!--defaultCornerRadiusM: 12vp, defaultCornerRadiusL: 16vp, defaultCornerRadiusXL: 24vp-->
                <fullScreen>off</fullScreen>
                <split>off</split>
                <float>off</float>
            </cornerRadius>
            <shadow>
                <focused>
                    <elevation>0</elevation>
                    <color>#000000</color>
                    <offsetX>0</offsetX>
                    <offsetY>0</offsetY>
                    <alpha>0</alpha>
                </focused>
                <unfocused>
                    <elevation>0</elevation>
                    <color>#000000</color>
                    <offsetX>0</offsetX>
                    <offsetY>0</offsetY>
                    <alpha>0</alpha>
                </unfocused>
            </shadow>
        </appWindows>
    </windowEffect>
</Configs>
)";

class WindowSceneConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    ConfigItem ReadConfig(const std::string& xmlStr);
};

void WindowSceneConfigTest::SetUpTestCase() {}

void WindowSceneConfigTest::TearDownTestCase() {}

void WindowSceneConfigTest::SetUp() {}

void WindowSceneConfigTest::TearDown() {}

ConfigItem WindowSceneConfigTest::ReadConfig(const std::string& xmlStr)
{
    ConfigItem cfgItem;
    xmlDocPtr docPtr = xmlParseMemory(xmlStr.c_str(), xmlStr.length());
    if (docPtr == nullptr) {
        return cfgItem;
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        xmlFreeDoc(docPtr);
        return cfgItem;
    }

    std::map<std::string, ConfigItem> configMap;
    cfgItem.SetValue(configMap);
    WindowSceneConfig::ReadConfig(rootPtr, *cfgItem.mapValue_);
    xmlFreeDoc(docPtr);
    return cfgItem;
}
namespace {
/**
 * @tc.name: AnimationConfig
 * @tc.desc: animation config test
 * @tc.type: FUNC
 * @tc.require issueI5N26H
 */
HWTEST_F(WindowSceneConfigTest, AnimationConfig, TestSize.Level1)
{
    WindowSceneConfig::config_ = ReadConfig(XML_STR);
    ConfigItem item = WindowSceneConfig::config_["windowAnimation"];
    ASSERT_EQ(true, item.IsMap());
    item = WindowSceneConfig::config_["windowAnimation"]["timing"]["duration"];
    ASSERT_EQ(true, item.IsInts());

    auto value = *item.intsValue_;
    ASSERT_EQ(true, value.size() == 1);
    ASSERT_EQ(350, value[0]);

    item = WindowSceneConfig::config_["windowAnimation"]["timing"]["curve"].GetProp("name");
    ASSERT_EQ(true, item.IsString());
    ASSERT_EQ("easeOut", item.stringValue_);

    item = WindowSceneConfig::config_["windowAnimation"]["scale"];
    ASSERT_EQ(true, item.IsFloats());
    ASSERT_EQ(true, item.floatsValue_->size() == 2);

    item = WindowSceneConfig::config_["windowAnimation"]["rotation"];
    ASSERT_EQ(true, item.IsFloats());
    ASSERT_EQ(true, item.floatsValue_->size() == 4);

    item = WindowSceneConfig::config_["windowAnimation"]["translate"];
    ASSERT_EQ(true, item.IsFloats());
    ASSERT_EQ(true, item.floatsValue_->size() == 2);

    item = WindowSceneConfig::config_["windowAnimation"]["opacity"];
    ASSERT_EQ(true, item.IsFloats());
    ASSERT_EQ(true, item.floatsValue_->size() == 1);
}

/**
 * @tc.name: maxAppWindowNumber
 * @tc.desc: maxAppWindowNumber test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneConfigTest, MaxAppWindowNumber, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxAppWindowNumber>0</maxAppWindowNumber>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    WindowSceneConfig::ConfigItem item;
    std::vector<int> value;
    item = WindowSceneConfig::config_["maxAppWindowNumber"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(true, item.IsInts());
    value = *item.intsValue_;
    ASSERT_EQ(true, value.size() >= 1);
    ASSERT_EQ(0, value[0]);

    xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxAppWindowNumber>-2</maxAppWindowNumber>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    item = WindowSceneConfig::config_["maxAppWindowNumber"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(true, item.IsInts());
    ASSERT_EQ(true, item.intsValue_->size() == 0);

    xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxAppWindowNumber>4</maxAppWindowNumber>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    item = WindowSceneConfig::config_["maxAppWindowNumber"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(true, item.IsInts());
    ASSERT_EQ(1, item.intsValue_->size());
    value = *item.intsValue_;
    ASSERT_EQ(4, value[0]);

    xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxAppWindowNumber>1000</maxAppWindowNumber>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    item = WindowSceneConfig::config_["maxAppWindowNumber"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(true, item.IsInts());
    ASSERT_EQ(1, item.intsValue_->size());
    value = *item.intsValue_;
    ASSERT_EQ(1000, value[0]);
}

/**
 * @tc.name: DecorConfig01
 * @tc.desc: set decor true and false without mode support.
 * @tc.type: FUNC
 * @tc.require: issueI68QCO
 */
HWTEST_F(WindowSceneConfigTest, DecorConfig01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\"/>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ASSERT_EQ(true, WindowSceneConfig::config_["decor"].GetProp("enable").boolValue_);

    xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"false\"/>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ASSERT_EQ(false, WindowSceneConfig::config_["decor"].GetProp("enable").boolValue_);
}

/**
 * @tc.name: DecorConfig02
 * @tc.desc: set decor true and mode support fullscreen.
 * @tc.type: FUNC
 * @tc.require: issueI68QCO
 */
HWTEST_F(WindowSceneConfigTest, DecorConfig02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>fullscreen</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    WindowSceneConfig::ConfigItem item = WindowSceneConfig::config_["decor"]["supportedMode"];
    ASSERT_EQ(true, item.IsStrings());
    ASSERT_EQ(1, item.stringsValue_->size());
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(false, item.IsString());
    std::vector<std::string> supportedModes;
    supportedModes = *item.stringsValue_;
    ASSERT_EQ("fullscreen", supportedModes[0]);
}

/**
 * @tc.name: DecorConfig03
 * @tc.desc: set decor true and mode support floating.
 * @tc.type: FUNC
 * @tc.require: issueI68QCO
 */
HWTEST_F(WindowSceneConfigTest, DecorConfig03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>floating</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    WindowSceneConfig::ConfigItem item = WindowSceneConfig::config_["decor"]["supportedMode"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(false, item.IsString());
    ASSERT_EQ(true, item.IsStrings());
    ASSERT_EQ(1, item.stringsValue_->size());
    std::vector<std::string> supportedModes;
    supportedModes = *item.stringsValue_;
    ASSERT_EQ("floating", supportedModes[0]);
}

/**
 * @tc.name: DecorConfig04
 * @tc.desc: set decor true and mode support fullscreen|floating.
 * @tc.type: FUNC
 * @tc.require: issueI68QCO
 */
HWTEST_F(WindowSceneConfigTest, DecorConfig04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>fullscreen floating</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    WindowSceneConfig::ConfigItem item = WindowSceneConfig::config_["decor"]["supportedMode"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(false, item.IsString());
    ASSERT_EQ(true, item.IsStrings());
    ASSERT_EQ(2, item.stringsValue_->size());
    std::vector<std::string> supportedModes;
    supportedModes = *item.stringsValue_;
    ASSERT_NE("fullscreen floating", supportedModes[0]);
}

/**
 * @tc.name: MoveResampleSecondaryPhaseConfig
 * @tc.desc: Parse secondary phase configuration and lead time from XML
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneConfigTest, MoveResampleSecondaryPhaseConfig, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowLayout>"
        "<moveDrag>"
        "<moveResample enable=\"true\">"
        "<secondaryPhase enable=\"true\">"
        "<leadTimeMs>2</leadTimeMs>"
        "</secondaryPhase>"
        "</moveResample>"
        "</moveDrag>"
        "</windowLayout>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);

    auto secondaryPhase = WindowSceneConfig::config_["windowLayout"]["moveDrag"]["moveResample"]["secondaryPhase"];
    ASSERT_TRUE(secondaryPhase.IsMap());
    ASSERT_TRUE(secondaryPhase.GetProp("enable").IsBool());
    EXPECT_TRUE(secondaryPhase.GetProp("enable").boolValue_);
    auto leadTime = secondaryPhase["leadTimeMs"];
    ASSERT_TRUE(leadTime.IsInts());
    ASSERT_EQ(leadTime.intsValue_->size(), 1);
    EXPECT_EQ((*leadTime.intsValue_)[0], 2);
}

/**
 * @tc.name: LoadconfigXml
 * @tc.desc: load config xml
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WindowSceneConfigTest, LoadConfigXml, TestSize.Level1)
{
    auto result = WindowSceneConfig::LoadConfigXml();
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: ReadFloatNumbersConfigInfo
 * @tc.desc: ReadFloatNumbersConfigInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneConfigTest, ReadFloatNumbersConfigInfo, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(NULL, BAD_CAST "nodeName");
    auto result = WindowSceneConfig::ReadFloatNumbersConfigInfo(currNode, false);
    ASSERT_EQ(0, result.size());
}

/**
 * @tc.name: ReadStringConfigInfo
 * @tc.desc: ReadStringConfigInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneConfigTest, ReadStringConfigInfo, TestSize.Level1)
{
    xmlNodePtr currNode = xmlNewNode(NULL, BAD_CAST "nodeName");
    auto result = WindowSceneConfig::ReadStringConfigInfo(currNode);
    ASSERT_EQ(0, result.size());
}

/**
 * @tc.name: MaxMidSceneNumConfig01
 * @tc.desc: set maxMidSceneNum with 3.
 * @tc.type: FUNC
 * @tc.require: issueI68QCO
 */
HWTEST_F(WindowSceneConfigTest, MaxMidSceneNumConfig01, Function | SmallTest | Level2)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxMidSceneNum>3</maxMidSceneNum>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    WindowSceneConfig::ConfigItem item = WindowSceneConfig::config_["maxMidSceneNum"];
    ASSERT_EQ(false, item.IsMap());
    ASSERT_EQ(true, item.IsInts());
    auto value = *item.intsValue_;
    ASSERT_EQ(true, value.size() >= 1);
    ASSERT_EQ(3, value[0]);
}

} // namespace
} // namespace Rosen
} // namespace OHOS
