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

#include <gtest/gtest.h>
#include <regex>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>

#include "common_test_utils.h"
#include "context.h"
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_scene_session.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_accesstoken_kit.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_window_manager_agent_lite.h"
#include "session_manager/include/session_manager_agent_controller.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionManagerKioskTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerKioskTest::ssm_ = nullptr;

void SceneSessionManagerKioskTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerKioskTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerKioskTest::SetUp() {}

void SceneSessionManagerKioskTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name  : ExitKioskMode_ShouleReturnInvalidPermission_WhenCalledByNonSystemApp
 * @tc.number: ExitKioskModeTest_001
 * @tc.desc  : When a non-system application calls ExitKioskMode, it should return WM_ERROR_INVALID_PERMISSION.
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_ExitKioskMode_ShouleReturnInvalidPermission_WhenCalledByNonSystemApp, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    WMError result = ssm_->ExitKioskMode();
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name  : ExitKioskMode_ShouldCallKioskModeChangeFunc_WhenKioskModeChangeFuncIsNotNull
 * @tc.number: ExitKioskModeTest_002
 * @tc.desc  : When kioskModeChangeFunc_ is not empty, it should be called and WM_OK should be returned.
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_ExitKioskMode_ShouldCallKioskModeChangeFunc_WhenKioskModeChangeFuncIsNotNull, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(true);
    ssm_->kioskModeChangeFunc_ = [](bool mode, int32_t sessionId, KioskType kioskType) {
        return WMError::WM_OK;
    };
    WMError result = ssm_->ExitKioskMode();
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name  : ExitKioskMode_ShouldCallKioskModeChangeFunc_WhenKioskModeChangeFuncIsNull
 * @tc.number: ExitKioskModeTest_003
 * @tc.desc  : When kioskModeChangeFunc_ is empty, isKioskMode_ should be set and WM_OK should be returned.
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_ExitKioskMode_ShouldCallKioskModeChangeFunc_WhenKioskModeChangeFuncIsNull, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(true);
    ssm_->kioskModeChangeFunc_ = nullptr;
    WMError result = ssm_->ExitKioskMode();
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_FALSE(ssm_->isKioskMode_);
    EXPECT_EQ(ssm_->kioskAppPersistentId_, INVALID_SESSION_ID);
}

/**
 * @tc.name  : EnterKioskMode_ShouldReturnInvalidPermission_WhenCalledByNonSystemOrSA
 * @tc.number: EnterKioskMode_001
 * @tc.desc  : When the caller is not a system application or SA, it should return WM_ERROR_INVALID_PERMISSION
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_EnterKioskMode_ShouldReturnInvalidPermission_WhenCalledByNonSystemOrSA, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    sptr<IRemoteObject> token = new MockIRemoteObject();
    EXPECT_EQ(ssm_->EnterKioskMode(token), WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name  : EnterKioskMode_ShouldReturnInvalidParam_WhenSessionNotFound
 * @tc.number: EnterKioskMode_002
 * @tc.desc  : When the corresponding session cannot be found, EnterKioskMode should return WM_ERROR_INVALID_PARAM
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_EnterKioskMode_ShouldReturnInvalidParam_WhenSessionNotFound, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(false);
    sptr<IRemoteObject> token = new MockIRemoteObject();
    EXPECT_EQ(ssm_->EnterKioskMode(token), WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : EnterKioskMode_ShouldCallKioskModeChangeFunc_WhenKioskModeChangeFuncIsNotNull
 * @tc.number: EnterKioskMode_003
 * @tc.desc  : Test when kioskModeChangeFunc_ is not empty, EnterKioskMode should call func and return WM_OK
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_EnterKioskMode_ShouldCallKioskModeChangeFunc_WhenKioskModeChangeFuncIsNotNull, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(true);
    SessionInfo info;
    info.persistentId_ = 101;
    sptr<SceneSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->kioskModeChangeFunc_ = [](bool, int32_t, KioskType) {};
    sptr<IRemoteObject> token = new MockIRemoteObject();
    session->SetAbilityToken(token);
    ssm_->sceneSessionMap_[101] = session;
    EXPECT_EQ(ssm_->EnterKioskMode(token), WMError::WM_OK);
}

/**
 * @tc.name  : EnterKioskMode_ShouldSetKioskModeAndPersistentId_WhenKioskModeChangeFuncIsNull
 * @tc.number: EnterKioskMode_004
 * @tc.desc  : Test that when kioskModeChangeFun_ is empty, EnterKioskMode should set isKioskMode_ and return WM_OK
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_EnterKioskMode_ShouldSetKioskModeAndPersistentId_WhenKioskModeChangeFuncIsNull, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(true);
    SessionInfo info;
    info.persistentId_ = 101;
    sptr<SceneSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->kioskModeChangeFunc_ = nullptr;
    sptr<IRemoteObject> token = new MockIRemoteObject();
    session->SetAbilityToken(token);
    ssm_->sceneSessionMap_[101] = session;
    EXPECT_EQ(ssm_->EnterKioskMode(token), WMError::WM_OK);
    EXPECT_TRUE(ssm_->isKioskMode_);
    EXPECT_EQ(ssm_->kioskAppPersistentId_, session->GetPersistentId());
}

/**
 * @tc.name  : UpdateKioskAppList_ShouldReturnInvalidPermission_WhenCalledByNonSystemOrSA
 * @tc.number: UpdateKioskAppList_001
 * @tc.desc  : when the caller is not a system application or SA, it should return WM_ERROR_INVALID_PERMISSION
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_UpdateKioskAppList_ShouldReturnInvalidPermission_WhenCalledByNonSystemOrSA, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    std::vector<std::string> kioskAppList = {"app1", "app2"};
    WMError result = ssm_->UpdateKioskAppList(kioskAppList);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name  : UpdateKioskAppList_ShouldCallUpdateKioskAppListFunc_WhenUpdateKioskAppListFuncIsNotNull
 * @tc.number: UpdateKioskAppList_003
 * @tc.desc  : When updateKioskAppListFun_ is not empty, UpdateKioskAppList should call updateKisokAppListFunc_
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_UpdateKioskAppList_ShouldCallUpdateKioskAppListFunc_WhenUpdateKioskAppListFuncIsNotNull, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(true);
    ssm_->updateKioskAppListFunc_ = [](const std::vector<std::string>& kioskAppList) {
        return WMError::WM_OK;
    };
    std::vector<std::string> kioskAppList = {"app1", "app2"};
    WMError result = ssm_->UpdateKioskAppList(kioskAppList);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name  : UpdateKioskAppList_ShouldCacheKioskAppList_WhenUpdateKioskAppListFuncIsNull
 * @tc.number: UpdateKioskAppList_004
 * @tc.desc  : When updateKioskAppListFun_ is empty, UpdateKioskAppList should cache kioskAppList to kioskAppListCache_
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_UpdateKioskAppList_ShouldCacheKioskAppList_WhenUpdateKioskAppListFuncIsNull, TestSize.Level0)
{
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(false);
    ssm_->updateKioskAppListFunc_ = nullptr;
    std::vector<std::string> kioskAppList = {"app1", "app2"};
    WMError result = ssm_->UpdateKioskAppList(kioskAppList);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(ssm_->kioskAppListCache_.size(), kioskAppList.size());
}

/**
 * @tc.name  : RegisterUpdateKioskAppListCallback_ShouldRegisterCallBack_WhenFuncProvided
 * @tc.number: RegisterUpdateKioskAppListCallbackTest_001
 * @tc.desc  : Test when providing a callback function, RegisterUpdateKioskAppListCallback should succeed
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_RegisterUpdateKioskAppListCallback_ShouldRegisterCallBack_WhenFuncProvided, TestSize.Level0)
{
    auto mockFunc = [](const std::vector<std::string>& appList) {};
    ssm_->RegisterUpdateKioskAppListCallback(std::move(mockFunc));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(ssm_->updateKioskAppListFunc_ != nullptr);
}

/**
 * @tc.name  : RegisterUpdateKioskAppListCallback_ShouldCallCallback_WhenKioskAppListCacheNotEmpty
 * @tc.number: RegisterUpdateKioskAppListCallbackTest_002
 * @tc.desc  : Test when kioskAppListCache_ is not null, callback function should be called
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_RegisterUpdateKioskAppListCallback_ShouldCallCallback_WhenKioskAppListCacheNotEmpty, TestSize.Level0)
{
    bool callbackCalled = false;
    auto mockFunc = [&callbackCalled](const std::vector<std::string>& appList) {
        callbackCalled = true;
    };
    ssm_->kioskAppListCache_.push_back("app1");
    ssm_->kioskAppListCache_.push_back("app2");
    ssm_->RegisterUpdateKioskAppListCallback(std::move(mockFunc));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(callbackCalled, true);
}

/**
 * @tc.name  : RegisterKioskModeChangeCallback_ShouldCallCallback_WhenValidFuncIsProvided
 * @tc.number: RegisterKioskModeChangeCallbackTest_001
 * @tc.desc  : Test whether the function is called correctly when a valid callback function is passed in
 */
HWTEST_F(SceneSessionManagerKioskTest,
    ATC_RegisterKioskModeChangeCallback_ShouldCallCallback_WhenValidFuncIsProvided, TestSize.Level0)
{
    ssm_->isKioskMode_ = false;
    auto callback = [](bool isKioskMode, int32_t kioskAppPersistentId, KioskType kioskType) {
        EXPECT_FALSE(isKioskMode);
    };
    ssm_->RegisterKioskModeChangeCallback(std::move(callback));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(ssm_->kioskModeChangeFunc_ != nullptr);
}
} // namespace
} // namespace Rosen
} // namespace OHOS