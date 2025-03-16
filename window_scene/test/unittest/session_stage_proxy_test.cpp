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

#include "session/container/include/zidl/session_stage_proxy.h"
#include "iremote_object_mocker.h"
#include <gtest/gtest.h>
#include "proto.h"
#include "string_wrapper.h"
#include "util.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "mock_message_parcel.h"


using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionStageProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker = new (std::nothrow) IRemoteObjectMocker();
    sptr<SessionStageProxy> sessionStage_ = new SessionStageProxy(iRemoteObjectMocker);
};

void SessionStageProxyTest::SetUpTestCase()
{
}

void SessionStageProxyTest::TearDownTestCase()
{
}

void SessionStageProxyTest::SetUp()
{
}

void SessionStageProxyTest::TearDown()
{
}

namespace {
/**
 * @tc.name: SetActive
 * @tc.desc: test function : SetActive
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetActive, Function | SmallTest | Level1)
{
    bool active = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->SetActive(active);
    ASSERT_EQ(WSError::WS_OK, res);
    active = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    res = sessionStage_->SetActive(active);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateDisplayId
 * @tc.desc: test function : UpdateDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateDisplayId, Function | SmallTest | Level1)
{
    uint64_t displayID = 0;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateDisplayId(displayID);
    ASSERT_EQ(WSError::WS_OK, res);
    displayID = 1;
    res = sessionStage_->UpdateDisplayId(displayID);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateRect
 * @tc.desc: test function : UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateRect, Function | SmallTest | Level1)
{
    WSRect rect;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateRect(rect, reason);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: HandleBackEvent
 * @tc.desc: test function : HandleBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, HandleBackEvent, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->HandleBackEvent();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: test function : GetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, GetUIContentRemoteObj, Function | SmallTest | Level1)
{
    ASSERT_NE(sessionStage_, nullptr);
    sptr<IRemoteObject> remoteObj;
    WSError res = sessionStage_->GetUIContentRemoteObj(remoteObj);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: MarkProcessed
 * @tc.desc: test function : MarkProcessed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, MarkProcessed, Function | SmallTest | Level1)
{
    int32_t eventId = 1;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->MarkProcessed(eventId);
    ASSERT_EQ(WSError::WS_DO_NOTHING, res);
}

/**
 * @tc.name: UpdateFocus
 * @tc.desc: test function : UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateFocus, Function | SmallTest | Level1)
{
    bool focus = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateFocus(focus);
    ASSERT_EQ(WSError::WS_OK, res);
    focus = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    res = sessionStage_->UpdateFocus(focus);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyDestroy
 * @tc.desc: test function : NotifyDestroy
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDestroy, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyDestroy();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyTransferComponentData
 * @tc.desc: test function : NotifyTransferComponentData
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTransferComponentData, Function | SmallTest | Level1)
{
    AAFwk::WantParams wantParams;
    std::string testValue = "testValue";
    wantParams.SetParam("keyStr", OHOS::AAFwk::String::Box(testValue));
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyTransferComponentData(wantParams);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfo
 * @tc.desc: test function : NotifyOccupiedAreaChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyOccupiedAreaChangeInfo, Function | SmallTest | Level1)
{
    sptr<OccupiedAreaChangeInfo> info = new OccupiedAreaChangeInfo();
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyOccupiedAreaChangeInfo(info);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: test function : UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateAvoidArea, Function | SmallTest | Level1)
{
    sptr<AvoidArea> avoidArea = new AvoidArea();
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateAvoidArea(avoidArea, type);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: test function : DumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, DumpSessionElementInfo, Function | SmallTest | Level1)
{
    std::vector<std::string> params;
    params.push_back("test1");
    params.push_back("test2");
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->DumpSessionElementInfo(params);
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: test function : NotifyScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyScreenshot, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyScreenshot();
}

/**
 * @tc.name: NotifyTouchOutside
 * @tc.desc: test function : NotifyTouchOutside
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTouchOutside, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyTouchOutside();
}

/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: test function : UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateWindowMode, Function | SmallTest | Level1)
{
    WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->UpdateWindowMode(mode);
}

/**
 * @tc.name: NotifySessionFullScreen
 * @tc.desc: test function : NotifySessionFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifySessionFullScreen, Function | SmallTest | Level1)
{
    bool fullScreen = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifySessionFullScreen(fullScreen);
    fullScreen = false;
    sessionStage_->NotifySessionFullScreen(fullScreen);
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: test function : NotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyForegroundInteractiveStatus, Function | SmallTest | Level1)
{
    bool interactive = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyForegroundInteractiveStatus(interactive);
    interactive = false;
    sessionStage_->NotifyForegroundInteractiveStatus(interactive);
}

/**
 * @tc.name: NotifySessionForeground
 * @tc.desc: test function : NotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifySessionForeground, Function | SmallTest | Level1)
{
    uint32_t reason = 1;
    bool withAnimation = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifySessionForeground(reason, withAnimation);
}

/**
 * @tc.name: NotifySessionBackground
 * @tc.desc: test function : NotifySessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifySessionBackground, Function | SmallTest | Level1)
{
    uint32_t reason = 1;
    bool withAnimation = true;
    bool isFromInnerkits = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
}

/**
 * @tc.name: NotifyWindowVisibility
 * @tc.desc: test function : NotifyWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyWindowVisibility, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyWindowVisibility(true);
}

/**
 * @tc.name: NotifyCompatibleModeEnableInPad
 * @tc.desc: test function : NotifyCompatibleModeEnableInPad
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyCompatibleModeEnableInPad, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyCompatibleModeEnableInPad(true);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyDensityFollowHost
 * @tc.desc: test function : NotifyDensityFollowHost
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDensityFollowHost, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    ASSERT_EQ(WSError::WS_OK, sessionStage_->NotifyDensityFollowHost(true, 1.0f));
}

/**
 * @tc.name: SetPipActionEvent
 * @tc.desc: test function : SetPipActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetPipActionEvent, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->SetPipActionEvent("close", 0);
}

/**
 * @tc.name: SetPiPControlEvent
 * @tc.desc: test function : SetPiPControlEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetPiPControlEvent, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    sessionStage_->SetPiPControlEvent(controlType, status);
}

/**
 * @tc.name: PcAppInPadNormalClose
 * @tc.desc: test function : PcAppInPadNormalClose
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, PcAppInPadNormalClose, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    auto res = sessionStage_->PcAppInPadNormalClose();
    ASSERT_NE(WSError::WS_ERROR_INVALID_WINDOW, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sessionStage_->PcAppInPadNormalClose();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);
    res = sessionStage->PcAppInPadNormalClose();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyDumpInfo
 * @tc.desc: test function : NotifyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDumpInfo, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    auto res = sessionStage_->NotifyDumpInfo(params, info);
    ASSERT_NE(WSError::WS_OK, res);

    MockMessageParcel::SetReadStringVectorErrorFlag(true);
    res = sessionStage_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sessionStage_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    sptr<SessionStageProxy> sessionStage = new SessionStageProxy(nullptr);
    res = sessionStage->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetEnableDragBySystem
 * @tc.desc: test function : SetEnableDragBySystem
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetEnableDragBySystem, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SetEnableDragBySystem(false);
    ASSERT_EQ(WSError::WS_OK, res);
}
}
}
}