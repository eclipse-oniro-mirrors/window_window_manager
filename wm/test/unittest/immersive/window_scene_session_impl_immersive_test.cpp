/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "mock_window_scene_session_impl.h"
#include "scene_board_judgement.h"
#include "session/host/include/scene_session.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class WindowSceneSessionImplImmersiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    static constexpr uint32_t WAIT_SERVERAL_FRAMES = 36000;
};

void WindowSceneSessionImplImmersiveTest::SetUpTestCase() {}

void WindowSceneSessionImplImmersiveTest::TearDownTestCase() {}

void WindowSceneSessionImplImmersiveTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplImmersiveTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, GetAvoidAreaByType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAvoidAreaByType");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    AvoidArea avoidArea;
    auto ret = windowSceneSessionImpl->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidArea);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidArea);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: SetOwnSystemBarProperty
 * @tc.desc: SetOwnSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, SetOwnSystemBarProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetOwnSystemBarProperty");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(window->ownSystemBarPropertyMap_.size(), 0);
    PartialSystemBarProperty prop;
    window->SetOwnSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop, SystemBarPropertyOwner::APPLICATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_.size(), 1);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 1);
    window->SetOwnSystemBarProperty(
        WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, prop, SystemBarPropertyOwner::APPLICATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_.size(), 2);
    window->SetOwnSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop, SystemBarPropertyOwner::ARKUI_NAVIGATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 2);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ARKUI_NAVIGATION);
    window->SetOwnSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop, SystemBarPropertyOwner::APPLICATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 2);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::APPLICATION);
    window->SetOwnSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop, SystemBarPropertyOwner::ABILITY_RUNTIME);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 3);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ABILITY_RUNTIME);
    window->SetOwnSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop, SystemBarPropertyOwner::APPLICATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 3);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ABILITY_RUNTIME);
    PartialSystemBarProperty prop2;
    prop2.flag_.enableFlag = true;
    window->SetOwnSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop2, SystemBarPropertyOwner::APPLICATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 4);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ABILITY_RUNTIME);
}

/**
 * @tc.name: RemoveOwnSystemBarProperty
 * @tc.desc: RemoveOwnSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, RemoveOwnSystemBarProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveOwnSystemBarProperty");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(window->ownSystemBarPropertyMap_.size(), 0);
    PartialSystemBarProperty prop;
    prop.flag_.enableFlag = true;
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR] = std::list<OwnSystemBarPropertyPair>();
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].push_front(
        OwnSystemBarPropertyPair(SystemBarPropertyOwner::APPLICATION, prop));
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].push_front(
        OwnSystemBarPropertyPair(SystemBarPropertyOwner::ARKUI_NAVIGATION, prop));
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].push_front(
        OwnSystemBarPropertyPair(SystemBarPropertyOwner::ATOMIC_SERVICE, prop));
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 3);
    window->RemoveOwnSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, prop.flag_, SystemBarPropertyOwner::ATOMIC_SERVICE);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 2);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ARKUI_NAVIGATION);
    window->RemoveOwnSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, prop.flag_, SystemBarPropertyOwner::ABILITY_RUNTIME);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 2);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ARKUI_NAVIGATION);
    SystemBarPropertyFlag invalidMask;
    window->RemoveOwnSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, invalidMask, SystemBarPropertyOwner::ARKUI_NAVIGATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].size(), 2);
    EXPECT_EQ(window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].front().first,
              SystemBarPropertyOwner::ARKUI_NAVIGATION);
    window->RemoveOwnSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, prop.flag_, SystemBarPropertyOwner::ARKUI_NAVIGATION);
    window->RemoveOwnSystemBarProperty(
        WindowType::WINDOW_TYPE_STATUS_BAR, prop.flag_, SystemBarPropertyOwner::APPLICATION);
    EXPECT_EQ(window->ownSystemBarPropertyMap_.size(), 0);
}

/**
 * @tc.name: GetCurrentActiveSystemBarProperty
 * @tc.desc: GetCurrentActiveSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, GetCurrentActiveSystemBarProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetCurrentActiveSystemBarProperty");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    // prepare several own properties with different owners and flags
    PartialSystemBarProperty propApp;
    propApp.flag_.backgroundColorFlag = true;
    propApp.backgroundColor_ = 0x111111;

    PartialSystemBarProperty propArk;
    propArk.flag_.contentColorFlag = true;
    propArk.contentColor_ = 0x222222;

    PartialSystemBarProperty propAbility;
    propAbility.flag_.backgroundColorFlag = true;
    propAbility.flag_.enableFlag = true;
    propAbility.backgroundColor_ = 0x333333;
    propAbility.enable_ = false;

    // insert into list: push_front so the last pushed becomes front
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR] = std::list<OwnSystemBarPropertyPair>();
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].push_front(
        OwnSystemBarPropertyPair(SystemBarPropertyOwner::APPLICATION, propApp));
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].push_front(
        OwnSystemBarPropertyPair(SystemBarPropertyOwner::ARKUI_NAVIGATION, propArk));
    window->ownSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR].push_front(
        OwnSystemBarPropertyPair(SystemBarPropertyOwner::ABILITY_RUNTIME, propAbility));

    SystemBarProperty cur = window->GetCurrentActiveSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR);
    // ABILITY_RUNTIME should take precedence for background color and enable
    EXPECT_EQ(cur.backgroundColor_, propAbility.backgroundColor_);
    EXPECT_EQ(cur.contentColor_, propArk.contentColor_);
    EXPECT_EQ(cur.enable_, propAbility.enable_);
    uint32_t flags = static_cast<uint32_t>(cur.settingFlag_);
    EXPECT_TRUE((flags & static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING)) != 0);
    EXPECT_TRUE((flags & static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING)) != 0);
}

/**
 * @tc.name: SetStatusBarColorForNavigation
 * @tc.desc: SetStatusBarColorForNavigation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, SetStatusBarColorForNavigation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetStatusBarColorForNavigation");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetStatusBarColorForNavigation(std::nullopt));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    // set a color
    uint32_t color = 0xAABBCC;
    window->SetStatusBarColorForNavigation(color);
    auto it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    EXPECT_FALSE(it == window->ownSystemBarPropertyMap_.end());
    bool found = false;
    for (const auto& pair : it->second) {
        if (pair.first == SystemBarPropertyOwner::ARKUI_NAVIGATION && pair.second.flag_.contentColorFlag) {
            EXPECT_EQ(pair.second.contentColor_, color);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    // clear
    window->SetStatusBarColorForNavigation(std::nullopt);
    it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    if (it != window->ownSystemBarPropertyMap_.end()) {
        bool still = std::any_of(it->second.begin(), it->second.end(), [](const OwnSystemBarPropertyPair& p) {
            return p.first == SystemBarPropertyOwner::ARKUI_NAVIGATION;
        });
        EXPECT_FALSE(still);
    }
}

/**
 * @tc.name: SetSystemBarPropertyForPage
 * @tc.desc: SetSystemBarPropertyForPage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, SetSystemBarPropertyForPage, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSystemBarPropertyForPage");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
        window->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, std::nullopt));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    SystemBarProperty prop;
    prop.enable_ = false;
    prop.enableAnimation_ = true;
    window->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, prop);
    auto it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    EXPECT_FALSE(it == window->ownSystemBarPropertyMap_.end());
    bool found = false;
    for (const auto& pair : it->second) {
        if (pair.first == SystemBarPropertyOwner::ARKUI_NAVIGATION && pair.second.flag_.enableFlag &&
            pair.second.flag_.enableAnimationFlag) {
            EXPECT_EQ(pair.second.enable_, prop.enable_);
            EXPECT_EQ(pair.second.enableAnimation_, prop.enableAnimation_);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    // clear
    window->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, std::nullopt);
    it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    if (it != window->ownSystemBarPropertyMap_.end()) {
        bool still = std::any_of(it->second.begin(), it->second.end(), [](const OwnSystemBarPropertyPair& p) {
            return p.first == SystemBarPropertyOwner::ARKUI_NAVIGATION;
        });
        EXPECT_FALSE(still);
    }
}

/**
 * @tc.name: SetStatusBarColorForPage
 * @tc.desc: SetStatusBarColorForPage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, SetStatusBarColorForPage, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetStatusBarColorForPage");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    uint32_t color = 0x334455;
    window->SetStatusBarColorForPage(color);
    auto it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    EXPECT_FALSE(it == window->ownSystemBarPropertyMap_.end());
    bool found = false;
    for (const auto& pair : it->second) {
        if (pair.first == SystemBarPropertyOwner::ATOMIC_SERVICE && pair.second.flag_.contentColorFlag) {
            EXPECT_EQ(pair.second.contentColor_, color);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    // clear
    window->SetStatusBarColorForPage(std::nullopt);
    it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    if (it != window->ownSystemBarPropertyMap_.end()) {
        bool still = std::any_of(it->second.begin(), it->second.end(), [](const OwnSystemBarPropertyPair& p) {
            return p.first == SystemBarPropertyOwner::ATOMIC_SERVICE;
        });
        EXPECT_FALSE(still);
    }
}

/**
 * @tc.name: UpdateSystemBarPropertyForPage
 * @tc.desc: UpdateSystemBarPropertyForPage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, UpdateSystemBarPropertyForPage, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSystemBarPropertyForPage_Direct");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SystemBarProperty property;
    property.enable_ = true;
    property.backgroundColor_ = 0x445566;
    property.contentColor_ = 0x778899;
    property.enableAnimation_ = false;

    SystemBarPropertyFlag flag;
    flag.backgroundColorFlag = true;
    flag.contentColorFlag = true;
    flag.enableFlag = true;
    flag.enableAnimationFlag = false;

    window->UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, property, flag);
    auto it = window->ownSystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
    EXPECT_FALSE(it == window->ownSystemBarPropertyMap_.end());
    bool found = false;
    for (const auto& pair : it->second) {
        if (pair.first == SystemBarPropertyOwner::APPLICATION) {
            EXPECT_TRUE(pair.second.flag_.backgroundColorFlag);
            EXPECT_TRUE(pair.second.flag_.contentColorFlag);
            EXPECT_TRUE(pair.second.flag_.enableFlag);
            EXPECT_FALSE(pair.second.flag_.enableAnimationFlag);
            EXPECT_EQ(pair.second.backgroundColor_, property.backgroundColor_);
            EXPECT_EQ(pair.second.contentColor_, property.contentColor_);
            EXPECT_EQ(pair.second.enable_, property.enable_);
            EXPECT_EQ(pair.second.enableAnimation_, property.enableAnimation_);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}


/**
 * @tc.name: UpdateSystemBarProperties
 * @tc.desc: UpdateSystemBarProperties
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, UpdateSystemBarProperties, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSystemBarProperties");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_ = sptr<WindowSessionProperty>::MakeSptr();
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->state_ = WindowState::STATE_SHOWN;
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimationFlag = true;
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColorFlag = true;
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag = true;
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimationFlag = true;
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    window->state_ = WindowState::STATE_DESTROYED;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
        window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
