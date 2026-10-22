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

#include "pointer_event.h"
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
 * @tc.name: SetProcessWatermark
 * @tc.desc: test function : SetProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, SetProcessWatermark, TestSize.Level1)
{
    int32_t pid = 100;
    const std::string watermarkName = "test";
    bool isEnabled = true;
    auto expectRet = SceneSessionManager::GetInstance().SetProcessWatermark(pid, watermarkName, isEnabled);
    auto ret = SceneSessionManagerLite::GetInstance().SetProcessWatermark(pid, watermarkName, isEnabled);
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

/**
 * @tc.name: UnregisterPipChgListenerByScreenId
 * @tc.desc: test function : UnregisterPipChgListenerByScreenId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UnregisterPipChgListenerByScreenId, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().UnregisterPipChgListenerByScreenId(1),
        WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: RegisterPipChgListenerByScreenId
 * @tc.desc: test function : RegisterPipChgListenerByScreenId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, RegisterPipChgListenerByScreenId, TestSize.Level1)
{
    auto result = SceneSessionManagerLite::GetInstance().RegisterPipChgListenerByScreenId(1, nullptr);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetPipEnableByScreenId
 * @tc.desc: test function : SetPipEnableByScreenId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, SetPipEnableByScreenId, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().SetPipEnableByScreenId(1, true),
        WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: UnsetPipEnableByScreenId
 * @tc.desc: test function : UnsetPipEnableByScreenId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UnsetPipEnableByScreenId, TestSize.Level1)
{
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().UnsetPipEnableByScreenId(1),
        WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: UpdateRogWindowConfig02
 * @tc.desc: test function : UpdateRogWindowConfig with valid params
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateRogWindowConfig02, TestSize.Level1)
{
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = {"com.test.app1", "com.test.app2"};
    windowConfig.width = 1920;
    windowConfig.height = 1080;
    windowConfig.dpi = 480;
    windowConfig.scale = 1.5f;
    auto ret = SceneSessionManagerLite::GetInstance().UpdateRogWindowConfig(windowConfig);
    auto expectRet = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: UpdateRogWindowConfig03
 * @tc.desc: test function : UpdateRogWindowConfig with different configs
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateRogWindowConfig03, TestSize.Level1)
{
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = {"app1", "app2", "app3"};
    windowConfig.width = 2560;
    windowConfig.height = 1440;
    windowConfig.dpi = 640;
    windowConfig.scale = 2.0f;
    auto ret = SceneSessionManagerLite::GetInstance().UpdateRogWindowConfig(windowConfig);
    auto expectRet = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: UpdateRogWindowConfig05
 * @tc.desc: test function : UpdateRogWindowConfig with large app list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateRogWindowConfig05, TestSize.Level1)
{
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = std::vector<std::string>(100, "com.test.app");
    windowConfig.width = 1920;
    windowConfig.height = 1080;
    windowConfig.dpi = 480;
    windowConfig.scale = 1.5f;
    auto ret = SceneSessionManagerLite::GetInstance().UpdateRogWindowConfig(windowConfig);
    auto expectRet = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: UpdateRogWindowConfig06
 * @tc.desc: test function : UpdateRogWindowConfig with 720P configuration
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateRogWindowConfig06, TestSize.Level1)
{
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = {"com.test.720p.app1", "com.test.720p.app2"};
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.dpi = 320;
    windowConfig.scale = 1.0f;
    auto ret = SceneSessionManagerLite::GetInstance().UpdateRogWindowConfig(windowConfig);
    auto expectRet = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: UpdateRogWindowConfig07
 * @tc.desc: test function : UpdateRogWindowConfig with 1080P configuration
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateRogWindowConfig07, TestSize.Level1)
{
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = {"com.test.1080p.app"};
    windowConfig.width = 1920;
    windowConfig.height = 1080;
    windowConfig.dpi = 480;
    windowConfig.scale = 1.5f;
    auto ret = SceneSessionManagerLite::GetInstance().UpdateRogWindowConfig(windowConfig);
    auto expectRet = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: UpdateRogWindowConfig08
 * @tc.desc: test function : UpdateRogWindowConfig with 2K configuration
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, UpdateRogWindowConfig08, TestSize.Level1)
{
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = {"com.test.2k.app"};
    windowConfig.width = 2560;
    windowConfig.height = 1440;
    windowConfig.dpi = 640;
    windowConfig.scale = 2.0f;
    auto ret = SceneSessionManagerLite::GetInstance().UpdateRogWindowConfig(windowConfig);
    auto expectRet = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: RegisterUpdateRogWindowConfigCallback01
 * @tc.desc: test function : RegisterUpdateRogWindowConfigCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, RegisterUpdateRogWindowConfigCallback01, TestSize.Level1)
{
    auto callback = [](const RogWindowConfig& config) {};
    SceneSessionManager::GetInstance().RegisterUpdateRogWindowConfigCallback(callback);
    RogWindowConfig windowConfig;
    windowConfig.xhdpiAppList = {"com.test.callback.app"};
    windowConfig.width = 1920;
    windowConfig.height = 1080;
    windowConfig.dpi = 480;
    windowConfig.scale = 1.5f;
    auto ret = SceneSessionManager::GetInstance().UpdateRogWindowConfig(windowConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: RecoverProcessWatermark01
 * @tc.desc: test function : RecoverProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, RecoverProcessWatermark01, TestSize.Level1)
{
    int32_t pid = 100;
    std::string watermarkName = "testWatermark";
    auto ret = SceneSessionManagerLite::GetInstance().RecoverProcessWatermark(pid, watermarkName);
    auto expectRet = SceneSessionManager::GetInstance().RecoverProcessWatermark(pid, watermarkName);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: RecoverProcessWatermark02
 * @tc.desc: test function : RecoverProcessWatermark with empty name
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLiteTest, RecoverProcessWatermark02, TestSize.Level1)
{
    int32_t pid = 100;
    std::string watermarkName = "";
    auto ret = SceneSessionManagerLite::GetInstance().RecoverProcessWatermark(pid, watermarkName);
    auto expectRet = SceneSessionManager::GetInstance().RecoverProcessWatermark(pid, watermarkName);
    EXPECT_EQ(ret, expectRet);
}
} // namespace
} // namespace Rosen
} // namespace OHOS