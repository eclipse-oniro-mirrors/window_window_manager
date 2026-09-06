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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_session.h"
#include "oh_window_event_filter.h"
#include "window_option.h"
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class MockWindowForFilter : public WindowSceneSessionImpl {
public:
    explicit MockWindowForFilter(const sptr<WindowOption>& option) : WindowSceneSessionImpl(option) {}

    MOCK_METHOD(WMError, SetKeyEventFilter, (KeyEventFilterFunc filter), (override));
    MOCK_METHOD(WMError, ClearKeyEventFilter, (), (override));
    MOCK_METHOD(WMError, SetMouseEventFilter, (MouseEventFilterFunc filter), (override));
    MOCK_METHOD(WMError, ClearMouseEventFilter, (), (override));
    MOCK_METHOD(WMError, SetTouchEventFilter, (TouchEventFilterFunc filter), (override));
    MOCK_METHOD(WMError, ClearTouchEventFilter, (), (override));

    MOCK_METHOD(WMError, SaveNativeKeyEventFilter, (NativeKeyEventFilter), (override));
    MOCK_METHOD(NativeKeyEventFilter, GetNativeKeyEventFilter, (), (const, override));
    MOCK_METHOD(WMError, ClearNativeKeyEventFilter, (), (override));

    MOCK_METHOD(WMError, SaveNativeMouseEventFilter, (NativeMouseEventFilter), (override));
    MOCK_METHOD(NativeMouseEventFilter, GetNativeMouseEventFilter, (), (const, override));
    MOCK_METHOD(WMError, ClearNativeMouseEventFilter, (), (override));

    MOCK_METHOD(WMError, SaveNativeTouchEventFilter, (NativeTouchEventFilter), (override));
    MOCK_METHOD(NativeTouchEventFilter, GetNativeTouchEventFilter, (), (const, override));
    MOCK_METHOD(WMError, ClearNativeTouchEventFilter, (), (override));
};

class OHWindowEventFilterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void OHWindowEventFilterTest::SetUpTestCase() {}

void OHWindowEventFilterTest::TearDownTestCase() {}

void OHWindowEventFilterTest::SetUp() {}

void OHWindowEventFilterTest::TearDown() {}

namespace {
static bool KeyEventFilterFunc(Input_KeyEvent* keyEvent)
{
    return false;
}

static bool MouseEventFilterFunc(Input_MouseEvent* mouseEvent)
{
    return false;
}

static bool TouchEventFilterFunc(Input_TouchEvent* touchEvent)
{
    return false;
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_RegisterKeyEventFilter, TestSize.Level0)
{
    // Branch 1: Parameter validation - filter is nullptr (must setup window first)
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("test_window_param");
    auto mockWindow0 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow0->property_->SetPersistentId(99);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_param", std::make_pair(99, mockWindow0)));

    auto ret = OH_NativeWindowManager_RegisterKeyEventFilter(99, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);

    // Branch 2: Window does not exist - invalid windowId (clear map to ensure window not found)
    WindowSceneSessionImpl::windowSessionMap_.clear();
    ret = OH_NativeWindowManager_RegisterKeyEventFilter(-1, KeyEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 3: SERVICE_ERROR - SetKeyEventFilter returns error
    option->SetWindowName("test_window_error");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_error", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, SetKeyEventFilter(_)).WillOnce(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ret = OH_NativeWindowManager_RegisterKeyEventFilter(100, KeyEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::SERVICE_ERROR), ret);

    // Branch 4: Success path - window exists and SetKeyEventFilter succeeds
    option->SetWindowName("test_window_ok");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_ok", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, SetKeyEventFilter(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*mockWindow2, SaveNativeKeyEventFilter(_)).WillOnce(Return(WMError::WM_OK));
    ret = OH_NativeWindowManager_RegisterKeyEventFilter(101, KeyEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_UnregisterKeyEventFilter, TestSize.Level0)
{
    // Branch 1: Window does not exist - invalid windowId
    auto ret = OH_NativeWindowManager_UnregisterKeyEventFilter(-1);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 2: SERVICE_ERROR - ClearKeyEventFilter returns error
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("test_window_error");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_error", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, ClearKeyEventFilter()).WillOnce(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ret = OH_NativeWindowManager_UnregisterKeyEventFilter(100);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::SERVICE_ERROR), ret);

    // Branch 3: Success path - window exists and ClearKeyEventFilter succeeds
    option->SetWindowName("test_window_ok");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_ok", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, ClearKeyEventFilter()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*mockWindow2, ClearNativeKeyEventFilter()).WillOnce(Return(WMError::WM_OK));
    ret = OH_NativeWindowManager_UnregisterKeyEventFilter(101);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_RegisterMouseEventFilter, TestSize.Level0)
{
    // Branch 1: Parameter validation - filter is nullptr (must setup window first)
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("test_window_param");
    auto mockWindow0 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow0->property_->SetPersistentId(99);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_param", std::make_pair(99, mockWindow0)));

    auto ret = OH_NativeWindowManager_RegisterMouseEventFilter(99, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);

    // Branch 2: Window does not exist - invalid windowId (clear map to ensure window not found)
    WindowSceneSessionImpl::windowSessionMap_.clear();
    ret = OH_NativeWindowManager_RegisterMouseEventFilter(-1, MouseEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 3: SERVICE_ERROR - SetMouseEventFilter returns error
    option->SetWindowName("test_window_error");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_error", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, SetMouseEventFilter(_)).WillOnce(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ret = OH_NativeWindowManager_RegisterMouseEventFilter(100, MouseEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::SERVICE_ERROR), ret);

    // Branch 4: Success path - window exists and SetMouseEventFilter succeeds
    option->SetWindowName("test_window_ok");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_ok", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, SetMouseEventFilter(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*mockWindow2, SaveNativeMouseEventFilter(_)).WillOnce(Return(WMError::WM_OK));
    ret = OH_NativeWindowManager_RegisterMouseEventFilter(101, MouseEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_UnregisterMouseEventFilter, TestSize.Level0)
{
    // Branch 1: Window does not exist - invalid windowId
    auto ret = OH_NativeWindowManager_UnregisterMouseEventFilter(-1);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 2: SERVICE_ERROR - ClearMouseEventFilter returns error
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("test_window_error");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_error", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, ClearMouseEventFilter()).WillOnce(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ret = OH_NativeWindowManager_UnregisterMouseEventFilter(100);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::SERVICE_ERROR), ret);

    // Branch 3: Success path - window exists and ClearMouseEventFilter succeeds
    option->SetWindowName("test_window_ok");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_ok", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, ClearMouseEventFilter()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*mockWindow2, ClearNativeMouseEventFilter()).WillOnce(Return(WMError::WM_OK));
    ret = OH_NativeWindowManager_UnregisterMouseEventFilter(101);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_RegisterTouchEventFilter, TestSize.Level0)
{
    // Branch 1: Parameter validation - filter is nullptr (must setup window first)
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("test_window_param");
    auto mockWindow0 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow0->property_->SetPersistentId(99);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_param", std::make_pair(99, mockWindow0)));

    auto ret = OH_NativeWindowManager_RegisterTouchEventFilter(99, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);

    // Branch 2: Window does not exist - invalid windowId (clear map to ensure window not found)
    WindowSceneSessionImpl::windowSessionMap_.clear();
    ret = OH_NativeWindowManager_RegisterTouchEventFilter(-1, TouchEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 3: SERVICE_ERROR - SetTouchEventFilter returns error
    option->SetWindowName("test_window_error");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_error", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, SetTouchEventFilter(_)).WillOnce(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ret = OH_NativeWindowManager_RegisterTouchEventFilter(100, TouchEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::SERVICE_ERROR), ret);

    // Branch 4: Success path - window exists and SetTouchEventFilter succeeds
    option->SetWindowName("test_window_ok");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_ok", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, SetTouchEventFilter(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*mockWindow2, SaveNativeTouchEventFilter(_)).WillOnce(Return(WMError::WM_OK));
    ret = OH_NativeWindowManager_RegisterTouchEventFilter(101, TouchEventFilterFunc);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_UnregisterTouchEventFilter, TestSize.Level0)
{
    // Branch 1: Window does not exist - invalid windowId
    auto ret = OH_NativeWindowManager_UnregisterTouchEventFilter(-1);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 2: SERVICE_ERROR - ClearTouchEventFilter returns error
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("test_window_error");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_error", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, ClearTouchEventFilter()).WillOnce(Return(WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    ret = OH_NativeWindowManager_UnregisterTouchEventFilter(100);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::SERVICE_ERROR), ret);

    // Branch 3: Success path - window exists and ClearTouchEventFilter succeeds
    option->SetWindowName("test_window_ok");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_ok", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, ClearTouchEventFilter()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*mockWindow2, ClearNativeTouchEventFilter()).WillOnce(Return(WMError::WM_OK));
    ret = OH_NativeWindowManager_UnregisterTouchEventFilter(101);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_GetKeyEventFilter, TestSize.Level0)
{
    OH_NativeWindowManager_KeyEventFilter outFilter = nullptr;

    // Branch 1: Parameter validation - outFilter is nullptr (must setup window first)
    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("test_window_param");
    auto mockWindow0 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow0->property_->SetPersistentId(99);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_param", std::make_pair(99, mockWindow0)));

    auto ret = OH_NativeWindowManager_GetKeyEventFilter(99, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);

    // Branch 2: Window does not exist - invalid windowId (clear map to ensure window not found)
    WindowSceneSessionImpl::windowSessionMap_.clear();
    ret = OH_NativeWindowManager_GetKeyEventFilter(-1, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 3: Success path - window exists but filter not registered (returns nullptr)
    option1->SetWindowName("test_window_null");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_null", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, GetNativeKeyEventFilter()).WillOnce(Return(nullptr));
    ret = OH_NativeWindowManager_GetKeyEventFilter(100, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(nullptr, outFilter);

    // Branch 4: Success path - window exists and filter registered (returns valid filter)
    option1->SetWindowName("test_window_valid");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_valid", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, GetNativeKeyEventFilter()).WillOnce(Return(KeyEventFilterFunc));
    ret = OH_NativeWindowManager_GetKeyEventFilter(101, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(KeyEventFilterFunc, outFilter);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_GetMouseEventFilter, TestSize.Level0)
{
    OH_NativeWindowManager_MouseEventFilter outFilter = nullptr;

    // Branch 1: Parameter validation - outFilter is nullptr (must setup window first)
    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("test_window_param");
    auto mockWindow0 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow0->property_->SetPersistentId(99);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_param", std::make_pair(99, mockWindow0)));

    auto ret = OH_NativeWindowManager_GetMouseEventFilter(99, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);

    // Branch 2: Window does not exist - invalid windowId (clear map to ensure window not found)
    WindowSceneSessionImpl::windowSessionMap_.clear();
    ret = OH_NativeWindowManager_GetMouseEventFilter(-1, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 3: Success path - window exists but filter not registered (returns nullptr)
    option1->SetWindowName("test_window_null");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_null", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, GetNativeMouseEventFilter()).WillOnce(Return(nullptr));
    ret = OH_NativeWindowManager_GetMouseEventFilter(100, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(nullptr, outFilter);

    // Branch 4: Success path - window exists and filter registered (returns valid filter)
    option1->SetWindowName("test_window_valid");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_valid", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, GetNativeMouseEventFilter()).WillOnce(Return(MouseEventFilterFunc));
    ret = OH_NativeWindowManager_GetMouseEventFilter(101, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(MouseEventFilterFunc, outFilter);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}

HWTEST_F(OHWindowEventFilterTest, OH_NativeWindowManager_GetTouchEventFilter, TestSize.Level0)
{
    OH_NativeWindowManager_TouchEventFilter outFilter = nullptr;

    // Branch 1: Parameter validation - outFilter is nullptr (must setup window first)
    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("test_window_param");
    auto mockWindow0 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow0->property_->SetPersistentId(99);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_param", std::make_pair(99, mockWindow0)));

    auto ret = OH_NativeWindowManager_GetTouchEventFilter(99, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);

    // Branch 2: Window does not exist - invalid windowId (clear map to ensure window not found)
    WindowSceneSessionImpl::windowSessionMap_.clear();
    ret = OH_NativeWindowManager_GetTouchEventFilter(-1, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::INVALID_WINDOW_ID), ret);

    // Branch 3: Success path - window exists but filter not registered (returns nullptr)
    option1->SetWindowName("test_window_null");
    auto mockWindow1 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow1->property_->SetPersistentId(100);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_null", std::make_pair(100, mockWindow1)));

    EXPECT_CALL(*mockWindow1, GetNativeTouchEventFilter()).WillOnce(Return(nullptr));
    ret = OH_NativeWindowManager_GetTouchEventFilter(100, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(nullptr, outFilter);

    // Branch 4: Success path - window exists and filter registered (returns valid filter)
    option1->SetWindowName("test_window_valid");
    auto mockWindow2 = sptr<MockWindowForFilter>::MakeSptr(option1);
    mockWindow2->property_->SetPersistentId(101);

    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(
        std::make_pair("test_window_valid", std::make_pair(101, mockWindow2)));

    EXPECT_CALL(*mockWindow2, GetNativeTouchEventFilter()).WillOnce(Return(TouchEventFilterFunc));
    ret = OH_NativeWindowManager_GetTouchEventFilter(101, &outFilter);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(TouchEventFilterFunc, outFilter);

    WindowSceneSessionImpl::windowSessionMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS