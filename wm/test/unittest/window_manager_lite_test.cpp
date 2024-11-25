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

class TestWindowModeChangedListener : public IWindowModeChangedListener {
public:
    void OnWindowModeUpdate(WindowModeType mode) override {};
};

class TestWMSConnectionChangedListener : public IWMSConnectionChangedListener {
public:
    void OnConnected(int32_t userId, int32_t screenId) override {};
    void OnDisconnected(int32_t userId, int32_t screenId) override {};
};

class TestFocusChangedListener : public IFocusChangedListener {
public:
    void OnFocused(const sptr<FocusChangeInfo>& focusChangeInfo) override {};
    void OnUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo) override {};
};

class TestDrawingContentChangedListener : public IDrawingContentChangedListener {
public:
    void OnWindowDrawingContentChanged(
        const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingInfo) override {};
};

class TestVisibilityChangedListener : public IVisibilityChangedListener {
public:
    void OnWindowVisibilityChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfo) override {};
};

class IWMSConnectionChangedListenerSon : public IWMSConnectionChangedListener {
public:
    void OnConnected(int32_t userId, int32_t screenId) override {};
    void OnDisconnected(int32_t userId, int32_t screenId) override {};
};

class TestWindowStyleChangedListener : public IWindowStyleChangedListener {
public:
    void OnWindowStyleUpdate(WindowStyleType styleType)
    {
        TLOGI(WmsLogTag::DMS, "TestWindowStyleChangedListener");
    }
};

class TestPiPStateChangedListener : public IPiPStateChangedListener {
public:
    void OnPiPStateChanged(const std::string& bundleName, bool isForeground) override {}
};

class TestWindowUpdateListener : public IWindowUpdateListener {
public:
    void OnWindowUpdate(const std::vector<sptr<AccessibilityWindowInfo>>& infos, WindowUpdateType type) override
    {
        WLOGI("TestWindowUpdateListener");
    }
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
 * @tc.name: UpdateCameraWindowStatus01
 * @tc.desc: UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateCameraWindowStatus01, Function | SmallTest | Level2)
{
    uint32_t accessTokenId = 0;
    bool isShowing = true;
    auto ret = 0;
    WindowManagerLite::GetInstance().UpdateCameraWindowStatus(accessTokenId, isShowing);
    ASSERT_EQ(0, ret);
}

/**
 * @tc.name: UpdateCameraWindowStatus02
 * @tc.desc: UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateCameraWindowStatus02, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<TestCameraWindowChangedListener> listener = new (std::nothrow) TestCameraWindowChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterCameraWindowChangedListener(listener));

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
    sptr<IWMSConnectionChangedListener> listener4 = nullptr;
    WMError res9 = WindowManagerLite::GetInstance().RegisterWMSConnectionChangedListener(listener4);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res9);
    WMError res10 = WindowManagerLite::GetInstance().UnregisterWMSConnectionChangedListener();
    ASSERT_EQ(WMError::WM_OK, res10);
}

/**
 * @tc.name: Test04
 * @tc.desc: Test04
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, Test04, Function | SmallTest | Level2)
{
    WindowManagerLite lite;
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    ASSERT_NE(lite.pImpl_, nullptr);
    lite.pImpl_->NotifyFocused(focusChangeInfo);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    focusChangeInfo = new FocusChangeInfo();
    lite.pImpl_->NotifyFocused(focusChangeInfo);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    lite.pImpl_->NotifyWindowVisibilityInfoChanged(windowVisibilityInfos);
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos;
    lite.pImpl_->NotifyWindowDrawingContentInfoChanged(windowDrawingContentInfos);
    lite.pImpl_->NotifyWindowModeChange(WindowModeType::WINDOW_MODE_SPLIT);
}

/**
 * @tc.name: RegisterWindowModeChangedListener02
 * @tc.desc: check RegisterWindowModeChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowModeChangedListener02, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowModeListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowModeListeners_;
    windowManager.pImpl_->windowModeListenerAgent_ = nullptr;
    windowManager.pImpl_->windowModeListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWindowModeChangedListener(nullptr));

    sptr<TestWindowModeChangedListener> listener = new (std::nothrow) TestWindowModeChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWindowModeChangedListener(listener));

    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowModeChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->windowModeListeners_.size());

    // to check that the same listner can not be registered twice
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowModeChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->windowModeListeners_.size());

    windowManager.pImpl_->windowModeListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->windowModeListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowModeChangedListener02
 * @tc.desc: check UnregisterWindowModeChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowModeChangedListener02, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowModeListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowModeListeners_;
    windowManager.pImpl_->windowModeListenerAgent_ = nullptr;
    windowManager.pImpl_->windowModeListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.UnregisterWindowModeChangedListener(nullptr));

    sptr<TestWindowModeChangedListener> listener1 = new (std::nothrow) TestWindowModeChangedListener();
    ASSERT_TRUE(listener1 != nullptr);
    sptr<TestWindowModeChangedListener> listener2 = new (std::nothrow) TestWindowModeChangedListener();
    ASSERT_TRUE(listener2 != nullptr);
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowModeChangedListener(listener1));

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterWindowModeChangedListener(listener1);
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterWindowModeChangedListener(listener2);
    ASSERT_EQ(2, windowManager.pImpl_->windowModeListeners_.size());
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowModeChangedListener(listener1));

    EXPECT_CALL(m->Mock(), UnregisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowModeChangedListener(listener2));
    ASSERT_EQ(0, windowManager.pImpl_->windowModeListeners_.size());
    ASSERT_EQ(nullptr, windowManager.pImpl_->windowModeListenerAgent_);

    windowManager.pImpl_->windowModeListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowModeChangedListener(listener1));
    ASSERT_EQ(0, windowManager.pImpl_->windowModeListeners_.size());

    windowManager.pImpl_->windowModeListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->windowModeListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener02
 * @tc.desc: check RegisterWMSConnectionChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWMSConnectionChangedListener02, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWMSConnectionChangedListener(nullptr));

    sptr<TestWMSConnectionChangedListener> listener = new (std::nothrow) TestWMSConnectionChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWMSConnectionChangedListener(listener));

    windowManager.pImpl_->wmsConnectionChangedListener_ = nullptr;
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWMSConnectionChangedListener(listener));
}

/**
 * @tc.name: GetMainWindowInfos
 * @tc.desc: get top main window info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetMainWindowInfos, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<MainWindowInfo> topNInfo;
    std::vector<MainWindowInfo> topNInfoResult;
    MainWindowInfo info1;
    info1.pid_ = 1900;
    info1.bundleName_ = "calendar";

    MainWindowInfo info2;
    info1.pid_ = 1901;
    info1.bundleName_ = "settings";

    MainWindowInfo info3;
    info1.pid_ = 1902;
    info1.bundleName_ = "photos";

    topNInfoResult.push_back(info1);
    topNInfoResult.push_back(info2);
    topNInfoResult.push_back(info3);

    int32_t topN = 3;

    EXPECT_CALL(m->Mock(), GetMainWindowInfos(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(topNInfoResult),
        Return(WMError::WM_OK)));

    WindowManagerLite::GetInstance().GetMainWindowInfos(topN, topNInfo);

    auto it1 = topNInfo.begin();
    auto it2 = topNInfoResult.begin();
    for (; it1 != topNInfo.end() && it2 != topNInfoResult.end(); it1++, it2++) {
        ASSERT_EQ(it1->pid_, it2->pid_);
        ASSERT_EQ(it1->bundleName_, it2->bundleName_);
    }
}

/**
 * @tc.name: TestUpdateFocusChangeInfo
 * @tc.desc: TestUpdateFocusChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, TestUpdateFocusChangeInfo, Function | SmallTest | Level2)
{
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    WindowManagerLite::GetInstance().UpdateFocusChangeInfo(focusChangeInfo, true);
    sptr<FocusChangeInfo> focusChangeInfo1 = new FocusChangeInfo();
    WindowManagerLite::GetInstance().UpdateFocusChangeInfo(focusChangeInfo1, true);
    WindowManagerLite::GetInstance().UpdateFocusChangeInfo(focusChangeInfo1, false);
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WindowManagerLite::GetInstance().UpdateWindowVisibilityInfo(infos);
    std::vector<sptr<WindowDrawingContentInfo>> infos1;
    WindowManagerLite::GetInstance().UpdateWindowDrawingContentInfo(infos1);
    WindowManagerLite lite;
    lite.destroyed_ = true;
    lite.OnRemoteDied();
    lite.destroyed_ = false;
    lite.OnRemoteDied();
    ASSERT_EQ(nullptr, lite.pImpl_->focusChangedListenerAgent_);
    ASSERT_EQ(nullptr, lite.pImpl_->windowUpdateListenerAgent_);
    ASSERT_EQ(nullptr, lite.pImpl_->windowDrawingContentListenerAgent_);
}

/**
 * @tc.name: NotifyFocusedWithUn
 * @tc.desc: NotifyFocused With MotifyUnFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyFocusedWithUn, Function | SmallTest | Level2)
{
    WindowManagerLite lite;
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    lite.pImpl_->NotifyFocused(focusChangeInfo);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    ASSERT_EQ(focusChangeInfo, nullptr);
    focusChangeInfo = new (std::nothrow) FocusChangeInfo();
    lite.pImpl_->focusChangedListeners_.push_back(nullptr);
    sptr<TestFocusChangedListener> testFocusChangedListener = new (std::nothrow) TestFocusChangedListener();
    lite.pImpl_->focusChangedListeners_.push_back(testFocusChangedListener);
    lite.pImpl_->NotifyFocused(focusChangeInfo);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    ASSERT_NE(focusChangeInfo, nullptr);
}

/**
 * @tc.name: NotifyWindowDrawingContentInfoChanged02
 * @tc.desc: NotifyWindowDrawingContentInfoChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowDrawingContentInfoChanged02, Function | SmallTest | Level2)
{
    WindowManagerLite lite;
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<TestDrawingContentChangedListener> listener = new (std::nothrow) TestDrawingContentChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterDrawingContentChangedListener(listener));

    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos;
    lite.pImpl_->NotifyWindowDrawingContentInfoChanged(windowDrawingContentInfos);
}

/**
 * @tc.name: NotifyWindowModeChange02
 * @tc.desc: NotifyWindowModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowModeChange02, Function | SmallTest | Level2)
{
    WindowManagerLite lite;
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<TestWindowModeChangedListener> listener = new (std::nothrow) TestWindowModeChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowModeChangedListener(listener));

    lite.pImpl_->NotifyWindowModeChange(WindowModeType::WINDOW_MODE_SPLIT);
}

/**
 * @tc.name: NotifyWindowVisibilityInfoChanged02
 * @tc.desc: NotifyWindowVisibilityInfoChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowVisibilityInfoChanged02, Function | SmallTest | Level2)
{
    WindowManagerLite lite;
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<TestVisibilityChangedListener> listener = new (std::nothrow) TestVisibilityChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterVisibilityChangedListener(listener));

    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    lite.pImpl_->NotifyWindowVisibilityInfoChanged(windowVisibilityInfos);
}

/**
 * @tc.name: NotifyUnfocused01
 * @tc.desc: NotifyUnfocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyUnfocused01, Function | SmallTest | Level2)
{
    WindowManagerLite lite;
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<FocusChangeInfo> focusChangeInfo = new (std::nothrow) FocusChangeInfo();
    ASSERT_TRUE(focusChangeInfo != nullptr);

    sptr<TestFocusChangedListener> listener = new (std::nothrow) TestFocusChangedListener();
    ASSERT_TRUE(listener != nullptr);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterFocusChangedListener(listener));
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
}

/**
 * @tc.name: GetAllMainWindowInfos001
 * @tc.desc: GetAllMainWindowInfos001
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAllMainWindowInfos001, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<MainWindowInfo> infos;
    std::vector<MainWindowInfo> infosResult;
    MainWindowInfo info1;
    info1.pid_ = 1900;
    info1.bundleName_ = "calendar";

    MainWindowInfo info2;
    info1.pid_ = 1901;
    info1.bundleName_ = "settings";

    MainWindowInfo info3;
    info1.pid_ = 1902;
    info1.bundleName_ = "photos";

    infosResult.push_back(info1);
    infosResult.push_back(info2);
    infosResult.push_back(info3);

    EXPECT_CALL(m->Mock(), GetAllMainWindowInfos(_)).Times(1).WillOnce(DoAll(SetArgReferee<0>(infosResult),
        Return(WMError::WM_OK)));
    
    auto errorCode = WindowManagerLite::GetInstance().GetAllMainWindowInfos(infos);
    ASSERT_EQ(WMError::WM_OK, errorCode);
    auto it1 = infos.begin();
    auto it2 = infosResult.begin();
    for (; it1 != infos.end() && it2 != infosResult.end(); it1++, it2++) {
        ASSERT_EQ(it1->pid_, it2->pid_);
        ASSERT_EQ(it1->bundleName_, it2->bundleName_);
        ASSERT_EQ(it1->persistentId_, it2->persistentId_);
        ASSERT_EQ(it1->bundleType_, it2->bundleType_);
    }
}

/**
 * @tc.name: GetAllMainWindowInfos002
 * @tc.desc: GetAllMainWindowInfos002
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAllMainWindowInfos002, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<MainWindowInfo> infos;
    MainWindowInfo info1;
    info1.pid_ = 1900;
    info1.bundleName_ = "calendar";
    infos.push_back(info1);

    EXPECT_CALL(m->Mock(), GetAllMainWindowInfos(_)).Times(0).WillOnce(DoAll(Return(WMError::WM_OK)));
    
    auto errorCode = WindowManagerLite::GetInstance().GetAllMainWindowInfos(infos);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, errorCode);
}

/**
 * @tc.name: ClearMainSessions001
 * @tc.desc: ClearMainSessions001
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ClearMainSessions001, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<int32_t> persistentIds;

    EXPECT_CALL(m->Mock(), ClearMainSessions(_)).Times(0).WillOnce(Return(WMError::WM_OK));
    
    auto errorCode = WindowManagerLite::GetInstance().ClearMainSessions(persistentIds);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: ClearMainSessions002
 * @tc.desc: ClearMainSessions002
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ClearMainSessions002, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<int32_t> persistentIds = { 1, 2 };

    EXPECT_CALL(m->Mock(), ClearMainSessions(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    
    auto errorCode = WindowManagerLite::GetInstance().ClearMainSessions(persistentIds);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: ClearMainSessions003
 * @tc.desc: ClearMainSessions003
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ClearMainSessions003, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<int32_t> persistentIds;
    std::vector<int32_t> clearFailedIds;
    EXPECT_CALL(m->Mock(), ClearMainSessions(_, _)).Times(0).WillOnce(Return(WMError::WM_OK));
    
    auto errorCode = WindowManagerLite::GetInstance().ClearMainSessions(persistentIds, clearFailedIds);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: ClearMainSessions004
 * @tc.desc: ClearMainSessions004
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ClearMainSessions004, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<int32_t> persistentIds = { 1, 2 };
    std::vector<int32_t> clearFailedIds;
    EXPECT_CALL(m->Mock(), ClearMainSessions(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    
    auto errorCode = WindowManagerLite::GetInstance().ClearMainSessions(persistentIds, clearFailedIds);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: GetWindowModeType
 * @tc.desc: GetWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowModeType, Function | SmallTest | Level2)
{
    WindowModeType windowModeType = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    auto ret = WindowManagerLite::GetInstance().GetWindowModeType(windowModeType);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: RaiseWindowToTop
 * @tc.desc: RaiseWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RaiseWindowToTop, Function | SmallTest | Level2)
{
    auto ret = WindowManagerLite::GetInstance().RaiseWindowToTop(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: NotifyWMSConnected01
 * @tc.desc: NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected01, Function | SmallTest | Level2)
{
    WindowManagerLite::GetInstance().pImpl_->wmsConnectionChangedListener_ = nullptr;
    WindowManagerLite::GetInstance().pImpl_->NotifyWMSConnected(0, 0);
    WindowManagerLite::GetInstance().pImpl_->NotifyWMSDisconnected(0, 0);
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    WindowManagerLite::GetInstance().UpdateFocusChangeInfo(focusChangeInfo, true);
    focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    ASSERT_NE(nullptr, focusChangeInfo);
    WindowManagerLite::GetInstance().UpdateFocusChangeInfo(focusChangeInfo, true);
    WindowManagerLite::GetInstance().UpdateFocusChangeInfo(focusChangeInfo, false);
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    WindowManagerLite::GetInstance().UpdateWindowVisibilityInfo(windowVisibilityInfos);
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos;
    WindowManagerLite::GetInstance().UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
    WindowManagerLite::GetInstance().OnRemoteDied();
    WindowManagerLite::GetInstance().OnWMSConnectionChanged(0, 0, true);
    WindowManagerLite::GetInstance().OnWMSConnectionChanged(0, 0, false);
    WindowModeType windowModeType = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    WindowManagerLite::GetInstance().UpdateWindowModeTypeInfo(windowModeType);
    auto ret = WindowManagerLite::GetInstance().GetWindowModeType(windowModeType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: NotifyWMSConnected02
 * @tc.desc: NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected02, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<TestWMSConnectionChangedListener> listener = new (std::nothrow) TestWMSConnectionChangedListener();
    ASSERT_TRUE(listener != nullptr);
    windowManager.pImpl_->wmsConnectionChangedListener_ = nullptr;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWMSConnectionChangedListener(listener));
    int32_t userId = 0;
    int32_t screenId = 0;
    WindowManagerLite::GetInstance().pImpl_->NotifyWMSConnected(userId, screenId);
    WindowManagerLite::GetInstance().pImpl_->NotifyWMSDisconnected(userId, screenId);
}

/**
 * @tc.name: NotifyWMSConnected03
 * @tc.desc: NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected03, Function | SmallTest | Level2)
{
    WindowManagerLite::GetInstance().pImpl_->wmsConnectionChangedListener_ = new IWMSConnectionChangedListenerSon();
    WindowManagerLite::GetInstance().pImpl_->NotifyWMSConnected(0, 0);
    EXPECT_NE(WindowManagerLite::GetInstance().pImpl_->wmsConnectionChangedListener_, nullptr);
    WindowManagerLite::GetInstance().pImpl_->NotifyWMSDisconnected(0, 0);
    EXPECT_NE(WindowManagerLite::GetInstance().pImpl_->wmsConnectionChangedListener_, nullptr);
}

/**
 * @tc.name: RegisterWindowStyleChangedListener
 * @tc.desc: check RegisterWindowStyleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowStyleChangedListener, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowStyleListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowStyleListeners_;
    windowManager.pImpl_->windowStyleListenerAgent_ = nullptr;
    windowManager.pImpl_->windowStyleListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWindowStyleChangedListener(nullptr));

    sptr<IWindowStyleChangedListener> listener = new TestWindowStyleChangedListener();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWindowStyleChangedListener(listener));
    ASSERT_EQ(nullptr, windowManager.pImpl_->windowStyleListenerAgent_);

    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowStyleChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->windowStyleListeners_.size());

    // to check that the same listner can not be registered twice
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowStyleChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->windowStyleListeners_.size());

    windowManager.pImpl_->windowStyleListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->windowStyleListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowStyleChangedListener
 * @tc.desc: check UnregisterWindowStyleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowStyleChangedListener, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowStyleListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowStyleListeners_;
    windowManager.pImpl_->windowStyleListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
    windowManager.pImpl_->windowStyleListeners_.clear();
    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.UnregisterWindowStyleChangedListener(nullptr));

    sptr<TestWindowStyleChangedListener> listener1 = new TestWindowStyleChangedListener();
    sptr<TestWindowStyleChangedListener> listener2 = new TestWindowStyleChangedListener();
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowStyleChangedListener(listener1));

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterWindowStyleChangedListener(listener1);
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterWindowStyleChangedListener(listener2);
    ASSERT_EQ(2, windowManager.pImpl_->windowStyleListeners_.size());

    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowStyleChangedListener(listener1));
    EXPECT_CALL(m->Mock(), UnregisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowStyleChangedListener(listener2));
    ASSERT_EQ(0, windowManager.pImpl_->windowStyleListeners_.size());
    ASSERT_EQ(nullptr, windowManager.pImpl_->windowStyleListenerAgent_);

    windowManager.pImpl_->windowStyleListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowStyleChangedListener(listener1));
    ASSERT_EQ(0, windowManager.pImpl_->windowStyleListeners_.size());
    windowManager.pImpl_->windowStyleListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->windowStyleListeners_ = oldListeners;
}

/**
 * @tc.name: NotifyWindowStyleChange01
 * @tc.desc: check NotifyWindowStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowStyleChange01, Function | SmallTest | Level2)
{
    WindowStyleType type = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    auto ret = WindowManagerLite::GetInstance().NotifyWindowStyleChange(type);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyWindowStyleChange02
 * @tc.desc: check NotifyWindowStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowStyleChange02, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    sptr<IWindowStyleChangedListener> listener = new (std::nothrow) TestWindowStyleChangedListener();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowStyleChangedListener(listener));

    WindowStyleType type = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    auto ret = WindowManagerLite::GetInstance().NotifyWindowStyleChange(type);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: check GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowStyleType, Function | SmallTest | Level2)
{
    WindowStyleType type = WindowManagerLite::GetInstance().GetWindowStyleType();
    ASSERT_EQ(Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT, type);
}

/**
 * @tc.name: TerminateSessionByPersistentId001
 * @tc.desc: TerminateSessionByPersistentId001
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, TerminateSessionByPersistentId001, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    int32_t persistentId = 1;
    EXPECT_CALL(m->Mock(), TerminateSessionByPersistentId(_)).Times(1).WillOnce(Return(WMError::WM_OK));

    auto errorCode = WindowManagerLite::GetInstance().TerminateSessionByPersistentId(persistentId);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: TerminateSessionByPersistentId002
 * @tc.desc: TerminateSessionByPersistentId002
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, TerminateSessionByPersistentId002, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    int32_t persistentId = 0;
    auto errorCode = WindowManagerLite::GetInstance().TerminateSessionByPersistentId(persistentId);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, errorCode);
}

/**
 * @tc.name: OnRemoteDied01
 * @tc.desc: OnRemoteDied01
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, OnRemoteDied01, Function | SmallTest | Level2)
{
    WindowManagerLite::GetInstance().destroyed_ = true;
    WindowManagerLite::GetInstance().OnRemoteDied();
    ASSERT_EQ(WindowManagerLite::GetInstance().destroyed_, true);
}

/**
 * @tc.name: CloseTargetFloatWindow
 * @tc.desc: check CloseTargetFloatWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, CloseTargetFloatWindow, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::string bundleName = "test";
    EXPECT_CALL(m->Mock(), CloseTargetFloatWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));

    auto errorCode = WindowManagerLite::GetInstance().CloseTargetFloatWindow(bundleName);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: RegisterPiPStateChangedListener
 * @tc.desc: check RegisterPiPStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterPiPStateChangedListener, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->pipStateChangedListenerAgent_;
    auto oldListeners = windowManager.pImpl_->pipStateChangedListeners_;
    windowManager.pImpl_->pipStateChangedListenerAgent_ = nullptr;
    windowManager.pImpl_->pipStateChangedListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterPiPStateChangedListener(nullptr));

    sptr<IPiPStateChangedListener> listener = new TestPiPStateChangedListener();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterPiPStateChangedListener(listener));
    ASSERT_EQ(nullptr, windowManager.pImpl_->pipStateChangedListenerAgent_);

    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterPiPStateChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->pipStateChangedListeners_.size());

    // to check that the same listner can not be registered twice
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterPiPStateChangedListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->pipStateChangedListeners_.size());

    windowManager.pImpl_->pipStateChangedListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->pipStateChangedListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterPiPStateChangedListener
 * @tc.desc: check UnregisterPiPStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterPiPStateChangedListener, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->pipStateChangedListenerAgent_;
    auto oldListeners = windowManager.pImpl_->pipStateChangedListeners_;
    windowManager.pImpl_->pipStateChangedListenerAgent_ = new (std::nothrow) WindowManagerAgentLite();
    windowManager.pImpl_->pipStateChangedListeners_.clear();
    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.UnregisterPiPStateChangedListener(nullptr));

    sptr<IPiPStateChangedListener> listener1 = new TestPiPStateChangedListener();
    sptr<IPiPStateChangedListener> listener2 = new TestPiPStateChangedListener();
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterPiPStateChangedListener(listener1));

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterPiPStateChangedListener(listener1);
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterPiPStateChangedListener(listener2);
    ASSERT_EQ(2, windowManager.pImpl_->pipStateChangedListeners_.size());

    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterPiPStateChangedListener(listener1));
    EXPECT_CALL(m->Mock(), UnregisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterPiPStateChangedListener(listener2));
    ASSERT_EQ(0, windowManager.pImpl_->pipStateChangedListeners_.size());
    ASSERT_EQ(nullptr, windowManager.pImpl_->pipStateChangedListenerAgent_);

    windowManager.pImpl_->pipStateChangedListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterPiPStateChangedListener(listener1));
    ASSERT_EQ(0, windowManager.pImpl_->pipStateChangedListeners_.size());
    windowManager.pImpl_->pipStateChangedListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->pipStateChangedListeners_ = oldListeners;
}

/**
 * @tc.name: CloseTargetPiPWindow
 * @tc.desc: check CloseTargetPiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, CloseTargetPiPWindow, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::string bundleName = "test";
    EXPECT_CALL(m->Mock(), CloseTargetPiPWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));

    auto errorCode = WindowManagerLite::GetInstance().CloseTargetPiPWindow(bundleName);
    ASSERT_EQ(WMError::WM_OK, errorCode);
}

/**
 * @tc.name: GetCurrentPiPWindowInfo01
 * @tc.desc: check GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetCurrentPiPWindowInfo01, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetCurrentPiPWindowInfo(_)).Times(1).WillOnce(Return(WMError::WM_OK));

    std::string bundleName;
    auto errorCode = WindowManagerLite::GetInstance().GetCurrentPiPWindowInfo(bundleName);
    ASSERT_EQ(WMError::WM_OK, errorCode);
    ASSERT_EQ("", bundleName);
}

/**
 * @tc.name: GetCurrentPiPWindowInfo02
 * @tc.desc: check GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetCurrentPiPWindowInfo02, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::string testBundleName = "test";
    bool testState = true;
    EXPECT_CALL(m->Mock(), GetCurrentPiPWindowInfo(_)).Times(1).WillOnce(DoAll(SetArgReferee<0>(testBundleName),
        Return(WMError::WM_OK)));

    std::string bundleName;
    auto errorCode = WindowManagerLite::GetInstance().GetCurrentPiPWindowInfo(bundleName);
    ASSERT_EQ(WMError::WM_OK, errorCode);
    ASSERT_EQ(testBundleName, bundleName);
}

/**
 * @tc.name: GetAccessibilityWindowInfo01
 * @tc.desc: check GetAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAccessibilityWindowInfo01, Function | SmallTest | Level2)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    infos.clear();
    EXPECT_CALL(m->Mock(), GetAccessibilityWindowInfo(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, WindowManagerLite::GetInstance().GetAccessibilityWindowInfo(infos));
    EXPECT_CALL(m->Mock(), GetAccessibilityWindowInfo(_)).Times(1).WillOnce(Return(WMError::WM_ERROR_INVALID_WINDOW));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, WindowManagerLite::GetInstance().GetAccessibilityWindowInfo(infos));
}

/**
 * @tc.name: RegisterWindowUpdateListener01
 * @tc.desc: check RegisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowUpdateListener01, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowUpdateListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowUpdateListeners_;
    windowManager.pImpl_->windowUpdateListenerAgent_ = nullptr;
    windowManager.pImpl_->windowUpdateListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWindowUpdateListener(nullptr));

    sptr<TestWindowUpdateListener> listener = new TestWindowUpdateListener();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.RegisterWindowUpdateListener(listener));
    ASSERT_EQ(nullptr, windowManager.pImpl_->windowUpdateListenerAgent_);

    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowUpdateListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->windowUpdateListeners_.size());

    // to check that the same listner can not be registered twice
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.RegisterWindowUpdateListener(listener));
    ASSERT_EQ(1, windowManager.pImpl_->windowUpdateListeners_.size());

    windowManager.pImpl_->windowUpdateListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->windowUpdateListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowUpdateListener01
 * @tc.desc: check UnregisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowUpdateListener01, Function | SmallTest | Level2)
{
    auto& windowManager = WindowManagerLite::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowUpdateListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowUpdateListeners_;
    windowManager.pImpl_->windowUpdateListenerAgent_ = nullptr;
    windowManager.pImpl_->windowUpdateListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.UnregisterWindowUpdateListener(nullptr));

    sptr<TestWindowUpdateListener> listener1 = new TestWindowUpdateListener();
    sptr<TestWindowUpdateListener> listener2 = new TestWindowUpdateListener();
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowUpdateListener(listener1));

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterWindowUpdateListener(listener1);
    EXPECT_CALL(m->Mock(), RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    windowManager.RegisterWindowUpdateListener(listener2);
    ASSERT_EQ(2, windowManager.pImpl_->windowUpdateListeners_.size());

    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowUpdateListener(listener1));
    EXPECT_CALL(m->Mock(), UnregisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowUpdateListener(listener2));
    ASSERT_EQ(0, windowManager.pImpl_->windowUpdateListeners_.size());
    ASSERT_EQ(nullptr, windowManager.pImpl_->windowUpdateListenerAgent_);

    windowManager.pImpl_->windowUpdateListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowUpdateListener(listener1));
    ASSERT_EQ(0, windowManager.pImpl_->windowUpdateListeners_.size());

    windowManager.pImpl_->windowUpdateListenerAgent_ = oldWindowManagerAgent;
    windowManager.pImpl_->windowUpdateListeners_ = oldListeners;
}
}
}