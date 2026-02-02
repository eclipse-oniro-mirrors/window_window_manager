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

#include <system_ability_definition.h>

#include "screen_session_manager.h"
#include "setting_provider.h"
#include "hilog/log.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10000;
}
class SettingProviderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SettingProviderTest::SetUpTestCase()
{
}

void SettingProviderTest::TearDownTestCase()
{
}

void SettingProviderTest::SetUp()
{
}

void SettingProviderTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: GetIntValue
 * @tc.desc: test function : GetIntValue
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, GetIntValue, TestSize.Level1)
{
    std::string key = "";
    int32_t value = 0;
    EXPECT_NE(SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).GetIntValue(key, value), ERR_OK);
}

/**
 * @tc.name: GetLongValue
 * @tc.desc: test function : GetLongValue
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, GetLongValue, TestSize.Level1)
{
    std::string key = "";
    int64_t value = 0;
    EXPECT_NE(SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).GetLongValue(key, value), ERR_OK);
}

/**
 * @tc.name: GetBoolValue
 * @tc.desc: test function : GetBoolValue
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, GetBoolValue, TestSize.Level1)
{
    std::string key = "";
    bool value = false;
    EXPECT_NE(SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).GetBoolValue(key, value), ERR_OK);
}

/**
 * @tc.name: ExecRegisterCb01
 * @tc.desc: test function : ExecRegisterCb
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, ExecRegisterCb01, TestSize.Level1)
{
    sptr<SettingObserver> observer = nullptr;
    SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).ExecRegisterCb(observer);
    EXPECT_EQ(observer, nullptr);
}

/**
 * @tc.name: ExecRegisterCb02
 * @tc.desc: test function : ExecRegisterCb
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, ExecRegisterCb02, TestSize.Level1)
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    auto observer = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).CreateObserver(
        "settings.power.suspend_sources", updateFunc);
    SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).ExecRegisterCb(observer);
    EXPECT_NE(observer, nullptr);
}

/**
 * @tc.name: RegisterObserver_ObserverNullptr
 * @tc.desc: test function : RegisterObserver_ObserverNullptr
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, RegisterObserver_ObserverNullptr, TestSize.Level1)
{
    sptr<SettingObserver> observer = nullptr;
    ErrCode res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).RegisterObserver(observer);
    EXPECT_EQ(res, ERR_NO_INIT);
}

/**
 * @tc.name: UnregisterObserver_ObserverNullptr
 * @tc.desc: test function : UnregisterObserver_ObserverNullptr
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, UnregisterObserver_ObserverNullptr, TestSize.Level1)
{
    sptr<SettingObserver> observer = nullptr;
    ErrCode res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserver(observer);
    EXPECT_EQ(res, ERR_NO_INIT);
}

/**
 * @tc.name: UnregisterObserver
 * @tc.desc: test function : UnregisterObserver
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, UnregisterObserver, TestSize.Level1)
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    auto observer = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).CreateObserver(
        "settings.power.suspend_sources", updateFunc);
    EXPECT_EQ(SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserver(observer), ERR_NO_INIT);
}

/**
 * @tc.name: PutStringValue01
 * @tc.desc: test function : PutStringValue
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, PutStringValue01, TestSize.Level1)
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    std::string key = "key";
    std::string value = "value";
    bool needNotify = true;
    auto observer = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).CreateObserver(
        "settings.power.suspend_sources", updateFunc);
    EXPECT_EQ(
        SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).PutStringValue(key, value, needNotify),
        ERR_NO_INIT);
}

/**
 * @tc.name: PutStringValue02
 * @tc.desc: test function : PutStringValue
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, PutStringValue02, TestSize.Level1)
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    std::string key = "key";
    std::string value = "value";
    bool needNotify = false;
    auto observer = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).CreateObserver(
        "settings.power.suspend_sources", updateFunc);
    EXPECT_EQ(
        SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).PutStringValue(key, value, needNotify),
        ERR_NO_INIT);
}

/**
 * @tc.name: AssembleUriMultiUser
 * @tc.desc: test function : AssembleUriMultiUser
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, AssembleUriMultiUser, TestSize.Level1)
{
    int32_t saveSettings = ScreenSessionManager::GetInstance().currentUserIdForSettings_;
    ScreenSessionManager::GetInstance().currentUserIdForSettings_ = 1;

    // key == DURING_CALL_KEY
    std::string key = "during_call_state";
    auto res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).AssembleUriMultiUser(key);
    EXPECT_TRUE(res.ToString().find("USER_SETTINGSDATA_SECURE_") != std::string::npos);

    // key != DURING_CALL_KEY
    key = "ut_test";
    res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).AssembleUriMultiUser(key);
    EXPECT_FALSE(res.ToString().find("USER_SETTINGSDATA_SECURE_") != std::string::npos);

    //userId = 0
    ScreenSessionManager::GetInstance().currentUserIdForSettings_ = 0;
    key = "ut_test";
    res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).AssembleUriMultiUser(key);
    EXPECT_TRUE(res.ToString().find("datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA") !=
        std::string::npos);

    key = "wallpaperAodDisplay";
    res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).AssembleUriMultiUser(key);
    EXPECT_TRUE(res.ToString().find("wallpaperAodDisplay") != std::string::npos);

    ScreenSessionManager::GetInstance().currentUserIdForSettings_ = saveSettings;
}

/**
 * @tc.name: RegisterObserverByTable_ObserverNullptr
 * @tc.desc: test function : RegisterObserverByTable_ObserverNullptr
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, RegisterObserverByTable_ObserverNullptr, TestSize.Level1)
{
    sptr<SettingObserver> observer = nullptr;
    std::string tableName = "table_name";
    ErrCode res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).RegisterObserverByTable(observer, tableName);
    EXPECT_EQ(res, ERR_NO_INIT);
}

/**
 * @tc.name: UnregisterObserverByTable_ObserverNullptr
 * @tc.desc: test function : UnregisterObserverByTable_ObserverNullptr
 * @tc.type: FUNC
 */
HWTEST_F(SettingProviderTest, UnregisterObserverByTable_ObserverNullptr, TestSize.Level1)
{
    sptr<SettingObserver> observer = nullptr;
    std::string tableName = "table_name";
    ErrCode res = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserverByTable(observer, tableName);
    EXPECT_EQ(res, ERR_NO_INIT);
}
}
}
}