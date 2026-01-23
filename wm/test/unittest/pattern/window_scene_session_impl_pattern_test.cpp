/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <parameters.h>

#include "ability_context_impl.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "scene_board_judgement.h"
#include "session/host/include/scene_session.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void LogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg += msg;
    }
}
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;

class MockWindowChangeListener : public IWindowChangeListener {
public:
    MOCK_METHOD3(OnSizeChange,
                 void(Rect rect, WindowSizeChangeReason reason, const std::shared_ptr<RSTransaction>& rsTransaction));
};

class WindowSceneSessionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:

    static constexpr uint32_t waitSyncInNs = 200000;
};

void WindowSceneSessionImplTest::SetUpTestCase() {}

void WindowSceneSessionImplTest::TearDownTestCase() {}

void WindowSceneSessionImplTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplTest::TearDown()
{
    usleep(waitSyncInNs);
    abilityContext_ = nullptr;
}


namespace {


} // namespace
} // namespace Rosen
} // namespace OHOS
