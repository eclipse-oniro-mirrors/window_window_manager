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
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "session/host/include/main_session.h"
#include "session/host/include/session.h"
#include "session/host/include/sub_session.h"
#include "session/screen/include/screen_session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MainSessionLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
};

void MainSessionLayoutTest::SetUpTestCase() {}

void MainSessionLayoutTest::TearDownTestCase() {}

void MainSessionLayoutTest::SetUp() {}

void MainSessionLayoutTest::TearDown() {}

RSSurfaceNode::SharedPtr MainSessionLayoutTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: SetSubWindowBoundsDuringCross
 * @tc.desc: Check reason when setSubWindowBoundsDuringCross
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, SetSubWindowBoundsDuringCross, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "SetSubWindowBoundsDuringCross";
    info.moduleName_ = "SetSubWindowBoundsDuringCross";
    info.abilityName_ = "SetSubWindowBoundsDuringCross";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    mainSession->subSession_.emplace_back(nullptr);
    mainSession->subSession_.emplace_back(subSession);
    mainSession->GetLayoutController()->SetSessionRect({ 50, 50, 500, 500 });
    subSession->GetLayoutController()->SetSessionRect({ 0, 0, 200, 200 });

    subSession->state_ = SessionState::STATE_FOREGROUND;
    subSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    mainSession->SetSubWindowBoundsDuringCross(mainSession->GetSessionRect(), true, true);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::RESIZE);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    mainSession->SetSubWindowBoundsDuringCross(mainSession->GetSessionRect(), true, true);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::UNDEFINED);

    subSession->state_ = SessionState::STATE_BACKGROUND;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    mainSession->SetSubWindowBoundsDuringCross(mainSession->GetSessionRect(), true, true);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::RESIZE);
}

/**
 * @tc.name: NotifySubSessionRectChangeByAnchor
 * @tc.desc: Check reason when NotifySubSessionRectChangeByAnchor
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, NotifySubSessionRectChangeByAnchor, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "NotifySubSessionRectChangeByAnchor";
    info.moduleName_ = "NotifySubSessionRectChangeByAnchor";
    info.abilityName_ = "NotifySubSessionRectChangeByAnchor";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    mainSession->subSession_.emplace_back(nullptr);
    mainSession->subSession_.emplace_back(subSession);
    mainSession->GetLayoutController()->SetSessionRect({ 50, 50, 500, 500 });
    subSession->GetLayoutController()->SetSessionRect({ 0, 0, 200, 200 });
    const auto& func = [subSession](const WSRect& rect,
                                    SizeChangeReason reason,
                                    DisplayId displayId) {
        subSession->GetLayoutController()->SetSessionRect(rect);
        subSession->Session::UpdateSizeChangeReason(reason);
    };
    subSession->SetSessionRectChangeCallback(func);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    mainSession->NotifySubSessionRectChangeByAnchor(mainSession->GetSessionRect(), SizeChangeReason::RESIZE);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::UNDEFINED);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    mainSession->NotifySubSessionRectChangeByAnchor(mainSession->GetSessionRect(), SizeChangeReason::RESIZE);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::RESIZE);
}

/**
 * @tc.name: HandleSubSessionSurfaceNodeByWindowAnchor
 * @tc.desc: Check reason when HandleSubSessionSurfaceNodeByWindowAnchor
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, HandleSubSessionSurfaceNodeByWindowAnchor, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "HandleSubSessionSurfaceNodeByWindowAnchor";
    info.moduleName_ = "HandleSubSessionSurfaceNodeByWindowAnchor";
    info.abilityName_ = "HandleSubSessionSurfaceNodeByWindowAnchor";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    mainSession->subSession_.emplace_back(nullptr);
    mainSession->subSession_.emplace_back(subSession);
    auto surfaceNode = CreateRSSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    subSession->SetSurfaceNode(surfaceNode);
    subSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });

    subSession->state_ = SessionState::STATE_FOREGROUND;
    subSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    subSession->cloneNodeCountDuringCross_.store(0);
    mainSession->HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG, 0);
    EXPECT_EQ(subSession->cloneNodeCountDuringCross_, 0);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    mainSession->HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG, 0);
    EXPECT_EQ(subSession->cloneNodeCountDuringCross_, 1);

    subSession->state_ = SessionState::STATE_BACKGROUND;
    mainSession->HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG_END, 0);
    EXPECT_EQ(subSession->cloneNodeCountDuringCross_, 1);
}
} // namespace
} // namespace Rosen
} // namespace OHOS