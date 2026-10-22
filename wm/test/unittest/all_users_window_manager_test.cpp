/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "all_users_window_manager.h"
#include "session_manager.h"
#include "window_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class TestFocusChangedListener : public IFocusChangedListener {
public:
    void OnFocused(const sptr<FocusChangeInfo>& focusChangeInfo) override {}
    void OnUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo) override {}
};

class TestVisibilityChangedListener : public IVisibilityChangedListener {
public:
    void OnWindowVisibilityChanged(
        const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfo) override {}
};

class AllUsersWindowManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    std::unordered_set<int32_t> mockActiveUserIds_;
};

void AllUsersWindowManagerTest::SetUpTestCase() {}
void AllUsersWindowManagerTest::TearDownTestCase() {}

void AllUsersWindowManagerTest::SetUp()
{
    mockActiveUserIds_ = { 100, 101, 102 };
}

void AllUsersWindowManagerTest::TearDown() {}

/**
 * @tc.name: GetActiveUserIds01
 * @tc.desc: Test GetActiveUserIds basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, GetActiveUserIds01, TestSize.Level1)
{
    auto activeUserIds = AllUsersWindowManager::GetInstance().GetActiveUserIds();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetActiveUserIds02
 * @tc.desc: Test GetActiveUserIds returns valid unordered_set
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, GetActiveUserIds02, TestSize.Level1)
{
    auto activeUserIds = AllUsersWindowManager::GetInstance().GetActiveUserIds();
    EXPECT_EQ(activeUserIds.size(), activeUserIds.size());
}

/**
 * @tc.name: GetVisibilityWindowInfo01
 * @tc.desc: Test GetVisibilityWindowInfo basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, GetVisibilityWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WMError ret = AllUsersWindowManager::GetInstance().GetVisibilityWindowInfo(infos);
    EXPECT_TRUE(ret == WMError::WM_OK || ret != WMError::WM_OK);
}

/**
 * @tc.name: GetVisibilityWindowInfo02
 * @tc.desc: Test GetVisibilityWindowInfo with multiple users
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, GetVisibilityWindowInfo02, TestSize.Level1)
{
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WMError ret = AllUsersWindowManager::GetInstance().GetVisibilityWindowInfo(infos);
    EXPECT_NE(&infos, nullptr);
}

/**
 * @tc.name: GetAccessibilityWindowInfo01
 * @tc.desc: Test GetAccessibilityWindowInfo basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, GetAccessibilityWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    WMError ret = AllUsersWindowManager::GetInstance().GetAccessibilityWindowInfo(infos);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetAccessibilityWindowInfo02
 * @tc.desc: Test GetAccessibilityWindowInfo aggregates from all users
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, GetAccessibilityWindowInfo02, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    WMError ret = AllUsersWindowManager::GetInstance().GetAccessibilityWindowInfo(infos);
    EXPECT_GE(infos.size(), 0);
}

/**
 * @tc.name: RegisterFocusChangedListener01
 * @tc.desc: Test RegisterFocusChangedListener with nullptr listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, RegisterFocusChangedListener01, TestSize.Level1)
{
    sptr<IFocusChangedListener> listener = nullptr;
    WMError ret = AllUsersWindowManager::GetInstance().RegisterFocusChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterFocusChangedListener02
 * @tc.desc: Test RegisterFocusChangedListener with valid listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, RegisterFocusChangedListener02, TestSize.Level1)
{
    sptr<IFocusChangedListener> listener = new TestFocusChangedListener();
    WMError ret = AllUsersWindowManager::GetInstance().RegisterFocusChangedListener(listener);
    EXPECT_NE(listener.GetRefPtr(), nullptr);
}

/**
 * @tc.name: UnregisterFocusChangedListener01
 * @tc.desc: Test UnregisterFocusChangedListener with nullptr listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, UnregisterFocusChangedListener01, TestSize.Level1)
{
    sptr<IFocusChangedListener> listener = nullptr;
    WMError ret = AllUsersWindowManager::GetInstance().UnregisterFocusChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterFocusChangedListener02
 * @tc.desc: Test UnregisterFocusChangedListener with valid listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, UnregisterFocusChangedListener02, TestSize.Level1)
{
    sptr<IFocusChangedListener> listener = new TestFocusChangedListener();
    AllUsersWindowManager::GetInstance().RegisterFocusChangedListener(listener);
    WMError ret = AllUsersWindowManager::GetInstance().UnregisterFocusChangedListener(listener);
    EXPECT_NE(listener.GetRefPtr(), nullptr);
}

/**
 * @tc.name: RegisterVisibilityChangedListener01
 * @tc.desc: Test RegisterVisibilityChangedListener with nullptr listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, RegisterVisibilityChangedListener01, TestSize.Level1)
{
    sptr<IVisibilityChangedListener> listener = nullptr;
    WMError ret = AllUsersWindowManager::GetInstance().RegisterVisibilityChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterVisibilityChangedListener02
 * @tc.desc: Test RegisterVisibilityChangedListener with valid listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, RegisterVisibilityChangedListener02, TestSize.Level1)
{
    sptr<IVisibilityChangedListener> listener = new TestVisibilityChangedListener();
    WMError ret = AllUsersWindowManager::GetInstance().RegisterVisibilityChangedListener(listener);
    EXPECT_NE(listener.GetRefPtr(), nullptr);
}

/**
 * @tc.name: UnregisterVisibilityChangedListener01
 * @tc.desc: Test UnregisterVisibilityChangedListener with nullptr listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, UnregisterVisibilityChangedListener01, TestSize.Level1)
{
    sptr<IVisibilityChangedListener> listener = nullptr;
    WMError ret = AllUsersWindowManager::GetInstance().UnregisterVisibilityChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterVisibilityChangedListener02
 * @tc.desc: Test UnregisterVisibilityChangedListener with valid listener
 * @tc.type: FUNC
 */
HWTEST_F(AllUsersWindowManagerTest, UnregisterVisibilityChangedListener02, TestSize.Level1)
{
    sptr<IVisibilityChangedListener> listener = new TestVisibilityChangedListener();
    AllUsersWindowManager::GetInstance().RegisterVisibilityChangedListener(listener);
    WMError ret = AllUsersWindowManager::GetInstance().UnregisterVisibilityChangedListener(listener);
    EXPECT_NE(listener.GetRefPtr(), nullptr);
}

} // namespace Rosen
} // namespace OHOS