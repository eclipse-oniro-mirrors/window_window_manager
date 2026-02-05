/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "window_scene_session_impl.h"
#include "mock_session.h"
#include "mock_session_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSceneSessionImplEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowSceneSessionImplEventTest::SetUpTestCase()
{
}

void WindowSceneSessionImplEventTest::TearDownTestCase()
{
}

void WindowSceneSessionImplEventTest::SetUp()
{
}

void WindowSceneSessionImplEventTest::TearDown()
{
}

namespace {
/**
 * @tc.name: LockCursor
 * @tc.desc: LockCursor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplEventTest, LockCursor, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("LockCursor");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->LockCursor(1, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->persistentId_ = 10;
    window->state_ = WindowState::STATE_SHOWN;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    ret = window->LockCursor(1, true);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UnlockCursor
 * @tc.desc: UnlockCursor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplEventTest, UnlockCursor, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnlockCursor");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->UnlockCursor(1);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->persistentId_ = 10;
    window->state_ = WindowState::STATE_SHOWN;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    ret = window->UnlockCursor(1);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetReceiveDragEventEnabled
 * @tc.desc: SetReceiveDragEventEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplEventTest, SetReceiveDragEventEnabled, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetReceiveDragEventEnabled");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->SetReceiveDragEventEnabled(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->persistentId_ = 11;
    window->state_ = WindowState::STATE_SHOWN;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    ret = window->SetReceiveDragEventEnabled(true);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: IsReceiveDragEventEnabled
 * @tc.desc: IsReceiveDragEventEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplEventTest, IsReceiveDragEventEnabled, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsReceiveDragEventEnabled");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->SetReceiveDragEventEnabled(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    auto isRet =  window->IsReceiveDragEventEnabled();
    EXPECT_EQ(isRet, true);

    window->property_->persistentId_ = 11;
    window->state_ = WindowState::STATE_SHOWN;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;

    ret = window->SetReceiveDragEventEnabled(false);
    EXPECT_EQ(ret, WMError::WM_OK);

    isRet =  window->IsReceiveDragEventEnabled();
    EXPECT_EQ(isRet, false);
}

/**
 * @tc.name: SetSeparationTouchEnabled
 * @tc.desc: SetSeparationTouchEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplEventTest, SetSeparationTouchEnabled, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSeparationTouchEnabled");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->SetSeparationTouchEnabled(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->persistentId_ = 11;
    window->state_ = WindowState::STATE_SHOWN;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;
    ret = window->SetSeparationTouchEnabled(true);
    EXPECT_EQ(ret, WMError::WM_OK);

    window->property_->persistentId_ = 12;
    window->state_ = WindowState::STATE_SHOWN;
    window->hostSession_ = session;
    ret = window->SetSeparationTouchEnabled(false);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: IsSeparationTouchEnabled
 * @tc.desc: IsSeparationTouchEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplEventTest, IsSeparationTouchEnabled, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsSeparationTouchEnabled");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->SetSeparationTouchEnabled(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    auto isRet =  window->IsSeparationTouchEnabled();
    EXPECT_EQ(isRet, true);

    window->property_->persistentId_ = 11;
    window->state_ = WindowState::STATE_SHOWN;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = session;

    ret = window->SetSeparationTouchEnabled(false);
    EXPECT_EQ(ret, WMError::WM_OK);

    isRet =  window->IsSeparationTouchEnabled();
    EXPECT_EQ(isRet, false);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
