/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "mock_window_adapter_lite.h"
#include "singleton_mocker.h"
#include "window_manager_lite.cpp"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Rosen {
using Mocker = SingletonMocker<WindowAdapterLite, MockWindowAdapterLite>;

class TestCameraWindowChangedListener : public ICameraWindowChangedListener {
public:
    void OnCameraWindowChange(uint32_t accessTokenId, bool isShowing) override {};
};

class WindowManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowManagerLiteTest::SetUpTestCase()
{
}

void WindowManagerLiteTest::TearDownTestCase()
{
}

void WindowManagerLiteTest::SetUp()
{
}

void WindowManagerLiteTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: using windowManagerLite to get focus info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetFocusWindowInfo, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    FocusChangeInfo infosInput;
    FocusChangeInfo infosResult;
    infosResult.pid_ = 10;
    infosResult.uid_ = 11;
    infosResult.displayId_ = 12;
    infosResult.windowId_ = 13;
    EXPECT_CALL(m->Mock(), GetFocusWindowInfo(_)).Times(1).WillOnce(DoAll(SetArgReferee<0>(infosResult), Return()));
    WindowManagerLite::GetInstance().GetFocusWindowInfo(infosInput);
    ASSERT_EQ(infosInput.windowId_, infosResult.windowId_);
    ASSERT_EQ(infosInput.uid_, infosResult.uid_);
    ASSERT_EQ(infosInput.pid_, infosResult.pid_);
    ASSERT_EQ(infosInput.displayId_, infosResult.displayId_);
}

/**
 * @tc.name: UpdateCameraWindowStatus
 * @tc.desc: UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateCameraWindowStatus, Function | SmallTest | Level2)
{
    uint32_t accessTokenId = 0;
    bool isShowing = true;
    auto ret = 0;
    WindowManagerLite::GetInstance().UpdateCameraWindowStatus(accessTokenId, isShowing);
    ASSERT_EQ(0, ret);
}


/**
 * @tc.name: RegisterCameraWindowChangedListener01
 * @tc.desc: check RegisterCameraWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterCameraWindowChangedListener01, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->cameraWindowChangedListenerAgent_;
    auto oldListeners = windowManager.pImpl_->cameraWindowChangedListeners_;
    windowManager.pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
    windowManager.pImpl_->cameraWindowChangedListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterCameraWindowChangedListener(nullptr));

    sptr<TestCameraWindowChangedListener> listener = new TestCameraWindowChangedListener();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterCameraWindowChangedListener(listener));

    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterCameraWindowChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->cameraWindowChangedListeners_.size());

    // to check that the same listner can not be registered twice
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterCameraWindowChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->cameraWindowChangedListeners_.size());

    windowManager.pImpl_->cameraWindowChangedListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->cameraWindowChangedListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterCameraWindowChangedListener01
 * @tc.desc: check UnregisterCameraWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterCameraWindowChangedListener01, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->cameraWindowChangedListenerAgent_;
    auto oldListeners = windowManager.pImpl_->cameraWindowChangedListeners_;
    windowManager.pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
    windowManager.pImpl_->cameraWindowChangedListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.UnregisterCameraWindowChangedListener(nullptr));

    sptr<TestCameraWindowChangedListener> listener1 = new TestCameraWindowChangedListener();
    sptr<TestCameraWindowChangedListener> listener2 = new TestCameraWindowChangedListener();
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterCameraWindowChangedListener(listener1));

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterCameraWindowChangedListener(listener1);
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterCameraWindowChangedListener(listener2);
    ASSERT_EQ(2, windowManager.pImpl_->cameraWindowChangedListeners_.size());
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterCameraWindowChangedListener(listener1));

    EXPECT_CALL(m->Mock(), UnregisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterCameraWindowChangedListener(listener2));
    ASSERT_EQ(0, windowManager.pImpl_->cameraWindowChangedListeners_.size());
    ASSERT_EQ(nullptr, windowManager.pImpl_->cameraWindowChangedListenerAgent_);

    windowManager.pImpl_->cameraWindowChangedListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterCameraWindowChangedListener(listener1));
    ASSERT_EQ(0, windowManager.pImpl_->cameraWindowChangedListeners_.size());

    windowManager.pImpl_->cameraWindowChangedListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->cameraWindowChangedListeners_ = oldListeners;
}

/**
 * @tc.name: Test01
 * @tc.desc: check UnregisterCameraWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, Test01, Function | SmallTest | Level2)
{
    sptr<IFocusChangedListener> listener = nullptr;
    auto ret1 = WindowManagerLite::GetInstance().RegisterFocusChangedListener(listener);
    auto ret2 = WindowManagerLite::GetInstance().UnregisterFocusChangedListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret2);
    sptr<IVisibilityChangedListener> listener1 = nullptr;
    auto ret3 = WindowManagerLite::GetInstance().RegisterVisibilityChangedListener(listener1);
    auto ret4 = WindowManagerLite::GetInstance().UnregisterVisibilityChangedListener(listener1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret3);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret4);
    std::vector<sptr<WindowVisibilityInfo>> infos;
    ASSERT_EQ(WMError::WM_OK, WindowManagerLite::GetInstance().GetVisibilityWindowInfo(infos));
    sptr<IDrawingContentChangedListener> listener2 = nullptr;
    auto ret5 = WindowManagerLite::GetInstance().RegisterDrawingContentChangedListener(listener2);
    auto ret6 = WindowManagerLite::GetInstance().UnregisterDrawingContentChangedListener(listener2);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret5);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret6);
    sptr<IWindowModeChangedListener> listener3 = nullptr;
    auto ret7 = WindowManagerLite::GetInstance().RegisterWindowModeChangedListener(listener3);
    auto ret8 = WindowManagerLite::GetInstance().UnregisterWindowModeChangedListener(listener3);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret7);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret8);
}

/**
 * @tc.name: GetTopNMainWindowInfos
 * @tc.desc: get top main window info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetTopNMainWindowInfos, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<TopNMainWindowInfo> topNInfo;
    std::vector<TopNMainWindowInfo> topNInfoResult;
    TopNMainWindowInfo info1;
    info1.pid = 1900;
    info1.bundleName = "calendar";

    TopNMainWindowInfo info2;
    info1.pid = 1901;
    info1.bundleName = "settings";

    TopNMainWindowInfo info3;
    info1.pid = 1902;
    info1.bundleName = "photos";

    topNInfoResult.push_back(info1);
    topNInfoResult.push_back(info2);
    topNInfoResult.push_back(info3);

    int32_t topN = 3;

    EXPECT_CALL(m->Mock(), GetTopNMainWindowInfos(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(infosResult),
        Return(WMError::WM_OK)));

    WindowManagerLite::GetInstance().GetTopNMainWindowInfos(topN, topNInfo);

    auto it1 = topNInfo.begin();
    auto it2 = topNInfoResult.begin();
    for (; it1 != topNInfo.end() && it2 != topNInfoResult.end(); it1++, it2++) {
        ASSERT_EQ(it1->pid, it2->pid);
        ASSERT_EQ(it1->bundleName, it2->bundleName);
    }
}
}
}