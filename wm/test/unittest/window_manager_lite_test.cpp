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
#include "common/include/window_session_property.h"
#include "pointer_event.h"
#include "session/host/include/scene_session.h"
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
        g_errLog += msg;
    }

/**
 * Mock the functions in WindowAdapterLite to return ok.
 */
class MockWindowAdapterLite : public WindowAdapterLite {
public:
    explicit MockWindowAdapterLite(const int32_t userId) : WindowAdapterLite(userId) {}
    ~MockWindowAdapterLite() = default;

    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError GetWindowModeType(WindowModeType& windowModeType) override
    {
        return WMError::WM_OK;
    }

    WMError RaiseWindowToTop(int32_t persistentId) override
    {
        return WMError::WM_OK;
    }

    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) override
    {
        return WMError::WM_OK;
    }

    WMError SetGlobalDragResizeType(DragResizeType dragResizeType) override
    {
        return WMError::WM_OK;
    }

    WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType) override
    {
        return WMError::WM_OK;
    }

    WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType) override
    {
        return WMError::WM_OK;
    }

    WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy) override
    {
        return WMError::WM_OK;
    }

    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override
    {
        return WMError::WM_OK;
    }
};

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
    void OnConnected(int32_t userId, int32_t screenId) override
    {
        TLOGI(WmsLogTag::WMS_SCB, "wms on connected");
    };
    void OnDisconnected(int32_t userId, int32_t screenId) override
    {
        TLOGI(WmsLogTag::WMS_SCB, "wms disconnected");
    };
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

class TestWindowInfoChangedListener : public IWindowInfoChangedListener {
public:
    int32_t count_ = 0;

    void OnWindowInfoChanged(const std::vector<std::unordered_map<WindowInfoKey,
        WindowChangeInfoType>>& windowInfoList) override
    {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "TestWindowInfoChangedListener");
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
    void OnPiPStateChanged(const std::string& bundleName, bool isForeground) override
    {
        TLOGI(WmsLogTag::WMS_PIP, "Pip state changed");
    }
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

class TestAllGroupInfoChangedListener : public IAllGroupInfoChangedListener {
public:
    void OnDisplayGroupInfoChange(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd) override {}
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
    sptr<WindowAdapterLite> adapter_ = nullptr;

    // Mock functions in WindowAdapterLite return ok.
    int32_t mockUserId_ = 200;
    sptr<WindowManagerLite> mockInstance_ = nullptr;
    sptr<MockWindowAdapterLite> mockAdapter_ = nullptr;
};

void WindowManagerLiteTest::SetUpTestCase() {}

void WindowManagerLiteTest::TearDownTestCase() {}

void WindowManagerLiteTest::SetUp()
{
    instance_ = &WindowManagerLite::GetInstance(userId_);
    adapter_ = sptr<WindowAdapterLite>::MakeSptr(userId_);
    // Make sure the register function in windowAdapter which not mocked will return failed.
    adapter_->isProxyValid_ = true;
    adapter_->windowManagerServiceProxy_ = nullptr;
    WindowAdapterLite::windowAdapterLiteMap_[userId_] = adapter_;

    // Note: To mock WindowAdapterLite.GetInstance(userId_).xxx return ok, we can create a mocked adapter instance
    // and insert it to windowAdapterLiteMap_. Then, WindowAdapter::GetInstance(mockUserId_) will get it.
    mockAdapter_ = sptr<MockWindowAdapterLite>::MakeSptr(mockUserId_);
    WindowAdapterLite::windowAdapterLiteMap_[mockUserId_] = mockAdapter_;
    mockInstance_ = &WindowManagerLite::GetInstance(mockUserId_);
}

void WindowManagerLiteTest::TearDown()
{
    WindowManagerLite::RemoveInstanceByUserId(userId_);
    WindowManagerLite::RemoveInstanceByUserId(mockUserId_);
    WindowAdapterLite::windowAdapterLiteMap_.clear();

    instance_ = nullptr;
    mockInstance_ = nullptr;
    adapter_ = nullptr;
    mockAdapter_ = nullptr;
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

    auto listener = sptr<TestWindowInfoChangedListener>::MakeSptr();

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
    float speed = -2.0f;
    WMError ret = instance_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    speed = 1.0f;
    instance_->UpdateAnimationSpeedWithPid(pid, speed);
}

/**
 * @tc.name: UpdateFocusChangeInfo
 * @tc.desc: UpdateFocusChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateFocusChangeInfo, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<FocusChangeInfo> info = nullptr;
    bool focused = true;

    // branch 1: info is null
    instance_->UpdateFocusChangeInfo(info, focused);
    EXPECT_TRUE(g_errLog.find("focusChangeInfo is nullptr.") != std::string::npos);

    // branch 2: focused is true
    info = sptr<FocusChangeInfo>::MakeSptr();
    instance_->UpdateFocusChangeInfo(info, focused);

    // branch 3: focused is false
    focused = false;
    instance_->UpdateFocusChangeInfo(info, focused);

    LOG_SetCallback(nullptr);
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
 * @tc.name: GetWindowModeType
 * @tc.desc: GetWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetWindowModeType, TestSize.Level1)
{
    WMError ret;
    auto type = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;

    // branch 1: return failed
    ret = instance_->GetWindowModeType(type);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: use mock and return ok
    ret = mockInstance_->GetWindowModeType(type);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: check UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    int userId = 100;
    ASSERT_NE(instance_, nullptr);
    std::string bundleName = "UpdateScreenLockStatusForApp";
    bool isRelease = true;
    WMError expectRet = WindowAdapterLite::GetInstance(userId).UpdateScreenLockStatusForApp(bundleName, isRelease);
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
 * @tc.name: SetProcessWatermark
 * @tc.desc: check SetProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetProcessWatermark, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    int32_t pid = 1000;
    const std::string watermarkName = "SetProcessWatermarkName";
    bool isEnabled = true;
    WMError expectRet = WindowAdapterLite::GetInstance(userId_).SetProcessWatermark(pid, watermarkName, isEnabled);
    auto ret = instance_->SetProcessWatermark(pid, watermarkName, isEnabled);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: RaiseWindowToTop
 * @tc.desc: RaiseWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RaiseWindowToTop, TestSize.Level1)
{
    WMError ret;
    int32_t persistentId = 0;

    // branch 1: return failed
    ret = instance_->RaiseWindowToTop(persistentId);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: use mock and return ok
    ret = mockInstance_->RaiseWindowToTop(persistentId);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: Impl::NotifyWMSConnected
 * @tc.desc: check NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSConnected, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId = 0;
    int32_t screenId = 0;
    sptr<IWMSConnectionChangedListener> listener = sptr<TestWMSConnectionChangedListener>::MakeSptr();

    // branch 1: wmsConnectionChangedListener_ is null
    instance_->pImpl_->wmsConnectionChangedListener_ = nullptr;
    instance_->pImpl_->NotifyWMSConnected(userId, screenId);

    // branch 2: on connected
    instance_->pImpl_->wmsConnectionChangedListener_ = listener;
    instance_->pImpl_->NotifyWMSConnected(userId, screenId);
    EXPECT_TRUE(g_errLog.find("wms on connected") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: Impl::NotifyWMSDisconnected
 * @tc.desc: check NotifyWMSDisconnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, NotifyWMSDisconnected, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId = 0;
    int32_t screenId = 0;
    sptr<IWMSConnectionChangedListener> listener = sptr<TestWMSConnectionChangedListener>::MakeSptr();

    // branch 1: wmsConnectionChangedListener_ is null
    instance_->pImpl_->wmsConnectionChangedListener_ = nullptr;
    instance_->pImpl_->NotifyWMSDisconnected(userId, screenId);

    // branch 2: on connected
    instance_->pImpl_->wmsConnectionChangedListener_ = listener;
    instance_->pImpl_->NotifyWMSDisconnected(userId, screenId);
    EXPECT_TRUE(g_errLog.find("wms disconnected") != std::string::npos);

    LOG_SetCallback(nullptr);
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
    WMError ret;
    sptr<IPiPStateChangedListener> listener = nullptr;

    // branch 1: listener is null
    ret = instance_->RegisterPiPStateChangedListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: window adapter register failed
    listener = sptr<TestPiPStateChangedListener>::MakeSptr();
    ret = instance_->RegisterPiPStateChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock adpter to return ok
    ret = mockInstance_->RegisterPiPStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 4: cover all branches
    ret = mockInstance_->RegisterPiPStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterPiPStateChangedListener
 * @tc.desc: check UnregisterPiPStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterPiPStateChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IPiPStateChangedListener> listener = nullptr;

    // branch 1: listener is null
    ret = instance_->UnregisterPiPStateChangedListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: listeners is empty
    listener = sptr<TestPiPStateChangedListener>::MakeSptr();
    ret = instance_->UnregisterPiPStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: erase listener success but windowAdapter return failed
    instance_->pImpl_->pipStateChangedListeners_.push_back(listener);
    instance_->pImpl_->pipStateChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    ret = instance_->UnregisterPiPStateChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 4: mock WindowAdapterLite return ok
    mockInstance_->pImpl_->pipStateChangedListeners_.push_back(listener);
    mockInstance_->pImpl_->pipStateChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(mockUserId_);
    ret = mockInstance_->UnregisterPiPStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UpdatePiPWindowStateChanged
 * @tc.desc: check UpdatePiPWindowStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UpdatePiPWindowStateChanged, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    std::string bundleName = "chery";
    bool isForeground = true;
    sptr<IPiPStateChangedListener> listener = nullptr;

    // branch 1: in WindowManagerLite
    instance_->UpdatePiPWindowStateChanged(bundleName, isForeground);

    // branch 2: in WindowManagerLite::Impl
    listener = sptr<TestPiPStateChangedListener>::MakeSptr();
    instance_->pImpl_->pipStateChangedListeners_.push_back(listener);
    instance_->UpdatePiPWindowStateChanged(bundleName, isForeground);
    EXPECT_TRUE(g_errLog.find("Pip state changed") != std::string::npos);

    LOG_SetCallback(nullptr);
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
 * @tc.name: UnregisterWindowUpdateListener
 * @tc.desc: check UnregisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowUpdateListener, TestSize.Level1)
{
    WMError ret;
    sptr<IWindowUpdateListener> listener = nullptr;

    // branch 1: listener is null
    ret = instance_->UnregisterWindowUpdateListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: not find listener and return ok
    listener = sptr<TestWindowUpdateListener>::MakeSptr();
    instance_->pImpl_->windowUpdateListeners_.clear();
    ret = instance_->UnregisterWindowUpdateListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: erase listener success but adapter return failed
    instance_->pImpl_->windowUpdateListeners_.push_back(listener);
    instance_->pImpl_->windowUpdateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    ret = instance_->UnregisterWindowUpdateListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 4: mock window adapter and return success
    mockInstance_->pImpl_->windowUpdateListeners_.push_back(listener);
    mockInstance_->pImpl_->windowUpdateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(mockUserId_);
    ret = mockInstance_->UnregisterWindowUpdateListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterCallingWindowDisplayChangedListener
 * @tc.desc: check RegisterCallingWindowDisplayChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterCallingWindowDisplayChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IKeyboardCallingWindowDisplayChangedListener> listener = nullptr;

    // branch 1: listener is null
    ret = instance_->RegisterCallingWindowDisplayChangedListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: window adapter register failed
    listener = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();
    ret = instance_->RegisterCallingWindowDisplayChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: mock window adpter and return ok
    ret = mockInstance_->RegisterCallingWindowDisplayChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 4: cover all branches
    ret = mockInstance_->RegisterCallingWindowDisplayChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterCallingWindowDisplayChangedListener
 * @tc.desc: check UnregisterCallingWindowDisplayChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterCallingWindowDisplayChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IKeyboardCallingWindowDisplayChangedListener> listener = nullptr;
 
    // branch 1: listener is null
    ret = instance_->UnregisterCallingWindowDisplayChangedListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
 
    // branch 2: not find listener and return ok
    listener = sptr<TestIKeyboardCallingWindowDisplayChangedListener>::MakeSptr();
    instance_->pImpl_->callingDisplayChangedListeners_.clear();
    ret = instance_->UnregisterCallingWindowDisplayChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
 
    // branch 3: erase listener success but adapter return failed
    instance_->pImpl_->callingDisplayChangedListeners_.push_back(listener);
    instance_->pImpl_->callingDisplayListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    ret = instance_->UnregisterCallingWindowDisplayChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);
 
    // branch 4: mock window adapter and return success
    mockInstance_->pImpl_->callingDisplayChangedListeners_.push_back(listener);
    mockInstance_->pImpl_->callingDisplayListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(mockUserId_);
    ret = mockInstance_->UnregisterCallingWindowDisplayChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
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
 * @tc.name: RegisterWindowInfoChangeCallback
 * @tc.desc: Check RegisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterWindowInfoChangeCallback, Function | SmallTest | Level2)
{
    sptr<IWindowInfoChangedListener> listener = nullptr;
    WMError ret;
    std::unordered_set<WindowInfoKey> observedInfo;

    // branch 1: listener is null
    ret = instance_->RegisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2
    listener = sptr<TestWindowInfoChangedListener>::MakeSptr();
    observedInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    ret = instance_->RegisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowInfoChangeCallback
 * @tc.desc: Check UnregisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterWindowInfoChangeCallback, Function | SmallTest | Level2)
{
    WMError ret;
    std::unordered_set<WindowInfoKey> observedInfo;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1: null listener
    ret = instance_->UnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2
    listener = sptr<TestWindowVisibilityStateListener>::MakeSptr();
    auto interestInfoSizeOld = listener->GetInterestInfo().size();
    observedInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    ret = instance_->UnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());

    // branch 3
    std::unordered_set<WindowInfoKey> observedInfo1;
    observedInfo1.insert(WindowInfoKey::BUNDLE_NAME);
    ret = instance_->UnregisterWindowInfoChangeCallback(observedInfo1, listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
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
 * @tc.name: UnregisterVisibilityStateChangedListener
 * @tc.desc: check UnregisterVisibilityStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterVisibilityStateChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1: listener is null
    ret = instance_->UnregisterVisibilityStateChangedListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: erase success but adapter return failed
    listener = sptr<TestWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->windowVisibilityStateListeners_.emplace_back(listener);
    instance_->pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    ret = instance_->UnregisterVisibilityStateChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: mock window adapter and return ok
    mockInstance_->pImpl_->windowVisibilityStateListeners_.emplace_back(listener);
    mockInstance_->pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr(userId_);
    ret = mockInstance_->UnregisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: check SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetGlobalDragResizeType, TestSize.Level1)
{
    WMError ret;
    auto dragResizeType = DragResizeType::RESIZE_EACH_FRAME;

    // branch 1: return failed
    ret = instance_->SetGlobalDragResizeType(dragResizeType);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock window adapter and return ok
    ret = mockInstance_->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: check GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetGlobalDragResizeType, TestSize.Level1)
{
    WMError ret;
    auto dragResizeType = DragResizeType::RESIZE_EACH_FRAME;

    // branch 1: return failed
    ret = instance_->GetGlobalDragResizeType(dragResizeType);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock window adapter and return ok
    ret = mockInstance_->GetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: check SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetAppDragResizeType, TestSize.Level1)
{
    WMError ret;
    auto dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    std::string bundleName = "test";

    // branch 1: return failed
    ret = instance_->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock window adapter and return ok
    ret = mockInstance_->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: check GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, GetAppDragResizeType, TestSize.Level1)
{
    WMError ret;
    auto dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    std::string bundleName = "test";

    // branch 1: return failed
    ret = instance_->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock window adapter and return ok
    ret = mockInstance_->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetAppKeyFramePolicy
 * @tc.desc: check SetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, SetAppKeyFramePolicy, TestSize.Level1)
{
    WMError ret;
    std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    keyFramePolicy.animationDelay_ = 200;

    // branch 1: return failed
    ret = instance_->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock window adapter and return ok
    ret = mockInstance_->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(WMError::WM_OK, ret);
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
    WMError ret;
    const std::vector<uint64_t> windowIds = { 1, 2 };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;

    // branch 1: return failed
    ret = instance_->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock window adapter and return ok
    ret = mockInstance_->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterAllGroupInfoChangedListener
 * @tc.desc: check RegisterAllGroupInfoChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, RegisterAllGroupInfoChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IAllGroupInfoChangedListener> listener = nullptr;
 
    // branch 1
    ret = instance_->RegisterAllGroupInfoChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
 
    // branch 2: Not use mock adapter and return failed.
    listener = sptr<TestAllGroupInfoChangedListener>::MakeSptr();
    ret = instance_->RegisterAllGroupInfoChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);
 
    // branch 3: use mock and return ok
    ret = mockInstance_->RegisterAllGroupInfoChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
 
    // branch 4: branch overried
    ret = mockInstance_->RegisterAllGroupInfoChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}
 
/**
 * @tc.name: UnregisterAllGroupInfoChangedListener
 * @tc.desc: check UnregisterAllGroupInfoChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerLiteTest, UnregisterAllGroupInfoChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IAllGroupInfoChangedListener> listener = nullptr;
 
    // branch 1: Nullptr
    ret = instance_->UnregisterAllGroupInfoChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
 
    // branch 2: Not find listener and return ok.
    listener = sptr<TestAllGroupInfoChangedListener>::MakeSptr();
    ret = instance_->UnregisterAllGroupInfoChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
 
    // branch 3: Into windowAdapter and unregister failed.
    instance_->pImpl_->allGroupInfoChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr();
    instance_->pImpl_->allGroupInfoChangedListeners_.emplace_back(listener);
    ret = instance_->UnregisterAllGroupInfoChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);
 
    // branch 4: Into windowAdapter and use mock to unregister success.
    mockInstance_->pImpl_->allGroupInfoChangedListenerAgent_ = sptr<WindowManagerAgentLite>::MakeSptr();
    mockInstance_->pImpl_->allGroupInfoChangedListeners_.emplace_back(listener);
    ret = mockInstance_->UnregisterAllGroupInfoChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}
}
} // namespace
} // namespace OHOS::Rosen
