/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <regex>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/session_utils.h"
#include "session/host/include/main_session.h"
#include "session/host/include/move_drag_controller.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "context.h"
#include "iremote_object_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace {
const std::string EMPTY_DEVICE_ID = "";
constexpr int WAIT_SLEEP_TIME = 1;
using ConfigItem = WindowSceneConfig::ConfigItem;
ConfigItem ReadConfig(const std::string& xmlStr)
{
    ConfigItem config;
    xmlDocPtr docPtr = xmlParseMemory(xmlStr.c_str(), xmlStr.length());
    if (docPtr == nullptr) {
        return config;
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == nullptr || rootPtr->name == nullptr ||
        xmlStrcmp(rootPtr->name, reinterpret_cast<const xmlChar*>("Configs"))) {
        xmlFreeDoc(docPtr);
        return config;
    }

    std::map<std::string, ConfigItem> configMap;
    config.SetValue(configMap);
    WindowSceneConfig::ReadConfig(rootPtr, *config.mapValue_);
    xmlFreeDoc(docPtr);
    return config;
}
} // namespace
class SceneSessionManagerTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static bool gestureNavigationEnabled_;
    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest2::ssm_ = nullptr;

bool SceneSessionManagerTest2::gestureNavigationEnabled_ = true;
ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest2::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void SceneSessionManagerTest2::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest2::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest2::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest2::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: SetGestureNavigationEnabled
 * @tc.desc: SceneSessionManager set gesture navigation enabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetGestureNavigationEnabled, TestSize.Level1)
{
    ASSERT_NE(callbackFunc_, nullptr);

    WMError result00 = ssm_->SetGestureNavigationEnabled(true);
    EXPECT_EQ(result00, WMError::WM_OK);

    ssm_->SetGestureNavigationEnabledChangeListener(callbackFunc_);
    WMError result01 = ssm_->SetGestureNavigationEnabled(true);
    EXPECT_EQ(result01, WMError::WM_OK);
    sleep(WAIT_SLEEP_TIME);
    EXPECT_EQ(gestureNavigationEnabled_, true);

    WMError result02 = ssm_->SetGestureNavigationEnabled(false);
    EXPECT_EQ(result02, WMError::WM_OK);
    sleep(WAIT_SLEEP_TIME);
    EXPECT_EQ(gestureNavigationEnabled_, false);

    ssm_->SetGestureNavigationEnabledChangeListener(nullptr);
    WMError result03 = ssm_->SetGestureNavigationEnabled(true);
    EXPECT_EQ(result03, WMError::WM_OK);
}

/**
 * @tc.name: RegisterWindowManagerAgent01
 * @tc.desc: SceneSesionManager rigister window manager agent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RegisterWindowManagerAgent01, TestSize.Level1)
{
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ssm_->RegisterWindowManagerAgent(type, windowManagerAgent));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent));
}

/**
 * @tc.name: RegisterWindowManagerAgent02
 * @tc.desc: SceneSesionManager rigister window manager agent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RegisterWindowManagerAgent02, TestSize.Level1)
{
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;

    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ssm_->RegisterWindowManagerAgent(type, windowManagerAgent));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent));
}

/**
 * @tc.name: RegisterWindowPropertyChangeAgent01
 * @tc.desc: Register and unregister window property change agent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RegisterWindowPropertyChangeAgent01, TestSize.Level1)
{
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    uint32_t interestInfo = static_cast<uint32_t>(WindowInfoKey::WINDOW_ID);
    ssm_->observedFlags_ = 0;
    ssm_->interestedFlags_ = 0;

    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION,
        ssm_->RegisterWindowPropertyChangeAgent(WindowInfoKey::DISPLAY_ID, interestInfo, windowManagerAgent));
    EXPECT_EQ(ssm_->observedFlags_, static_cast<uint32_t>(WindowInfoKey::DISPLAY_ID));
    EXPECT_EQ(ssm_->interestedFlags_, static_cast<uint32_t>(WindowInfoKey::WINDOW_ID));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION,
        ssm_->UnregisterWindowPropertyChangeAgent(WindowInfoKey::DISPLAY_ID, interestInfo, windowManagerAgent));
    EXPECT_EQ(0, ssm_->observedFlags_);
    EXPECT_EQ(0, ssm_->interestedFlags_);
}

/**
 * @tc.name: ConfigWindowLayout
 * @tc.desc: Verify ConfigWindowLayout handles invalid/valid configs correctly
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowLayout, TestSize.Level1)
{
    // Case 1: windowLayoutConfig is NOT a MAP → return false
    {
        ConfigItem layout;
        EXPECT_FALSE(layout.IsMap());

        bool ret = ssm_->ConfigWindowLayout(layout);
        EXPECT_FALSE(ret);
    }

    // Case 2: windowLayoutConfig is MAP → call ConfigMoveDrag → return true
    {
        ConfigItem layout;
        ConfigItem moveDrag;

        // Construct map: { "moveDrag": moveDrag }
        layout.SetValue({ { "moveDrag", moveDrag } });
        EXPECT_TRUE(layout.IsMap());

        bool ret = ssm_->ConfigWindowLayout(layout);
        EXPECT_TRUE(ret);
    }
}

/**
 * @tc.name: ConfigMoveDrag
 * @tc.desc: Verify ConfigMoveDrag only validates top-level type and returns expected values
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigMoveDrag, TestSize.Level1)
{
    // Case 1: moveDragConfig is NOT a MAP → return false
    {
        ConfigItem moveDrag;
        EXPECT_FALSE(moveDrag.IsMap());

        bool ret = ssm_->ConfigMoveDrag(moveDrag);
        EXPECT_FALSE(ret);
    }

    // Case 2: moveDragConfig is a MAP → return true (sub-config errors are ignored)
    {
        ConfigItem moveResample; // even if invalid, ConfigMoveDrag still returns true
        moveResample.SetValue(123); // not a map
        ConfigItem moveDrag;
        moveDrag.SetValue({ { "moveResample", moveResample } });

        bool ret = ssm_->ConfigMoveDrag(moveDrag);
        EXPECT_TRUE(ret);
    }
}

/**
 * @tc.name: ConfigMoveResampleWithInvalidConfig
 * @tc.desc: Verify ConfigMoveResample returns false for invalid config structure
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigMoveResampleWithInvalidConfig, TestSize.Level1)
{
    // Case 1: moveResampleConfig is NOT a MAP → return false
    {
        ConfigItem moveResample;
        moveResample.SetValue(123);

        EXPECT_FALSE(ssm_->ConfigMoveResample(moveResample));
    }

    // Case 2: enable MISSING → return false
    {
        ConfigItem moveResample;
        moveResample.SetValue(std::map<std::string, ConfigItem>{});

        EXPECT_FALSE(ssm_->ConfigMoveResample(moveResample));
    }

    // Case 3: enable NOT BOOL → return false
    {
        ConfigItem enableProp;
        enableProp.SetValue(std::string("not a bool"));
        ConfigItem moveResample;
        moveResample.SetProperty({ { "enable", enableProp } });
        moveResample.SetValue(std::map<std::string, ConfigItem>{});

        EXPECT_FALSE(ssm_->ConfigMoveResample(moveResample));
    }
}

/**
 * @tc.name: ConfigMoveResampleFpsRangeFallback
 * @tc.desc: Verify invalid fpsRange falls back to default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigMoveResampleFpsRangeFallback, TestSize.Level1)
{
    ConfigItem enableProp;
    enableProp.SetValue(true);

    // Case 1: resampleFpsRange INVALID TYPE → fallback to default (nullopt/nullopt), return true
    {
        ConfigItem fpsRange;
        fpsRange.SetValue(std::string("not an int array"));

        ConfigItem moveResample;
        moveResample.SetProperty({ { "enable", enableProp } });
        moveResample.SetValue({ { "resampleFpsRange", fpsRange } });

        EXPECT_TRUE(ssm_->ConfigMoveResample(moveResample));

        auto [enable, minFps, maxFps] = MoveDragController::GetMoveResampleSystemConfig();
        EXPECT_TRUE(enable);
        EXPECT_FALSE(minFps.has_value());
        EXPECT_FALSE(maxFps.has_value());
    }

    // Case 2: resampleFpsRange SIZE != 2 → fallback to default (nullopt/nullopt), return true
    {
        ConfigItem fpsRange;
        fpsRange.SetValue(std::vector<int>{60, 120, 144});

        ConfigItem moveResample;
        moveResample.SetProperty({ { "enable", enableProp } });
        moveResample.SetValue({ { "resampleFpsRange", fpsRange } });

        EXPECT_TRUE(ssm_->ConfigMoveResample(moveResample));

        auto [enable, minFps, maxFps] = MoveDragController::GetMoveResampleSystemConfig();
        EXPECT_TRUE(enable);
        EXPECT_FALSE(minFps.has_value());
        EXPECT_FALSE(maxFps.has_value());
    }
}


/**
 * @tc.name: ConfigMoveResampleFpsRangeValid
 * @tc.desc: Verify valid fpsRange is normalized and applied
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigMoveResampleFpsRangeValid, TestSize.Level1)
{
    ConfigItem enableProp;
    enableProp.SetValue(true);

    ConfigItem fpsRange;
    fpsRange.SetValue(std::vector<int>{120, 60}); // reversed

    ConfigItem moveResample;
    moveResample.SetProperty({ { "enable", enableProp } });
    moveResample.SetValue({ { "resampleFpsRange", fpsRange } });

    EXPECT_TRUE(ssm_->ConfigMoveResample(moveResample));

    auto [enable, minFps, maxFps] = MoveDragController::GetMoveResampleSystemConfig();
    EXPECT_TRUE(enable);
    ASSERT_TRUE(minFps.has_value());
    ASSERT_TRUE(maxFps.has_value());
    EXPECT_EQ(*minFps, 60);
    EXPECT_EQ(*maxFps, 120);
}

/**
 * @tc.name: ConfigWindowSizeLimits01
 * @tc.desc: call ConfigWindowSizeLimits and check the systemConfig_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSizeLimits01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<mainWindowSizeLimits>"
        "<miniWidth>10</miniWidth>"
        "<miniHeight>20</miniHeight>"
        "</mainWindowSizeLimits>"
        "<subWindowSizeLimits>"
        "<miniWidth>30</miniWidth>"
        "<miniHeight>40</miniHeight>"
        "</subWindowSizeLimits>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSizeLimits();
    EXPECT_EQ(ssm_->systemConfig_.miniWidthOfMainWindow_, static_cast<uint32_t>(10));
    EXPECT_EQ(ssm_->systemConfig_.miniHeightOfMainWindow_, static_cast<uint32_t>(20));
    EXPECT_EQ(ssm_->systemConfig_.miniWidthOfSubWindow_, static_cast<uint32_t>(30));
    EXPECT_EQ(ssm_->systemConfig_.miniHeightOfSubWindow_, static_cast<uint32_t>(40));
}

/**
 * @tc.name: ConfigWindowEffect01
 * @tc.desc: call ConfigWindowEffect all success focused
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<cornerRadius>"
        "<fullScreen>off</fullScreen>"
        "<split>off</split>"
        "<float>off</float>"
        "</cornerRadius>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</focused>"
        "</shadow>"
        "<shadowDark>"
        "<focused>"
        "<elevation>0</elevation>"
        "<color>#111111</color>"
        "<offsetX>2</offsetX>"
        "<offsetY>2</offsetY>"
        "<alpha>1</alpha>"
        "<radius>1</radius>"
        "</focused>"
        "</shadowDark>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.radius_, 0.5);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadowDark_.alpha_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadowDark_.offsetX_, 2);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadowDark_.offsetY_, 2);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadowDark_.radius_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadowDark_.color_, "#ff111111");
    EXPECT_EQ(ssm_->appWindowSceneConfig_.floatCornerRadius_, 0);
    EXPECT_EQ(ssm_->systemConfig_.defaultCornerRadius_, 0);
}

/**
 * @tc.name: ConfigWindowEffect02
 * @tc.desc: call ConfigWindowEffect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<cornerRadius>"
        "<fullScreen>off</fullScreen>"
        "<split>off</split>"
        "</cornerRadius>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<alpha>0</alpha>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.alpha_, 0);
}

/**
 * @tc.name: ConfigWindowEffect03
 * @tc.desc: call ConfigWindowEffect ConfigAppWindowShadow unfocused
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "<shadowDark>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#111111</color>"
        "<offsetX>2</offsetX>"
        "<offsetY>2</offsetY>"
        "<alpha>1</alpha>"
        "<radius>1</radius>"
        "</unfocused>"
        "</shadowDark>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadowDark_.alpha_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadowDark_.offsetX_, 2);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadowDark_.offsetY_, 2);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadowDark_.radius_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadowDark_.color_, "#ff111111");
}

/**
 * @tc.name: ConfigWindowEffect04
 * @tc.desc: call ConfigWindowEffect all
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<cornerRadius>"
        "<fullScreen>off</fullScreen>"
        "<split>off</split>"
        "<float>off</float>"
        "</cornerRadius>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.radius_, 0.5);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: ConfigWindowEffect05
 * @tc.desc: call ConfigWindowEffect all offsetX.size is not 1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect05, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetX>1</offsetX>"
        "<offsetX>2</offsetX>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: ConfigWindowEffect06
 * @tc.desc: call ConfigWindowEffect offsetY.size is not 1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect06, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetY>1</offsetY>"
        "<offsetY>2</offsetY>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: ConfigWindowEffect07
 * @tc.desc: call ConfigWindowEffect alpha.size is not 1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect07, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<alpha>1</alpha>"
        "<alpha>2</alpha>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: ConfigWindowEffect08
 * @tc.desc: call ConfigWindowEffect radius.size is not 1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowEffect08, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<radius>1</radius>"
        "<radius>2</radius>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: call ConfigDecor fullscreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigDecor01, TestSize.Level1)
{
    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"111\">"
        "<supportedMode>fullscreen</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>fullscreen</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.decorWindowModeSupportType_,
              static_cast<uint32_t>(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN));
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: call ConfigDecor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigDecor02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.decorWindowModeSupportType_, WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: call ConfigDecor floating
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigDecor03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>floating</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.decorWindowModeSupportType_, WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING);
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: call ConfigDecor pip
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigDecor04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>pip</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.decorWindowModeSupportType_, WindowModeSupport::WINDOW_MODE_SUPPORT_PIP);
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: call ConfigDecor split
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigDecor05, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>split</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.decorWindowModeSupportType_,
              WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                  WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY);
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: call ConfigDecor default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigDecor06, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<decor enable=\"true\">"
        "<supportedMode>111</supportedMode>"
        "</decor>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.decorWindowModeSupportType_, WINDOW_MODE_SUPPORT_ALL);
}

/**
 * @tc.name: ConfigWindowSceneXml01
 * @tc.desc: call defaultWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultWindowMode>10</defaultWindowMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultWindowMode>102</defaultWindowMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.defaultWindowMode_, static_cast<WindowMode>(static_cast<uint32_t>(102)));
}

/**
 * @tc.name: ConfigWindowSceneXml02
 * @tc.desc: call defaultWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultWindowMode>1 1</defaultWindowMode>"
        "<uiType>phone</uiType>"
        "<backgroundScreenLock enable=\"true\"></backgroundScreenLock>"
        "<rotationMode>windowRotation</rotationMode>"
        "<supportTypeFloatWindow enable=\"true\"></supportTypeFloatWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultWindowMode>1</defaultWindowMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.defaultWindowMode_, static_cast<WindowMode>(static_cast<uint32_t>(1)));
}

/**
 * @tc.name: ConfigWindowSceneXml03
 * @tc.desc: call defaultMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultMaximizeMode>1 1</defaultMaximizeMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultMaximizeMode>1</defaultMaximizeMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(SceneSession::maximizeMode_, static_cast<MaximizeMode>(static_cast<uint32_t>(1)));
}

/**
 * @tc.name: ConfigWindowSceneXml04
 * @tc.desc: call defaultMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultMaximizeMode>111</defaultMaximizeMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<defaultMaximizeMode>0</defaultMaximizeMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(SceneSession::maximizeMode_, static_cast<MaximizeMode>(static_cast<uint32_t>(0)));
}

/**
 * @tc.name: ConfigWindowSceneXml05
 * @tc.desc: call maxFloatingWindowSize
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml05, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxFloatingWindowSize>1</maxFloatingWindowSize>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<maxFloatingWindowSize>1</maxFloatingWindowSize>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.maxFloatingWindowSize_, static_cast<uint32_t>(1));
}

/**
 * @tc.name: ConfigWindowSceneXml07
 * @tc.desc: call backgroundScreenLock
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml07, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<backgroundScreenLock enable=\"true\"></backgroundScreenLock>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->appWindowSceneConfig_.backgroundScreenLock_, true);
}

/**
 * @tc.name: ConfigWindowSceneXml08
 * @tc.desc: call rotationMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowSceneXml08, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<rotationMode>rotation</rotationMode>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->appWindowSceneConfig_.rotationMode_, "rotation");
}

/**
 * @tc.name: ConfigWindowAnimation01
 * @tc.desc: call ConfigWindowAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "<duration>350</duration>"
        "<curve name=\"easeOut\"></curve>"
        "</timing>"
        "<scale>0.7 0.7</scale>"
        "<rotation>0 0 1 0</rotation>"
        "<translate>0 0</translate>"
        "<opacity>0</opacity>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.duration_, 350);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleX_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleY_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationZ_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.angle_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.opacity_, 0);
}

/**
 * @tc.name: ConfigWindowAnimation02
 * @tc.desc: call ConfigWindowAnimation no change
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "<duration>350</duration>"
        "<curve name=\"easeOut\"></curve>"
        "</timing>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.duration_, 350);
}

/**
 * @tc.name: ConfigWindowAnimation03
 * @tc.desc: call ConfigWindowAnimation no timing
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "</timing>"
        "<scale>0.7 0.7</scale>"
        "<rotation>0 0 1 0</rotation>"
        "<translate>0 0</translate>"
        "<opacity>0</opacity>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleX_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleY_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationZ_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.angle_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.opacity_, 0);
}

/**
 * @tc.name: ConfigWindowAnimation04
 * @tc.desc: call ConfigWindowAnimation default timing is not int
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "<duration>aaa</duration>"
        "<curve></curve>"
        "</timing>"
        "<scale>0.7 0.7</scale>"
        "<rotation>0 0 1 0</rotation>"
        "<translate>0 0</translate>"
        "<opacity>0</opacity>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleX_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleY_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationZ_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.angle_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.opacity_, 0);
}

/**
 * @tc.name: ConfigWindowAnimation05
 * @tc.desc: call ConfigWindowAnimation default timing is error size
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation05, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "<duration>350 350</duration>"
        "<curve></curve>"
        "</timing>"
        "<scale>0.7 0.7</scale>"
        "<rotation>0 0 1 0</rotation>"
        "<translate>0 0</translate>"
        "<opacity>0</opacity>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleX_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.scaleY_, static_cast<float>(0.7));
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.rotationZ_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.angle_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateX_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.translateY_, 0);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.opacity_, 0);
}

/**
 * @tc.name: ConfigWindowAnimation06
 * @tc.desc: call ConfigWindowAnimation default change is not int
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation06, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "<duration>350</duration>"
        "<curve name=\"easeOut\"></curve>"
        "</timing>"
        "<scale>a a</scale>"
        "<rotation>a a a a</rotation>"
        "<translate>a a</translate>"
        "<opacity>a</opacity>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.duration_, 350);
}

/**
 * @tc.name: ConfigWindowAnimation07
 * @tc.desc: call ConfigWindowAnimation default change error size
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigWindowAnimation07, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowAnimation>"
        "<timing>"
        "<duration>350</duration>"
        "<curve name=\"easeOut\"></curve>"
        "</timing>"
        "<scale>0.7 0.7 0.7</scale>"
        "<rotation>0 0 1 0 1</rotation>"
        "<translate>0 0 1</translate>"
        "<opacity>0 1</opacity>"
        "</windowAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->appWindowSceneConfig_.windowAnimation_.duration_, 350);
}

/**
 * @tc.name: ConfigStartingWindowAnimation01
 * @tc.desc: call ConfigStartingWindowAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigStartingWindowAnimation01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<startWindowTransitionAnimation enable=\"false\">"
        "<timing>"
        "<duration>200</duration>"
        "<curve name=\"linear\"></curve>"
        "</timing>"
        "<opacityStart>1</opacityStart>"
        "<opacityEnd>0</opacityEnd>"
        "</startWindowTransitionAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.enabled_, false);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.duration_, 200);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.opacityStart_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.opacityEnd_, 0);
}

/**
 * @tc.name: ConfigStartingWindowAnimation02
 * @tc.desc: call ConfigStartingWindowAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigStartingWindowAnimation02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<startWindowTransitionAnimation enable=\"aaa\">"
        "<timing>"
        "<duration>200</duration>"
        "<curve name=\"linear\"></curve>"
        "</timing>"
        "<opacityStart>1</opacityStart>"
        "<opacityEnd>0</opacityEnd>"
        "</startWindowTransitionAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.duration_, 200);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.opacityStart_, 1);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.opacityEnd_, 0);
}

/**
 * @tc.name: ConfigStartingWindowAnimation03
 * @tc.desc: call ConfigStartingWindowAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigStartingWindowAnimation03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<startWindowTransitionAnimation enable=\"false\">"
        "<timing>"
        "<duration>aaa</duration>"
        "<curve name=\"linear\"></curve>"
        "</timing>"
        "<opacityStart>aaa</opacityStart>"
        "<opacityEnd>aaa</opacityEnd>"
        "</startWindowTransitionAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.enabled_, false);
}

/**
 * @tc.name: ConfigStartingWindowAnimation04
 * @tc.desc: call ConfigStartingWindowAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigStartingWindowAnimation04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<startWindowTransitionAnimation enable=\"false\">"
        "<timing>"
        "<duration>200 200</duration>"
        "<curve name=\"linear\"></curve>"
        "</timing>"
        "<opacityStart>1 1</opacityStart>"
        "<opacityEnd>0 1</opacityEnd>"
        "</startWindowTransitionAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.enabled_, false);
}

/**
 * @tc.name: ConfigStartingWindowAnimation05
 * @tc.desc: call ConfigStartingWindowAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigStartingWindowAnimation05, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<startWindowTransitionAnimation enable=\"false\">"
        "<timing>"
        "<duration>aaa aaa</duration>"
        "<curve name=\"linear\"></curve>"
        "</timing>"
        "<opacityStart>a a</opacityStart>"
        "<opacityEnd>a a</opacityEnd>"
        "</startWindowTransitionAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.startingWindowAnimationConfig_.enabled_, false);
}

/**
 * @tc.name: ConfigSnapshotScale01
 * @tc.desc: call ConfigSnapshotScale and check the snapshotScale_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSnapshotScale01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<snapshotScale>0.7</snapshotScale>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigSnapshotScale();
    EXPECT_EQ(ssm_->snapshotScale_, static_cast<float>(0.7));
}

/**
 * @tc.name: ConfigSnapshotScale02
 * @tc.desc: call ConfigSnapshotScale and check the snapshotScale_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSnapshotScale02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<snapshotScale>0.7 0.7</snapshotScale>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigSnapshotScale();
    EXPECT_EQ(ssm_->snapshotScale_, 0.7f);
}

/**
 * @tc.name: ConfigSnapshotScale03
 * @tc.desc: call ConfigSnapshotScale and check the snapshotScale_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSnapshotScale03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<snapshotScale>aaa</snapshotScale>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigSnapshotScale();
    EXPECT_EQ(ssm_->snapshotScale_, 0.7f);
}

/**
 * @tc.name: ConfigSnapshotScale04
 * @tc.desc: call ConfigSnapshotScale and check the snapshotScale_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSnapshotScale04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<snapshotScale>-0.1</snapshotScale>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigSnapshotScale();
    EXPECT_EQ(ssm_->snapshotScale_, 0.7f);
}

/**
 * @tc.name: ConfigSnapshotScale05
 * @tc.desc: call ConfigSnapshotScale and check the snapshotScale_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSnapshotScale05, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<snapshotScale>1.5</snapshotScale>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigSnapshotScale();
    EXPECT_EQ(ssm_->snapshotScale_, 0.7f);
}

/**
 * @tc.name: ConfigSystemUIStatusBar01
 * @tc.desc: call ConfigSystemUIStatusBar default.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSystemUIStatusBar01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<systemUIStatusBar>"
        "<showInLandscapeMode>1</showInLandscapeMode>"
        "<immersiveStatusBarBgColor>#4c000000</immersiveStatusBarBgColor>"
        "<immersiveStatusBarContentColor>#ffffee</immersiveStatusBarContentColor>"
        "</systemUIStatusBar>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    sceneSessionManager->ConfigWindowSceneXml();
    ASSERT_EQ(sceneSessionManager->appWindowSceneConfig_.systemUIStatusBarConfig_.showInLandscapeMode_, 1);
    ASSERT_STREQ(sceneSessionManager->appWindowSceneConfig_.systemUIStatusBarConfig_.immersiveStatusBarBgColor_.c_str(),
                 "#4c000000");
    ASSERT_STREQ(
        sceneSessionManager->appWindowSceneConfig_.systemUIStatusBarConfig_.immersiveStatusBarContentColor_.c_str(),
        "#ffffee");
}

/**
 * @tc.name: ConfigFreeMultiWindowForDefaultDragResizeType01
 * @tc.desc: call ConfigFreeMultiWindow and check the defaultDragResizeType_ default value.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowForDefaultDragResizeType01, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    EXPECT_EQ(ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_, DragResizeType::RESIZE_TYPE_UNDEFINED);
}

/**
 * @tc.name: ConfigFreeMultiWindowForDefaultDragResizeType02
 * @tc.desc: call ConfigFreeMultiWindow and check the defaultDragResizeType_ invalid string value.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowForDefaultDragResizeType02, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<freeMultiWindow enable=\"true\">"
        "<defaultDragResizeType>na</defaultDragResizeType>"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    EXPECT_EQ(ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_, DragResizeType::RESIZE_TYPE_UNDEFINED);
}

/**
 * @tc.name: ConfigFreeMultiWindowForDefaultDragResizeType03
 * @tc.desc: call ConfigFreeMultiWindow and check the defaultDragResizeType_ invalid int value.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowForDefaultDragResizeType03, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<freeMultiWindow enable=\"true\">"
        "<defaultDragResizeType>-2</defaultDragResizeType>"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    EXPECT_EQ(ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_, DragResizeType::RESIZE_TYPE_UNDEFINED);
}

/**
 * @tc.name: ConfigFreeMultiWindowForDefaultDragResizeType04
 * @tc.desc: call ConfigFreeMultiWindow and check the defaultDragResizeType_ exceeding the max value.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowForDefaultDragResizeType04, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<freeMultiWindow enable=\"true\">"
        "<defaultDragResizeType>999999</defaultDragResizeType>"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    EXPECT_EQ(ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_, DragResizeType::RESIZE_TYPE_UNDEFINED);
}

/**
 * @tc.name: ConfigFreeMultiWindowForDefaultDragResizeType05
 * @tc.desc: call ConfigFreeMultiWindow and check the defaultDragResizeType_ valid value.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowForDefaultDragResizeType05, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<freeMultiWindow enable=\"true\">"
        "<defaultDragResizeType>2</defaultDragResizeType>"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    EXPECT_EQ(ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_, DragResizeType::RESIZE_WHEN_DRAG_END);
}

/**
 * @tc.name: ConfigFreeMultiWindow
 * @tc.desc: call ConfigFreeMultiWindow.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowTest, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<freeMultiWindow enable=\"true\">"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetX>1</offsetX>"
        "<offsetY>2</offsetY>"
        "<radius>0.5</radius>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "<shadowDark>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetX>1</offsetX>"
        "<offsetY>2</offsetY>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadowDark>"
        "</appWindows>"
        "</windowEffect>"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    // freeMultiWindow windowEffect
    auto windowEffect = ssm_->systemConfig_.freeMultiWindowConfig_.appWindowSceneConfig_;
    
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.alpha_, 0);
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.offsetX_, 1);
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.offsetY_, 2);
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.radius_, 0.5);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.alpha_, 0);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.offsetX_, 1);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.offsetY_, 1);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: ConfigFreeMultiWindow2
 * @tc.desc: call ConfigFreeMultiWindow2.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigFreeMultiWindowTest2, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetX>1</offsetX>"
        "<offsetY>2</offsetY>"
        "<radius>40</radius>"
        "<alpha>1</alpha>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>1</offsetX>"
        "<offsetY>1</offsetY>"
        "<alpha>0</alpha>"
        "<radius>20</radius>"
        "</unfocused>"
        "</shadow>"
        "</appWindows>"
        "</windowEffect>"
        "<freeMultiWindow enable=\"true\">"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    // freeMultiWindow windowEffect
    auto windowEffect = ssm_->systemConfig_.freeMultiWindowConfig_.appWindowSceneConfig_;
    
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.alpha_, 1);
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.offsetX_, 1);
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.offsetY_, 2);
    EXPECT_FLOAT_EQ(windowEffect.focusedShadow_.radius_, 40);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.alpha_, 0);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.offsetX_, 1);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.offsetY_, 1);
    EXPECT_FLOAT_EQ(windowEffect.unfocusedShadow_.radius_, 20);
}

/**
 * @tc.name: LoadFreeMultiWindowConfigTest
 * @tc.desc: call LoadFreeMultiWindowConfig
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, LoadFreeMultiWindowConfigTest, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<alpha>1</alpha>"
        "<offsetX>2</offsetX>"
        "<offsetY>2</offsetY>"
        "<radius>0</radius>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>2</offsetX>"
        "<offsetY>2</offsetY>"
        "<alpha>1</alpha>"
        "<radius>0</radius>"
        "</unfocused>"
        "</shadow>"
        "<shadowDark>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetX>2</offsetX>"
        "<offsetY>2</offsetY>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>2</offsetX>"
        "<offsetY>2</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadowDark>"
        "</appWindows>"
        "</windowEffect>"
        "<freeMultiWindow enable=\"true\">"
        "<windowEffect>"
        "<appWindows>"
        "<shadow>"
        "<focused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<alpha>0</alpha>"
        "<offsetX>3</offsetX>"
        "<offsetY>3</offsetY>"
        "<radius>0.5</radius>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>3</offsetX>"
        "<offsetY>3</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadow>"
        "<shadowDark>"
        "<focused>"
        "<elevation>0</elevation>"
        "<offsetX>3</offsetX>"
        "<offsetY>3</offsetY>"
        "<radius>0.5</radius>"
        "</focused>"
        "<unfocused>"
        "<elevation>0</elevation>"
        "<color>#000000</color>"
        "<offsetX>3</offsetX>"
        "<offsetY>3</offsetY>"
        "<alpha>0</alpha>"
        "<radius>0.5</radius>"
        "</unfocused>"
        "</shadowDark>"
        "</appWindows>"
        "</windowEffect>"
        "</freeMultiWindow>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigFreeMultiWindow();
    ASSERT_EQ(ssm_->systemConfig_.freeMultiWindowSupport_, true);
    // load windowEffect
    ssm_->LoadFreeMultiWindowConfig(false);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetX_, 2);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetY_, 2);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.alpha_, 1);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.radius_, 0);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 1);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 2);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 2);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0);

    // load FreeMultiWindow windowEffect config
    ssm_->LoadFreeMultiWindowConfig(true);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.alpha_, 0);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetX_, 3);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.offsetY_, 3);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.focusedShadow_.radius_, 0.5);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.alpha_, 0);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetX_, 3);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.offsetY_, 3);
    EXPECT_FLOAT_EQ(ssm_->appWindowSceneConfig_.unfocusedShadow_.radius_, 0.5);
}

/**
 * @tc.name: Init
 * @tc.desc: SceneSessionManager init
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, Init, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->Init();
    ssm_->RegisterAppListener();
    EXPECT_FALSE(g_logMsg.find("appMgrClient is nullptr.") != std::string::npos);
    
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->Init();
    EXPECT_TRUE(g_logMsg.find("SSM init success.") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterBrightnessDataChangeListener
 * @tc.desc: SceneSessionManager RegisterBrightnessDataChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RegisterBrightnessDataChangeListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->RegisterBrightnessDataChangeListener();
    EXPECT_TRUE(g_logMsg.find("RegisterBrightnessDataChangeListener") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnregisterBrightnessDataChangeListener
 * @tc.desc: SceneSessionManager UnregisterBrightnessDataChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, UnregisterBrightnessDataChangeListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->UnregisterBrightnessDataChangeListener();
    EXPECT_TRUE(g_logMsg.find("UnregisterBrightnessDataChangeListener") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 *@tc.name: HandleUserSwitching
 *@tc.desc: SceneSesionManager HandleUserSwitching
 *@tc.type: FUNC
*/
HWTEST_F(SceneSessionManagerTest2, HandleUserSwitching, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->HandleUserSwitching(true);
    EXPECT_TRUE(g_logMsg.find("taskScheduler is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: LoadWindowSceneXml
 * @tc.desc: SceneSesionManager load window scene xml
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, LoadWindowSceneXml, TestSize.Level1)
{
    ssm_->LoadWindowSceneXml();
    ssm_->ConfigWindowSceneXml();
    ssm_->SetEnableInputEvent(true);
    ssm_->SetEnableInputEvent(false);
    EXPECT_EQ(ssm_->IsInputEventEnabled(), false);
}

/**
 * @tc.name: UpdateRecoveredSessionInfo
 * @tc.desc: SceneSessionManager load window scene xml
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, UpdateRecoveredSessionInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<int32_t> recoveredPersistentIds;
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    recoveredPersistentIds.push_back(0);
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    ssm_->sceneSessionMap_.erase(0);
}

/**
 * @tc.name: UpdateRecoveredSessionInfo02
 * @tc.desc: Test if failRecoverPersistentSet exist or not exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, UpdateRecoveredSessionInfo02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    std::vector<int32_t> recoveredPersistentIds;
    recoveredPersistentIds.push_back(0);
    recoveredPersistentIds.push_back(1);
    SessionInfo info;
    info.abilityName_ = "UpdateRecoveredSessionInfo02";
    info.bundleName_ = "SceneSessionManagerTest2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->failRecoveredPersistentIdSet_.insert(0);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    EXPECT_FALSE(g_logMsg.find("Session is nullptr,") != std::string::npos);
    LOG_SetCallback(nullptr);
    ssm_->failRecoveredPersistentIdSet_.erase(0);
    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: NotifyCreateSubSession
 * @tc.desc: Test if createSubSessionFuncMap_ exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, NotifyCreateSubSession, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "NotifyCreateSubSession";
    info.bundleName_ = "SceneSessionManagerTest2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    NotifyCreateSubSessionFunc func;
    ssm_->createSubSessionFuncMap_.insert({ 1, func });
    ssm_->NotifyCreateSubSession(1, sceneSession, 256);
    EXPECT_FALSE(g_logMsg.find("SubSession is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
    ssm_->createSubSessionFuncMap_.erase(1);
}

/**
 * @tc.name: SetSessionContinueState
 * @tc.desc: SceneSesionManager set session continue state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetSessionContinueState, TestSize.Level1)
{
    MessageParcel* data = new MessageParcel();
    sptr<IRemoteObject> token = data->ReadRemoteObject();
    auto continueState = static_cast<ContinueState>(data->ReadInt32());
    WSError result02 = ssm_->SetSessionContinueState(nullptr, continueState);
    WSError result01 = ssm_->SetSessionContinueState(token, continueState);
    EXPECT_EQ(result02, WSError::WS_ERROR_INVALID_PARAM);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_PARAM);
    delete data;
}

/**
 * @tc.name: SetSessionContinueState002
 * @tc.desc: SceneSesionManager set session continue state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetSessionContinueState002, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MessageParcel* data = new MessageParcel();
    sptr<IRemoteObject> token = data->ReadRemoteObject();
    auto continueState = static_cast<ContinueState>(0);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1000, sceneSession });
    ssm_->SetSessionContinueState(token, continueState);
    EXPECT_TRUE(g_logMsg.find("in") != std::string::npos);
    LOG_SetCallback(nullptr);
    delete data;
}

/**
 * @tc.name: StartWindowInfoReportLoop
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, StartWindowInfoReportLoop, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->StartWindowInfoReportLoop();
    ssm_->eventHandler_ = nullptr;
    ssm_->StartWindowInfoReportLoop();
    ssm_->isReportTaskStart_ = true;
    ssm_->StartWindowInfoReportLoop();
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetFocusWindowInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    FocusChangeInfo info;
    ssm_->GetFocusWindowInfo(info);
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetFocusWindowInfo2, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    FocusChangeInfo fcinfo;
    ssm_->GetFocusWindowInfo(fcinfo);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->GetFocusWindowInfo(fcinfo);
}

/**
 * @tc.name: GetFocusWindowInfoByAbilityToken_SACalling
 * @tc.desc: Test function GetFocusWindowInfoByAbilityToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetFocusWindowInfoByAbilityToken_SACalling, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    MockAccesstokenKit::MockIsSACalling(false);
    FocusChangeInfo info;
    sptr<IRemoteObject> token;
    ssm_->GetFocusWindowInfoByAbilityToken(info, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);
    MockAccesstokenKit::MockIsSACalling(true);
}

/**
 * @tc.name: GetFocusWindowInfoByAbilityToken
 * @tc.desc: Test function GetFocusWindowInfoByAbilityToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetFocusWindowInfoByAbilityToken, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "GetFocusWindowInfoByAbilityToken";
    info.bundleName_ = "GetFocusWindowInfoByAbilityToken";
    FocusChangeInfo focusInfo{};
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> sceneSession = nullptr;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 2, sceneSession1 });
    ssm_->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);

    sceneSession1->SetAbilityToken(token);
    sceneSession1->property_->SetDisplayId(DISPLAY_ID_INVALID);
    ssm_->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);

    ssm_->windowFocusController_->AddFocusGroup(0, 0);
    sceneSession1->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    sceneSession1->property_->SetPersistentId(2);
    focusGroup->SetFocusedSessionId(1);
    ssm_->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);

    focusGroup->SetFocusedSessionId(2);
    ssm_->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_NE(focusInfo.windowId_, INVALID_WINDOW_ID);
    ssm_->windowFocusController_->RemoveFocusGroup(0, 0);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SetSessionLabel
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetSessionLabel, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->SetSessionLabel(nullptr, "test");

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->SetSessionLabel(nullptr, "test");
}

/**
 * @tc.name: SetSessionIcon
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetSessionIcon, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->SetSessionIcon(nullptr, nullptr);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->SetSessionIcon(nullptr, nullptr);
}

/**
 * @tc.name: InitWithRenderServiceAdded
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, InitWithRenderServiceAdded, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->InitWithRenderServiceAdded();
}

/**
 * @tc.name: PendingSessionToForeground
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, PendingSessionToForeground, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->PendingSessionToForeground(nullptr);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->PendingSessionToForeground(nullptr);
}

/**
 * @tc.name: GetFocusSessionToken
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetFocusSessionToken, TestSize.Level1)
{
    WSError ret;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ret = ssm_->GetFocusSessionToken(token);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, ret);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ret = ssm_->GetFocusSessionToken(token);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: GetFocusSessionElement
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetFocusSessionElement, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    AppExecFwk::ElementName element;
    ssm_->GetFocusSessionElement(element);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->GetFocusSessionElement(element);
}

/**
 * @tc.name: GetAllAbilityInfos
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetAllAbilityInfos, TestSize.Level1)
{
    WSError ret;
    AAFwk::Want want;
    AppExecFwk::ElementName elementName = want.GetElement();
    int32_t userId = 1;
    std::vector<SCBAbilityInfo> scbAbilityInfos;

    ret = ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    elementName.bundleName_ = "test";
    ret = ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    elementName.abilityName_ = "test";
    ret = ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    elementName.bundleName_ = "";
    ret = ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    ssm_->bundleMgr_ = nullptr;
    ret = ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: GetIsLayoutFullScreen
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetIsLayoutFullScreen, TestSize.Level1)
{
    WSError ret;
    bool isLayoutFullScreen = true;
    ret = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(WSError::WS_OK, ret);

    isLayoutFullScreen = false;
    ret = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(WSError::WS_OK, ret);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    isLayoutFullScreen = true;
    ret = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(WSError::WS_OK, ret);

    isLayoutFullScreen = false;
    ret = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: UpdateSessionTouchOutsideListener
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, UpdateSessionTouchOutsideListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    int32_t persistentId = 100;
    ssm_->UpdateSessionTouchOutsideListener(persistentId, true);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->UpdateSessionTouchOutsideListener(persistentId, true);

    ssm_->UpdateSessionTouchOutsideListener(persistentId, false);
}

/**
 * @tc.name: GetSessionSnapshotById
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetSessionSnapshotById, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionSnapshot snapshot;
    ssm_->GetSessionSnapshotById(100, snapshot);
}

/**
 * @tc.name: ClearSession
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ClearSession, TestSize.Level1)
{
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto ret = ssm_->ClearSession(100);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: ClearAllSessions
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ClearAllSessions, TestSize.Level1)
{
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto ret = ssm_->ClearAllSessions();
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: GetTopWindowId
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetTopWindowId, TestSize.Level1)
{
    WMError ret;
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    uint32_t persistentId = 100;
    uint32_t topWinId = 200;
    ret = ssm_->GetTopWindowId(persistentId, topWinId);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ret = ssm_->GetTopWindowId(persistentId, topWinId);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: InitPersistentStorage
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, InitPersistentStorage, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    ssm_->InitPersistentStorage();
}

/**
 * @tc.name: GetAccessibilityWindowInfo
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetAccessibilityWindowInfo, TestSize.Level1)
{
    WMError ret;
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    ret = ssm_->GetAccessibilityWindowInfo(infos);
    EXPECT_EQ(WMError::WM_OK, ret);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ret = ssm_->GetAccessibilityWindowInfo(infos);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: OnScreenshot
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, OnScreenshot, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    DisplayId displayId = 0;
    ssm_->OnScreenshot(displayId);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->OnScreenshot(displayId);

    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->OnScreenshot(displayId);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->OnScreenshot(displayId);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    ssm_->OnScreenshot(displayId);
    sceneSession->SetSessionState(SessionState::STATE_END);
    ssm_->OnScreenshot(displayId);
}

/**
 * @tc.name: GetSurfaceNodeIdsFromSubSession
 * @tc.desc: Test GetSurfaceNodeIdsFromSubSession;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, GetSurfaceNodeIdsFromSubSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<uint64_t> surfaceNodeIds;
    auto ret = ssm_->GetSurfaceNodeIdsFromSubSession(nullptr, surfaceNodeIds);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    SessionInfo info;
    info.abilityName_ = "GetSurfaceNodeIdsFromSubSession";
    info.bundleName_ = "GetSurfaceNodeIdsFromSubSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ret = ssm_->GetSurfaceNodeIdsFromSubSession(sceneSession, surfaceNodeIds);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->SetLeashWinSurfaceNode(surfaceNode);

    sptr<SceneSession> subSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession1->SetSurfaceNode(surfaceNode);
    subSession1->SetLeashWinSurfaceNode(surfaceNode);
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession2->surfaceNode_ = nullptr;
    sptr<SceneSession> subSession3 = nullptr;
    sceneSession->subSession_.push_back(subSession1);
    sceneSession->subSession_.push_back(subSession2);
    sceneSession->subSession_.push_back(subSession3);

    ret = ssm_->GetSurfaceNodeIdsFromSubSession(sceneSession, surfaceNodeIds);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetSurfaceNodeIds
 * @tc.desc: Test SetSurfaceNodeIds;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetSurfaceNodeIds, TestSize.Level1)
{
    sptr<IDisplayChangeListener> listener = sptr<DisplayChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    DisplayId displayId = 9999;
    std::vector<uint64_t> surfaceNodeIds = { 1001, 1002 };
    listener->OnSetSurfaceNodeIds(displayId, surfaceNodeIds);

    ASSERT_NE(nullptr, ssm_);
    displayId = 10;

    SessionInfo info;
    info.abilityName_ = "SetSurfaceNodeIds";
    info.bundleName_ = "SetSurfaceNodeIds";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100001, sceneSession });
    auto ret = ssm_->SetSurfaceNodeIds(displayId, surfaceNodeIds);
    EXPECT_EQ(WMError::WM_OK, ret);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->GetSurfaceNode()->SetId(1001);
    ret = ssm_->SetSurfaceNodeIds(displayId, surfaceNodeIds);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: OnVirtualScreenDisconnected
 * @tc.desc: Test OnVirtualScreenDisconnected;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, OnVirtualScreenDisconnected, TestSize.Level1)
{
    sptr<IDisplayChangeListener> listener = sptr<DisplayChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    listener->OnVirtualScreenDisconnected(9999);

    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SessionInfo info;
    info.abilityName_ = "OnVirtualScreenDisconnected";
    info.bundleName_ = "OnVirtualScreenDisconnected";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto ret = ssm_->OnVirtualScreenDisconnected(1);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    ssm_->sessionBlackListInfoMap_[1];
    ret = ssm_->OnVirtualScreenDisconnected(1);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_[1];
    ret = ssm_->OnVirtualScreenDisconnected(1);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: UpdateSubSessionBlackList
 * @tc.desc: Test UpdateSubSessionBlackList;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, UpdateSubSessionBlackList, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sessionBlackListInfoMap_.clear();
    SessionInfo info;
    info.abilityName_ = "UpdateSubSessionBlackList";
    info.bundleName_ = "UpdateSubSessionBlackList";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 9998;
    sceneSession->parentSession_ = nullptr;
    auto ret = ssm_->UpdateSubSessionBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    sptr<SceneSession> parentSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    parentSceneSession->persistentId_ = 100003;
    sceneSession->SetParentSession(parentSceneSession);
    ssm_->sessionBlackListInfoMap_[11].insert({ .windowId = 100001 });
    ssm_->sessionBlackListInfoMap_[11].insert({ .windowId = 100002 });
    EXPECT_EQ(2, ssm_->sessionBlackListInfoMap_[11].size());
    ret = ssm_->UpdateSubSessionBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->GetSurfaceNode()->SetId(1001);
    sceneSession->SetLeashWinSurfaceNode(surfaceNode);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession2 = nullptr;
    ssm_->sceneSessionMap_.insert({ 9998, sceneSession });
    ssm_->sceneSessionMap_.insert({ 100001, sceneSession1 });
    ssm_->sceneSessionMap_.insert({ 100002, sceneSession2 });
    parentSceneSession->persistentId_ = 100002;
    ret = ssm_->UpdateSubSessionBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);

    sceneSession->persistentId_ = 9999;
    ret = ssm_->UpdateSubSessionBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RemoveSessionFromBlackList
 * @tc.desc: Test RemoveSessionFromBlackList;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RemoveSessionFromBlackList, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sessionBlackListInfoMap_.clear();
    SessionInfo info;
    info.abilityName_ = "RemoveSessionFromBlackList";
    info.bundleName_ = "RemoveSessionFromBlackList";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 9998;
    auto ret = ssm_->RemoveSessionFromBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    ssm_->sessionBlackListInfoMap_[11].insert({ .windowId = 9996 });
    ssm_->sessionBlackListInfoMap_[11].insert({ .windowId = 9997 });
    ssm_->sessionBlackListInfoMap_[12].insert({ .windowId = 9998 });
    ret = ssm_->RemoveSessionFromBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);

    sceneSession->persistentId_ = 9996;
    ret = ssm_->RemoveSessionFromBlackList(sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = ssm_->RemoveSessionFromBlackList(nullptr);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: ProcessSubSessionForeground
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ProcessSubSessionForeground, TestSize.Level1)
{
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->ProcessSubSessionForeground(sceneSession);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->ProcessSubSessionForeground(sceneSession);

    sptr<SceneSession> sub1 = nullptr;
    sptr<SceneSession> sub2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::vector<sptr<SceneSession>> subs;
    std::vector<sptr<SceneSession>> dialogs;
    subs.push_back(sub1);
    subs.push_back(sub2);
    dialogs.push_back(sub1);
    dialogs.push_back(sub2);
    sceneSession->subSession_ = subs;
    ssm_->ProcessSubSessionForeground(sceneSession);
    sptr<SceneSession> sub3 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sub3->state_ = SessionState::STATE_FOREGROUND;
    subs.push_back(sub3);
    dialogs.push_back(sub3);
    sceneSession->subSession_ = subs;
    ssm_->ProcessSubSessionForeground(sceneSession);
    sptr<SceneSession> sub4 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sub4->state_ = SessionState::STATE_FOREGROUND;
    sub4->persistentId_ = 100;
    subs.push_back(sub4);
    dialogs.push_back(sub4);
    sceneSession->subSession_ = subs;
    ssm_->ProcessSubSessionForeground(sceneSession);

    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->ProcessSubSessionForeground(sceneSession);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;
    ssm_->ProcessSubSessionForeground(sceneSession);
    ASSERT_NE(nullptr, ssm_);
}

/**
 * @tc.name: ConfigSystemUIStatusBar
 * @tc.desc: call ConfigSystemUIStatusBar default.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ConfigSystemUIStatusBar02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<systemUIStatusBar>"
        "</systemUIStatusBar>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(sceneSessionManager, nullptr);
    sceneSessionManager->ConfigWindowSceneXml();
}

/**
 * @tc.name: ClosePipWindowIfExist
 * @tc.desc: ClosePipWindowIfExist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ClosePipWindowIfExist, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->ClosePipWindowIfExist(WindowType::WINDOW_TYPE_PIP);

    SessionInfo info;
    info.sessionState_ = { 1 };
    Rect reqRect = { 0, 0, 10, 10 };
    property->SetRequestRect(reqRect);
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    ASSERT_EQ(false, ssm_->IsEnablePiPCreate(property));
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: RecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RecoverAndConnectSpecificSession, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetParentId(1);
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    sptr<ISession> session;
    sptr<IRemoteObject> token;
    auto result =
        ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateGestureBackEnabled
 * @tc.desc: UpdateGestureBackEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, UpdateGestureBackEnabled, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<SceneSession> nextSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(nextSession, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    nextSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->sceneSessionMap_.insert({ 2, nextSession });
    sceneSession->persistentId_ = 1;
    nextSession->persistentId_ = 2;

    sceneSession->isEnableGestureBack_ = false;
    sceneSession->isEnableGestureBackHadSet_ = true;
    ssm_->UpdateGestureBackEnabled(1);
    sceneSession->isEnableGestureBack_ = true;
    ssm_->UpdateGestureBackEnabled(1);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->NotifyEnterRecentTask(false);
    EXPECT_EQ(ssm_->enterRecent_.load(), false);
    sceneSession->UpdateFocus(true);
    EXPECT_EQ(sceneSession->IsFocused(), true);
    ssm_->UpdateGestureBackEnabled(1, 2);
    sleep(WAIT_SLEEP_TIME);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->UpdateGestureBackEnabled(1, 2);
    sleep(WAIT_SLEEP_TIME);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ssm_->UpdateGestureBackEnabled(1);
    sleep(WAIT_SLEEP_TIME);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    ssm_->UpdateGestureBackEnabled(1);
    sleep(WAIT_SLEEP_TIME);

    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    ssm_->UpdateGestureBackEnabled(1);
    sleep(WAIT_SLEEP_TIME);

    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->NotifyEnterRecentTask(true);
    ssm_->UpdateGestureBackEnabled(1);
    sleep(WAIT_SLEEP_TIME);

    ssm_->NotifyEnterRecentTask(false);
    ssm_->UpdateGestureBackEnabled(1, 2);
    sleep(WAIT_SLEEP_TIME);
    sceneSession->UpdateFocus(false);
    ssm_->UpdateGestureBackEnabled(1);
    sleep(WAIT_SLEEP_TIME);

    ssm_->UpdateGestureBackEnabled(1, 3);
    sleep(WAIT_SLEEP_TIME);
    nextSession->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ssm_->UpdateGestureBackEnabled(1, 2);
    sleep(WAIT_SLEEP_TIME);
    sceneSession->UpdateFocus(true);
    ssm_->UpdateGestureBackEnabled(1, 2);
    sleep(WAIT_SLEEP_TIME);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession02
 * @tc.desc: RecoverAndConnectSpecificSession02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RecoverAndConnectSpecificSession02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "RecoverAndConnectSpecificSession02";
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_END);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetSessionInfo(sessionInfo);
    property->SetPersistentId(1);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    sptr<ISession> session;
    sptr<IRemoteObject> token;
    ASSERT_NE(ssm_, nullptr);
    auto result =
        ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetAlivePersistentIds
 * @tc.desc: SetAlivePersistentIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, SetAlivePersistentIds, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<int32_t> recoveredPersistentIds = { 0, 1, 2 };
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    ASSERT_EQ(ssm_->alivePersistentIds_, recoveredPersistentIds);
}

/**
 * @tc.name: NotifyCreateToastSession
 * @tc.desc: NotifyCreateToastSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, NotifyCreateToastSession, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->NotifyCreateToastSession(1, nullptr);
    SessionInfo Info;
    Info.persistentId_ = 1;
    int32_t persistentId = Info.persistentId_;
    Info.abilityName_ = "testInfo1a";
    Info.bundleName_ = "testInfo1b";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(Info, nullptr);
    ssm_->NotifyCreateToastSession(persistentId, session);
}

/**
 * @tc.name: RecoverCachedDialogSession
 * @tc.desc: RecoverCachedDialogSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, RecoverCachedDialogSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(sceneSession, nullptr);

    int32_t parentPersistentId = 1;
    ssm_->RecoverCachedDialogSession(parentPersistentId);
    EXPECT_EQ(ssm_->recoverDialogSessionCacheMap_[parentPersistentId].size(), 0);
    ssm_->recoverDialogSessionCacheMap_[parentPersistentId].emplace_back(sceneSession);
    EXPECT_EQ(ssm_->recoverDialogSessionCacheMap_[parentPersistentId].size(), 1);
    ssm_->recoverDialogSessionCacheMap_[parentPersistentId].emplace_back(sceneSession);
    EXPECT_EQ(ssm_->recoverDialogSessionCacheMap_[parentPersistentId].size(), 2);
    ssm_->RecoverCachedDialogSession(parentPersistentId);
    EXPECT_EQ(ssm_->recoverDialogSessionCacheMap_[parentPersistentId].size(), 0);
}

/**
 * @tc.name: ExtractSupportWindowModeFromMetaData
 * @tc.desc: ExtractSupportWindowModeFromMetaData
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ExtractSupportWindowModeFromMetaData, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(sceneSession, nullptr);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    AppExecFwk::AbilityInfo abilityInfo;
    int ret = 0;
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes =
        ssm_->ExtractSupportWindowModeFromMetaData(std::make_shared<OHOS::AppExecFwk::AbilityInfo>(abilityInfo));
    EXPECT_EQ(ret, 0);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->systemConfig_.freeMultiWindowEnable_ = false;
    updateWindowModes =
        ssm_->ExtractSupportWindowModeFromMetaData(std::make_shared<OHOS::AppExecFwk::AbilityInfo>(abilityInfo));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ParseWindowModeFromMetaData
 * @tc.desc: ParseWindowModeFromMetaData
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest2, ParseWindowModeFromMetaData, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(sceneSession, nullptr);

    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = { AppExecFwk::SupportWindowMode::FULLSCREEN,
                                                                     AppExecFwk::SupportWindowMode::SPLIT,
                                                                     AppExecFwk::SupportWindowMode::FLOATING };
    ASSERT_EQ(updateWindowModes, ssm_->ParseWindowModeFromMetaData("fullscreen,split,floating"));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
