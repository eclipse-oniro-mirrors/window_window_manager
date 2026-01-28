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

#include "abstract_screen.h"
#include "abstract_screen_controller.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class AbstractScreenTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<AbstractScreen> absScreen_;
    static sptr<AbstractScreenGroup> absScreenGroup_;
    static std::recursive_mutex mutex_;
    static std::string name_;
};

sptr<AbstractScreen> AbstractScreenTest::absScreen_ = nullptr;
sptr<AbstractScreenGroup> AbstractScreenTest::absScreenGroup_ = nullptr;
std::recursive_mutex AbstractScreenTest::mutex_;
std::string AbstractScreenTest::name_ = "AbstractScreenTest";

void AbstractScreenTest::SetUpTestCase()
{
    sptr<AbstractScreenController> absScreenController = new AbstractScreenController(mutex_);
    absScreen_ = new AbstractScreen(absScreenController, name_, 0, 0);
    absScreenGroup_ = new AbstractScreenGroup(absScreenController,
        0, 0, name_, ScreenCombination::SCREEN_ALONE);
    absScreen_->modes_.clear();
    absScreen_->activeIdx_ = 0;
}

void AbstractScreenTest::TearDownTestCase()
{
}

void AbstractScreenTest::SetUp()
{
}

void AbstractScreenTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetScreenMode
 * @tc.desc: Get screen mode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetScreenMode, TestSize.Level1)
{
    sptr<SupportedScreenModes> mode0 = new SupportedScreenModes();
    sptr<SupportedScreenModes> mode1 = new SupportedScreenModes();
    absScreen_->modes_ = {mode0, mode1};

    absScreen_->activeIdx_ = -1;
    ASSERT_EQ(nullptr, absScreen_->GetActiveScreenMode());
    absScreen_->activeIdx_ = static_cast<int32_t>(absScreen_->modes_.size());
    ASSERT_EQ(nullptr, absScreen_->GetActiveScreenMode());
    absScreen_->activeIdx_ = 0;
    ASSERT_EQ(mode0, absScreen_->GetActiveScreenMode());
    absScreen_->activeIdx_ = 1;
    ASSERT_EQ(mode1, absScreen_->GetActiveScreenMode());

    ASSERT_EQ(mode0, (absScreen_->GetAbstractScreenModes())[0]);
    ASSERT_EQ(mode1, (absScreen_->GetAbstractScreenModes())[1]);
}

/**
 * @tc.name: ConvertToScreenInfo
 * @tc.desc: Convert to screen info
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, ConvertToScreenInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, absScreen_->ConvertToScreenInfo());
}

/**
 * @tc.name: RSTree
 * @tc.desc: RS tree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, RSTree, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    absScreen_->rsDisplayNode_ = nullptr;
    absScreen_->UpdateRSTree(surfaceNode, true);
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, true);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
    absScreen_->UpdateRSTree(surfaceNode, true);
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, true);

    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    absScreen_->UpdateRSTree(surfaceNode, true);
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, false);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, false);
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, false);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, false, false);
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, false);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, false, false);
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, true);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: InitRSDisplayNode
 * @tc.desc: InitRSDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, InitRSDisplayNode, TestSize.Level1)
{
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    RSDisplayNodeConfig config_;
    Point startPoint;
    absScreen_->InitRSDisplayNode(config_, startPoint);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
}

/**
 * @tc.name: InitRSDefaultDisplayNode
 * @tc.desc: InitRSDefaultDisplayNode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, InitRSDefaultDisplayNode, TestSize.Level1)
{
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    RSDisplayNodeConfig config_;
    Point startPoint;
    absScreen_->InitRSDefaultDisplayNode(config_, startPoint);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
}

/**
 * @tc.name: SetScreenColorGamut
 * @tc.desc: SetScreenColorGamut
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, SetScreenColorGamut, TestSize.Level1)
{
    int32_t colorGamutIdx = 0;
    auto result = absScreen_->SetScreenColorGamut(colorGamutIdx);
    ASSERT_EQ(result, DMError::DM_ERROR_RENDER_SERVICE_FAILED);
}

/**
 * @tc.name: ColorGamut
 * @tc.desc: Screen color gamut
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, ColorGamut, TestSize.Level1)
{
    sptr<AbstractScreenController> absScreenController0 = new AbstractScreenController(mutex_);
    sptr<AbstractScreen> absScreen0 = new AbstractScreen(absScreenController0, name_, 0, -1ULL);
    std::vector<ScreenColorGamut> colorGamuts;
    ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, absScreen0->GetScreenSupportedColorGamuts(colorGamuts));
    ScreenColorGamut colorGamut;
    ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, absScreen0->GetScreenColorGamut(colorGamut));

    ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, absScreen0->SetScreenColorGamut(0));

    ScreenGamutMap gamutMap;
    ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, absScreen0->GetScreenGamutMap(gamutMap));

    gamutMap = ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION;
    ASSERT_EQ(DMError::DM_ERROR_RENDER_SERVICE_FAILED, absScreen0->GetScreenGamutMap(gamutMap));
}

/**
 * @tc.name: FillScreenInfo
 * @tc.desc: Fill screen info
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, FillScreenInfo, TestSize.Level1)
{
    absScreen_->FillScreenInfo(nullptr);
    sptr<ScreenInfo> info = new ScreenInfo();
    ASSERT_NE(nullptr, info);

    absScreen_->virtualPixelRatio_ = 0.f;
    absScreen_->FillScreenInfo(info);
    ASSERT_EQ(1.f, info->virtualPixelRatio_);

    absScreen_->virtualPixelRatio_ = 2.f;
    absScreen_->FillScreenInfo(info);
    ASSERT_EQ(2.f, info->virtualPixelRatio_);
}

/**
 * @tc.name: CalcRotation
 * @tc.desc: Calc rotation
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, CalcRotation, TestSize.Level1)
{
    absScreen_->modes_.clear();
    absScreen_->activeIdx_ = 0;

    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::UNSPECIFIED));

    sptr<SupportedScreenModes> mode = new SupportedScreenModes();
    mode->width_ = 1;
    mode->height_ = 1;
    absScreen_->modes_ = {mode};

    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::UNSPECIFIED));
    ASSERT_EQ(Rotation::ROTATION_90, absScreen_->CalcRotation(Orientation::VERTICAL));
    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::HORIZONTAL));
    ASSERT_EQ(Rotation::ROTATION_270, absScreen_->CalcRotation(Orientation::REVERSE_VERTICAL));
    ASSERT_EQ(Rotation::ROTATION_180, absScreen_->CalcRotation(Orientation::REVERSE_HORIZONTAL));
    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::LOCKED));
}

/**
 * @tc.name: GetScreenGroupId
 * @tc.desc: get screen groupId
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetScreenGroupId, TestSize.Level1)
{
    ScreenId result = absScreen_->GetScreenGroupId();
    EXPECT_EQ(result, SCREEN_ID_INVALID);
    absScreen_->groupDmsId_ = 10086;
    result = absScreen_->GetScreenGroupId();
    EXPECT_EQ(result, 10086);
}

/**
 * @tc.name: SetScreenGamutMap
 * @tc.desc: set screen gamut map
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, SetScreenGamutMap, TestSize.Level1)
{
    ScreenGamutMap gamutMap = ScreenGamutMap::GAMUT_MAP_HDR_CONSTANT;
    DMError result = absScreen_->SetScreenGamutMap(gamutMap);
    EXPECT_EQ(result, DMError::DM_ERROR_RENDER_SERVICE_FAILED);

    gamutMap = static_cast<ScreenGamutMap>(static_cast<uint32_t>(ScreenGamutMap::GAMUT_MAP_HDR_EXTENSION) + 1);
    result = absScreen_->SetScreenGamutMap(gamutMap);
    EXPECT_EQ(result, DMError::DM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetScreenColorTransform
 * @tc.desc: set screen color transform
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, SetScreenColorTransform, TestSize.Level1)
{
    DMError result = absScreen_->SetScreenColorTransform();
    EXPECT_EQ(result, DMError::DM_OK);
}

/**
 * @tc.name: GetVirtualPixelRatio
 * @tc.desc: get virtual pixel ratio
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetVirtualPixelRatio, TestSize.Level1)
{
    float result = absScreen_->GetVirtualPixelRatio();
    EXPECT_EQ(result, 2.0f);
}

/**
 * @tc.name: GetSourceMode
 * @tc.desc: get source mode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetSourceMode, TestSize.Level1)
{
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_MIRROR;
    absScreen_->screenController_->dmsScreenGroupMap_.insert({10086, absScreenGroup_});
    absScreen_->groupDmsId_ = 10086;
    ScreenSourceMode result = absScreen_->GetSourceMode();
    EXPECT_EQ(result, ScreenSourceMode::SCREEN_MAIN);

    absScreen_->screenController_->defaultRsScreenId_ = 144;
    result = absScreen_->GetSourceMode();
    EXPECT_EQ(result, ScreenSourceMode::SCREEN_MIRROR);
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_EXPAND;
    result = absScreen_->GetSourceMode();
    EXPECT_EQ(result, ScreenSourceMode::SCREEN_EXTEND);
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_ALONE;
    result = absScreen_->GetSourceMode();
    EXPECT_EQ(result, ScreenSourceMode::SCREEN_ALONE);

    sptr<AbstractScreenController> absScreenController = new AbstractScreenController(mutex_);
    sptr<AbstractScreen> absScreenTest = new AbstractScreen(absScreenController, name_, 0, 0);
    result = absScreenTest->GetSourceMode();
    EXPECT_EQ(result, ScreenSourceMode::SCREEN_ALONE);
}

/**
 * @tc.name: ConvertToScreenGroupInfo
 * @tc.desc: convert to screen group info
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, ConvertToScreenGroupInfo, TestSize.Level1)
{
    Point point_(159, 357);
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({10086, absScreen_});
    sptr<ScreenGroupInfo> result = absScreenGroup_->ConvertToScreenGroupInfo();
    EXPECT_EQ(result->children_[0], 10086);
}

/**
 * @tc.name: GetRSDisplayNodeConfig01
 * @tc.desc: Get RSDisplay node config
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetRSDisplayNodeConfig01, TestSize.Level1)
{
    sptr<AbstractScreen> absTest = nullptr;
    RSDisplayNodeConfig config_;
    bool result = absScreenGroup_->GetRSDisplayNodeConfig(absTest, config_);
    EXPECT_FALSE(result);

    absScreenGroup_->combination_ = ScreenCombination::SCREEN_ALONE;
    result = absScreenGroup_->GetRSDisplayNodeConfig(absTest, config_);
    EXPECT_FALSE(result);

    absScreenGroup_->combination_ = ScreenCombination::SCREEN_MIRROR;
    absScreenGroup_->mirrorScreenId_ = 0;
    result = absScreenGroup_->GetRSDisplayNodeConfig(absTest, config_);
    EXPECT_FALSE(result);
    Point point_(159, 357);
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({10086, absScreen_});
    absScreenGroup_->mirrorScreenId_ = 10086;
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetRSDisplayNodeConfig01
 * @tc.desc: Get RSDisplay node config
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetRSDisplayNodeConfig02, TestSize.Level1)
{
    sptr<AbstractScreen> absTest = absScreen_;
    RSDisplayNodeConfig config_;

    absScreenGroup_->combination_ = ScreenCombination::SCREEN_ALONE;
    bool result = absScreenGroup_->GetRSDisplayNodeConfig(absTest, config_);
    EXPECT_EQ(true, result);

    absScreenGroup_->combination_ = ScreenCombination::SCREEN_EXPAND;
    result = absScreenGroup_->GetRSDisplayNodeConfig(absTest, config_);
    EXPECT_EQ(true, result);

    absScreenGroup_->combination_ = ScreenCombination::SCREEN_MIRROR;
    absScreenGroup_->mirrorScreenId_ = 0;
    result = absScreenGroup_->GetRSDisplayNodeConfig(absTest, config_);
    EXPECT_EQ(true, result);
    Point point_(159, 357);
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({10086, absScreen_});
    absScreenGroup_->mirrorScreenId_ = 10086;
    EXPECT_EQ(true, result);
}

/**
 * @tc.name: GetChildPosition
 * @tc.desc: GetChildPosition
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetChildPosition, TestSize.Level1)
{
    ScreenId screenId = 1;
    Point expectedPoint(0, 0);
    Point actualPoint = absScreenGroup_->GetChildPosition(screenId);
    EXPECT_EQ(expectedPoint.posX_, actualPoint.posX_);
    EXPECT_EQ(expectedPoint.posY_, actualPoint.posY_);
}

/**
 * @tc.name: AddChild01
 * @tc.desc: Add child
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, AddChild01, TestSize.Level1)
{
    absScreenGroup_->screenMap_.clear();
    sptr<AbstractScreen> absTest = nullptr;
    Point point_(159, 357);
    bool result = absScreenGroup_->AddChild(absTest, point_);
    EXPECT_FALSE(result);

    absScreenGroup_->mirrorScreenId_ = 10086;
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_MIRROR;
    result = absScreenGroup_->AddChild(absScreen_, point_);
    ASSERT_TRUE(result);
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_EXPAND;
    absScreen_->rsDisplayNode_ = nullptr;
    result = absScreenGroup_->AddChild(absScreen_, point_);
    EXPECT_FALSE(result);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    absScreen_->type_ = ScreenType::REAL;
    absScreenGroup_->defaultScreenId_ = 0;
    result = absScreenGroup_->AddChild(absScreen_, point_);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: AddChild02
 * @tc.desc: Add child
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, AddChild02, TestSize.Level1)
{
    absScreenGroup_->screenMap_.clear();
    Point point_(159, 357);
    absScreenGroup_->mirrorScreenId_ = 10086;
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_MIRROR;
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({0, absScreen_});
    absScreen_->rsDisplayNode_ = nullptr;
    bool result = absScreenGroup_->AddChild(absScreen_, point_);
    EXPECT_FALSE(result);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    absScreen_->type_ = ScreenType::REAL;
    absScreenGroup_->defaultScreenId_ = 0;
    result = absScreenGroup_->AddChild(absScreen_, point_);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: AddChildren
 * @tc.desc: Add children
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, AddChildren, TestSize.Level1)
{
    Point point_(159, 357);
    std::vector<Point> vecPoint({point_});
    std::vector<sptr<AbstractScreen>> vecSptr;
    bool result = absScreenGroup_->AddChildren(vecSptr, vecPoint);
    EXPECT_FALSE(result);

    vecSptr.push_back(absScreen_);
    result = absScreenGroup_->AddChildren(vecSptr, vecPoint);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: RemoveChild01
 * @tc.desc: Remove child
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, RemoveChild01, TestSize.Level1)
{
    sptr<AbstractScreen> absTest = nullptr;
    bool result = absScreenGroup_->RemoveChild(absTest);
    EXPECT_FALSE(result);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    result = absScreenGroup_->RemoveChild(absScreen_);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: RemoveChild02
 * @tc.desc: Remove child
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, RemoveChild02, TestSize.Level1)
{
    Point point_(159, 357);
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({0, absScreen_});
    bool result = absScreenGroup_->RemoveChild(absScreen_);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: RemoveDefaultScreen
 * @tc.desc: Remove default screen
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, RemoveDefaultScreen, TestSize.Level1)
{
    sptr<AbstractScreen> absTest = nullptr;
    bool result = absScreenGroup_->RemoveDefaultScreen(absTest);
    EXPECT_FALSE(result);

    absScreen_->rsDisplayNode_ = nullptr;
    result = absScreenGroup_->RemoveDefaultScreen(absScreen_);
    EXPECT_TRUE(result);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    result = absScreenGroup_->RemoveDefaultScreen(absScreen_);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: GetChildren
 * @tc.desc: Get children
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetChildren, TestSize.Level1)
{
    Point point_(159, 357);
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({10086, absScreen_});
    std::vector<sptr<AbstractScreen>> result = absScreenGroup_->GetChildren();
    ASSERT_EQ(result[0], absScreen_);
}

/**
 * @tc.name: GetChildrenPosition
 * @tc.desc: Get children position
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetChildrenPosition, TestSize.Level1)
{
    Point point_(159, 357);
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({10086, absScreen_});
    std::vector<Point> result = absScreenGroup_->GetChildrenPosition();
    EXPECT_EQ(result[0].posX_, 159);
    EXPECT_EQ(result[0].posY_, 357);
}

/**
 * @tc.name: ~AbstractScreenGroup
 * @tc.desc: Abstract screen group
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, AbstractScreenGroup, TestSize.Level1)
{
    sptr<AbstractScreenController> absScreenController = new AbstractScreenController(mutex_);
    absScreenGroup_ = new AbstractScreenGroup(absScreenController,
        0, 0, name_, ScreenCombination::SCREEN_ALONE);
    EXPECT_NE(absScreenController, nullptr);
}

/**
 * @tc.name: AddSurfaceNode
 * @tc.desc: AddSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, AddSurfaceNode, TestSize.Level1)
{
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    DMError ret = absScreen_->AddSurfaceNode(surfaceNode, true);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());

    ret = absScreen_->AddSurfaceNode(surfaceNode, true);
    ASSERT_EQ(ret, DMError::DM_OK);

    ret = absScreen_->AddSurfaceNode(surfaceNode, true, false);
    ASSERT_EQ(ret, DMError::DM_OK);

    ret = absScreen_->AddSurfaceNode(surfaceNode, false);
    ASSERT_EQ(ret, DMError::DM_OK);

    absScreen_->rsDisplayNode_ = nullptr;
    ret = absScreen_->AddSurfaceNode(surfaceNode, false);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    surfaceNode = nullptr;
    ret = absScreen_->AddSurfaceNode(surfaceNode, false);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: RemoveSurfaceNode
 * @tc.desc: RemoveSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, RemoveSurfaceNode, TestSize.Level1)
{
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    DMError ret = absScreen_->RemoveSurfaceNode(surfaceNode);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    ret = absScreen_->RemoveSurfaceNode(surfaceNode);
    ASSERT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);

    absScreen_->rsDisplayNode_ = nullptr;
    ret = absScreen_->RemoveSurfaceNode(surfaceNode);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);

    surfaceNode = nullptr;
    ret = absScreen_->RemoveSurfaceNode(surfaceNode);
    ASSERT_EQ(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetScreenMode
 * @tc.desc: Get screen mode
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, GetScreenMode01, TestSize.Level1)
{
    sptr<SupportedScreenModes> mode0 = new SupportedScreenModes();
    sptr<SupportedScreenModes> mode1 = new SupportedScreenModes();
    absScreen_->modes_ = {mode0, mode1};

    absScreen_->activeIdx_ = -1;
    ASSERT_EQ(nullptr, absScreen_->GetActiveScreenMode());
    absScreen_->activeIdx_ = static_cast<int32_t>(absScreen_->modes_.size());
    ASSERT_EQ(nullptr, absScreen_->GetActiveScreenMode());
    absScreen_->activeIdx_ = 0;
    ASSERT_EQ(mode0, absScreen_->GetActiveScreenMode());
    absScreen_->activeIdx_ = 1;
    ASSERT_EQ(mode1, absScreen_->GetActiveScreenMode());

    ASSERT_EQ(mode0, (absScreen_->GetAbstractScreenModes())[0]);
    ASSERT_EQ(mode1, (absScreen_->GetAbstractScreenModes())[1]);
}

/**
 * @tc.name: AddChild
 * @tc.desc: Add child
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, AddChild03, TestSize.Level1)
{
    absScreenGroup_->screenMap_.clear();
    Point point_(159, 357);
    absScreenGroup_->mirrorScreenId_ = 10086;
    absScreenGroup_->combination_ = ScreenCombination::SCREEN_MIRROR;
    absScreen_->startPoint_ = point_;
    (absScreenGroup_->screenMap_).insert({0, absScreen_});
    absScreen_->rsDisplayNode_ = nullptr;
    bool result = absScreenGroup_->AddChild(absScreen_, point_);
    EXPECT_FALSE(result);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    absScreen_->type_ = ScreenType::REAL;
    absScreenGroup_->defaultScreenId_ = 0;
    result = absScreenGroup_->AddChild(absScreen_, point_);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: CalcRotation
 * @tc.desc: Calc rotation
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, CalcRotation01, TestSize.Level1)
{
    absScreen_->modes_.clear();
    absScreen_->activeIdx_ = 0;

    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::UNSPECIFIED));

    sptr<SupportedScreenModes> mode = new SupportedScreenModes();
    mode->width_ = 1;
    mode->height_ = 1;
    absScreen_->modes_ = {mode};

    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::UNSPECIFIED));
    ASSERT_EQ(Rotation::ROTATION_90, absScreen_->CalcRotation(Orientation::VERTICAL));
    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::HORIZONTAL));
    ASSERT_EQ(Rotation::ROTATION_270, absScreen_->CalcRotation(Orientation::REVERSE_VERTICAL));
    ASSERT_EQ(Rotation::ROTATION_180, absScreen_->CalcRotation(Orientation::REVERSE_HORIZONTAL));
    ASSERT_EQ(Rotation::ROTATION_0, absScreen_->CalcRotation(Orientation::LOCKED));
}

/**
 * @tc.name: RemoveDefaultScreen
 * @tc.desc: Remove default screen
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, RemoveDefaultScreen02, TestSize.Level1)
{
    sptr<AbstractScreen> absTest = nullptr;
    bool result = absScreenGroup_->RemoveDefaultScreen(absTest);
    EXPECT_FALSE(result);

    absScreen_->rsDisplayNode_ = nullptr;
    result = absScreenGroup_->RemoveDefaultScreen(absScreen_);
    EXPECT_TRUE(result);

    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    result = absScreenGroup_->RemoveDefaultScreen(absScreen_);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: UpdateRSTree01
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree01, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    absScreen_->rsDisplayNode_ = nullptr;
    bool isAdd = true;
    bool needToUpdate = true;

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    ASSERT_EQ(nullptr, absScreen_->rsDisplayNode_);
}

/**
 * @tc.name: UpdateRSTree02
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree02, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    bool isAdd = true;
    bool needToUpdate = true;

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree03
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree03, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    bool isAdd = true;
    bool needToUpdate = false;

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree04
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree04, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    bool isAdd = false;
    bool needToUpdate = false;

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree05
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree05, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    bool isAdd = false;
    bool needToUpdate = true;

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree06
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree06, TestSize.Level1)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    bool isAdd = false;
    bool needToUpdate = true;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree07
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree07, TestSize.Level1)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    bool isAdd = true;
    bool needToUpdate = false;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree08
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree08, TestSize.Level1)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    bool isAdd = false;
    bool needToUpdate = false;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateRSTree09
 * @tc.desc: UpdateRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateRSTree09, TestSize.Level1)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    bool isAdd = false;
    bool needToUpdate = true;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateRSTree(surfaceNode, isAdd, needToUpdate);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateDisplayGroupRSTree01
 * @tc.desc: UpdateDisplayGroupRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateDisplayGroupRSTree01, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    bool isAdd = true;
    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, isAdd);
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);
}

/**
 * @tc.name: UpdateDisplayGroupRSTree02
 * @tc.desc: UpdateDisplayGroupRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateDisplayGroupRSTree02, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    bool isAdd = true;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, isAdd);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateDisplayGroupRSTree03
 * @tc.desc: UpdateDisplayGroupRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateDisplayGroupRSTree03, TestSize.Level1)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    bool isAdd = false;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, isAdd);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateDisplayGroupRSTree04
 * @tc.desc: UpdateDisplayGroupRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateDisplayGroupRSTree04, TestSize.Level1)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT, true, false, absScreen_->GetRSUIContext());
    bool isAdd = true;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, isAdd);
    absScreen_->rsDisplayNode_ = nullptr;
}

/**
 * @tc.name: UpdateDisplayGroupRSTree05
 * @tc.desc: UpdateDisplayGroupRSTree
 * @tc.type: FUNC
 */
HWTEST_F(AbstractScreenTest, UpdateDisplayGroupRSTree05, TestSize.Level1)
{
    std::shared_ptr<RSSurfaceNode> surfaceNode;
    bool isAdd = false;
    struct RSDisplayNodeConfig config;
    absScreen_->rsDisplayNode_ = std::make_shared<RSDisplayNode>(config, absScreen_->GetRSUIContext());
    ASSERT_NE(nullptr, absScreen_->rsDisplayNode_);

    absScreen_->UpdateDisplayGroupRSTree(surfaceNode, absScreen_->rsDisplayNode_, isAdd);
    absScreen_->rsDisplayNode_ = nullptr;
}
}
} // namespace Rosen
} // namespace OHOS
