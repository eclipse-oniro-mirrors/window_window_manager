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
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/zidl/session_interface.h"
#include "mock/mock_accesstoken_kit.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionEventTest::SetUpTestCase() {}

void SceneSessionEventTest::TearDownTestCase() {}

void SceneSessionEventTest::SetUp() {}

void SceneSessionEventTest::TearDown() {}

namespace {
/**
 * @tc.name: LockCursor
 * @tc.desc: LockCursor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionEventTest, LockCursor, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "LockCursor";
    info.bundleName_ = "LockCursor";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::vector<int32_t> parameters;

    // The test does not have the permission
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);

    // Test CheckParameters: The format is incorrect(size<1).
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    
    // Test CheckParameters: The format is incorrect(length error).
    parameters.emplace_back(LOCK_CURSOR_LENGTH + 1);
    ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Test CheckParameters: The format is incorrect(size error).
    parameters.clear();
    parameters.emplace_back(LOCK_CURSOR_LENGTH);
    ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Test：session is null
    parameters.clear();
    parameters.emplace_back(LOCK_CURSOR_LENGTH);
    parameters.emplace_back(1);
    parameters.emplace_back(1);
    sceneSession->persistentId_ = 2;
    ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_SESSION);

    // test normal process
    parameters.clear();
    parameters.emplace_back(LOCK_CURSOR_LENGTH);
    parameters.emplace_back(1);
    parameters.emplace_back(1);
    sceneSession->persistentId_ = 1;
    ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);

    // test normal process
    parameters.clear();
    parameters.emplace_back(LOCK_CURSOR_LENGTH);
    parameters.emplace_back(1);
    parameters.emplace_back(0);
    ret = sceneSession->LockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UnlockCursor
 * @tc.desc: UnlockCursor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionEventTest, UnlockCursor, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UnlockCursor";
    info.bundleName_ = "UnlockCursor";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::vector<int32_t> parameters;

    // The test does not have the permission.
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);

    // test CheckParameters: The format is incorrect(size<1).
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    
    // Test CheckParameters: The format is incorrect(length error).
    parameters.emplace_back(UNLOCK_CURSOR_LENGTH + 1);
    ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Test CheckParameters: The format is incorrect(size error).
    parameters.clear();
    parameters.emplace_back(UNLOCK_CURSOR_LENGTH);
    ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Test: session is null.
    parameters.clear();
    parameters.emplace_back(UNLOCK_CURSOR_LENGTH);
    parameters.emplace_back(1);
    sceneSession->persistentId_ = 2;
    ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_SESSION);

    // test normal process
    parameters.clear();
    parameters.emplace_back(UNLOCK_CURSOR_LENGTH);
    parameters.emplace_back(1);
    sceneSession->persistentId_ = 1;
    ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);

    // test normal process(repeat)
    parameters.clear();
    parameters.emplace_back(UNLOCK_CURSOR_LENGTH);
    parameters.emplace_back(1);
    ret = sceneSession->UnlockCursor(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetReceiveDragEventEnabled
 * @tc.desc: SetReceiveDragEventEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionEventTest, SetReceiveDragEventEnabled, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetReceiveDragEventEnabled";
    info.bundleName_ = "SetReceiveDragEventEnabled";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::vector<int32_t> parameters;

    // Test CheckParameters: The format is incorrect(size<1).
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    auto ret = sceneSession->SetReceiveDragEventEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    
    // Test CheckParameters: The format is incorrect(length error).
    parameters.emplace_back(SET_RECEIVE_DRAG_EVENT_LENGTH + 1);
    ret = sceneSession->SetReceiveDragEventEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Test CheckParameters: The format is incorrect(size error).
    parameters.clear();
    parameters.emplace_back(SET_RECEIVE_DRAG_EVENT_LENGTH);
    ret = sceneSession->SetReceiveDragEventEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // test normal process
    parameters.clear();
    parameters.emplace_back(SET_RECEIVE_DRAG_EVENT_LENGTH);
    parameters.emplace_back(1);
    sceneSession->persistentId_ = 1;
    ret = sceneSession->SetReceiveDragEventEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);

    // test normal process
    parameters.clear();
    parameters.emplace_back(SET_RECEIVE_DRAG_EVENT_LENGTH);
    parameters.emplace_back(0);
    sceneSession->persistentId_ = 1;
    ret = sceneSession->SetReceiveDragEventEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetSeparationTouchEnabled
 * @tc.desc: SetSeparationTouchEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionEventTest, SetSeparationTouchEnabled, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSeparationTouchEnabled";
    info.bundleName_ = "SetSeparationTouchEnabled";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::vector<int32_t> parameters;

    // Test CheckParameters: The format is incorrect(size<1).
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    auto ret = sceneSession->SetSeparationTouchEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    
    // Test CheckParameters: The format is incorrect(length error).
    parameters.emplace_back(WINDOW_SEPARATION_TOUCH_ENABLED_LENGTH + 1);
    ret = sceneSession->SetSeparationTouchEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Test CheckParameters: The format is incorrect(size error).
    parameters.clear();
    parameters.emplace_back(WINDOW_SEPARATION_TOUCH_ENABLED_LENGTH);
    ret = sceneSession->SetSeparationTouchEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // test normal process
    parameters.clear();
    parameters.emplace_back(WINDOW_SEPARATION_TOUCH_ENABLED_LENGTH);
    parameters.emplace_back(1);
    sceneSession->persistentId_ = 1;
    ret = sceneSession->SetSeparationTouchEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);

    // test normal process
    parameters.clear();
    parameters.emplace_back(WINDOW_SEPARATION_TOUCH_ENABLED_LENGTH);
    parameters.emplace_back(0);
    sceneSession->persistentId_ = 1;
    ret = sceneSession->SetSeparationTouchEnabled(parameters);
    EXPECT_EQ(ret, WMError::WM_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
