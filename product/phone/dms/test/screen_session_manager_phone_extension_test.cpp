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
#include <regex>
#include "screen_session_manager_phone_extension.h"
#include "screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "iconsumer_surface.h"
#include "connection/screen_cast_connection.h"
#include "screen_scene_config.h"
#include <surface.h>
#include "scene_board_judgement.h"
#include "fold_screen_state_internel.h"
#include "common_test_utils.h"
#include "os_account_manager.h"
#include "screen_session_manager_client.h"
#include "mock_accesstoken_kit.h"
#include <parameter.h>
#include <parameters.h>
#include "window_manager_hilog.h"
#include "test_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace PhoneExtension {
namespace {
const ScreenId SCREENID = 1000;
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
namespace {
std::string g_errLog;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    g_errLog += msg;
}
}
class ScreenSessionManagerPhoneExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<ScreenSessionManagerExt> ssm_;
    void SetAceessTokenPermission(const std::string processName);
};

sptr<ScreenSessionManagerExt> ScreenSessionManagerPhoneExtensionTest::ssm_ = nullptr;

void ScreenSessionManagerPhoneExtensionTest::SetUpTestCase()
{
    ssm_ = new ScreenSessionManagerExt();
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void ScreenSessionManagerPhoneExtensionTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerPhoneExtensionTest::SetUp()
{
}

void ScreenSessionManagerPhoneExtensionTest::TearDown()
{
    usleep(SLEEP_TIME_IN_US);
}

namespace {
/**
 * @tc.name: ScreenSessionManagerExt
 * @tc.desc: ScreenSessionManagerExt
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerPhoneExtensionTest, ScreenSessionManagerExt, TestSize.Level1)
{
    ScreenSessionManager::SetInstance(&ScreenSessionManagerExt::GetInstance());
    auto* expectedInstance = &ScreenSessionManagerExt::GetInstance();
    EXPECT_NE(expectedInstance, nullptr);
}
} // namespace
} // namespace PhoneExtension
} // namespace Rosen
} // namespace OHOS