/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/scene_session_manager_lite.h"
#include "session_manager/include/zidl/pip_change_listener_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionManagerLiteTest::SetUpTestCase() {}

void SceneSessionManagerLiteTest::TearDownTestCase() {}

void SceneSessionManagerLiteTest::SetUp() {}

void SceneSessionManagerLiteTest::TearDown() {}

namespace {
/**
 * @tc.name: GetInstance
 * @tc.desc: test function : GetInstance
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, GetInstance, TestSize.Level1)
{
    SceneSessionManagerLite& instance1 = SceneSessionManagerLite::GetInstance();
    EXPECT_NE(nullptr, &instance1);
    SceneSessionManagerLite& instance2 = SceneSessionManagerLite::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: test function : UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    std::string bundleName = "aaa";
    bool isRelease = false;
    auto expectRet = SceneSessionManager::GetInstance().UpdateScreenLockStatusForApp(bundleName, isRelease);
    auto ret = SceneSessionManagerLite::GetInstance().UpdateScreenLockStatusForApp(bundleName, isRelease);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: SendPointerEventForHover
 * @tc.desc: test function : SendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, SendPointerEventForHover, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().SendPointerEventForHover(pointerEvent),
        WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: RegisterWindowPropertyChangeAgent
 * @tc.desc: test function : RegisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, RegisterWindowPropertyChangeAgent, TestSize.Level1)
{
    WindowInfoKey windowInfoKey = WindowInfoKey::MID_SCENE;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto expectRet = SceneSessionManager::GetInstance().RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo,
        windowManagerAgent);
    auto ret = SceneSessionManagerLite::GetInstance().RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo,
        windowManagerAgent);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: UnregisterWindowPropertyChangeAgent
 * @tc.desc: test function : UnregisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UnregisterWindowPropertyChangeAgent, TestSize.Level1)
{
    WindowInfoKey windowInfoKey = WindowInfoKey::MID_SCENE;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto expectRet = SceneSessionManager::GetInstance().UnregisterWindowPropertyChangeAgent(windowInfoKey,
        interestInfo, windowManagerAgent);
    auto ret = SceneSessionManagerLite::GetInstance().UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo,
        windowManagerAgent);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: RecoverWindowPropertyChangeFlag
 * @tc.desc: test function : RecoverWindowPropertyChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, RecoverWindowPropertyChangeFlag, TestSize.Level1)
{
    auto expectRet = SceneSessionManager::GetInstance().RecoverWindowPropertyChangeFlag(0, 0);
    auto ret = SceneSessionManagerLite::GetInstance().RecoverWindowPropertyChangeFlag(0, 0);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: IsFocusWindowParent
 * @tc.desc: test function : IsFocusWindowParent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, IsFocusWindowParent, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    bool isParent = false;
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().IsFocusWindowParent(token, isParent),
        WSError::WS_ERROR_INVALID_PERMISSION);
}

class MockPipChgListener : public PipChangeListenerStub {
public:
    void OnPipStart(int32_t windowId) override {};
};

HWTEST_F(SceneSessionManagerLiteTest, UnregPipChgListenerByScreenId_ShouldStillRetOK_WhenNotReg, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().UnregisterPipChgListenerByScreenId(1), WMError::WM_OK);
}

HWTEST_F(SceneSessionManagerLiteTest, UnregPipChgListenerByScreenId_ShouldRetOK_WhenRegOk, TestSize.Level1)
{
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().RegisterPipChgListenerByScreenId(1, listener), WMError::WM_OK);
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().UnregisterPipChgListenerByScreenId(1), WMError::WM_OK);
}

HWTEST_F(SceneSessionManagerLiteTest, RegisterPipChgListenerByScreenId, TestSize.Level1)
{
    auto result = SceneSessionManagerLite::GetInstance().RegisterPipChgListenerByScreenId(1, nullptr);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);
}

HWTEST_F(SceneSessionManagerLiteTest, SetPipEnableByScreenId, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().SetPipEnableByScreenId(1, true), WMError::WM_OK);
}

HWTEST_F(SceneSessionManagerLiteTest, UnsetPipEnableByScreenId_ShouldRetOk_WhenNotSet, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().UnsetPipEnableByScreenId(1), WMError::WM_OK);
}

HWTEST_F(SceneSessionManagerLiteTest, UnsetPipEnableByScreenId_ShouldRetOk_WhenSetOk, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().SetPipEnableByScreenId(1, true), WMError::WM_OK);
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().UnsetPipEnableByScreenId(1), WMError::WM_OK);
}

/**
 * @tc.name: GetDisplayIdByWindowId01
 * @tc.desc: test function : GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, GetDisplayIdByWindowId01, TestSize.Level1)
{
    std::vector<uint64_t> windowIds;
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().GetDisplayIdByWindowId(windowIds, windowDisplayIdMap),
        WMError::WM_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS