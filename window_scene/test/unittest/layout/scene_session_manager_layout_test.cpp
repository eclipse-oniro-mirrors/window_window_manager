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

#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include <gtest/gtest.h>

#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/main_session.h"
#include "session/host/include/scene_session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string EMPTY_DEVICE_ID = "";
constexpr float SINGLE_HAND_SCALE = 0.75f;
constexpr float SINGLE_HAND_DEFAULT_SCALE = 1.0f;
} // namespace
class SceneSessionManagerLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerLayoutTest::ssm_ = nullptr;

void SceneSessionManagerLayoutTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerLayoutTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerLayoutTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerLayoutTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: GetNormalSingleHandTransform
 * @tc.desc: test function : GetNormalSingleHandTransform
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, GetNormalSingleHandTransform, TestSize.Level1)
{
    SingleHandTransform preTransform = ssm_->singleHandTransform_;
    ssm_->singleHandTransform_.posX = 100;
    EXPECT_EQ(100, ssm_->GetNormalSingleHandTransform().posX);
    ssm_->singleHandTransform_ = preTransform;
}

/**
 * @tc.name: NotifySingleHandInfoChange_TestUIType
 * @tc.desc: test function : NotifySingleHandInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, NotifySingleHandInfoChange_TestUIType, TestSize.Level1)
{
    SingleHandTransform singleHandTransform;
    ssm_->singleHandTransform_ = singleHandTransform;
    SingleHandScreenInfo singleHandScreenInfo;
    WSRect originRect, singleHandRect;
    singleHandScreenInfo.scaleRatio = SINGLE_HAND_SCALE;
    singleHandScreenInfo.mode = SingleHandMode::LEFT;
    originRect = { 0, 0, 400, 600 };
    singleHandRect = { 0, 100, 200, 300 };
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_NE(singleHandScreenInfo.scaleRatio, ssm_->singleHandTransform_.scaleX);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(singleHandScreenInfo.scaleRatio, ssm_->singleHandTransform_.scaleX);
    ssm_->singleHandTransform_ = singleHandTransform;
}

/**
 * @tc.name: NotifySingleHandInfoChange_TestWindowName
 * @tc.desc: test function : NotifySingleHandInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, NotifySingleHandInfoChange_TestWindowName, TestSize.Level1)
{
    SingleHandTransform singleHandTransform;
    ssm_->singleHandTransform_ = singleHandTransform;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "OneHandModeBackground_testWindow";
    sessionInfo.abilityName_ = "OneHandModeBackground_testWindow";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowName("OneHandModeBackground_testWindow");
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));
    SingleHandScreenInfo singleHandScreenInfo;
    WSRect originRect, singleHandRect;
    originRect = { 0, 0, 400, 600 };
    singleHandRect = { 0, 100, 200, 300 };
    singleHandScreenInfo.scaleRatio = SINGLE_HAND_SCALE;
    singleHandScreenInfo.mode = SingleHandMode::LEFT;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_NE(singleHandScreenInfo.scaleRatio, sceneSession->singleHandTransform_.scaleX);

    sceneSession->sessionInfo_.bundleName_ = "someApp_testWindow";
    sceneSession->sessionInfo_.abilityName_ = "someApp_testWindow";
    sceneSession->property_->SetWindowName("someApp_testWindow");
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(singleHandScreenInfo.scaleRatio, sceneSession->singleHandTransform_.scaleX);
}

/**
 * @tc.name: NotifySingleHandInfoChange_TestDisplayId
 * @tc.desc: test function : NotifySingleHandInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, NotifySingleHandInfoChange_TestDisplayId, TestSize.Level1)
{
    SingleHandTransform singleHandTransform;
    ssm_->singleHandTransform_ = singleHandTransform;
    SingleHandScreenInfo singleHandScreenInfo;
    WSRect originRect, singleHandRect;
    originRect = { 0, 0, 400, 600 };
    singleHandRect = { 0, 100, 200, 300 };
    singleHandScreenInfo.scaleRatio = SINGLE_HAND_SCALE;
    singleHandScreenInfo.mode = SingleHandMode::LEFT;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "NotifySingleHandInfoChange_TestDisplayId";
    sessionInfo.abilityName_ = "NotifySingleHandInfoChange_TestDisplayId";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->GetSessionProperty()->SetDisplayId(2025);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(singleHandScreenInfo.scaleRatio, sceneSession->singleHandTransform_.scaleX);

    sceneSession->GetSessionProperty()->SetDisplayId(0);
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(singleHandScreenInfo.scaleRatio, sceneSession->singleHandTransform_.scaleY);
    ssm_->singleHandTransform_ = singleHandTransform;
}

/**
 * @tc.name: NotifySingleHandInfoChange_TestMode
 * @tc.desc: test function : NotifySingleHandInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, NotifySingleHandInfoChange_TestMode, TestSize.Level1)
{
    SingleHandTransform singleHandTransform;
    ssm_->singleHandTransform_ = singleHandTransform;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));

    SingleHandScreenInfo singleHandScreenInfo;
    WSRect originRect, singleHandRect;
    originRect = { 0, 0, 400, 600 };
    singleHandRect = { 0, 100, 200, 300 };
    singleHandScreenInfo.scaleRatio = SINGLE_HAND_SCALE;
    singleHandScreenInfo.mode = SingleHandMode::LEFT;
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(100, ssm_->singleHandTransform_.posY);
    EXPECT_EQ(0, ssm_->singleHandTransform_.posX);
    ssm_->singleHandTransform_ = singleHandTransform;

    singleHandScreenInfo.mode = SingleHandMode::RIGHT;
    singleHandRect = { 50, 100, 200, 300 };
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(100, ssm_->singleHandTransform_.posY);
    EXPECT_EQ(50, ssm_->singleHandTransform_.posX);
    ssm_->singleHandTransform_ = singleHandTransform;

    singleHandScreenInfo.scaleRatio = SINGLE_HAND_DEFAULT_SCALE;
    singleHandScreenInfo.mode = SingleHandMode::MIDDLE;
    singleHandRect = { 0, 0, 200, 300 };
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(0, ssm_->singleHandTransform_.posY);
    EXPECT_EQ(0, ssm_->singleHandTransform_.posX);
    ssm_->singleHandTransform_ = singleHandTransform;
}

/**
 * @tc.name: NotifySingleHandInfoChange_TestNullSession
 * @tc.desc: test function : NotifySingleHandInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, NotifySingleHandInfoChange_TestNullSession, TestSize.Level1)
{
    SingleHandTransform singleHandTransform;
    ssm_->singleHandTransform_ = singleHandTransform;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "NotifySingleHandInfoChange_TestNullSession";
    sessionInfo.abilityName_ = "NotifySingleHandInfoChange_TestNullSession";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), nullptr });

    SingleHandScreenInfo singleHandScreenInfo;
    WSRect originRect, singleHandRect;
    originRect = { 0, 0, 400, 600 };
    singleHandRect = { 0, 100, 200, 300 };
    singleHandScreenInfo.scaleRatio = SINGLE_HAND_SCALE;
    singleHandScreenInfo.mode = SingleHandMode::LEFT;
    ssm_->NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ssm_->sceneSessionMap_[0], nullptr);
}

/**
 * @tc.name: ConvertToRelativeCoordinateExtended_TestMode
 * @tc.desc: test function : ConvertToRelativeCoordinateExtended
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, ConvertToRelativeCoordinateExtended_TestMode, TestSize.Level1)
{
    Rect rect = { 100, 3500, 400, 600};
    Rect newRect;
    DisplayId newDisplayId = 0;
    // Convert success
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::HALF_FOLDED,
        { 0, 0, 2472, 1648}, {0, 1648, 2472, 1648}, {0, 1642, 2472, 1648});
    ssm_->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(204, newRect.posY_);
    EXPECT_EQ(999, newDisplayId);

    // Convert Failed
    newDisplayId = 0;
    rect = { 100, 200, 400, 600 };
    ssm_->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(200, newRect.posY_);
    EXPECT_EQ(0, newDisplayId);
}

/**
 * @tc.name: SetHasRootSceneRequestedVsyncFunc
 * @tc.desc: SetHasRootSceneRequestedVsyncFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, SetHasRootSceneRequestedVsyncFunc, TestSize.Level1)
{
    ssm_->SetHasRootSceneRequestedVsyncFunc(nullptr);
    ASSERT_EQ(nullptr, ssm_->hasRootSceneRequestedVsyncFunc_);
    ssm_->SetHasRootSceneRequestedVsyncFunc([] {
        bool tempBool = false;
        return tempBool;
    });
    ASSERT_NE(nullptr, ssm_->hasRootSceneRequestedVsyncFunc_);
}

/**
 * @tc.name: HasRootSceneRequestedVsync
 * @tc.desc: HasRootSceneRequestedVsync
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, HasRootSceneRequestedVsync, TestSize.Level1)
{
    bool tempBool = false;
    ssm_->hasRootSceneRequestedVsyncFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ssm_->HasRootSceneRequestedVsync(tempBool));
    EXPECT_EQ(false, tempBool);

    ssm_->SetHasRootSceneRequestedVsyncFunc([] {
        bool tempInnerBool = true;
        return tempInnerBool;
    });
    EXPECT_EQ(WSError::WS_OK, ssm_->HasRootSceneRequestedVsync(tempBool));
    EXPECT_EQ(true, tempBool);
}

/**
 * @tc.name: SetRequestVsyncByRootSceneWhenModeChangeFunc
 * @tc.desc: SetRequestVsyncByRootSceneWhenModeChangeFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, SetRequestVsyncByRootSceneWhenModeChangeFunc, TestSize.Level1)
{
    ssm_->SetRequestVsyncByRootSceneWhenModeChangeFunc(nullptr);
    ASSERT_EQ(nullptr, ssm_->requestVsyncByRootSceneWhenModeChangeFunc_);
    ssm_->SetRequestVsyncByRootSceneWhenModeChangeFunc([](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
        auto tempCallback = vsyncCallback;
    });
    ASSERT_NE(nullptr, ssm_->requestVsyncByRootSceneWhenModeChangeFunc_);
}

/**
 * @tc.name: RequestVsyncByRootSceneWhenModeChange
 * @tc.desc: RequestVsyncByRootSceneWhenModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, RequestVsyncByRootSceneWhenModeChange, TestSize.Level1)
{
    std::shared_ptr<VsyncCallback> nextVsyncCallback = std::make_shared<VsyncCallback>();
    ssm_->requestVsyncByRootSceneWhenModeChangeFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ssm_->RequestVsyncByRootSceneWhenModeChange(nextVsyncCallback));
    ssm_->SetRequestVsyncByRootSceneWhenModeChangeFunc([](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
        auto tempCallback = vsyncCallback;
    });
    EXPECT_EQ(WSError::WS_OK, ssm_->RequestVsyncByRootSceneWhenModeChange(nextVsyncCallback));
}

/**
 * @tc.name: GetDisplaySizeById_TestDisplayId
 * @tc.desc: test function : GetDisplaySizeById
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, GetDisplaySizeById_TestDisplayId, TestSize.Level1)
{
    DisplayId displayId = 2025;
    int32_t displayWidth = 0;
    int32_t displayHeight = 0;
    EXPECT_EQ(false, ssm_->GetDisplaySizeById(displayId, displayWidth, displayHeight));

    displayId = 0;
    EXPECT_EQ(true, ssm_->GetDisplaySizeById(displayId, displayWidth, displayHeight));
}

/**
 * @tc.name: UpdateWindowModeByIdForUITest01
 * @tc.desc: test function : UpdateWindowModeByIdForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, UpdateWindowModeByIdForUITest01, TestSize.Level1)
{
    const int32_t windowId = 0;
    const int32_t updateMode = 1;
    EXPECT_EQ(ssm_->UpdateWindowModeByIdForUITest(windowId, updateMode), WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow
 * @tc.desc: test function : UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow, TestSize.Level1)
{
    ASSERT_TRUE(ssm_ != nullptr);
    std::string bundleName = "UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow_Test";
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = bundleName;
    sessionInfo.abilityName_ = bundleName;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId() + 1, nullptr });
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    sceneSession->GetSessionProperty()->SetHookWindowInfo(hookWindowInfo);

    // Case 1: open freeMultiWindow
    ssm_->UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow(true);
    EXPECT_EQ(sceneSession->GetSessionProperty()->GetHookWindowInfo().enableHookWindow, false);

    // Case 2: close freeMultiWindow
    ssm_->UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow(false);
    EXPECT_EQ(sceneSession->GetSessionProperty()->GetHookWindowInfo().enableHookWindow, true);
}

/**
 * @tc.name: UpdateRsCmdBlockingCount
 * @tc.desc: test function : UpdateRsCmdBlockingCount
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, UpdateRsCmdBlockingCount, TestSize.Level1)
{
    ssm_->rsCmdBlockingFlag_.count = 0;
    ssm_->rsCmdBlockingFlag_.startTime = 0;

    bool enable = true;
    ssm_->UpdateRsCmdBlockingCount(enable);
    EXPECT_EQ(ssm_->GetOrResetRsCmdBlockingCount(), 1);

    ssm_->rsCmdBlockingFlag_.startTime = 0;
    EXPECT_NE(ssm_->GetOrResetRsCmdBlockingCount(), 1);

    enable = false;
    ssm_->UpdateRsCmdBlockingCount(enable);
    EXPECT_EQ(ssm_->GetOrResetRsCmdBlockingCount(), 0);

    ssm_->rsCmdBlockingFlag_.startTime = -1;
    EXPECT_EQ(ssm_->GetOrResetRsCmdBlockingCount(), 0);
}

/**
 * @tc.name: RegisterGetRsCmdBlockingCount
 * @tc.desc: test function : RegisterGetRsCmdBlockingCount
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, RegisterGetRsCmdBlockingCount, TestSize.Level1)
{
    ssm_->RegisterGetRsCmdBlockingCountFunc(nullptr);
    SessionInfo info;
    info.abilityName_ = "RegisterGetRsCmdBlockingCount";
    info.bundleName_ = "RegisterGetRsCmdBlockingCount";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->RegisterGetRsCmdBlockingCountFunc(session);
    EXPECT_NE(nullptr, session->getRsCmdBlockingCountFunc_);
}

/**
 * @tc.name: TestRunAfterNVsyncs
 * @tc.desc: test function : TestRunAfterNVsyncs
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, TestRunAfterNVsyncs, TestSize.Level1)
{
    ssm_->vsyncStation_ = nullptr;
    bool callbackExecuted = false;
    ssm_->RunAfterNVsyncs(1, [&callbackExecuted](int64_t, int64_t) {
        callbackExecuted = true;
    });
    EXPECT_FALSE(callbackExecuted);

    auto vsyncStation = std::make_shared<VsyncStation>(1);
    ssm_->vsyncStation_ = vsyncStation;
    callbackExecuted = false;
    ssm_->RunAfterNVsyncs(0, [&callbackExecuted](int64_t, int64_t) {
        callbackExecuted = true;
    });
    ssm_->vsyncStation_->VsyncCallbackInner(0, 0);
    EXPECT_TRUE(callbackExecuted);

    callbackExecuted = false;
    ssm_->RunAfterNVsyncs(1, [&callbackExecuted](int64_t, int64_t) {
        callbackExecuted = true;
    });
    ssm_->vsyncStation_->VsyncCallbackInner(0, 0);
    EXPECT_TRUE(callbackExecuted);
}

/**
 * @tc.name: TestRegisterUpdateAppHookDisplayInfoFunc
 * @tc.desc: Verify RegisterUpdateAppHookDisplayInfoFunc behavior
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, TestRegisterUpdateAppHookDisplayInfoFunc, TestSize.Level1)
{
    // Case 1: sceneSession is nullptr
    ssm_->RegisterUpdateAppHookDisplayInfoFunc(nullptr);

    // Case 2: valid sceneSession
    SessionInfo info;
    auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->RegisterUpdateAppHookDisplayInfoFunc(session);
    EXPECT_NE(session->updateAppHookDisplayInfoFunc_, nullptr);
}

/**
 * @tc.name: GetAllWindowLayoutInfo
 * @tc.desc: test function : GetAllWindowLayoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, GetAllWindowLayoutInfo, TestSize.Level1)
{
    ASSERT_TRUE(ssm_ != nullptr);
    std::string bundleName = "GetAllWindowLayoutInfo_Test";
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = bundleName;
    sessionInfo.abilityName_ = bundleName;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    constexpr DisplayId TEST_DISPLAY_ID = 200;
    sceneSession->GetSessionProperty()->SetDisplayId(TEST_DISPLAY_ID);
    sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    sceneSession->SetSessionGlobalRect({ 0, 0, 800, 800 });
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    sceneSession->GetSessionProperty()->SetHookWindowInfo(hookWindowInfo);

    std::vector<sptr<WindowLayoutInfo>> info;
    ssm_->GetAllWindowLayoutInfo(TEST_DISPLAY_ID, info);
    ASSERT_NE(info.size(), 0);
    EXPECT_EQ(800, info[0]->rect.width_);

    hookWindowInfo.widthHookRatio = 0.5f;
    sceneSession->GetSessionProperty()->SetHookWindowInfo(hookWindowInfo);
    info.clear();
    ssm_->GetAllWindowLayoutInfo(TEST_DISPLAY_ID, info);
    ASSERT_NE(info.size(), 0);
    EXPECT_NE(800, info[0]->rect.width_);
}

/**
 * @tc.name: OnSessionRecoverStateChange_SetAnchorInfoOnProperty
 * @tc.desc: Verify anchor info is stored in session property during sub window recovery
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, OnSessionRecoverStateChange_SetAnchorInfoOnProperty, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    // Set up a sub window session
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(161);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchorEnabled_ = true;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = true;
    anchorInfo.attachOptions.isIntersectedWidthLimit = true;
    property->SetWindowAnchorInfo(anchorInfo);

    SessionInfo info;
    info.abilityName_ = "AnchorInfoRecoveryTest";
    info.bundleName_ = "AnchorInfoRecoveryTest";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_.insert({ 161, sceneSession });

    // Trigger recovery for sub window
    ssm_->OnSessionRecoverStateChange(SessionRecoverState::SESSION_FINISH_RECONNECT, property);

    // Verify anchor info is stored in session's property
    auto recoveredAnchorInfo = ssm_->GetSceneSession(161)->GetSessionProperty()->GetWindowAnchorInfo();
    EXPECT_EQ(recoveredAnchorInfo.isAnchorEnabled_, true);
    EXPECT_EQ(recoveredAnchorInfo.isAnchoredByAttach_, true);
    EXPECT_EQ(recoveredAnchorInfo.attachOptions.isIntersectedHeightLimit, true);
    EXPECT_EQ(recoveredAnchorInfo.attachOptions.isIntersectedWidthLimit, true);

    ssm_->sceneSessionMap_.erase(161);
}

/**
 * @tc.name: RecoverCachedSubSession_WithAnchorInfo
 * @tc.desc: Verify RecoverCachedSubSession propagates property anchor info to session's windowAnchorInfo_
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, RecoverCachedSubSession_WithAnchorInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    // Set up parent main session
    SessionInfo parentInfo;
    parentInfo.abilityName_ = "RecoverAnchorInfoParent";
    parentInfo.bundleName_ = "RecoverAnchorInfoParent";
    sptr<MainSession> parentSession = sptr<MainSession>::MakeSptr(parentInfo, nullptr);
    sptr<WindowSessionProperty> parentProperty = sptr<WindowSessionProperty>::MakeSptr();
    parentProperty->SetPersistentId(200);
    parentProperty->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentSession->SetSessionProperty(parentProperty);
    ssm_->sceneSessionMap_.insert({ 200, parentSession });

    // Set up sub session with anchor info in property
    SessionInfo subInfo;
    subInfo.abilityName_ = "RecoverAnchorInfoChild";
    subInfo.bundleName_ = "RecoverAnchorInfoChild";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subInfo, nullptr);
    sptr<WindowSessionProperty> subProperty = sptr<WindowSessionProperty>::MakeSptr();
    subProperty->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subProperty->SetParentPersistentId(200);

    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchorEnabled_ = true;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.isFromAttachOrDetach_ = false;
    anchorInfo.attachOptions.isIntersectedHeightLimit = true;
    anchorInfo.attachOptions.isIntersectedWidthLimit = false;
    subProperty->SetWindowAnchorInfo(anchorInfo);
    subSession->SetSessionProperty(subProperty);

    // Enable support for relative position so validation passes
    subSession->systemConfig_.supportFollowRelativePositionToParent_ = true;

    // Cache sub session for recovery
    ssm_->recoverSubSessionCacheMap_[200].emplace_back(subSession);

    // Register createSubSessionFunc so NotifyCreateSubSession establishes parent-child
    ssm_->createSubSessionFuncMap_[200] = [](const sptr<SceneSession>& session) {};

    // Recover - NotifyCreateSubSession sets up parent-child, then SetWindowAnchorInfo propagates
    ssm_->RecoverCachedSubSession(200);

    // Verify anchor info was propagated from property to session's windowAnchorInfo_
    auto sessionAnchorInfo = subSession->GetWindowAnchorInfo();
    EXPECT_EQ(sessionAnchorInfo.isAnchorEnabled_, true);
    EXPECT_EQ(sessionAnchorInfo.isAnchoredByAttach_, true);
    EXPECT_EQ(sessionAnchorInfo.attachOptions.isIntersectedHeightLimit, true);
    EXPECT_EQ(sessionAnchorInfo.attachOptions.isIntersectedWidthLimit, false);

    // Verify cache is cleared
    EXPECT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0u);

    // Clean up
    parentSession->subSession_.clear();
    subSession->parentSession_ = nullptr;
    ssm_->sceneSessionMap_.erase(200);
    ssm_->createSubSessionFuncMap_.erase(200);
}

/**
 * @tc.name: SetParentWindowInner_AnchorRebind01
 * @tc.desc: SetParentWindowInner with no anchor binding → anchor rebind not triggered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, SetParentWindowInner_AnchorRebind01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "AnchorRebind01";
    sessionInfo.abilityName_ = "AnchorRebind01";

    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> oldParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> newParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    oldParentSession->property_->SetSubWindowLevel(10);
    oldParentSession->persistentId_ = 10;
    newParentSession->property_->SetSubWindowLevel(1);
    newParentSession->persistentId_ = 1;
    subSession->persistentId_ = 100;

    // isAnchorEnabled_ defaults to false → anchor rebind should not trigger
    EXPECT_FALSE(subSession->GetWindowAnchorInfo().isAnchorEnabled_);

    ssm_->SetParentWindowInner(subSession, oldParentSession, newParentSession);

    // Verify anchor state unchanged
    EXPECT_FALSE(subSession->GetWindowAnchorInfo().isAnchorEnabled_);
}

/**
 * @tc.name: SetParentWindowInner_AnchorRebind02
 * @tc.desc: SetParentWindowInner with attach binding → reset and notify triggered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, SetParentWindowInner_AnchorRebind02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "AnchorRebind02";
    sessionInfo.abilityName_ = "AnchorRebind02";

    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> oldParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> newParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    oldParentSession->property_->SetSubWindowLevel(10);
    oldParentSession->persistentId_ = 10;
    newParentSession->property_->SetSubWindowLevel(1);
    newParentSession->persistentId_ = 1;
    subSession->persistentId_ = 100;

    // Set attach binding state
    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    subSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    subSession->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    subSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    subSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;

    ssm_->SetParentWindowInner(subSession, oldParentSession, newParentSession);

    // Verify anchor state is reset
    EXPECT_FALSE(subSession->GetWindowAnchorInfo().isAnchorEnabled_);
    EXPECT_FALSE(subSession->GetWindowAnchorInfo().isAnchoredByAttach_);
    EXPECT_TRUE(subSession->GetWindowAnchorInfo().isFromAttachOrDetach_);
}

/**
 * @tc.name: SetParentWindowInner_AnchorRebind03
 * @tc.desc: SetParentWindowInner with relativePosition binding (isAnchoredByAttach_=false) → reset triggered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLayoutTest, SetParentWindowInner_AnchorRebind03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "AnchorRebind03";
    sessionInfo.abilityName_ = "AnchorRebind03";

    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> oldParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> newParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    oldParentSession->property_->SetSubWindowLevel(10);
    oldParentSession->persistentId_ = 10;
    newParentSession->property_->SetSubWindowLevel(1);
    newParentSession->persistentId_ = 1;
    subSession->persistentId_ = 100;

    // Set relativePosition binding state
    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    subSession->windowAnchorInfo_.isAnchoredByAttach_ = false;

    ssm_->SetParentWindowInner(subSession, oldParentSession, newParentSession);

    // Verify anchor state is reset
    EXPECT_FALSE(subSession->GetWindowAnchorInfo().isAnchorEnabled_);
    EXPECT_TRUE(subSession->GetWindowAnchorInfo().isFromAttachOrDetach_);
}

} // namespace
} // namespace Rosen
} // namespace OHOS
