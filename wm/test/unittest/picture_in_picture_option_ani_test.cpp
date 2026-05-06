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

#include <gtest/gtest.h>
#include "picture_in_picture_option_ani.h"
#include "wm_common.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class PictureInPictureOptionAniTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;
};

void PictureInPictureOptionAniTest::SetUpTestCase() {
}

void PictureInPictureOptionAniTest::TearDownTestCase() {
}

void PictureInPictureOptionAniTest::SetUp() {
}

void PictureInPictureOptionAniTest::TearDown() {
}

namespace {

/**
 * @tc.name: Context
 * @tc.desc: SetContext/GetContext
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, Context, TestSize.Level1)
{
    void* contextPtr = nullptr;
    sptr<PipOptionAni> option = new PipOptionAni();
    option->SetContext(contextPtr);
    ASSERT_EQ(contextPtr, option->GetContext());
}

/**
 * @tc.name: PipTemplate
 * @tc.desc: SetPipTemplate/GetPipTemplate
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, PipTemplate, TestSize.Level1)
{
    sptr<PipOptionAni> option = new PipOptionAni();
    option->SetPipTemplate(100);
    ASSERT_EQ(100, option->GetPipTemplate());
}

/**
 * @tc.name: DefaultWindowSizeType
 * @tc.desc: SetDefaultWindowSizeType/GetDefaultWindowSizeType
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, DefaultWindowSizeType, Function | SmallTest | Level2)
{
    sptr<PipOptionAni> option = new PipOptionAni();
    option->SetDefaultWindowSizeType(100);
    ASSERT_EQ(100, option->GetDefaultWindowSizeType());
}

/**
 * @tc.name: PiPControlStatus
 * @tc.desc: SetPiPControlStatus/GetControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, SetPiPControlStatus, TestSize.Level1)
{
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::PLAY;
    option->SetPiPControlStatus(controlType, status);
    controlType = PiPControlType::END;
    option->SetPiPControlStatus(controlType, status);
    ASSERT_EQ(2, option->GetControlStatus().size());
}

/**
 * @tc.name: PiPControlEnable
 * @tc.desc: SetPiPControlEnabled/GetControlEnable
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, SetPiPControlEnabled, TestSize.Level1)
{
    auto option = sptr<PipOptionAni>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto enabled = PiPControlStatus::ENABLED;
    option->SetPiPControlEnabled(controlType, enabled);
    controlType = PiPControlType::END;
    option->SetPiPControlEnabled(controlType, enabled);
    ASSERT_EQ(2, option->GetControlEnable().size());
}

/**
 * @tc.name: NavigationId
 * @tc.desc: SetNavigationId/GetNavigationId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, NavigationId, TestSize.Level1)
{
    sptr<PipOptionAni> option = new PipOptionAni();
    std::string navigationId = "abc";
    option->SetNavigationId(navigationId);
    ASSERT_EQ(navigationId, option->GetNavigationId());
}

/**
 * @tc.name: ControlGroup
 * @tc.desc: SetControlGroup/GetControlGroup
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, SetGetControlGroupTest, TestSize.Level1)
{
    sptr<PipOptionAni> option = new PipOptionAni();
    std::vector<std::uint32_t> controlGroup;
    controlGroup.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_MICROPHONE_SWITCH));
    controlGroup.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_HANG_UP_BUTTON));
    controlGroup.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_CAMERA_SWITCH));
    option->SetControlGroup(controlGroup);
    ASSERT_EQ(option->GetControlGroup().size(), 3);
}

/**
 * @tc.name: ContentSize
 * @tc.desc: SetContentSize/GetContentSize
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, ContentSize, TestSize.Level1)
{
    sptr<PipOptionAni> option = new PipOptionAni();
    uint32_t width = 800;
    uint32_t height = 600;
    option->SetContentSize(width, height);
    uint32_t w = 0;
    uint32_t h = 0;
    option->GetContentSize(w, h);
    ASSERT_EQ(width, w);
    ASSERT_EQ(height, h);
}

/**
 * @tc.name: NodeController
 * @tc.desc: SetNodeControllerRef/GetNodeControllerRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, NodeController, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetNodeControllerRef(nullptr);
    ASSERT_EQ(option->GetNodeControllerRef(), nullptr);
}

/**
 * @tc.name: TypeNodeRef
 * @tc.desc: SetTypeNodeRef/GetTypeNodeRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, TypeNodeRef, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetTypeNodeRef(nullptr);
    ASSERT_EQ(option->GetTypeNodeRef(), nullptr);
}

/**
 * @tc.name: StorageRef
 * @tc.desc: SetStorageRef/GetStorageRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, StorageRef, Function | SmallTest | Level2)
{
    ani_ref ref = nullptr;
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetStorageRef(ref);
    ASSERT_EQ(option->GetStorageRef(), ref);
}

/**
 * @tc.name: GetANIPipContentCallbackRef
 * @tc.desc: RegisterPipContentListenerWithType/GetANIPipContentCallbackRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, GetANIPipContentCallbackRef, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->RegisterPipContentListenerWithType("nodeUpdate", nullptr);
    ASSERT_EQ(option->GetANIPipContentCallbackRef("nodeUpdate"), nullptr);
}

/**
 * @tc.name: UnRegisterPipContentListenerWithType
 * @tc.desc: RegisterPipContentListenerWithType/UnRegisterPipContentListenerWithType
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, UnRegisterPipContentListenerWithType, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->RegisterPipContentListenerWithType("testCallback", nullptr);
    option->UnRegisterPipContentListenerWithType("testCallback");
    ASSERT_EQ(option->GetANIPipContentCallbackRef("testCallback"), nullptr);
}

/**
 * @tc.name: TypeNodeEnabled
 * @tc.desc: SetTypeNodeEnabled/IsTypeNodeEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, TypeNodeEnabled, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetTypeNodeEnabled(true);
    ASSERT_TRUE(option->IsTypeNodeEnabled());
    option->SetTypeNodeEnabled(false);
    ASSERT_FALSE(option->IsTypeNodeEnabled());
}

/**
 * @tc.name: CornerAdsorptionEnabled
 * @tc.desc: SetCornerAdsorptionEnabled/GetCornerAdsorptionEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, CornerAdsorptionEnabled, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetCornerAdsorptionEnabled(true);
    ASSERT_TRUE(option->GetCornerAdsorptionEnabled());
    option->SetCornerAdsorptionEnabled(false);
    ASSERT_FALSE(option->GetCornerAdsorptionEnabled());
}

/**
 * @tc.name: GetPiPTemplateInfo
 * @tc.desc: GetPiPTemplateInfo/GetPipPriority
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, GetPiPTemplateInfo, TestSize.Level1)
{
    sptr<PipOptionAni> option = new PipOptionAni();
    uint32_t pipTypeTemplate = 5;
    uint32_t testValue = 0;
    ASSERT_EQ(testValue, option->GetPipPriority(pipTypeTemplate));
    ASSERT_EQ(testValue, option->GetPipPriority(pipTypeTemplate = 3));
    ASSERT_EQ(testValue, option->GetPipPriority(pipTypeTemplate = 0));
    ASSERT_EQ(testValue = 1, option->GetPipPriority(pipTypeTemplate = 1));
    PiPTemplateInfo pipTemplateInfo;
    option->SetDefaultWindowSizeType(testValue = 2);
    option->GetPiPTemplateInfo(pipTemplateInfo);
    ASSERT_EQ(testValue, pipTemplateInfo.defaultWindowSizeType);
}

/**
 * @tc.name: HandleId
 * @tc.desc: GetHandleId/SetHandleId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, GetHandleId, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    EXPECT_EQ(-1, option->GetHandleId());
    int handleId = 8;
    option->SetHandleId(handleId);
    EXPECT_EQ(handleId, option->GetHandleId());
}

/**
 * @tc.name: ClearAniRefs
 * @tc.desc: ClearAniRefs
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, ClearAniRefs, TestSize.Level1)
{
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetNodeControllerRef(nullptr);
    option->SetTypeNodeRef(nullptr);
    option->SetStorageRef(nullptr);
    option->ClearAniRefs(nullptr);
    ASSERT_EQ(option->GetNodeControllerRef(), nullptr);
    ASSERT_EQ(option->GetTypeNodeRef(), nullptr);
    ASSERT_EQ(option->GetStorageRef(), nullptr);
}

/**
 * @tc.name: XComponentController
 * @tc.desc: SetXComponentController/GetXComponentController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionAniTest, XComponentController, TestSize.Level1)
{
    std::shared_ptr<XComponentController> xComponentController = nullptr;
    sptr<PipOptionAni> option = sptr<PipOptionAni>::MakeSptr();
    option->SetXComponentController(xComponentController);
    ASSERT_EQ(option->GetXComponentController(), xComponentController);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
