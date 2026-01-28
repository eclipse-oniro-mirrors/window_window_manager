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
#include "picture_in_picture_option.h"
#include "wm_common.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class PictureInPictureOptionTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;
};

void PictureInPictureOptionTest::SetUpTestCase() {}

void PictureInPictureOptionTest::TearDownTestCase() {}

void PictureInPictureOptionTest::SetUp() {}

void PictureInPictureOptionTest::TearDown() {}

namespace {

/**
 * @tc.name: ClearNapiRefs
 * @tc.desc: ClearNapiRefs
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, ClearNapiRefs, Function | SmallTest | Level2)
{
    int num = 0;
    napi_ref ref = reinterpret_cast<napi_ref>(&num);
    ASSERT_NE(nullptr, ref);
    sptr<PipOption> option = new PipOption();
    option->SetNodeControllerRef(ref);
    ASSERT_NE(nullptr, option->GetNodeControllerRef());
    option->SetTypeNodeRef(ref);
    ASSERT_NE(nullptr, option->GetTypeNodeRef());
    option->SetStorageRef(ref);
    ASSERT_NE(nullptr, option->GetStorageRef());
    option->ClearNapiRefs(nullptr);
    ASSERT_EQ(nullptr, option->GetNodeControllerRef());
    ASSERT_EQ(nullptr, option->GetTypeNodeRef());
    ASSERT_EQ(nullptr, option->GetStorageRef());
}

/**
 * @tc.name: Context
 * @tc.desc: SetContext/GetContext
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, Context, TestSize.Level1)
{
    void* contextPtr = nullptr;
    sptr<PipOption> option = new PipOption();
    option->SetContext(contextPtr);
    ASSERT_EQ(contextPtr, option->GetContext());
}

/**
 * @tc.name: PipTemplate
 * @tc.desc: SetPipTemplate/GetPipTemplate
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, PipTemplate, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
    option->SetPipTemplate(100);
    ASSERT_EQ(100, option->GetPipTemplate());
}

/**
 * @tc.name: DefaultWindowSizeType
 * @tc.desc: SetDefaultWindowSizeType/GetDefaultWindowSizeType
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, DefaultWindowSizeType, Function | SmallTest | Level2)
{
    sptr<PipOption> option = new PipOption();
    option->SetDefaultWindowSizeType(100);
    ASSERT_EQ(100, option->GetDefaultWindowSizeType());
}

/**
 * @tc.name: PiPControlStatus
 * @tc.desc: SetPiPControlStatus/GetControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, SetPiPControlStatus, TestSize.Level1)
{
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::PLAY;
    option->SetPiPControlStatus(controlType, status);
    controlType = PiPControlType::END;
    option->SetPiPControlStatus(controlType, status);
    ASSERT_NE(0, option->GetControlStatus().size());
}

/**
 * @tc.name: PiPControlEnable
 * @tc.desc: SetPiPControlEnabled/GetControlEnable
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, SetPiPControlEnabled, TestSize.Level1)
{
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto enabled = PiPControlStatus::ENABLED;
    option->SetPiPControlEnabled(controlType, enabled);
    controlType = PiPControlType::END;
    option->SetPiPControlEnabled(controlType, enabled);
    ASSERT_NE(0, option->GetControlEnable().size());
}

/**
 * @tc.name: NavigationId
 * @tc.desc: SetNavigationId/GetNavigationId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, NavigationId, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
    std::string navigationId = "abc";
    option->SetNavigationId(navigationId);
    ASSERT_EQ(navigationId, option->GetNavigationId());
}

/**
 * @tc.name: ControlGroup
 * @tc.desc: SetControlGroup/GetControlGroup
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, SetGetControlGroupTest, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
    std::vector<std::uint32_t> controlGroup;
    controlGroup.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_MICROPHONE_SWITCH));
    controlGroup.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_HANG_UP_BUTTON));
    controlGroup.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_CALL_CAMERA_SWITCH));
    option->SetControlGroup(controlGroup);
    ASSERT_NE(option->GetControlGroup().size(), 0);
}

/**
 * @tc.name: ContentSize
 * @tc.desc: SetContentSize/GetContentSize
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, ContentSize, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
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
HWTEST_F(PictureInPictureOptionTest, NodeController, TestSize.Level1)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    option->SetNodeControllerRef(nullptr);
    ASSERT_EQ(option->GetNodeControllerRef(), nullptr);
}

/**
 * @tc.name: TypeNodeRef
 * @tc.desc: SetTypeNodeRef/GetTypeNodeRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, TypeNodeRef, TestSize.Level1)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    option->SetTypeNodeRef(nullptr);
    ASSERT_EQ(option->GetTypeNodeRef(), nullptr);
}

/**
 * @tc.name: StorageRef
 * @tc.desc: SetStorageRef/GetStorageRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, StorageRef, Function | SmallTest | Level2)
{
    int num = 0;
    napi_ref ref = reinterpret_cast<napi_ref>(&num);
    ASSERT_NE(nullptr, ref);
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    option->SetStorageRef(ref);
    ASSERT_EQ(option->GetStorageRef(), ref);
}

/**
 * @tc.name: TypeNodeEnabled
 * @tc.desc: SetTypeNodeEnabled/IsTypeNodeEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, TypeNodeEnabled, TestSize.Level1)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    option->SetTypeNodeEnabled(true);
    ASSERT_TRUE(option->IsTypeNodeEnabled());
    option->SetTypeNodeEnabled(false);
    ASSERT_TRUE(!option->IsTypeNodeEnabled());
}

/**
 * @tc.name: CornerAdsorptionEnabled
 * @tc.desc: SetCornerAdsorptionEnabled/GetCornerAdsorptionEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, CornerAdsorptionEnabled, TestSize.Level1)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
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
HWTEST_F(PictureInPictureOptionTest, GetPiPTemplateInfo, TestSize.Level1)
{
    sptr<PipOption> option = new PipOption();
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
 * @tc.name: SetHandleId
 * @tc.desc: SetHandleId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, SetHandleId, TestSize.Level1)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    int handleId = 10;
    option->SetHandleId(handleId);
    EXPECT_EQ(handleId, option->GetHandleId());
}

/**
 * @tc.name: GetHandleId
 * @tc.desc: GetHandleId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, GetHandleId, TestSize.Level1)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    EXPECT_EQ(-1, option->GetHandleId());

    int handleId = 8;
    option->SetHandleId(handleId);
    EXPECT_EQ(handleId, option->GetHandleId());
}
} // namespace
} // namespace Rosen
} // namespace OHOS