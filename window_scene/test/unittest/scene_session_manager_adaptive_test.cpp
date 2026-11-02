/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>

#include "interfaces/include/ws_common.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/scene_session_manager_lite.h"
#include "session_manager/include/session_manager_agent_controller.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "scene_board_judgement.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "window_manager_agent.h"
#include "window_manager_agent_stub.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_accesstoken_kit.h"
#include "context.h"
#include "session_permission.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t TEST_UID_A = 100;
constexpr int32_t TEST_UID_B = 200;
constexpr int32_t TEST_WINDOW_ID = 1;
constexpr int32_t TEST_WINDOW_ID_2 = 2;
constexpr int32_t TEST_WINDOW_ID_3 = 3;
constexpr float TEST_RATIO = 0.5f;

// Records the game adaptive callbacks; the other agent callbacks are out of scope here.
class GameAdaptiveAgentMocker : public WindowManagerAgentStub {
public:
    GameAdaptiveAgentMocker() = default;
    ~GameAdaptiveAgentMocker() = default;

    void OnGameAdaptiveResolutionCancelled(int32_t windowId) override { cancelledWindowId_ = windowId; }
    void OnGameAdaptiveResolutionAvailable(int32_t windowId) override { availableWindowId_ = windowId; }

    void UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) override {}
    void UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd) override {}
    void UpdateWindowModeTypeInfo(WindowModeType type) override {}
    void UpdateSystemBarRegionTints(DisplayId displayId, const SystemBarRegionTints& tints) override {}
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
        WindowUpdateType type) override {}
    void UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos) override {}
    void UpdateWindowDrawingContentInfo(
        const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos) override {}
    void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing) override {}
    void NotifyWaterMarkFlagChangedResult(bool isShowing) override {}
    void UpdateVisibleWindowNum(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo) override {}
    void NotifyGestureNavigationEnabledResult(bool enable) override {}
    void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing) override {}
    void NotifyWindowStyleChange(WindowStyleType type) override {}
    void NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo) override {}
    void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info) override {}
    void NotifyWindowSystemBarPropertyChange(WindowType type,
        const SystemBarProperty& systemBarProperty) override {}
    void UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground) override {}
    void NotifyWindowPropertyChange(uint32_t propertyDirtyFlags,
        const WindowInfoList& windowInfoList) override {}
    void NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo) override {}
    void NotifySessionSaveSnapShotComplete(int32_t persistentId) override {}

    int32_t cancelledWindowId_ = -1;
    int32_t availableWindowId_ = -1;
};
}

class SceneSessionManagerAdaptiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;
    static WindowUIType defaultUIType_;
    sptr<SceneSession> CreateTestSceneSession(int32_t persistentId, int32_t uid, WindowMode mode);
    GameAdaptiveResolutionConfig MakeConfig(int32_t windowId, bool enable, float ratio = TEST_RATIO);
};

sptr<SceneSessionManager> SceneSessionManagerAdaptiveTest::ssm_ = nullptr;
WindowUIType SceneSessionManagerAdaptiveTest::defaultUIType_ = WindowUIType::PHONE_WINDOW;

void SceneSessionManagerAdaptiveTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    defaultUIType_ = ssm_->systemConfig_.windowUIType_;
}

void SceneSessionManagerAdaptiveTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerAdaptiveTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
    ssm_->adaptiveResolutionWindows_.clear();
    ssm_->cancelledGameWindowsByUid_.clear();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
}

void SceneSessionManagerAdaptiveTest::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    ssm_->adaptiveResolutionWindows_.clear();
    ssm_->cancelledGameWindowsByUid_.clear();
    ssm_->systemConfig_.windowUIType_ = defaultUIType_;
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(200000); // 200ms
}

sptr<SceneSession> SceneSessionManagerAdaptiveTest::CreateTestSceneSession(
    int32_t persistentId, int32_t uid, WindowMode mode)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "adaptiveTestBundle";
    sessionInfo.abilityName_ = "adaptiveTestAbility";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    if (sceneSession == nullptr) {
        return nullptr;
    }
    sceneSession->persistentId_ = persistentId;
    sceneSession->callingUid_ = uid;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(mode);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_[persistentId] = sceneSession;
    return sceneSession;
}

GameAdaptiveResolutionConfig SceneSessionManagerAdaptiveTest::MakeConfig(
    int32_t windowId, bool enable, float ratio)
{
    GameAdaptiveResolutionConfig config;
    config.enable_ = enable;
    config.scaleRatio_ = ratio;
    config.windowId_ = windowId;
    return config;
}

namespace {
/**
 * @tc.name: SetForWindowVariants
 * @tc.desc: permission, missing session and success branches of the SCB entry
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, SetForWindowVariants, TestSize.Level1)
{
    auto session = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(session, nullptr);
    // non-SA caller rejected before any state change
    MockAccesstokenKit::MockIsSACalling(false);
    auto ret = ssm_->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_WINDOW_ID, true));
    EXPECT_EQ(ret, WMError::WM_ERROR_NOT_SYSTEM_APP);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    // missing session
    MockAccesstokenKit::MockIsSACalling(true);
    ret = ssm_->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_WINDOW_ID_2, true));
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    // success
    ret = ssm_->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_WINDOW_ID, true));
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID], TEST_UID_A);
}

/**
 * @tc.name: HandleConfigValidationVariants
 * @tc.desc: device, params, fullscreen, window-hook-conflict and null-property branches
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, HandleConfigValidationVariants, TestSize.Level1)
{
    auto session = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(session, nullptr);
    // pc device rejects both enable and disable
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_ERROR_INVALID_OPERATION);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, false)),
        WMError::WM_ERROR_INVALID_OPERATION);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    // phone/pad devices pass the gate
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_OK);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, false)),
        WMError::WM_OK);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;

    // windowId mismatch / ratio out of range / negative metrics
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID_2, true)),
        WMError::WM_ERROR_INVALID_PARAM);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true, 1.0f)),
        WMError::WM_ERROR_INVALID_PARAM);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true, 0.0f)),
        WMError::WM_ERROR_INVALID_PARAM);
    auto config = MakeConfig(TEST_WINDOW_ID, true);
    config.density_ = -1.0f;
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, config), WMError::WM_ERROR_INVALID_PARAM);
    config = MakeConfig(TEST_WINDOW_ID, true);
    config.xDpi_ = -1.0f;
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, config), WMError::WM_ERROR_INVALID_PARAM);
    config = MakeConfig(TEST_WINDOW_ID, true);
    config.yDpi_ = -1.0f;
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, config), WMError::WM_ERROR_INVALID_PARAM);
    // disable is always valid, even with an out-of-range ratio
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, false, 2.0f)),
        WMError::WM_OK);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());

    // enabling on a non-fullscreen window is rejected with a mode-class error
    auto floating = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FLOATING);
    ASSERT_NE(floating, nullptr);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(floating, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);

    // enabling while the window hook is on is rejected; disable is not blocked
    auto property = session->GetSessionProperty();
    ASSERT_NE(property, nullptr);
    HookWindowInfo hookInfo;
    hookInfo.enableHookWindow = true;
    property->SetHookWindowInfo(hookInfo);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_ERROR_INVALID_OPERATION);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(session, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, false)),
        WMError::WM_OK);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    property->SetHookWindowInfo(HookWindowInfo {}); // hook off again
}

/**
 * @tc.name: HandleConfigMapLifecycle
 * @tc.desc: enable/update/disable, sibling keep, and cancelled-record consumption
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, HandleConfigMapLifecycle, TestSize.Level1)
{
    auto sessionA = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionB = CreateTestSceneSession(TEST_WINDOW_ID_2, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(sessionA, nullptr);
    ASSERT_NE(sessionB, nullptr);

    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionA, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_OK);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID], TEST_UID_A);
    // enable again: update branch, map size unchanged
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionA, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_OK);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);

    // enable and disable both consume the cancelled restore record
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID, TEST_WINDOW_ID_2 };
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionA, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, false)),
        WMError::WM_OK);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].size(), 1u);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 0u);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionB, TEST_UID_A, MakeConfig(TEST_WINDOW_ID_2, true)),
        WMError::WM_OK);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());

    // disabling one of two same-uid windows keeps the other record
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionA, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, true)),
        WMError::WM_OK);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 2u);
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionA, TEST_UID_A, MakeConfig(TEST_WINDOW_ID, false)),
        WMError::WM_OK);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_2], TEST_UID_A);

    // full disable clears the map
    EXPECT_EQ(ssm_->HandleAdaptiveResolutionConfig(sessionB, TEST_UID_A, MakeConfig(TEST_WINDOW_ID_2, false)),
        WMError::WM_OK);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
}

/**
 * @tc.name: CheckOnModeChangeVariants
 * @tc.desc: null session, no config, cancel on non-fullscreen, restore check on fullscreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, CheckOnModeChangeVariants, TestSize.Level1)
{
    // null session: no crash
    ssm_->CheckAdaptiveResolutionOnModeChange(nullptr, WindowMode::WINDOW_MODE_FLOATING);

    auto session = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(session, nullptr);
    // no config + non-fullscreen: nothing happens
    ssm_->CheckAdaptiveResolutionOnModeChange(session, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());

    // config + non-fullscreen: window-level cancel, restore record kept
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID] = TEST_UID_A;
    ssm_->CheckAdaptiveResolutionOnModeChange(session, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);

    // config + fullscreen (not cancelled): restore check path without records
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID] = TEST_UID_A;
    ssm_->CheckAdaptiveResolutionOnModeChange(session, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.count(TEST_WINDOW_ID), 1u);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
}

/**
 * @tc.name: CheckOnHookWindowChangeVariants
 * @tc.desc: null session, enable+config cancels, enable w/o config noop, disable restores
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, CheckOnHookWindowChangeVariants, TestSize.Level1)
{
    ssm_->CheckAdaptiveResolutionOnHookWindowChange(nullptr, true);

    auto session = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(session, nullptr);
    // enable hook window without config: nothing happens
    ssm_->CheckAdaptiveResolutionOnHookWindowChange(session, true);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());

    // enable hook window with config: cancel
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID] = TEST_UID_A;
    ssm_->CheckAdaptiveResolutionOnHookWindowChange(session, true);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);

    // disable hook window: restore check clears the record when restorable
    ssm_->CheckAdaptiveResolutionOnHookWindowChange(session, false);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
}

/**
 * @tc.name: CancelVariants
 * @tc.desc: by-window miss, empty ids, sibling keep, null session, by-uid miss and all
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, CancelVariants, TestSize.Level1)
{
    auto sessionA = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionB = CreateTestSceneSession(TEST_WINDOW_ID_2, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionC = CreateTestSceneSession(TEST_WINDOW_ID_3, TEST_UID_B, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(sessionA, nullptr);
    ASSERT_NE(sessionB, nullptr);
    ASSERT_NE(sessionC, nullptr);

    // cancelling a window without config returns directly
    ssm_->CancelAdaptiveResolutionByWindow(TEST_WINDOW_ID);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    // empty window id list returns without touching state
    ssm_->CancelAdaptiveResolutionInternal(TEST_UID_A, {});
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    // cancelling an unknown uid returns directly
    ssm_->CancelAdaptiveResolutionByUid(TEST_UID_A);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());

    // cancelling one of two same-uid windows keeps the sibling record
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID] = TEST_UID_A;
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_2] = TEST_UID_A;
    ssm_->CancelAdaptiveResolutionInternal(TEST_UID_A, { TEST_WINDOW_ID });
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_2], TEST_UID_A);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);

    // cancelling by uid erases every window of the uid and records restore
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_3] = TEST_UID_B;
    ssm_->CancelAdaptiveResolutionByUid(TEST_UID_A);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_3], TEST_UID_B);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].size(), 2u);

    // destroyed sessions are skipped without crashing
    ssm_->sceneSessionMap_[TEST_WINDOW_ID] = nullptr;
    ssm_->CancelAdaptiveResolutionInternal(TEST_UID_B, { TEST_WINDOW_ID });
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_B].count(TEST_WINDOW_ID), 1u);
}

/**
 * @tc.name: NotifyAvailableVariants
 * @tc.desc: no records, stale records, restorable/blocked candidates and the hooked guard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, NotifyAvailableVariants, TestSize.Level1)
{
    auto sessionA = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionB = CreateTestSceneSession(TEST_WINDOW_ID_2, TEST_UID_A, WindowMode::WINDOW_MODE_FLOATING);
    ASSERT_NE(sessionA, nullptr);
    ASSERT_NE(sessionB, nullptr);

    // no cancelled records returns directly
    ssm_->NotifyAdaptiveResolutionAvailable(TEST_UID_A, false);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    // displayHooked=true blocks restore and keeps all records
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID };
    ssm_->NotifyAdaptiveResolutionAvailable(TEST_UID_A, true);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);
    // records of destroyed windows are dropped
    ssm_->sceneSessionMap_[TEST_WINDOW_ID] = nullptr;
    ssm_->NotifyAdaptiveResolutionAvailable(TEST_UID_A, false);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    // restorable windows consume the record; blocked ones keep it
    ssm_->sceneSessionMap_.erase(TEST_WINDOW_ID);
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID, TEST_WINDOW_ID_2 };
    ssm_->NotifyAdaptiveResolutionAvailable(TEST_UID_A, false);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].size(), 1u);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID_2), 1u);

    // the display-hook disable entry feeds the restore flow with the known end state
    ssm_->sceneSessionMap_[TEST_WINDOW_ID] = sessionA;
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID };
    ssm_->UpdateDisplayHookInfo(TEST_UID_A, 0, 0, 0.0f, false);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
}

/**
 * @tc.name: TryRestoreVariants
 * @tc.desc: entry fast path plus the core pending/blocked/success branches
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, TryRestoreVariants, TestSize.Level1)
{
    auto session = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(session, nullptr);

    // not pending: returns before any DMS query; null session returns directly
    ssm_->TryNotifyAdaptiveResolutionAvailable(session, true);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    ssm_->TryNotifyAdaptiveResolutionAvailable(nullptr, true);
    EXPECT_FALSE(ssm_->TryRestoreAdaptiveResolutionWindow(session, true, false));
    EXPECT_FALSE(ssm_->IsWindowPendingRestore(TEST_UID_A, TEST_WINDOW_ID));

    // pending windows fail the restore when conditions miss
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID };
    EXPECT_FALSE(ssm_->TryRestoreAdaptiveResolutionWindow(session, false, false)); // not fullscreen
    EXPECT_FALSE(ssm_->TryRestoreAdaptiveResolutionWindow(session, true, true));  // display hooked
    auto property = session->GetSessionProperty();
    ASSERT_NE(property, nullptr);
    HookWindowInfo hookInfo;
    hookInfo.enableHookWindow = true;
    property->SetHookWindowInfo(hookInfo);
    EXPECT_FALSE(ssm_->TryRestoreAdaptiveResolutionWindow(session, true, false)); // window hook on
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].count(TEST_WINDOW_ID), 1u);

    // entry with all conditions met consumes the record
    property->SetHookWindowInfo(HookWindowInfo {});
    ssm_->TryNotifyAdaptiveResolutionAvailable(session, true);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());

    // all conditions met erases the record via the core directly
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID };
    EXPECT_TRUE(ssm_->TryRestoreAdaptiveResolutionWindow(session, true, false));
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
}

/**
 * @tc.name: RestoreHelpersVariants
 * @tc.desc: record erase, fullscreen check and the DMS config conversion
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, RestoreHelpersVariants, TestSize.Level1)
{
    ssm_->EraseCancelledGameWindow(TEST_UID_A, TEST_WINDOW_ID);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID, TEST_WINDOW_ID_2 };
    ssm_->EraseCancelledGameWindow(TEST_UID_A, TEST_WINDOW_ID);
    EXPECT_EQ(ssm_->cancelledGameWindowsByUid_[TEST_UID_A].size(), 1u);
    ssm_->EraseCancelledGameWindow(TEST_UID_A, TEST_WINDOW_ID_2);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());

    EXPECT_FALSE(ssm_->IsWindowFullScreen(nullptr));
    auto sessionA = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionB = CreateTestSceneSession(TEST_WINDOW_ID_2, TEST_UID_A, WindowMode::WINDOW_MODE_FLOATING);
    ASSERT_NE(sessionA, nullptr);
    ASSERT_NE(sessionB, nullptr);
    EXPECT_TRUE(ssm_->IsWindowFullScreen(sessionA));
    EXPECT_FALSE(ssm_->IsWindowFullScreen(sessionB));

    GameAdaptiveResolutionConfig config;
    config.enable_ = true;
    config.scaleRatio_ = 0.75f;
    config.autoMetrics_ = false;
    config.notifyDisplay_ = false;
    config.density_ = 2.0f;
    config.xDpi_ = 320.0f;
    config.yDpi_ = 330.0f;
    auto dmConfig = ssm_->ToDmAdaptiveResolutionConfig(config);
    EXPECT_EQ(dmConfig.scaleRatio_, 0.75f);
    EXPECT_FALSE(dmConfig.autoMetrics_);
    EXPECT_FALSE(dmConfig.notifyDisplay_);
    EXPECT_EQ(dmConfig.density_, 2.0f);
    EXPECT_EQ(dmConfig.xDpi_, 320.0f);
    EXPECT_EQ(dmConfig.yDpi_, 330.0f);
}

/**
 * @tc.name: ClearVariants
 * @tc.desc: window destroy and process death cleanup paths
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, ClearVariants, TestSize.Level1)
{
    auto sessionA = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionB = CreateTestSceneSession(TEST_WINDOW_ID_2, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    auto sessionC = CreateTestSceneSession(TEST_WINDOW_ID_3, TEST_UID_B, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(sessionA, nullptr);
    ASSERT_NE(sessionB, nullptr);
    ASSERT_NE(sessionC, nullptr);

    // unknown window: nothing happens
    ssm_->ClearAdaptiveResolutionConfig(TEST_WINDOW_ID_3);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
    // cancelled record of a destroyed window is cleaned even without a config record
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID };
    ssm_->ClearAdaptiveResolutionConfig(TEST_WINDOW_ID);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    // sibling keeps the hook
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID] = TEST_UID_A;
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_2] = TEST_UID_A;
    ssm_->ClearAdaptiveResolutionConfig(TEST_WINDOW_ID);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_2], TEST_UID_A);
    // last window clears the record
    ssm_->ClearAdaptiveResolutionConfig(TEST_WINDOW_ID_2);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());

    // process death erases config and cancelled records of the uid
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID] = TEST_UID_A;
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_2] = TEST_UID_A;
    ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_3] = TEST_UID_B;
    ssm_->cancelledGameWindowsByUid_[TEST_UID_A] = { TEST_WINDOW_ID };
    ssm_->ClearAdaptiveResolutionConfigByUid(TEST_UID_A);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_[TEST_WINDOW_ID_3], TEST_UID_B);
    EXPECT_TRUE(ssm_->cancelledGameWindowsByUid_.empty());
    // unknown uid: no side effect
    ssm_->ClearAdaptiveResolutionConfigByUid(TEST_UID_B + 100);
    EXPECT_EQ(ssm_->adaptiveResolutionWindows_.size(), 1u);
}

/**
 * @tc.name: AdaptiveSessionFuncs
 * @tc.desc: func registration and the mode/hookWindow change notifications
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, AdaptiveSessionFuncs, TestSize.Level1)
{
    ssm_->RegisterAdaptiveResolutionConfigFunc(nullptr);
    ssm_->RegisterAdaptiveResolutionMonitorFunc(nullptr);

    auto session = CreateTestSceneSession(TEST_WINDOW_ID, TEST_UID_A, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(session, nullptr);
    ssm_->RegisterAdaptiveResolutionConfigFunc(session);
    ssm_->RegisterAdaptiveResolutionMonitorFunc(session);
    EXPECT_NE(session->adaptiveResolutionConfigFunc_, nullptr);

    WindowMode observedMode = WindowMode::WINDOW_MODE_FLOATING;
    bool observedHookEnabled = false;
    int32_t hookNotifyCount = 0;
    auto modeFunc = [&observedMode](WindowMode newMode) { observedMode = newMode; };
    auto hookFunc = [&observedHookEnabled, &hookNotifyCount](bool enable) {
        observedHookEnabled = enable;
        hookNotifyCount++;
    };
    // no crash without registered funcs
    session->NotifyAdaptiveResolutionModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    session->NotifyAdaptiveResolutionHookWindowChange(true);
    EXPECT_EQ(observedMode, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(hookNotifyCount, 0);

    session->SetAdaptiveResolutionModeChangeFunc(modeFunc);
    session->SetAdaptiveResolutionHookWindowChangeFunc(hookFunc);
    session->NotifyAdaptiveResolutionModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    session->NotifyAdaptiveResolutionHookWindowChange(true);
    EXPECT_EQ(observedMode, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_TRUE(observedHookEnabled);
    EXPECT_EQ(hookNotifyCount, 1);
}

/**
 * @tc.name: NotifyGameAdaptiveAgentsFanOut
 * @tc.desc: the posted task fans out to GAME_ADAPTIVE agents for both branches
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, NotifyGameAdaptiveAgentsFanOut, TestSize.Level1)
{
    auto& controller = SessionManagerAgentController::GetInstance();
    // no agent registered: no crash
    controller.NotifyGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    controller.NotifyGameAdaptiveResolutionAvailable(TEST_WINDOW_ID);

    auto agent = sptr<GameAdaptiveAgentMocker>::MakeSptr();
    ASSERT_NE(agent, nullptr);
    ASSERT_TRUE(controller.smAgentContainer_.RegisterAgent(agent,
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE));

    ssm_->NotifyGameAdaptiveAgents(true, TEST_WINDOW_ID);
    ssm_->NotifyGameAdaptiveAgents(false, TEST_WINDOW_ID_2);
    usleep(300000); // 300ms: let the scheduler thread run both posted tasks
    EXPECT_EQ(agent->cancelledWindowId_, TEST_WINDOW_ID);
    EXPECT_EQ(agent->availableWindowId_, TEST_WINDOW_ID_2);

    ASSERT_TRUE(controller.smAgentContainer_.UnregisterAgent(agent,
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE));
    // unregistered: no callback
    agent->cancelledWindowId_ = -1;
    controller.NotifyGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(agent->cancelledWindowId_, -1);
}

/**
 * @tc.name: RegisterAgentGameAdaptiveVariants
 * @tc.desc: SA-caller gate, null-agent rejection and the register/unregister round trip
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, RegisterAgentGameAdaptiveVariants, TestSize.Level1)
{
    auto agent = sptr<GameAdaptiveAgentMocker>::MakeSptr();
    ASSERT_NE(agent, nullptr);

    MockAccesstokenKit::MockIsSACalling(false);
    EXPECT_EQ(ssm_->RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE, agent),
        WMError::WM_ERROR_INVALID_PERMISSION);
    EXPECT_EQ(ssm_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE, agent),
        WMError::WM_ERROR_INVALID_PERMISSION);

    MockAccesstokenKit::MockIsSACalling(true);
    EXPECT_EQ(ssm_->RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE, nullptr),
        WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(ssm_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE, nullptr),
        WMError::WM_ERROR_NULLPTR);

    // round trip: the controller notify reaches the agent until unregistered
    EXPECT_EQ(ssm_->RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE, agent), WMError::WM_OK);
    SessionManagerAgentController::GetInstance().NotifyGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(agent->cancelledWindowId_, TEST_WINDOW_ID);
    EXPECT_EQ(ssm_->UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GAME_ADAPTIVE, agent), WMError::WM_OK);
    agent->cancelledWindowId_ = -1;
    SessionManagerAgentController::GetInstance().NotifyGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(agent->cancelledWindowId_, -1); // unregistered: no callback
}

/**
 * @tc.name: LiteForwardSetGameAdaptivePermissionDenied
 * @tc.desc: the lite forwarding entry keeps the SA-caller gate of the SCB entry
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAdaptiveTest, LiteForwardSetGameAdaptivePermissionDenied, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    auto ret = SceneSessionManagerLite::GetInstance().SetGameAdaptiveResolutionForWindow(
        MakeConfig(TEST_WINDOW_ID, true));
    EXPECT_EQ(ret, WMError::WM_ERROR_NOT_SYSTEM_APP);
    EXPECT_TRUE(ssm_->adaptiveResolutionWindows_.empty());
}

}
} // namespace Rosen
} // namespace OHOS
