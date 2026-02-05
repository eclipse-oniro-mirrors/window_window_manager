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
#include "screen_aod_plugin.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace Rosen {
namespace {
    static void* g_handle = nullptr;
    using IsInAodFunc = bool (*)();
    using StopAodFunc = bool (*)(int32_t);
    static IsInAodFunc g_isInAodFunc = nullptr;
    static StopAodFunc g_stopAodFunc = nullptr;
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog = msg;
    }
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string PLUGIN_AOD_SO_PATH = "/system/lib64/libaod_native.z.so";
#else
const std::string PLUGIN_AOD_SO_PATH = "/system/lib/libaod_native.z.so";
#endif
}
class IsInAodTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};
 
void IsInAodTest::SetUp() {}
 
void IsInAodTest::TearDown() {}
 
namespace {
 
/**
 * @tc.name: ATC_IsInAodAndStopAod01
 * @tc.desc: ATC_IsInAodAndStopAod01
 * @tc.type: FUNC
 */
HWTEST_F(IsInAodTest, ATC_IsInAodAndStopAod01, TestSize.Level0)
{
    g_handle = nullptr;
    UnloadAodLib();
    bool result = IsInAod();
    EXPECT_FALSE(result);
    result = StopAod(1);
    EXPECT_FALSE(result);
    LoadAodLib();
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    IsInAod();
    EXPECT_TRUE(g_errLog.find("dlsym error") == std::string::npos);
    g_errLog.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: LoadAodLib_GHandle_NotNullptr
 * @tc.desc: LoadAodLib_GHandle_NotNullptr
 * @tc.type: FUNC
 */
HWTEST_F(IsInAodTest, LoadAodLib_GHandle_NotNullptr, TestSize.Level0)
{
    g_handle = dlopen(PLUGIN_AOD_SO_PATH.c_str(), RTLD_LAZY);
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    bool result = LoadAodLib();
    EXPECT_TRUE(result);
    EXPECT_TRUE(g_errLog.find("aod plugin has already exits") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: StopAod_GStopAodFunc
 * @tc.desc: StopAod_GStopAodFunc
 * @tc.type: FUNC
 */
HWTEST_F(IsInAodTest, StopAod_GStopAodFunc, TestSize.Level0)
{
    LoadAodLib();
    IsInAod();
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    StopAod(1);
    EXPECT_TRUE(g_errLog.find("retry cnt") != std::string::npos);
    LOG_SetCallback(nullptr);
}
}
} // namespace Rosen
} // namespace OHOS