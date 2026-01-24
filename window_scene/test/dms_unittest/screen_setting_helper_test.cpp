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

#include "screen_session_manager/include/screen_session_manager.h"
#include "screen_session_manager/include/screen_setting_helper.h"
#include "window_manager_hilog.h"
#include "scene_board_judgement.h"
#include "nlohmann/json.hpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 100000;
}
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}

class ScreenSettingHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSettingHelperTest::SetUpTestCase()
{
}

void ScreenSettingHelperTest::TearDownTestCase()
{
}

void ScreenSettingHelperTest::SetUp()
{
}

void ScreenSettingHelperTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
    /**
     * @tc.name: RegisterSettingDpiObserver
     * @tc.desc: RegisterSettingDpiObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingDpiObserver, TestSize.Level1)
    {
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::RegisterSettingDpiObserver(func);
        ScreenSettingHelper::dpiObserver_ = nullptr;
        ASSERT_EQ(ScreenSettingHelper::dpiObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingDpiObserver01
     * @tc.desc: UnregisterSettingDpiObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingDpiObserver01, TestSize.Level1)
    {
        ScreenSettingHelper::dpiObserver_ = new SettingObserver;
        ScreenSettingHelper::UnregisterSettingDpiObserver();
        ASSERT_EQ(ScreenSettingHelper::dpiObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingDpiObserver02
     * @tc.desc: UnregisterSettingDpiObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingDpiObserver02, TestSize.Level1)
    {
        ScreenSettingHelper::dpiObserver_ = nullptr;
        ScreenSettingHelper::UnregisterSettingDpiObserver();
        ASSERT_EQ(ScreenSettingHelper::dpiObserver_, nullptr);
    }

    /**
     * @tc.name: GetSettingDpi01
     * @tc.desc: GetSettingDpi01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingDpi01, TestSize.Level1)
    {
        uint32_t dpi = 0;
        std::string key = "test";
        bool ret = ScreenSettingHelper::GetSettingDpi(dpi, key);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: RegisterSettingCastObserver01
     * @tc.desc: RegisterSettingCastObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingCastObserver01, TestSize.Level1)
    {
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::castObserver_ = new SettingObserver;
        ScreenSettingHelper::RegisterSettingCastObserver(func);
        ASSERT_NE(ScreenSettingHelper::castObserver_, nullptr);
    }

    /**
     * @tc.name: RegisterSettingCastObserver02
     * @tc.desc: RegisterSettingCastObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingCastObserver02, TestSize.Level1)
    {
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::castObserver_ = nullptr;
        ScreenSettingHelper::RegisterSettingCastObserver(func);
        ASSERT_EQ(ScreenSettingHelper::castObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingCastObserver01
     * @tc.desc: UnregisterSettingCastObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingCastObserver01, TestSize.Level1)
    {
        ScreenSettingHelper::castObserver_ = nullptr;
        ScreenSettingHelper::UnregisterSettingCastObserver();
        ASSERT_EQ(ScreenSettingHelper::castObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingCastObserver02
     * @tc.desc: UnregisterSettingCastObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingCastObserver02, TestSize.Level1)
    {
        ScreenSettingHelper::castObserver_ = new SettingObserver;
        ScreenSettingHelper::UnregisterSettingCastObserver();
        ASSERT_EQ(ScreenSettingHelper::castObserver_, nullptr);
    }

    /**
     * @tc.name: GetSettingCast01
     * @tc.desc: GetSettingCast01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingCast01, TestSize.Level1)
    {
        bool enable = true;
        std::string key = "test";
        bool ret = ScreenSettingHelper::GetSettingCast(enable, key);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetSettingCast02
     * @tc.desc: GetSettingCast02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingCast02, TestSize.Level1)
    {
        if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
            GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
        }
        bool enable = true;
        std::string key = "default_display_dpi";
        bool ret = ScreenSettingHelper::GetSettingCast(enable, key);
        ASSERT_TRUE(ret);
    }

    /**
     * @tc.name: UnregisterSettingRotationObserver01
     * @tc.desc: UnregisterSettingRotationObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingRotationObserver01, TestSize.Level1)
    {
        ScreenSettingHelper::rotationObserver_ = nullptr;
        ASSERT_EQ(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingRotationObserver02
     * @tc.desc: UnregisterSettingRotationObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingRotationObserver02, TestSize.Level1)
    {
        ScreenSettingHelper::rotationObserver_ = new SettingObserver;
        ASSERT_NE(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: SetSettingRotation
     * @tc.desc: SetSettingRotation
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, SetSettingRotation, TestSize.Level1)
    {
        int32_t rotation = 180;
        ScreenSettingHelper::SetSettingRotation(rotation);
        ASSERT_EQ(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: GetSettingRotation01
     * @tc.desc: GetSettingRotation01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotation01, TestSize.Level1)
    {
        int32_t rotation = 0;
        std::string key = "test";
        auto result = ScreenSettingHelper::GetSettingRotation(rotation, key);
        ASSERT_EQ(result, false);
    }

    /**
     * @tc.name: GetSettingRotation02
     * @tc.desc: GetSettingRotation02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotation02, TestSize.Level1)
    {
        if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
            GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
        }
        int32_t rotation = 0;
        std::string key = "default_display_dpi";
        auto result = ScreenSettingHelper::GetSettingRotation(rotation, key);
        ASSERT_NE(result, true);
    }

    /**
     * @tc.name: SetSettingRotationScreenId
     * @tc.desc: SetSettingRotationScreenId
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, SetSettingRotationScreenId, TestSize.Level1)
    {
        int32_t screenId = 0;
        ScreenSettingHelper::SetSettingRotationScreenId(screenId);
        ASSERT_NE(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: GetSettingRotationScreenID01
     * @tc.desc: GetSettingRotationScreenID01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotationScreenID01, TestSize.Level1)
    {
        int32_t screenId = 0;
        std::string key = "test";
        auto result = ScreenSettingHelper::GetSettingRotationScreenID(screenId, key);
        ASSERT_EQ(result, false);
    }

    /**
     * @tc.name: GetSettingRotationScreenID02
     * @tc.desc: GetSettingRotationScreenID02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotationScreenID02, TestSize.Level1)
    {
        int32_t screenId = 0;
        std::string key = "screen_rotation_screen_id_value";
        auto result = ScreenSettingHelper::GetSettingRotationScreenID(screenId, key);
        ASSERT_EQ(result, false);
    }

    /**
     * @tc.name: GetSettingValue
     * @tc.desc: GetSettingValue
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingValue, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        uint32_t value = 0;
        std::string key = "test";
        bool ret = screenSettingHelper.GetSettingValue(value, key);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetSettingValueString
     * @tc.desc: GetSettingValueString
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingValueString, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string value;
        std::string key = "test";
        bool ret = screenSettingHelper.GetSettingValue(key, value);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetSettingValueBool
     * @tc.desc: GetSettingValueBool
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingValueBool, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        bool value = false;
        std::string key = "test";
        bool ret = screenSettingHelper.GetSettingValue(key, value);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: RemoveInvalidChar01
     * @tc.desc: RemoveInvalidChar Test01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RemoveInvalidChar01, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string test_str = "test";
        auto ret = screenSettingHelper.RemoveInvalidChar(test_str);
        ASSERT_EQ(ret, "");
    }

    /**
     * @tc.name: RemoveInvalidChar02
     * @tc.desc: RemoveInvalidChar Test02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RemoveInvalidChar02, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string test_str = "test 2.0 ,";
        auto ret = screenSettingHelper.RemoveInvalidChar(test_str);
        ASSERT_EQ(ret, " 2.0 ,");
    }

    /**
     * @tc.name: SplitString
     * @tc.desc: SplitString Test01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, SplitString01, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::vector<std::string> splitValues = {"split", "test"};
        std::string input = "";
        char delimiter = ',';
        auto ret = screenSettingHelper.SplitString(splitValues, input, delimiter);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: SplitString
     * @tc.desc: SplitString Test02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, SplitString02, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::vector<std::string> splitValues = {};
        std::string input = "test, str";
        char delimiter = ',';
        auto ret = screenSettingHelper.SplitString(splitValues, input, delimiter);
        ASSERT_TRUE(ret);
    }

    /**
     * @tc.name: IsNumber01
     * @tc.desc: IsNumber01 Test
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, IsNumber01, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string test_str = "12.34";
        auto ret = screenSettingHelper.IsNumber(test_str);
        ASSERT_TRUE(ret);
    }

    /**
     * @tc.name: IsNumber02
     * @tc.desc: IsNumber02 Test
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, IsNumber02, TestSize.Level1)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string test_str = "test";
        auto ret = screenSettingHelper.IsNumber(test_str);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: ConvertStrToUInt64Test01
     * @tc.desc: ConvertStrToUInt64Test01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, ConvertStrToUInt64Test01, TestSize.Level1)
    {
        uint64_t num = 0;
        std::string str = "fvcea";
        bool ret = ScreenSettingHelper::ConvertStrToUint64(str, num);
        ASSERT_FALSE(ret);
        ASSERT_EQ(num, 0);
    }

    /**
     * @tc.name: ConvertStrToUInt64Test02
     * @tc.desc: ConvertStrToUInt64Test02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, ConvertStrToUInt64Test02, TestSize.Level1)
    {
        uint64_t num = 0;
        std::string str = "2349z";
        bool ret = ScreenSettingHelper::ConvertStrToUint64(str, num);
        ASSERT_FALSE(ret);
        ASSERT_EQ(num, 0);
    }

    /**
     * @tc.name: ConvertStrToUInt64Test03
     * @tc.desc: ConvertStrToUInt64Test03
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, ConvertStrToUint64Test03, TestSize.Level1)
    {
        uint64_t num = 0;
        std::string str = "2349";
        bool ret = ScreenSettingHelper::ConvertStrToUint64(str, num);
        ASSERT_TRUE(ret);
        ASSERT_EQ(num, 2349);
    }

    /**
     * @tc.name: RegisterSettingWireCastObserver01
     * @tc.desc: RegisterSettingWireCastObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingWireCastObserver01, TestSize.Level1)
    {
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::wireCastObserver_ = new SettingObserver;
        ScreenSettingHelper::RegisterSettingWireCastObserver(func);
        ASSERT_NE(ScreenSettingHelper::wireCastObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingWireCastObserver01
     * @tc.desc: UnregisterSettingWireCastObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingWireCastObserver01, TestSize.Level1)
    {
        ScreenSettingHelper::wireCastObserver_ = nullptr;
        ScreenSettingHelper::UnregisterSettingWireCastObserver();
        ASSERT_EQ(ScreenSettingHelper::wireCastObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingWireCastObserver02
     * @tc.desc: UnregisterSettingWireCastObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingWireCastObserver02, TestSize.Level1)
    {
        ScreenSettingHelper::wireCastObserver_ = new SettingObserver;
        ScreenSettingHelper::UnregisterSettingWireCastObserver();
        ASSERT_EQ(ScreenSettingHelper::wireCastObserver_, nullptr);
    }

    /**
     * @tc.name: UpdateScreenModeTest
     * @tc.desc: Test UpdateScreenMode for mode not equl nomal mode then return false
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UpdateScreenMode, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        uint32_t mode = 3; //unnomal mode
        bool isExternal = true;
        auto ret = ScreenSettingHelper::UpdateScreenMode(info, mode, isExternal);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: UpdateScreenModeTest001
     * @tc.desc: Test UpdateScreenMode for mode equl SCREEN_MAIN_IN_DATA then return true
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UpdateScreenModeTest001, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        uint32_t mode = 0; //mode is SCREEN_MAIN_IN_DATA
        bool isExternal = true;
        auto ret = ScreenSettingHelper::UpdateScreenMode(info, mode, isExternal);
        ASSERT_TRUE(ret);
        ASSERT_EQ(info.isExtendMain, isExternal);
    }

    /**
     * @tc.name: UpdateScreenModeTest002
     * @tc.desc: Test UpdateScreenMode for mode not equl SCREEN_MIRROR_IN_DATA then return true
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UpdateScreenModeTest002, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        uint32_t mode = 1; //mode is SCREEN_MIRROR_IN_DATA
        bool isExternal = true;
        auto ret = ScreenSettingHelper::UpdateScreenMode(info, mode, isExternal);
        ASSERT_TRUE(ret);
        ASSERT_EQ(info.multiScreenMode, MultiScreenMode::SCREEN_MIRROR);
    }

    /**
     * @tc.name: UpdateScreenModeTest003
     * @tc.desc: Test UpdateScreenMode for mode not equl SCREEN_EXTEND_IN_DATA then return true
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UpdateScreenModeTest003, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        uint32_t mode = 2; //mode is SCREEN_EXTEND_IN_DATA
        bool isExternal = true;
        auto ret = ScreenSettingHelper::UpdateScreenMode(info, mode, isExternal);
        ASSERT_TRUE(ret);
        ASSERT_EQ(info.multiScreenMode, MultiScreenMode::SCREEN_EXTEND);
    }

    /**
     * @tc.name: GetScreenActiveMode001
     * @tc.desc: Test GetScreenActiveMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenActiveMode001, Function | SmallTest | Level3)
    {
        SupportedScreenModes info;
        string inputString = "1 11";
        auto ret = ScreenSettingHelper::GetScreenActiveMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenActiveMode002
     * @tc.desc: Test GetScreenActiveMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenActiveMode002, Function | SmallTest | Level3)
    {
        SupportedScreenModes info;
        string inputString = "E";
        auto ret = ScreenSettingHelper::GetScreenActiveMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenActiveMode003
     * @tc.desc: Test GetScreenActiveMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenActiveMode003, Function | SmallTest | Level3)
    {
        SupportedScreenModes info;
        string inputString = "1";
        auto ret = ScreenSettingHelper::GetScreenActiveMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString;
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest001
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest001, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 ";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest002
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest002, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 2";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_TRUE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest003
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest003, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 2 3";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest004
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest004, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "3 2";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest005
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest005, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 3";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest006
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest006, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "e 1";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenModeTest007
     * @tc.desc: Test GetScreenMode func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenModeTest007, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 e";
        auto ret = ScreenSettingHelper::GetScreenMode(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString;
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest001
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest001, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 2";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        ASSERT_FALSE(ret);
    }

     /**
     * @tc.name: GetScreenRelativePositionTest002
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest002, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 2 3";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            ASSERT_FALSE(ret);
        } else {
            ASSERT_TRUE(ret);
        }
    }

    /**
     * @tc.name: GetScreenRelativePositionTest003
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest003, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 2 3 4";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest004
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest004, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "E 1 2";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest005
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest005, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 E 2";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest006
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest006, Function | SmallTest | Level3)
    {
        MultiScreenInfo info;
        string inputString = "1 2 E";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        ASSERT_FALSE(ret);
    }


    /**
     * @tc.name: GetScreenRelativePositionTest007
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest007, Function | SmallTest | Level3)
    {
        ScreenId screenId = 1001;
        ScreenSessionConfig config = {
            .screenId = screenId,
            .rsId = 1001,
            .name = "GetScreenRelativePositionTest007",
        };
        sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
        ScreenSessionManager::GetInstance().screenSessionMap_[screenId] = screenSession;
        MultiScreenInfo info;
        info.isExtendMain = true;
        screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
        screenSession->isInternal_ = true;
        string inputString = "1 2 3";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            ASSERT_FALSE(ret);
            ASSERT_EQ(info.mainScreenOption.screenId_, 0);
        } else {
            ASSERT_TRUE(ret);
            ASSERT_EQ(info.mainScreenOption.screenId_, 1);
        }
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(screenId);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest008
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest008, Function | SmallTest | Level3)
    {
        ScreenId screenId = 1001;
        ScreenSessionConfig config = {
            .screenId = screenId,
            .rsId = 1001,
            .name = "GetScreenRelativePositionTest008",
        };
        sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
        ScreenSessionManager::GetInstance().screenSessionMap_[screenId] = screenSession;
        MultiScreenInfo info;
        screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
        screenSession->isInternal_ = true;
        info.isExtendMain = false;
        string inputString = "1001 2 3";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            ASSERT_FALSE(ret);
            ASSERT_EQ(info.secondaryScreenOption.screenId_, 0);
        } else {
            ASSERT_TRUE(ret);
            ASSERT_EQ(info.secondaryScreenOption.screenId_, 1001);
        }
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(screenId);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest009
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest009, Function | SmallTest | Level3)
    {
        ScreenId screenId = 1001;
        ScreenSessionConfig config = {
            .screenId = screenId,
            .rsId = 1001,
            .name = "GetScreenRelativePositionTest009",
        };
        sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
        ScreenSessionManager::GetInstance().screenSessionMap_[screenId] = screenSession;
        MultiScreenInfo info;
        screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
        screenSession->isInternal_ = true;
        info.isExtendMain = true;
        string inputString = "1001 2 3";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            ASSERT_FALSE(ret);
            ASSERT_EQ(info.mainScreenOption.screenId_, 0);
        } else {
            ASSERT_TRUE(ret);
            ASSERT_EQ(info.mainScreenOption.screenId_, 1001);
        }
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(screenId);
    }

    /**
     * @tc.name: GetScreenRelativePositionTest0010
     * @tc.desc: Test GetScreenRelativePosition func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetScreenRelativePositionTest0010, Function | SmallTest | Level3)
    {
        ScreenId screenId = 1001;
        ScreenSessionConfig config = {
            .screenId = screenId,
            .rsId = 1001,
            .name = "GetScreenRelativePositionTest0010",
        };
        sptr<ScreenSession> screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
        ScreenSessionManager::GetInstance().screenSessionMap_[screenId] = screenSession;
        MultiScreenInfo info;
        screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
        screenSession->isInternal_ = true;
        info.isExtendMain = false;
        string inputString = "1 2 3";
        auto ret = ScreenSettingHelper::GetScreenRelativePosition(info, inputString);
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            ASSERT_FALSE(ret);
            ASSERT_EQ(info.secondaryScreenOption.screenId_, 0);
        } else {
            ASSERT_TRUE(ret);
            ASSERT_EQ(info.secondaryScreenOption.screenId_, 1);
        }
        ScreenSessionManager::GetInstance().screenSessionMap_.erase(screenId);
    }

    /**
     * @tc.name: GetMultiScreenInfoTest
     * @tc.desc: Test GetMultiScreenInfo func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetMultiScreenInfoTest, Function | SmallTest | Level3)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string value = "";
        std::string key = "user_set_screen_mode_edid";
        bool ret = screenSettingHelper.SetSettingValue(key, value);
        ASSERT_FALSE(ret);

        std::map<std::string, MultiScreenInfo> multiScreenInfoMap = screenSettingHelper.GetMultiScreenInfo(key);
        ASSERT_TRUE(multiScreenInfoMap.empty());
    }

    /**
     * @tc.name: ConvertStrToInt32
     * @tc.desc: Test ConvertStrToInt32 func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, ConvertStrToInt32, Function | SmallTest | Level3)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string str = "not_a_number";
        int32_t num1 = 0;
        EXPECT_FALSE(screenSettingHelper.ConvertStrToInt32(str, num1));
        str = "123";
        EXPECT_TRUE(screenSettingHelper.ConvertStrToInt32(str, num1));
        EXPECT_EQ(123, num1);
    }

    /**
     * @tc.name: RegisterSettingDuringCallStateObserver
     * @tc.desc: RegisterSettingDuringCallStateObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingDuringCallStateObserver, TestSize.Level1)
    {
        bool flag = false;
        auto func = [&flag] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag = true;
        };
        ScreenSettingHelper::RegisterSettingDuringCallStateObserver(func);
        ASSERT_EQ(ScreenSettingHelper::duringCallStateObserver_, nullptr);

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        bool flag1 = false;
        auto func1 = [&flag1] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag1 = true;
        };
        ScreenSettingHelper::RegisterSettingDuringCallStateObserver(func1);
        EXPECT_FALSE(g_errLog.find("during call state observer is registered") != std::string::npos);
        LOG_SetCallback(nullptr);
    }

    /**
     * @tc.name: UnregisterSettingDuringCallStateObserver
     * @tc.desc: UnregisterSettingDuringCallStateObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingDuringCallStateObserver, TestSize.Level1)
    {
        ScreenSettingHelper::UnregisterSettingDuringCallStateObserver();
        ASSERT_EQ(ScreenSettingHelper::duringCallStateObserver_, nullptr);

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        ScreenSettingHelper::UnregisterSettingDuringCallStateObserver();
        EXPECT_TRUE(g_errLog.find("duringCallStateObserver_ is nullptr") != std::string::npos);
        LOG_SetCallback(nullptr);
    }

    /**
     * @tc.name: RegisterSettingBorderingAreaPercentObserver
     * @tc.desc: RegisterSettingBorderingAreaPercentObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingBorderingAreaPercentObserver, TestSize.Level1)
    {
        bool flag = false;
        auto func = [&flag] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag = true;
        };
        ScreenSettingHelper::RegisterSettingBorderingAreaPercentObserver(func);
        ASSERT_EQ(ScreenSettingHelper::borderingAreaPercentObserver_, nullptr);

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        bool flag1 = false;
        auto func1 = [&flag1] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag1 = true;
        };
        ScreenSettingHelper::RegisterSettingBorderingAreaPercentObserver(func1);
        EXPECT_FALSE(g_errLog.find("setting bordering area percent observer is registered") != std::string::npos);
        LOG_SetCallback(nullptr);
    }

    /**
     * @tc.name: UnregisterSettingBorderingAreaPercentObserver
     * @tc.desc: UnregisterSettingBorderingAreaPercentObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingBorderingAreaPercentObserver, TestSize.Level1)
    {
        ScreenSettingHelper::UnregisterSettingBorderingAreaPercentObserver();
        ASSERT_EQ(ScreenSettingHelper::duringCallStateObserver_, nullptr);

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        ScreenSettingHelper::UnregisterSettingBorderingAreaPercentObserver();
        EXPECT_TRUE(g_errLog.find("setting observer is nullptr") != std::string::npos);
        LOG_SetCallback(nullptr);
    }

    /**
     * @tc.name: GetSettingDuringCallStateTest
     * @tc.desc: Test GetSettingDuringCallState func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingDuringCallStateTest, Function | SmallTest | Level3)
    {
        if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
            return;
        }
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        bool value = true;

        screenSettingHelper.SetSettingDuringCallState("during_call_state", false);
        screenSettingHelper.GetSettingDuringCallState(value);
        ASSERT_TRUE(value);
    }

/**
 * @tc.name: ConvertStringToFoldDisplayModeSafely
 * @tc.desc: Test ConvertStringToFoldDisplayModeSafely func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, ConvertStringToFoldDisplayModeSafely, TestSize.Level1)
{
    const std::string str0 = "0";
    const std::string str1 = "1";
    const std::string str2 = "2";
    const std::string str3 = "3";
    const std::string str4 = "4";
    const std::string str5 = "5";
    const std::string str8 = "8";

    auto foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str0);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::UNKNOWN);
    foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str1);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::FULL);
    foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str2);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::MAIN);
    foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str3);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::SUB);
    foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str4);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::COORDINATION);
    foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str5);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::GLOBAL_FULL);
    foldDisplayMode = ScreenSettingHelper::ConvertStringToFoldDisplayModeSafely(str8);
    EXPECT_EQ(foldDisplayMode, FoldDisplayMode::UNKNOWN);
}

/**
 * @tc.name: RegisterRotationCorrectionWhiteListObserver
 * @tc.desc: Test RegisterRotationCorrectionWhiteListObserver func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, RegisterRotationCorrectionWhiteListObserver, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    auto func = [] (const std::string&) {
        TLOGI(WmsLogTag::DMS, "UT test");
    };
    ScreenSettingHelper::correctionWhiteListObserver_ = nullptr;
    ScreenSettingHelper::RegisterRotationCorrectionWhiteListObserver(func);
    EXPECT_FALSE(g_errLog.find("observer is registered") != std::string::npos);

    auto func1 = [] (const std::string&) {
        TLOGI(WmsLogTag::DMS, "UT test");
    };
    ScreenSettingHelper::correctionWhiteListObserver_ = sptr<SettingObserver>::MakeSptr();
    ScreenSettingHelper::RegisterRotationCorrectionWhiteListObserver(func1);
    EXPECT_TRUE(g_errLog.find("observer is registered") != std::string::npos);
    LOG_SetCallback(nullptr);
    ScreenSettingHelper::correctionWhiteListObserver_ = nullptr;
}

/**
 * @tc.name: UnregisterRotationCorrectionWhiteListObserver
 * @tc.desc: UnregisterRotationCorrectionWhiteListObserver
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, UnregisterRotationCorrectionWhiteListObserver, TestSize.Level1)
{
    ScreenSettingHelper::correctionWhiteListObserver_ = sptr<SettingObserver>::MakeSptr();
    ScreenSettingHelper::UnregisterRotationCorrectionWhiteListObserver();
    ASSERT_EQ(ScreenSettingHelper::correctionWhiteListObserver_, nullptr);

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSettingHelper::correctionWhiteListObserver_ = nullptr;
    ScreenSettingHelper::UnregisterRotationCorrectionWhiteListObserver();
    EXPECT_TRUE(g_errLog.find("observer is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetRotationCorrectionWhiteList
 * @tc.desc: Test GetRotationCorrectionWhiteList func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, GetRotationCorrectionWhiteList, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
    std::unordered_map<std::string, RotationCorrectionWhiteConfig> appConfigs;
    auto ret = screenSettingHelper.GetRotationCorrectionWhiteList(appConfigs);
    EXPECT_TRUE(g_errLog.find("failed") != std::string::npos);
    LOG_SetCallback(nullptr);
    ret = screenSettingHelper.GetRotationCorrectionWhiteList(appConfigs, "testKey");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetCorrectionWhiteListFromJson01
 * @tc.desc: Test json is not array
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, GetCorrectionWhiteListFromJson01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
    std::string json_str = "aa";
    std::unordered_map<std::string, RotationCorrectionWhiteConfig> appConfigs;
    screenSettingHelper.GetCorrectionWhiteListFromJson(json_str, appConfigs);
    EXPECT_TRUE(g_errLog.find("parse json failed") != std::string::npos);
    g_errLog.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetCorrectionWhiteListFromJson02
 * @tc.desc: Test json is array
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, GetCorrectionWhiteListFromJson02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCorrectionWhiteListFromJson2 start";
    ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
    std::string json_str = R"({
        "com.test.app1": {
            "name": "teststr",
            "useLogicCamera": {
                "1": 1,
                "2": 0
            },
            "customLogicDirection": {
                "1": 1,
                "2": 2
            }
        },
        "com.test.app2": {
            "name": "",
            "useLogicCamera": {
                "1": 1,
                "2": 0
            },
            "customLogicDirection": {
                "1": 1,
                "2": 2
            }
        }
    })";

    std::unordered_map<std::string, RotationCorrectionWhiteConfig> appConfigs;
    screenSettingHelper.GetCorrectionWhiteListFromJson(json_str, appConfigs);
    EXPECT_EQ(1, appConfigs.size());
    GTEST_LOG_(INFO) << "GetCorrectionWhiteListFromJson2 end";
}

/**
 * @tc.name: GetWhiteConfigFromJson
 * @tc.desc: Test GetWhiteConfigFromJson
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, GetWhiteConfigFromJson, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWhiteConfigFromJson start";
    RotationCorrectionWhiteConfig appConfigs;
    std::string appName;
    using JSON = nlohmann::json;

    // Case1: json is not object or null
    const std::string json_null_str = "";
    const std::string json_not_object_str = "aa";
    JSON nullStrJson = JSON::parse(json_null_str, nullptr, false);
    JSON notObjectStrJson = JSON::parse(json_not_object_str, nullptr, false);
    bool ret = ScreenSettingHelper::GetWhiteConfigFromJson(nullStrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(notObjectStrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);

    // Case2: json not contain name fileds
    const std::string json_no_name_str = R"({
        "useLogicCamera": {
            "1": 1
        }
    })";
    JSON noNameStrJson = JSON::parse(json_no_name_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(noNameStrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);

    // Case3: name fileds is not string
    const std::string json_name_is_not_string_str = R"({
        "name": true,
        "useLogicCamera": {
            "1": 1
        }
    })";
    JSON nameIsNotStringStrJson = JSON::parse(json_name_is_not_string_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(nameIsNotStringStrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);

    // Case4: name is empty
    const std::string json_name_is_empty_str = R"({
        "name": "",
        "useLogicCamera": {
            "1": 1
        }
    })";
    JSON nameIsEmptyStrJson = JSON::parse(json_name_is_empty_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(nameIsEmptyStrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);

    // Case5: not contain useLogicCamera and customLogicDirection
    const std::string json_only_name_str = R"({
        "name": "app1"
    })";
    JSON onlyNameStrJson = JSON::parse(json_only_name_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(onlyNameStrJson, appConfigs, appName);
    EXPECT_EQ(ret, true);

    // Case6: contains useLogicCamera and customLogicDirection
    std::string json_str = R"({
        "name": "app1",
        "useLogicCamera": {
            "1": 1
        },
        "customLogicDirection": {
            "1": 1
        }
    })";
    JSON strJson = JSON::parse(json_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(strJson, appConfigs, appName);
    EXPECT_EQ(ret, true);

    // Case7: useLogicCamera is not object
    std::string json_not_object_2_str = R"({
        "name": "app1",
        "useLogicCamera": 1,
        "customLogicDirection": {
            "1": 1
        }
    })";
    JSON notObject2StrJson = JSON::parse(json_not_object_2_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(notObject2StrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);

    // Case8: customLogicDirection is not object
    std::string json_not_object_3_str = R"({
        "name": "app1",
        "useLogicCamera": {
            "1": 1
        },
        "customLogicDirection": 1
    })";
    JSON notObject3StrJson = JSON::parse(json_not_object_3_str, nullptr, false);
    ret = ScreenSettingHelper::GetWhiteConfigFromJson(notObject3StrJson, appConfigs, appName);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "GetWhiteConfigFromJson end";
}

/**
 * @tc.name: ParseJsonObjectToEnumMap
 * @tc.desc: Test ParseJsonObjectToEnumMap
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSettingHelperTest, ParseJsonObjectToEnumMap, Function | SmallTest | Level3)
{
    GTEST_LOG_(INFO) << "ParseJsonObjectToEnumMap start";
    std::unordered_map<FoldDisplayMode, int32_t> resultMap;
    std::string appName;
    using JSON = nlohmann::json;

    // Case1: json is not object or null
    const std::string json_null_str = "";
    const std::string json_not_object_str = "aa";
    JSON nullStrJson = JSON::parse(json_null_str, nullptr, false);
    JSON notObjectStrJson = JSON::parse(json_not_object_str, nullptr, false);
    bool ret = ScreenSettingHelper::ParseJsonObjectToEnumMap(nullStrJson, resultMap);
    EXPECT_EQ(ret, false);
    ret = ScreenSettingHelper::ParseJsonObjectToEnumMap(notObjectStrJson, resultMap);
    EXPECT_EQ(ret, false);

    // Case2: value is not integer
    std::string json_value_isNot_int_str = R"({
        "1": 1,
        "2": "test"
    })";
    JSON valueIsNotIntStrJson = JSON::parse(json_value_isNot_int_str, nullptr, false);
    ret = ScreenSettingHelper::ParseJsonObjectToEnumMap(valueIsNotIntStrJson, resultMap);
    EXPECT_EQ(ret, false);

    // Case3: value is integer
    std::string json_value_is_int_str = R"({
        "1": 1,
        "2": 2
    })";
    JSON valueIsIntStrJson = JSON::parse(json_value_is_int_str, nullptr, false);
    ret = ScreenSettingHelper::ParseJsonObjectToEnumMap(valueIsIntStrJson, resultMap);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "ParseJsonObjectToEnumMap end";
}

    /**
     * @tc.name: RegisterSettingResolutionEffectObserver
     * @tc.desc: RegisterSettingResolutionEffectObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingResolutionEffectObserver, TestSize.Level1)
    {
        bool flag = false;
        auto func = [&flag] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag = true;
        };
        ScreenSettingHelper::RegisterSettingResolutionEffectObserver(func);
        ASSERT_EQ(ScreenSettingHelper::resolutionEffectObserver_, nullptr);

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        bool flag1 = false;
        auto func1 = [&flag1] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag1 = true;
        };
        ScreenSettingHelper::RegisterSettingResolutionEffectObserver(func1);
        EXPECT_FALSE(g_errLog.find("setting observer is registered") != std::string::npos);
        LOG_SetCallback(nullptr);
        ScreenSettingHelper::resolutionEffectObserver_ = nullptr;
    }

    /**
     * @tc.name: UnRegisterSettingResolutionEffectObserver
     * @tc.desc: UnRegisterSettingResolutionEffectObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingResolutionEffectObserver, TestSize.Level1)
    {
        ScreenSettingHelper::resolutionEffectObserver_ = new SettingObserver;
        ScreenSettingHelper::UnregisterSettingResolutionEffectObserver();
        ASSERT_EQ(ScreenSettingHelper::resolutionEffectObserver_, nullptr);

        ScreenSettingHelper::resolutionEffectObserver_ = nullptr;
        ScreenSettingHelper::UnregisterSettingResolutionEffectObserver();
        ASSERT_EQ(ScreenSettingHelper::resolutionEffectObserver_, nullptr);
    }

    /**
     * @tc.name: UnRegisterSettingResolutionEffectObserver
     * @tc.desc: UnRegisterSettingResolutionEffectObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingResolutionEffectObserver02, TestSize.Level1)
    {
        ScreenSettingHelper::resolutionEffectObserver_ = nullptr;
        ScreenSettingHelper::UnregisterSettingResolutionEffectObserver();
        ASSERT_EQ(ScreenSettingHelper::resolutionEffectObserver_, nullptr);
    }

    /**
     * @tc.name: RegisterSettingCoordinationReadyObserver01
     * @tc.desc: RegisterSettingCoordinationReadyObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingCoordinationReadyObserver01, TestSize.Level1)
    {
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::RegisterSettingCoordinationReadyObserver(func);
        ScreenSettingHelper::coordinationReadyObserver_ = nullptr;
        ASSERT_EQ(ScreenSettingHelper::coordinationReadyObserver_, nullptr);
    }

    /**
     * @tc.name: RegisterSettingCoordinationReadyObserver02
     * @tc.desc: RegisterSettingCoordinationReadyObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingCoordinationReadyObserver02, TestSize.Level1)
    {
        sptr<SettingObserver> observer = new SettingObserver();
        ScreenSettingHelper::coordinationReadyObserver_ = observer;
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::RegisterSettingCoordinationReadyObserver(func);
        ASSERT_EQ(ScreenSettingHelper::coordinationReadyObserver_, observer);
        ScreenSettingHelper::coordinationReadyObserver_ = nullptr;
    }

    /**
     * @tc.name: UnregisterSettingCoordinationReadyObserver01
     * @tc.desc: UnregisterSettingCoordinationReadyObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingCoordinationReadyObserver01, TestSize.Level1)
    {
        ScreenSettingHelper::coordinationReadyObserver_ = nullptr;
        ScreenSettingHelper::UnregisterSettingCoordinationReadyObserver();
        ASSERT_EQ(ScreenSettingHelper::coordinationReadyObserver_, nullptr);
    }

    /**
     * @tc.name: GetResolutionEffectTest
     * @tc.desc: Test GetResolutionEffectTest func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetResolutionEffectTestTest, Function | SmallTest | Level3)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        bool value = false;
        auto ret = screenSettingHelper.GetResolutionEffect(value, "testsn");
        ASSERT_FALSE(ret);
        ASSERT_FALSE(value);
    }

    /**
     * @tc.name: RegisterRotationCorrectionExemptionListObserver
     * @tc.desc: RegisterRotationCorrectionExemptionListObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterRotationCorrectionExemptionListObserver, TestSize.Level1)
    {
        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        bool flag = false;
        auto func = [&flag] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag = true;
        };
        ScreenSettingHelper::correctionExemptionListObserver_ = nullptr;
        ScreenSettingHelper::RegisterRotationCorrectionExemptionListObserver(func);
        EXPECT_FALSE(g_errLog.find("observer is registered") != std::string::npos);
 
        bool flag1 = false;
        auto func1 = [&flag1] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
            flag1 = true;
        };
        ScreenSettingHelper::correctionExemptionListObserver_ = sptr<SettingObserver>::MakeSptr();
        ScreenSettingHelper::RegisterRotationCorrectionExemptionListObserver(func1);
        EXPECT_TRUE(g_errLog.find("observer is registered") != std::string::npos);
        LOG_SetCallback(nullptr);
        ScreenSettingHelper::correctionExemptionListObserver_ = nullptr;
    }
 
    /**
     * @tc.name: UnregisterRotationCorrectionExemptionListObserver
     * @tc.desc: UnregisterRotationCorrectionExemptionListObserver
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterRotationCorrectionExemptionListObserver, TestSize.Level1)
    {
        ScreenSettingHelper::correctionExemptionListObserver_ = sptr<SettingObserver>::MakeSptr();
        ScreenSettingHelper::UnregisterRotationCorrectionExemptionListObserver();
        ASSERT_EQ(ScreenSettingHelper::correctionExemptionListObserver_, nullptr);

        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        ScreenSettingHelper::correctionExemptionListObserver_ = nullptr;
        ScreenSettingHelper::UnregisterRotationCorrectionExemptionListObserver();
        EXPECT_TRUE(g_errLog.find("observer is nullptr") != std::string::npos);
        LOG_SetCallback(nullptr);
    }
 
    /**
     * @tc.name: GetRotationCorrectionExemptionList
     * @tc.desc: Test GetRotationCorrectionExemptionList func
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetRotationCorrectionExemptionList, Function | SmallTest | Level3)
    {
        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::vector<std::string> exemptionApps;
        auto ret = screenSettingHelper.GetRotationCorrectionExemptionList(exemptionApps);
        EXPECT_TRUE(g_errLog.find("failed") != std::string::npos);
        LOG_SetCallback(nullptr);
        ret = screenSettingHelper.GetRotationCorrectionExemptionList(exemptionApps, "testKey");
        ASSERT_FALSE(ret);
    }
 
    /**
     * @tc.name: GetCorrectionExemptionListFromJson
     * @tc.desc: Test json is not array
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetCorrectionExemptionListFromJson01, Function | SmallTest | Level3)
    {
        g_errLog.clear();
        LOG_SetCallback(MyLogCallback);
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string json_str = "aa";
        std::vector<std::string> exemptionApps;
        screenSettingHelper.GetCorrectionExemptionListFromJson(json_str, exemptionApps);
        EXPECT_TRUE(g_errLog.find("parse json failed") != std::string::npos);
        g_errLog.clear();
        LOG_SetCallback(nullptr);
    }
 
    /**
     * @tc.name: GetCorrectionExemptionListFromJson
     * @tc.desc: Test json is array
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetCorrectionExemptionListFromJson02, Function | SmallTest | Level3)
    {
        GTEST_LOG_(INFO) << "GetCorrectionExemptionListFromJson02 start";
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string json_str = R"({
            "com.test.app1": {
                "test": "teststr"
            },
            "com.test.app2": {
                "name": "com.test.app2"
            },
            "com.test.app3": {
                "name": "com.test.app3",
                "mode": 8
            },
            "com.test.app4": {
                "name": "com.test.app4",
                "mode": 8,
                "exemptNaturalDirectionCorrect": true
            },
            "com.test.app5": {
                "name": "com.test.app5",
                "mode": 6,
                "exemptNaturalDirectionCorrect": true
            },
            "com.test.app6": {
                "name": "com.test.app6",
                "mode": 8,
                "exemptNaturalDirectionCorrect": false
            }
        })";
 
        std::vector<std::string> exemptionApps;
        screenSettingHelper.GetCorrectionExemptionListFromJson(json_str, exemptionApps);
        EXPECT_EQ(1, exemptionApps.size());
    }
 
    /**
     * @tc.name: GetJsonValue
     * @tc.desc: Test GetJsonValue failed
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetJsonValue, Function | SmallTest | Level3)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string json_str = R"({"testKey": 1})";
        nlohmann::json root1 = nlohmann::json::parse(json_str, nullptr, false);
        std::string name = "";
        bool ret = screenSettingHelper.GetJsonValue(root1, "testKey", name);
        EXPECT_FALSE(ret);

        json_str = R"({"testKey": "aa"})";
        nlohmann::json root2 = nlohmann::json::parse(json_str, nullptr, false);
        int32_t mode = -1;
        ret = screenSettingHelper.GetJsonValue(root2, "testKey", mode);
        EXPECT_FALSE(ret);

        json_str = R"({"testKey": "aa"})";
        nlohmann::json root3 = nlohmann::json::parse(json_str, nullptr, false);
        bool valueBool = false;
        ret = screenSettingHelper.GetJsonValue(root3, "testKey", valueBool);
        EXPECT_FALSE(valueBool);
    }
}
} // namespace Rosen
} // namespace OHOS
