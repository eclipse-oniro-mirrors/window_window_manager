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
        sptr<MoveDragController>::MakeSptr(subSessionId, subSession->GetWindowType());
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

    // Case 3: aspect ratio is valid, moveDragController is null
    session->moveDragController_ = nullptr;
    result = session->SetContentAspectRatio(2.0f, false, false);
    EXPECT_EQ(result, WSError::WS_OK);

    // Case 4: aspect ratio is valid, moveDragController is not null
    session->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(session->GetPersistentId(), session->GetWindowType());
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
 * @tc.name: TestRunAfterNVsyncs
 * @tc.desc: Test RunAfterNVsyncs and RestoreGravityWhenDragEnd with various conditions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest6, TestRunAfterNVsyncs, TestSize.Level1)
{
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    bool taskExecuted = false;

    // Case 1: vsyncCount = 1
    session->requestNextVsyncFunc_ = [](const std::shared_ptr<VsyncCallback>& cb) {
        cb->onCallback(0, 0);
    };
    session->RunAfterNVsyncs(1, [&] { taskExecuted = true; });
    EXPECT_TRUE(taskExecuted);

    // Case 2: vsyncCount = 3
    taskExecuted = false;
    session->requestNextVsyncFunc_ = [](const std::shared_ptr<VsyncCallback>& cb) {
        static int times = 0;
        if (++times <= 3) cb->onCallback(0, 0);
    };
    session->RunAfterNVsyncs(3, [&] { taskExecuted = true; });
    EXPECT_TRUE(taskExecuted);

    // Case 3: requestNextVsyncFunc_ = nullptr
    taskExecuted = false;
    session->requestNextVsyncFunc_ = nullptr;
    session->RunAfterNVsyncs(1, [&] { taskExecuted = true; });
    EXPECT_FALSE(taskExecuted);

    // Case 4: RestoreGravityWhenDragEnd will not crash
    session->RestoreGravityWhenDragEnd();
    SUCCEED();
}
} // namespace
} // namespace Rosen
} // namespace OHOS