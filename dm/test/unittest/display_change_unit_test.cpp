/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "display_info.h"
#include "display_manager.h"
#include "mock_display_manager_adapter.h"
#include "screen_manager.h"
#include "screen_manager/rs_screen_mode_info.h"
#include "singleton_mocker.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<DisplayManagerAdapter, MockDisplayManagerAdapter>;

class DisplayChangeEventListener : public DisplayManager::IDisplayListener {
public:
    virtual void OnCreate(DisplayId displayId) {}

    virtual void OnDestroy(DisplayId displayId) {}

    virtual void OnChange(DisplayId displayId) {}
};

class DisplayChangeUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    bool ScreenSizeEqual(const sptr<Screen> screen, const sptr<SupportedScreenModes> curInfo);
    bool DisplaySizeEqual(const sptr<Display> display, const sptr<SupportedScreenModes> curInfo);
    static DisplayId defaultDisplayId_;
    static ScreenId defaultScreenId_;
    sptr<DisplayChangeEventListener> listener_ = new DisplayChangeEventListener();
};
DisplayId DisplayChangeUnitTest::defaultDisplayId_ = DISPLAY_ID_INVALID;
ScreenId DisplayChangeUnitTest::defaultScreenId_ = SCREEN_ID_INVALID;

void DisplayChangeUnitTest::SetUpTestCase()
{
    defaultDisplayId_ = DisplayManager::GetInstance().GetDefaultDisplayId();
    sptr<Display> defaultDisplay = DisplayManager::GetInstance().GetDisplayById(defaultDisplayId_);
    if (defaultDisplay != nullptr) {
        defaultScreenId_ = defaultDisplay->GetScreenId();
    }
}

void DisplayChangeUnitTest::TearDownTestCase()
{
}

void DisplayChangeUnitTest::SetUp()
{
}

void DisplayChangeUnitTest::TearDown()
{
}

bool DisplayChangeUnitTest::DisplaySizeEqual(const sptr<Display> display, const sptr<SupportedScreenModes> curInfo)
{
    uint32_t dWidth = static_cast<uint32_t>(display->GetWidth());
    uint32_t dHeight = static_cast<uint32_t>(display->GetHeight());
    TLOGI(WmsLogTag::DMS, "DisplaySize: %{public}u %{public}u, ActiveModeInfoSize: %{public}u %{public}u",
        dWidth, dHeight, curInfo->width_, curInfo->height_);
    return ((curInfo->width_ == dWidth) && (curInfo->height_ == dHeight));
}

namespace {
/**
 * @tc.name: RegisterDisplayChangeListener01
 * @tc.desc: Register and Unregister displayChangeListener with valid listener and check return true
 * @tc.type: FUNC
 */
HWTEST_F(DisplayChangeUnitTest, RegisterDisplayChangeListener01, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), RegisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER))
        .Times(1).WillOnce(Return(DMError::DM_OK));
    DMError ret = DisplayManager::GetInstance().RegisterDisplayListener(listener_);
    ASSERT_EQ(DMError::DM_OK, ret);

    EXPECT_CALL(m.Mock(), UnregisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER))
        .Times(1).WillOnce(Return(DMError::DM_OK));
    ret = DisplayManager::GetInstance().UnregisterDisplayListener(listener_);
    ASSERT_EQ(DMError::DM_OK, ret);
}

/**
 * @tc.name: RegisterDisplayChangeListener02
 * @tc.desc: Register and Unregister displayChangeListener with nullptr and check return false
 * @tc.type: FUNC
 */
HWTEST_F(DisplayChangeUnitTest, RegisterDisplayChangeListener02, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), RegisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER))
        .Times(0);
    DMError ret = DisplayManager::GetInstance().RegisterDisplayListener(nullptr);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ret);

    EXPECT_CALL(m.Mock(), UnregisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER))
        .Times(0);
    ret = DisplayManager::GetInstance().UnregisterDisplayListener(nullptr);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterDisplayChangeListener03
 * @tc.desc: Register and Unregister displayChangeListener when ipc fails and check return false
 * @tc.type: FUNC
 */
HWTEST_F(DisplayChangeUnitTest, RegisterDisplayChangeListener03, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), RegisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER))
        .Times(1).WillOnce(Return(DMError::DM_ERROR_NULLPTR));
    DMError ret = DisplayManager::GetInstance().RegisterDisplayListener(listener_);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ret);

    EXPECT_CALL(m.Mock(), UnregisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER))
        .Times(0);
    ret  = DisplayManager::GetInstance().UnregisterDisplayListener(listener_);
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ret);
}
}
} // namespace Rosen
} // namespace OHOS