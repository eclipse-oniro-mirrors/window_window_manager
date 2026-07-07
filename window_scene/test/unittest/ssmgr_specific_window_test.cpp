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

#include "context.h"
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
static constexpr uint32_t WAIT_SYNC_IN_NS = 200000U;
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

class SSMgrSpecificWindowTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
};

sptr<SceneSessionManager> SSMgrSpecificWindowTest::ssm_ = nullptr;

void SSMgrSpecificWindowTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SSMgrSpecificWindowTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SSMgrSpecificWindowTest::SetUp() {}

void SSMgrSpecificWindowTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: ConfigKeyboardAnimation01
 * @tc.desc: call ConfigKeyboardAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ConfigKeyboardAnimation01, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<keyboardAnimation>"
        "<animationIn>"
        "<timing>"
        "<duration>abv</duration>"
        "<curve name=\"cubic\">0.2 0.0 0.2 1.0</curve>"
        "</timing>"
        "</animationIn>"
        "<animationOut>"
        "<timing>"
        "<duration>abv</duration>"
        "<curve name=\"cubic\">0.2 0.0 0.2 1.0</curve>"
        "</timing>"
        "</animationOut>"
        "</keyboardAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();

    std::string xmlStr1 =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<keyboardAnimation>"
        "<animationIn>"
        "<timing>"
        "<duration>500</duration>"
        "<curve name=\"cubic\">0.2 0.0 0.2 1.0</curve>"
        "</timing>"
        "</animationIn>"
        "<animationOut>"
        "<timing>"
        "<duration>300</duration>"
        "<curve name=\"cubic\">0.2 0.0 0.2 1.0</curve>"
        "</timing>"
        "</animationOut>"
        "</keyboardAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr1);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.animationIn_.duration_, static_cast<uint32_t>(500));
    ASSERT_EQ(ssm_->systemConfig_.animationOut_.duration_, static_cast<uint32_t>(300));
}

/**
 * @tc.name: ConfigKeyboardAnimation02
 * @tc.desc: call ConfigKeyboardAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ConfigKeyboardAnimation02, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<keyboardAnimation>"
        "<animationIn>"
        "<timing>"
        "<duration>500</duration>"
        "<duration>600</duration>"
        "</timing>"
        "</animationIn>"
        "<animationOut>"
        "<timing>"
        "<duration>300</duration>"
        "</timing>"
        "</animationOut>"
        "</keyboardAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.animationOut_.duration_, static_cast<uint32_t>(300));
}

/**
 * @tc.name: ConfigKeyboardAnimation03
 * @tc.desc: call ConfigKeyboardAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ConfigKeyboardAnimation03, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<keyboardAnimation>"
        "<animationIn>"
        "<timing>"
        "<duration>500</duration>"
        "</timing>"
        "</animationIn>"
        "<animationOut>"
        "<timing>"
        "<duration>300</duration>"
        "<duration>400</duration>"
        "</timing>"
        "</animationOut>"
        "</keyboardAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.animationIn_.duration_, static_cast<uint32_t>(500));
}

/**
 * @tc.name: ConfigKeyboardAnimation04
 * @tc.desc: call ConfigKeyboardAnimation default
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ConfigKeyboardAnimation04, TestSize.Level1)
{
    std::string xmlStr =
        "<?xml version='1.0' encoding=\"utf-8\"?>"
        "<Configs>"
        "<keyboardAnimation>"
        "<animationIn>"
        "<timing>"
        "<duration>500</duration>"
        "</timing>"
        "</animationIn>"
        "</keyboardAnimation>"
        "</Configs>";
    WindowSceneConfig::config_ = ReadConfig(xmlStr);
    ssm_->ConfigWindowSceneXml();
    ASSERT_EQ(ssm_->systemConfig_.animationIn_.duration_, static_cast<uint32_t>(500));
}

/**
 * @tc.name: IsKeyboardForeground
 * @tc.desc: IsKeyboardForeground
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, IsKeyboardForeground, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    ASSERT_NE(sceneSession, nullptr);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    session->property_ = nullptr;
    auto result = ssm_->IsKeyboardForeground();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: ProcessSubSessionForeground
 * @tc.desc: ProcessSubSessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ProcessSubSessionForeground, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionForeground(sceneSession);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionForeground(sceneSession);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, subSession);
    sceneSession->AddSubSession(subSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionForeground(sceneSession);
    ASSERT_NE(nullptr, subSession);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionForeground(sceneSession);
    ASSERT_NE(nullptr, subSession);
    subSession->SetSessionState(SessionState::STATE_ACTIVE);
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionForeground(sceneSession);
    ASSERT_NE(nullptr, subSession);
    subSession->SetSessionState(SessionState::STATE_INACTIVE);
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionForeground(sceneSession);
}

/**
 * @tc.name: ProcessSubSessionBackground
 * @tc.desc: ProcessSubSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ProcessSubSessionBackground, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, subSession);
    sceneSession->AddSubSession(subSession);
    subSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
    ASSERT_NE(nullptr, subSession);
    subSession->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
    ASSERT_NE(nullptr, subSession);
    subSession->state_ = SessionState::STATE_INACTIVE;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
}

/**
 * @tc.name: ProcessSubSessionBackground01
 * @tc.desc: ProcessSubSessionBackground01
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ProcessSubSessionBackground01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->dialogVec_.clear();
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->dialogVec_.push_back(sceneSession1);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    sceneSession1->persistentId_ = 1;
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    ssm_->ProcessSubSessionBackground(sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->ProcessSubSessionBackground(sceneSession);
    sptr<SceneSession> toastSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, toastSession);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->AddToastSession(toastSession);
    toastSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessSubSessionBackground(sceneSession);
    toastSession->state_ = SessionState::STATE_ACTIVE;
    ssm_->ProcessSubSessionBackground(sceneSession);
    toastSession->state_ = SessionState::STATE_INACTIVE;
    ssm_->ProcessSubSessionBackground(sceneSession);
}

/**
 * @tc.name: DestroyDialogWithMainWindow
 * @tc.desc: DestroyDialogWithMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, DestroyDialogWithMainWindow, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    auto result = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    ASSERT_NE(sceneSession, nullptr);

    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    session->GetDialogVector().clear();
    result = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(result, WSError::WS_OK);

    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->GetSceneSession(1);
    result = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: DestroyDialogWithMainWindow02
 * @tc.desc: DestroyDialogWithMainWindow02
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, DestroyDialogWithMainWindow02, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    ASSERT_NE(sceneSession, nullptr);

    sptr<Session> dialogSession1 = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession2 = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession1, nullptr);
    ASSERT_NE(dialogSession2, nullptr);
    dialogSession1->persistentId_ = 0;
    dialogSession2->persistentId_ = 1;
    sceneSession->dialogVec_.push_back(dialogSession1);
    sceneSession->dialogVec_.push_back(dialogSession2);

    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({ 0, nullptr });
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });

    auto ret = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: ConfigKeyboardAnimation
 * @tc.desc: SceneSesionManager config keyboard animation
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, ConfigKeyboardAnimation, TestSize.Level1)
{
    WindowSceneConfig::ConfigItem animationConfig;
    WindowSceneConfig::ConfigItem itemCurve;
    WindowSceneConfig::ConfigItem itemDuration;
    // prepare ItemName
    WindowSceneConfig::ConfigItem nameProp;
    std::string name = "cubic";
    nameProp.SetValue(name);
    itemCurve.SetProperty({ { "name", nameProp } });

    // prepare duration
    std::vector<int> durationVec = { 39 };
    itemDuration.SetValue(durationVec);

    // prepare timing
    WindowSceneConfig::ConfigItem timing;
    WindowSceneConfig::ConfigItem timingObj;
    timingObj.SetValue({ { "curve", itemCurve }, { "duration", itemDuration } });
    timing.SetValue({ { "timing", timingObj } });

    WindowSceneConfig::ConfigItem timing2(timing);
    // prepare animationConfig
    animationConfig.SetValue({ { "animationIn", timing }, { "animationOut", timing2 } });

    ssm_->ConfigKeyboardAnimation(animationConfig);
    ASSERT_EQ(ssm_->systemConfig_.animationIn_.curveType_, "cubic");
    ASSERT_EQ(ssm_->systemConfig_.animationIn_.duration_, durationVec.at(0));

    uint32_t result = 150;
    KeyboardSceneAnimationConfig animationIn;
    KeyboardSceneAnimationConfig animationOut;
    ssm_->systemConfig_.animationIn_.curveType_ = "";
    ssm_->ConfigDefaultKeyboardAnimation(animationIn, animationOut);
    ASSERT_EQ(animationIn.duration_, result);
}

/**
 * @tc.name: UpdateParentSessionForDialog
 * @tc.desc: SceneSesionManager UpdateParentSessionForDialog
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, UpdateParentSessionForDialog, TestSize.Level1)
{
    WSError result = ssm_->UpdateParentSessionForDialog(nullptr, nullptr);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    result = ssm_->UpdateParentSessionForDialog(nullptr, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test3";
    info.appIndex_ = 10;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    result = ssm_->UpdateParentSessionForDialog(sceneSession, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    result = ssm_->UpdateParentSessionForDialog(sceneSession, property);
    EXPECT_EQ(result, WSError::WS_OK);

    SessionInfo info1;
    info1.abilityName_ = "test2";
    info1.bundleName_ = "test3";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });
    property->SetParentPersistentId(2);
    result = ssm_->UpdateParentSessionForDialog(sceneSession, property);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSession
 * @tc.desc: SceneSesionManager destroy and disconnect specific session
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, DestroyAndDisconnectSpecificSession, TestSize.Level1)
{
    int32_t persistentId = 0;
    WSErrorResult result = ssm_->DestroyAndDisconnectSpecificSession(persistentId);
    ASSERT_EQ(result.errCode, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: SceneSesionManager destroy and disconnect specific session with detach callback
 * @tc.type: FUNC
 */
HWTEST_F(SSMgrSpecificWindowTest, DestroyAndDisconnectSpecificSessionWithDetachCallback, TestSize.Level1)
{
    int32_t persistentId = 0;
    WSErrorResult result = ssm_->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, nullptr);
    ASSERT_EQ(result.errCode, WSError::WS_ERROR_NULLPTR);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
