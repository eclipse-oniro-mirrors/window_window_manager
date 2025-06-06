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

#include "fold_screen_controller/super_fold_state_manager.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

namespace {
#define ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {return;}
}
    
class SuperFoldStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SuperFoldStateManagerTest::SetUpTestCase()
{
}

void SuperFoldStateManagerTest::TearDownTestCase()
{
}

void SuperFoldStateManagerTest::SetUp()
{
}

void SuperFoldStateManagerTest::TearDown()
{
}

namespace {

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : ANGLE_CHANGE_EXPANDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange01, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::EXPANDED;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : ANGLE_CHANGE_HALF_FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange02, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : ANGLE_CHANGE_HALF_FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange03, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::EXPANDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : ANGLE_CHANGE_FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange04, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_FOLDED;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::FOLDED;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : KEYBOARD_ON
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange05, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::KEYBOARD_ON;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::KEYBOARD;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : KEYBOARD_ON
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange06, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::KEYBOARD_ON;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::EXPANDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().
            HandleSuperFoldStatusChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED);
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::KEYBOARD;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : KEYBOARD_OFF
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange07, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::KEYBOARD_OFF;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::KEYBOARD);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };

    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;

    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : SYSTEM_KEYBOARD_ON
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange08, Function | SmallTest | Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };
 
    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;
 
    EXPECT_EQ(curState, expState);
}
 
/**
 * @tc.name: HandleSuperFoldStatusChange
 * @tc.desc: test function : SYSTEM_KEYBOARD_OFF
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, HandleSuperFoldStatusChange09, Function | SmallTest | Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF;
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
    };
 
    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;
 
    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: MatchSuperFoldStatusToFoldStatus
 * @tc.desc: test function : EXPANDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, MatchSuperFoldStatusToFoldStatus01, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatus superFoldStatus = SuperFoldStatus::EXPANDED;
    FoldStatus foldStatus;
    std::function<void()> func = [&]()
    {
        foldStatus = SuperFoldStateManager::GetInstance().MatchSuperFoldStatusToFoldStatus(superFoldStatus);
    };

    func();
    FoldStatus foldStatusExp = FoldStatus::EXPAND;
    EXPECT_EQ(foldStatus, foldStatusExp);
}

/**
 * @tc.name: MatchSuperFoldStatusToFoldStatus
 * @tc.desc: test function : HALF_FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, MatchSuperFoldStatusToFoldStatus02, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatus superFoldStatus = SuperFoldStatus::HALF_FOLDED;
    FoldStatus foldStatus;
    std::function<void()> func = [&]()
    {
        foldStatus = SuperFoldStateManager::GetInstance().MatchSuperFoldStatusToFoldStatus(superFoldStatus);
    };

    func();
    FoldStatus foldStatusExp = FoldStatus::HALF_FOLD;
    EXPECT_EQ(foldStatus, foldStatusExp);
}

/**
 * @tc.name: MatchSuperFoldStatusToFoldStatus
 * @tc.desc: test function : FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, MatchSuperFoldStatusToFoldStatus03, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatus superFoldStatus = SuperFoldStatus::FOLDED;
    FoldStatus foldStatus;
    std::function<void()> func = [&]()
    {
        foldStatus = SuperFoldStateManager::GetInstance().MatchSuperFoldStatusToFoldStatus(superFoldStatus);
    };

    func();
    FoldStatus foldStatusExp = FoldStatus::FOLDED;
    EXPECT_EQ(foldStatus, foldStatusExp);
}

/**
 * @tc.name: MatchSuperFoldStatusToFoldStatus
 * @tc.desc: test function : KEYBOARD
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, MatchSuperFoldStatusToFoldStatus04, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatus superFoldStatus = SuperFoldStatus::KEYBOARD;
    FoldStatus foldStatus;
    std::function<void()> func = [&]()
    {
        foldStatus = SuperFoldStateManager::GetInstance().MatchSuperFoldStatusToFoldStatus(superFoldStatus);
    };

    func();
    FoldStatus foldStatusExp = FoldStatus::HALF_FOLD;
    EXPECT_EQ(foldStatus, foldStatusExp);
}

/**
 * @tc.name: MatchSuperFoldStatusToFoldStatus
 * @tc.desc: test function : UNKNOWN
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, MatchSuperFoldStatusToFoldStatus05, TestSize.Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStatus superFoldStatus = SuperFoldStatus::UNKNOWN;
    FoldStatus foldStatus;
    std::function<void()> func = [&]()
    {
        foldStatus = SuperFoldStateManager::GetInstance().MatchSuperFoldStatusToFoldStatus(superFoldStatus);
    };

    func();
    FoldStatus foldStatusExp = FoldStatus::UNKNOWN;
    EXPECT_EQ(foldStatus, foldStatusExp);
}

/**
 * @tc.name: SetSystemKeyboardStatus
 * @tc.desc: SetSystemKeyboardStatus with true as parameter
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, SetSystemKeyboardStatus01, Function | SmallTest | Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().SetSystemKeyboardStatus(true);
    };
 
    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;
 
    EXPECT_EQ(curState, expState);
}
 
/**
 * @tc.name: SetSystemKeyboardStatus
 * @tc.desc: SetSystemKeyboardStatus with false as parameter
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, SetSystemKeyboardStatus02, Function | SmallTest | Level1)
{
    ONLY_FOR_SUPERFOLD_DISPLAY_DEVICE
    SuperFoldStateManager::GetInstance().SetCurrentStatus(SuperFoldStatus::HALF_FOLDED);
    std::function<void()> func = [&]()
    {
        SuperFoldStateManager::GetInstance().SetSystemKeyboardStatus(false);
    };
 
    func();
    SuperFoldStatus curState = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    SuperFoldStatus expState = SuperFoldStatus::HALF_FOLDED;
 
    EXPECT_EQ(curState, expState);
}

/**
 * @tc.name: GetCurrentStatus_ShouldReturnKeyboard_WhenHalfScreenIsFalse
 * @tc.desc: Test GetCurrentStatus function when isHalfScreen_ is false.
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldStateManagerTest, GetCurrentStatus_ShouldReturnKeyboard_WhenHalfScreenIsFalse, TestSize.Level1)
{
    SuperFoldStateManager superFoldStateManager = SuperFoldStateManager();
    superFoldStateManager.SetCurrentStatus(SuperFoldStatus::UNKNOWN);
    superFoldStateManager.ChangeScreenState(false);

    SuperFoldStatus curState = superFoldStateManager.GetCurrentStatus();

    EXPECT_EQ(curState, SuperFoldStatus::UNKNOWN);
}
}
}
}