/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "picture_in_picture_manager.h"
#include "scene_board_judgement.h"
#include "singleton_container.h"
#include "window_scene_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class PictureInPictureManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class MockWindow : public Window {
public:
    MockWindow() {};
    ~MockWindow() {};
};

void PictureInPictureManagerTest::SetUpTestCase() {}

void PictureInPictureManagerTest::TearDownTestCase() {}

void PictureInPictureManagerTest::SetUp() {}

void PictureInPictureManagerTest::TearDown() {}

namespace {

/**
 * @tc.name: IsSupportPiP
 * @tc.desc: IsSupportPiP
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, IsSupportPiP, TestSize.Level1)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(true, PictureInPictureManager::IsSupportPiP());
    } else {
        ASSERT_EQ(false, PictureInPictureManager::IsSupportPiP());
    }
}

/**
 * @tc.name: PiPWindowState
 * @tc.desc: PutPipControllerInfo/RemovePipControllerInfo/ReportPiPStartWindow/ReportPiPStopWindow/ReportPiPActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, PipControllerInfo, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipController = new PictureInPictureController(option, nullptr, 100, nullptr);
    PictureInPictureManager::PutPipControllerInfo(100, pipController);
    ASSERT_EQ(1, static_cast<int>(PictureInPictureManager::windowToControllerMap_.size()));
    PictureInPictureManager::RemovePipControllerInfo(100);
    ASSERT_EQ(0, static_cast<int>(PictureInPictureManager::windowToControllerMap_.size()));
    int32_t source = 0;
    std::string errorReason = "";
    SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(source, 1, 1, errorReason);
    SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(source, 1, 1, errorReason);
    source = 1;
    SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(source, 1, 1, errorReason);
    SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(source, 1, 1, errorReason);
    std::string actionEvent = "";
    SingletonContainer::Get<PiPReporter>().ReportPiPActionEvent(1, actionEvent);
    actionEvent = "nextVideo";
    SingletonContainer::Get<PiPReporter>().ReportPiPActionEvent(1, actionEvent);
}

/**
 * @tc.name: PictureInPictureController
 * @tc.desc: SetActiveController/IsActiveController/HasActiveController/RemoveActiveController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, PictureInPictureController, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipController = new PictureInPictureController(option, nullptr, 100, nullptr);
    PictureInPictureManager::activeController_ = nullptr;
    ASSERT_FALSE(PictureInPictureManager::HasActiveController());
    PictureInPictureManager::RemoveActiveController(pipController);
    ASSERT_FALSE(PictureInPictureManager::HasActiveController());
    ASSERT_FALSE(PictureInPictureManager::IsActiveController(pipController));

    PictureInPictureManager::SetActiveController(pipController);
    ASSERT_TRUE(PictureInPictureManager::HasActiveController());
    ASSERT_TRUE(PictureInPictureManager::IsActiveController(pipController));
    ASSERT_TRUE(PictureInPictureManager::IsAttachedToSameWindow(100));
    ASSERT_FALSE(PictureInPictureManager::IsAttachedToSameWindow(1));
    PictureInPictureManager::RemoveActiveController(pipController);
    ASSERT_FALSE(PictureInPictureManager::HasActiveController());
    ASSERT_FALSE(PictureInPictureManager::IsActiveController(pipController));
}

/**
 * @tc.name: ShouldAbortPipStart
 * @tc.desc: ShouldAbortPipStart
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, ShouldAbortPipStart, TestSize.Level1)
{
    ASSERT_FALSE(PictureInPictureManager::ShouldAbortPipStart());

    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::SetActiveController(pipController);
    pipController->curState_ = PiPWindowState::STATE_STARTING;

    ASSERT_TRUE(PictureInPictureManager::ShouldAbortPipStart());
}

/**
 * @tc.name: GetPipControllerInfo
 * @tc.desc: GetPipControllerInfo
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, GetPipControllerInfo, TestSize.Level1)
{
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::SetActiveController(pipController);
    int32_t windowId = 1;
    ASSERT_EQ(nullptr, PictureInPictureManager::GetPipControllerInfo(windowId));

    sptr<PictureInPictureControllerBase> pipController1 =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController1, nullptr);
    PictureInPictureManager::windowToControllerMap_.insert(std::make_pair(windowId, pipController1));
    ASSERT_EQ(pipController1, PictureInPictureManager::GetPipControllerInfo(windowId));
}

/**
 * @tc.name: GetActiveController
 * @tc.desc: GetActiveController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, GetActiveController, TestSize.Level1)
{
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureControllerBase> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::SetActiveController(pipController);
    ASSERT_EQ(pipController, PictureInPictureManager::GetActiveController());

    PictureInPictureManager::SetActiveController(nullptr);
    ASSERT_EQ(nullptr, PictureInPictureManager::GetActiveController());
}

/**
 * @tc.name: AttachAutoStartController
 * @tc.desc: AttachAutoStartController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, AttachAutoStartController, TestSize.Level1)
{
    PictureInPictureManager::AttachAutoStartController(0, nullptr);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::SetActiveController(pipController);
    wptr<PictureInPictureController> pipController1 =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController1, nullptr);
    PictureInPictureManager::AttachAutoStartController(1, pipController1);
    ASSERT_EQ(pipController1, PictureInPictureManager::autoStartControllerMap_[1]);
}

/**
 * @tc.name: DetachAutoStartController
 * @tc.desc: DetachAutoStartController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DetachAutoStartController, TestSize.Level1)
{
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    wptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    wptr<PictureInPictureController> pipController1 =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController1, nullptr);
    PictureInPictureManager::AttachAutoStartController(0, pipController);
    PictureInPictureManager::AttachAutoStartController(1, pipController1);
    ASSERT_EQ(2, PictureInPictureManager::autoStartControllerMap_.size());
    PictureInPictureManager::DetachAutoStartController(0, nullptr);
    ASSERT_EQ(1, PictureInPictureManager::autoStartControllerMap_.size());
    PictureInPictureManager::DetachAutoStartController(0, pipController);
    ASSERT_EQ(1, PictureInPictureManager::autoStartControllerMap_.size());
}

/**
 * @tc.name: IsAttachedToSameWindow
 * @tc.desc: IsAttachedToSameWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, IsAttachedToSameWindow, TestSize.Level1)
{
    bool res = PictureInPictureManager::IsAttachedToSameWindow(0);
    ASSERT_EQ(res, false);

    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);

    PictureInPictureManager::activeController_ = nullptr;
    ASSERT_FALSE(PictureInPictureManager::HasActiveController());
    bool res1 = PictureInPictureManager::IsAttachedToSameWindow(1);
    ASSERT_EQ(res1, false);

    PictureInPictureManager::SetActiveController(pipController);
    bool res2 = PictureInPictureManager::IsAttachedToSameWindow(100);
    ASSERT_EQ(res2, true);
    bool res3 = PictureInPictureManager::IsAttachedToSameWindow(1);
    ASSERT_EQ(res3, false);
}

/**
 * @tc.name: GetCurrentWindow
 * @tc.desc: GetCurrentWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, GetCurrentWindow, TestSize.Level1)
{
    ASSERT_EQ(nullptr, PictureInPictureManager::GetCurrentWindow());

    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);

    PictureInPictureManager::activeController_ = nullptr;
    ASSERT_FALSE(PictureInPictureManager::HasActiveController());
    ASSERT_EQ(nullptr, PictureInPictureManager::GetCurrentWindow());

    sptr<Window> window = nullptr;
    ASSERT_EQ(window, pipController->window_);
    PictureInPictureManager::SetActiveController(pipController);
    window = PictureInPictureManager::GetCurrentWindow();
    ASSERT_EQ(window, pipController->window_);
}

/**
 * @tc.name: DoPreRestore
 * @tc.desc: DoPreRestore
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoPreRestore, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::SetActiveController(pipController);
    PictureInPictureManager::DoPreRestore();
    ASSERT_EQ(pipController->curState_, PiPWindowState::STATE_RESTORING);
}

/**
 * @tc.name: DoPrepareSource
 * @tc.desc: DoPrepareSource
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoPrepareSource, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::activeController_ = nullptr;
    PictureInPictureManager::DoPrepareSource();
    PictureInPictureManager::SetActiveController(pipController);
    PictureInPictureManager::DoPrepareSource();
}

/**
 * @tc.name: DoLocateSource
 * @tc.desc: DoLocateSource
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoLocateSource, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::activeController_ = nullptr;
    PictureInPictureManager::DoLocateSource();
    PictureInPictureManager::SetActiveController(pipController);
    PictureInPictureManager::DoLocateSource();
}

/**
 * @tc.name: DoRestore
 * @tc.desc: DoRestore
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoRestore, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    pipController->curState_ = PiPWindowState::STATE_STARTED;
    PictureInPictureManager::activeController_ = nullptr;
    PictureInPictureManager::DoRestore();
    ASSERT_EQ(pipController->curState_, PiPWindowState::STATE_STARTED);
    pipController->window_ = mw;
    PictureInPictureManager::SetActiveController(pipController);
    PictureInPictureManager::DoRestore();
    ASSERT_EQ(pipController->curState_, PiPWindowState::STATE_STOPPING);
}

/**
 * @tc.name: DoClose
 * @tc.desc: DoClose
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoClose, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    pipController->curState_ = PiPWindowState::STATE_STARTED;
    PictureInPictureManager::activeController_ = nullptr;
    ASSERT_EQ(false, PictureInPictureManager::HasActiveController());
    PictureInPictureManager::SetActiveController(pipController);
    ASSERT_EQ(true, PictureInPictureManager::HasActiveController());
    PictureInPictureManager::DoClose(true, true);
    pipController->window_ = mw;
    PictureInPictureManager::DoClose(false, true);
    ASSERT_EQ(false, PictureInPictureManager::HasActiveController());
    ASSERT_EQ(pipController->curState_, PiPWindowState::STATE_STOPPED);
}

/**
 * @tc.name: DoActionEvent
 * @tc.desc: DoActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoActionEvent, TestSize.Level1)
{
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::activeController_ = nullptr;
    ASSERT_EQ(false, PictureInPictureManager::HasActiveController());
    std::string actionName = "test";
    PictureInPictureManager::DoActionEvent(actionName, 0);
    PictureInPictureManager::SetActiveController(pipController);
    ASSERT_EQ(true, PictureInPictureManager::HasActiveController());
    const std::string ACTION_CLOSE = "close";
    PictureInPictureManager::DoActionEvent(ACTION_CLOSE, 0);
}

/**
 * @tc.name: AutoStartPipWindow
 * @tc.desc: AutoStartPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, AutoStartPipWindow, TestSize.Level1)
{
    PictureInPictureManager::autoStartController_ = nullptr;
    PictureInPictureManager::AutoStartPipWindow();

    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipController =
        new (std::nothrow) PictureInPictureController(option, nullptr, 100, nullptr);
    PictureInPictureManager::autoStartController_ = pipController;
    option->SetTypeNodeEnabled(true);
    PictureInPictureManager::AutoStartPipWindow();
    option->SetTypeNodeEnabled(false);
    PictureInPictureManager::AutoStartPipWindow();
    SingletonContainer::Get<PiPReporter>().ReportPiPActionEvent(1, "close");
}

/**
 * @tc.name: DoDestroy
 * @tc.desc: DoDestroy
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoDestroy, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);

    PictureInPictureManager::activeController_ = pipController;
    pipController->window_ = mw;
    PictureInPictureManager::DoDestroy();
    ASSERT_EQ(pipController->curState_, PiPWindowState::STATE_STOPPED);
}

/**
 * @tc.name: DoActiveStatusChangeEvent
 * @tc.desc: DoActiveStatusChangeEvent
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoActiveStatusChangeEvent, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);

    PictureInPictureManager::activeController_ = pipController;
    pipController->window_ = mw;
    PictureInPictureManager::DoActiveStatusChangeEvent(true);
    ASSERT_EQ(pipController->curActiveStatus_, true);
}

/**
 * @tc.name: DoCloseWithReason
 * @tc.desc: DoCloseWithReason
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureManagerTest, DoCloseWithReason, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipController = sptr<PictureInPictureController>::MakeSptr(option, nullptr, 100, nullptr);
    ASSERT_NE(pipController, nullptr);
    PictureInPictureManager::activeController_ = nullptr;
    ASSERT_EQ(false, PictureInPictureManager::HasActiveController());
    PictureInPictureManager::DoCloseWithReason(true, true, PiPStateChangeReason::REQUEST_DELETE);
    PictureInPictureManager::SetActiveController(pipController);
    ASSERT_EQ(true, PictureInPictureManager::HasActiveController());
    PictureInPictureManager::DoCloseWithReason(true, true, PiPStateChangeReason::REQUEST_DELETE);
    PictureInPictureManager::DoActionCloseByRequest();
    PictureInPictureManager::DoActionCloseByPanel();
    PictureInPictureManager::DoActionCloseByDumpster();
}
} // namespace
} // namespace Rosen
} // namespace OHOS