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
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "context.h"
#include "session_manager/include/scene_session_dirty_manager.h"

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
class SceneSessionManagerTest3 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static bool gestureNavigationEnabled_;
    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
};

sptr<SceneSessionManager> SceneSessionManagerTest3::ssm_ = nullptr;

bool SceneSessionManagerTest3::gestureNavigationEnabled_ = true;
ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest3::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void WindowChangedFuncTest3(int32_t persistentId, WindowUpdateType type) {}

void ProcessStatusBarEnabledChangeFuncTest(bool enable, const std::string& bundleName) {}

void SceneSessionManagerTest3::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest3::TearDownTestCase()
{
    ssm_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

void SceneSessionManagerTest3::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest3::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    ssm_->sceneSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: ConfigDecor
 * @tc.desc: SceneSesionManager config decor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigDecor, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem* item = new (std::nothrow) WindowSceneConfig::ConfigItem;
    ASSERT_NE(nullptr, item);
    ssm_->ConfigDecor(*item);
    delete item;
}

/**
 * @tc.name: ConfigWindowEffect
 * @tc.desc: SceneSesionManager config window effect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigWindowEffect, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem* item = new (std::nothrow) WindowSceneConfig::ConfigItem;
    ASSERT_NE(nullptr, item);
    ssm_->ConfigWindowEffect(*item);
    delete item;
}

/**
 * @tc.name: ConfigAppWindowCornerRadius
 * @tc.desc: SceneSesionManager config app window coener radius
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigAppWindowCornerRadius, TestSize.Level1)
{
    float out = 0.0f;
    WindowSceneConfig::ConfigItem item01;
    std::string str = "defaultCornerRadiusM";
    item01.SetValue(str);
    bool result01 = ssm_->ConfigAppWindowCornerRadius(item01, out);
    EXPECT_EQ(result01, true);

    WindowSceneConfig::ConfigItem item02;
    item02.SetValue(new string("defaultCornerRadiusS"));
    bool result02 = ssm_->ConfigAppWindowCornerRadius(item02, out);
    EXPECT_EQ(result02, false);
    EXPECT_EQ(out, 12.0f);
}

/**
 * @tc.name: ConfigAppWindowShadow
 * @tc.desc: SceneSesionManager config app window shadow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigAppWindowShadow, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem item;
    WindowSceneConfig::ConfigItem shadowConfig;
    WindowShadowConfig outShadow;
    std::vector<float> floatTest = { 0.0f, 0.1f, 0.2f, 0.3f };
    bool result01 = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result01, true);

    item.SetValue(floatTest);
    shadowConfig.SetValue({ { "radius", item } });
    bool result02 = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result02, false);
    shadowConfig.SetValue({ { "alpha", item } });
    bool result03 = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result03, false);
    shadowConfig.SetValue({ { "sffsetY", item } });
    bool result04 = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result04, true);
    shadowConfig.SetValue({ { "sffsetX", item } });
    bool result05 = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result05, true);

    item.SetValue(new std::string("color"));
    shadowConfig.SetValue({ { "color", item } });
    bool result06 = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result06, true);
}

/**
 * @tc.name: ConfigStatusBar
 * @tc.desc: ConfigStatusBar config window immersive status bar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigStatusBar, TestSize.Level1)
{
    StatusBarConfig out;
    WindowSceneConfig::ConfigItem enable;
    enable.SetValue(true);
    WindowSceneConfig::ConfigItem showHide;
    showHide.SetProperty({ { "enable", enable } });
    WindowSceneConfig::ConfigItem item01;
    WindowSceneConfig::ConfigItem contentColor;
    contentColor.SetValue(std::string("#12345678"));
    WindowSceneConfig::ConfigItem backgroundColor;
    backgroundColor.SetValue(std::string("#12345678"));
    item01.SetValue(
        { { "showHide", showHide }, { "contentColor", contentColor }, { "backgroundColor", backgroundColor } });
    bool result01 = ssm_->ConfigStatusBar(item01, out);
    EXPECT_EQ(result01, true);
    EXPECT_EQ(out.showHide_, true);
    EXPECT_EQ(out.contentColor_, "#12345678");
    EXPECT_EQ(out.backgroundColor_, "#12345678");
}

/**
 * @tc.name: ConfigKeyboardAnimation
 * @tc.desc: SceneSesionManager config keyboard animation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigWindowImmersive, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<immersive>"
        "<inDesktopStatusBarConfig>"
        "<showHide enable=\"true\"/>"
        "<backgroundColor>#12341234</backgroundColor>"
        "<contentColor>#12341234</contentColor>"
        "</inDesktopStatusBarConfig>"
        "<inSplitStatusBarConfig>"
        "<upDownSplit>"
        "<showHide enable=\"true\"/>"
        "<backgroundColor>#12341234</backgroundColor>"
        "<contentColor>#12341234</contentColor>"
        "</upDownSplit>"
        "<leftRightSplit>"
        "<showHide enable=\"true\"/>"
        "<backgroundColor>#12341234</backgroundColor>"
        "<contentColor>#12341234</contentColor>"
        "</leftRightSplit>"
        "</inSplitStatusBarConfig>"
        "</immersive>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.desktopStatusBarConfig_.showHide_, true);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.desktopStatusBarConfig_.backgroundColor_, "#12341234");
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.desktopStatusBarConfig_.contentColor_, "#12341234");
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.upDownStatusBarConfig_.showHide_, true);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.upDownStatusBarConfig_.backgroundColor_, "#12341234");
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.upDownStatusBarConfig_.contentColor_, "#12341234");
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.leftRightStatusBarConfig_.showHide_, true);
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.leftRightStatusBarConfig_.backgroundColor_, "#12341234");
    EXPECT_EQ(ssm_->appWindowSceneConfig_.windowImmersive_.leftRightStatusBarConfig_.contentColor_, "#12341234");
}

/**
 * @tc.name: ConfigWindowAnimation
 * @tc.desc: SceneSesionManager config window animation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigWindowAnimation, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem windowAnimationConfig;
    WindowSceneConfig::ConfigItem item;
    std::vector<float> opacity = { 0.1f };
    std::vector<float> translate = { 0.1f, 0.2f };
    std::vector<float> rotation = { 0.1f, 0.2f, 0.3f, 0.4f };
    std::vector<float> scale = { 0.1f, 0.2f };
    std::vector<int> duration = { 39 };

    item.SetValue(opacity);
    windowAnimationConfig.SetValue({ { "opacity", item } });
    EXPECT_TRUE(windowAnimationConfig.IsMap());
    ssm_->ConfigWindowAnimation(windowAnimationConfig);

    item.SetValue(rotation);
    windowAnimationConfig.SetValue({ { "rotation", item } });
    EXPECT_TRUE(windowAnimationConfig.IsMap());
    ssm_->ConfigWindowAnimation(windowAnimationConfig);

    item.SetValue(translate);
    windowAnimationConfig.SetValue({ { "translate", item } });
    EXPECT_TRUE(windowAnimationConfig.IsMap());
    ssm_->ConfigWindowAnimation(windowAnimationConfig);

    item.SetValue(scale);
    windowAnimationConfig.SetValue({ { "scale", item } });
    EXPECT_TRUE(windowAnimationConfig.IsMap());
    ssm_->ConfigWindowAnimation(windowAnimationConfig);

    item.SetValue(duration);
    item.SetValue({ { "duration", item } });
    windowAnimationConfig.SetValue({ { "timing", item } });
    ssm_->ConfigWindowAnimation(windowAnimationConfig);

    item.SetValue(duration);
    item.SetValue({ { "curve", item } });
    windowAnimationConfig.SetValue({ { "timing", item } });
    ssm_->ConfigWindowAnimation(windowAnimationConfig);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: check func RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, RecoverAndReconnectSceneSession, TestSize.Level1)
{
    sptr<ISession> session;
    auto result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, nullptr, nullptr);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = { 0, 1, 2 };
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    property->SetPersistentId(1);
    result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, property, nullptr);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: ConfigStartingWindowAnimation
 * @tc.desc: SceneSesionManager config start window animation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigStartingWindowAnimation, TestSize.Level1)
{
    std::vector<float> midFloat = { 0.1f };
    std::vector<int> midInt = { 1 };
    WindowSceneConfig::ConfigItem middleFloat;
    middleFloat.SetValue(midFloat);
    WindowSceneConfig::ConfigItem middleInt;
    middleInt.SetValue(midInt);
    WindowSceneConfig::ConfigItem curve;
    curve.SetValue(midFloat);
    curve.SetValue({ { "curve", curve } });
    WindowSceneConfig::ConfigItem enableConfigItem;
    enableConfigItem.SetValue(false);
    EXPECT_EQ(enableConfigItem.boolValue_, false);
    std::map<std::string, WindowSceneConfig::ConfigItem> midMap = { { "duration", middleInt }, { "curve", curve } };
    WindowSceneConfig::ConfigItem timing;
    timing.SetValue(midMap);
    std::map<std::string, WindowSceneConfig::ConfigItem> middleMap = { { "enable", enableConfigItem },
                                                                       { "timing", timing },
                                                                       { "opacityStart", middleFloat },
                                                                       { "opacityEnd", middleFloat } };
    WindowSceneConfig::ConfigItem configItem;
    configItem.SetValue(middleMap);
    ssm_->ConfigStartingWindowAnimation(configItem);
    midMap.clear();
    middleMap.clear();
}

/**
 * @tc.name: CreateCurve
 * @tc.desc: SceneSesionManager create curve
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, CreateCurve, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem curveConfig;
    std::string result01 = std::get<std::string>(ssm_->CreateCurve(curveConfig));
    EXPECT_EQ(result01, "easeOut");

    std::string value02 = "userName";
    curveConfig.SetValue(value02);
    curveConfig.SetValue({ { "name", curveConfig } });
    std::string result02 = std::get<std::string>(ssm_->CreateCurve(curveConfig));
    EXPECT_EQ(result02, "easeOut");

    std::string value03 = "interactiveSpring";
    curveConfig.SetValue(value03);
    curveConfig.SetValue({ { "name", curveConfig } });
    std::string result03 = std::get<std::string>(ssm_->CreateCurve(curveConfig));
    EXPECT_EQ(result03, "easeOut");

    std::string value04 = "cubic";
    curveConfig.SetValue(value04);
    curveConfig.SetValue({ { "name", curveConfig } });
    std::string result04 = std::get<std::string>(ssm_->CreateCurve(curveConfig));
    EXPECT_EQ(result04, "easeOut");
}

/**
 * @tc.name: GetRootSceneSession
 * @tc.desc: SceneSesionManager get root scene session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetRootSceneSession, TestSize.Level1)
{
    sptr<RootSceneSession> res = ssm_->GetRootSceneSession();
    EXPECT_EQ(res, ssm_->rootSceneSession_);
}

/**
 * @tc.name: GetSceneSession
 * @tc.desc: SceneSesionManager get scene session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetSceneSession, TestSize.Level1)
{
    int32_t persistentId = 65535;
    ASSERT_EQ(ssm_->GetSceneSession(persistentId), nullptr);
}

/**
 * @tc.name: GetSceneSession002
 * @tc.desc: SceneSesionManager get scene session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetSceneSession002, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 65535, sceneSession });
    int32_t persistentId = 65535;
    ASSERT_NE(ssm_->GetSceneSession(persistentId), nullptr);
}

/**
 * @tc.name: GetSceneSessionByIdentityInfo
 * @tc.desc: SceneSesionManager get scene session by name
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetSceneSessionByIdentityInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test3";
    info.appIndex_ = 10;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    std::string bundleName1 = "test1";
    std::string moduleName1 = "test2";
    std::string abilityName1 = "test3";
    int32_t appIndex1 = 10;
    SessionIdentityInfo identityInfo = { bundleName1, moduleName1, abilityName1, appIndex1 };
    ASSERT_EQ(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::string bundleName2 = "test11";
    std::string moduleName2 = "test22";
    std::string abilityName2 = "test33";
    int32_t appIndex2 = 100;
    ASSERT_EQ(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
    identityInfo = { bundleName1, moduleName2, abilityName2, appIndex2 };
    ASSERT_EQ(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
    identityInfo = { bundleName2, moduleName1, abilityName2, appIndex2 };
    ASSERT_EQ(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
    identityInfo = { bundleName2, moduleName2, abilityName1, appIndex2 };
    ASSERT_EQ(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
    identityInfo = { bundleName2, moduleName2, abilityName2, appIndex1 };
    ASSERT_EQ(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: GetSceneSessionVectorByTypeAndDisplayId
 * @tc.desc: SceneSesionManager get scene session vector by type
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetSceneSessionVectorByTypeAndDisplayId, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint64_t displayId = -1ULL;
    ssm_->GetSceneSessionVectorByTypeAndDisplayId(WindowType::APP_MAIN_WINDOW_BASE, displayId);
    displayId = 1;
    ssm_->GetSceneSessionVectorByTypeAndDisplayId(WindowType::APP_MAIN_WINDOW_BASE, displayId);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test3";
    info.appIndex_ = 10;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetSceneSessionVectorByTypeAndDisplayId(WindowType::APP_MAIN_WINDOW_BASE, displayId);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ssm_->GetSceneSessionVectorByTypeAndDisplayId(WindowType::APP_MAIN_WINDOW_BASE, displayId);
    sceneSession->property_->SetDisplayId(1);
    ssm_->GetSceneSessionVectorByTypeAndDisplayId(WindowType::APP_MAIN_WINDOW_BASE, displayId);
    ssm_->sceneSessionMap_.erase(1);
    EXPECT_FALSE(g_logMsg.find("displayId is invalid") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetWindowLimits
 * @tc.desc: GetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetWindowLimits, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test3";
    info.appIndex_ = 10;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    WindowLimits limits;
    limits.maxHeight_ = 1000;
    limits.minHeight_ = 500;
    limits.maxWidth_ = 1000;
    limits.minWidth_ = 500;
    sceneSession->property_->SetWindowLimits(limits);
    sceneSession->property_->SetWindowLimitsVP(limits);

    int32_t windowId = 1;
    WindowLimits windowlimits;
    ssm_->sceneSessionMap_.insert({ windowId, sceneSession });
    auto defaultUIType = ssm_->systemConfig_.windowUIType_;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    ssm_->systemConfig_.freeMultiWindowEnable_ = false;
    ssm_->systemConfig_.freeMultiWindowSupport_ = false;
    auto ret = ssm_->GetWindowLimits(windowId, windowlimits);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    ret = ssm_->GetWindowLimits(windowId, windowlimits);
    EXPECT_EQ(ret, WMError::WM_OK);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = ssm_->GetWindowLimits(windowId, windowlimits);
    ssm_->sceneSessionMap_.erase(windowId);
    ssm_->systemConfig_.windowUIType_ = defaultUIType;
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_EQ(windowlimits.maxHeight_, 1000);
    EXPECT_EQ(windowlimits.minHeight_, 500);
    EXPECT_EQ(windowlimits.maxWidth_, 1000);
    EXPECT_EQ(windowlimits.minWidth_, 500);
}

/**
 * @tc.name: CheckWindowId
 * @tc.desc: CheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, CheckWindowId, TestSize.Level1)
{
    int32_t windowId = 1;
    int32_t pid = 2;
    ssm_->CheckWindowId(windowId, pid);
    ssm_->sceneSessionMap_.insert({ windowId, nullptr });
    ssm_->CheckWindowId(windowId, pid);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ windowId, sceneSession });
    ssm_->CheckWindowId(windowId, pid);
    ssm_->PerformRegisterInRequestSceneSession(sceneSession);
    ssm_->sceneSessionMap_.erase(windowId);
}

/**
 * @tc.name: CheckAppIsInDisplay
 * @tc.desc: CheckAppIsInDisplay
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, CheckAppIsInDisplay, TestSize.Level1)
{
    int ret = 0;
    sptr<SceneSession> sceneSession;
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    SessionInfo info;
    ret++;
    ssm_->DestroyDialogWithMainWindow(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->DestroyDialogWithMainWindow(sceneSession);
    ssm_->DestroySubSession(sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->SetSessionProperty(property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    AppExecFwk::Configuration config;
    ssm_->SetAbilitySessionInfo(sceneSession);
    bool res = true;
    ssm_->PrepareTerminate(1, res);
    ssm_->isPrepareTerminateEnable_ = true;
    ssm_->PrepareTerminate(1, res);
    ssm_->isPrepareTerminateEnable_ = false;
    ssm_->PrepareTerminate(1, res);
    ssm_->StartUIAbilityBySCB(sceneSession);
    ssm_->sceneSessionMap_.insert({ 1, nullptr });
    ssm_->IsKeyboardForeground();
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, true);
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, false);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ssm_->IsKeyboardForeground();
    ssm_->StartUIAbilityBySCB(sceneSession);
    ssm_->PrepareTerminate(1, res);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->DestroyDialogWithMainWindow(sceneSession);
    ssm_->sceneSessionMap_.erase(1);
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(nullptr, abilitySessionInfo);
    ssm_->StartUIAbilityBySCB(abilitySessionInfo, sceneSession);
    ssm_->DestroySubSession(sceneSession);
    ssm_->EraseSceneSessionMapById(2);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: StartUIAbilityBySCB
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, StartUIAbilityBySCB, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "StartUIAbilityBySCB";
    info.bundleName_ = "StartUIAbilityBySCB";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    int32_t ret = ssm_->StartUIAbilityBySCB(sceneSession);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: ChangeUIAbilityVisibilityBySCB
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ChangeUIAbilityVisibilityBySCB, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ChangeUIAbilityVisibilityBySCB";
    info.bundleName_ = "ChangeUIAbilityVisibilityBySCB";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    int32_t ret = ssm_->ChangeUIAbilityVisibilityBySCB(sceneSession, true, false);
    EXPECT_EQ(ret, 2097202);
    ret = ssm_->ChangeUIAbilityVisibilityBySCB(sceneSession, true, false, false);
    EXPECT_EQ(ret, 2097202);
    ret = ssm_->ChangeUIAbilityVisibilityBySCB(sceneSession, true, false, true);
    EXPECT_EQ(ret, 2097202);
}

/**
 * @tc.name: SetAbilitySessionInfo
 * @tc.desc: SceneSesionManager set ability session info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetAbilitySessionInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAbilitySessionInfo";
    info.bundleName_ = "SetAbilitySessionInfo";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<OHOS::AAFwk::SessionInfo> ret = ssm_->SetAbilitySessionInfo(sceneSession);
    OHOS::AppExecFwk::ElementName retElementName = ret->want.GetElement();
    EXPECT_EQ(retElementName.GetAbilityName(), info.abilityName_);
    EXPECT_EQ(retElementName.GetBundleName(), info.bundleName_);
}

/**
 * @tc.name: SetAbilitySessionInfo2
 * @tc.desc: SceneSesionManager set ability session info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetAbilitySessionInfo2, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAbilitySessionInfo";
    info.bundleName_ = "SetAbilitySessionInfo";
    std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>();
    wantPtr->SetBundle(info.bundleName_);
    info.want = wantPtr;
    info.persistentId_ = 1;
    info.requestId = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->SetAbilitySessionInfo(sceneSession, true);
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    sptr<OHOS::AAFwk::SessionInfo> ret = ssm_->SetAbilitySessionInfo(sceneSession, true);
    ssm_->ClearRequestTaskInfo(1);
    EXPECT_EQ(ret->want.GetBundle(), "SetAbilitySessionInfo");
}

/**
 * @tc.name: SetAbilitySessionInfo3
 * @tc.desc: SceneSesionManager set ability session info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetAbilitySessionInfo3, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAbilitySessionInfo3";
    info.bundleName_ = "SetAbilitySessionInfo3";
    info.isRestartApp_ = true;
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({1, sceneSession});

    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "SetAbilitySessionInfo3";
    sessionInfo.bundleName_ = "SetAbilitySessionInfo3";
    sessionInfo.isRestartApp_ = true;
    sessionInfo.persistentId_ = 2;
    sessionInfo.callerPersistentId_ = 1;
    sptr<WindowSessionProperty> property;
    sptr<SceneSession> createdSceneSession = ssm_->CreateSceneSession(sessionInfo, property);
    ASSERT_NE(nullptr, createdSceneSession);

    sptr<OHOS::AAFwk::SessionInfo> ret = ssm_->SetAbilitySessionInfo(sceneSession);
    ASSERT_NE(nullptr, ret);

    ret = ssm_->SetAbilitySessionInfo(createdSceneSession);
    ASSERT_NE(nullptr, ret);
}
 
/**
 * @tc.name: AddRequestTaskInfo
 * @tc.desc: SceneSesionManager add request
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, AddRequestTaskInfo, TestSize.Level1)
{
    SessionInfo infoTmp;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(infoTmp, nullptr);
    SessionInfo& info = sceneSession->sessionInfo_;
    info.abilityName_ = "AddRequestTaskInfo";
    info.bundleName_ = "AddRequestTaskInfo";
    ssm_->AddRequestTaskInfo(nullptr, info.requestId);
    info.requestId = -1;
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    info.requestId = 1;
    sceneSession->persistentId_ = INVALID_SESSION_ID;
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    sceneSession->persistentId_ = 1;
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>();
    info.want = wantPtr;
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    std::shared_ptr<AAFwk::Want> wantRet = ssm_->GetRequestWantFromTaskInfoMap(1, 1);
    EXPECT_NE(wantRet, nullptr);
    ssm_->ClearRequestTaskInfo(1);
}

/**
 * @tc.name: RemoveRequestTaskInfo
 * @tc.desc: SceneSesionManager remove request task info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, RemoveRequestTaskInfo, TestSize.Level1)
{
    SessionInfo infoTmp;
    infoTmp.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(infoTmp, nullptr);
    SessionInfo& info = sceneSession->sessionInfo_;
    info.abilityName_ = "RemoveRequestTaskInfo";
    info.bundleName_ = "RemoveRequestTaskInfo";
    info.requestId = 1;
    info.persistentId_ = 1;
    std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>();
    info.want = wantPtr;
    ssm_->ClearRequestTaskInfo(0);
    ssm_->RemoveRequestTaskInfo(-1, 0);
    ssm_->RemoveRequestTaskInfo(-1, 1);
    ssm_->RemoveRequestTaskInfo(1, 0);
    ssm_->RemoveRequestTaskInfo(1, 1);
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    ssm_->RemoveRequestTaskInfo(1, 1);
    std::shared_ptr<AAFwk::Want> wantRet = ssm_->GetRequestWantFromTaskInfoMap(1, 1);
    EXPECT_EQ(wantRet, nullptr);
    ssm_->ClearRequestTaskInfo(1);
}
 
/**
 * @tc.name: GetRequestWantFromTaskInfoMap
 * @tc.desc: SceneSesionManager get request want from task info map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetRequestWantFromTaskInfoMap, TestSize.Level1)
{
    SessionInfo infoTmp;
    infoTmp.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(infoTmp, nullptr);
    SessionInfo& info = sceneSession->sessionInfo_;
    info.abilityName_ = "GetRequestWantFromTaskInfoMap";
    info.bundleName_ = "GetRequestWantFromTaskInfoMap";
    info.requestId = 1;
    info.persistentId_ = 1;
    std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>();
    info.want = wantPtr;
    ssm_->GetRequestWantFromTaskInfoMap(-1, 0);
    ssm_->GetRequestWantFromTaskInfoMap(-1, 1);
    ssm_->GetRequestWantFromTaskInfoMap(1, 0);
    ssm_->GetRequestWantFromTaskInfoMap(1, 1);
    ssm_->AddRequestTaskInfo(sceneSession, info.requestId);
    ssm_->GetRequestWantFromTaskInfoMap(1, 1);
    ssm_->RemoveRequestTaskInfo(1, 1);
    std::shared_ptr<AAFwk::Want> wantRet = ssm_->GetRequestWantFromTaskInfoMap(1, 1);
    EXPECT_EQ(wantRet, nullptr);
    ssm_->ClearRequestTaskInfo(1);
}

/**
 * @tc.name: PrepareTerminate
 * @tc.desc: SceneSesionManager prepare terminate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, PrepareTerminate, TestSize.Level1)
{
    int32_t persistentId = 0;
    bool isPrepareTerminate = false;
    ASSERT_EQ(WSError::WS_OK, ssm_->PrepareTerminate(persistentId, isPrepareTerminate));
}

/**
 * @tc.name: DestroyDialogWithMainWindow
 * @tc.desc: SceneSesionManager destroy dialog with main window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, DestroyDialogWithMainWindow, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DestroyDialogWithMainWindow";
    info.bundleName_ = "DestroyDialogWithMainWindow";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    EXPECT_EQ(WSError::WS_OK, ssm_->DestroyDialogWithMainWindow(sceneSession));
}

/**
 * @tc.name: AddClientDeathRecipient
 * @tc.desc: SceneSesionManager add client death recipient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, AddClientDeathRecipient, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "AddClientDeathRecipient";
    info.bundleName_ = "AddClientDeathRecipient";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->AddClientDeathRecipient(nullptr, sceneSession);
    EXPECT_FALSE(g_logMsg.find("failed to add death recipient") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: DestroySpecificSession
 * @tc.desc: SceneSesionManager destroy specific session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, DestroySpecificSession, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<IRemoteObject> remoteObject = nullptr;
    ssm_->DestroySpecificSession(remoteObject);
    EXPECT_TRUE(g_logMsg.find("Invalid remoteObject") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetCreateSystemSessionListener
 * @tc.desc: SceneSesionManager set create specific session listener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetCreateSystemSessionListener, TestSize.Level1)
{
    ssm_->SetCreateSystemSessionListener(nullptr);
    ASSERT_EQ(nullptr, ssm_->createSystemSessionFunc_);
}

/**
 * @tc.name: SetGestureNavigationEnabledChangeListener
 * @tc.desc: SceneSesionManager set gesture navigation enabled change listener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetGestureNavigationEnabledChangeListener, TestSize.Level1)
{
    ssm_->SetGestureNavigationEnabledChangeListener(nullptr);
    ASSERT_EQ(nullptr, ssm_->gestureNavigationEnabledChangeFunc_);
}

/**
 * @tc.name: OnOutsideDownEvent
 * @tc.desc: SceneSesionManager on out side down event
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, OnOutsideDownEvent, TestSize.Level1)
{
    int32_t x = 32;
    int32_t y = 32;
    int32_t ret = 0;
    ProcessOutsideDownEventFunc func = [&ret](int32_t x, int32_t y) {
        ret = x + y;
    };
    auto tempFunc = ssm_->outsideDownEventFunc_;
    ssm_->outsideDownEventFunc_ = func;
    ssm_->OnOutsideDownEvent(x, y);
    EXPECT_EQ(ret, 64);
    ssm_->outsideDownEventFunc_ = tempFunc;
}

/**
 * @tc.name: IsSameDisplayGroupId
 * @tc.desc: test IsSameDisplayGroupId01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, IsSameDisplayGroupId01, TestSize.Level1)
{
    bool result = ssm_->IsSameDisplayGroupId(nullptr, 0);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSameDisplayGroupId
 * @tc.desc: test IsSameDisplayGroupId02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, IsSameDisplayGroupId02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsSameDisplayGroupId02";
    info.bundleName_ = "IsSameDisplayGroupId02";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetDisplayId(DEFAULT_DISPLAY_ID);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionProperty(property);
    bool result = ssm_->IsSameDisplayGroupId(sceneSession, 0);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetWindowSceneConfig
 * @tc.desc: SceneSesionManager get window scene config
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetWindowSceneConfig, TestSize.Level1)
{
    AppWindowSceneConfig appWindowSceneConfig_ = ssm_->GetWindowSceneConfig();
    EXPECT_EQ(appWindowSceneConfig.backgroundScreenLock_, ssm_->appWindowSceneConfig_.backgroundScreenLock_);
}

/**
 * @tc.name: ProcessBackEvent
 * @tc.desc: SceneSesionManager process back event
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ProcessBackEvent, TestSize.Level1)
{
    WSError result = ssm_->ProcessBackEvent();
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: HandleUserSwitch
 * @tc.desc: SceneSesionManager notify switching user
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, HandleUserSwitch, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->HandleUserSwitch(UserSwitchEventType::SWITCHING, true);
    ssm_->HandleUserSwitch(UserSwitchEventType::SWITCHED, true);
    ssm_->HandleUserSwitch(UserSwitchEventType::SWITCHING, false);
    ssm_->HandleUserSwitch(UserSwitchEventType::SWITCHED, false);
    EXPECT_FALSE(g_logMsg.find("Invalid switchEventType") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandleUserSwitch1
 * @tc.desc: notify switching user on PC
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, HandleUserSwitch1, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.backgroundswitch = true;
    SessionInfo info;
    info.abilityName_ = "IsNeedChangeLifeCycleOnUserSwitch3";
    info.bundleName_ = "IsNeedChangeLifeCycleOnUserSwitch3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetCallingPid(45678);
    property->SetIsAppSupportPhoneInPc(false);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), nullptr });
    ssm_->HandleUserSwitch(UserSwitchEventType::SWITCHING, false);
    ssm_->HandleUserSwitch(UserSwitchEventType::SWITCHED, false);
}

/**
 * @tc.name: GetSessionInfoByContinueSessionId
 * @tc.desc: SceneSesionManager GetSessionInfoByContinueSessionId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetSessionInfoByContinueSessionId, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    std::string continueSessionId = "";
    SessionInfoBean missionInfo;
    EXPECT_EQ(ssm_->GetSessionInfoByContinueSessionId(continueSessionId, missionInfo),
              WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: PreHandleCollaborator
 * @tc.desc: SceneSesionManager prehandle collaborator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, PreHandleCollaborator, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "PreHandleCollaborator";
    info.bundleName_ = "PreHandleCollaborator";
    sptr<SceneSession> sceneSession = nullptr;
    bool result = ssm_->PreHandleCollaborator(sceneSession);
    EXPECT_FALSE(result);

    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    result = ssm_->PreHandleCollaborator(sceneSession);
    EXPECT_TRUE(result);

    sceneSession = nullptr;
    AppExecFwk::ApplicationInfo applicationInfo_;
    applicationInfo_.codePath = std::to_string(CollaboratorType::RESERVE_TYPE);
    AppExecFwk::AbilityInfo abilityInfo_;
    abilityInfo_.applicationInfo = applicationInfo_;
    info.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo_);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    result = ssm_->PreHandleCollaborator(sceneSession);
    EXPECT_TRUE(result);

    sceneSession = nullptr;
    applicationInfo_.codePath = std::to_string(CollaboratorType::OTHERS_TYPE);
    abilityInfo_.applicationInfo = applicationInfo_;
    info.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo_);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    result = ssm_->PreHandleCollaborator(sceneSession);
    EXPECT_TRUE(result);

    EXPECT_EQ(sceneSession->GetSessionInfo().want, nullptr);
    sceneSession = nullptr;
    info.want = std::make_shared<AAFwk::Want>();
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    result = ssm_->PreHandleCollaborator(sceneSession);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCollaboratorType
 * @tc.desc: SceneSesionManager check collborator type
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, CheckCollaboratorType, TestSize.Level1)
{
    int32_t type = CollaboratorType::RESERVE_TYPE;
    EXPECT_TRUE(ssm_->CheckCollaboratorType(type));
    type = CollaboratorType::OTHERS_TYPE;
    EXPECT_TRUE(ssm_->CheckCollaboratorType(type));
    type = CollaboratorType::REDIRECT_TYPE;
    EXPECT_TRUE(ssm_->CheckCollaboratorType(type));
    type = CollaboratorType::DEFAULT_TYPE;
    ASSERT_FALSE(ssm_->CheckCollaboratorType(type));
}

/**
 * @tc.name: NotifyUpdateSessionInfo
 * @tc.desc: SceneSesionManager notify update session info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, NotifyUpdateSessionInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyUpdateSessionInfo";
    info.bundleName_ = "NotifyUpdateSessionInfo";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->NotifyUpdateSessionInfo(nullptr);
    ASSERT_EQ(sceneSession->GetSessionInfo().want, nullptr);
    ssm_->NotifyUpdateSessionInfo(sceneSession);
    int32_t collaboratorType = CollaboratorType::RESERVE_TYPE;
    int32_t persistentId = 10086;
    ssm_->NotifyMoveSessionToForeground(collaboratorType, persistentId);
    ssm_->NotifyClearSession(collaboratorType, persistentId);
}

/**
 * @tc.name: QueryAbilityInfoFromBMS
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS NotifyStartAbility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, QueryAbilityInfoFromBMS, TestSize.Level1)
{
    const int32_t uId = 32;
    SessionInfo sessionInfo_;
    sessionInfo_.bundleName_ = "BundleName";
    sessionInfo_.abilityName_ = "AbilityName";
    sessionInfo_.moduleName_ = "ModuleName";
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t collaboratorType = CollaboratorType::RESERVE_TYPE;
    ssm_->QueryAbilityInfoFromBMS(uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_);
    EXPECT_EQ(sessionInfo_.want, nullptr);
    ssm_->Init();
    ssm_->QueryAbilityInfoFromBMS(uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_);
    ssm_->NotifyStartAbility(collaboratorType, sessionInfo_);
    sessionInfo_.want = std::make_shared<AAFwk::Want>();
    collaboratorType = CollaboratorType::OTHERS_TYPE;
    ssm_->NotifyStartAbility(collaboratorType, sessionInfo_);
    ASSERT_NE(sessionInfo_.want, nullptr);
}

/**
 * @tc.name: NotifyStartAbility
 * @tc.desc: SceneSesionManager NotifyStartAbility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, NotifyStartAbility, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "SceneSessionManagerTest";
    sessionInfo.bundleName_ = "SceneSessionManagerTest3";
    sessionInfo.abilityName_ = "NotifyStartAbility";
    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = iface_cast<AAFwk::IAbilityManagerCollaborator>(nullptr);
    ssm_->collaboratorMap_.clear();
    ssm_->collaboratorMap_.insert(std::make_pair(1, collaborator));
    int32_t collaboratorType = 1;
    auto ret1 = ssm_->NotifyStartAbility(collaboratorType, sessionInfo);
    EXPECT_EQ(ret1, BrokerStates::BROKER_UNKOWN);

    sessionInfo.want = std::make_shared<AAFwk::Want>();
    auto ret2 = ssm_->NotifyStartAbility(collaboratorType, sessionInfo);
    EXPECT_EQ(ret2, BrokerStates::BROKER_UNKOWN);
    ssm_->collaboratorMap_.clear();
}

/**
 * @tc.name: IsSessionClearable
 * @tc.desc: SceneSesionManager is session clearable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, IsSessionClearable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsSessionClearable";
    info.bundleName_ = "IsSessionClearable";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    EXPECT_FALSE(ssm_->IsSessionClearable(nullptr));
    EXPECT_FALSE(ssm_->IsSessionClearable(sceneSession));
    AppExecFwk::AbilityInfo abilityInfo_;
    abilityInfo_.excludeFromMissions = true;
    info.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo_);
    sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    EXPECT_FALSE(ssm_->IsSessionClearable(sceneSession));
    abilityInfo_.excludeFromMissions = false;
    abilityInfo_.unclearableMission = true;
    info.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo_);
    sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    EXPECT_FALSE(ssm_->IsSessionClearable(sceneSession));
    abilityInfo_.unclearableMission = false;
    info.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo_);
    info.lockedState = true;
    sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    EXPECT_FALSE(ssm_->IsSessionClearable(sceneSession));
    info.lockedState = false;
    info.isSystem_ = true;
    sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    EXPECT_FALSE(ssm_->IsSessionClearable(sceneSession));
    info.isSystem_ = false;
    sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_TRUE(ssm_->IsSessionClearable(sceneSession));
}

/**
 * @tc.name: HandleTurnScreenOn
 * @tc.desc: SceneSesionManager handle turn screen on and keep screen on
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, HandleTurnScreenOn, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "HandleTurnScreenOn";
    info.bundleName_ = "HandleTurnScreenOn";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->HandleTurnScreenOn(sceneSession);
    std::string lockName = "windowLock";
    bool requireLock = true;
    ssm_->HandleKeepScreenOn(sceneSession, requireLock, lockName, sceneSession->keepScreenLock_);
    requireLock = false;
    ssm_->HandleKeepScreenOn(sceneSession, requireLock, lockName, sceneSession->keepScreenLock_);
}

/**
 * @tc.name: HandleHideNonSystemFloatingWindows
 * @tc.desc: SceneSesionManager update hide non system floating windows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, HandleHideNonSystemFloatingWindows, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleHideNonSystemFloatingWindows";
    info.bundleName_ = "HandleHideNonSystemFloatingWindows";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = nullptr;
    ssm_->UpdateForceHideState(sceneSession, property, true);
    property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);
    property->SetHideNonSystemFloatingWindows(true);
    ssm_->UpdateForceHideState(sceneSession, property, true);
    ssm_->UpdateForceHideState(sceneSession, property, false);
    property->SetHideNonSystemFloatingWindows(false);
    property->SetFloatingWindowAppType(true);
    ssm_->UpdateForceHideState(sceneSession, property, true);
    ssm_->UpdateForceHideState(sceneSession, property, false);
    uint32_t result = property->GetWindowModeSupportType();
    EXPECT_EQ(result, WindowModeSupport::WINDOW_MODE_SUPPORT_ALL);
}

/**

@tc.name: UpdateForceHideState
@tc.desc: SceneSesionManager update Force Hide system state
@tc.type: FUNC
*/
HWTEST_F(SceneSessionManagerTest3, UpdateForceHideState, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateForceHideState";
    info.bundleName_ = "UpdateForceHideState";
    sptr sceneSession = sptr::MakeSptr(info, nullptr);
    sptr property = sptr::MakeSptr();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    property->SetHideNonSystemFloatingWindows(true);
    sceneSession->property_->windowMode_ = WindowMode::WINDOW_MODE_PIP;
    ssm_->systemTopSceneSessionMap_.clear();
    ssm_->UpdateForceHideState(sceneSession, property, true);
    EXPECT_EQ(ssm_->systemTopSceneSessionMap_.size(), 0);
    property->SetHideNonSystemFloatingWindows(true);
    sceneSession->property_->windowMode_ = WindowMode::WINDOW_MODE_FLOATING;
    ssm_->systemTopSceneSessionMap_.clear();
    ssm_->UpdateForceHideState(sceneSession, property, true);
    EXPECT_EQ(ssm_->systemTopSceneSessionMap_.size(), 1);
    property->SetHideNonSystemFloatingWindows(false);
    property->SetFloatingWindowAppType(true);
    property->SetSystemCalling(false);
    sceneSession->property_->windowMode_ = WindowMode::WINDOW_MODE_PIP;
    ssm_->systemTopSceneSessionMap_.clear();
    ssm_->UpdateForceHideState(sceneSession, property, true);
    EXPECT_EQ(ssm_->systemTopSceneSessionMap_.size(), 0);
    property->SetHideNonSystemFloatingWindows(false);
    sceneSession->property_->windowMode_ = WindowMode::WINDOW_MODE_FLOATING;
    ssm_->systemTopSceneSessionMap_.clear();
    ssm_->UpdateForceHideState(sceneSession, property, true);
    EXPECT_EQ(ssm_->systemTopSceneSessionMap_.size(), 0);
}

/**
 * @tc.name: UpdateBrightness01
 * @tc.desc: SceneSesionManager update brightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, UpdateBrightness01, TestSize.Level1)
{
    int32_t persistentId = 10086;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result01 = ssm_->UpdateBrightness(persistentId);
    EXPECT_EQ(result01, WSError::WS_ERROR_NULLPTR);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    result01 = ssm_->UpdateBrightness(persistentId);
    EXPECT_EQ(result01, WSError::WS_OK);
}

/**
 * @tc.name: UpdateBrightness02
 * @tc.desc: WINDOW_TYPE_WALLET_SWIPE_CARD
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, UpdateBrightness02, TestSize.Level1)
{
    int32_t persistentId = 10086;
    SessionInfo info;
    info.abilityName_ = "UpdateBrightness";
    info.bundleName_ = "UpdateBrightness";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD);
    info.isSystem_ = true;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, session));
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto  result01 = ssm_->UpdateBrightness(persistentId);
    EXPECT_EQ(result01, WSError::WS_OK);
}

/**
 * @tc.name: SetDisplayBrightness
 * @tc.desc: SceneSesionManager set display brightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetDisplayBrightness, TestSize.Level1)
{
    float brightness = 2.0f;
    ssm_->SetDisplayBrightness(brightness);
    float result02 = ssm_->GetDisplayBrightness();
    EXPECT_EQ(result02, 2.0f);
}

/**
 * @tc.name: SetGestureNavigationEnabled02
 * @tc.desc: SceneSesionManager set gesture navigation enable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SetGestureNavigationEnabled02, TestSize.Level1)
{
    bool enable = true;
    WMError result01 = ssm_->SetGestureNavigationEnabled(enable);
    EXPECT_EQ(result01, WMError::WM_OK);
    ProcessGestureNavigationEnabledChangeFunc funcGesture_ = SceneSessionManagerTest3::callbackFunc_;
    ssm_->SetGestureNavigationEnabledChangeListener(funcGesture_);
    WMError result02 = ssm_->SetGestureNavigationEnabled(enable);
    EXPECT_EQ(result02, WMError::WM_OK);
    ProcessStatusBarEnabledChangeFunc funcStatus_ = ProcessStatusBarEnabledChangeFuncTest;
    ssm_->SetStatusBarEnabledChangeListener(funcStatus_);
    WMError result03 = ssm_->SetGestureNavigationEnabled(enable);
    EXPECT_EQ(result03, WMError::WM_OK);
}

/**
 * @tc.name: RequestFocusStatus
 * @tc.desc: SceneSesionManager request focus status
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, RequestFocusStatus, TestSize.Level1)
{
    int32_t focusedSession = ssm_->GetFocusedSessionId();
    EXPECT_EQ(focusedSession, 10086);

    int32_t persistentId = INVALID_SESSION_ID;
    WMError result01 = ssm_->RequestFocusStatus(persistentId, true);
    EXPECT_EQ(result01, WMError::WM_ERROR_NULLPTR);
    FocusChangeReason reasonResult = ssm_->GetFocusChangeReason();
    EXPECT_EQ(reasonResult, FocusChangeReason::DEFAULT);

    persistentId = 10000;
    FocusChangeReason reasonInput = FocusChangeReason::SCB_SESSION_REQUEST;
    WMError result02 = ssm_->RequestFocusStatus(persistentId, true, true, reasonInput);
    EXPECT_EQ(result02, WMError::WM_ERROR_NULLPTR);
    reasonResult = ssm_->GetFocusChangeReason();
    EXPECT_EQ(reasonResult, FocusChangeReason::DEFAULT);

    reasonInput = FocusChangeReason::SPLIT_SCREEN;
    WMError result03 = ssm_->RequestFocusStatus(persistentId, false, true, reasonInput);
    EXPECT_EQ(result03, WMError::WM_ERROR_NULLPTR);
    reasonResult = ssm_->GetFocusChangeReason();
    EXPECT_EQ(reasonResult, FocusChangeReason::DEFAULT);
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession
 * @tc.desc: SceneSesionManager Gets the most recent session whose blockingType property is true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetTopNearestBlockingFocusSession, TestSize.Level1)
{
    uint32_t zOrder = 9999;
    sptr<SceneSession> session = ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, zOrder, true);
    EXPECT_EQ(session, nullptr);

    session = ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, zOrder, false);
    EXPECT_EQ(session, nullptr);
}

/**
 * @tc.name: RaiseWindowToTop
 * @tc.desc: SceneSesionManager raise window to top
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, RaiseWindowToTop, TestSize.Level1)
{
    int32_t focusedSession = ssm_->GetFocusedSessionId();
    EXPECT_EQ(focusedSession, 10086);
    int32_t persistentId = INVALID_SESSION_ID;
    WSError result01 = ssm_->RaiseWindowToTop(persistentId);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_PERMISSION);
    persistentId = 10000;
    WSError result02 = ssm_->RaiseWindowToTop(persistentId);
    EXPECT_EQ(result02, WSError::WS_ERROR_INVALID_PERMISSION);
    WSError result03 = ssm_->RaiseWindowToTop(persistentId);
    EXPECT_EQ(result03, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ShiftAppWindowFocus
 * @tc.desc: SceneSesionManager shift app window focus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ShiftAppWindowFocus, TestSize.Level1)
{
    int32_t focusedSession = ssm_->GetFocusedSessionId();
    EXPECT_EQ(focusedSession, 10086);
    int32_t sourcePersistentId = 1;
    int32_t targetPersistentId = 10086;
    WSError result01 = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_SESSION);
    SessionInfo info;
    info.abilityName_ = "ShiftAppWindowFocus";
    info.bundleName_ = "ShiftAppWindowFocus";
    auto sourceSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sourceSceneSession->persistentId_ = 1;
    sourceSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sourceSceneSession));
    result01 = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_OPERATION);

    auto targetSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    targetSceneSession->persistentId_ = 10086;
    targetSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert(std::make_pair(10086, targetSceneSession));
    result01 = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_OPERATION);
    sourcePersistentId = 10086;
    WSError result02 = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result02, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: RegisterSessionExceptionFunc
 * @tc.desc: SceneSesionManager register session expection func
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, RegisterSessionExceptionFunc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterSessionExceptionFunc";
    info.bundleName_ = "RegisterSessionExceptionFunc";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterSessionExceptionFunc(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->RegisterSessionExceptionFunc(sceneSession);
    bool result01 = ssm_->IsSessionVisibleForeground(sceneSession);
    EXPECT_FALSE(result01);
}

/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: SceneSesionManager update window mode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, UpdateWindowMode, TestSize.Level1)
{
    int32_t persistentId = 10086;
    int32_t windowMode = 3;
    WSError result = ssm_->UpdateWindowMode(persistentId, windowMode);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_WINDOW);
    WindowChangedFunc func = [](int32_t persistentId, WindowUpdateType type) {
        OHOS::Rosen::WindowChangedFuncTest3(persistentId, type);
    };
    ssm_->RegisterWindowChanged(func);
}

/**
 * @tc.name: SetScreenLocked && IsScreenLocked
 * @tc.desc: SceneSesionManager update screen locked state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, IsScreenLocked, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ssm_->SetScreenLocked(true);
    sleep(1);
    EXPECT_TRUE(ssm_->IsScreenLocked());
    ssm_->SetScreenLocked(false);
    sleep(1);
    EXPECT_FALSE(ssm_->IsScreenLocked());
}

/**
 * @tc.name: UpdatePrivateStateAndNotify
 * @tc.desc: SceneSesionManager update private state and notify
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, UpdatePrivateStateAndNotify, TestSize.Level1)
{
    int32_t persistentId = 10086;
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterSessionStateChangeNotifyManagerFunc(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->RegisterSessionStateChangeNotifyManagerFunc(sceneSession);
    ssm_->UpdatePrivateStateAndNotify(persistentId);
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleList;
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundleList);
    EXPECT_EQ(privacyBundleList[displayId].size(), 0);
}

/**
 * @tc.name: UpdatePrivateStateAndNotifyForAllScreens
 * @tc.desc: SceneSesionManager update private state and notify for all screens
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, UpdatePrivateStateAndNotifyForAllScreens, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    ssm_->UpdatePrivateStateAndNotifyForAllScreens();
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleList;
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundleList);
    EXPECT_EQ(privacyBundleList[displayId].size(), 0);
}

/**
 * @tc.name: GerPrivacyBundleListOneWindow
 * @tc.desc: get privacy bundle list when one window exist only.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GerPrivacyBundleListOneWindow, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "privacy.test";
    sessionInfo.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    sceneSession->GetSessionProperty()->displayId_ = 0;
    sceneSession->GetSessionProperty()->isPrivacyMode_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleList;
    sceneSession->GetSessionProperty()->isPrivacyMode_ = false;
    privacyBundleList.clear();
    ssm_->GetSceneSessionPrivacyModeBundles(0, privacyBundleList);
    EXPECT_EQ(privacyBundleList[0].size(), 0);

    sceneSession->GetSessionProperty()->isPrivacyMode_ = true;
    sceneSession->state_ = SessionState::STATE_BACKGROUND;
    privacyBundleList.clear();
    ssm_->GetSceneSessionPrivacyModeBundles(0, privacyBundleList);
    EXPECT_EQ(privacyBundleList[0].size(), 0);

    sceneSession->GetSessionProperty()->isPrivacyMode_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    privacyBundleList.clear();
    ssm_->GetSceneSessionPrivacyModeBundles(0, privacyBundleList);
    EXPECT_EQ(privacyBundleList[0].size(), 1);

    privacyBundleList.clear();
    ssm_->GetSceneSessionPrivacyModeBundles(1, privacyBundleList);
    EXPECT_EQ(privacyBundleList[1].size(), 0);
}

/**
 * @tc.name: GetTopWindowId
 * @tc.desc: get top window id by main window id.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, GetTopWindowId, TestSize.Level1)
{
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "mainWin";
    sessionInfo1.abilityName_ = "mainAbilityName";
    sessionInfo1.persistentId_ = 100;
    auto sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->SetCallingPid(65534);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession1 });

    SessionInfo sessionInfo2;
    sessionInfo2.bundleName_ = "subWin1";
    sessionInfo2.abilityName_ = "subAbilityName1";
    sessionInfo2.persistentId_ = 101;
    auto sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->SetCallingPid(65535);
    ssm_->sceneSessionMap_.insert({ 101, sceneSession2 });

    SessionInfo sessionInfo3;
    sessionInfo3.bundleName_ = "subWin2";
    sessionInfo3.abilityName_ = "subAbilityName2";
    sessionInfo3.persistentId_ = 102;
    auto sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    ASSERT_NE(sceneSession3, nullptr);
    sceneSession3->SetCallingPid(65534);
    ssm_->sceneSessionMap_.insert({ 102, sceneSession3 });

    sceneSession1->AddSubSession(sceneSession2);
    sceneSession1->AddSubSession(sceneSession3);
    uint32_t topWinId;
    ASSERT_EQ(ssm_->GetTopWindowId(static_cast<uint32_t>(sceneSession1->GetPersistentId()), topWinId),
              WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ConfigWindowImmersive
 * @tc.desc: ConfigWindowImmersive SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigWindowImmersive01, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem immersiveConfig;
    immersiveConfig.boolValue_ = false;
    ASSERT_NE(ssm_, nullptr);
    ssm_->ConfigWindowImmersive(immersiveConfig);

    ASSERT_EQ(ssm_->SwitchFreeMultiWindow(false), WSError::WS_OK);
    SystemSessionConfig systemConfig;
    systemConfig.freeMultiWindowSupport_ = true;
    ssm_->SwitchFreeMultiWindow(false);
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: SceneSesionManager config decor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigDecor02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem* item = new WindowSceneConfig::ConfigItem;
    ASSERT_NE(item, nullptr);
    ssm_->ConfigDecor(*item, false);
    delete item;
}

/**
 * @tc.name: ConfigAppWindowShadow
 * @tc.desc: SceneSesionManager config app window shadow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigAppWindowShadow02, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem item;
    WindowSceneConfig::ConfigItem shadowConfig;
    WindowShadowConfig outShadow;
    std::vector<float> floatTest = {};
    bool result = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    EXPECT_EQ(result, true);

    item.SetValue(floatTest);
    shadowConfig.SetValue({ { "radius", item } });
    result = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    EXPECT_EQ(result, false);

    item.SetValue(new std::string(""));
    shadowConfig.SetValue({ { "", item } });
    result = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ConfigWindowAnimation
 * @tc.desc: SceneSesionManager config window animation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigWindowAnimation02, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem windowAnimationConfig;
    WindowSceneConfig::ConfigItem item;
    std::vector<float> rotation = { 0.1f, 0.2f, 0.3f, 0.4f };
    ASSERT_NE(ssm_, nullptr);

    item.SetValue(rotation);
    item.SetValue({ { "curve", item } });
    windowAnimationConfig.SetValue({ { "timing", item } });
    ssm_->ConfigWindowAnimation(windowAnimationConfig);
}

/**
 * @tc.name: ConfigStartingWindowAnimation
 * @tc.desc: SceneSesionManager config start window animation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigStartingWindowAnimation02, TestSize.Level1)
{
    std::vector<float> midFloat = { 0.1f };
    std::vector<int> midInt = { 1 };
    ASSERT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem middleFloat;
    middleFloat.SetValue(midFloat);
    ssm_->ConfigStartingWindowAnimation(middleFloat);
    WindowSceneConfig::ConfigItem middleInt;
    middleInt.SetValue(midInt);
    ssm_->ConfigStartingWindowAnimation(middleInt);

    WindowSceneConfig::ConfigItem curve;
    curve.SetValue(midFloat);
    curve.SetValue({ { "curve", curve } });
    ssm_->ConfigStartingWindowAnimation(curve);
}

/**
 * @tc.name: ConfigMainWindowSizeLimits
 * @tc.desc: call ConfigMainWindowSizeLimits and check the systemConfig_.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigMainWindowSizeLimits02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<int> maInt = { 1, 2, 3, 4 };
    WindowSceneConfig::ConfigItem mainleInt;
    mainleInt.SetValue(maInt);
    mainleInt.SetValue({ { "miniWidth", mainleInt } });
    ssm_->ConfigMainWindowSizeLimits(mainleInt);
    mainleInt.ClearValue();

    std::vector<float> maFloat = { 0.1f };
    WindowSceneConfig::ConfigItem mainFloat;
    mainFloat.SetValue(maFloat);
    mainFloat.SetValue({ { "miniWidth", mainFloat } });
    ssm_->ConfigMainWindowSizeLimits(mainFloat);
    mainFloat.ClearValue();

    WindowSceneConfig::ConfigItem mainleInt02;
    mainleInt02.SetValue(maInt);
    mainleInt02.SetValue({ { "miniHeight", mainleInt02 } });
    ssm_->ConfigMainWindowSizeLimits(mainleInt02);

    WindowSceneConfig::ConfigItem mainFloat02;
    mainFloat02.SetValue(maFloat);
    mainFloat02.SetValue({ { "miniHeight", mainFloat02 } });
    ssm_->ConfigMainWindowSizeLimits(mainFloat02);
}

/**
 * @tc.name: ConfigSubWindowSizeLimits
 * @tc.desc: call ConfigSubWindowSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigSubWindowSizeLimits02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<int> subInt = { 1, 2, 3, 4 };
    WindowSceneConfig::ConfigItem subleInt;
    subleInt.SetValue(subInt);
    subleInt.SetValue({ { "miniWidth", subleInt } });
    ssm_->ConfigSubWindowSizeLimits(subleInt);
    subleInt.ClearValue();

    std::vector<float> subFloat = { 0.1f };
    WindowSceneConfig::ConfigItem mainFloat;
    mainFloat.SetValue(subFloat);
    mainFloat.SetValue({ { "miniWidth", mainFloat } });
    ssm_->ConfigSubWindowSizeLimits(mainFloat);
    mainFloat.ClearValue();

    WindowSceneConfig::ConfigItem subleInt02;
    subleInt02.SetValue(subInt);
    subleInt02.SetValue({ { "miniHeight", subleInt02 } });
    ssm_->ConfigSubWindowSizeLimits(subleInt02);

    WindowSceneConfig::ConfigItem mainFloat02;
    mainFloat02.SetValue(subFloat);
    mainFloat02.SetValue({ { "miniHeight", mainFloat02 } });
    ssm_->ConfigSubWindowSizeLimits(mainFloat02);
}

/**
 * @tc.name: ConfigDialogWindowSizeLimits
 * @tc.desc: call ConfigDialogWindowSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigDialogWindowSizeLimits01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<int> subInt = { 1, 2, 3, 4 };
    WindowSceneConfig::ConfigItem subleInt;
    subleInt.SetValue(subInt);
    subleInt.SetValue({ { "miniWidth", subleInt } });
    ssm_->ConfigDialogWindowSizeLimits(subleInt);
    subleInt.ClearValue();

    std::vector<float> subFloat = { 0.1f };
    WindowSceneConfig::ConfigItem mainFloat;
    mainFloat.SetValue(subFloat);
    mainFloat.SetValue({ { "miniWidth", mainFloat } });
    ssm_->ConfigDialogWindowSizeLimits(mainFloat);
    mainFloat.ClearValue();

    WindowSceneConfig::ConfigItem subleInt02;
    subleInt02.SetValue(subInt);
    subleInt02.SetValue({ { "miniHeight", subleInt02 } });
    ssm_->ConfigDialogWindowSizeLimits(subleInt02);

    WindowSceneConfig::ConfigItem mainFloat02;
    mainFloat02.SetValue(subFloat);
    mainFloat02.SetValue({ { "miniHeight", mainFloat02 } });
    ssm_->ConfigDialogWindowSizeLimits(mainFloat02);
}

/**
 * @tc.name: RegisterSetForegroundWindowNumCallback
 * @tc.desc: call RegisterSetForegroundWindowNumCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, RegisterSetForegroundWindowNumCallback, TestSize.Level1)
{
    EXPECT_NE(ssm_, nullptr);
    std::function<void(uint32_t windowNum)> func = [](uint32_t windowNum) {
        return;
    };
    ssm_->RegisterSetForegroundWindowNumCallback(std::move(func));
    EXPECT_NE(ssm_->setForegroundWindowNumFunc_, nullptr);
}

/**
 * @tc.name: ConfigSingleHandCompatibleMode
 * @tc.desc: call ConfigSingleHandCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, ConfigSingleHandCompatibleMode, TestSize.Level1)
{
    EXPECT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem configItem;
    configItem.SetValue(true);
    configItem.SetValue({ { "test", configItem } });
    ssm_->ConfigSingleHandCompatibleMode(configItem);
    EXPECT_EQ(ssm_->singleHandCompatibleModeConfig_.enabled, configItem.boolValue_);
}

/**
 * @tc.name: NotifySessionTouchOutside
 * @tc.desc: call NotifySessionTouchOutside02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, NotifySessionTouchOutside02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    auto ssm = ssm_;
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession->SetSessionProperty(property);
    ssm->sceneSessionMap_.insert({1, sceneSession});
    ssm->windowFocusController_->displayId2GroupIdMap_[20] = 20;

    SessionInfo info02;
    info02.abilityName_ = "test1";
    info02.bundleName_ = "test2";
    info02.windowInputType_ = static_cast<uint32_t>(MMI::WindowInputType::NORMAL);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info02, nullptr);
    sceneSession02->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();

    auto sessionTouchOutsideFun = [ssm](int32_t persistentId, DisplayId displayId) {
        ssm->NotifySessionTouchOutside(persistentId, 20);
    };
    auto outsideDownEventFun = [sceneSession02](int32_t x, int32_t y) {
        int z = x + y;
        sceneSession02->SetCollaboratorType(z);
    };
    sceneSession02->specificCallback_->onSessionTouchOutside_ = sessionTouchOutsideFun;
    sceneSession02->specificCallback_->onOutsideDownEvent_ = outsideDownEventFun;
    sceneSession02->SetSessionProperty(property);
    ssm->sceneSessionMap_.insert({2, sceneSession02});
    sceneSession->isVisible_ = true;
    sceneSession02->isVisible_ = true;
    ssm->NotifySessionTouchOutside(1, 20);
    ssm->NotifySessionTouchOutside(1, 0);
    EXPECT_EQ(true, sceneSession->IsVisible());
    EXPECT_EQ(true, sceneSession02->IsVisible());
    EXPECT_EQ(0, sceneSession->GetDisplayId());
    EXPECT_EQ(0, sceneSession02->GetDisplayId());
    EXPECT_EQ(20, ssm->GetDisplayGroupId(20));
    EXPECT_EQ(false, ssm->IsSameDisplayGroupId(sceneSession, 20));
    EXPECT_EQ(false, ssm->IsSameDisplayGroupId(sceneSession02, 20));
    EXPECT_EQ(0, ssm->GetDisplayGroupId(sceneSession->GetDisplayId()));
    EXPECT_EQ(0, ssm->GetDisplayGroupId(sceneSession02->GetDisplayId()));
    EXPECT_EQ(WSError::WS_OK, sceneSession02->ProcessPointDownSession(3, 4));
    ssm->sceneSessionMap_.erase(1);
    ssm->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: SchedulePcAppInPadLifecycle
 * @tc.desc: call SchedulePcAppInPadLifecycle
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, SchedulePcAppInPadLifecycle, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetIsPcAppInPad(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    SessionInfo info1;
    info1.abilityName_ = "test3";
    info1.bundleName_ = "test4";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sceneSession2->GetSessionProperty()->SetIsPcAppInPad(false);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    SessionInfo info2;
    info2.abilityName_ = "test5";
    info2.bundleName_ = "test6";
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    sceneSession3->GetSessionProperty()->SetIsPcAppInPad(true);
    sceneSession3->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession3->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    auto trayAppListOld = ssm_->trayAppList_;
    ssm_->SchedulePcAppInPadLifecycle(true);
    ssm_->sceneSessionMap_.insert({1, sceneSession});
    ssm_->sceneSessionMap_.insert({2, sceneSession2});
    ssm_->sceneSessionMap_.insert({3, sceneSession3});
    ssm_->sceneSessionMap_.insert({4, nullptr});
    std::vector<std::string> trayAppList;
    trayAppList.push_back("test2");
    ssm_->SetTrayAppList(std::move(trayAppList));
    ssm_->SchedulePcAppInPadLifecycle(true);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(SessionState::STATE_FOREGROUND, sceneSession->GetSessionState());
    EXPECT_EQ(SessionState::STATE_FOREGROUND, sceneSession2->GetSessionState());
    ssm_->SchedulePcAppInPadLifecycle(false);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(SessionState::STATE_FOREGROUND, sceneSession->GetSessionState());
    EXPECT_EQ(SessionState::STATE_FOREGROUND, sceneSession2->GetSessionState());
    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
    ssm_->sceneSessionMap_.erase(3);
    ssm_->sceneSessionMap_.erase(4);
    ssm_->SetTrayAppList(std::move(trayAppListOld));
}
 
/**
 * @tc.name: StartOrMinimizePcAppInPadUIAbilityBySCB
 * @tc.desc: call StartOrMinimizePcAppInPadUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest3, StartOrMinimizePcAppInPadUIAbilityBySCB, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetIsPcAppInPad(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);

    SessionInfo info1;
    info1.abilityName_ = "test3";
    info1.bundleName_ = "test4";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->GetSessionProperty()->SetIsPcAppInPad(true);
    sceneSession2->SetSessionState(SessionState::STATE_DISCONNECT);
    SessionInfo info2;
    info2.abilityName_ = "test5";
    info2.bundleName_ = "test6";
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession3, nullptr);
    sceneSession3->GetSessionProperty()->SetIsPcAppInPad(true);
    sceneSession3->SetSessionState(SessionState::STATE_FOREGROUND);

    auto trayAppListOld = ssm_->trayAppList_;
    std::vector<std::string> trayAppList;
    trayAppList.push_back("test2");
    ssm_->SetTrayAppList(std::move(trayAppList));
    EXPECT_EQ(WSError::WS_DO_NOTHING, ssm_->StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession, true));
    EXPECT_EQ(WSError::WS_DO_NOTHING, ssm_->StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession, false));
    EXPECT_EQ(WSError::WS_DO_NOTHING, ssm_->StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession2, true));
    EXPECT_EQ(WSError::WS_DO_NOTHING, ssm_->StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession2, false));
    EXPECT_EQ(WSError::WS_OK, ssm_->StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession3, true));
    EXPECT_EQ(WSError::WS_OK, ssm_->StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession3, false));
    ssm_->SetTrayAppList(std::move(trayAppListOld));
}
} // namespace
} // namespace Rosen
} // namespace OHOS