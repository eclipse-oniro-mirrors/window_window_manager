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

#include "transaction/rs_uiextension_data.h"
#include "input_manager.h"
#include "session_manager/include/scene_session_dirty_manager.h"
#include <gtest/gtest.h>
#include <parameter.h>
#include <parameters.h>
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/sub_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "transaction/rs_uiextension_data.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace Rosen {
constexpr int POINTER_CHANGE_AREA_SIXTEEN = 16;
constexpr int POINTER_CHANGE_AREA_DEFAULT = 0;
constexpr int POINTER_CHANGE_AREA_FIVE = 5;
static int32_t g_screenRotationOffset = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
class SceneSessionDirtyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};
SceneSessionDirtyManager* manager_;
SceneSessionManager* ssm_;
void SceneSessionDirtyManagerTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionDirtyManagerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionDirtyManagerTest::SetUp()
{
    manager_ = new SceneSessionDirtyManager();
}

void SceneSessionDirtyManagerTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    delete manager_;
    manager_ = nullptr;
}
namespace {

/**
 * @tc.name: NotifyWindowInfoChange
 * @tc.desc: NotifyWindowInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, NotifyWindowInfoChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    manager_->NotifyWindowInfoChange(nullptr, WindowUpdateType::WINDOW_UPDATE_ADDED, true);
    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    manager_->NotifyWindowInfoChange(sceneSession, WindowUpdateType::WINDOW_UPDATE_ADDED, true);
    manager_->NotifyWindowInfoChange(sceneSession, WindowUpdateType::WINDOW_UPDATE_REMOVED, true);
    manager_->NotifyWindowInfoChange(sceneSession, WindowUpdateType::WINDOW_UPDATE_ACTIVE, true);
    manager_->NotifyWindowInfoChange(sceneSession, WindowUpdateType::WINDOW_UPDATE_FOCUSED, true);
    EXPECT_FALSE(g_logMsg.find("sceneSession is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetFullWindowInfoList
 * @tc.desc: GetFullWindowInfoList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetFullWindowInfoList, TestSize.Level0)
{
    auto [windowInfoList, pixelMapList] = manager_->GetFullWindowInfoList();
    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    {
        sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
        ASSERT_NE(sceneSession, nullptr);
        sceneSession->UpdateVisibilityInner(true);
        ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    }
    {
        ssm_->sceneSessionMap_.insert({ 111, nullptr });
    }
    {
        sptr<SceneSession> sceneSessionDialog1 = sptr<SceneSession>::MakeSptr(info, nullptr);
        ASSERT_NE(sceneSessionDialog1, nullptr);
        sceneSessionDialog1->UpdateVisibilityInner(true);
        sptr<WindowSessionProperty> propertyDialog1 = sceneSessionDialog1->GetSessionProperty();
        propertyDialog1->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
        ssm_->sceneSessionMap_.insert({ sceneSessionDialog1->GetPersistentId(), sceneSessionDialog1 });
    }
    {
        sptr<SceneSession> sceneSessionModal1 = sptr<SceneSession>::MakeSptr(info, nullptr);
        ASSERT_NE(sceneSessionModal1, nullptr);
        sceneSessionModal1->UpdateVisibilityInner(true);
        sptr<WindowSessionProperty> propertyModal1 = sceneSessionModal1->GetSessionProperty();
        propertyModal1->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
        ssm_->sceneSessionMap_.insert({ sceneSessionModal1->GetPersistentId(), sceneSessionModal1 });
    }
    auto [windowInfoList1, pixelMapList1] = manager_->GetFullWindowInfoList();
    ASSERT_EQ(windowInfoList.size() + 3, windowInfoList1.size());
}

/**
 * @tc.name: IsFilterSession
 * @tc.desc: IsFilterSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, IsFilterSession, TestSize.Level1)
{
    bool ret = manager_->IsFilterSession(nullptr);
    ASSERT_EQ(ret, true);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.sceneType_ = SceneType::INPUT_SCENE;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sceneSession->GetSessionProperty();
    sceneSession->sessionInfo_ = info;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    info.isSystem_ = false;
    sceneSession->sessionInfo_ = info;
    sceneSession->isVisible_ = false;
    sceneSession->isSystemActive_ = false;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    info.isSystem_ = true;
    sceneSession->sessionInfo_ = info;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    sceneSession->isVisible_ = true;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    sceneSession->isSystemActive_ = true;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    info.isSystem_ = true;
    sceneSession->sessionInfo_ = info;
    sceneSession->isVisible_ = true;
    sceneSession->SetSystemActive(true);
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    info.isSystem_ = false;
    sceneSession->sessionInfo_ = info;
    sceneSession->isVisible_ = false;
    sceneSession->SetSystemActive(false);
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    sceneSession->isVisible_ = false;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: GetDialogSessionMap
 * @tc.desc: GetDialogSessionMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetDialogSessionMap, TestSize.Level1)
{
    std::map<int32_t, sptr<SceneSession>> sessionMap;
    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    sessionMap.emplace(1, nullptr);
    auto sessionList = manager_->GetDialogSessionMap(sessionMap);
    ASSERT_EQ(0, sessionList.size());
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sessionMap.emplace(2, sceneSession);
    auto sessionList2 = manager_->GetDialogSessionMap(sessionMap);
    ASSERT_EQ(0, sessionList2.size());
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sceneSession->SetSessionProperty(property);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    sceneSession->SetParentSession(session);
    auto sessionList3 = manager_->GetDialogSessionMap(sessionMap);
    ASSERT_EQ(1, sessionList3.size());
    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    auto sessionList4 = manager_->GetDialogSessionMap(sessionMap);
    ASSERT_EQ(1, sessionList4.size());
    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    sceneSession->SetParentSession(nullptr);
    auto sessionList5 = manager_->GetDialogSessionMap(sessionMap);
    ASSERT_EQ(0, sessionList5.size());
}

/**
 * @tc.name: GetDialogSessionMap02
 * @tc.desc: GetDialogSessionMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetDialogSessionMap02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    sptr<SubSession> sceneSession = sptr<SubSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    sceneSession->SetSessionProperty(property);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    sceneSession->SetParentSession(session);
    std::map<int32_t, sptr<SceneSession>> sessionMap;
    sessionMap.emplace(1, sceneSession);
    auto sessionList = manager_->GetDialogSessionMap(sessionMap);
    ASSERT_EQ(2, sessionList.size());
}

/**
 * @tc.name: UpdatePointerAreas
 * @tc.desc: UpdatePointerAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdatePointerAreas, TestSize.Level0)
{
    std::vector<int32_t> pointerChangeAreas;
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    auto oriProperty = sceneSession->property_;
    sceneSession->property_ = nullptr;
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    ASSERT_EQ(0, pointerChangeAreas.size());
    sceneSession->property_ = oriProperty;
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetDragEnabled(false);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    ASSERT_EQ(0, pointerChangeAreas.size());
    sceneSession->property_->SetDragEnabled(true);
    float vpr = 1.5f;
    sceneSession->property_->SetDisplayId(100);
    int32_t pointerAreaFivePx = static_cast<int32_t>(POINTER_CHANGE_AREA_FIVE * vpr);
    int32_t pointerAreaSixteenPx = static_cast<int32_t>(POINTER_CHANGE_AREA_SIXTEEN * vpr);
    WindowLimits limits;
    limits.maxHeight_ = 1;
    limits.minHeight_ = 0;
    limits.maxWidth_ = 0;
    limits.minWidth_ = 0;
    sceneSession->property_->SetWindowLimits(limits);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    std::vector<int32_t> compare2 = {POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx,
        POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
        pointerAreaFivePx, POINTER_CHANGE_AREA_DEFAULT,  POINTER_CHANGE_AREA_DEFAULT};
    ASSERT_EQ(compare2, pointerChangeAreas);
    limits.maxHeight_ = 0;
    limits.maxWidth_ = 1;
    sceneSession->property_->SetWindowLimits(limits);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    std::vector<int32_t> compare3 = {POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
        POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx, POINTER_CHANGE_AREA_DEFAULT,
        POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx};
    ASSERT_EQ(compare3, pointerChangeAreas);
    limits.maxHeight_ = 1;
    sceneSession->property_->SetWindowLimits(limits);
    sceneSession->property_->displayId_ = 0;
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    std::vector<int32_t> compare4 = pointerChangeAreas = {pointerAreaSixteenPx, pointerAreaFivePx,
        pointerAreaSixteenPx, pointerAreaFivePx, pointerAreaSixteenPx,
        pointerAreaFivePx, pointerAreaSixteenPx, pointerAreaFivePx};
    ASSERT_EQ(compare4, pointerChangeAreas);
    manager_->UpdatePointerAreas(nullptr, pointerChangeAreas);
    ASSERT_EQ(compare4, pointerChangeAreas);
}

/**
 * @tc.name: UpdatePrivacyMode
 * @tc.desc: UpdatePrivacyMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdatePrivacyMode, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    MMI::WindowInfo windowinfo;
    auto tempProperty = sceneSession->GetSessionProperty();
    sceneSession->property_ = nullptr;
    manager_->UpdatePrivacyMode(sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.privacyMode, MMI::SecureFlag::DEFAULT_MODE);
    sceneSession->property_ = tempProperty;
    sceneSession->property_->isPrivacyMode_ = true;
    sceneSession->property_->isSystemPrivacyMode_ = false;
    manager_->UpdatePrivacyMode(sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.privacyMode, MMI::SecureFlag::PRIVACY_MODE);
    sceneSession->property_->isPrivacyMode_ = false;
    sceneSession->property_->isSystemPrivacyMode_ = true;
    manager_->UpdatePrivacyMode(sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.privacyMode, MMI::SecureFlag::PRIVACY_MODE);
    sceneSession->property_->isPrivacyMode_ = false;
    sceneSession->property_->isSystemPrivacyMode_ = false;
    manager_->UpdatePrivacyMode(sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.privacyMode, MMI::SecureFlag::DEFAULT_MODE);
    manager_->UpdatePrivacyMode(nullptr, windowinfo);
    ASSERT_EQ(windowinfo.privacyMode, MMI::SecureFlag::DEFAULT_MODE);
}

/**
 * @tc.name: AddModalExtensionWindowInfo
 * @tc.desc: AddModalExtensionWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, AddModalExtensionWindowInfo, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::WindowInfo windowInfo;
    windowInfoList.emplace_back(windowInfo);
    ExtensionWindowEventInfo extensionInfo = { .persistentId = 12345, .pid = 1234 };
    manager_->AddModalExtensionWindowInfo(windowInfoList, windowInfo, nullptr, extensionInfo);
    EXPECT_EQ(windowInfoList.size(), 1);

    sceneSession->AddNormalModalUIExtension(extensionInfo);
    manager_->AddModalExtensionWindowInfo(windowInfoList, windowInfo, sceneSession, extensionInfo);
    ASSERT_EQ(windowInfoList.size(), 2);
    EXPECT_TRUE(windowInfoList[1].defaultHotAreas.empty());

    Rect windowRect{ 1, 1, 7, 8 };
    extensionInfo.windowRect = windowRect;
    sceneSession->UpdateNormalModalUIExtension(extensionInfo);
    manager_->AddModalExtensionWindowInfo(windowInfoList, windowInfo, sceneSession, extensionInfo);
    ASSERT_EQ(windowInfoList.size(), 3);
    EXPECT_EQ(windowInfoList[2].defaultHotAreas.size(), 1);
}

/**
 * @tc.name: GetHostComponentWindowInfo
 * @tc.desc: GetHostComponentWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetHostComponentWindowInfo, TestSize.Level1)
{
    MMI::WindowInfo hostWindowinfo;
    SecSurfaceInfo secSurfaceInfo;
    Matrix3f transform;
    MMI::WindowInfo ret;
    MMI::WindowInfo windowInfo;
    ret = manager_->GetHostComponentWindowInfo(secSurfaceInfo, hostWindowinfo, nullptr, transform);
    ASSERT_EQ(ret.id, windowInfo.id);

    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    ret = manager_->GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.id, windowInfo.id);

    SecRectInfo secRectInfo;
    secSurfaceInfo.upperNodes.emplace_back(secRectInfo);
    secSurfaceInfo.hostPid = 1;
    windowInfo.pid = 1;
    ret = manager_->GetHostComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.pid, windowInfo.pid);

    ret = manager_->GetHostComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.defaultHotAreas.size(), 1);

    sceneSession->SetIsStartMoving(true);
    ret = manager_->GetHostComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.agentPid, static_cast<int32_t>(getpid()));
}

/**
 * @tc.name: GetSecComponentWindowInfo
 * @tc.desc: GetSecComponentWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetSecComponentWindowInfo, TestSize.Level1)
{
    MMI::WindowInfo hostWindowinfo;
    SecSurfaceInfo secSurfaceInfo;
    Matrix3f transform;
    MMI::WindowInfo ret;
    MMI::WindowInfo windowInfo;
    ret = manager_->GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, nullptr, transform);
    ASSERT_EQ(ret.id, windowInfo.id);

    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    ret = manager_->GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.id, windowInfo.id);

    secSurfaceInfo.uiExtensionNodeId = 1;
    sceneSession->AddUIExtSurfaceNodeId(1, 1);
    windowInfo.privacyUIFlag = true;
    ret = manager_->GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.privacyUIFlag, windowInfo.privacyUIFlag);

    windowInfo.id = 1;
    ret = manager_->GetSecComponentWindowInfo(secSurfaceInfo, hostWindowinfo, sceneSession, transform);
    ASSERT_EQ(ret.id, windowInfo.id);
}

/**
 * @tc.name: GetSecSurfaceWindowinfoList
 * @tc.desc: GetSecSurfaceWindowinfoList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetSecSurfaceWindowinfoList, TestSize.Level1)
{
    MMI::WindowInfo hostWindowinfo;
    SecSurfaceInfo secSurfaceInfo;
    Matrix3f transform;
    MMI::WindowInfo windowInfo;
    auto ret = manager_->GetSecSurfaceWindowinfoList(nullptr, hostWindowinfo, transform);
    ASSERT_EQ(ret.size(), 0);

    std::vector<SecSurfaceInfo> secSurfaceInfoList;
    secSurfaceInfoList.emplace_back(secSurfaceInfo);
    manager_->secSurfaceInfoMap_.emplace(1, secSurfaceInfoList);
    ret = manager_->GetSecSurfaceWindowinfoList(nullptr, hostWindowinfo, transform);
    ASSERT_EQ(ret.size(), 0);

    SessionInfo info;
    info.abilityName_ = "TestAbilityName";
    info.bundleName_ = "TestBundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    surfaceNode->id_ = 1;
    sceneSession->surfaceNode_ = surfaceNode;
    ret = manager_->GetSecSurfaceWindowinfoList(sceneSession, hostWindowinfo, transform);
    ASSERT_EQ(ret.size(), 2);

    manager_->secSurfaceInfoMap_.emplace(0, secSurfaceInfoList);
    ret = manager_->GetSecSurfaceWindowinfoList(sceneSession, hostWindowinfo, transform);
    ASSERT_EQ(ret.size(), 2);

    manager_->secSurfaceInfoMap_.clear();
    manager_->secSurfaceInfoMap_.emplace(1, secSurfaceInfoList);
    ret = manager_->GetSecSurfaceWindowinfoList(sceneSession, hostWindowinfo, transform);
    ASSERT_EQ(ret.size(), 2);

    sceneSession->surfaceNode_ = nullptr;
    ret = manager_->GetSecSurfaceWindowinfoList(sceneSession, hostWindowinfo, transform);
    ASSERT_EQ(ret.size(), 0);
}

/**
 * @tc.name: UpdateSecSurfaceInfo
 * @tc.desc: UpdateSecSurfaceInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateSecSurfaceInfo, TestSize.Level1)
{
    std::map<uint64_t, std::vector<SecSurfaceInfo>> secSurfaceInfoMap;
    SecRectInfo secRectInfo1;
    SecRectInfo secRectInfo2;
    SecSurfaceInfo secSurfaceInfo1;
    secSurfaceInfo1.upperNodes.emplace_back(secRectInfo1);
    SecSurfaceInfo secSurfaceInfo2;
    secSurfaceInfo2.upperNodes.emplace_back(secRectInfo2);
    std::vector<SecSurfaceInfo> secSurfaceInfoList1;
    std::vector<SecSurfaceInfo> secSurfaceInfoList2;
    secSurfaceInfoList1.emplace_back(secSurfaceInfo1);
    secSurfaceInfoList2.emplace_back(secSurfaceInfo2);
    manager_->secSurfaceInfoMap_.emplace(1, secSurfaceInfoList1);
    secSurfaceInfoMap.emplace(1, secSurfaceInfoList2);
    manager_->UpdateSecSurfaceInfo(secSurfaceInfoMap);
    ASSERT_EQ(secSurfaceInfoMap.size(), manager_->secSurfaceInfoMap_.size());

    secSurfaceInfoMap.emplace(2, secSurfaceInfoList2);
    manager_->UpdateSecSurfaceInfo(secSurfaceInfoMap);
    ASSERT_EQ(secSurfaceInfoMap.size(), manager_->secSurfaceInfoMap_.size());

    secSurfaceInfoMap.clear();
    manager_->secSurfaceInfoMap_.clear();
    secSurfaceInfoList1.clear();
    secSurfaceInfoList2.clear();
    secSurfaceInfo1.uiExtensionRectInfo.scale[0] = 1;
    secSurfaceInfoList1.emplace_back(secSurfaceInfo1);
    secSurfaceInfoList2.emplace_back(secSurfaceInfo2);
    secSurfaceInfoMap.emplace(1, secSurfaceInfoList1);
    manager_->secSurfaceInfoMap_.emplace(1, secSurfaceInfoList2);
    manager_->UpdateSecSurfaceInfo(secSurfaceInfoMap);
    ASSERT_EQ(secSurfaceInfoMap.size(), manager_->secSurfaceInfoMap_.size());
}

/**
 * @tc.name: UpdateSecSurfaceInfoTest
 * @tc.desc: UpdateSecSurfaceInfo test for same upperNodes
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateSecSurfaceInfoTest, TestSize.Level1)
{
    std::map<uint64_t, std::vector<SecSurfaceInfo>> secSurfaceInfoMap;
    SecRectInfo secRectInfo1;
    SecRectInfo secRectInfo2;
    SecSurfaceInfo secSurfaceInfo1;
    secSurfaceInfo1.upperNodes.emplace_back(secRectInfo1);
    secSurfaceInfo1.uiExtensionRectInfo.scale[0] = 1;
    SecSurfaceInfo secSurfaceInfo2;
    secSurfaceInfo2.upperNodes.emplace_back(secRectInfo2);
    secSurfaceInfo2.upperNodes.emplace_back(secRectInfo2);
    SecSurfaceInfo secSurfaceInfo3;
    std::vector<SecSurfaceInfo> secSurfaceInfoList1;
    std::vector<SecSurfaceInfo> secSurfaceInfoList2;
    secSurfaceInfoList1.emplace_back(secSurfaceInfo1);
    secSurfaceInfoList2.emplace_back(secSurfaceInfo2);
    secSurfaceInfoList2.emplace_back(secSurfaceInfo3);

    secSurfaceInfoMap.emplace(1001, secSurfaceInfoList1);
    secSurfaceInfoMap.emplace(1002, secSurfaceInfoList2);
    manager_->secSurfaceInfoMap_.clear();
    manager_->secSurfaceInfoMap_.emplace(1, secSurfaceInfoList1);
    manager_->UpdateSecSurfaceInfo(secSurfaceInfoMap);
    ASSERT_EQ(secSurfaceInfoMap.size(), manager_->secSurfaceInfoMap_.size());
}

/**
 * @tc.name: GetLastConstrainedModalUIExtInfo
 * @tc.desc: GetLastConstrainedModalUIExtInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetLastConstrainedModalUIExtInfo, TestSize.Level1)
{
    SecSurfaceInfo constrainedModalUIExtInfo;
    bool ret = manager_->GetLastConstrainedModalUIExtInfo(nullptr, constrainedModalUIExtInfo);
    ASSERT_EQ(ret, false);

    std::vector<SecSurfaceInfo> surfaceInfoList;
    SecSurfaceInfo secSurfaceInfo;
    manager_->constrainedModalUIExtInfoMap_.clear();
    manager_->constrainedModalUIExtInfoMap_[0] = surfaceInfoList;
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ret = manager_->GetLastConstrainedModalUIExtInfo(sceneSession, constrainedModalUIExtInfo);
    ASSERT_EQ(ret, false);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    surfaceNode->SetId(1);
    sceneSession->SetSurfaceNode(surfaceNode);
    ret = manager_->GetLastConstrainedModalUIExtInfo(sceneSession, constrainedModalUIExtInfo);
    ASSERT_EQ(ret, false);

    sceneSession->GetSurfaceNode()->SetId(0);
    ret = manager_->GetLastConstrainedModalUIExtInfo(sceneSession, constrainedModalUIExtInfo);
    ASSERT_EQ(ret, false);

    manager_->constrainedModalUIExtInfoMap_[0].emplace_back(secSurfaceInfo);
    ret = manager_->GetLastConstrainedModalUIExtInfo(sceneSession, constrainedModalUIExtInfo);
    ASSERT_EQ(ret, true);
    manager_->constrainedModalUIExtInfoMap_.clear();
}

/**
 * @tc.name: GetModalUIExtensionInfo
 * @tc.desc: GetModalUIExtensionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetModalUIExtensionInfo, TestSize.Level1)
{
    std::vector<MMI::WindowInfo> windowInfoList;
    MMI::WindowInfo windowInfo;

    // normal modal UIExt
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    int len = windowInfoList.size();
    manager_->GetModalUIExtensionInfo(windowInfoList, sceneSession, windowInfo);

    ExtensionWindowEventInfo extensionInfo;
    extensionInfo.persistentId = 12345;
    extensionInfo.pid = 1234;
    extensionInfo.windowRect = { 1, 2, 3, 4 };
    sceneSession->AddNormalModalUIExtension(extensionInfo);
    manager_->GetModalUIExtensionInfo(windowInfoList, sceneSession, windowInfo);
    ASSERT_EQ(len + 1, windowInfoList.size());

    // constrained modal UIExt
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    surfaceNode->SetId(0);
    sceneSession->SetSurfaceNode(surfaceNode);
    std::vector<SecSurfaceInfo> surfaceInfoList;
    SecSurfaceInfo secSurfaceInfo;
    manager_->constrainedModalUIExtInfoMap_.clear();
    manager_->constrainedModalUIExtInfoMap_[0] = surfaceInfoList;
    manager_->constrainedModalUIExtInfoMap_[0].emplace_back(secSurfaceInfo);
    manager_->GetModalUIExtensionInfo(windowInfoList, sceneSession, windowInfo);
    ASSERT_EQ(len + 2, windowInfoList.size());
    manager_->constrainedModalUIExtInfoMap_.clear();
}

/**
 * @tc.name: ResetFlushWindowInfoTask
 * @tc.desc: ResetFlushWindowInfoTask
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, ResetFlushWindowInfoTask, TestSize.Level1)
{
    auto preFlushWindowInfoCallback = manager_->flushWindowInfoCallback_;
    manager_->flushWindowInfoCallback_ = nullptr;
    manager_->ResetFlushWindowInfoTask();
    EXPECT_TRUE(manager_->hasPostTask_.load());
    manager_->flushWindowInfoCallback_ = preFlushWindowInfoCallback;
}

/**
 * @tc.name: ResetFlushWindowInfoTask1
 * @tc.desc: ResetFlushWindowInfoTask1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, ResetFlushWindowInfoTask1, TestSize.Level1)
{
    manager_->ResetFlushWindowInfoTask();
    EXPECT_TRUE(manager_->hasPostTask_.load());
}

/**
 * @tc.name: DumpRect
 * @tc.desc: DumpRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, DumpRect, TestSize.Level1)
{
    std::vector<MMI::Rect> rects(0);
    for (int i = 0; i < 2; i++) {
        MMI::Rect rect = { .x = i * 10, .y = i * 10, .width = i * 10, .height = i * 10 };
        rects.emplace_back(rect);
    }
    std::string ret = DumpRect(rects);
    std::string checkStr = "hot:0,0,0,0|10,10,10,10|";
    ASSERT_EQ(ret, checkStr);
}

/**
 * To test UpdatePointerAreas with different drag activated settings
 * To expect pointerChangeAreas changes when dragActivated and dragEnable both are true
 *
 * @tc.name: CheckDragActivatedInUpdatePointerAreas
 * @tc.desc: CheckDragActivatedInUpdatePointerAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CheckDragActivatedInUpdatePointerAreas, TestSize.Level1)
{
    std::vector<int32_t> pointerChangeAreas;
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    sceneSession->property_->SetDragEnabled(true);
    sceneSession->SetDragActivated(false);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    ASSERT_EQ(0, pointerChangeAreas.size());

    sceneSession->property_->SetDragEnabled(false);
    sceneSession->SetDragActivated(true);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    ASSERT_EQ(0, pointerChangeAreas.size());

    sceneSession->property_->SetDragEnabled(false);
    sceneSession->SetDragActivated(false);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    ASSERT_EQ(0, pointerChangeAreas.size());

    sceneSession->property_->SetDragEnabled(true);
    sceneSession->SetDragActivated(true);
    float vpr = 1.5f;
    sceneSession->property_->SetDisplayId(100);
    int32_t pointerAreaFivePx = static_cast<int32_t>(POINTER_CHANGE_AREA_FIVE * vpr);
    WindowLimits limits;
    limits.maxHeight_ = 1;
    limits.minHeight_ = 0;
    limits.maxWidth_ = 0;
    limits.minWidth_ = 0;
    sceneSession->property_->SetWindowLimits(limits);
    manager_->UpdatePointerAreas(sceneSession, pointerChangeAreas);
    std::vector<int32_t> compare = { POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx,
                                     POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT,
                                     POINTER_CHANGE_AREA_DEFAULT, pointerAreaFivePx,
                                     POINTER_CHANGE_AREA_DEFAULT, POINTER_CHANGE_AREA_DEFAULT };
    ASSERT_EQ(compare, pointerChangeAreas);
}

/**
 * @tc.name: GetSingleHandData
 * @tc.desc: get session data in single hand mode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetSingleHandData, TestSize.Level1)
{
    SingleHandData testSingleHandData;
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    sceneSession->property_->SetDisplayId(2000);
    SingleHandData res = manager_->GetSingleHandData(sceneSession);
    ASSERT_EQ(testSingleHandData.singleHandY, res.singleHandY);
    sceneSession->property_->SetDisplayId(0);
    res = manager_->GetSingleHandData(sceneSession);
    ASSERT_EQ(testSingleHandData.singleHandY, res.singleHandY);
}

/**
 * @tc.name: IsFilterSession02
 * @tc.desc: IsFilterSession02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, IsFilterSession02, TestSize.Level1)
{
    bool ret = manager_->IsFilterSession(nullptr);
    ASSERT_EQ(ret, true);
    SessionInfo info;
    info.abilityName_ = "test3";
    info.bundleName_ = "test4";
    info.sceneType_ = SceneType::WINDOW_SCENE;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sceneSession->GetSessionProperty();
    sceneSession->sessionInfo_ = info;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, true);
    info.isSystem_ = false;
    sceneSession->sessionInfo_ = info;
    sceneSession->isVisible_ = false;
    sceneSession->isSystemActive_ = false;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, true);
    info.isSystem_ = true;
    sceneSession->sessionInfo_ = info;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, true);
    sceneSession->isVisible_ = true;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    sceneSession->isSystemActive_ = true;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    info.isSystem_ = true;
    sceneSession->sessionInfo_ = info;
    sceneSession->isVisible_ = true;
    sceneSession->SetSystemActive(true);
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, false);
    info.isSystem_ = false;
    sceneSession->sessionInfo_ = info;
    sceneSession->isVisible_ = false;
    sceneSession->SetSystemActive(false);
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, true);
    sceneSession->isVisible_ = false;
    ret = manager_->IsFilterSession(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CalNotRotateTransform
 * @tc.desc: CalNotRotateTransform
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalNotRotateTransform, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalNotRotateTransform";
    sessionInfo.moduleName_ = "sessionInfo";
    Matrix3f transform;
    Matrix3f testTransform = transform;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    manager_->CalNotRotateTransform(nullptr, transform);
    ASSERT_EQ(transform, testTransform);
    auto screenId = 0;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    manager_->CalNotRotateTransform(sceneSession, transform);
    ASSERT_EQ(transform, testTransform);
    ScreenProperty screenProperty0;
    screenProperty0.SetRotation(0.0f);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::UNKNOWN);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    testTransform.SetZero();
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    manager_->CalNotRotateTransform(sceneSession, transform);
    ASSERT_EQ(transform, testTransform);
    screenProperty0.SetRotation(90.0f);
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    manager_->CalNotRotateTransform(sceneSession, transform);
    ASSERT_EQ(transform, testTransform);
    screenProperty0.SetRotation(180.0f);
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    manager_->CalNotRotateTransform(sceneSession, transform);
    ASSERT_EQ(transform, testTransform);
    screenProperty0.SetRotation(270.0f);
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    manager_->CalNotRotateTransform(sceneSession, transform);
    ASSERT_EQ(transform, testTransform);
}

/**
 * @tc.name: CalNotRotateTransform2
 * @tc.desc: CalNotRotateTransform2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalNotRotateTransform2, TestSize.Level1)
{
    SessionInfo sessionInfo;
    Matrix3f transform;
    Matrix3f testTransform = transform;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    auto screenId = 0;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    ScreenProperty screenProperty0;
    screenProperty0.SetRotation(0.0f);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::UNKNOWN);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    testTransform.SetZero();
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    auto oriProperty = sceneSession->GetSessionProperty();
    sceneSession->property_ = nullptr;
    manager_->CalNotRotateTransform(sceneSession, transform);
    ASSERT_EQ(transform, testTransform);
    sceneSession->property_ = oriProperty;
}

/**
 * @tc.name: CalSpecialNotRotateTransformTest
 * @tc.desc: CalSpecialNotRotateTransform
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalSpecialNotRotateTransformTest, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenProperty screenProperty;
    Matrix3f transform;
    manager_->CalSpecialNotRotateTransform(nullptr, screenProperty, transform);
    EXPECT_TRUE(g_logMsg.find("sceneSession is null") != std::string::npos);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalSpecialNotRotateTransformTest";
    sessionInfo.moduleName_ = "sessionInfo";
    Matrix3f testTransform = transform;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    manager_->CalSpecialNotRotateTransform(sceneSession, screenProperty, transform);
    EXPECT_TRUE(g_logMsg.find("sceneSession is null") == std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateWindowFlags
 * @tc.desc: UpdateWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateWindowFlags, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    MMI::WindowInfo windowinfo;
    auto screenId = 0;
    manager_->UpdateWindowFlags(screenId, sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.flags, 0);
    ScreenProperty screenProperty0;
    screenProperty0.SetRotation(0.0f);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    screenSession->SetTouchEnabledFromJs(true);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::UNKNOWN);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    manager_->UpdateWindowFlags(screenId, sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.flags, 0);
    screenSession->SetTouchEnabledFromJs(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    ScreenSessionManagerClient::GetInstance().OnPropertyChanged(screenId, screenProperty0, reason);
    manager_->UpdateWindowFlags(screenId, sceneSession, windowinfo);
    ASSERT_EQ(windowinfo.flags, 0);
}

/**
 * @tc.name: CalTransform
 * @tc.desc: CalTransform
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransform, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransform";
    sessionInfo.moduleName_ = "CalTransform";
    SingleHandData testSingleHandData;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f translate = sceneSession->GetSessionGlobalPosition(false);
    Vector2f offset = sceneSession->GetSessionGlobalPosition(false);
    Matrix3f transform;
    Matrix3f testTransform = transform;
    manager_->CalTransform(nullptr, transform, testSingleHandData);
    ASSERT_EQ(transform, testTransform);
    sessionInfo.isRotable_ = true;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    sessionInfo.isSystem_ = true;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    sessionInfo.isRotable_ = false;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    sessionInfo.isRotable_ = true;
    sessionInfo.isSystem_ = false;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    sessionInfo.isSystem_ = true;
    auto preScreenSessionManager = ScreenSessionManagerClient::GetInstance().screenSessionManager_;
    ScreenSessionManagerClient::GetInstance().screenSessionManager_ = nullptr;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    ScreenSessionManagerClient::GetInstance().screenSessionManager_ = preScreenSessionManager;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    testSingleHandData.singleHandY = 0;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
}

/**
 * @tc.name: CalTransform_CompatMode
 * @tc.desc: CalTransform_CompatMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransform_CompatMode, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransform_CompatMode";
    sessionInfo.moduleName_ = "CalTransform_CompatMode";
    sessionInfo.isRotable_ = true;
    SingleHandData testSingleHandData;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);

    session->SetScale(1.0f, 1.0f, 0.5f, 0.5f);
    Vector2f scale(session->GetScaleX(), session->GetScaleY());
    Vector2f translate = session->GetSessionGlobalPosition(false);
    Matrix3f transform;
    Matrix3f testTransform = Matrix3f::IDENTITY;

    manager_->CalTransform(session, transform, testSingleHandData, false);
    EXPECT_EQ(transform, testTransform.Translate(translate).Scale(scale, 0.5f, 0.5f).Inverse());
    manager_->CalTransform(session, transform, testSingleHandData, true);
    EXPECT_EQ(transform, testTransform.Translate(translate).Scale(scale, 0.5f, 0.5f).Inverse());

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);

    manager_->CalTransform(session, transform, testSingleHandData, true);
    EXPECT_EQ(transform, testTransform.Translate(translate).Scale(scale, 0.5f, 0.5f).Inverse());
    compatibleModeProperty->SetIsAdaptToImmersive(false);
    manager_->CalTransform(session, transform, testSingleHandData, true);
    EXPECT_EQ(transform, testTransform.Translate(translate).Scale(scale, 0.5f, 0.5f).Inverse());

    session->SetScale(0.5f, 0.5f, 0.5f, 0.5f);
    scale[0] = session->GetScaleX();
    scale[1] = session->GetScaleY();

    manager_->CalTransform(session, transform, testSingleHandData, true);
    EXPECT_EQ(transform, testTransform.Translate(translate).Scale(scale, 0.5f, 0.5f).Inverse());

    ExtensionWindowEventInfo extensionInfo;
    extensionInfo.persistentId = 12345;
    extensionInfo.pid = 1234;
    extensionInfo.windowRect = { 0, 120, 1260, 2600 };
    session->AddNormalModalUIExtension(extensionInfo);
    session->isScbCoreEnabled_ = true;
    session->SetSessionGlobalRect({ 0, 0, 1260, 2720 });
    translate = session->GetSessionGlobalPosition(true);
    Vector2f translateOffset(0, 60);

    manager_->CalTransform(session, transform, testSingleHandData, true);
    testTransform = Matrix3f::IDENTITY;
    EXPECT_EQ(transform, testTransform.Translate(translate).Scale(scale, 0.5f, 0.5f)
        .Inverse().Translate(translateOffset));
}

/**
 * @tc.name: CalTransformSessionPropertyNullptr
 * @tc.desc: CalTransform
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransformSessionPropertyNullptr, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransformSessionPropertyNullptr";
    sessionInfo.moduleName_ = "CalTransformSessionPropertyNullptr";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    Matrix3f transform;
    SingleHandData testSingleHandData;
    
    auto oriProperty = sceneSession->GetSessionProperty();
    sceneSession->property_ = nullptr;
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    EXPECT_TRUE(g_logMsg.find("sessionProperty is null") != std::string::npos);
    sceneSession->property_ = oriProperty;
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: CalTransformTest_01
 * @tc.desc: CalTransform isRotate false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransformTest_01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransformTest_01";
    sessionInfo.moduleName_ = "CalTransformTest_01";
    sessionInfo.isRotable_ = false;
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f translate = sceneSession->GetSessionGlobalPosition(false);
    Vector2f offset = sceneSession->GetSessionGlobalPosition(false);
    Matrix3f transform;
    SingleHandData testSingleHandData;

    auto screenId = 1001;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    auto displayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::UNKNOWN);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());

    sceneSession->sessionInfo_.bundleName_ = "CalTransformTest_SCBScreenLock_01";
    screenSession->GetScreenProperty().SetPhysicalRotation(180.0f);
    screenSession->GetScreenProperty().SetScreenComponentRotation(90.0f);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);

    sceneSession->sessionInfo_.bundleName_ = "CalTransformTest_SCBDesktop_01";
    manager_->CalTransform(sceneSession, transform, testSingleHandData);

    sceneSession->sessionInfo_.bundleName_ = "CalTransformTest_SCBScreenLock_01";
    screenSession->GetScreenProperty().SetPhysicalRotation(180.0f);
    screenSession->GetScreenProperty().SetScreenComponentRotation(180.0f);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: CalTransformTest_02
 * @tc.desc: CalTransform isRotate false isRotateWindow false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransformTest_02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransformTest_02";
    sessionInfo.moduleName_ = "CalTransformTest_02";
    sessionInfo.isRotable_ = false;
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f translate = sceneSession->GetSessionGlobalPosition(false);
    Vector2f offset = sceneSession->GetSessionGlobalPosition(false);
    Matrix3f transform;
    SingleHandData testSingleHandData;
    auto displayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();

    auto screenId = 1001;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    screenSession->GetScreenProperty().SetPhysicalRotation(180.0f);
    screenSession->GetScreenProperty().SetScreenComponentRotation(90.0f);

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    sceneSession->SetCurrentRotation(90);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::FULL);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());

    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::GLOBAL_FULL);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());

    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::MAIN);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::COORDINATION);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.erase(screenId);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: CalTransformTest_03
 * @tc.desc: CalTransform isRotate true isRotateWindow false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransformTest_03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransformTest_SCBScreenLock_03";
    sessionInfo.moduleName_ = "CalTransformTest_03";
    sessionInfo.isRotable_ = true;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f translate = sceneSession->GetSessionGlobalPosition(false);
    Vector2f offset = sceneSession->GetSessionGlobalPosition(false);
    Matrix3f transform;
    SingleHandData testSingleHandData;
    auto displayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();

    auto screenId = 1001;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    screenSession->GetScreenProperty().SetPhysicalRotation(180.0f);
    screenSession->GetScreenProperty().SetScreenComponentRotation(90.0f);

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    sceneSession->SetCurrentRotation(90);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::UNKNOWN);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.erase(screenId);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: CalTransformTest_04
 * @tc.desc: CalTransform isRotate true isRotateWindow true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, CalTransformTest_04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CalTransformTest_SCBScreenLock_04";
    sessionInfo.moduleName_ = "CalTransformTest_04";
    sessionInfo.isRotable_ = true;
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    Vector2f scale(sceneSession->GetScaleX(), sceneSession->GetScaleY());
    Vector2f translate = sceneSession->GetSessionGlobalPosition(false);
    Vector2f offset = sceneSession->GetSessionGlobalPosition(false);
    Matrix3f transform;
    SingleHandData testSingleHandData;
    auto displayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();

    auto screenId = 1001;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    screenSession->GetScreenProperty().SetPhysicalRotation(180.0f);
    screenSession->GetScreenProperty().SetScreenComponentRotation(90.0f);

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    sceneSession->SetCurrentRotation(0);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::MAIN);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());

    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(FoldDisplayMode::FULL);
    manager_->CalTransform(sceneSession, transform, testSingleHandData);
    ASSERT_EQ(transform, transform.Translate(translate)
        .Scale(scale, sceneSession->GetPivotX(), sceneSession->GetPivotY()).Inverse());
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.erase(screenId);
    ScreenSessionManagerClient::GetInstance().OnUpdateFoldDisplayMode(displayMode);
}

/**
 * @tc.name: UpdateModalExtensionInCompatStatus
 * @tc.desc: UpdateModalExtensionInCompatStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateModalExtensionInCompatStatus, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "UpdateModalExtensionInCompatStatus";
    sessionInfo.moduleName_ = "UpdateModalExtensionInCompatStatus";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);

    Matrix3f transform;
    EXPECT_FALSE(manager_->UpdateModalExtensionInCompatStatus(nullptr, transform));
    EXPECT_FALSE(manager_->UpdateModalExtensionInCompatStatus(session, transform));
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->SetScale(0.5f, 0.5f, 0.5f, 0.5f);
    EXPECT_FALSE(manager_->UpdateModalExtensionInCompatStatus(session, transform));

    ExtensionWindowEventInfo extensionInfo;
    extensionInfo.persistentId = 12345;
    extensionInfo.pid = 1234;
    extensionInfo.windowRect = { 0, 120, 1260, 2600 };
    session->AddNormalModalUIExtension(extensionInfo);

    EXPECT_TRUE(manager_->UpdateModalExtensionInCompatStatus(session, transform));
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    EXPECT_TRUE(manager_->UpdateModalExtensionInCompatStatus(session, transform));
}

/**
 * @tc.name: UpdateHotAreas
 * @tc.desc: UpdateHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateHotAreas, TestSize.Level0)
{
    std::vector<MMI::Rect> touchHotAreas(0);
    std::vector<MMI::Rect> pointerHotAreas(0);
    manager_->UpdateHotAreas(nullptr, touchHotAreas, pointerHotAreas);
    ASSERT_EQ(touchHotAreas.size(), 0);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "UpdateHotAreas";
    sessionInfo.moduleName_ = "UpdateHotAreas";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    std::vector<OHOS::Rosen::Rect> touchHotAreasInSceneSession(0);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    sceneSession->persistentId_ = 1;
    for (int i = 0; i < 2; i++) {
        OHOS::Rosen::Rect area;
        area.posX_ = i * 10;
        area.posY_ = i * 10;
        area.width_ = 10;
        area.height_ = 10;
        touchHotAreasInSceneSession.emplace_back(area);
    }
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    touchHotAreas.clear();
    pointerHotAreas.clear();
    manager_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    ASSERT_EQ(touchHotAreas.size(), 2);
    for (int i = 2; i < 10; i++) {
        OHOS::Rosen::Rect area;
        area.posX_ = i * 10;
        area.posY_ = i * 10;
        area.width_ = 10;
        area.height_ = 10;
        touchHotAreasInSceneSession.emplace_back(area);
    }
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    touchHotAreas.clear();
    pointerHotAreas.clear();
    manager_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    ASSERT_EQ(touchHotAreas.size(), 10);
    std::vector<OHOS::Rosen::Rect> fullSceneSession(static_cast<uint32_t>(MMI::WindowInfo::MAX_HOTAREA_COUNT));
    sceneSession->GetSessionProperty()->SetTouchHotAreas(fullSceneSession);
    touchHotAreas.clear();
    pointerHotAreas.clear();
    manager_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    ASSERT_EQ(touchHotAreas.size(), 1);
}

/**
 * @tc.name: UpdateHotAreas2
 * @tc.desc: UpdateHotAreas2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateHotAreas2, TestSize.Level1)
{
    std::vector<MMI::Rect> touchHotAreas(0);
    std::vector<MMI::Rect> pointerHotAreas(0);
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    std::vector<OHOS::Rosen::Rect> touchHotAreasInSceneSession(0);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    sceneSession->GetSessionProperty()->type_ = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    touchHotAreas.clear();
    pointerHotAreas.clear();
    sceneSession->GetSessionProperty()->SetDisplayId(-100);
    manager_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    ASSERT_EQ(touchHotAreas.size(), 1);
}

/**
 * @tc.name: UpdateDefaultHotAreas
 * @tc.desc: UpdateDefaultHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateDefaultHotAreas, TestSize.Level1)
{
    std::vector<MMI::Rect> empty(0);
    manager_->UpdateDefaultHotAreas(nullptr, empty, empty);
    ASSERT_EQ(empty.size(), 0);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "UpdateDefaultHotAreas";
    sessionInfo.moduleName_ = "UpdateDefaultHotAreas";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    float vpr = 1.5f; // 1.5: default vp
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    if (screenSession != nullptr) {
        vpr = screenSession->GetScreenProperty().GetDensity();
    }
    uint32_t offset = static_cast<uint32_t>(HOTZONE_TOUCH * vpr);
    WSRect rect = { 0, 0, 320, 240 };
    sceneSession->SetSessionRect(rect);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, -offset);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, -offset);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, -offset);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, 0);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, -offset);
    sceneSession->singleHandModeFlag_ = true;
    SceneSessionManager::GetInstance().singleHandScreenInfo_.mode = SingleHandMode::LEFT;
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, -offset);
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, 0);
    SceneSessionManager::GetInstance().singleHandScreenInfo_.mode = SingleHandMode::MIDDLE;
}

/**
 * @tc.name: UpdateDefaultHotAreas2
 * @tc.desc: UpdateDefaultHotAreas2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, UpdateDefaultHotAreas2, TestSize.Level1)
{
    std::vector<MMI::Rect> empty(0);
    manager_->UpdateDefaultHotAreas(nullptr, empty, empty);
    EXPECT_EQ(empty.size(), 0);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "UpdateDefaultHotAreas";
    sessionInfo.moduleName_ = "UpdateDefaultHotAreas";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    float vpr = 1.5f; // 1.5: default vp
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    if (screenSession != nullptr) {
        vpr = screenSession->GetScreenProperty().GetDensity();
    }
    uint32_t offset = static_cast<uint32_t>(HOTZONE_TOUCH * vpr);
    WSRect rect = { 0, 0, 320, 240 };
    sceneSession->SetSessionRect(rect);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->GetSessionProperty()->dragEnabled_ = true;
    sceneSession->dragActivated_ = true;
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, -offset);
    sceneSession->dragActivated_ = false;
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, 0);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, 0);
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    empty.clear();
    manager_->UpdateDefaultHotAreas(sceneSession, empty, empty);
    ASSERT_EQ(empty[0].x, 0);
}

/**
 * @tc.name: GetWindowInfo
 * @tc.desc: GetWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, GetWindowInfo, TestSize.Level0)
{
    manager_->GetWindowInfo(nullptr, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    SessionInfo info;
    info.abilityName_ = "111";
    info.bundleName_ = "111";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = session->GetSessionProperty();
    session->SetSessionProperty(windowSessionProperty);
    manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    windowSessionProperty->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    std::pair<MMI::WindowInfo, std::shared_ptr<Media::PixelMap>> ret;
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.id, session->GetWindowId());
    windowSessionProperty->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.windowType, static_cast<int32_t>(windowSessionProperty->GetWindowType()));
    windowSessionProperty->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.id, session->GetWindowId());
    info.isSetPointerAreas_ = true;
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.id, session->GetWindowId());
    windowSessionProperty->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING));
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.flags, static_cast<int32_t>(MMI::WindowInfo::FLAG_BIT_HANDWRITING));
    windowSessionProperty->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.flags, 0);
    ret = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
    ASSERT_EQ(ret.first.id, session->GetWindowId());
}

/**
 * @tc.name: ConvertDegreeToMMIRotation
 * @tc.desc: ConvertDegreeToMMIRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, ConvertDegreeToMMIRotation, TestSize.Level1)
{
    MMI::Direction dirction = MMI::DIRECTION0;
    dirction = ConvertDegreeToMMIRotation(0.0);
    ASSERT_EQ(dirction, MMI::DIRECTION0);
    dirction = ConvertDegreeToMMIRotation(90.0);
    ASSERT_EQ(dirction, MMI::DIRECTION90);
    dirction = ConvertDegreeToMMIRotation(180.0);
    ASSERT_EQ(dirction, MMI::DIRECTION180);
    dirction = ConvertDegreeToMMIRotation(270.0);
    ASSERT_EQ(dirction, MMI::DIRECTION270);
    dirction = ConvertDegreeToMMIRotation(30.0);
    ASSERT_EQ(dirction, MMI::DIRECTION0);
}

/**
 * @tc.name: ConvertToMMIRotation
 * @tc.desc: ConvertToMMIRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest, ConvertToMMIRotation, TestSize.Level1)
{
    MMI::Rotation rotation = MMI::Rotation::ROTATION_0;
    rotation = ConvertToMMIRotation(0.0);
    ASSERT_EQ(rotation, MMI::Rotation::ROTATION_0);
    rotation = ConvertToMMIRotation(90.0);
    ASSERT_EQ(rotation, MMI::Rotation::ROTATION_90);
    rotation = ConvertToMMIRotation(180.0);
    ASSERT_EQ(rotation, MMI::Rotation::ROTATION_180);
    rotation = ConvertToMMIRotation(270.0);
    ASSERT_EQ(rotation, MMI::Rotation::ROTATION_270);
    rotation = ConvertToMMIRotation(30.0);
    ASSERT_EQ(rotation, MMI::Rotation::ROTATION_0);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
