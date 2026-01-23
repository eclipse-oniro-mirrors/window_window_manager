/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <regex>

#include "pointer_event.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionManagerEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionManagerEventTest::SetUpTestCase()
{
}

void SceneSessionManagerEventTest::TearDownTestCase()
{
}

void SceneSessionManagerEventTest::SetUp()
{
}

void SceneSessionManagerEventTest::TearDown()
{
}
namespace {
/**
 * @tc.name: SendTouchEventTestWithNullPointerEvent
 * @tc.desc: SendTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerEventTest, SendTouchEventTestWithNullPointerEvent, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm_ = &SceneSessionManager::GetInstance();
    ASSERT_NE(nullptr, ssm_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;

    auto ret = ssm_->SendTouchEvent(pointerEvent, 0);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: SendTouchEventTestWithValidPointerEvent
 * @tc.desc: SendTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerEventTest, SendTouchEventTestWithValidPointerEvent, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm_ = &SceneSessionManager::GetInstance();
    ASSERT_NE(nullptr, ssm_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.pointerId_ = 0;
    pointerEvent->pointerId_ = 0;
    pointerEvent->AddPointerItem(pointerItem);

    auto ret = ssm_->SendTouchEvent(pointerEvent, 0);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: SendTouchEventTestWithInvalidPointerEvent
 * @tc.desc: SendTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerEventTest, SendTouchEventTestWithInvalidPointerEvent, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm_ = &SceneSessionManager::GetInstance();
    ASSERT_NE(nullptr, ssm_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.pointerId_ = 0;
    pointerEvent->pointerId_ = 1;
    pointerEvent->AddPointerItem(pointerItem);

    auto ret = ssm_->SendTouchEvent(pointerEvent, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: SendAxisEventTestWithNullPointerEvent
 * @tc.desc: SendAxisEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerEventTest, SendAxisEventTestWithNullPointerEvent, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm_ = &SceneSessionManager::GetInstance();
    ASSERT_NE(nullptr, ssm_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;

    auto ret = ssm_->SendAxisEvent(pointerEvent);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: SendAxisEventTestWithValidPointerEvent
 * @tc.desc: SendAxisEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerEventTest, SendAxisEventTestWithValidPointerEvent, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm_ = &SceneSessionManager::GetInstance();
    ASSERT_NE(nullptr, ssm_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.pointerId_ = 0;
    pointerEvent->pointerId_ = 0;
    pointerEvent->AddPointerItem(pointerItem);

    auto ret = ssm_->SendAxisEvent(pointerEvent);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: SendAxisEventTestWithInvalidPointerEvent
 * @tc.desc: SendAxisEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerEventTest, SendAxisEventTestWithInvalidPointerEvent, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm_ = &SceneSessionManager::GetInstance();
    ASSERT_NE(nullptr, ssm_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.pointerId_ = 0;
    pointerEvent->pointerId_ = 1;
    pointerEvent->AddPointerItem(pointerItem);

    auto ret = ssm_->SendAxisEvent(pointerEvent);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
}
}
}
}