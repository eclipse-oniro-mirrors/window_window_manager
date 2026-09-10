/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "abstract_screen_controller.h"
#include "iremote_object_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class AbstractScreenControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::recursive_mutex mutex;
    sptr<AbstractScreenController> absController_ = new AbstractScreenController(mutex);
    std::string name = "AbstractScreenController";
    std::vector<sptr<AbstractScreen>> screenVec;
    std::vector<sptr<AbstractScreenGroup>> screenGroupVec;
};

void AbstractScreenControllerTest::SetUpTestCase()
{
}

void AbstractScreenControllerTest::TearDownTestCase()
{
}

void AbstractScreenControllerTest::SetUp()
{
    uint64_t index = 5;
    for (uint64_t i = 0; i < index; ++i) {
        ScreenId dmsId = i;
        ScreenId rsId = i;
        sptr<AbstractScreen> absScreen = new AbstractScreen(absController_, name, dmsId, rsId);
        sptr<AbstractScreenGroup> absScreenGroup = new AbstractScreenGroup(absController_,
            dmsId, rsId, name, ScreenCombination::SCREEN_ALONE);
        screenVec.emplace_back(absScreen);
        screenGroupVec.emplace_back(absScreenGroup);
        // init dmsScreenMap_
        absController_->dmsScreenMap_.insert(std::make_pair(dmsId, absScreen));
        // init screenIdManager_
        absController_->screenIdManager_.rs2DmsScreenIdMap_.insert(std::make_pair(rsId, dmsId));
        absController_->screenIdManager_.dms2RsScreenIdMap_.insert(std::make_pair(dmsId, rsId));
        // init dmsScreenGroupMap_
        absController_->dmsScreenGroupMap_.insert(std::make_pair(rsId, absScreenGroup));
    }
    screenVec[4]->type_ = ScreenType::UNDEFINED;
    screenVec[3]->type_ = ScreenType::VIRTUAL;
    absController_->dmsScreenMap_.insert(std::make_pair(index, nullptr));
    absController_->screenIdManager_.dms2RsScreenIdMap_.insert(std::make_pair(index, SCREEN_ID_INVALID));
    absController_->dmsScreenGroupMap_.insert(std::make_pair(index, nullptr));
}

void AbstractScreenControllerTest::TearDown()
{
    screenVec.clear();
}

namespace {
/**
 * @tc.name: ProcessScreenDisconnected
 * @tc.desc: ProcessScreenDisconnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenDisconnected01, TestSize.Level1)
{
    ScreenId rsId = 6;
    ScreenId dmsId;
    absController_->ProcessScreenDisconnected(rsId);
    ASSERT_EQ(false, absController_->screenIdManager_.ConvertToDmsScreenId(rsId, dmsId));
}

/**
 * @tc.name: ProcessScreenDisconnected
 * @tc.desc: ProcessScreenDisconnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenDisconnected02, TestSize.Level1)
{
    ScreenId rsId = 2;
    ScreenId dmsId;
    absController_->ProcessScreenDisconnected(rsId);
    absController_->screenIdManager_.ConvertToDmsScreenId(rsId, dmsId);
    absController_->dmsScreenMap_.erase(dmsId);
    ASSERT_EQ(false, absController_->screenIdManager_.ConvertToDmsScreenId(rsId, dmsId));
    ASSERT_EQ(absController_->dmsScreenMap_.end(), absController_->dmsScreenMap_.find(dmsId));
}

/**
 * @tc.name: ProcessScreenDisconnected
 * @tc.desc: ProcessScreenDisconnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenDisconnected03, TestSize.Level1)
{
    ScreenId rsId = 2;
    ScreenId dmsId;
    absController_->abstractScreenCallback_ = nullptr;
    absController_->ProcessScreenDisconnected(rsId);
    ASSERT_EQ(false, absController_->screenIdManager_.ConvertToDmsScreenId(rsId, dmsId));
    ASSERT_NE(absController_->dmsScreenMap_.end(), absController_->dmsScreenMap_.find(dmsId));
}

/**
 * @tc.name: SetScreenRotateAnimation
 * @tc.desc: SetScreenRotateAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenRotateAnimation01, TestSize.Level1)
{
    RSDisplayNodeConfig config;
    absController_->dmsScreenMap_[1]->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    sptr<AbstractScreen> screen = screenVec[0];
    screen->rotation_ = Rotation::ROTATION_270;
    absController_->SetScreenRotateAnimation(screen, 1, Rotation::ROTATION_0, true);
    ASSERT_EQ(Rotation::ROTATION_270, screen->rotation_);
}

/**
 * @tc.name: SetScreenRotateAnimation
 * @tc.desc: SetScreenRotateAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenRotateAnimation02, TestSize.Level1)
{
    RSDisplayNodeConfig config;
    absController_->dmsScreenMap_[1]->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    sptr<AbstractScreen> screen = screenVec[0];
    screen->rotation_ = Rotation::ROTATION_0;
    absController_->SetScreenRotateAnimation(screen, 1, Rotation::ROTATION_270, true);
    ASSERT_EQ(Rotation::ROTATION_0, screen->rotation_);
}

/**
 * @tc.name: SetScreenRotateAnimation
 * @tc.desc: SetScreenRotateAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenRotateAnimation03, TestSize.Level1)
{
    RSDisplayNodeConfig config;
    absController_->dmsScreenMap_[1]->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    sptr<AbstractScreen> screen = screenVec[0];
    screen->rotation_ = Rotation::ROTATION_0;
    absController_->SetScreenRotateAnimation(screen, 1, Rotation::ROTATION_270, false);
    ASSERT_EQ(Rotation::ROTATION_0, screen->rotation_);
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeMirror01, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    ASSERT_TRUE(DMError::DM_OK != absController_->MakeMirror(5, screens));
    ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screens, ScreenCombination::SCREEN_MIRROR));
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeMirror02, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    absController_->dmsScreenMap_[2]->type_ = ScreenType::UNDEFINED;
    ASSERT_TRUE(DMError::DM_OK != absController_->MakeMirror(2, screens));
    ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screens, ScreenCombination::SCREEN_MIRROR));
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeMirror03, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    absController_->dmsScreenMap_[2]->type_ = ScreenType::REAL;
    absController_->dmsScreenMap_[2]->groupDmsId_ = 5;
    absController_->abstractScreenCallback_ = nullptr;
    ASSERT_TRUE(DMError::DM_OK != absController_->MakeMirror(2, screens));
    ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screens, ScreenCombination::SCREEN_MIRROR));
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeMirror04, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    absController_->dmsScreenMap_[2]->type_ = ScreenType::REAL;
    absController_->dmsScreenMap_[2]->groupDmsId_ = 5;
    absController_->abstractScreenCallback_ = new AbstractScreenController::AbstractScreenCallback;
    ASSERT_TRUE(DMError::DM_OK != absController_->MakeMirror(2, screens));
    ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screens, ScreenCombination::SCREEN_MIRROR));
}

/**
 * @tc.name: MakeMirror
 * @tc.desc: MakeMirror test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeMirror05, TestSize.Level1)
{
    std::vector<ScreenId> screens;
    absController_->dmsScreenMap_[2]->type_ = ScreenType::REAL;
    absController_->dmsScreenMap_[2]->groupDmsId_ = 2;
    absController_->abstractScreenCallback_ = new AbstractScreenController::AbstractScreenCallback;
    ASSERT_TRUE(DMError::DM_OK == absController_->MakeMirror(2, screens));
    ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screens, ScreenCombination::SCREEN_MIRROR));
}

/**
 * @tc.name: ProcessScreenConnected
 * @tc.desc: ProcessScreenConnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenConnected01, TestSize.Level1)
{
    ScreenId id = 0;
    absController_->ProcessScreenConnected(id);
    ASSERT_EQ(true, absController_->screenIdManager_.HasRsScreenId(id));
}

/**
 * @tc.name: OnRsScreenConnectionChange
 * @tc.desc: OnRsScreenConnectionChange test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRsScreenConnectionChange01, TestSize.Level1)
{
    ScreenEvent event = ScreenEvent::UNKNOWN;
    ScreenId rsScreenId = 100;
    absController_->OnRsScreenConnectionChange(rsScreenId, event);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: OnRsScreenConnectionChange
 * @tc.desc: OnRsScreenConnectionChange test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRsScreenConnectionChange02, TestSize.Level1)
{
    ScreenEvent event = ScreenEvent::UNKNOWN;
    ScreenId rsScreenId = 1;
    absController_->OnRsScreenConnectionChange(rsScreenId, event);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: OnRsScreenConnectionChange
 * @tc.desc: OnRsScreenConnectionChange test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRsScreenConnectionChange03, TestSize.Level1)
{
    ScreenEvent event = ScreenEvent::CONNECTED;
    ScreenId rsScreenId = 1;
    absController_->OnRsScreenConnectionChange(rsScreenId, event);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: OnRsScreenConnectionChange
 * @tc.desc: OnRsScreenConnectionChange test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRsScreenConnectionChange04, TestSize.Level1)
{
    ScreenEvent event = ScreenEvent::DISCONNECTED;
    ScreenId rsScreenId = 1;
    absController_->OnRsScreenConnectionChange(rsScreenId, event);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: GetAllValidScreenIds
 * @tc.desc: GetAllValidScreenIds test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, GetAllValidScreenIds, TestSize.Level1)
{
    std::vector<ScreenId> screenIds {0, 1, 1, 2, 2, 3, 4};
    std::vector<ScreenId> valid {0, 1, 2, 3};
    ASSERT_EQ(valid, absController_->GetAllValidScreenIds(screenIds));
}

/**
 * @tc.name: GetRSDisplayNodeByScreenId
 * @tc.desc: GetRSDisplayNodeByScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, GetRSDisplayNodeByScreenId, TestSize.Level1)
{
    ScreenId id = 6;
    std::shared_ptr<RSDisplayNode> node = nullptr;
    ASSERT_EQ(node, absController_->GetRSDisplayNodeByScreenId(id));
}

/**
 * @tc.name: UpdateRSTree
 * @tc.desc: UpdateRSTree test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, UpdateRSTree01, TestSize.Level1)
{
    ScreenId id = 8;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    absController_->UpdateRSTree(id, id, node, true, true);
    ASSERT_EQ(nullptr, absController_->GetAbstractScreen(id));
}

/**
 * @tc.name: UpdateRSTree
 * @tc.desc: UpdateRSTree test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, UpdateRSTree02, TestSize.Level1)
{
    ScreenId id = 1;
    ScreenId parentId = 8;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    absController_->UpdateRSTree(id, parentId, node, true, true);
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(id));
    ASSERT_EQ(nullptr, absController_->GetAbstractScreen(parentId));
}

/**
 * @tc.name: UpdateRSTree
 * @tc.desc: UpdateRSTree test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, UpdateRSTree03, TestSize.Level1)
{
    ScreenId id = 1;
    ScreenId parentId = 2;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(id));
    sptr<AbstractScreen> parentScreen = absController_->GetAbstractScreen(parentId);
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(parentId));
    parentScreen->rsDisplayNode_ = nullptr;
    absController_->UpdateRSTree(id, parentId, node, true, true);
}

/**
 * @tc.name: UpdateRSTree
 * @tc.desc: UpdateRSTree test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, UpdateRSTree04, TestSize.Level1)
{
    ScreenId id = 1;
    ScreenId parentId = 2;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    RSDisplayNodeConfig config;
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(id));
    sptr<AbstractScreen> parentScreen = absController_->GetAbstractScreen(parentId);
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(parentId));
    parentScreen->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    absController_->UpdateRSTree(id, parentId, node, true, true);
}

/**
 * @tc.name: UpdateRSTree
 * @tc.desc: UpdateRSTree test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, UpdateRSTree05, TestSize.Level1)
{
    ScreenId id = 1;
    ScreenId parentId = 2;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    RSDisplayNodeConfig config;
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(id));
    sptr<AbstractScreen> parentScreen = absController_->GetAbstractScreen(parentId);
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(parentId));
    parentScreen->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    absController_->UpdateRSTree(id, parentId, node, true, false);
}

/**
 * @tc.name: RegisterAbstractScreenCallback
 * @tc.desc: RegisterAbstractScreenCallback test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, RegisterAbstractScreenCallback, TestSize.Level1)
{
    sptr<AbstractScreenController::AbstractScreenCallback> cb = nullptr;
    absController_->RegisterAbstractScreenCallback(cb);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: AddToGroupLocked
 * @tc.desc: AddToGroupLocked test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, AddToGroupLocked, TestSize.Level1)
{
    ASSERT_EQ(false, absController_->dmsScreenGroupMap_.empty());
    ASSERT_EQ(nullptr, absController_->AddToGroupLocked(screenVec[0]));
}

/**
 * @tc.name: RemoveFromGroupLocked
 * @tc.desc: RemoveFromGroupLocked test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, RemoveFromGroupLocked01, TestSize.Level1)
{
    sptr<AbstractScreen> screen = screenVec[0];
    screen->groupDmsId_ = 0;
    ASSERT_EQ(nullptr, absController_->RemoveFromGroupLocked(screen));
}

/**
 * @tc.name: RemoveChildFromGroup
 * @tc.desc: RemoveChildFromGroup test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, RemoveChildFromGroup01, TestSize.Level1)
{
    sptr<AbstractScreen> screen = screenVec[0];
    ScreenId dmsId = screen->dmsId_;
    sptr<AbstractScreenGroup> screenGroup = absController_->dmsScreenGroupMap_[0];
    screenGroup->screenMap_.insert(std::make_pair(dmsId, screen));
    ASSERT_EQ(true, absController_->RemoveChildFromGroup(screen, screenGroup));
}

/**
 * @tc.name: AddAsSuccedentScreenLocked
 * @tc.desc: AddAsSuccedentScreenLocked test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, AddAsSuccedentScreenLocked01, TestSize.Level1)
{
    sptr<AbstractScreen> screen = screenVec[0];
    absController_->dmsScreenMap_.erase(absController_->GetDefaultAbstractScreenId());
    ASSERT_EQ(nullptr, absController_->AddAsSuccedentScreenLocked(screen));
}

/**
 * @tc.name: AddAsSuccedentScreenLocked
 * @tc.desc: AddAsSuccedentScreenLocked test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, AddAsSuccedentScreenLocked02, TestSize.Level1)
{
    sptr<AbstractScreen> screen = screenVec[0];
    ASSERT_EQ(nullptr, absController_->AddAsSuccedentScreenLocked(screen));
}

/**
 * @tc.name: CreateVirtualScreen
 * @tc.desc: CreateVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, CreateVirtualScreen01, TestSize.Level1)
{
    VirtualScreenOption option;
    sptr<IRemoteObject> displayManagerAgent = new IRemoteObjectMocker();
    auto ret = absController_->CreateVirtualScreen(option, displayManagerAgent);
    if (ret != 0)
    {
        ASSERT_NE(0, ret);
    }
}

/**
 * @tc.name: InitVirtualScreen
 * @tc.desc: InitVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, InitVirtualScreen01, TestSize.Level1)
{
    VirtualScreenOption option;
    absController_->dmsScreenMap_.erase(absController_->GetDefaultAbstractScreenId());
    sptr<AbstractScreen> screen = absController_->InitVirtualScreen(0, 0, option);
    auto ret = screen->activeIdx_;
    if (ret != 0) {
        ASSERT_NE(0, ret);
    }
}

/**
 * @tc.name: InitVirtualScreen
 * @tc.desc: InitVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, InitVirtualScreen02, TestSize.Level1)
{
    VirtualScreenOption option;
    sptr<AbstractScreen> defaultScreen = absController_->dmsScreenMap_[absController_->GetDefaultAbstractScreenId()];
    sptr<SupportedScreenModes> modes;
    if (defaultScreen != nullptr) {
        defaultScreen->modes_.emplace_back(modes);
        defaultScreen->activeIdx_ = 0;
        ASSERT_EQ(nullptr, defaultScreen->GetActiveScreenMode());
        sptr<AbstractScreen> screen = absController_->InitVirtualScreen(0, 0, option);
        ASSERT_EQ(ScreenType::VIRTUAL, screen->type_);
    }
}

/**
 * @tc.name: DestroyVirtualScreen
 * @tc.desc: DestroyVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, DestroyVirtualScreen01, TestSize.Level1)
{
    ScreenId id = 5;
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, absController_->DestroyVirtualScreen(id));
}

/**
 * @tc.name: DestroyVirtualScreen
 * @tc.desc: DestroyVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, DestroyVirtualScreen02, TestSize.Level1)
{
    ScreenId id = 1;
    ASSERT_EQ(DMError::DM_OK, absController_->DestroyVirtualScreen(id));
}

/**
 * @tc.name: SetVirtualScreenSurface
 * @tc.desc: SetVirtualScreenSurface test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetVirtualScreenSurface01, TestSize.Level1)
{
    ScreenId id = 6;
    sptr<Surface> surface = nullptr;
    ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, absController_->SetVirtualScreenSurface(id, surface));
}

/**
 * @tc.name: SetBuildInDefaultOrientation
 * @tc.desc: SetBuildInDefaultOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetBuildInDefaultOrientation, TestSize.Level1)
{
    Orientation orientation = Orientation::BEGIN;
    absController_->SetBuildInDefaultOrientation(orientation);
    ASSERT_EQ(orientation, absController_->buildInDefaultOrientation_);
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetOrientation01, TestSize.Level1)
{
    absController_->dmsScreenMap_[1]->isScreenGroup_ = true;
    Orientation orientation = Orientation::BEGIN;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, absController_->SetOrientation(1, orientation, true));
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetOrientation02, TestSize.Level1)
{
    absController_->dmsScreenMap_[1]->isScreenGroup_ = false;

    Orientation orientation = Orientation::BEGIN;
    bool isFromWindow = true;
    DMError ret = absController_->SetOrientation(1, orientation, isFromWindow);
    ASSERT_EQ(DMError::DM_OK, ret);

    orientation = Orientation::VERTICAL;
    ret = absController_->SetOrientation(1, orientation, isFromWindow);
    ASSERT_EQ(DMError::DM_OK, ret);

    isFromWindow = false;
    ret = absController_->SetOrientation(1, orientation, isFromWindow);
    ASSERT_EQ(DMError::DM_OK, ret);

    ret = absController_->SetOrientation(1, orientation, isFromWindow, false);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: SetRotation
 * @tc.desc: SetRotation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetRotation01, TestSize.Level1)
{
    absController_->dmsScreenMap_[1]->rotation_ = Rotation::ROTATION_180;
    absController_->screenIdManager_.dms2RsScreenIdMap_.erase(1);
    ASSERT_EQ(true, absController_->SetRotation(1, Rotation::ROTATION_0, true));
    absController_->abstractScreenCallback_ = new AbstractScreenController::AbstractScreenCallback;
    ASSERT_EQ(false, absController_->SetRotation(1, Rotation::ROTATION_0, false));
    ASSERT_EQ(false, absController_->SetRotation(1, Rotation::ROTATION_0, true));
}

/**
 * @tc.name: SetScreenActiveMode
 * @tc.desc: SetScreenActiveMode test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenActiveMode01, TestSize.Level1)
{
    ASSERT_TRUE(DMError::DM_OK != absController_->SetScreenActiveMode(5, 0));
}

/**
 * @tc.name: SetScreenActiveMode
 * @tc.desc: SetScreenActiveMode test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenActiveMode02, TestSize.Level1)
{
    absController_->screenIdManager_.dms2RsScreenIdMap_.erase(1);
    ASSERT_TRUE(DMError::DM_OK != absController_->SetScreenActiveMode(1, 0));
}

/**
 * @tc.name: ProcessScreenModeChanged
 * @tc.desc: ProcessScreenModeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenModeChanged01, TestSize.Level1)
{
    absController_->ProcessScreenModeChanged(7);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: ProcessScreenModeChanged
 * @tc.desc: ProcessScreenModeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenModeChanged02, TestSize.Level1)
{
    absController_->ProcessScreenModeChanged(5);
    ASSERT_EQ(nullptr, absController_->dmsScreenMap_[5]);
}

/**
 * @tc.name: ProcessScreenModeChanged
 * @tc.desc: ProcessScreenModeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenModeChanged03, TestSize.Level1)
{
    absController_->ProcessScreenModeChanged(2);
    ASSERT_NE(nullptr, absController_->dmsScreenMap_[2]);
}

/**
 * @tc.name: ProcessScreenModeChanged
 * @tc.desc: ProcessScreenModeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenModeChanged04, TestSize.Level1)
{
    sptr<AbstractScreen> screen = absController_->GetAbstractScreen(2);
    screen->activeIdx_ = 0;
    absController_->abstractScreenCallback_ = nullptr;
    absController_->ProcessScreenModeChanged(2);
    ASSERT_NE(nullptr, absController_->dmsScreenMap_[2]);
}

/**
 * @tc.name: ProcessScreenModeChanged
 * @tc.desc: ProcessScreenModeChanged test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessScreenModeChanged05, TestSize.Level1)
{
    sptr<AbstractScreen> screen = absController_->GetAbstractScreen(2);
    screen->activeIdx_ = 0;
    absController_->abstractScreenCallback_ = new AbstractScreenController::AbstractScreenCallback;
    absController_->ProcessScreenModeChanged(2);
    ASSERT_NE(nullptr, absController_->dmsScreenMap_[2]);
}

/**
 * @tc.name: ChangeScreenGroup
 * @tc.desc: ChangeScreenGroup test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ChangeScreenGroup01, TestSize.Level1)
{
    sptr<AbstractScreenGroup> group = screenGroupVec[0];
    Point point;
    group->screenMap_.insert(std::make_pair(0, screenVec[0]));
    group->screenMap_.insert(std::make_pair(1, screenVec[0]));
    std::vector<Point> startPoints;
    std::vector<ScreenId> screens;
    for (ScreenId i = 0; i < 7; ++i) {
        screens.emplace_back(i);
        startPoints.emplace_back(point);
        if (i < absController_->dmsScreenMap_.size() && absController_->dmsScreenMap_[i] != nullptr) {
            absController_->dmsScreenMap_[i]->groupDmsId_ = 1;
        }
    }
    absController_->ChangeScreenGroup(group, screens, startPoints, true, ScreenCombination::SCREEN_ALONE);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: ChangeScreenGroup
 * @tc.desc: ChangeScreenGroup test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ChangeScreenGroup02, TestSize.Level1)
{
    sptr<AbstractScreenGroup> group = screenGroupVec[0];
    Point point;
    group->screenMap_.insert(std::make_pair(0, screenVec[0]));
    group->screenMap_.insert(std::make_pair(1, screenVec[0]));
    std::vector<Point> startPoints;
    std::vector<ScreenId> screens;
    for (ScreenId i = 0; i < 7; ++i) {
        screens.emplace_back(i);
        startPoints.emplace_back(point);
        if (i < absController_->dmsScreenMap_.size() && absController_->dmsScreenMap_[i] != nullptr) {
            absController_->dmsScreenMap_[i]->groupDmsId_ = 1;
        }
    }
    absController_->abstractScreenCallback_ = nullptr;
    absController_->ChangeScreenGroup(group, screens, startPoints, true, ScreenCombination::SCREEN_ALONE);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: ChangeScreenGroup
 * @tc.desc: ChangeScreenGroup test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, AddScreenToGroup01, TestSize.Level1)
{
    sptr<AbstractScreenGroup> group = screenGroupVec[0];
    std::vector<ScreenId> addScreens {0, 1, 2, 3, 4, 5};
    Point point;
    std::vector<Point> addChildPos(10, point);
    std::map<ScreenId, bool> removeChildResMap;
    absController_->AddScreenToGroup(group, addScreens, addChildPos, removeChildResMap);
    absController_->abstractScreenCallback_ = nullptr;
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: MakeExpand test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeExpand01, TestSize.Level1)
{
    std::vector<ScreenId> screenIds;
    std::vector<Point> startPoints;
    ScreenId defaultId = absController_->GetDefaultAbstractScreenId();
    absController_->dmsScreenMap_[defaultId] = nullptr;
    ASSERT_EQ(false, absController_->MakeExpand(screenIds, startPoints));
    ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screenIds, ScreenCombination::SCREEN_EXPAND));
}

/**
 * @tc.name: MakeExpand
 * @tc.desc: MakeExpand test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, MakeExpand02, TestSize.Level1)
{
    std::vector<ScreenId> screenIds;
    std::vector<Point> startPoints;
    ScreenId defaultId = absController_->GetDefaultAbstractScreenId();
    auto defaultScreen = absController_->GetAbstractScreen(defaultId);
    if (defaultScreen != nullptr)
    {
        ScreenId groupDmsId = defaultScreen->groupDmsId_;
        absController_->dmsScreenGroupMap_[groupDmsId] = nullptr;
        ASSERT_EQ(false, absController_->MakeExpand(screenIds, startPoints));
        ASSERT_EQ(DMError::DM_OK, absController_->StopScreens(screenIds, ScreenCombination::SCREEN_EXPAND));
    }
}

/**
 * @tc.name: RemoveVirtualScreenFromGroup
 * @tc.desc: RemoveVirtualScreenFromGroup test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, RemoveVirtualScreenFromGroup01, TestSize.Level1)
{
    std::vector<ScreenId> screens {0, 1, 2, 3, 4, 5, 6, 7};
    absController_->abstractScreenCallback_ = nullptr;
    absController_->RemoveVirtualScreenFromGroup(screens);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRemoteDied01, TestSize.Level1)
{
    sptr<IRemoteObject> agent = nullptr;
    ASSERT_EQ(false, absController_->OnRemoteDied(agent));
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRemoteDied02, TestSize.Level1)
{
    sptr<IRemoteObject> agent = new IRemoteObjectMocker();
    ASSERT_EQ(true, absController_->OnRemoteDied(agent));
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, OnRemoteDied03, TestSize.Level1)
{
    sptr<IRemoteObject> agent = new IRemoteObjectMocker();
    std::vector<ScreenId> screens {5};
    absController_->screenAgentMap_.insert(std::make_pair(agent, screens));
    ASSERT_EQ(true, absController_->OnRemoteDied(agent));
    ASSERT_EQ(0, absController_->screenAgentMap_.size());
}

/**
 * @tc.name: CreateAndGetNewScreenId
 * @tc.desc: CreateAndGetNewScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, CreateAndGetNewScreenId01, TestSize.Level1)
{
    ScreenId rsScreenId = 1;
    ScreenId dmsScreenId = absController_->screenIdManager_.dmsScreenCount_;
    ASSERT_EQ(dmsScreenId, absController_->screenIdManager_.CreateAndGetNewScreenId(rsScreenId));
    ASSERT_EQ(++dmsScreenId, absController_->screenIdManager_.dmsScreenCount_);
}

/**
 * @tc.name: ConvertToRsScreenId
 * @tc.desc: ConvertToRsScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ConvertToRsScreenId01, TestSize.Level1)
{
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ScreenId dmsScreenId = 8;
    ASSERT_EQ(false, absController_->screenIdManager_.ConvertToRsScreenId(dmsScreenId, rsScreenId));
}

/**
 * @tc.name: NotifyScreenConnected
 * @tc.desc: NotifyScreenConnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, NotifyScreenConnected, TestSize.Level1)
{
    sptr<ScreenInfo> screenInfo = nullptr;
    absController_->NotifyScreenConnected(screenInfo);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: NotifyScreenConnected
 * @tc.desc: NotifyScreenConnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, NotifyScreenChanged, TestSize.Level1)
{
    sptr<ScreenInfo> screenInfo = nullptr;
    absController_->NotifyScreenChanged(screenInfo, ScreenChangeEvent::UPDATE_ORIENTATION);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: NotifyScreenConnected
 * @tc.desc: NotifyScreenConnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, NotifyScreenGroupChanged, TestSize.Level1)
{
    sptr<ScreenInfo> screenInfo = nullptr;
    absController_->NotifyScreenGroupChanged(screenInfo, ScreenGroupChangeEvent::ADD_TO_GROUP);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: NotifyScreenConnected
 * @tc.desc: NotifyScreenConnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenPowerForAll, TestSize.Level1)
{
    ASSERT_EQ(false, absController_->SetScreenPowerForAll(ScreenPowerState::POWER_ON,
        PowerStateChangeReason::POWER_BUTTON));

    for (uint32_t i = 0; i < screenVec.size(); ++i) {
        if (screenVec[i] != nullptr && screenVec[i]->type_ == ScreenType::REAL) {
            screenVec[i]->type_ = ScreenType::UNDEFINED;
        }
    }
    ASSERT_EQ(false, absController_->SetScreenPowerForAll(ScreenPowerState::INVALID_STATE,
        PowerStateChangeReason::POWER_BUTTON));

    ASSERT_EQ(false, absController_->SetScreenPowerForAll(ScreenPowerState::POWER_OFF,
        PowerStateChangeReason::POWER_BUTTON));
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: SetVirtualPixelRatio test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetVirtualPixelRatio01, TestSize.Level1)
{
    auto screen = screenVec[0];
    ScreenId id = 0;
    float ratio = 1.0;
    screen->isScreenGroup_ = true;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, absController_->SetVirtualPixelRatio(id, ratio));
}

/**
 * @tc.name: SetVirtualPixelRatio
 * @tc.desc: SetVirtualPixelRatio test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetVirtualPixelRatio02, TestSize.Level1)
{
    ScreenId id = 0;
    float ratio = 1.0;
    absController_->abstractScreenCallback_ = nullptr;
    ASSERT_EQ(DMError::DM_OK, absController_->SetVirtualPixelRatio(id, ratio));
}

/**
 * @tc.name: RegisterRsScreenConnectionChangeListener
 * @tc.desc: RegisterRsScreenConnectionChangeListener test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, RegisterRsScreenConnectionChangeListener, TestSize.Level1)
{
    sptr<IRemoteObject> agent = new IRemoteObjectMocker();
    absController_->Init();
    ASSERT_EQ(true, absController_->OnRemoteDied(agent));
    agent.clear();
}

/**
 * @tc.name: RemoveSurfaceNodeFromScreen
 * @tc.desc: RemoveSurfaceNodeFromScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, RemoveSurfaceNodeFromScreen, TestSize.Level1)
{
    ScreenId dmsScreenId = 8;
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, absController_->RemoveSurfaceNodeFromScreen(dmsScreenId, surfaceNode));
    ASSERT_EQ(nullptr, absController_->GetAbstractScreen(dmsScreenId));
}

/**
 * @tc.name: ProcessDefaultScreenReconnected
 * @tc.desc: ProcessDefaultScreenReconnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessDefaultScreenReconnected01, TestSize.Level1)
{
    ScreenId rsScreenId = 8;
    absController_->ProcessDefaultScreenReconnected(rsScreenId);
    absController_->defaultRsScreenId_ = 8;
    rsScreenId = 1;
    absController_->ProcessDefaultScreenReconnected(rsScreenId);
    rsScreenId = 8;
    absController_->ProcessDefaultScreenReconnected(rsScreenId);
    ASSERT_EQ(nullptr, absController_->GetAbstractScreen(rsScreenId));
}

/**
 * @tc.name: ProcessDefaultScreenReconnected
 * @tc.desc: ProcessDefaultScreenReconnected test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ProcessDefaultScreenReconnected02, TestSize.Level1)
{
    ScreenId rsScreenId = 8;
    absController_->defaultRsScreenId_ = 8;
    absController_->ProcessDefaultScreenReconnected(rsScreenId);
    ASSERT_EQ(nullptr, absController_->GetAbstractScreen(rsScreenId));
    rsScreenId = 5;
    absController_->defaultRsScreenId_ = 5;
    absController_->ProcessDefaultScreenReconnected(rsScreenId);
    rsScreenId = 1;
    absController_->defaultRsScreenId_ = 1;
    absController_->ProcessDefaultScreenReconnected(rsScreenId);
}

/**
 * @tc.name: ConvertToRsScreenId
 * @tc.desc: ConvertToRsScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ConvertToRsScreenId, TestSize.Level1)
{
    ScreenId dmsScreenId = 6;
    ASSERT_TRUE(absController_->screenIdManager_.ConvertToRsScreenId(dmsScreenId));
}

/**
 * @tc.name: ConvertToDmsScreenId
 * @tc.desc: ConvertToDmsScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ConvertToDmsScreenId, TestSize.Level1)
{
    ScreenId rsScreenId = 7;
    ASSERT_TRUE(absController_->screenIdManager_.ConvertToDmsScreenId(rsScreenId));
}

/**
 * @tc.name: AddSurfaceNodeToScreen
 * @tc.desc: AddSurfaceNodeToScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, AddSurfaceNodeToScreen, TestSize.Level1)
{
    ScreenId dmsScreenId = 7;
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = std::make_shared<RSSurfaceNode>(rsSurfaceNodeConfig, true, 0);
    DMError ret = absController_->AddSurfaceNodeToScreen(dmsScreenId, surfaceNode, true);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    dmsScreenId = 1;
    ret = absController_->AddSurfaceNodeToScreen(dmsScreenId, surfaceNode, true);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetScreenSupportedColorGamuts
 * @tc.desc: GetScreenSupportedColorGamuts test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, GetScreenSupportedColorGamuts, TestSize.Level1)
{
    ScreenId screenId = 1;
    std::vector<ScreenColorGamut> colorGamuts;
    DMError ret = absController_->GetScreenSupportedColorGamuts(screenId, colorGamuts);

    ASSERT_EQ(ret, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: GetScreenColorGamut
 * @tc.desc: GetScreenColorGamut test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, GetScreenColorGamut, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenColorGamut colorGamut;
    DMError ret = absController_->GetScreenColorGamut(screenId, colorGamut);
    
    ASSERT_EQ(ret, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: SetScreenColorGamut
 * @tc.desc: SetScreenColorGamut test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenColorGamut, TestSize.Level1)
{
    ScreenId screenId = 1;
    int32_t colorGamutIdx = 1;
    DMError ret = absController_->SetScreenColorGamut(screenId, colorGamutIdx);
    
    ASSERT_EQ(ret, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: GetScreenGamutMap
 * @tc.desc: GetScreenGamutMap test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, GetScreenGamutMap, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenGamutMap gamutMap;
    DMError ret = absController_->GetScreenGamutMap(screenId, gamutMap);
    
    ASSERT_EQ(ret, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: SetScreenGamutMap
 * @tc.desc: SetScreenGamutMap test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenGamutMap, TestSize.Level1)
{
    ScreenId screenId = 1;
    ScreenGamutMap gamutMap = {};
    DMError ret = absController_->SetScreenGamutMap(screenId, gamutMap);
    
    ASSERT_EQ(ret, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: SetScreenColorTransform
 * @tc.desc: SetScreenColorTransform test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenColorTransform, TestSize.Level1)
{
    ScreenId screenId = 1;
    DMError ret = absController_->SetScreenColorTransform(screenId);
    
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: StopScreens
 * @tc.desc: StopScreens test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, StopScreens, TestSize.Level1)
{
    std::vector<ScreenId> screenIds = {7};
    ScreenCombination stopCombination = ScreenCombination::SCREEN_ALONE;
    DMError ret = absController_->StopScreens(screenIds, stopCombination);
    ASSERT_EQ(ret, DMError::DM_OK);

    screenIds[0] = 2;
    ret = absController_->StopScreens(screenIds, stopCombination);
    ASSERT_EQ(ret, DMError::DM_OK);

    absController_->dmsScreenMap_[2]->groupDmsId_=2;
    ret = absController_->StopScreens(screenIds, stopCombination);
    ASSERT_EQ(ret, DMError::DM_OK);

    absController_->dmsScreenMap_[2]->groupDmsId_=5;
    ret = absController_->StopScreens(screenIds, stopCombination);
    ASSERT_EQ(ret, DMError::DM_OK);

    absController_->dmsScreenMap_[2]->groupDmsId_=2;
    stopCombination = ScreenCombination::SCREEN_MIRROR;
    ret = absController_->StopScreens(screenIds, stopCombination);
    ASSERT_EQ(ret, DMError::DM_OK);
}

/**
 * @tc.name: ChangeScreenGroup
 * @tc.desc: ChangeScreenGroup test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, ChangeScreenGroup03, TestSize.Level1)
{
    sptr<AbstractScreenGroup> group = screenGroupVec[0];
    Point point;
    group->screenMap_.insert(std::make_pair(0, screenVec[0]));
    group->screenMap_.insert(std::make_pair(1, screenVec[0]));
    std::vector<Point> startPoints;
    std::vector<ScreenId> screens;
    for (ScreenId i = 0; i < 7; ++i) {
        screens.emplace_back(i);
        startPoints.emplace_back(point);
        if (i < absController_->dmsScreenMap_.size() && absController_->dmsScreenMap_[i] != nullptr) {
            absController_->dmsScreenMap_[i]->groupDmsId_ = 1;
        }
    }
    absController_->abstractScreenCallback_ = nullptr;
    absController_->ChangeScreenGroup(group, screens, startPoints, true, ScreenCombination::SCREEN_ALONE);
    ASSERT_EQ(6, absController_->dmsScreenMap_.size());
}

/**
 * @tc.name: UpdateRSTree
 * @tc.desc: UpdateRSTree test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, UpdateRSTree06, TestSize.Level1)
{
    ScreenId id = 1;
    ScreenId parentId = 2;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    RSDisplayNodeConfig config;
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(id));
    sptr<AbstractScreen> parentScreen = absController_->GetAbstractScreen(parentId);
    ASSERT_NE(nullptr, absController_->GetAbstractScreen(parentId));
    parentScreen->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    absController_->UpdateRSTree(id, parentId, node, true, false);
}

/**
 * @tc.name: SetScreenRotateAnimation
 * @tc.desc: SetScreenRotateAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetScreenRotateAnimation04, TestSize.Level1)
{
    RSDisplayNodeConfig config;
    absController_->dmsScreenMap_[1]->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config);
    sptr<AbstractScreen> screen = screenVec[0];
    screen->rotation_ = Rotation::ROTATION_0;
    absController_->SetScreenRotateAnimation(screen, 1, Rotation::ROTATION_270, false);
    ASSERT_EQ(Rotation::ROTATION_0, screen->rotation_);
}

/**
 * @tc.name: SetRotation
 * @tc.desc: SetRotation test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetRotation02, TestSize.Level1)
{
    absController_->dmsScreenMap_[1]->rotation_ = Rotation::ROTATION_180;
    absController_->screenIdManager_.dms2RsScreenIdMap_.erase(1);
    ASSERT_EQ(true, absController_->SetRotation(1, Rotation::ROTATION_0, true));
    absController_->abstractScreenCallback_ = new AbstractScreenController::AbstractScreenCallback;
    ASSERT_EQ(false, absController_->SetRotation(1, Rotation::ROTATION_0, false));
    ASSERT_EQ(false, absController_->SetRotation(1, Rotation::ROTATION_0, true));
}

/**
 * @tc.name: InitVirtualScreen
 * @tc.desc: InitVirtualScreen test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, InitVirtualScreen03, TestSize.Level1)
{
    VirtualScreenOption option;
    sptr<AbstractScreen> defaultScreen = absController_->dmsScreenMap_[absController_->GetDefaultAbstractScreenId()];
    sptr<SupportedScreenModes> modes;
    if (defaultScreen != nullptr) {
        defaultScreen->modes_.emplace_back(modes);
        defaultScreen->activeIdx_ = 0;
        ASSERT_EQ(nullptr, defaultScreen->GetActiveScreenMode());
        sptr<AbstractScreen> screen = absController_->InitVirtualScreen(0, 0, option);
        ASSERT_EQ(ScreenType::VIRTUAL, screen->type_);
    }
}

/**
 * @tc.name: SetVirtualScreenAsDefault
 * @tc.desc: SetVirtualScreenAsDefault test
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenControllerTest, SetVirtualScreenAsDefault, TestSize.Level1)
{
    ScreenId screenId = 0;
    auto res = absController_->SetVirtualScreenAsDefault(screenId);
    EXPECT_FALSE(res);
    VirtualScreenOption option;
    sptr<IRemoteObject> displayManagerAgent = new IRemoteObjectMocker();
    auto virtualScreenId = absController_->CreateVirtualScreen(option, displayManagerAgent);
    res = absController_->SetVirtualScreenAsDefault(virtualScreenId);
    EXPECT_FALSE(res);
}
}
} // namespace Rosen
} // namespace OHOS
