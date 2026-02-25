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
#include "session/host/include/scb_system_session.h"
#include "common/include/session_permission.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "session/host/include/session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
class SCBSystemSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    SessionInfo info;
    sptr<SCBSystemSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SCBSystemSession> scbSystemSession_;
};

void SCBSystemSessionTest::SetUpTestCase() {}

void SCBSystemSessionTest::TearDownTestCase() {}

void SCBSystemSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSCBSystemSession1";
    info.moduleName_ = "testSCBSystemSession2";
    info.bundleName_ = "testSCBSystemSession3";
    scbSystemSession_ = sptr<SCBSystemSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(nullptr, scbSystemSession_);
}

void SCBSystemSessionTest::TearDown()
{
    scbSystemSession_ = nullptr;
}

namespace {
/**
 * @tc.name: IsVisibleNotBackground
 * @tc.desc: check func IsVisibleNotBackground
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, IsVisibleNotBackground, TestSize.Level1)
{
    ASSERT_NE(scbSystemSession_, nullptr);
    EXPECT_EQ(false, scbSystemSession_->IsVisibleNotBackground());
    scbSystemSession_->isVisible_ = true;
    EXPECT_EQ(true, scbSystemSession_->IsVisibleNotBackground());
}

/**
 * @tc.name: TransferKeyEvent01
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, TransferKeyEvent01, TestSize.Level1)
{
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, scbSystemSession_->TransferKeyEvent(nullptr));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, TransferKeyEvent02, TestSize.Level1)
{
    scbSystemSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    scbSystemSession_->windowEventChannel_ = nullptr;

    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, scbSystemSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: UpdateFocus01
 * @tc.desc: check func UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, UpdateFocus01, TestSize.Level1)
{
    bool isFocused = scbSystemSession_->isFocused_;

    EXPECT_EQ(WSError::WS_DO_NOTHING, scbSystemSession_->UpdateFocus(isFocused));
}

/**
 * @tc.name: UpdateFocus02
 * @tc.desc: check func UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, UpdateFocus02, TestSize.Level1)
{
    scbSystemSession_->isFocused_ = true;
    bool isFocused = scbSystemSession_->isFocused_;

    EXPECT_EQ(WSError::WS_OK, scbSystemSession_->UpdateFocus(!isFocused));

    scbSystemSession_->isFocused_ = false;
    isFocused = scbSystemSession_->isFocused_;

    EXPECT_EQ(WSError::WS_OK, scbSystemSession_->UpdateFocus(!isFocused));
}

/**
 * @tc.name: BindKeyboardSession01
 * @tc.desc: check func BindKeyboardSession
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, BindKeyboardSession01, TestSize.Level1)
{
    ASSERT_NE(nullptr, scbSystemSession_);
    scbSystemSession_->BindKeyboardSession(nullptr);
}

/**
 * @tc.name: BindKeyboardSession02
 * @tc.desc: check func BindKeyboardSession
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, BindKeyboardSession02, TestSize.Level1)
{
    ASSERT_NE(nullptr, scbSystemSession_);
    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, callback);
    scbSystemSession_->BindKeyboardSession(session);
}

/**
 * @tc.name: SetSystemSceneBlockingFocus
 * @tc.desc: check func SetSystemSceneBlockingFocus
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, SetSystemSceneBlockingFocus01, TestSize.Level1)
{
    ASSERT_NE(nullptr, scbSystemSession_);
    WSError ret = scbSystemSession_->SetSystemSceneBlockingFocus(true);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: PresentFocusIfPointDown
 * @tc.desc: check func PresentFocusIfPointDown
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, PresentFocusIfPointDown01, TestSize.Level1)
{
    ASSERT_NE(nullptr, scbSystemSession_);
    scbSystemSession_->PresentFocusIfPointDown();
}

/**
 * @tc.name: GetKeyboardSession
 * @tc.desc: check func GetKeyboardSession
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, GetKeyboardSession01, TestSize.Level1)
{
    ASSERT_NE(nullptr, scbSystemSession_);
    scbSystemSession_->GetKeyboardSession();
}

/**
 * @tc.name: ProcessPointDownSession
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, ProcessPointDownSession, TestSize.Level1)
{
    int32_t posX = 0;
    int32_t posY = 0;
    WSError ret = scbSystemSession_->ProcessPointDownSession(posX, posY);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: PresentFocusIfPointDown02
 * @tc.desc: check func PresentFocusIfPointDown
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, PresentFocusIfPointDown02, TestSize.Level1)
{
    scbSystemSession_->isFocused_ = true;
    auto ret = scbSystemSession_->SetFocusable(false);
    ASSERT_EQ(WSError::WS_OK, ret);
    scbSystemSession_->PresentFocusIfPointDown();
    scbSystemSession_->PresentFocusIfNeed(2, 0);
    ASSERT_EQ(scbSystemSession_->isFocused_, true);

    scbSystemSession_->isFocused_ = false;
    ret = scbSystemSession_->SetFocusable(false);
    ASSERT_EQ(WSError::WS_OK, ret);
    scbSystemSession_->PresentFocusIfPointDown();
    scbSystemSession_->PresentFocusIfNeed(2, 0);
    ASSERT_EQ(scbSystemSession_->isFocused_, false);

    scbSystemSession_->isFocused_ = true;
    ret = scbSystemSession_->SetFocusable(true);
    ASSERT_EQ(WSError::WS_OK, ret);
    scbSystemSession_->PresentFocusIfPointDown();
    scbSystemSession_->PresentFocusIfNeed(2, 0);
    ASSERT_EQ(scbSystemSession_->isFocused_, true);
}

/**
 * @tc.name: PresentFocusIfNeed
 * @tc.desc: check func PresentFocusIfNeed
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, PresentFocusIfNeed, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t pointerAction = 8;
    scbSystemSession_->PresentFocusIfNeed(pointerAction, 0);
    EXPECT_FALSE(g_errLog.find("OnClick down, id:") != std::string::npos);

    pointerAction = 100;
    scbSystemSession_->PresentFocusIfNeed(pointerAction, 0);
    EXPECT_FALSE(g_errLog.find("OnClick down, id:") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSkipEventOnCastPlus01
 * @tc.desc: check func SetSkipEventOnCastPlus
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    scbSystemSession_->SetSkipEventOnCastPlus(false);
    ASSERT_EQ(false, scbSystemSession_->GetSessionProperty()->GetSkipEventOnCastPlus());
}

/**
 * @tc.name: SCBSystemSession
 * @tc.desc: check func SCBSystemSession Construction method
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, SCBSystemSessionConstruction001, TestSize.Level1)
{
    SessionInfo info1;
    info1.abilityName_ = "testSCBSystemSession1";
    info1.moduleName_ = "testSCBSystemSession2";
    info1.bundleName_ = "SCBDesktop";
    info1.isSystem_ = true;
    auto scbSystemSession1 = sptr<SCBSystemSession>::MakeSptr(info1, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession1);

    SessionInfo info2;
    info2.abilityName_ = "testSCBSystemSession1";
    info2.moduleName_ = "testSCBSystemSession2";
    info2.bundleName_ = "SCBWallpaper";
    info2.isSystem_ = true;
    auto scbSystemSession2 = sptr<SCBSystemSession>::MakeSptr(info2, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession2);

    SessionInfo info3;
    info3.abilityName_ = "testSCBSystemSession1";
    info3.moduleName_ = "testSCBSystemSession2";
    info3.bundleName_ = "SCBScreenLock";
    info3.isSystem_ = true;
    auto scbSystemSession3 = sptr<SCBSystemSession>::MakeSptr(info3, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession3);

    SessionInfo info4;
    info4.abilityName_ = "testSCBSystemSession1";
    info4.moduleName_ = "testSCBSystemSession2";
    info4.bundleName_ = "SCBNegativeScreen";
    info4.isSystem_ = true;
    auto scbSystemSession4 = sptr<SCBSystemSession>::MakeSptr(info4, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession4);

    SessionInfo info5;
    info5.abilityName_ = "testSCBSystemSession1";
    info5.moduleName_ = "testSCBSystemSession2";
    info5.bundleName_ = "SCBDropdownPanel";
    info5.isSystem_ = true;
    auto scbSystemSession5 = sptr<SCBSystemSession>::MakeSptr(info5, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession5);
}

/**
 * @tc.name: SCBSystemSession
 * @tc.desc: check func SCBSystemSession Construction method
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionTest, SCBSystemSessionConstruction002, TestSize.Level1)
{
    SessionInfo info6;
    info6.abilityName_ = "testSCBSystemSession1";
    info6.moduleName_ = "testSCBSystemSession2";
    info6.bundleName_ = "SCBVolumePanel";
    info6.isSystem_ = true;
    auto scbSystemSession6 = sptr<SCBSystemSession>::MakeSptr(info6, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession6);

    SessionInfo info7;
    info7.abilityName_ = "testSCBSystemSession1";
    info7.moduleName_ = "testSCBSystemSession2";
    info7.bundleName_ = "SCBBannerNotification";
    info7.isSystem_ = true;
    auto scbSystemSession7 = sptr<SCBSystemSession>::MakeSptr(info7, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession7);

    SessionInfo info8;
    info8.abilityName_ = "testSCBSystemSession1";
    info8.moduleName_ = "testSCBSystemSession2";
    info8.bundleName_ = "SCBGestureBack";
    info8.isSystem_ = true;
    auto scbSystemSession8 = sptr<SCBSystemSession>::MakeSptr(info8, specificCallback);
    ASSERT_NE(nullptr, scbSystemSession8);
}
} // namespace
} // namespace Rosen
} // namespace OHOSS