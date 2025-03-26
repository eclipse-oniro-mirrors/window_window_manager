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

#include "screen_session_manager/include/screen_setting_helper.h"
#include "window_manager_hilog.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 100000;
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
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingDpiObserver, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingDpiObserver01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingDpiObserver02, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, GetSettingDpi01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingCastObserver01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingCastObserver02, Function | SmallTest | Level3)
    {
        auto func = [] (const std::string&) {
            TLOGI(WmsLogTag::DMS, "UT test");
        };
        ScreenSettingHelper::castObserver_ = nullptr;
        ScreenSettingHelper::RegisterSettingCastObserver(func);
        ASSERT_NE(ScreenSettingHelper::castObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingCastObserver01
     * @tc.desc: UnregisterSettingCastObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingCastObserver01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingCastObserver02, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, GetSettingCast01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, GetSettingCast02, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingRotationObserver01, Function | SmallTest | Level3)
    {
        ScreenSettingHelper::rotationObserver_ = nullptr;
        ASSERT_EQ(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: UnregisterSettingRotationObserver02
     * @tc.desc: UnregisterSettingRotationObserver02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingRotationObserver02, Function | SmallTest | Level3)
    {
        ScreenSettingHelper::rotationObserver_ = new SettingObserver;
        ASSERT_NE(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: SetSettingRotation
     * @tc.desc: SetSettingRotation
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, SetSettingRotation, Function | SmallTest | Level3)
    {
        int32_t rotation = 180;
        ScreenSettingHelper::SetSettingRotation(rotation);
        ASSERT_NE(ScreenSettingHelper::rotationObserver_, nullptr);
    }

    /**
     * @tc.name: GetSettingRotation01
     * @tc.desc: GetSettingRotation01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotation01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotation02, Function | SmallTest | Level3)
    {
        if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
            GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
        }
        int32_t rotation = 0;
        std::string key = "default_display_dpi";
        auto result = ScreenSettingHelper::GetSettingRotation(rotation, key);
        ASSERT_EQ(result, true);
    }

    /**
     * @tc.name: SetSettingRotationScreenId
     * @tc.desc: SetSettingRotationScreenId
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, SetSettingRotationScreenId, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotationScreenID01, Function | SmallTest | Level3)
    {
        int32_t screenId = 0;
        std::string key = "test";
        auto result = ScreenSettingHelper::GetSettingRotation(screenId, key);
        ASSERT_EQ(result, false);
    }

    /**
     * @tc.name: GetSettingRotationScreenID02
     * @tc.desc: GetSettingRotationScreenID02
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingRotationScreenID02, Function | SmallTest | Level3)
    {
        int32_t screenId = 0;
        std::string key = "screen_rotation_screen_id_value";
        auto result = ScreenSettingHelper::GetSettingRotation(screenId, key);
        ASSERT_EQ(result, false);
    }

    /**
     * @tc.name: GetSettingValue
     * @tc.desc: GetSettingValue
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, GetSettingValue, Function | SmallTest | Level3)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        uint32_t value = 0;
        std::string key = "test";
        bool ret = screenSettingHelper.GetSettingValue(value, key);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: RemoveInvalidChar01
     * @tc.desc: RemoveInvalidChar Test01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RemoveInvalidChar01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, RemoveInvalidChar02, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, SplitString01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, SplitString02, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, IsNumber01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, IsNumber02, Function | SmallTest | Level3)
    {
        ScreenSettingHelper screenSettingHelper = ScreenSettingHelper();
        std::string test_str = "test";
        auto ret = screenSettingHelper.IsNumber(test_str);
        ASSERT_FALSE(ret);
    }

    /**
     * @tc.name: RegisterSettingWireCastObserver01
     * @tc.desc: RegisterSettingWireCastObserver01
     * @tc.type: FUNC
     */
    HWTEST_F(ScreenSettingHelperTest, RegisterSettingWireCastObserver01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingWireCastObserver01, Function | SmallTest | Level3)
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
    HWTEST_F(ScreenSettingHelperTest, UnregisterSettingWireCastObserver02, Function | SmallTest | Level3)
    {
        ScreenSettingHelper::wireCastObserver_ = new SettingObserver;
        ScreenSettingHelper::UnregisterSettingWireCastObserver();
        ASSERT_EQ(ScreenSettingHelper::wireCastObserver_, nullptr);
    }
}
} // namespace Rosen
} // namespace OHOS