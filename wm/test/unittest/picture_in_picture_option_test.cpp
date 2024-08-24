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

void PictureInPictureOptionTest::SetUpTestCase() {
}

void PictureInPictureOptionTest::TearDownTestCase() {
}

void PictureInPictureOptionTest::SetUp() {
}

void PictureInPictureOptionTest::TearDown() {
}

namespace {

/**
 * @tc.name: Context
 * @tc.desc: SetContext/GetContext
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, Context, Function | SmallTest | Level2)
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
HWTEST_F(PictureInPictureOptionTest, PipTemplate, Function | SmallTest | Level2)
{
    sptr<PipOption> option = new PipOption();
    option->SetPipTemplate(100);
    ASSERT_EQ(100, option->GetPipTemplate());
}

/**
 * @tc.name: PiPControlStatus
 * @tc.desc: SetPiPControlStatus/GetControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, PiPControlStatus, Function | SmallTest | Level2)
{
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::PLAY;
    option->SetPiPControlStatus(controlType, status);
    ASSERT_NE(0, option->GetControlStatus().size());
}

/**
 * @tc.name: PiPControlEnable
 * @tc.desc: SetPiPControlEnabled/GetControlEnable
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, PiPControlEnable, Function | SmallTest | Level2)
{
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto enabled = PiPControlStatus::ENABLED;
    option->SetPiPControlEnabled(controlType, enabled);
    ASSERT_NE(0, option->GetControlEnable().size());
}

/**
 * @tc.name: NavigationId
 * @tc.desc: SetNavigationId/GetNavigationId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, NavigationId, Function | SmallTest | Level2)
{
    sptr<PipOption> option = new PipOption();
    std::string navigationId = "abc";
    option->SetNavigationId(navigationId);
    ASSERT_EQ(navigationId, option->GetNavigationId());
}

/**
 * @tc.name: NavigationId
 * @tc.desc: SetNavigationId/GetNavigationId
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, SetGetControlGroupTest, Function | SmallTest | Level2)
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
HWTEST_F(PictureInPictureOptionTest, ContentSize, Function | SmallTest | Level2)
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
HWTEST_F(PictureInPictureOptionTest, NodeController, Function | SmallTest | Level2)
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
HWTEST_F(PictureInPictureOptionTest, TypeNodeRef, Function | SmallTest | Level2)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    option->SetTypeNodeRef(nullptr);
    ASSERT_EQ(option->GetTypeNodeRef(), nullptr);
}

/**
 * @tc.name: TypeNodeEnabled
 * @tc.desc: SetTypeNodeEnabled/IsTypeNodeEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureOptionTest, TypeNodeEnabled, Function | SmallTest | Level2)
{
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    option->SetTypeNodeEnabled(true);
    ASSERT_TRUE(option->IsTypeNodeEnabled());
    option->SetTypeNodeEnabled(false);
    ASSERT_TRUE(!option->IsTypeNodeEnabled());
}
}
}
}