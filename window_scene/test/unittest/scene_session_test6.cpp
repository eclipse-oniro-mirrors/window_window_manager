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

#include <algorithm>
#include <gtest/gtest.h>
#include <pointer_event.h>
#include <transaction/rs_transaction.h>

#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_scene_session.h"
#include "mock_vsync_station.h"
#include "screen_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/main_session.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/system_session.h"
#include <ui/rs_surface_node.h>
#include "wm_common.h"
#include "dm_common.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME = 100000; // 100ms
std::string g_errlog;
void ScreenSessionLogCallback(const LogType type,
                              const LogLevel level,
                              const unsigned int domain,
                              const char* tag,
                              const char* msg)
{
    g_errlog = msg;
}
} // namespace

class SceneSessionTest6 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionTest6::SetUpTestCase() {}

void SceneSessionTest6::TearDownTestCase() {}

void SceneSessionTest6::SetUp() {}

void SceneSessionTest6::TearDown() {}

namespace {

/**
 * @tc.name: RegisterNotifySurfaceBoundsChangeFunc
 * @tc.desc: RegisterNotifySurfaceBoundsChangeFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RegisterNotifySurfaceBoundsChangeFunc01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    constexpr int sessionId = 10001;
    sceneSession->RegisterNotifySurfaceBoundsChangeFunc(sessionId, nullptr);
    ASSERT_EQ(nullptr, sceneSession->notifySurfaceBoundsChangeFuncMap_[sessionId]);

    auto task = [](const WSRect& rect, bool isGlobal, bool needFlush) {};
    sceneSession->RegisterNotifySurfaceBoundsChangeFunc(sessionId, std::move(task));
    ASSERT_NE(nullptr, sceneSession->notifySurfaceBoundsChangeFuncMap_[sessionId]);

    sceneSession->UnregisterNotifySurfaceBoundsChangeFunc(sessionId);
    ASSERT_EQ(nullptr, sceneSession->notifySurfaceBoundsChangeFuncMap_[sessionId]);
}

/**
 * @tc.name: NotifyUpdateGravity
 * @tc.desc: NotifyUpdateGravity
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyUpdateGravity01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    int32_t subSessionId = subSession->GetPersistentId();

    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, mainSession);

    constexpr int sessionId = 10001;
    auto task = [](const WSRect& rect, bool isGlobal, bool needFlush) {};
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(sessionId, std::move(task));
    mainSession->NotifyUpdateGravity();
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[sessionId]);

    subSession->isFollowParentLayout_ = false;
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(subSessionId, std::move(task));
    mainSession->NotifyUpdateGravity();
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSessionId]);

    subSession->isFollowParentLayout_ = true;
    mainSession->NotifyUpdateGravity();
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSessionId]);

    sptr<MoveDragController> followController =
        sptr<MoveDragController>::MakeSptr(wptr(subSession));
    ASSERT_NE(nullptr, followController);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    subSession->surfaceNode_ = nullptr;
    subSession->moveDragController_ = nullptr;
    mainSession->NotifyUpdateGravity();
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSessionId]);

    subSession->surfaceNode_ = surfaceNode;
    mainSession->NotifyUpdateGravity();
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSessionId]);

    subSession->moveDragController_ = followController;
    mainSession->NotifyUpdateGravity();
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSessionId]);
}

/**
 * @tc.name: GetSceneSessionById
 * @tc.desc: GetSceneSessionById
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, GetSceneSessionById01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> findSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->specificCallback_ = nullptr;
    sptr<SceneSession> ret = sceneSession->GetSceneSessionById(findSession->GetPersistentId());
    ASSERT_EQ(nullptr, ret);

    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, callBack);
    sceneSession->specificCallback_ = callBack;
    ret = sceneSession->GetSceneSessionById(findSession->GetPersistentId());
    ASSERT_EQ(nullptr, ret);

    auto task = [&findSession](int32_t persistentId) { return findSession; };
    callBack->onGetSceneSessionByIdCallback_ = task;
    ret = sceneSession->GetSceneSessionById(findSession->GetPersistentId());
    ASSERT_EQ(findSession->GetPersistentId(), ret->GetPersistentId());
}

/**
 * @tc.name: CheckAndGetAbilityInfoByWant
 * @tc.desc: CheckAndGetAbilityInfoByWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, CheckAndGetAbilityInfoByWant, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto want = std::make_shared<AAFwk::Want>();
    AppExecFwk::AbilityInfo abilityInfo;

    sceneSession->specificCallback_ = nullptr;
    bool ret = sceneSession->CheckAndGetAbilityInfoByWant(want, abilityInfo);
    EXPECT_FALSE(ret);

    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession->specificCallback_ = callBack;
    ret = sceneSession->CheckAndGetAbilityInfoByWant(want, abilityInfo);
    EXPECT_FALSE(ret);

    auto task = [](const std::shared_ptr<AAFwk::Want>& want, AppExecFwk::AbilityInfo& abilityInfo) {
        return true;
    };
    callBack->onCheckAndGetAbilityInfoByWantCallback_ = task;
    ret = sceneSession->CheckAndGetAbilityInfoByWant(want, abilityInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetSessionInfoByWant
 * @tc.desc: GetSessionInfoByWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, GetSessionInfoByWant, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "GetSessionInfoByWant";
    info.moduleName_ = "GetSessionInfoByWant";
    info.abilityName_ = "GetSessionInfoByWant";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto want = std::make_shared<AAFwk::Want>();
    want->SetElementName("GetSessionInfoByWant", "GetSessionInfoByWant",
        "GetSessionInfoByWant", "GetSessionInfoByWant");
    SessionInfo ret = SceneSession::GetSessionInfoByWant(want, sceneSession);
    EXPECT_EQ(ret.bundleName_, info.bundleName_);

    want->SetElementName("deviceId", "bundleName",
        "moduleName", "abilityName");
    ret = SceneSession::GetSessionInfoByWant(want, sceneSession);
    EXPECT_EQ(ret.bundleName_, want->GetElement().GetBundleName());
}

/**
 * @tc.name: RestartApp
 * @tc.desc: RestartApp
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RestartApp, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RestartApp";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::shared_ptr<AAFwk::Want> want = nullptr;
    EXPECT_EQ(sceneSession->RestartApp(want), WSError::WS_ERROR_INVALID_PARAM);

    want = std::make_shared<AAFwk::Want>();
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(sceneSession->RestartApp(want), WSError::WS_ERROR_INVALID_SESSION);

    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->state_ = SessionState::STATE_BACKGROUND;
    EXPECT_EQ(sceneSession->RestartApp(want), WSError::WS_ERROR_INVALID_PERMISSION);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_EQ(sceneSession->RestartApp(want), WSError::WS_ERROR_INVALID_OPERATION);

    want->SetBundle(info.bundleName_);
    EXPECT_EQ(sceneSession->RestartApp(want), WSError::WS_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: SetWindowAnchorInfoChangeFunc
 * @tc.desc: SetWindowAnchorInfoChangeFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetWindowAnchorInfoChangeFunc01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->SetWindowAnchorInfoChangeFunc(nullptr);
    EXPECT_EQ(nullptr, sceneSession->onWindowAnchorInfoChangeFunc_);

    NotifyWindowAnchorInfoChangeFunc func = [](const WindowAnchorInfo& windowAnchorInfo) {};
    sceneSession->SetWindowAnchorInfoChangeFunc(std::move(func));
    EXPECT_NE(nullptr, sceneSession->onWindowAnchorInfoChangeFunc_);
}

/**
 * @tc.name: SetWindowAnchorInfo
 * @tc.desc: SetWindowAnchorInfo01, check the param
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetWindowAnchorInfo01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WindowAnchorInfo windowAnchorInfo = { true, WindowAnchor::TOP_START, 0, 0 };

    WSError ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->property_ = property;
    property->subWindowLevel_ = 100;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);

    sceneSession->isFollowParentLayout_ = true;
    ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    sceneSession->isFollowParentLayout_ = false;
    sceneSession->systemConfig_.supportFollowRelativePositionToParent_ = true;
    ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: SetWindowAnchorInfo
 * @tc.desc: SetWindowAnchorInfo02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetWindowAnchorInfo02, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowRelativePositionToParent_ = true;
    // test set isAnchorEnabled_
    sceneSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    WindowAnchorInfo windowAnchorInfo = { true, WindowAnchor::TOP_START, 0, 0 };
    sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_TRUE(sceneSession->windowAnchorInfo_.isAnchorEnabled_);

    //test after set flag, call func
    std::shared_ptr<bool> isCall = std::make_shared<bool>(false);
    NotifyWindowAnchorInfoChangeFunc callback = [isCall](const WindowAnchorInfo& windowAnchorInfo) {
        *isCall = true;
    };
    sceneSession->SetWindowAnchorInfoChangeFunc(std::move(callback));
    EXPECT_NE(nullptr, sceneSession->onWindowAnchorInfoChangeFunc_);
    sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_TRUE(*isCall);
}

/**
 * @tc.name: CalcSubWindowRectByAnchor01
 * @tc.desc: CalcSubWindowRectByAnchor01, check the param
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, CalcSubWindowRectByAnchor01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowRelativePositionToParent_ = true;

    WindowAnchorInfo windowAnchorInfo = { false, WindowAnchor::TOP_START, 0, 0 };
    WSError ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_OK);

    WSRect parentRect;
    WSRect subRect;
    WSRect retRect;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    EXPECT_EQ(subRect, retRect);

    parentRect = {0, 0, 1000, 1000};
    subRect = {0, 0, 400, 400};
    retRect = {0, 0, 400, 400};
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    EXPECT_EQ(subRect, retRect);
}

/**
 * @tc.name: CalcSubWindowRectByAnchor02
 * @tc.desc: CalcSubWindowRectByAnchor02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, CalcSubWindowRectByAnchor02, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowRelativePositionToParent_ = true;

    WindowAnchorInfo windowAnchorInfo = { true, WindowAnchor::TOP_START, 0, 0 };
    WSError ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_OK);

    WSRect parentRect = {0, 0, 1000, 1000};
    WSRect subRect = {0, 0, 400, 400};
    WSRect retRect;

    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {0, 0, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::TOP;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {300, 0, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::TOP_END;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {600, 0, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::START;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {0, 300, 400, 400};
    EXPECT_EQ(subRect, retRect);
}

/**
 * @tc.name: CalcSubWindowRectByAnchor03
 * @tc.desc: CalcSubWindowRectByAnchor03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, CalcSubWindowRectByAnchor03, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowRelativePositionToParent_ = true;

    WindowAnchorInfo windowAnchorInfo = { true, WindowAnchor::CENTER, 0, 0 };
    WSError ret = sceneSession->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WSError::WS_OK);

    WSRect parentRect = {0, 0, 1000, 1000};
    WSRect subRect = {0, 0, 400, 400};
    WSRect retRect;

    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {300, 300, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::END;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {600, 300, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::BOTTOM_START;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {0, 600, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::BOTTOM;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {300, 600, 400, 400};
    EXPECT_EQ(subRect, retRect);

    sceneSession->windowAnchorInfo_.windowAnchor_ = WindowAnchor::BOTTOM_END;
    sceneSession->CalcSubWindowRectByAnchor(parentRect, subRect);
    retRect = {600, 600, 400, 400};
    EXPECT_EQ(subRect, retRect);
}

/**
 * @tc.name: SetFollowParentRectFunc
 * @tc.desc: SetFollowParentRectFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetFollowParentRectFunc01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->SetFollowParentRectFunc(nullptr);
    ASSERT_EQ(nullptr, sceneSession->followParentRectFunc_);

    NotifyFollowParentRectFunc func = [](bool isFollow) {};
    sceneSession->SetFollowParentRectFunc(std::move(func));
    ASSERT_NE(nullptr, sceneSession->followParentRectFunc_);
}

/**
 * @tc.name: SetFollowParentWindowLayoutEnabled
 * @tc.desc: SetFollowParentWindowLayoutEnabled01, check the param
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetFollowParentWindowLayoutEnabled01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    WSError ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowParentWindowLayout_ = true;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 100;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 1;
    ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->systemConfig_.supportFollowParentWindowLayout_ = false;
    ret = sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);
}


/**
 * @tc.name: SetFollowParentWindowLayoutEnabled
 * @tc.desc: SetFollowParentWindowLayoutEnabled02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetFollowParentWindowLayoutEnabled02, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowParentWindowLayout_ = true;
    // test set isFollowParentLayout_
    sceneSession->isFollowParentLayout_ = false;
    sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_TRUE(sceneSession->isFollowParentLayout_);
    // test after set flag, call func
    bool isCall = false;
    NotifyFollowParentRectFunc func = [&isCall](bool isFollow) { isCall = true; };
    sceneSession->SetFollowParentRectFunc(std::move(func));
    ASSERT_NE(nullptr, sceneSession->followParentRectFunc_);
    sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_TRUE(isCall);
}

/**
 * @tc.name: SetFollowParentWindowLayoutEnabled
 * @tc.desc: SetFollowParentWindowLayoutEnabled03, test register callback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetFollowParentWindowLayoutEnabled03, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->subWindowLevel_ = 1;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    sceneSession->systemConfig_.supportFollowParentWindowLayout_ = true;

    sptr<MainSession> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, parentSession);

    sceneSession->parentSession_ = parentSession;
    sceneSession->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_NE(nullptr, parentSession->notifySurfaceBoundsChangeFuncMap_[sceneSession->GetPersistentId()]);
    WSRect rect;
    parentSession->NotifySubAndDialogFollowRectChange(rect, false, false);

    sceneSession->SetFollowParentWindowLayoutEnabled(false);
    ASSERT_EQ(nullptr, parentSession->notifySurfaceBoundsChangeFuncMap_[sceneSession->GetPersistentId()]);
}

/**
 * @tc.name: NotifyKeyboardAnimationCompleted
 * @tc.desc: NotifyKeyboardAnimationCompleted
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardAnimationCompleted, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = property;
    sceneSession->sessionStage_ = nullptr;
    bool isShowAnimation = true;
    WSRect beginRect = { 0, 2720, 1260, 1020 };
    WSRect endRect = { 0, 1700, 1260, 1020 };
    sceneSession->NotifyKeyboardAnimationCompleted(isShowAnimation, beginRect, endRect);
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->sessionStage_);

    sceneSession->NotifyKeyboardDidShowRegistered(true);
    sceneSession->NotifyKeyboardAnimationCompleted(isShowAnimation, beginRect, endRect);

    isShowAnimation = false;
    beginRect = { 0, 1700, 1260, 1020 };
    endRect = { 0, 2720, 1260, 1020 };
    sceneSession->NotifyKeyboardAnimationCompleted(isShowAnimation, beginRect, endRect);
    sceneSession->NotifyKeyboardDidHideRegistered(true);
    sceneSession->NotifyKeyboardAnimationCompleted(isShowAnimation, beginRect, endRect);
}

/**
 * @tc.name: IsInCompatScaleStatus
 * @tc.desc: IsInCompatScaleStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, IsInCompatScaleStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsInCompatScaleStatus";
    info.bundleName_ = "IsInCompatScaleStatus";
    info.screenId_ = 0;
    sptr<SceneSession> session = sptr<MainSession>::MakeSptr(info, nullptr);

    EXPECT_FALSE(session->IsInCompatScaleStatus());
    session->SetScale(1.0f, 1.0f, 0.5f, 0.5f);

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_FALSE(session->IsInCompatScaleStatus());

    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    EXPECT_FALSE(session->IsInCompatScaleStatus());
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    EXPECT_FALSE(session->IsInCompatScaleStatus());

    session->SetScale(2.0f, 1.0f, 0.5f, 0.5f);
    EXPECT_TRUE(session->IsInCompatScaleStatus());
    session->SetScale(2.0f, 0.5f, 0.5f, 0.5f);
    EXPECT_TRUE(session->IsInCompatScaleStatus());
}

/**
 * @tc.name: IsInCompatScaleMode
 * @tc.desc: IsInCompatScaleMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, IsInCompatScaleMode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsInCompatScaleMode";
    info.bundleName_ = "IsInCompatScaleMode";
    info.screenId_ = 0;
    sptr<SceneSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    EXPECT_FALSE(session->IsInCompatScaleMode());

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_TRUE(session->IsInCompatScaleMode());

    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    EXPECT_TRUE(session->IsInCompatScaleMode());
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    EXPECT_TRUE(session->IsInCompatScaleMode());
}

/**
 * @tc.name: IsInCompatScaleMode_forSubWindow
 * @tc.desc: IsInCompatScaleMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, IsInCompatScaleMode_forSubWindow, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsInCompatScaleMode_forSubWindow";
    info.bundleName_ = "IsInCompatScaleMode_forSubWindow";
    info.screenId_ = 0;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> mainSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_FALSE(subSession->IsInCompatScaleMode());
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_FALSE(subSession->IsInCompatScaleMode());

    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    subSession->specificCallback_ = callBack;
    auto task = [&mainSession](int32_t persistentId) { return mainSession; };
    callBack->onGetSceneSessionByIdCallback_ = task;
    subSession->SetCallingPid(1);
    mainSession->SetCallingPid(2);
    EXPECT_FALSE(subSession->IsInCompatScaleMode());

    mainSession->SetCallingPid(1);
    EXPECT_FALSE(subSession->IsInCompatScaleMode());

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_TRUE(subSession->IsInCompatScaleMode());

    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    EXPECT_TRUE(subSession->IsInCompatScaleMode());
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    EXPECT_TRUE(subSession->IsInCompatScaleMode());
}

/**
 * @tc.name: GetSystemAvoidArea
 * @tc.desc: GetSystemAvoidArea function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, GetSystemAvoidArea, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea";
    info.bundleName_ = "GetSystemAvoidArea";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    ASSERT_NE(session->GetSessionProperty(), nullptr);
    session->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(WindowMode::WINDOW_MODE_FLOATING, session->GetSessionProperty()->GetWindowMode());
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);

    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    session->GetSessionProperty()->SetDisplayId(2025);
    session->SetIsMidScene(false);
    EXPECT_EQ(session->GetIsMidScene(), false);

    WSRect rect;
    AvoidArea avoidArea;
    session->GetSystemAvoidArea(rect, avoidArea);
    int32_t statusBarHeight = session->GetStatusBarHeight();
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_NE(display, nullptr);
    float vpr = display->GetVirtualPixelRatio();
    int32_t maxFloatTitleBarHeight = 40;
    int height = std::min(static_cast<int32_t>(vpr * maxFloatTitleBarHeight), statusBarHeight);
    EXPECT_EQ(height, avoidArea.topRect_.height_);

    auto task = [](DisplayId displayId, WSRect& barArea) {
        barArea.height_ = 100;
    };
    session->RegisterGetStatusBarAvoidHeightFunc(std::move(task));
    session->GetSystemAvoidArea(rect, avoidArea);
    EXPECT_EQ(0, avoidArea.topRect_.height_);
}

/**
 * @tc.name: NotifyWindowAttachStateListenerRegistered
 * @tc.desc: NotifyWindowAttachStateListenerRegistered about session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyWindowAttachStateListenerRegistered_session, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->NotifyWindowAttachStateListenerRegistered(true);
    EXPECT_EQ(sceneSession->needNotifyAttachState_, true);
    sceneSession->NotifyWindowAttachStateListenerRegistered(false);
    EXPECT_EQ(sceneSession->needNotifyAttachState_, false);
}

/**
 * @tc.name: UpdateFollowScreenChange
 * @tc.desc: UpdateFollowScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, UpdateFollowScreenChange, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    bool isFollowScreenChange = true;
    sceneSession->specificCallback_ = nullptr;
    WSError ret = sceneSession->UpdateFollowScreenChange(isFollowScreenChange);
    EXPECT_EQ(WSError::WS_OK, ret);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, callback);
    sceneSession->specificCallback_ = callback;
    ret = sceneSession->UpdateFollowScreenChange(isFollowScreenChange);
    EXPECT_EQ(WSError::WS_OK, ret);

    auto task = [] (bool isFollowScreenChange) {};
    callback->onUpdateFollowScreenChange_ = task;
    ret = sceneSession->UpdateFollowScreenChange(isFollowScreenChange);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RegisterFollowScreenChangeCallback
 * @tc.desc: RegisterFollowScreenChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RegisterFollowScreenChangeCallback, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->specificCallback_ = nullptr;
    auto task = [] (bool isFollowScreenChange) {};
    sceneSession->RegisterFollowScreenChangeCallback(std::move(task));
    EXPECT_EQ(nullptr, sceneSession->specificCallback_);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, callback);
    sceneSession->specificCallback_ = callback;
    EXPECT_EQ(nullptr, callback->onUpdateFollowScreenChange_);
    sceneSession->RegisterFollowScreenChangeCallback(std::move(task));
    EXPECT_NE(nullptr, callback->onUpdateFollowScreenChange_);
}

/**
 * @tc.name: GetFollowScreenChange
 * @tc.desc: GetFollowScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, GetFollowScreenChange01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool isFollowScreenChange = true;
    sceneSession->SetFollowScreenChange(isFollowScreenChange);
    bool res = sceneSession->GetFollowScreenChange();
    EXPECT_EQ(res, isFollowScreenChange);

    isFollowScreenChange = false;
    sceneSession->SetFollowScreenChange(isFollowScreenChange);
    res = sceneSession->GetFollowScreenChange();
    EXPECT_EQ(res, isFollowScreenChange);

    isFollowScreenChange = true;
    sceneSession->SetFollowScreenChange(isFollowScreenChange);
    res = sceneSession->GetFollowScreenChange();
    EXPECT_EQ(res, isFollowScreenChange);
}

/**
 * @tc.name: HandleActionUpdateFollowScreenChange
 * @tc.desc: test HandleActionUpdateFollowScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, HandleActionUpdateFollowScreenChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateFollowScreenChange";
    info.bundleName_ = "HandleActionUpdateFollowScreenChange";
    info.isSystem_ = true;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetFollowScreenChange(true);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE;
    auto res = session->HandleActionUpdateFollowScreenChange(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: test for NotifyKeyboardAnimationWillBegin when sessionStage_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardAnimationWillBeginInvalidSessionStage, Function | SmallTest | Level1)
{
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->sessionStage_ = nullptr;
    bool isShowAnimation = true;
    WSRect beginRect = { 0, 2720, 1260, 1020 };
    WSRect endRect = { 0, 1700, 1260, 1020 };
    bool withAnimation = false;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();
    sceneSession->NotifyKeyboardAnimationWillBegin(isShowAnimation, beginRect, endRect, withAnimation, rsTransaction);
    if (HiLogIsLoggable(HILOG_DOMAIN_WINDOW, g_domainContents[static_cast<uint32_t>(WmsLogTag::DEFAULT)], LOG_DEBUG)) {
        EXPECT_TRUE(g_errlog.find("sessionStage_ is null") != std::string::npos);
    }
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: NotifyKeyboardAnimationWillBegin when willShow notification not registered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardAnimationWillBeginNotRegisteredWillShow, Function | SmallTest | Level1)
{
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    bool isShowAnimation = true;
    WSRect beginRect = { 0, 2720, 1260, 1020 };
    WSRect endRect = { 0, 1700, 1260, 1020 };
    bool withAnimation = false;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    sceneSession->NotifyKeyboardWillShowRegistered(false);
    sceneSession->NotifyKeyboardAnimationWillBegin(isShowAnimation, beginRect, endRect, withAnimation, rsTransaction);
    EXPECT_TRUE(g_errlog.find("keyboard will show listener is not registered") != std::string::npos);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: NotifyKeyboardAnimationWillBegin when willHide notification not registered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardAnimationWillBeginNotRegisteredWillHide, Function | SmallTest | Level1)
{
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    
    WSRect beginRect = { 0, 2720, 1260, 1020 };
    WSRect endRect = { 0, 1700, 1260, 1020 };
    bool withAnimation = false;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    bool isShowAnimation = false;
    sceneSession->NotifyKeyboardWillHideRegistered(false);
    sceneSession->NotifyKeyboardAnimationWillBegin(isShowAnimation, beginRect, endRect, withAnimation, rsTransaction);
    EXPECT_TRUE(g_errlog.find("keyboard will hide listener is not registered") != std::string::npos);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: NotifyKeyboardAnimationWillBegin when willShow notification registered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardAnimationWillBeginRegisteredWillShow, Function | SmallTest | Level1)
{
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();

    WSRect beginRect = { 0, 2720, 1260, 1020 };
    WSRect endRect = { 0, 1700, 1260, 1020 };
    bool withAnimation = false;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    bool isShowAnimation = true;
    sceneSession->NotifyKeyboardWillShowRegistered(true);
    sceneSession->NotifyKeyboardAnimationWillBegin(isShowAnimation, beginRect, endRect, withAnimation, rsTransaction);
    EXPECT_TRUE(g_errlog.find("keyboard will show listener is not registered") == std::string::npos);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: NotifyKeyboardAnimationWillBegin when willHide notification registered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardAnimationWillBeginRegisteredWillHide, Function | SmallTest | Level1)
{
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    WSRect beginRect = { 0, 2720, 1260, 1020 };
    WSRect endRect = { 0, 1700, 1260, 1020 };
    bool withAnimation = false;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    bool isShowAnimation = false;
    sceneSession->NotifyKeyboardWillHideRegistered(true);
    sceneSession->NotifyKeyboardAnimationWillBegin(isShowAnimation, beginRect, endRect, withAnimation, rsTransaction);
    EXPECT_TRUE(g_errlog.find("keyboard will hide listener is not registered") == std::string::npos);
}

/**
 * @tc.name: NotifyKeyboardWillShowRegistered
 * @tc.desc: NotifyKeyboardWillShowRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardWillShowRegistered, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->NotifyKeyboardWillShowRegistered(true);
    EXPECT_EQ(true, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardWillShowRegistered_);
    sceneSession->NotifyKeyboardWillShowRegistered(false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardWillShowRegistered_);
}

/**
 * @tc.name: NotifyKeyboardWillHideRegistered
 * @tc.desc: NotifyKeyboardWillHideRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardWillHideRegistered, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->NotifyKeyboardWillHideRegistered(true);
    EXPECT_EQ(true, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardWillHideRegistered_);
    sceneSession->NotifyKeyboardWillHideRegistered(false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardWillHideRegistered_);
}

/**
 * @tc.name: NotifyKeyboardDidShowRegistered
 * @tc.desc: NotifyKeyboardDidShowRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardDidShowRegistered, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->NotifyKeyboardDidShowRegistered(true);
    EXPECT_EQ(true, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardDidShowRegistered_);
    sceneSession->NotifyKeyboardDidShowRegistered(false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardDidShowRegistered_);
}

/**
 * @tc.name: NotifyKeyboardDidHideRegistered
 * @tc.desc: NotifyKeyboardDidHideRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyKeyboardDidHideRegistered, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->NotifyKeyboardDidHideRegistered(true);
    EXPECT_EQ(true, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardDidHideRegistered_);
    sceneSession->NotifyKeyboardDidHideRegistered(false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->EditSessionInfo().isKeyboardDidHideRegistered_);
}

/**
 * @tc.name: CloseSpecificScene
 * @tc.desc: test CloseSpecificScene
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, CloseSpecificScene, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->sessionStage_ = nullptr;
    auto res = sceneSession->CloseSpecificScene();
    EXPECT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetSubWindowSourceFunc
 * @tc.desc: test SetSubWindowSourceFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSubWindowSourceFunc, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSubWindowSourceFunc(nullptr);
    EXPECT_EQ(nullptr, sceneSession->subWindowSourceFunc_);
    NotifySetSubWindowSourceFunc func = [](SubWindowSource source) {};
    sceneSession->SetSubWindowSourceFunc(std::move(func));
    EXPECT_NE(nullptr, sceneSession->subWindowSourceFunc_);
}

/**
 * @tc.name: SetSubWindowSource
 * @tc.desc: test SetSubWindowSource
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSubWindowSource, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto res = sceneSession->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_ARKUI);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_WINDOW, res);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->property_ = property;
    // test set SubWindowSource::SUB_WINDOW_SOURCE_UNKNOWN
    sceneSession->subWindowSource_ = SubWindowSource::SUB_WINDOW_SOURCE_UNKNOWN;
    sceneSession->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_ARKUI);
    EXPECT_TRUE(sceneSession->subWindowSource_ == SubWindowSource::SUB_WINDOW_SOURCE_ARKUI);
}

/**
 * @tc.name: AnimateTo01
 * @tc.desc: test AnimateTo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, AnimateTo01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WindowAnimationProperty animationProperty;
    animationProperty.targetScale = 1.5f;
    WindowAnimationOption animationOption;
    animationOption.curve = WindowAnimationCurve::INTERPOLATION_SPRING;
    animationOption.duration = 500;

    float resultScale = 0;
    WindowAnimationCurve curve = WindowAnimationCurve::LINEAR;
    auto callback = [&resultScale, &curve](const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption) {
        resultScale = animationProperty.targetScale;
        curve = animationOption.curve;
    };
    sceneSession->AnimateTo(animationProperty, animationOption);
    usleep(SLEEP_TIME);
    ASSERT_EQ(resultScale, 0);

    sceneSession->RegisterAnimateToCallback(callback);
    usleep(SLEEP_TIME);
    sceneSession->AnimateTo(animationProperty, animationOption);
    usleep(SLEEP_TIME);
    ASSERT_EQ(resultScale, animationProperty.targetScale);
    ASSERT_EQ(curve, WindowAnimationCurve::INTERPOLATION_SPRING);
}

/**
 * @tc.name: RegisterUpdateAppUseControlCallback
 * @tc.desc: RegisterUpdateAppUseControlCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RegisterUpdateAppUseControlCallback, Function | SmallTest | Level3)
{
    ControlInfo controlInfo = {
        .isNeedControl = true,
        .isControlRecentOnly = true
    };
    SessionInfo info;
    info.bundleName_ = "app";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto callback = [](ControlAppType type, bool isNeedControl, bool isControlRecentOnly) {};
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
 
    std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>> allAppUseMap;
    sceneSession->SetGetAllAppUseControlMapFunc([&allAppUseMap]() ->
        std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>>& {return allAppUseMap;});
    sceneSession->RegisterUpdateAppUseControlCallback(callback);

    std::string key = "app#0";
    allAppUseMap[key][ControlAppType::APP_LOCK] = controlInfo;
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
    ASSERT_NE(nullptr, sceneSession->onUpdateAppUseControlFunc_);
}

/**
 * @tc.name: RegisterUpdateAppUseControlCallbackHasPrivacyModeControl
 * @tc.desc: RegisterUpdateAppUseControlCallbackHasPrivacyModeControl
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RegisterUpdateAppUseControlCallbackHasPrivacyModeControl, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "app";
    info.hasPrivacyModeControl = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto callback = [](ControlAppType type, bool isNeedControl, bool isControlRecentOnly) {};
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
    usleep(SLEEP_TIME);
    EXPECT_TRUE(sceneSession->appUseControlMap_[ControlAppType::PRIVACY_WINDOW].isNeedControl);
}

/**
 * @tc.name: GetScreenWidthAndHeightFromClient
 * @tc.desc: GetScreenWidthAndHeightFromClient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, GetScreenWidthAndHeightFromClient, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "GetScreenWidthAndHeightFromClient";
    info.abilityName_ = "GetScreenWidthAndHeightFromClient";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_EQ(sceneSession->GetScreenWidthAndHeightFromClient(property, screenWidth, screenHeight), true);

    sceneSession->SetIsSystemKeyboard(true);
    EXPECT_EQ(sceneSession->IsSystemKeyboard(), true);
    EXPECT_EQ(sceneSession->GetScreenWidthAndHeightFromClient(property, screenWidth, screenHeight), true);
}

/**
 * @tc.name: SetFrameRectForPartialZoomIn
 * @tc.desc: SetFrameRectForPartialZoomIn
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetFrameRectForPartialZoomIn, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "SetFrameRectForPartialZoomIn";
    info.abilityName_ = "SetFrameRectForPartialZoomIn";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    Rect frameRect = { 10, 10, 10, 10 };  // 10 is valid frame rect param
    MockAccesstokenKit::MockIsSACalling(true);
    EXPECT_EQ(sceneSession->SetFrameRectForPartialZoomIn(frameRect), WSError::WS_OK);

    MockAccesstokenKit::MockIsSACalling(false);
    EXPECT_EQ(sceneSession->SetFrameRectForPartialZoomIn(frameRect), WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetFrameRectForPartialZoomInInner
 * @tc.desc: SetFrameRectForPartialZoomInInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetFrameRectForPartialZoomInInner, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "SetFrameRectForPartialZoomInInner";
    info.abilityName_ = "SetFrameRectForPartialZoomInInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession->mainHandler_, nullptr);
    EXPECT_EQ(sceneSession->GetSurfaceNode(), nullptr);

    Rect frameRect = { 10, 10, 10, 10 };  // 10 is valid frame rect param
    WSError ret = sceneSession->SetFrameRectForPartialZoomInInner(frameRect);
    EXPECT_EQ(ret, WSError::WS_OK);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    EXPECT_NE(surfaceNode, nullptr);
    sceneSession->surfaceNode_ = surfaceNode;
    ret = sceneSession->SetFrameRectForPartialZoomInInner(frameRect);
    EXPECT_EQ(ret, WSError::WS_OK);

    sceneSession->mainHandler_ = nullptr;
    ret = sceneSession->SetFrameRectForPartialZoomInInner(frameRect);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SendPointerEventForHover
 * @tc.desc: SendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SendPointerEventForHover, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "SendPointerEventForHover";
    info.abilityName_ = "SendPointerEventForHover";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WSError ret = sceneSession->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ret = sceneSession->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TestUpdateGlobalDisplayRectFromClient
 * @tc.desc: Verify UpdateGlobalDisplayRectFromClient updates rect asynchronously when necessary.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestUpdateGlobalDisplayRectFromClient, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Case 1: Same rect, should early return and skip update
    {
        WSRect rect = session->GetGlobalDisplayRect();
        auto result = session->UpdateGlobalDisplayRectFromClient(rect, SizeChangeReason::MOVE);
        EXPECT_EQ(result, WSError::WS_OK);
    }

    // Case 2: Different rect, update should be posted and processed
    {
        WSRect rect = session->GetGlobalDisplayRect();
        WSRect newRect = { rect.posX_ + 10, rect.posY_ + 20, rect.width_, rect.height_ };
        auto result = session->UpdateGlobalDisplayRectFromClient(newRect, SizeChangeReason::MOVE);
        EXPECT_EQ(result, WSError::WS_OK);
    }
}

/**
 * @tc.name: PatchAINavigationBarArea
 * @tc.desc: PatchAINavigationBarArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, PatchAINavigationBarArea, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    AvoidArea avoidArea;
    AvoidArea avoidAreaEmpty;
    Rect rect = { 600, 2710, 500, 10 };
    session->PatchAINavigationBarArea(avoidArea);
    EXPECT_EQ(avoidArea, avoidAreaEmpty);
    avoidArea.topRect_ = { 600, 2710, 500, 10 };
    session->PatchAINavigationBarArea(avoidArea);
    EXPECT_EQ(avoidArea.bottomRect_, rect);
    avoidArea.topRect_ = { 0, 0, 0, 0 };
    avoidArea.leftRect_ = { 600, 2710, 500, 10 };
    session->PatchAINavigationBarArea(avoidArea);
    EXPECT_EQ(avoidArea.bottomRect_, rect);
    avoidArea.leftRect_ = { 0, 0, 0, 0 };
    avoidArea.rightRect_ = { 600, 2710, 500, 10 };
    session->PatchAINavigationBarArea(avoidArea);
    EXPECT_EQ(avoidArea.bottomRect_, rect);
}

/**
 * @tc.name: SetWindowTransitionAnimation
 * @tc.desc: SetWindowTransitionAnimation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetWindowTransitionAnimation, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    WindowTransitionType transitionType = WindowTransitionType::DESTROY;
    TransitionAnimation animation;
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    session->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    auto ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);

    session->GetSessionProperty()->SetIsPcAppInPad(true);
    ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_OK);
    session->GetSessionProperty()->SetIsPcAppInPad(false);
    ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);

    session->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_OK);

    session->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    session->systemConfig_.freeMultiWindowEnable_ = false;
    session->systemConfig_.freeMultiWindowSupport_ = false;
    ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);

    session->systemConfig_.freeMultiWindowEnable_ = true;
    session->systemConfig_.freeMultiWindowSupport_ = true;
    ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_OK);

    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = session->SetWindowTransitionAnimation(transitionType, animation);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: SetSceneAnimationConfig01
 * @tc.desc: SetSceneAnimationConfig01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSceneAnimationConfig01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSceneAnimationConfig01";
    info.bundleName_ = "SetSceneAnimationConfig01";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
 
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    SceneAnimationConfig animationConfig;
    animationConfig.animationDelay_ = 0;
    animationConfig.animationDuration_ = 300;
    animationConfig.animationCurve_ = WindowAnimationCurve::LINEAR;
    animationConfig.animationParam_ = {0.0f, 0.0f, 0.0f, 0.0f};
    auto result = sceneSession->SetSceneAnimationConfig(animationConfig);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetSupportEnterWaterfallMode
 * @tc.desc: SetSupportEnterWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSupportEnterWaterfallMode, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    session->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->SetSupportEnterWaterfallMode(true);

    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    session->SetSupportEnterWaterfallMode(true);
    EXPECT_TRUE(session->sessionStage_ != nullptr);
}

/**
 * @tc.name: SetSecurityLayerWhenEnterForeground
 * @tc.desc: SetSecurityLayerWhenEnterForeground function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSecurityLayerWhenEnterForeground, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSecurityLayerWhenEnterForeground";
    info.bundleName_ = "SetSecurityLayerWhenEnterForeground";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(nullptr, session->GetLeashWinShadowSurfaceNode());
    session->SetSecurityLayerWhenEnterForeground();

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    EXPECT_NE(nullptr, surfaceNode);
    session->SetLeashWinSurfaceNode(surfaceNode);
    EXPECT_NE(nullptr, session->GetLeashWinShadowSurfaceNode());
    session->SetSecurityLayerWhenEnterForeground();
}

/**
 * @tc.name: TestSetContentAspectRatio
 * @tc.desc: Verify SetContentAspectRatio covers all branches
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestSetContentAspectRatio, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Case 1: sessionProperty is null
    session->property_ = nullptr;
    auto result = session->SetContentAspectRatio(1.5f, true, true);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    // Case 2: aspect ratio is invalid
    session->property_ = sptr<WindowSessionProperty>::MakeSptr();
    WindowLimits limits(400, 200, 200, 100, FLT_MAX, 0.0f);
    session->property_->SetWindowLimits(limits);
    result = session->SetContentAspectRatio(0.5f, true, true);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);

    // Case 3: aspect ratio is valid, isPersistent and needUpdateRect are false
    result = session->SetContentAspectRatio(2.0f, false, false);
    EXPECT_EQ(result, WSError::WS_OK);

    // Case 4: aspect ratio is valid, isPersistent and needUpdateRect are true
    result = session->SetContentAspectRatio(2.0f, true, true);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TestGetWindowDecoration
 * @tc.desc: Verify GetWindowDecoration covers all branches
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestGetWindowDecoration, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto prop = session->GetSessionProperty();

    WindowDecoration emptyDecor{0, 0, 0, 0};

    // Case 1: Not visible
    session->SetDecorVisible(false);
    auto decor = session->GetWindowDecoration();
    EXPECT_EQ(decor, emptyDecor);

    // Case 2: Not enabled
    session->SetDecorVisible(true);
    session->systemConfig_.isSystemDecorEnable_ = false;
    decor = session->GetWindowDecoration();
    EXPECT_EQ(decor, emptyDecor);

    prop->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    prop->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->systemConfig_.decorWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    session->systemConfig_.isSystemDecorEnable_ = true;

    // Case 3: display is null
    prop->SetDisplayId(DISPLAY_ID_INVALID);
    decor = session->GetWindowDecoration();
    EXPECT_EQ(decor, emptyDecor);
}

/**
 * @tc.name: TestRunAfterNVsyncsOnce
 * @tc.desc: Verify RunAfterNVsyncs executes callback after 1 vsync
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRunAfterNVsyncsOnce, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);

    bool taskExecuted = false;

    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .Times(1)
        .WillOnce(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));

    session->RunAfterNVsyncs(1, [&](int64_t, int64_t) {
        taskExecuted = true;
    });

    EXPECT_TRUE(taskExecuted);
}

/**
 * @tc.name: TestRunAfterNVsyncsThreeTimes
 * @tc.desc: Verify RunAfterNVsyncs executes callback after 3 vsyncs
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRunAfterNVsyncsThreeTimes, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);

    bool taskExecuted = false;
    int vsyncTimes = 0;

    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .Times(3)
        .WillRepeatedly(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            if (++vsyncTimes <= 3) {
                cb->onCallback(0, 0);
            }
        }));

    session->RunAfterNVsyncs(3, [&](int64_t, int64_t) {
        taskExecuted = true;
    });

    EXPECT_EQ(vsyncTimes, 3);
    EXPECT_TRUE(taskExecuted);
}

/**
 * @tc.name: TestRunAfterNVsyncsWithNullVsyncStation
 * @tc.desc: Verify RunAfterNVsyncs does nothing when VsyncStation is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRunAfterNVsyncsWithNullVsyncStation, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool taskExecuted = false;

    session->SetVsyncStation(nullptr);
    session->RunAfterNVsyncs(1, [&](int64_t, int64_t) {
        taskExecuted = true;
    });

    EXPECT_FALSE(taskExecuted);
}

/**
 * @tc.name: TestRestoreGravityWhenDragEndNormal
 * @tc.desc: Verify RestoreGravityWhenDragEnd triggers RestoreToPreDragGravity after delayed vsyncs
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRestoreGravityWhenDragEndNormal, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    // Force PostTask to execute immediately
    session->handler_ = nullptr;

    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    moveDragController->preDragGravity_ = Gravity::BOTTOM_RIGHT;
    session->moveDragController_ = moveDragController;

    struct RSSurfaceNodeConfig config;
    auto surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));
    session->SetVsyncStation(mockVsyncStation);

    session->RestoreGravityWhenDragEnd();

    Gravity gravity = surfaceNode->GetStagingProperties().GetFrameGravity();
    EXPECT_EQ(gravity, Gravity::BOTTOM_RIGHT);
    EXPECT_EQ(moveDragController->preDragGravity_, std::nullopt);
}

/**
 * @tc.name: DisableUIFirstIfNeed
 * @tc.desc: DisableUIFirstIfNeed function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, DisableUIFirstIfNeed, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisableUIFirstIfNeed";
    info.bundleName_ = "DisableUIFirstIfNeed";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(false, session->isUIFirstEnabled_);
    EXPECT_EQ(nullptr, session->GetLeashWinShadowSurfaceNode());
    session->DisableUIFirstIfNeed();
    EXPECT_EQ(false, session->isUIFirstEnabled_);

    session->isUIFirstEnabled_ = true;
    EXPECT_EQ(true, session->isUIFirstEnabled_);
    EXPECT_EQ(nullptr, session->GetLeashWinShadowSurfaceNode());
    session->DisableUIFirstIfNeed();
    EXPECT_EQ(true, session->isUIFirstEnabled_);

    session->isUIFirstEnabled_ = true;
    EXPECT_EQ(true, session->isUIFirstEnabled_);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    EXPECT_NE(nullptr, surfaceNode);
    session->SetLeashWinSurfaceNode(surfaceNode);
    EXPECT_NE(nullptr, session->GetLeashWinShadowSurfaceNode());
    session->DisableUIFirstIfNeed();
    EXPECT_EQ(false, session->isUIFirstEnabled_);
}

/**
 * @tc.name: RegisterRotationLockChangeCallback
 * @tc.desc: RegisterRotationLockChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RegisterRotationLockChangeCallback, TestSize.Level0)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->specificCallback_ = nullptr;
    auto task = [] (bool locked) {};
    sceneSession->RegisterRotationLockChangeCallback(std::move(task));
    EXPECT_EQ(nullptr, sceneSession->specificCallback_);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sceneSession->specificCallback_ = callback;
    sceneSession->RegisterRotationLockChangeCallback(nullptr);
    EXPECT_EQ(nullptr, callback->onRotationLockChange_);
    sceneSession->RegisterRotationLockChangeCallback(std::move(task));
    EXPECT_NE(nullptr, callback->onRotationLockChange_);
}

/**
 * @tc.name: HandleActionUpdateRotationLockChange
 * @tc.desc: HandleActionUpdateRotationLockChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, HandleActionUpdateRotationLockChange, TestSize.Level0)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetRotationLocked(false);
    sceneSession->specificCallback_ = nullptr;
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE;
    WMError ret = sceneSession->HandleActionUpdateRotationLockChange(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sceneSession->specificCallback_ = callback;
    ret = sceneSession->HandleActionUpdateRotationLockChange(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);

    auto task = [] (bool locked) {};
    callback->onRotationLockChange_ = task;
    ret = sceneSession->HandleActionUpdateRotationLockChange(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: NotifyPageRotationIsIgnored
 * @tc.desc: NotifyPageRotationIsIgnored function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyPageRotationIsIgnored, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyPageRotationIsIgnored";
    info.bundleName_ = "NotifyPageRotationIsIgnored";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    auto ret = session->NotifyPageRotationIsIgnored();
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertOrientationAndRotation
 * @tc.desc: ConvertOrientationAndRotation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertOrientationAndRotation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertOrientationAndRotation";
    info.bundleName_ = "ConvertOrientationAndRotation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    session->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    RotationInfoType from = RotationInfoType::DISPLAY_ORIENTATION;
    RotationInfoType to = RotationInfoType::DISPLAY_ORIENTATION;
    int32_t value = -1;
    int32_t convertedValue = 0;
    auto ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    value = 4;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    value = 2;
    from = RotationInfoType::DISPLAY_ROTATION;
    to = RotationInfoType::DISPLAY_ROTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ROTATION;
    to = RotationInfoType::WINDOW_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ROTATION;
    to = RotationInfoType::DISPLAY_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ORIENTATION;
    to = RotationInfoType::WINDOW_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ORIENTATION;
    to = RotationInfoType::DISPLAY_ROTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::WINDOW_ORIENTATION;
    to = RotationInfoType::DISPLAY_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::WINDOW_ORIENTATION;
    to = RotationInfoType::DISPLAY_ROTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertDisplayOrientationToWindowOrientation
 * @tc.desc: ConvertDisplayOrientationToWindowOrientation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertDisplayOrientationToWindowOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayOrientationToWindowOrientation";
    info.bundleName_ = "ConvertDisplayOrientationToWindowOrientation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    int32_t value = 4;
    int32_t convertedValue = 0;

    auto ret = session->ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    value = 5;
    ret = session->ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    value = 2;
    ret = session->ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertWindowOrientationToDisplayOrientation
 * @tc.desc: ConvertWindowOrientationToDisplayOrientation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertWindowOrientationToDisplayOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertWindowOrientationToDisplayOrientation";
    info.bundleName_ = "ConvertWindowOrientationToDisplayOrientation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    int32_t value = 4;
    int32_t convertedValue = 0;
    value = 5;
    auto ret = session->ConvertWindowOrientationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    value = 2;
    ret = session->ConvertWindowOrientationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertDisplayRotationToDisplayOrientation
 * @tc.desc: ConvertDisplayRotationToDisplayOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertDisplayRotationToDisplayOrientation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayRotationToDisplayOrientation";
    info.bundleName_ = "ConvertDisplayRotationToDisplayOrientation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertDisplayRotationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);
    
    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertDisplayRotationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: ConvertDisplayOrientationToDisplayRotation
 * @tc.desc: ConvertDisplayOrientationToDisplayRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertDisplayOrientationToDisplayRotation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayOrientationToDisplayRotation";
    info.bundleName_ = "ConvertDisplayOrientationToDisplayRotation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertDisplayOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);
    
    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertDisplayOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: ConvertDisplayRotationToWindowOrientation
 * @tc.desc: ConvertDisplayRotationToWindowOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertDisplayRotationToWindowOrientation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayRotationToWindowOrientation";
    info.bundleName_ = "ConvertDisplayRotationToWindowOrientation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertDisplayRotationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);
    
    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertDisplayRotationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: ConvertWindowOrientationToDisplayRotation
 * @tc.desc: ConvertWindowOrientationToDisplayRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, ConvertWindowOrientationToDisplayRotation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertWindowOrientationToDisplayRotation";
    info.bundleName_ = "ConvertWindowOrientationToDisplayRotation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertWindowOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);
    
    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertWindowOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/*
 * @tc.name: RegisterSnapshotSkipChangeCallback
 * @tc.desc: RegisterSnapshotSkipChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, RegisterSnapshotSkipChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterSnapshotSkipChangeCallback";
    info.bundleName_ = "RegisterSnapshotSkipChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->onSnapshotSkipChangeFunc_ = nullptr;
    sceneSession->RegisterSnapshotSkipChangeCallback(nullptr);
    EXPECT_EQ(sceneSession->onSnapshotSkipChangeFunc_, nullptr);
    NotifySnapshotSkipChangeFunc func = [](bool isSkip) {};
    sceneSession->RegisterSnapshotSkipChangeCallback(std::move(func));
    EXPECT_NE(sceneSession->onSnapshotSkipChangeFunc_, nullptr);
}
 
/**
 * @tc.name: HandleActionUpdateSnapshotSkip
 * @tc.desc: HandleActionUpdateSnapshotSkip
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, HandleActionUpdateSnapshotSkip, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->shadowSurfaceNode_ = surfaceNode;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetSnapshotSkip(false);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP;
    WMError ret = sceneSession->HandleActionUpdateSnapshotSkip(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
 
    NotifySnapshotSkipChangeFunc func = [](bool isSkip) {};
    sceneSession->RegisterSnapshotSkipChangeCallback(std::move(func));
    ret = sceneSession->HandleActionUpdateSnapshotSkip(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: NotifySnapshotUpdate
 * @tc.desc: NotifySnapshotUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifySnapshotUpdate, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WMError ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->collaboratorType_ = static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
    ret = sceneSession->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: NotifyRemovePrelaunchStartingWindow
 * @tc.desc: NotifyRemovePrelaunchStartingWindow test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, NotifyRemovePrelaunchStartingWindow, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WMError ret = sceneSession->NotifyRemovePrelaunchStartingWindow();
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetMoveRectForWindowDrag_Test
 * @tc.desc: GetMoveRectForWindowDrag_Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, GetMoveRectForWindowDrag, TestSize.Level0)
{
    SessionInfo info;
    // Case 1: sessionProperty is null
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_ = nullptr;
    WSRect rect1 = session->GetMoveRectForWindowDrag();
    EXPECT_EQ(rect1, session->GetGlobalOrWinRect());

    // Case 2: Window type is WINDOW_TYPE_INPUT_METHOD_FLOAT and keyboardPanelSession_ is not null
    session->property_ = sptr<WindowSessionProperty>::MakeSptr();
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    session->keyboardPanelSession_ = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect2 = session->GetMoveRectForWindowDrag();
    EXPECT_EQ(rect2, session->keyboardPanelSession_->GetSessionRect());

    // Case 3: Window type is WINDOW_TYPE_INPUT_METHOD_FLOAT and keyboardPanelSession_ is null
    session->keyboardPanelSession_ = nullptr;
    WSRect rect3 = session->GetMoveRectForWindowDrag();
    EXPECT_EQ(rect3, session->GetGlobalOrWinRect());

    // Case 4: Window type is not WINDOW_TYPE_INPUT_METHOD_FLOAT
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    WSRect rect4 = session->GetMoveRectForWindowDrag();
    EXPECT_EQ(rect4, session->GetGlobalOrWinRect());
}

/**
 * @tc.name: TestIsCrossDisplayDragSupported
 * @tc.desc: Verify IsCrossDisplayDragSupported with different window types.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestIsCrossDisplayDragSupported, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Case 1: ANCO windows are not supported
    session->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));
    EXPECT_FALSE(session->IsCrossDisplayDragSupported());
    session->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::DEFAULT_TYPE));

    // Case 2: Normal system windows are not supported
    session->GetSessionProperty()->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    EXPECT_FALSE(session->IsCrossDisplayDragSupported());

    // Case 3: WINDOW_TYPE_FLOAT windows are supported
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    EXPECT_TRUE(session->IsCrossDisplayDragSupported());

    // Case 4: WINDOW_TYPE_SCREENSHOT windows are supported
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_SCREENSHOT);
    EXPECT_TRUE(session->IsCrossDisplayDragSupported());

    // Case 5: Input Windows are supported
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_TRUE(session->IsCrossDisplayDragSupported());
}

/**
 * @tc.name: TestRequestMoveResampleOnNextVsyncNotStartMove
 * @tc.desc: Skip resample when drag not started
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRequestMoveResampleOnNextVsyncNotStartMove, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));

    // Simulate move not started
    session->moveDragController_->isStartMove_ = false;

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);

    // Force PostTask to execute immediately
    session->handler_ = nullptr;

    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));

    EXPECT_CALL(*session, SetSurfaceBounds(_, _, _)).Times(0);

    session->RequestMoveResampleOnNextVsync();
}

/**
 * @tc.name: TestRequestMoveResampleOnNextVsyncResampleInactive
 * @tc.desc: Skip resample when move resample is inactive
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRequestMoveResampleOnNextVsyncResampleInactive, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));

    // Simulate move started but resample inactive
    session->moveDragController_->isStartMove_ = true;
    session->moveDragController_->moveDragProperty_.isMoveResampleActive_ = false;

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);
    session->handler_ = nullptr;

    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));

    EXPECT_CALL(*session, SetSurfaceBounds(_, _, _)).Times(0);

    session->RequestMoveResampleOnNextVsync();
}

/**
 * @tc.name: TestRequestMoveResampleOnNextVsyncResampleActive
 * @tc.desc: Resample updates target rect and schedules next vsync
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRequestMoveResampleOnNextVsyncResampleActive, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->moveDragController_->isStartMove_ = true;
    session->moveDragController_->moveDragProperty_.isMoveResampleActive_ = true;
    session->moveDragController_->moveDragProperty_.isResampleFpsRangeChecked_ = true;

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);
    session->handler_ = nullptr;

    int vsyncCount = 0;
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            if (++vsyncCount <= 2) {
                cb->onCallback(0, 0);
            }
        }));

    EXPECT_CALL(*session, SetSurfaceBounds(_, true, true)).Times(AtLeast(1));

    session->RequestMoveResampleOnNextVsync();

    EXPECT_GE(vsyncCount, 2);
}

/**
 * @tc.name: AddSidebarBlur1
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, AddSidebarBlur1, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    session->AddSidebarBlur();
    EXPECT_TRUE(g_errlog.find("sessionStage is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddSidebarBlur2
 * @tc.desc: AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, AddSidebarBlur2, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = sessionStageMocker;
    EXPECT_CALL(*sessionStageMocker, AddSidebarBlur()).Times(1);
    session->AddSidebarBlur();
}

/**
 * @tc.name: SetSidebarBlur1
 * @tc.desc: SetSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSidebarBlur1, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    session->SetSidebarBlur(false, false);
    EXPECT_TRUE(g_errlog.find("sessionStage is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlur2
 * @tc.desc: SetSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSidebarBlur2, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = sessionStageMocker;

    EXPECT_CALL(*sessionStageMocker, SetSidebarBlurStyleWithType(SidebarBlurType::NONE)).Times(2);
    session->SetSidebarBlur(false, false);
    session->SetSidebarBlur(false, true);
    EXPECT_CALL(*sessionStageMocker, SetSidebarBlurStyleWithType(SidebarBlurType::INITIAL)).Times(1);
    session->SetSidebarBlur(true, false);
    EXPECT_CALL(*sessionStageMocker, SetSidebarBlurStyleWithType(SidebarBlurType::DEFAULT_FLOAT)).Times(1);
    session->SetSidebarBlur(true, true);
}

/**
 * @tc.name: SetSidebarBlurMaximize1
 * @tc.desc: SetSidebarBlurMaximize
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSidebarBlurMaximize1, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    g_errlog.clear();
    LOG_SetCallback(ScreenSessionLogCallback);
    session->SetSidebarBlurMaximize(false);
    EXPECT_TRUE(g_errlog.find("sessionStage is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSidebarBlurMaximize2
 * @tc.desc: SetSidebarBlurMaximize
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, SetSidebarBlurMaximize2, Function | SmallTest | Level1)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = sessionStageMocker;
    EXPECT_CALL(*sessionStageMocker, SetSidebarBlurStyleWithType(SidebarBlurType::DEFAULT_FLOAT)).Times(1);
    session->SetSidebarBlurMaximize(false);
    EXPECT_CALL(*sessionStageMocker, SetSidebarBlurStyleWithType(SidebarBlurType::DEFAULT_MAXIMIZE)).Times(1);
    session->SetSidebarBlurMaximize(true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS