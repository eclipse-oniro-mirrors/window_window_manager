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

#include "scene_session_manager.h"
#include "session_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionManagerTest : public Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static sptr<SceneSessionManager> ssm_;
};

sptr<SceneSessionManager> SceneSessionManagerTest::ssm_ = nullptr;

void SceneSessionManagerTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest::TearDown()
{
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: IsPcSceneSessionLifecycle1
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsPcSceneSessionLifecycle1, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.backgroundswitch = true;
    SessionInfo info;
    info.abilityName_ = "IsPcSceneSessionLifecycle1";
    info.bundleName_ = "IsPcSceneSessionLifecycle1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->property_->SetIsAppSupportPhoneInPc(false);
    sceneSession->property_->SetIsPcAppInPad(false);
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;;

    bool ret = ssm_->IsPcSceneSessionLifecycle(sceneSession);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsPcSceneSessionLifecycle2
 * @tc.desc: pc app in pad
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsPcSceneSessionLifecycle2, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.backgroundswitch = false;
    SessionInfo info;
    info.abilityName_ = "IsPcSceneSessionLifecycle2";
    info.bundleName_ = "IsPcSceneSessionLifecycle2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    auto defaultUIType = ssm_->systemConfig_.windowUIType_;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    bool oldLocked = ssm_->isScreenLocked_;
    sceneSession->property_->SetIsAppSupportPhoneInPc(false);
    sceneSession->property_->SetIsPcAppInPad(true);
    ssm_->isScreenLocked_ = false;
    bool ret = ssm_->IsPcSceneSessionLifecycle(sceneSession);
    ssm_->isScreenLocked_ = oldLocked;
    ssm_->systemConfig_.windowUIType_ = defaultUIType;
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsPcSceneSessionLifecycle3
 * @tc.desc: Compatible mode in pc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsPcSceneSessionLifecycle3, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.backgroundswitch = true;
    SessionInfo info;
    info.abilityName_ = "IsPcSceneSessionLifecycle3";
    info.bundleName_ = "IsPcSceneSessionLifecycle3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->property_->SetIsAppSupportPhoneInPc(true);
    sceneSession->property_->SetIsPcAppInPad(false);
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;;

    bool ret = ssm_->IsPcSceneSessionLifecycle(sceneSession);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: InitUserInfo
 * @tc.desc: SceneSesionManager init user info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, InitUserInfo, Function | SmallTest | Level3)
{
    int32_t newUserId = 10086;
    std::string fileDir;
    WSError result01 = ssm_->InitUserInfo(newUserId, fileDir);
    ASSERT_EQ(result01, WSError::WS_DO_NOTHING);
    fileDir = "newFileDir";
    WSError result02 = ssm_->InitUserInfo(newUserId, fileDir);
    ASSERT_EQ(result02, WSError::WS_OK);
}

/**
 * @tc.name: IsInvalidMainSessionOnUserSwitch1
 * @tc.desc: invalid window type
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsNeedChangeLifeCycleOnUserSwitch1, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.backgroundswitch = true;
    int32_t pid = 12345;
    SessionInfo info;
    info.abilityName_ = "IsNeedChangeLifeCycleOnUserSwitch1";
    info.bundleName_ = "IsNeedChangeLifeCycleOnUserSwitch1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetCallingPid(45678);
    sceneSession->property_->SetIsAppSupportPhoneInPc(false);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);

    bool ret = ssm_->IsNeedChangeLifeCycleOnUserSwitch(sceneSession, pid);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNeedChangeLifeCycleOnUserSwitch2
 * @tc.desc: invalid window state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsNeedChangeLifeCycleOnUserSwitch2, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = 12345;
    ssm_->systemConfig_.backgroundswitch = true;
    SessionInfo info;
    info.abilityName_ = "IsNeedChangeLifeCycleOnUserSwitch2";
    info.bundleName_ = "IsNeedChangeLifeCycleOnUserSwitch2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetCallingPid(45678);
    sceneSession->property_->SetIsAppSupportPhoneInPc(false);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionState(SessionState::STATE_END);

    bool ret = ssm_->IsNeedChangeLifeCycleOnUserSwitch(sceneSession, pid);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsNeedChangeLifeCycleOnUserSwitch3
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsNeedChangeLifeCycleOnUserSwitch3, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = 12345;
    ssm_->systemConfig_.backgroundswitch = true;
    SessionInfo info;
    info.abilityName_ = "IsNeedChangeLifeCycleOnUserSwitch3";
    info.bundleName_ = "IsNeedChangeLifeCycleOnUserSwitch3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetCallingPid(45678);
    sceneSession->property_->SetIsAppSupportPhoneInPc(false);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);

    bool ret = ssm_->IsNeedChangeLifeCycleOnUserSwitch(sceneSession, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsNeedChangeLifeCycleOnUserSwitch4
 * @tc.desc: Invalid pid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsNeedChangeLifeCycleOnUserSwitch4, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = 12345;
    SessionInfo info;
    info.abilityName_ = "IsNeedChangeLifeCycleOnUserSwitch4";
    info.bundleName_ = "IsNeedChangeLifeCycleOnUserSwitch4";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetCallingPid(pid);

    bool ret = ssm_->IsNeedChangeLifeCycleOnUserSwitch(sceneSession, pid);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: StartOrMinimizeUIAbilityBySCB1
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, StartOrMinimizeUIAbilityBySCB1, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "StartOrMinimizeUIAbilityBySCB1";
    info.bundleName_ = "StartOrMinimizeUIAbilityBySCB1";
    ssm_->systemConfig_.backgroundswitch = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->SetSessionProperty(property);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetIsAppSupportPhoneInPc(false);

    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession->SetMinimizedFlagByUserSwitch(true);

    WSError ret = ssm_->StartOrMinimizeUIAbilityBySCB(sceneSession, true);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: StartOrMinimizeUIAbilityBySCB2
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, StartOrMinimizeUIAbilityBySCB2, Function | SmallTest | Level3)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "StartOrMinimizeUIAbilityBySCB2";
    info.bundleName_ = "StartOrMinimizeUIAbilityBySCB2";
    ssm_->systemConfig_.backgroundswitch = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->SetSessionProperty(property);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetIsAppSupportPhoneInPc(false);

    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    WSError ret = ssm_->StartOrMinimizeUIAbilityBySCB(sceneSession, false);
    EXPECT_EQ(ret, WSError::WS_OK);
}
} // namespace Rosen
} // namespace OHOS