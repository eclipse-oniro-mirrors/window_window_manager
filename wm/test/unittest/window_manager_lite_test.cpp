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
#include "common/include/window_session_property.h"
#include "mock_window_adapter_lite.h"
#include "session/host/include/scene_session.h"
#include "singleton_mocker.h"
#include "window_manager_hilog.h"
#include "window_manager_lite.cpp"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
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
    void OnWindowDrawingContentChanged(const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingInfo) override {
    };
};

class TestVisibilityChangedListener : public IVisibilityChangedListener {
public:
    void OnWindowVisibilityChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfo) override {};
};

class MockWindowInfoChangedListener : public IWindowInfoChangedListener {
public:
    int32_t count_ = 0;

    void OnWindowInfoChanged(const std::vector<std::unordered_map<WindowInfoKey,
        WindowChangeInfoType>>& windowInfoList) override
    {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "MockWindowInfoChangedListener");
        ++count_;
    };
};

class TestInterestWindowIdsListener : public IWindowInfoChangedListener {
public:
    void OnWindowInfoChanged(const WindowInfoList& windowInfoList) override
    {
        received_ = windowInfoList;
    }
    WindowInfoList received_;
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

class TestIKeyboardCallingWindowDisplayChangedListener : public IKeyboardCallingWindowDisplayChangedListener {
public:
    void OnCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
    {
        info = callingWindowInfo;
        isNotified = true;
    }

private:
    CallingWindowInfo info = {0, -1, 0, 0};
    bool isNotified = false;
};

class TestWindowVisibilityStateListener : public IWindowInfoChangedListener {
public:
    void OnWindowInfoChanged(const WindowInfoList& windowInfoList) override
    {
        WLOGI("TestWindowUpdateListener");
    };
};

class WindowManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // keyboard
    bool CheckCallingWindowInfo(const CallingWindowInfo& desiredInfo, const CallingWindowInfo& actualInfo);

private:
    int32_t userId_ = 100;
    sptr<WindowManagerLite> instance_ = nullptr;
};

void WindowManagerLiteTest::SetUpTestCase() {}

void WindowManagerLiteTest::TearDownTestCase() {}

void WindowManagerLiteTest::SetUp()
{
    instance_ = &WindowManagerLite::GetInstance(userId_);
}

void WindowManagerLiteTest::TearDown()
{
    WindowManagerLite::RemoveInstanceByUserId(userId_);
    instance_ = nullptr;
}

bool WindowManagerLiteTest::CheckCallingWindowInfo(
    const CallingWindowInfo& desiredInfo, const CallingWindowInfo& actualInfo)
{
    return desiredInfo.windowId_ == actualInfo.windowId_ && desiredInfo.callingPid_ == actualInfo.callingPid_ &&
        desiredInfo.displayId_ == actualInfo.displayId_ && desiredInfo.userId_ == actualInfo.userId_;
}

namespace {
/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: using windowManagerLite to get focus info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetFocusWindowInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    FocusChangeInfo infosInput;
    FocusChangeInfo infosResult;
    infosResult.pid_ = 10;
    infosResult.uid_ = 11;
    infosResult.displayId_ = 12;
    infosResult.windowId_ = 13;
    instance_->GetFocusWindowInfo(infosInput);
}

/**
 * @tc.name: NotifyAccessibilityWindowInfo
 * @tc.desc: NotifyAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyAccessibilityWindowInfo, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WindowManagerLite lite;
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    WindowUpdateType type = WindowUpdateType::WINDOW_UPDATE_ADDED;

    ASSERT_NE(nullptr, instance_);
    instance_->NotifyAccessibilityWindowInfo(infos, type);

    lite.pImpl_->NotifyAccessibilityWindowInfo(infos, type);
    EXPECT_TRUE(g_errLog.find("infos is empty") != std::string::npos);

    sptr<AccessibilityWindowInfo> info = sptr<AccessibilityWindowInfo>::MakeSptr();
    infos.push_back(info);
    lite.pImpl_->NotifyAccessibilityWindowInfo(infos, type);

    sptr<TestWindowUpdateListener> listener = sptr<TestWindowUpdateListener>::MakeSptr();
    instance_->RegisterWindowUpdateListener(listener);
    lite.pImpl_->NotifyAccessibilityWindowInfo(infos, type);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyMidSceneStatusChange01
 * @tc.desc: check NotifyMidSceneStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyMidSceneStatusChange01, TestSize.Level1)
{
    WindowInfoList windowInfoList;
    windowInfoList.push_back({{WindowInfoKey::MID_SCENE, true}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(3)}});

    auto& windowManager = WindowManagerLite::GetInstance();
    windowManager.pImpl_->midSceneStatusChangeListeners_.clear();
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);

    windowManager.pImpl_->midSceneStatusChangeListeners_.push_back(nullptr);
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);

    auto listener = sptr<MockWindowInfoChangedListener>::MakeSptr();

    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    EXPECT_EQ(listener->count_, 0);

    windowManager.pImpl_->midSceneStatusChangeListeners_.push_back(listener);
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);

    listener->SetInterestWindowIds({ 9 });
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);
}

/**
 * @tc.name: UpdateCameraWindowStatus01
 * @tc.desc: UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateCameraWindowStatus01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 0;
    bool isShowing = true;
    instance_->UpdateCameraWindowStatus(accessTokenId, isShowing);
    EXPECT_FALSE(g_errLog.find("Camera window, accessTokenId=%{public}u, isShowing=%{public}u") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateCameraWindowStatus02
 * @tc.desc: UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateCameraWindowStatus02, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    sptr<TestCameraWindowChangedListener> listener = sptr<TestCameraWindowChangedListener>::MakeSptr();
    instance_->RegisterCameraWindowChangedListener(listener);

    uint32_t accessTokenId = 0;
    bool isShowing = true;
    instance_->UpdateCameraWindowStatus(accessTokenId, isShowing);
}

/**
 * @tc.name: RegisterCameraWindowChangedListener01
 * @tc.desc: check RegisterCameraWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterCameraWindowChangedListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->cameraWindowChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->cameraWindowChangedListeners_;
    instance_->pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
    instance_->pImpl_->cameraWindowChangedListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterCameraWindowChangedListener(nullptr));

    sptr<TestCameraWindowChangedListener> listener1 = sptr<TestCameraWindowChangedListener>::MakeSptr();
    sptr<TestCameraWindowChangedListener> listener2 = sptr<TestCameraWindowChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener1));

    instance_->RegisterCameraWindowChangedListener(listener1);
    instance_->RegisterCameraWindowChangedListener(listener2);

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->cameraWindowChangedListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->cameraWindowChangedListenerAgent_);

    instance_->pImpl_->cameraWindowChangedListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->cameraWindowChangedListeners_.size());

    instance_->pImpl_->cameraWindowChangedListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->cameraWindowChangedListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterCameraWindowChangedListener01
 * @tc.desc: check UnregisterCameraWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterCameraWindowChangedListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->cameraWindowChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->cameraWindowChangedListeners_;
    instance_->pImpl_->cameraWindowChangedListenerAgent_ = nullptr;
    instance_->pImpl_->cameraWindowChangedListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterCameraWindowChangedListener(nullptr));

    sptr<TestCameraWindowChangedListener> listener1 = sptr<TestCameraWindowChangedListener>::MakeSptr();
    sptr<TestCameraWindowChangedListener> listener2 = sptr<TestCameraWindowChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener1));

    instance_->RegisterCameraWindowChangedListener(listener1);
    instance_->RegisterCameraWindowChangedListener(listener2);

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->cameraWindowChangedListeners_.size());

    instance_->pImpl_->cameraWindowChangedListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraWindowChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->cameraWindowChangedListeners_.size());

    instance_->pImpl_->cameraWindowChangedListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->cameraWindowChangedListeners_ = oldListeners;
}

/**
 * @tc.name: Test01
 * @tc.desc: check UnregisterCameraWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, Test01, TestSize.Level1)
{
    sptr<IFocusChangedListener> listener = nullptr;
    ASSERT_NE(nullptr, instance_);
    auto ret1 = instance_->RegisterFocusChangedListener(listener);
    auto ret2 = instance_->UnregisterFocusChangedListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret2);
    sptr<IVisibilityChangedListener> listener1 = nullptr;
    auto ret3 = instance_->RegisterVisibilityChangedListener(listener1);
    auto ret4 = instance_->UnregisterVisibilityChangedListener(listener1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret3);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret4);
    std::vector<sptr<WindowVisibilityInfo>> infos;
    ASSERT_EQ(WMError::WM_OK, instance_->GetVisibilityWindowInfo(infos));
    sptr<IDrawingContentChangedListener> listener2 = nullptr;
    auto ret5 = instance_->RegisterDrawingContentChangedListener(listener2);
    auto ret6 = instance_->UnregisterDrawingContentChangedListener(listener2);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret5);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret6);
    sptr<IWindowModeChangedListener> listener3 = nullptr;
    auto ret7 = instance_->RegisterWindowModeChangedListener(listener3);
    auto ret8 = instance_->UnregisterWindowModeChangedListener(listener3);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret7);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret8);
    sptr<IWMSConnectionChangedListener> listener4 = nullptr;
    WMError res9 = instance_->RegisterWMSConnectionChangedListener(listener4);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res9);
    WMError res10 = instance_->UnregisterWMSConnectionChangedListener();
    ASSERT_EQ(WMError::WM_OK, res10);
    sptr<IAllGroupInfoChangedListener> listener5 = nullptr;
    WMError res11 = instance_->RegisterAllGroupInfoChangedListener(listener5);
    WMError res12 = instance_->UnregisterAllGroupInfoChangedListener(listener5);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res11);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res12);
}

/**
 * @tc.name: Test04
 * @tc.desc: Test04
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, Test04, TestSize.Level1)
{
    WindowManagerLite lite;
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    ASSERT_NE(lite.pImpl_, nullptr);
    lite.pImpl_->NotifyFocused(focusChangeInfo);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
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
HWTEST_F(WindowManagerLiteTest, RegisterWindowModeChangedListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowModeListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowModeListeners_;
    instance_->pImpl_->windowModeListenerAgent_ = nullptr;
    instance_->pImpl_->windowModeListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWindowModeChangedListener(nullptr));

    sptr<TestWindowModeChangedListener> listener = sptr<TestWindowModeChangedListener>::MakeSptr();
    instance_->RegisterWindowModeChangedListener(listener);
    ASSERT_NE(nullptr, instance_);

    instance_->pImpl_->windowModeListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowModeListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowModeChangedListener02
 * @tc.desc: check UnregisterWindowModeChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowModeChangedListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowModeListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowModeListeners_;
    instance_->pImpl_->windowModeListenerAgent_ = nullptr;
    instance_->pImpl_->windowModeListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowModeChangedListener(nullptr));

    sptr<TestWindowModeChangedListener> listener1 = sptr<TestWindowModeChangedListener>::MakeSptr();
    sptr<TestWindowModeChangedListener> listener2 = sptr<TestWindowModeChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener1));

    instance_->RegisterWindowModeChangedListener(listener1);
    instance_->RegisterWindowModeChangedListener(listener2);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->windowModeListeners_.size());

    instance_->pImpl_->windowModeListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->windowModeListeners_.size());

    instance_->pImpl_->windowModeListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowModeListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener02
 * @tc.desc: check RegisterWMSConnectionChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWMSConnectionChangedListener02, TestSize.Level1)
{
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWMSConnectionChangedListener(nullptr));

    sptr<TestWMSConnectionChangedListener> listener = sptr<TestWMSConnectionChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterWMSConnectionChangedListener(listener));

    instance_->pImpl_->wmsConnectionChangedListener_ = nullptr;
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterWMSConnectionChangedListener(listener));
}

/**
 * @tc.name: GetMainWindowInfos
 * @tc.desc: get top main window info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetMainWindowInfos, TestSize.Level1)
{
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

    ASSERT_NE(nullptr, instance_);
    instance_->GetMainWindowInfos(topN, topNInfo);

    auto it1 = topNInfo.begin();
    auto it2 = topNInfoResult.begin();
    for (; it1 != topNInfo.end() && it2 != topNInfoResult.end(); it1++, it2++) {
        ASSERT_EQ(it1->pid_, it2->pid_);
        ASSERT_EQ(it1->bundleName_, it2->bundleName_);
    }
}

/**
 * @tc.name: UpdateAnimationSpeedWithPid
 * @tc.desc: Check UpdateAnimationSpeedWithPid
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateAnimationSpeedWithPid, TestSize.Level1)
{
    pid_t pid = 15234;
    float speed = 2.0f;
    WMError ret_1 = instance_->UpdateAnimationSpeedWithPid(pid, speed);
    ASSERT_EQ(WMError::WM_OK, ret_1);
    speed = 1.0f;
    WMError ret_2 = instance_->UpdateAnimationSpeedWithPid(pid, speed);
    ASSERT_EQ(WMError::WM_OK, ret_2);
}

/**
 * @tc.name: TestUpdateFocusChangeInfo
 * @tc.desc: TestUpdateFocusChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, TestUpdateFocusChangeInfo, TestSize.Level1)
{
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    instance_->UpdateFocusChangeInfo(focusChangeInfo, true);
    sptr<FocusChangeInfo> focusChangeInfo1 = sptr<FocusChangeInfo>::MakeSptr();
    instance_->UpdateFocusChangeInfo(focusChangeInfo1, true);
    instance_->UpdateFocusChangeInfo(focusChangeInfo1, false);
    std::vector<sptr<WindowVisibilityInfo>> infos;
    instance_->UpdateWindowVisibilityInfo(infos);
    std::vector<sptr<WindowDrawingContentInfo>> infos1;
    instance_->UpdateWindowDrawingContentInfo(infos1);
    WindowManagerLite lite;
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
HWTEST_F(WindowManagerLiteTest, NotifyFocusedWithUn, TestSize.Level1)
{
    WindowManagerLite lite;
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    lite.pImpl_->NotifyFocused(focusChangeInfo);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    ASSERT_EQ(focusChangeInfo, nullptr);
    focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    lite.pImpl_->focusChangedListeners_.push_back(nullptr);
    sptr<TestFocusChangedListener> testFocusChangedListener = sptr<TestFocusChangedListener>::MakeSptr();
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
HWTEST_F(WindowManagerLiteTest, NotifyWindowDrawingContentInfoChanged02, TestSize.Level1)
{
    WindowManagerLite lite;
    auto listener = sptr<TestDrawingContentChangedListener>::MakeSptr();

    ASSERT_NE(nullptr, instance_);
    instance_->RegisterDrawingContentChangedListener(listener);

    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos;
    lite.pImpl_->NotifyWindowDrawingContentInfoChanged(windowDrawingContentInfos);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterDrawingContentChangedListener(listener));
}

/**
 * @tc.name: NotifyWindowModeChange02
 * @tc.desc: NotifyWindowModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowModeChange02, TestSize.Level1)
{
    WindowManagerLite lite;
    auto listener = sptr<TestWindowModeChangedListener>::MakeSptr();

    ASSERT_NE(nullptr, instance_);
    instance_->RegisterWindowModeChangedListener(listener);
    lite.pImpl_->NotifyWindowModeChange(WindowModeType::WINDOW_MODE_SPLIT);
}

/**
 * @tc.name: NotifyWindowVisibilityInfoChanged02
 * @tc.desc: NotifyWindowVisibilityInfoChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowVisibilityInfoChanged02, TestSize.Level1)
{
    auto listener = sptr<TestVisibilityChangedListener>::MakeSptr();
    ASSERT_NE(nullptr, instance_);
    instance_->RegisterVisibilityChangedListener(listener);

    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    instance_->pImpl_->NotifyWindowVisibilityInfoChanged(windowVisibilityInfos);
    instance_->RegisterVisibilityChangedListener(listener);
}

/**
 * @tc.name: NotifyWindowVisibilityStateChanged
 * @tc.desc: NotifyWindowVisibilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowVisibilityStateChanged, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WindowManagerLite lite;
    sptr<TestWindowVisibilityStateListener> listener;
    lite.pImpl_->windowVisibilityStateListeners_.push_back(listener);
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    sptr<WindowVisibilityInfo> info = sptr<WindowVisibilityInfo>::MakeSptr();
    windowVisibilityInfos.push_back(info);
    lite.pImpl_->NotifyWindowVisibilityStateChanged(windowVisibilityInfos);
    EXPECT_TRUE(g_errLog.find("listener is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyUnfocused01
 * @tc.desc: NotifyUnfocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyUnfocused01, TestSize.Level1)
{
    WindowManagerLite lite;
    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();

    auto listener = sptr<TestFocusChangedListener>::MakeSptr();
    ASSERT_TRUE(listener != nullptr);
    instance_->RegisterFocusChangedListener(listener);
    lite.pImpl_->NotifyUnfocused(focusChangeInfo);
    instance_->RegisterFocusChangedListener(listener);
    ASSERT_NE(nullptr, instance_);
}

/**
 * @tc.name: GetAllMainWindowInfos001
 * @tc.desc: GetAllMainWindowInfos001
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAllMainWindowInfos001, TestSize.Level1)
{
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

    ASSERT_NE(nullptr, instance_);
    instance_->GetAllMainWindowInfos(infos);
}

/**
 * @tc.name: GetAllMainWindowInfos002
 * @tc.desc: GetAllMainWindowInfos002
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAllMainWindowInfos002, TestSize.Level1)
{
    std::vector<MainWindowInfo> infos;
    MainWindowInfo info1;
    info1.pid_ = 1900;
    info1.bundleName_ = "calendar";
    infos.push_back(info1);

    ASSERT_NE(nullptr, instance_);
    instance_->GetAllMainWindowInfos(infos);
}

/**
 * @tc.name: ClearMainSessions001
 * @tc.desc: ClearMainSessions001
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ClearMainSessions001, TestSize.Level1)
{
    std::vector<int32_t> persistentIds;
    ASSERT_NE(nullptr, instance_);
    instance_->ClearMainSessions(persistentIds);

    // branch 2
    persistentIds.push_back(1);
    instance_->ClearMainSessions(persistentIds);
}


/**
 * @tc.name: ClearMainSessions002
 * @tc.desc: ClearMainSessions002
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ClearMainSessions003, TestSize.Level1)
{
    std::vector<int32_t> persistentIds;
    std::vector<int32_t> clearFailedIds;

    ASSERT_NE(nullptr, instance_);
    instance_->ClearMainSessions(persistentIds, clearFailedIds);

    // branch 2
    persistentIds.push_back(1);
    ASSERT_NE(nullptr, instance_);
    instance_->ClearMainSessions(persistentIds, clearFailedIds);
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: check UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    std::string bundleName = "UpdateScreenLockStatusForApp";
    bool isRelease = true;
    WMError expectRet = WindowAdapterLite::GetInstance(userId_).UpdateScreenLockStatusForApp(bundleName, isRelease);
    auto ret = instance_->UpdateScreenLockStatusForApp(bundleName, isRelease);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: NotifyWindowPropertyChange01
 * @tc.desc: check NotifyWindowPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowPropertyChange01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    uint32_t flags = static_cast<int32_t>(WindowInfoKey::MID_SCENE);
    WindowInfoList windowInfoList;
    windowInfoList.push_back({{WindowInfoKey::MID_SCENE, true}, {WindowInfoKey::WINDOW_ID, 0}});

    auto oldInfoKeyMap = instance_->interestInfoMap_;
    auto oldListeners = instance_->pImpl_->midSceneStatusChangeListeners_;
    instance_->interestInfoMap_.clear();
    instance_->pImpl_->midSceneStatusChangeListeners_.clear();

    instance_->interestInfoMap_[WindowInfoKey::MID_SCENE] = 0;
    instance_->interestInfoMap_[WindowInfoKey::VISIBILITY_STATE] = 3;
    sptr<IWindowInfoChangedListener> listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    std::unordered_set<WindowInfoKey> interestInfo;
    interestInfo.insert(WindowInfoKey::MID_SCENE);
    interestInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    interestInfo.insert(WindowInfoKey::BUNDLE_NAME);
    listener->SetInterestInfo(interestInfo);
    std::unordered_set<int32_t> interestWindowIds;
    interestWindowIds.insert({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

    auto ret = instance_->RegisterMidSceneChangedListener(nullptr);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    ret = instance_->RegisterMidSceneChangedListener(listener);
    EXPECT_NE(ret, WMError::WM_OK);
    ret = instance_->RegisterMidSceneChangedListener(listener);
    EXPECT_NE(ret, WMError::WM_OK);

    instance_->NotifyWindowPropertyChange(flags, windowInfoList);

    ret = instance_->UnregisterMidSceneChangedListener(nullptr);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    ret = instance_->UnregisterMidSceneChangedListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);

    instance_->pImpl_->midSceneStatusChangeListeners_.emplace_back(nullptr);
    sptr<IWindowInfoChangedListener> listener2 = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    ret = instance_->RegisterMidSceneChangedListener(listener2);
    EXPECT_NE(ret, WMError::WM_OK);
    instance_->NotifyWindowPropertyChange(flags, windowInfoList);

    instance_->pImpl_->midSceneStatusChangeListeners_ = oldListeners;
    instance_->interestInfoMap_ = oldInfoKeyMap;
}

/**
 * @tc.name: GetWindowModeType
 * @tc.desc: GetWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowModeType, TestSize.Level1)
{
    WindowModeType windowModeType = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    auto ret = instance_->GetWindowModeType(windowModeType);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: RaiseWindowToTop
 * @tc.desc: RaiseWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RaiseWindowToTop, TestSize.Level1)
{
    auto ret = instance_->RaiseWindowToTop(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: NotifyWMSConnected01
 * @tc.desc: NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected01, TestSize.Level1)
{
    instance_->pImpl_->wmsConnectionChangedListener_ = nullptr;
    instance_->pImpl_->NotifyWMSConnected(0, 0);
    instance_->pImpl_->NotifyWMSDisconnected(0, 0);
    sptr<FocusChangeInfo> focusChangeInfo = nullptr;
    instance_->UpdateFocusChangeInfo(focusChangeInfo, true);
    focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    ASSERT_NE(nullptr, focusChangeInfo);
    instance_->UpdateFocusChangeInfo(focusChangeInfo, true);
    instance_->UpdateFocusChangeInfo(focusChangeInfo, false);
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    instance_->UpdateWindowVisibilityInfo(windowVisibilityInfos);
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos;
    instance_->UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
    instance_->OnRemoteDied();
    instance_->OnWMSConnectionChanged(0, 0, true);
    instance_->OnWMSConnectionChanged(0, 0, false);
    WindowModeType windowModeType = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    instance_->UpdateWindowModeTypeInfo(windowModeType);
    auto ret = instance_->GetWindowModeType(windowModeType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: NotifyWMSConnected02
 * @tc.desc: NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected02, TestSize.Level1)
{
    sptr<TestWMSConnectionChangedListener> listener = sptr<TestWMSConnectionChangedListener>::MakeSptr();
    instance_->pImpl_->wmsConnectionChangedListener_ = nullptr;
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterWMSConnectionChangedListener(listener));
    int32_t userId = 0;
    int32_t screenId = 0;
    instance_->pImpl_->NotifyWMSConnected(userId, screenId);
    instance_->pImpl_->NotifyWMSDisconnected(userId, screenId);
}

/**
 * @tc.name: NotifyWMSConnected03
 * @tc.desc: NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected03, TestSize.Level1)
{
    instance_->pImpl_->wmsConnectionChangedListener_ = sptr<IWMSConnectionChangedListenerSon>::MakeSptr();
    instance_->pImpl_->NotifyWMSConnected(0, 0);
    EXPECT_NE(instance_->pImpl_->wmsConnectionChangedListener_, nullptr);
    instance_->pImpl_->NotifyWMSDisconnected(0, 0);
    EXPECT_NE(instance_->pImpl_->wmsConnectionChangedListener_, nullptr);
}

/**
 * @tc.name: RegisterWindowStyleChangedListener
 * @tc.desc: check RegisterWindowStyleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowStyleChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowStyleListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowStyleListeners_;
    instance_->pImpl_->windowStyleListenerAgent_ = nullptr;
    instance_->pImpl_->windowStyleListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWindowStyleChangedListener(nullptr));

    sptr<IWindowStyleChangedListener> listener = sptr<TestWindowStyleChangedListener>::MakeSptr();
    instance_->RegisterWindowStyleChangedListener(listener);

    // to check that the same listner can not be registered twice
    instance_->RegisterWindowStyleChangedListener(listener);

    instance_->pImpl_->windowStyleListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowStyleListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowStyleChangedListener
 * @tc.desc: check UnregisterWindowStyleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowStyleChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowStyleListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowStyleListeners_;
    instance_->pImpl_->windowStyleListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr();
    instance_->pImpl_->windowStyleListeners_.clear();
    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowStyleChangedListener(nullptr));

    sptr<TestWindowStyleChangedListener> listener1 = sptr<TestWindowStyleChangedListener>::MakeSptr();
    sptr<TestWindowStyleChangedListener> listener2 = sptr<TestWindowStyleChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowStyleChangedListener(listener1));

    instance_->RegisterWindowStyleChangedListener(listener1);
    instance_->RegisterWindowStyleChangedListener(listener2);

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowStyleChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowStyleChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->windowStyleListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->windowStyleListenerAgent_);

    instance_->pImpl_->windowStyleListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowStyleChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->windowStyleListeners_.size());
    instance_->pImpl_->windowStyleListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowStyleListeners_ = oldListeners;
}

/**
 * @tc.name: NotifyWindowStyleChange01
 * @tc.desc: check NotifyWindowStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowStyleChange01, TestSize.Level1)
{
    WindowStyleType type = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    auto ret = instance_->NotifyWindowStyleChange(type);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyWindowStyleChange02
 * @tc.desc: check NotifyWindowStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWindowStyleChange02, TestSize.Level1)
{
    sptr<IWindowStyleChangedListener> listener = sptr<TestWindowStyleChangedListener>::MakeSptr();
    ASSERT_NE(nullptr, instance_);
    instance_->RegisterWindowStyleChangedListener(listener);

    WindowStyleType type = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    auto ret = instance_->NotifyWindowStyleChange(type);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: check GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowStyleType, TestSize.Level1)
{
    WindowStyleType type = instance_->GetWindowStyleType();
    ASSERT_EQ(Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT, type);
}

/**
 * @tc.name: TerminateSessionByPersistentId001
 * @tc.desc: TerminateSessionByPersistentId001
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, TerminateSessionByPersistentId001, TestSize.Level1)
{
    int32_t persistentId = 1;
    ASSERT_NE(nullptr, instance_);
    instance_->TerminateSessionByPersistentId(persistentId);
}

/**
 * @tc.name: TerminateSessionByPersistentId002
 * @tc.desc: TerminateSessionByPersistentId002
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, TerminateSessionByPersistentId002, TestSize.Level1)
{
    int32_t persistentId = 0;
    auto errorCode = instance_->TerminateSessionByPersistentId(persistentId);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, errorCode);
}

/**
 * @tc.name: CloseTargetFloatWindow
 * @tc.desc: check CloseTargetFloatWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, CloseTargetFloatWindow, TestSize.Level1)
{
    std::string bundleName = "test";
    ASSERT_NE(nullptr, instance_);
    instance_->CloseTargetFloatWindow(bundleName);

    std::string bundleName1 = "";
    ASSERT_NE(nullptr, instance_);
    instance_->CloseTargetFloatWindow(bundleName1);
}

/**
 * @tc.name: RegisterPiPStateChangedListener
 * @tc.desc: check RegisterPiPStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterPiPStateChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->pipStateChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->pipStateChangedListeners_;
    instance_->pImpl_->pipStateChangedListenerAgent_ = nullptr;
    instance_->pImpl_->pipStateChangedListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterPiPStateChangedListener(nullptr));

    sptr<IPiPStateChangedListener> listener = sptr<TestPiPStateChangedListener>::MakeSptr();
    instance_->RegisterPiPStateChangedListener(listener);
    ASSERT_NE(nullptr, instance_);

    instance_->RegisterPiPStateChangedListener(listener);
    ASSERT_NE(nullptr, instance_);

    instance_->pImpl_->pipStateChangedListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->pipStateChangedListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterPiPStateChangedListener
 * @tc.desc: check UnregisterPiPStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterPiPStateChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->pipStateChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->pipStateChangedListeners_;
    instance_->pImpl_->pipStateChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr();
    instance_->pImpl_->pipStateChangedListeners_.clear();
    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterPiPStateChangedListener(nullptr));

    sptr<IPiPStateChangedListener> listener1 = sptr<TestPiPStateChangedListener>::MakeSptr();
    sptr<IPiPStateChangedListener> listener2 = sptr<TestPiPStateChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterPiPStateChangedListener(listener1));

    instance_->RegisterPiPStateChangedListener(listener1);
    instance_->RegisterPiPStateChangedListener(listener2);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterPiPStateChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterPiPStateChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->pipStateChangedListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->pipStateChangedListenerAgent_);

    instance_->pImpl_->pipStateChangedListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterPiPStateChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->pipStateChangedListeners_.size());
    instance_->pImpl_->pipStateChangedListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->pipStateChangedListeners_ = oldListeners;
}

/**
 * @tc.name: CloseTargetPiPWindow
 * @tc.desc: check CloseTargetPiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, CloseTargetPiPWindow, TestSize.Level1)
{
    std::string bundleName = "test";
    ASSERT_NE(nullptr, instance_);
    instance_->CloseTargetPiPWindow(bundleName);

    std::string bundleName1 = "";
    auto ret = instance_->CloseTargetPiPWindow(bundleName1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: GetCurrentPiPWindowInfo01
 * @tc.desc: check GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetCurrentPiPWindowInfo01, TestSize.Level1)
{
    std::string bundleName;
    ASSERT_NE(nullptr, instance_);
    instance_->GetCurrentPiPWindowInfo(bundleName);
    ASSERT_EQ("", bundleName);
}

/**
 * @tc.name: GetCurrentPiPWindowInfo02
 * @tc.desc: check GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetCurrentPiPWindowInfo02, TestSize.Level1)
{
    std::string testBundleName = "test";
    std::string bundleName;
    ASSERT_NE(nullptr, instance_);
    instance_->GetCurrentPiPWindowInfo(bundleName);
}

/**
 * @tc.name: GetAccessibilityWindowInfo01
 * @tc.desc: check GetAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAccessibilityWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    infos.clear();
    ASSERT_NE(nullptr, instance_);
    instance_->GetAccessibilityWindowInfo(infos);
}

/**
 * @tc.name: RegisterWindowUpdateListener01
 * @tc.desc: check RegisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowUpdateListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowUpdateListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowUpdateListeners_;
    instance_->pImpl_->windowUpdateListenerAgent_ = nullptr;
    instance_->pImpl_->windowUpdateListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWindowUpdateListener(nullptr));

    sptr<TestWindowUpdateListener> listener = sptr<TestWindowUpdateListener>::MakeSptr();
    instance_->RegisterWindowUpdateListener(listener);
    ASSERT_NE(nullptr, instance_);

    instance_->pImpl_->windowUpdateListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowUpdateListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowUpdateListener01
 * @tc.desc: check UnregisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowUpdateListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowUpdateListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowUpdateListeners_;
    instance_->pImpl_->windowUpdateListenerAgent_ = nullptr;
    instance_->pImpl_->windowUpdateListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowUpdateListener(nullptr));

    sptr<TestWindowUpdateListener> listener1 = sptr<TestWindowUpdateListener>::MakeSptr();
    sptr<TestWindowUpdateListener> listener2 = sptr<TestWindowUpdateListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener1));

    instance_->RegisterWindowUpdateListener(listener1);
    instance_->RegisterWindowUpdateListener(listener2);
    ASSERT_EQ(2, instance_->pImpl_->windowUpdateListeners_.size());

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());

    instance_->pImpl_->windowUpdateListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());

    instance_->pImpl_->windowUpdateListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowUpdateListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterCallingWindowDisplayChangedListener1
 * @tc.desc: check RegisterCallingWindowDisplayChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterCallingWindowDisplayChangedListener1, Function | SmallTest | Level2)
{
    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();
    instance_->pImpl_->callingDisplayChangedListeners_.clear();
    instance_->pImpl_->callingDisplayListenerAgent_ = nullptr;
    WMError ret = instance_->RegisterCallingWindowDisplayChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    ret = instance_->RegisterCallingWindowDisplayChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_NE(nullptr, instance_->pImpl_->callingDisplayListenerAgent_);
    ASSERT_EQ(1, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));

    // Register the same listener repeatedly.
    ret = instance_->RegisterCallingWindowDisplayChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(1, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));
}

/**
 * @tc.name: RegisterCallingWindowDisplayChangedListener2
 * @tc.desc: check RegisterCallingWindowDisplayChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterCallingWindowDisplayChangedListener2, Function | SmallTest | Level2)
{
    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener1
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();
    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener2
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();

    instance_->pImpl_->callingDisplayChangedListeners_.clear();
    instance_->pImpl_->callingDisplayListenerAgent_ = nullptr;
    WMError ret = instance_->RegisterCallingWindowDisplayChangedListener(listener1);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_NE(nullptr, instance_->pImpl_->callingDisplayListenerAgent_);
    ASSERT_EQ(1, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));

    ret = instance_->RegisterCallingWindowDisplayChangedListener(listener2);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(2, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));

    ret = instance_->UnregisterCallingWindowDisplayChangedListener(listener1);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(1, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));
    ASSERT_NE(nullptr, instance_->pImpl_->callingDisplayListenerAgent_);

    // Unregister the same listener repeatedly.
    ret = instance_->UnregisterCallingWindowDisplayChangedListener(listener1);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(1, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));

    ret = instance_->UnregisterCallingWindowDisplayChangedListener(listener2);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(0, static_cast<uint32_t>(instance_->pImpl_->callingDisplayChangedListeners_.size()));
    ASSERT_EQ(nullptr, instance_->pImpl_->callingDisplayListenerAgent_);
}

/**
 * @tc.name: RegisterCallingWindowDisplayChangedListener3
 * @tc.desc: check RegisterCallingWindowDisplayChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterCallingWindowDisplayChangedListener3, Function | SmallTest | Level2)
{
    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();

    instance_->pImpl_->callingDisplayChangedListeners_.clear();
    instance_->pImpl_->callingDisplayListenerAgent_ = nullptr;
    WMError ret = instance_->UnregisterCallingWindowDisplayChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    ret = instance_->UnregisterCallingWindowDisplayChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyCallingWindowDisplayChanged1
 * @tc.desc: check NotifyCallingWindowDisplayChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyCallingWindowDisplayChanged1, Function | SmallTest | Level2)
{
    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();

    instance_->pImpl_->callingDisplayChangedListeners_.clear();
    instance_->pImpl_->callingDisplayListenerAgent_ = nullptr;

    const CallingWindowInfo& callingWindowInfo = {86, 57256, 12, 100};
    instance_->NotifyCallingWindowDisplayChanged(callingWindowInfo);
    ASSERT_EQ(false, listener->isNotified);

    instance_->pImpl_->callingDisplayChangedListeners_.emplace_back(nullptr);
    instance_->NotifyCallingWindowDisplayChanged(callingWindowInfo);
    ASSERT_EQ(false, listener->isNotified);
}

/**
 * @tc.name: NotifyCallingWindowDisplayChanged2
 * @tc.desc: check NotifyCallingWindowDisplayChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyCallingWindowDisplayChanged2, Function | SmallTest | Level2)
{
    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener1
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();

    sptr<TestIKeyboardCallingWindowDisplayChangedListener> listener2
        = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();

    instance_->pImpl_->callingDisplayChangedListeners_.clear();
    instance_->pImpl_->callingDisplayListenerAgent_ = nullptr;

    instance_->pImpl_->callingDisplayChangedListeners_.emplace_back(listener1);
    instance_->pImpl_->callingDisplayChangedListeners_.emplace_back(listener2);

    const CallingWindowInfo& callingWindowInfo = {86, 57256, 12, 100};
    instance_->NotifyCallingWindowDisplayChanged(callingWindowInfo);
    ASSERT_EQ(true, listener1->isNotified);
    ASSERT_EQ(true, listener2->isNotified);
    ASSERT_EQ(true, CheckCallingWindowInfo(callingWindowInfo, listener1->info));
    ASSERT_EQ(true, CheckCallingWindowInfo(callingWindowInfo, listener2->info));
}

/**
 * @tc.name: UpdatePiPWindowStateChanged
 * @tc.desc: UpdatePiPWindowStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdatePiPWindowStateChanged, TestSize.Level1)
{
    WindowManagerLite lite;
    sptr<TestPiPStateChangedListener> listener = nullptr;
    lite.pImpl_->pipStateChangedListeners_.push_back(listener);
    const std::string& bundleName = "test";

    ASSERT_NE(nullptr, instance_);
    instance_->UpdatePiPWindowStateChanged(bundleName, true);
    lite.pImpl_->UpdatePiPWindowStateChanged(bundleName, true);

    lite.pImpl_->pipStateChangedListeners_.clear();
    listener = sptr<TestPiPStateChangedListener>::MakeSptr();
    lite.pImpl_->pipStateChangedListeners_.push_back(listener);
    instance_->UpdatePiPWindowStateChanged(bundleName, true);
    lite.pImpl_->UpdatePiPWindowStateChanged(bundleName, true);

    lite.pImpl_->pipStateChangedListeners_.clear();
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterPiPStateChangedListener(listener));
}

/**
 * @tc.name: UnregisterFocusChangedListener
 * @tc.desc: UnregisterFocusChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterFocusChangedListener, TestSize.Level1)
{
    sptr<TestFocusChangedListener> listener = sptr<TestFocusChangedListener>::MakeSptr();
    ASSERT_TRUE(listener != nullptr);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterFocusChangedListener(listener));
}

/**
 * @tc.name: UnregisterVisibilityChangedListener
 * @tc.desc: UnregisterVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterVisibilityChangedListener, TestSize.Level1)
{
    sptr<TestVisibilityChangedListener> listener = sptr<TestVisibilityChangedListener>::MakeSptr();
    ASSERT_TRUE(listener != nullptr);

    instance_->RegisterVisibilityChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibilityChangedListener(listener));
}

/**
 * @tc.name: ProcessRegisterWindowInfoChangeCallback01
 * @tc.desc: Check ProcessRegisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ProcessRegisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    sptr<TestWindowVisibilityStateListener> listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    WindowInfoKey observedInfo = WindowInfoKey::VISIBILITY_STATE;
    ASSERT_NE(nullptr, instance_);
    instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener);

    auto ret = instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    observedInfo = WindowInfoKey::BUNDLE_NAME;
    ret = instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: ProcessUnregisterWindowInfoChangeCallback01
 * @tc.desc: Check ProcessUnregisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, ProcessUnregisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    sptr<TestWindowVisibilityStateListener> listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    WindowInfoKey observedInfo = WindowInfoKey::VISIBILITY_STATE;
    auto ret = instance_->ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = instance_->ProcessUnregisterWindowInfoChangeCallback(observedInfo, nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    observedInfo = WindowInfoKey::BUNDLE_NAME;
    ret = instance_->ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: RegisterWindowInfoChangeCallback01
 * @tc.desc: Check RegisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    sptr<TestWindowVisibilityStateListener> listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    auto interestInfoSizeOld = listener->GetInterestInfo().size();
    std::unordered_set<WindowInfoKey> observedInfo;
    observedInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    auto ret = instance_->RegisterWindowInfoChangeCallback(observedInfo, listener);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    ASSERT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());
    std::unordered_set<WindowInfoKey> observedInfo1;
    observedInfo1.insert(WindowInfoKey::BUNDLE_NAME);
    ret = instance_->RegisterWindowInfoChangeCallback(observedInfo1, listener);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: UnregisterWindowInfoChangeCallback01
 * @tc.desc: Check UnregisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    sptr<TestWindowVisibilityStateListener> listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    auto interestInfoSizeOld = listener->GetInterestInfo().size();
    std::unordered_set<WindowInfoKey> observedInfo;
    observedInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    auto ret = instance_->UnregisterWindowInfoChangeCallback(observedInfo, listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());
    std::unordered_set<WindowInfoKey> observedInfo1;
    observedInfo1.insert(WindowInfoKey::BUNDLE_NAME);
    ret = instance_->UnregisterWindowInfoChangeCallback(observedInfo1, listener);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: RegisterVisibilityStateChangedListener01
 * @tc.desc: check RegisterVisibilityStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterVisibilityStateChangedListener01, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowVisibilityStateListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowVisibilityStateListeners_;
    instance_->pImpl_->windowVisibilityStateListenerAgent_ = nullptr;
    instance_->pImpl_->windowVisibilityStateListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterVisibilityStateChangedListener(nullptr));

    sptr<TestWindowVisibilityStateListener> listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    instance_->RegisterVisibilityStateChangedListener(listener);

    // to check that the same listner can not be registered twice
    instance_->RegisterVisibilityStateChangedListener(listener);

    instance_->pImpl_->windowVisibilityStateListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowVisibilityStateListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterVisibilityStateChangedListener01
 * @tc.desc: check UnregisterVisibilityStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterVisibilityStateChangedListener01, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowVisibilityStateListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowVisibilityStateListeners_;
    instance_->pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr();
    instance_->pImpl_->windowVisibilityStateListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterVisibilityStateChangedListener(nullptr));

    sptr<TestWindowVisibilityStateListener> listener1 = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    sptr<TestWindowVisibilityStateListener> listener2 = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, instance_->UnregisterVisibilityStateChangedListener(listener1));

    instance_->RegisterVisibilityStateChangedListener(listener1);
    instance_->RegisterVisibilityStateChangedListener(listener2);

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibilityStateChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibilityStateChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->windowVisibilityStateListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->windowVisibilityStateListenerAgent_);

    instance_->pImpl_->windowVisibilityStateListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibilityStateChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->windowVisibilityStateListeners_.size());

    instance_->pImpl_->windowVisibilityStateListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowVisibilityStateListeners_ = oldListeners;
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: check SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetGlobalDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    auto ret = instance_->SetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: check GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetGlobalDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    auto ret = instance_->GetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: check SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetAppDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    auto ret = instance_->SetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: check GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAppDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";
    auto ret = instance_->GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: EffectiveDragResizeType
 * @tc.desc: test EffectiveDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, EffectiveDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";

    DragResizeType globalDragResizeType = DragResizeType::RESIZE_WHEN_DRAG_END;
    DragResizeType appDragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    instance_->SetGlobalDragResizeType(globalDragResizeType);
    instance_->SetAppDragResizeType(bundleName, appDragResizeType);
    instance_->GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(dragResizeType, globalDragResizeType);
    instance_->SetGlobalDragResizeType(DragResizeType::RESIZE_TYPE_UNDEFINED);
    instance_->GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(dragResizeType, appDragResizeType);
}

/**
 * @tc.name: SetAppKeyFramePolicy01
 * @tc.desc: check SetAppKeyFramePolicy enable
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetAppKeyFramePolicy01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    keyFramePolicy.animationDelay_ = 200;
    auto ret = instance_->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetAppKeyFramePolicy02
 * @tc.desc: check SetAppKeyFramePolicy disable
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetAppKeyFramePolicy02, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    auto ret = instance_->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SendPointerEventForHover
 * @tc.desc: SendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SendPointerEventForHover, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    WMError ret = instance_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    pointerEvent = MMI::PointerEvent::Create();
    ret = instance_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER;
    pointerEvent->sourceType_ = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    ret = instance_->SendPointerEventForHover(pointerEvent);
}

/**
 * @tc.name: GetInstance
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetInstanceMulti, TestSize.Level1)
{
    int32_t userId;
    sptr<WindowManagerLite> instance = nullptr;

    userId = -1;
    instance = &WindowManagerLite::GetInstance(userId);
    ASSERT_NE(nullptr, instance);

    // branch overried
    instance_ = &WindowManagerLite::GetInstance(userId_);
    ASSERT_NE(nullptr, instance);
}

/**
 * @tc.name: GetWindowInfoListByInterestWindowIds_NullListener
 * @tc.desc: GetWindowInfoListByInterestWindowIds_NullListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowInfoListByInterestWindowIds_NullListener, Function | SmallTest | Level2)
{
    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info;
    info.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1));
    windowInfoList.emplace_back(info);

    auto result = instance_->pImpl_->GetWindowInfoListByInterestWindowIds(nullptr, windowInfoList);
    EXPECT_EQ(windowInfoList, result);
}

/**
 * @tc.name: GetWindowInfoListByInterestWindowIds_EmptyInterestIds
 * @tc.desc: GetWindowInfoListByInterestWindowIds_EmptyInterestIds
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowInfoListByInterestWindowIds_EmptyInterestIds, Function | SmallTest | Level2)
{
    auto listener = sptr<TestInterestWindowIdsListener>::MakeSptr();
    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info;
    info.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1));
    windowInfoList.emplace_back(info);

    auto result = instance_->pImpl_->GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
    EXPECT_EQ(windowInfoList, result);
}

/**
 * @tc.name: GetWindowInfoListByInterestWindowIds_FilterMatch
 * @tc.desc: GetWindowInfoListByInterestWindowIds_FilterMatch
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowInfoListByInterestWindowIds_FilterMatch, Function | SmallTest | Level2)
{
    auto listener = sptr<TestInterestWindowIdsListener>::MakeSptr();
    listener->AddInterestWindowId(1);

    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info1;
    info1.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1));
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info2;
    info2.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2));
    windowInfoList.emplace_back(info1);
    windowInfoList.emplace_back(info2);

    auto result = instance_->pImpl_->GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(info1, result.front());
}


/**
 * @tc.name: GetDisplayIdByWindowId
 * @tc.desc: check GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetDisplayIdByWindowId, TestSize.Level1)
{
    const std::vector<uint64_t> windowIds = { 1, 2 };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    auto ret = instance_->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    ASSERT_EQ(WMError::WM_OK, ret);
}
}
} // namespace
} // namespace OHOS::Rosen
