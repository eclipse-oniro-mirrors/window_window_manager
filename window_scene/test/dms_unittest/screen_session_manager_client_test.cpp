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

#include "iremote_object_mocker.h"
#include "screen_session_manager_client.h"
#include "zidl/screen_session_manager_proxy.h"
#include "display_manager.h"
#include "window_manager_hilog.h"
#include "scene_board_judgement.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "ScreenSessionManagerClientTest"};
}

namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg = msg;
    }
}

class DmPrivateWindowListener : public DisplayManager::IPrivateWindowListener {
public:
    void OnPrivateWindow(bool hasPrivate) {WLOGFI("IPrivateWindowListener hasPrivatewindow: %{public}u", hasPrivate);}
};
class DmPrivateWindowListChangeListener : public DisplayManager::IPrivateWindowListChangeListener {
public:
    void OnPrivateWindowListChange(DisplayId displayId, std::vector<std::string> privacyWindowList)
    {
        WLOGFI("IPrivateWindowListChangeListener displayId: %{public}" PRIu64".", displayId);
        std::string concatenatedString;
        std::vector<std::string> result = privacyWindowList;
        for (const auto& window : result) {
            concatenatedString.append(window);
            concatenatedString.append("  ");
        }
        WLOGFI("privacyWindowList: %{public}s", concatenatedString.c_str());
        callback_(privacyWindowList);
    }
    void setCallback(std::function<void(std::vector<std::string>)> callback)
    {
        callback_ = callback;
    }
private:
    std::function<void(std::vector<std::string>)> callback_;
};

class MockDisplayChangeListener : public IDisplayChangeListener {
public:
    virtual void OnDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> info,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type) { return; }
    virtual void OnScreenshot(DisplayId displayId) { return; }
};

class TentModeListenerMock : public ITentModeListener {
public:
    void OnTentModeChange(const TentMode tentMode) override
    {
        WLOGFI("OnTentModeChange has triggered");
    };
};

class ScreenSessionManagerClientTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    ScreenSessionManagerClient* screenSessionManagerClient_;
};

void ScreenSessionManagerClientTest::SetUp()
{
    screenSessionManagerClient_ = &ScreenSessionManagerClient::GetInstance();
    {
        std::lock_guard<std::mutex> lock(screenSessionManagerClient_->screenSessionMapMutex_);
        screenSessionManagerClient_->screenSessionMap_.clear();
        screenSessionManagerClient_->extraScreenSessionMap_.clear();
    }
}

void ScreenSessionManagerClientTest::TearDown()
{
    {
        std::lock_guard<std::mutex> lock(screenSessionManagerClient_->screenSessionMapMutex_);
        screenSessionManagerClient_->screenSessionMap_.clear();
        screenSessionManagerClient_->extraScreenSessionMap_.clear();
    }
    screenSessionManagerClient_ = nullptr;
}

/**
 * @tc.name: RegisterScreenConnectionListener
 * @tc.desc: RegisterScreenConnectionListener test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, RegisterScreenConnectionListener, TestSize.Level1)
{
    IScreenConnectionListener* listener = nullptr;
    screenSessionManagerClient_->RegisterScreenConnectionListener(listener);
    EXPECT_EQ(screenSessionManagerClient_->screenConnectionListener_, nullptr);
}

/**
 * @tc.name: RegisterTentModeChangeListener
 * @tc.desc: RegisterTentModeChangeListener test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, RegisterTentModeChangeListener, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ITentModeListener* listener = nullptr;
    screenSessionManagerClient_->RegisterTentModeChangeListener(listener);
    EXPECT_TRUE(logMsg.find("Failed to register tent mode listener, listener is null") != std::string::npos);
    logMsg.clear();
    listener = new TentModeListenerMock();
    screenSessionManagerClient_->RegisterTentModeChangeListener(listener);
    EXPECT_TRUE(logMsg.find("Failed to register tent mode listener, listener is null") == std::string::npos);
    logMsg.clear();
}

/**
 * @tc.name: OnTentModeChange
 * @tc.desc: OnTentModeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnTentModeChange, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ITentModeListener* listener = nullptr;
    screenSessionManagerClient_->RegisterTentModeChangeListener(listener);
    screenSessionManagerClient_->OnTentModeChange(TentMode::UNKNOWN);
    EXPECT_EQ(screenSessionManagerClient_->tentModeListener_, nullptr);
    listener = new TentModeListenerMock();
    screenSessionManagerClient_->RegisterTentModeChangeListener(listener);
    screenSessionManagerClient_->OnTentModeChange(TentMode::UNKNOWN);
    EXPECT_TRUE(logMsg.find("OnTentModeChange has triggered") != std::string::npos);
    logMsg.clear();
}

/**
 * @tc.name: GetScreenSession
 * @tc.desc: GetScreenSession test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSession, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession, nullptr);

    screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    auto screenSession2 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession2, screenSession);

    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSession = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: GetScreenSessionExtra
 * @tc.desc: GetScreenSessionExtra test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSessionExtra, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = screenSessionManagerClient_->GetScreenSessionExtra(screenId);
    EXPECT_EQ(screenSession, nullptr);

    screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->extraScreenSessionMap_.emplace(screenId, screenSession);

    auto screenSession2 = screenSessionManagerClient_->GetScreenSessionExtra(screenId);
    EXPECT_EQ(screenSession2, screenSession);

    screenSessionManagerClient_->extraScreenSessionMap_.clear();
    screenSession = screenSessionManagerClient_->GetScreenSessionExtra(screenId);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: OnScreenConnectionChanged01
 * @tc.desc: OnScreenConnectionChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenConnectionChanged01, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    screenSessionManagerClient_->screenSessionManager_ = new ScreenSessionManagerProxy(iRemoteObjectMocker);
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);

    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
    };
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    screenSessionManagerClient_->OnScreenConnectionChanged(option, screenEvent);
    sptr<ScreenSession> screenSession2 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_NE(screenSession2, nullptr);
}

/**
 * @tc.name: OnScreenConnectionChanged02
 * @tc.desc: OnScreenConnectionChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenConnectionChanged02, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    ScreenEvent screenEvent = ScreenEvent::DISCONNECTED;
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
    };
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    screenSessionManagerClient_->OnScreenConnectionChanged(option, screenEvent);
 
    screenEvent = ScreenEvent::CONNECTED;
    screenSessionManagerClient_->OnScreenConnectionChanged(option, screenEvent);
    sptr<ScreenSession> screenSession2 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_NE(screenSession2, nullptr);
}

/**
 * @tc.name: CheckIfNeedConnectScreen01
 * @tc.desc: CheckIfNeedConnectScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CheckIfNeedConnectScreen01, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);

    SessionOption option = {
        .rsId_ = 0,
        .screenId_ = 0,
        .name_ = "HiCar",
    };
    ScreenId screenId = 0;
    screenSessionManagerClient_->screenSessionManager_->GetScreenProperty(screenId).SetScreenType(ScreenType::VIRTUAL);
    bool result = screenSessionManagerClient_->CheckIfNeedConnectScreen(option);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckIfNeedConnectScreen02
 * @tc.desc: CheckIfNeedConnectScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CheckIfNeedConnectScreen02, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    SessionOption option = {
        .rsId_ = SCREEN_ID_INVALID,
        .screenId_ = 0,
        .name_ = "",
    };
    ScreenId screenId = 0;
    screenSessionManagerClient_->screenSessionManager_->GetScreenProperty(screenId).SetScreenType(ScreenType::VIRTUAL);
    bool result = screenSessionManagerClient_->CheckIfNeedConnectScreen(option);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckIfNeedConnectScreen03
 * @tc.desc: CheckIfNeedConnectScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CheckIfNeedConnectScreen03, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    SessionOption option = {
        .rsId_ = 0,
        .screenId_ = 0,
        .name_ = "",
    };
    ScreenId screenId = 0;
    screenSessionManagerClient_->screenSessionManager_->GetScreenProperty(screenId).SetScreenType(ScreenType::REAL);
    bool result = screenSessionManagerClient_->CheckIfNeedConnectScreen(option);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckIfNeedConnectScreen04
 * @tc.desc: CheckIfNeedConnectScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CheckIfNeedConnectScreen04, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    SessionOption option = {
        .rsId_ = 0,
        .screenId_ = 0,
        .name_ = "HiCar",
    };
    ScreenId screenId = 0;
    screenSessionManagerClient_->screenSessionManager_->GetScreenProperty(screenId).SetScreenType(ScreenType::VIRTUAL);
    bool result = screenSessionManagerClient_->CheckIfNeedConnectScreen(option);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckIfNeedConnectScreen05
 * @tc.desc: CheckIfNeedConnectScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CheckIfNeedConnectScreen05, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    SessionOption option = {
        .rsId_ = 0,
        .screenId_ = 0,
        .name_ = "SuperLauncher",
    };
    ScreenId screenId = 0;
    screenSessionManagerClient_->screenSessionManager_->GetScreenProperty(screenId).SetScreenType(ScreenType::VIRTUAL);
    bool result = screenSessionManagerClient_->CheckIfNeedConnectScreen(option);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckIfNeedConnectScreen06
 * @tc.desc: CheckIfNeedConnectScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CheckIfNeedConnectScreen06, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    SessionOption option = {
        .rsId_ = 0,
        .screenId_ = 0,
        .name_ = "CastEngine",
    };
    ScreenId screenId = 0;
    screenSessionManagerClient_->screenSessionManager_->GetScreenProperty(screenId).SetScreenType(ScreenType::VIRTUAL);
    bool result = screenSessionManagerClient_->CheckIfNeedConnectScreen(option);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: OnPowerStatusChanged
 * @tc.desc: OnPowerStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnPowerStatusChanged, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    DisplayPowerEvent event = DisplayPowerEvent::WAKE_UP;
    EventStatus status = EventStatus::BEGIN;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_ACCESS;
    screenSessionManagerClient_->OnPowerStatusChanged(event, status, reason);
    sptr<ScreenSession> screenSession1 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_NE(screenSession1, nullptr);
}

/**
 * @tc.name: GetAllScreensProperties
 * @tc.desc: GetAllScreensProperties test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetAllScreensProperties, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    EXPECT_EQ(1, screenSessionManagerClient_->GetAllScreensProperties().size());
}

/**
 * @tc.name: SetPrivacyStateByDisplayId01
 * @tc.desc: SetPrivacyStateByDisplayId01 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetPrivacyStateByDisplayId01, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    EXPECT_EQ(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->ConnectToServer();
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);

    sptr<DisplayManager::IPrivateWindowListener> displayListener_ = new DmPrivateWindowListener();
    DisplayManager::GetInstance().RegisterPrivateWindowListener(displayListener_);
    
    DisplayId id = 0;
    bool hasPrivate = true;
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId = {{id, hasPrivate}};
    screenSessionManagerClient_->SetPrivacyStateByDisplayId(privacyBundleDisplayId);

    bool result = false;
    screenSessionManagerClient_->screenSessionManager_->HasPrivateWindow(id, result);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, true);
    } else {
        EXPECT_NE(result, true);
    }
}

/**
 * @tc.name: SetPrivacyStateByDisplayId02
 * @tc.desc: SetPrivacyStateByDisplayId02 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetPrivacyStateByDisplayId02, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    DisplayId id = 0;
    bool hasPrivate = false;
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId = {{id, hasPrivate}};
    screenSessionManagerClient_->SetPrivacyStateByDisplayId(privacyBundleDisplayId);
    bool result = true;
    screenSessionManagerClient_->screenSessionManager_->HasPrivateWindow(id, result);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, false);
    } else {
        EXPECT_NE(result, false);
    }
}

/**
 * @tc.name: SetScreenPrivacyWindowList
 * @tc.desc: SetScreenPrivacyWindowList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetScreenPrivacyWindowList, TestSize.Level1)
{
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    EXPECT_EQ(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->ConnectToServer();
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);

    DisplayId id = 0;
    std::vector<std::string> privacyWindowList{"win0", "win1"};
    std::vector<std::string> privacyWindowList2{"win0"};
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener_ = new DmPrivateWindowListChangeListener();
    listener_->setCallback([privacyWindowList, privacyWindowList2](std::vector<std::string> windowList)
    {
        EXPECT_EQ(windowList, privacyWindowList);
        EXPECT_NE(windowList, privacyWindowList2);
    });
    DisplayManager::GetInstance().RegisterPrivateWindowListChangeListener(listener_);

    std::unordered_map<DisplayId, std::vector<std::string>> privacyBundleList = {{id, privacyWindowList}};
    screenSessionManagerClient_->SetScreenPrivacyWindowList(privacyBundleList);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/**
 * @tc.name: GetFoldDisplayMode01
 * @tc.desc: GetFoldDisplayMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetFoldDisplayMode01, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_NE(FoldDisplayMode::FULL, screenSessionManagerClient_->GetFoldDisplayMode());
        EXPECT_NE(FoldStatus::UNKNOWN, screenSessionManagerClient_->GetFoldStatus());
    } else if (screenSessionManagerClient_->IsFoldable()) {
        EXPECT_NE(FoldDisplayMode::UNKNOWN, screenSessionManagerClient_->GetFoldDisplayMode());
        EXPECT_NE(FoldStatus::UNKNOWN, screenSessionManagerClient_->GetFoldStatus());
    } else {
        EXPECT_NE(FoldDisplayMode::FULL, screenSessionManagerClient_->GetFoldDisplayMode());
        EXPECT_EQ(FoldStatus::UNKNOWN, screenSessionManagerClient_->GetFoldStatus());
    }
    EXPECT_EQ(0, screenSessionManagerClient_->GetCurvedCompressionArea());
}

/**
 * @tc.name: GetFoldDisplayMode02
 * @tc.desc: GetFoldDisplayMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetFoldDisplayMode02, TestSize.Level1)
{
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    ScreenId screenId = 0;
    bool foldToExpand = true;
    DMRect area;
    screenSessionManagerClient_->GetPhyScreenProperty(screenId);
    screenSessionManagerClient_->UpdateAvailableArea(screenId, area);
    screenSessionManagerClient_->NotifyFoldToExpandCompletion(foldToExpand);
    if (screenSessionManagerClient_->IsFoldable() && !FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_NE(FoldDisplayMode::UNKNOWN, screenSessionManagerClient_->GetFoldDisplayMode());
    } else {
        EXPECT_EQ(FoldDisplayMode::UNKNOWN, screenSessionManagerClient_->GetFoldDisplayMode());
    }
    EXPECT_EQ(FoldStatus::UNKNOWN, screenSessionManagerClient_->GetFoldStatus());
    EXPECT_EQ(0, screenSessionManagerClient_->GetCurvedCompressionArea());
}

/**
 * @tc.name: UpdateScreenRotationProperty
 * @tc.desc: UpdateScreenRotationProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateScreenRotationProperty, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenId displayNodeScreenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    DMRect area;
    bool foldToExpand = true;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    float scaleX = 1.0;
    float scaleY = 1.0;
    ScreenDirectionInfo directionInfo;
    directionInfo.notifyRotation_ = 90;
    directionInfo.screenRotation_ = 90;
    directionInfo.rotation_ = 90;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_BEGIN;
    screenSessionManagerClient_->UpdateScreenRotationProperty(screenId, bounds, directionInfo,
        screenPropertyChangeType);
    screenSessionManagerClient_->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
    screenSessionManagerClient_->GetPhyScreenProperty(screenId);
    screenSessionManagerClient_->UpdateAvailableArea(screenId, area);
    screenSessionManagerClient_->NotifyFoldToExpandCompletion(foldToExpand);
    screenSessionManagerClient_->GetScreenSnapshot(screenId, scaleX, scaleY);
    sptr<ScreenSession> screenSession1 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_NE(screenSession1, nullptr);
}

/**
 * @tc.name: GetScreenSnapshot
 * @tc.desc: GetScreenSnapshot test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSnapshot, TestSize.Level1)
{
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    ScreenId displayNodeScreenId = 0;
    float scaleX = 1.0;
    float scaleY = 1.0;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    ScreenDirectionInfo directionInfo;
    directionInfo.notifyRotation_ = 90;
    directionInfo.screenRotation_ = 90;
    directionInfo.rotation_ = 90;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_BEGIN;
    screenSessionManagerClient_->UpdateScreenRotationProperty(screenId, bounds, directionInfo,
        screenPropertyChangeType);
    screenSessionManagerClient_->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
    std::shared_ptr<Media::PixelMap> res = screenSessionManagerClient_->GetScreenSnapshot(screenId, scaleX, scaleY);
    EXPECT_EQ(nullptr, res);
}

/**
 * @tc.name: SetScreenOffDelayTime
 * @tc.desc: SetScreenOffDelayTime test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetScreenOffDelayTime, TestSize.Level1)
{
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    int32_t ret = screenSessionManagerClient_->SetScreenOffDelayTime(1);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GetDeviceScreenConfig
 * @tc.desc: GetDeviceScreenConfig test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetDeviceScreenConfig, TestSize.Level1)
{
    DeviceScreenConfig deviceScreenConfig = screenSessionManagerClient_->GetDeviceScreenConfig();
    EXPECT_FALSE(deviceScreenConfig.rotationPolicy_.empty());
}

/**
 * @tc.name: OnUpdateFoldDisplayMode
 * @tc.desc: OnUpdateFoldDisplayMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetFoldDisplayMode, TestSize.Level1)
{
    screenSessionManagerClient_->OnUpdateFoldDisplayMode(FoldDisplayMode::UNKNOWN);
    auto ret1 = screenSessionManagerClient_->GetFoldDisplayMode();
    EXPECT_EQ(FoldDisplayMode::UNKNOWN, ret1);
    screenSessionManagerClient_->OnUpdateFoldDisplayMode(FoldDisplayMode::FULL);
    auto ret2 = screenSessionManagerClient_->GetFoldDisplayMode();
    EXPECT_EQ(FoldDisplayMode::FULL, ret2);
    screenSessionManagerClient_->OnUpdateFoldDisplayMode(FoldDisplayMode::MAIN);
    auto ret3 = screenSessionManagerClient_->GetFoldDisplayMode();
    EXPECT_EQ(FoldDisplayMode::MAIN, ret3);
}

/**
 * @tc.name: GetScreenSessionById
 * @tc.desc: GetScreenSessionById test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSessionById, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<ScreenSession> ret = screenSessionManagerClient_->GetScreenSessionById(screenId);
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name: ConnectToServer01
 * @tc.desc: ConnectToServer test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, ConnectToServer01, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    screenSessionManagerClient_->ConnectToServer();
}

/**
 * @tc.name: OnPropertyChanged
 * @tc.desc: OnPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnPropertyChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenProperty property;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnPropertyChanged(screenId, property, reason);
}

/**
 * @tc.name: OnFoldPropertyChanged
 * @tc.desc: OnFoldPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnFoldPropertyChange_InitRotation, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenProperty property;
    ScreenProperty midProperty;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    screenSessionManagerClient_->OnFoldPropertyChange(screenId, property, reason, displayMode, midProperty);
    logMsg.clear();
    LOG_SetCallback(nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: OnFoldPropertyChanged
 * @tc.desc: OnFoldPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnFoldPropertyChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenProperty property;
    ScreenProperty midProperty;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnFoldPropertyChanged(screenId, property, reason, displayMode, midProperty);
}

/**
 * @tc.name: OnSensorRotationChanged01
 * @tc.desc: OnSensorRotationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnSensorRotationChanged01, TestSize.Level1)
{
    ScreenId screenId = 0;
    float sensorRotation = 0;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnSensorRotationChanged(screenId, sensorRotation, false);
}

/**
 * @tc.name: OnSensorRotationChanged02
 * @tc.desc: OnSensorRotationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnSensorRotationChanged02, TestSize.Level1)
{
    ScreenId screenId = 1;
    float sensorRotation = 90.0f;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnSensorRotationChanged(screenId, sensorRotation, false);
}

/**
 * @tc.name: OnHoverStatusChanged01
 * @tc.desc: OnHoverStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnHoverStatusChanged01, TestSize.Level1)
{
    ScreenId screenId = 0;
    int32_t hoverStatus = 0;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnHoverStatusChanged(screenId, hoverStatus);
}

/**
 * @tc.name: OnScreenOrientationChanged01
 * @tc.desc: OnScreenOrientationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenOrientationChanged01, TestSize.Level1)
{
    ScreenId screenId = 0;
    float screenOrientation = 0;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnScreenOrientationChanged(screenId, screenOrientation);
}

/**
 * @tc.name: OnScreenOrientationChanged02
 * @tc.desc: OnScreenOrientationChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenOrientationChanged02, TestSize.Level1)
{
    ScreenId screenId = 1;
    float screenOrientation = 90.0f;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnScreenOrientationChanged(screenId, screenOrientation);
}

/**
 * @tc.name: OnScreenRotationLockedChanged01
 * @tc.desc: OnScreenRotationLockedChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenRotationLockedChanged01, TestSize.Level1)
{
    ScreenId screenId = 0;
    bool isLocked = false;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnScreenRotationLockedChanged(screenId, isLocked);
}

/**
 * @tc.name: OnScreenRotationLockedChanged02
 * @tc.desc: OnScreenRotationLockedChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenRotationLockedChanged02, TestSize.Level1)
{
    ScreenId screenId = 1;
    bool isLocked = true;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnScreenRotationLockedChanged(screenId, isLocked);
}

/**
 * @tc.name: OnSuperFoldStatusChanged
 * @tc.desc: OnSuperFoldStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnSuperFoldStatusChanged, TestSize.Level1)
{
    ScreenId screenId = 0;
    SuperFoldStatus superFoldStatus = SuperFoldStatus::UNKNOWN;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnSuperFoldStatusChanged(screenId, superFoldStatus);
}

/**
 * @tc.name: RegisterDisplayChangeListener
 * @tc.desc: RegisterDisplayChangeListener test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, RegisterDisplayChangeListener, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->RegisterDisplayChangeListener(nullptr);
}

/**
 * @tc.name: OnDisplayStateChanged01
 * @tc.desc: OnDisplayStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnDisplayStateChanged01, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap {
        { displayInfo->GetDisplayId(), displayInfo },
    };
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnDisplayStateChanged(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: OnDisplayStateChanged02
 * @tc.desc: OnDisplayStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnDisplayStateChanged02, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap {
        { displayInfo->GetDisplayId(), displayInfo },
    };
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->displayChangeListener_ = nullptr;
    screenSessionManagerClient_->OnDisplayStateChanged(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: OnGetSurfaceNodeIdsFromMissionIdsChanged01
 * @tc.desc: OnGetSurfaceNodeIdsFromMissionIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnGetSurfaceNodeIdsFromMissionIdsChanged01, TestSize.Level1)
{
    std::vector<uint64_t> missionIds = {0, 1};
    std::vector<uint64_t> surfaceNodeIds;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnGetSurfaceNodeIdsFromMissionIdsChanged(missionIds, surfaceNodeIds);
}

/**
 * @tc.name: OnGetSurfaceNodeIdsFromMissionIdsChanged02
 * @tc.desc: OnGetSurfaceNodeIdsFromMissionIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnGetSurfaceNodeIdsFromMissionIdsChanged02, TestSize.Level1)
{
    std::vector<uint64_t> missionIds = {0, 1, 2};
    std::vector<uint64_t> surfaceNodeIds;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnGetSurfaceNodeIdsFromMissionIdsChanged(missionIds, surfaceNodeIds);
}

/**
 * @tc.name: OnGetSurfaceNodeIdsFromMissionIdsChanged03
 * @tc.desc: OnGetSurfaceNodeIdsFromMissionIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnGetSurfaceNodeIdsFromMissionIdsChanged03, TestSize.Level1)
{
    std::vector<uint64_t> missionIds = {0, 1};
    std::vector<uint64_t> surfaceNodeIds;
    const std::vector<uint32_t> needWindowTypeList = {1234};

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->displayChangeListener_ = nullptr;
    screenSessionManagerClient_->OnGetSurfaceNodeIdsFromMissionIdsChanged(missionIds, surfaceNodeIds,
        needWindowTypeList);
}

/**
 * @tc.name: OnSetSurfaceNodeIdsChanged01
 * @tc.desc: OnSetSurfaceNodeIdsChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnSetSurfaceNodeIdsChanged01, TestSize.Level1)
{
    sptr<ScreenSessionManagerClient> client = sptr<ScreenSessionManagerClient>::MakeSptr();
    ASSERT_NE(nullptr, client);

    DisplayId displayId = 0;
    std::vector<uint64_t> surfaceNodeIds = { 100, 101 };
    sptr<MockDisplayChangeListener> listener = sptr<MockDisplayChangeListener>::MakeSptr();
    client->displayChangeListener_ = listener;
    client->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);

    client->displayChangeListener_ = nullptr;
    client->OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);
}

/**
 * @tc.name: OnVirtualScreenDisconnected01
 * @tc.desc: OnVirtualScreenDisconnected test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnVirtualScreenDisconnected01, TestSize.Level1)
{
    sptr<ScreenSessionManagerClient> client = sptr<ScreenSessionManagerClient>::MakeSptr();
    ASSERT_NE(nullptr, client);

    DisplayId displayId = 0;
    sptr<MockDisplayChangeListener> listener = sptr<MockDisplayChangeListener>::MakeSptr();
    client->displayChangeListener_ = listener;
    client->OnVirtualScreenDisconnected(displayId);

    client->displayChangeListener_ = nullptr;
    client->OnVirtualScreenDisconnected(displayId);
}

/**
 * @tc.name: OnScreenshot01
 * @tc.desc: OnScreenshot test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenshot01, TestSize.Level1)
{
    DisplayId displayId = 0;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnScreenshot(displayId);
}

/**
 * @tc.name: OnScreenshot02
 * @tc.desc: OnScreenshot test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnScreenshot02, TestSize.Level1)
{
    DisplayId displayId = 0;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->displayChangeListener_ = nullptr;
    screenSessionManagerClient_->OnScreenshot(displayId);
}

/**
 * @tc.name: OnImmersiveStateChanged01
 * @tc.desc: OnImmersiveStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnImmersiveStateChanged01, TestSize.Level1)
{
    bool immersive = false;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnImmersiveStateChanged(0u, immersive);
}

/**
 * @tc.name: OnImmersiveStateChanged02
 * @tc.desc: OnImmersiveStateChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnImmersiveStateChanged02, TestSize.Level1)
{
    bool immersive = false;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->displayChangeListener_ = nullptr;
    screenSessionManagerClient_->OnImmersiveStateChanged(0u, immersive);
}

/**
 * @tc.name: SetDisplayNodeScreenId
 * @tc.desc: SetDisplayNodeScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetDisplayNodeScreenId, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenId displayNodeScreenId = 0;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
}

/**
 * @tc.name: GetCurvedCompressionArea
 * @tc.desc: GetCurvedCompressionArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetCurvedCompressionArea, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetCurvedCompressionArea();
}

/**
 * @tc.name: GetPhyScreenProperty
 * @tc.desc: GetPhyScreenProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetPhyScreenProperty, TestSize.Level1)
{
    ScreenId screenId = 0;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetPhyScreenProperty(screenId);
}

/**
 * @tc.name: NotifyDisplayChangeInfoChanged
 * @tc.desc: NotifyDisplayChangeInfoChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, NotifyDisplayChangeInfoChanged, TestSize.Level1)
{
    sptr<DisplayChangeInfo> info = new DisplayChangeInfo();
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->NotifyDisplayChangeInfoChanged(info);
}

/**
 * @tc.name: SetScreenPrivacyState01
 * @tc.desc: SetScreenPrivacyState test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetScreenPrivacyState01, TestSize.Level1)
{
    bool hasPrivate = false;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SetScreenPrivacyState(hasPrivate);
}

/**
 * @tc.name: SetScreenPrivacyState02
 * @tc.desc: SetScreenPrivacyState test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetScreenPrivacyState02, TestSize.Level1)
{
    bool hasPrivate = false;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    screenSessionManagerClient_->screenSessionManager_ = new ScreenSessionManagerProxy(iRemoteObjectMocker);
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->SetScreenPrivacyState(hasPrivate);
}

/**
 * @tc.name: SetPrivacyStateByDisplayId
 * @tc.desc: SetPrivacyStateByDisplayId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetPrivacyStateByDisplayId, TestSize.Level1)
{
    DisplayId id = 0;
    bool hasPrivate = false;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId = {{id, hasPrivate}};
    screenSessionManagerClient_->SetPrivacyStateByDisplayId(privacyBundleDisplayId);
}

/**
 * @tc.name: UpdateAvailableArea
 * @tc.desc: UpdateAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateAvailableArea, TestSize.Level1)
{
    ScreenId screenId = 0;
    DMRect area;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->UpdateAvailableArea(screenId, area);
}

/**
 * @tc.name: NotifyFoldToExpandCompletion
 * @tc.desc: NotifyFoldToExpandCompletion test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, NotifyFoldToExpandCompletion, TestSize.Level1)
{
    bool foldToExpand = true;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->NotifyFoldToExpandCompletion(foldToExpand);
}

/**
 * @tc.name: NotifyScreenConnectCompletion
 * @tc.desc: NotifyScreenConnectCompletion test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, NotifyScreenConnectCompletion, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->NotifyScreenConnectCompletion(1001);
}

/**
 * @tc.name: SwitchUserCallback01
 * @tc.desc: SwitchUserCallback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SwitchUserCallback01, TestSize.Level1)
{
    std::vector<int32_t> oldScbPids = {0, 1};
    int32_t currentScbPid = 1;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SwitchUserCallback(oldScbPids, currentScbPid);
}

/**
 * @tc.name: SwitchUserCallback02
 * @tc.desc: SwitchUserCallback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SwitchUserCallback02, TestSize.Level1)
{
    std::vector<int32_t> oldScbPids = {};
    int32_t currentScbPid = 1;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SwitchUserCallback(oldScbPids, currentScbPid);
}

/**
 * @tc.name: SwitchUserCallback03
 * @tc.desc: SwitchUserCallback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SwitchUserCallback03, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<int32_t> oldScbPids = {0};
    int32_t currentScbPid = 1;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    screenSessionManagerClient_->screenSessionManager_ = sptr<ScreenSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ScreenId screenId = 1234;
    sptr<ScreenSession> screenSession = nullptr;
    screenSessionManagerClient_->screenSessionMap_.insert({screenId, screenSession});
    screenSessionManagerClient_->SwitchUserCallback(oldScbPids, currentScbPid);
    EXPECT_TRUE(logMsg.find("screenSession is null") != std::string::npos);
    logMsg.clear();
    screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.erase(screenId);
    screenSessionManagerClient_->screenSessionMap_.insert({screenId, screenSession});
    screenSessionManagerClient_->SwitchUserCallback(oldScbPids, currentScbPid);
    EXPECT_FALSE(logMsg.find("screenSession is null") != std::string::npos);
    screenSessionManagerClient_->screenSessionMap_.erase(screenId);
    logMsg.clear();
}

/**
 * @tc.name: SwitchingCurrentUser
 * @tc.desc: SwitchingCurrentUser test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SwitchingCurrentUser, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SwitchingCurrentUser();
}

/**
 * @tc.name: GetFoldStatus
 * @tc.desc: GetFoldStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetFoldStatus, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetFoldStatus();
}

/**
 * @tc.name: GetSuperFoldStatus
 * @tc.desc: GetSuperFoldStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetSuperFoldStatus, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetSuperFoldStatus();
}

/**
 * @tc.name: SetLandscapeLockStatus
 * @tc.desc: SetLandscapeLockStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetLandscapeLockStatus, Function | SmallTest | Level2)
{
    bool isLocked = false;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SetLandscapeLockStatus(isLocked);
}

/**
 * @tc.name: GetDefaultScreenId
 * @tc.desc: GetDefaultScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetDefaultScreenId, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetDefaultScreenId();
}

/**
 * @tc.name: IsFoldable
 * @tc.desc: IsFoldable test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, IsFoldable, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->IsFoldable();
}

/**
 * @tc.name: SetVirtualPixelRatioSystem
 * @tc.desc: SetVirtualPixelRatioSystem test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetVirtualPixelRatioSystem, TestSize.Level1)
{
    ScreenId screenId = 0;
    float virtualPixelRatio = 1.0f;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
}

/**
 * @tc.name: UpdateDisplayHookInfo
 * @tc.desc: UpdateDisplayHookInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateDisplayHookInfo, TestSize.Level1)
{
    int32_t uid = 0;
    bool enable = false;
    DMHookInfo hookInfo;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->UpdateDisplayHookInfo(uid, enable, hookInfo);
}

HWTEST_F(ScreenSessionManagerClientTest, GetDisplayHookInfo, Function | SmallTest | Level2)
{
    int32_t uid = 0;
    DMHookInfo hookInfo;
    hookInfo.enableHookRotation_ = true;
    hookInfo.rotation_ = true;
    hookInfo.density_ = 1.1;
    hookInfo.width_ = 100;
    hookInfo.height_ = 200;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);

    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    screenSessionManagerClient_->UpdateDisplayHookInfo(uid, true, hookInfo);
    screenSessionManagerClient_->GetDisplayHookInfo(uid, hookInfo);
    EXPECT_TRUE(logMsg.find("screenSessionManager_ is null") == std::string::npos);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnFoldStatusChangedReportUE
 * @tc.desc: OnFoldStatusChangedReportUE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnFoldStatusChangedReportUE, TestSize.Level1)
{
    std::vector<std::string> screenFoldInfo;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnFoldStatusChangedReportUE(screenFoldInfo);
}

/**
 * @tc.name: UpdateDisplayScale01
 * @tc.desc: UpdateDisplayScale test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateDisplayScale01, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    const float scaleX = 1.0f;
    const float scaleY = 1.0f;
    const float pivotX = 0.5f;
    const float pivotY = 0.5f;
    const float translateX = 0.0f;
    const float translateY = 0.0f;
    ScreenId defaultScreenId = screenSessionManagerClient_->GetDefaultScreenId();
    screenSessionManagerClient_->UpdateDisplayScale(defaultScreenId, scaleX, scaleY, pivotX, pivotY, translateX,
                                                    translateY);
}

/**
 * @tc.name: UpdateDisplayScale02
 * @tc.desc: UpdateDisplayScale test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateDisplayScale02, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    const float scaleX = 1.0f;
    const float scaleY = 1.0f;
    const float pivotX = 0.5f;
    const float pivotY = 0.5f;
    const float translateX = 0.0f;
    const float translateY = 0.0f;
    uint64_t fakeScreenId = 100;
    screenSessionManagerClient_->UpdateDisplayScale(fakeScreenId, scaleX, scaleY, pivotX, pivotY, translateX,
                                                    translateY);
}

/**
 * @tc.name: RegisterScreenConnectionListener02
 * @tc.desc: RegisterScreenConnectionListener test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, RegisterScreenConnectionListener02, TestSize.Level1)
{
    IScreenConnectionListener* listener = nullptr;
    screenSessionManagerClient_->RegisterScreenConnectionListener(listener);
    EXPECT_EQ(screenSessionManagerClient_->screenConnectionListener_, nullptr);
}

/**
 * @tc.name: GetScreenSession02
 * @tc.desc: GetScreenSession test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSession02, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession, nullptr);

    screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    auto screenSession2 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession2, screenSession);

    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSession = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_EQ(screenSession, nullptr);
}

/**
 * @tc.name: SwitchingCurrentUser02
 * @tc.desc: SwitchingCurrentUser test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SwitchingCurrentUser02, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SwitchingCurrentUser();
}

/**
 * @tc.name: GetFoldStatus02
 * @tc.desc: GetFoldStatus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetFoldStatus02, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetFoldStatus();
}

/**
 * @tc.name: GetDefaultScreenId02
 * @tc.desc: GetDefaultScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetDefaultScreenId02, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->GetDefaultScreenId();
}

/**
 * @tc.name: IsFoldable02
 * @tc.desc: IsFoldable test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, IsFoldable02, TestSize.Level1)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->IsFoldable();
}

/**
 * @tc.name: SetVirtualPixelRatioSystem02
 * @tc.desc: SetVirtualPixelRatioSystem test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetVirtualPixelRatioSystem02, TestSize.Level1)
{
    ScreenId screenId = 0;
    float virtualPixelRatio = 1.0f;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
}

/**
 * @tc.name: UpdateDisplayHookInfo02
 * @tc.desc: UpdateDisplayHookInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateDisplayHookInfo02, TestSize.Level1)
{
    int32_t uid = 0;
    bool enable = false;
    DMHookInfo hookInfo;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->UpdateDisplayHookInfo(uid, enable, hookInfo);
}

/**
 * @tc.name: OnFoldStatusChangedReportUE02
 * @tc.desc: OnFoldStatusChangedReportUE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnFoldStatusChangedReportUE02, TestSize.Level1)
{
    std::vector<std::string> screenFoldInfo;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->OnFoldStatusChangedReportUE(screenFoldInfo);
}

/**
 * @tc.name: SetPrivacyStateByDisplayId03
 * @tc.desc: SetPrivacyStateByDisplayId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetPrivacyStateByDisplayId03, TestSize.Level1)
{
    DisplayId id = 0;
    bool hasPrivate = false;
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->SetPrivacyStateByDisplayId(id, hasPrivate);
}

/**
 * @tc.name: UpdateAvailableArea02
 * @tc.desc: UpdateAvailableArea test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateAvailableArea02, TestSize.Level1)
{
    ScreenId screenId = 0;
    DMRect area;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->UpdateAvailableArea(screenId, area);
}

/**
 * @tc.name: NotifyFoldToExpandCompletion02
 * @tc.desc: NotifyFoldToExpandCompletion test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, NotifyFoldToExpandCompletion02, TestSize.Level1)
{
    bool foldToExpand = true;

    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->NotifyFoldToExpandCompletion(foldToExpand);
}

/**
 * @tc.name: OnPowerStatusChanged02
 * @tc.desc: OnPowerStatusChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnPowerStatusChanged02, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    DisplayPowerEvent event = DisplayPowerEvent::WAKE_UP;
    EventStatus status = EventStatus::BEGIN;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_ACCESS;
    screenSessionManagerClient_->OnPowerStatusChanged(event, status, reason);
    sptr<ScreenSession> screenSession1 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_NE(screenSession1, nullptr);
}

/**
 * @tc.name: GetAllScreensProperties02
 * @tc.desc: GetAllScreensProperties test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetAllScreensProperties02, TestSize.Level1)
{
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    EXPECT_EQ(1, screenSessionManagerClient_->GetAllScreensProperties().size());
}

/**
 * @tc.name: SetScreenPrivacyWindowList02
 * @tc.desc: SetScreenPrivacyWindowList test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetScreenPrivacyWindowList02, TestSize.Level1)
{
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    EXPECT_EQ(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->ConnectToServer();
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);

    DisplayId id = 0;
    std::vector<std::string> privacyWindowList{"win0", "win1"};
    std::vector<std::string> privacyWindowList2{"win0"};
    sptr<DisplayManager::IPrivateWindowListChangeListener> listener_ = new DmPrivateWindowListChangeListener();
    listener_->setCallback([privacyWindowList, privacyWindowList2](std::vector<std::string> windowList)
    {
        EXPECT_EQ(windowList, privacyWindowList);
        EXPECT_NE(windowList, privacyWindowList2);
    });
    DisplayManager::GetInstance().RegisterPrivateWindowListChangeListener(listener_);

    screenSessionManagerClient_->SetScreenPrivacyWindowList(id, privacyWindowList);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/**
 * @tc.name: UpdateScreenRotationProperty02
 * @tc.desc: UpdateScreenRotationProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, UpdateScreenRotationProperty02, TestSize.Level1)
{
    ScreenId screenId = 0;
    ScreenId displayNodeScreenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(0, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    DMRect area;
    bool foldToExpand = true;
    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    float scaleX = 1.0;
    float scaleY = 1.0;
    ScreenDirectionInfo directionInfo;
    directionInfo.notifyRotation_ = 90;
    directionInfo.screenRotation_ = 90;
    directionInfo.rotation_ = 90;
    ScreenPropertyChangeType screenPropertyChangeType = ScreenPropertyChangeType::ROTATION_BEGIN;
    screenSessionManagerClient_->UpdateScreenRotationProperty(screenId, bounds, directionInfo,
        screenPropertyChangeType);
    screenSessionManagerClient_->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
    screenSessionManagerClient_->GetPhyScreenProperty(screenId);
    screenSessionManagerClient_->UpdateAvailableArea(screenId, area);
    screenSessionManagerClient_->NotifyFoldToExpandCompletion(foldToExpand);
    screenSessionManagerClient_->GetScreenSnapshot(screenId, scaleX, scaleY);
    sptr<ScreenSession> screenSession1 = screenSessionManagerClient_->GetScreenSession(screenId);
    EXPECT_NE(screenSession1, nullptr);
}

/**
 * @tc.name: ScreenCaptureNotify
 * @tc.desc: ScreenCaptureNotify test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, ScreenCaptureNotify, TestSize.Level1)
{
    ScreenId screenId = 0;
    int32_t uid = 0;
    std::string clientName = "test";
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->ScreenCaptureNotify(screenId, uid, clientName);
}

/**
 * @tc.name: HandleScreenConnection
 * @tc.desc: HandleScreenConnection test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, HandleScreenConnection, TestSize.Level2)
{
    SessionOption option1 = {
        .rsId_ = -1ULL,
    };
    screenSessionManagerClient_->HandleScreenConnection(option1);

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = screenSessionManagerClient_->screenSessionManager_->
        CreateVirtualScreen(virtualOption, nullptr);
    SessionOption option2 = {
        .name_ = "HiCar",
        .screenId_ = screenId,
    };
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->HandleScreenConnection(option2);
}

/**
 * @tc.name: HandleScreenDisconnection
 * @tc.desc: HandleScreenDisconnection test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, HandleScreenDisconnection, TestSize.Level2)
{
    sptr<ScreenSessionManagerClient> client = new ScreenSessionManagerClient();
    ASSERT_TRUE(client != nullptr);

    sptr<ScreenSession> screenSession1 = new ScreenSession(50, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    client->screenSessionMap_[50] = screenSession1;

    SessionOption option = {
        .screenId_ = 50,
    };
    client->HandleScreenDisconnection(option);
    SessionOption option1 = {
        .screenId_ = 51,
    };
    ASSERT_TRUE(client != nullptr);
    client->HandleScreenDisconnection(option1);
}

/**
 * @tc.name: OnCreateScreenSessionOnly
 * @tc.desc: OnCreateScreenSessionOnly test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnCreateScreenSessionOnly, TestSize.Level2)
{
    sptr<ScreenSessionManagerClient> client = sptr<ScreenSessionManagerClient>::MakeSptr();
    client->ConnectToServer();

    sptr<ScreenSession> screenSession1 = sptr<ScreenSession>::MakeSptr(50, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    client->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = nullptr;
    client->screenSessionMap_[51] = screenSession2;

    auto ret = client->OnCreateScreenSessionOnly(50, 50, "test1", true);
    EXPECT_EQ(ret, true);

    ret = client->OnCreateScreenSessionOnly(51, 51, "test2", true);
    EXPECT_EQ(ret, true);

    ret = client->OnCreateScreenSessionOnly(52, 52, "test3", true);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: OnExtendDisplayNodeChange
 * @tc.desc: OnExtendDisplayNodeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnExtendDisplayNodeChange, TestSize.Level2)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->ConnectToServer();

    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node = std::make_shared<RSDisplayNode>(config);
    sptr<ScreenSession> screenSession1 = new ScreenSession(50, 50, "test1", ScreenProperty(), node);
    ASSERT_NE(nullptr, screenSession1);
    screenSessionManagerClient_->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, 51, "test1", ScreenProperty(), node);
    ASSERT_NE(nullptr, screenSession2);
    screenSessionManagerClient_->screenSessionMap_[51] = screenSession2;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, 52, "test1", ScreenProperty(), nullptr);
    ASSERT_NE(nullptr, screenSession3);
    screenSessionManagerClient_->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = nullptr;
    screenSessionManagerClient_->screenSessionMap_[53] = screenSession4;

    auto ret = screenSessionManagerClient_->OnExtendDisplayNodeChange(50, 53);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnExtendDisplayNodeChange(53, 50);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnExtendDisplayNodeChange(50, 52);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnExtendDisplayNodeChange(52, 52);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnExtendDisplayNodeChange(52, 50);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnExtendDisplayNodeChange(51, 50);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: OnMainDisplayNodeChange
 * @tc.desc: OnMainDisplayNodeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnMainDisplayNodeChange, TestSize.Level2)
{
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    auto ret = screenSessionManagerClient_->OnMainDisplayNodeChange(53, 50, 50);
    EXPECT_EQ(ret, false);

    screenSessionManagerClient_->screenSessionManager_ = sptr<ScreenSessionManagerProxyMock>::MakeSptr();
    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node1 = std::make_shared<RSDisplayNode>(config);
    std::shared_ptr<RSDisplayNode> node2 = std::make_shared<RSDisplayNode>(config);
    sptr<ScreenSession> screenSession1 = new ScreenSession(50, 50, "test1", ScreenProperty(), node1);
    ASSERT_NE(nullptr, screenSession1);
    screenSessionManagerClient_->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = new ScreenSession(51, 51, "test1", ScreenProperty(), node2);
    ASSERT_NE(nullptr, screenSession2);
    screenSessionManagerClient_->screenSessionMap_[51] = screenSession2;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, 52, "test1", ScreenProperty(), nullptr);
    ASSERT_NE(nullptr, screenSession3);
    screenSessionManagerClient_->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = nullptr;
    screenSessionManagerClient_->screenSessionMap_[53] = screenSession4;

    ret = screenSessionManagerClient_->OnMainDisplayNodeChange(53, 50, 50);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnMainDisplayNodeChange(50, 52, 52);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnMainDisplayNodeChange(52, 50, 50);
    EXPECT_EQ(ret, false);

    ret = screenSessionManagerClient_->OnMainDisplayNodeChange(52, 52, 52);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SetScreenCombination
 * @tc.desc: SetScreenCombination test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetScreenCombination, TestSize.Level2)
{
    sptr<ScreenSessionManagerClient> client = new ScreenSessionManagerClient();
    ASSERT_TRUE(client != nullptr);
    client->ConnectToServer();

    sptr<ScreenSession> screenSession1 = new ScreenSession(50, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    client->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession2 = nullptr;
    client->screenSessionMap_[51] = screenSession2;

    ScreenCombination combination = ScreenCombination::SCREEN_EXPAND;

    client->SetScreenCombination(51, 50, combination);
    client->SetScreenCombination(50, 51, combination);
    client->SetScreenCombination(50, 50, combination);
    EXPECT_NE(client, nullptr);
}

/**
 * @tc.name: ExtraDestroyScreen
 * @tc.desc: ExtraDestroyScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, ExtraDestroyScreen, TestSize.Level2)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    sptr<ScreenSessionManagerClient> client = new ScreenSessionManagerClient();
    ASSERT_TRUE(client != nullptr);
    client->ConnectToServer();

    ScreenId screenId = 1;
    sptr<ScreenSession> screenSession1 = new ScreenSession(screenId, ScreenProperty(), 0);
    ASSERT_NE(nullptr, screenSession1);
    client->extraScreenSessionMap_.emplace(screenId, screenSession1);

    ScreenId screenId11 = 11;
    client->extraScreenSessionMap_.emplace(screenId11, nullptr);
    client->ExtraDestroyScreen(screenId11);
    EXPECT_TRUE(logMsg.find("extra screenSession is null") != std::string::npos);
}

/**
 * @tc.name: OnDumperClientScreenSessions
 * @tc.desc: OnDumperClientScreenSessions test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnDumperClientScreenSessions, TestSize.Level2)
{
    sptr<ScreenSessionManagerClient> client = new ScreenSessionManagerClient();
    ASSERT_TRUE(client != nullptr);
    client->ConnectToServer();

    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node = std::make_shared<RSDisplayNode>(config);
    sptr<ScreenSession> screenSession1 = new ScreenSession(50, 50, "test1", ScreenProperty(), node);
    ASSERT_NE(nullptr, screenSession1);
    client->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, 52, "test1", ScreenProperty(), nullptr);
    ASSERT_NE(nullptr, screenSession3);
    client->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = nullptr;
    client->screenSessionMap_[53] = screenSession4;

    auto ret = client->OnDumperClientScreenSessions();
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: OnBeforeScreenPropertyChanged
 * @tc.desc: OnBeforeScreenPropertyChanged test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, OnBeforeScreenPropertyChanged, TestSize.Level2)
{
    sptr<ScreenSessionManagerClient> client = new ScreenSessionManagerClient();
    client->ConnectToServer();
    ASSERT_TRUE(client != nullptr);

    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node = std::make_shared<RSDisplayNode>(config);
    sptr<ScreenSession> screenSession1 = new ScreenSession(50, 50, "test1", ScreenProperty(), node);
    ASSERT_NE(nullptr, screenSession1);
    client->screenSessionMap_[50] = screenSession1;

    sptr<ScreenSession> screenSession3 = new ScreenSession(52, 52, "test1", ScreenProperty(), nullptr);
    ASSERT_NE(nullptr, screenSession3);
    client->screenSessionMap_[52] = screenSession3;

    sptr<ScreenSession> screenSession4 = nullptr;
    client->screenSessionMap_[53] = screenSession4;

    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    client->OnBeforeScreenPropertyChanged(foldStatus);
    EXPECT_NE(client, nullptr);
}

/**
 * @tc.name: SetPrimaryDisplaySystemDpi
 * @tc.desc: SetPrimaryDisplaySystemDpi test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetPrimaryDisplaySystemDpi, TestSize.Level2)
{
    sptr<ScreenSessionManagerClient> client = new ScreenSessionManagerClient();
    ASSERT_TRUE(client != nullptr);
    client->ConnectToServer();

    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node = std::make_shared<RSDisplayNode>(config);
    sptr<ScreenSession> screenSession = new ScreenSession(50, 50, "SetPrimaryDisplaySystemDpi", ScreenProperty(), node);
    ASSERT_NE(nullptr, screenSession);
    float dpi = 2.125f;
    screenSession->SetDensityInCurResolution(dpi);
    screenSession->SetIsExtend(false);
    client->screenSessionMap_[0] = screenSession;
    EXPECT_EQ(screenSession->GetDensityInCurResolution(), 2.125f);
    dpi = 2.2f;
    client->SetPrimaryDisplaySystemDpi(dpi);
    EXPECT_EQ(DisplayManager::GetInstance().GetPrimaryDisplaySystemDpi(), 2.2f);
}
/**
 * @tc.name: DisconnectAllExternalScreen
 * @tc.desc: DisconnectAllExternalScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, DisconnectAllExternalScreen, TestSize.Level2)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ASSERT_NE(screenSessionManagerClient_, nullptr);
    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node1 = std::make_shared<RSDisplayNode>(config);
    screenSessionManagerClient_->screenSessionMap_[50] = nullptr;
    screenSessionManagerClient_->DisconnectAllExternalScreen();
    EXPECT_TRUE(logMsg.find("screenSession is nullptr") != std::string::npos);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(50, 50, "test1", ScreenProperty(), node1);
    ASSERT_NE(nullptr, screenSession);
    screenSessionManagerClient_->screenSessionMap_[50] = screenSession;
    screenSession->SetScreenType(ScreenType::REAL);
    screenSession->SetIsExtend(true);
    screenSessionManagerClient_->DisconnectAllExternalScreen();
    EXPECT_TRUE(logMsg.find("disconnect extend screen") != std::string::npos);
}

/**
 * @tc.name: CreateTempScreenSession
 * @tc.desc: CreateTempScreenSession test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, CreateTempScreenSession, TestSize.Level2)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    RSDisplayNodeConfig config;
    std::shared_ptr<RSDisplayNode> node1 = std::make_shared<RSDisplayNode>(config);
    std::shared_ptr<RSDisplayNode> node2 = std::make_shared<RSDisplayNode>(config);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(50, 50, "test1", ScreenProperty(), node1);
    ASSERT_NE(nullptr, screenSession);
    screenSessionManagerClient_->screenSessionMap_[50] = screenSession;
    screenSessionManagerClient_->screenSessionManager_ = sptr<ScreenSessionManagerProxyMock>::MakeSptr();
 
    auto tempScreenSession = screenSessionManagerClient_->CreateTempScreenSession(50, 51, node2);
    ASSERT_NE(nullptr, tempScreenSession);
    tempScreenSession = screenSessionManagerClient_->CreateTempScreenSession(52, 51, node2);
    ASSERT_NE(nullptr, tempScreenSession);
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
}

/**
 * @tc.name: FreezeScreen
 * @tc.desc: FreezeScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, FreezeScreen, TestSize.Level2)
{
    LOG_SetCallback(MyLogCallback);
    logMsg.clear();
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    bool isFreeze = false;
    screenSessionManagerClient_->FreezeScreen(screenId, isFreeze);
    EXPECT_TRUE(logMsg.find("get screen session is null, screenId is 0") != std::string::npos);

    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, screenProperty, screenId);
    screenSessionManagerClient_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    screenSessionManagerClient_->FreezeScreen(screenId, isFreeze);
    screenSessionManagerClient_->screenSessionMap_.clear();
    logMsg.clear();
}

/**
 * @tc.name: GetScreenSnapshotWithAllWindows01
 * @tc.desc: GetScreenSnapshotWithAllWindows01 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSnapshotWithAllWindows01, TestSize.Level2)
{
    LOG_SetCallback(MyLogCallback);
    logMsg.clear();
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    float scaleX = 1.0;
    float scaleY = 1.0;
    bool isNeedCheckDrmAndSurfaceLock = false;
    screenSessionManagerClient_->GetScreenSnapshotWithAllWindows(screenId, scaleX, scaleY,
        isNeedCheckDrmAndSurfaceLock);
    EXPECT_TRUE(logMsg.find("get screen session is null, screenId is 0") != std::string::npos);
    screenSessionManagerClient_->screenSessionMap_.clear();
    logMsg.clear();
}

/**
 * @tc.name: GetScreenSnapshotWithAllWindows02
 * @tc.desc: GetScreenSnapshotWithAllWindows02 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetScreenSnapshotWithAllWindows02, TestSize.Level2)
{
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenProperty screenProperty;
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, screenProperty, screenId);
    screenSessionManagerClient_->screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    float scaleX = 1.0;
    float scaleY = 1.0;
    bool isNeedCheckDrmAndSurfaceLock = false;
    std::shared_ptr<Media::PixelMap> res = screenSessionManagerClient_->GetScreenSnapshotWithAllWindows(screenId,
        scaleX, scaleY, isNeedCheckDrmAndSurfaceLock);
    EXPECT_EQ(res, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: NotifySwitchUserAnimationFinish
 * @tc.desc: NotifySwitchUserAnimationFinish test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, NotifySwitchUserAnimationFinish, TestSize.Level1)
{
    sptr<ScreenSessionManagerClient> client = sptr<ScreenSessionManagerClient>::MakeSptr();
    ASSERT_NE(client, nullptr);
    client->ConnectToServer();
    

    std::string desc1 = "desc1";
    std::string desc2 = "desc2";
    std::string desc3 = "desc3";
    client->NotifySwitchUserAnimationFinish(desc1);

    client->animateFinishDescriptionSet_.insert(desc2);
    client->animateFinishDescriptionSet_.insert(desc3);

    logMsg.clear();
    client->NotifySwitchUserAnimationFinish(desc1);
    EXPECT_TRUE(logMsg.find("not find description in map") != std::string::npos);

    client->NotifySwitchUserAnimationFinish(desc2);
    EXPECT_NE(client->animateFinishNotificationSet_.find(desc2), client->animateFinishNotificationSet_.end());

    logMsg.clear();
    client->NotifySwitchUserAnimationFinish(desc3);
    EXPECT_TRUE(logMsg.find("notify all animate finished") != std::string::npos);
    logMsg.clear();
    EXPECT_TRUE(client->animateFinishNotificationSet_.empty());
    

    client->screenSessionManager_ = nullptr;
    client->NotifySwitchUserAnimationFinish(desc2);
    client->NotifySwitchUserAnimationFinish(desc3);
    EXPECT_TRUE(logMsg.find("screenSessionManager_ is null") != std::string::npos);
    logMsg.clear();
}

/**
 * @tc.name: RegisterSwitchUserAnimationNotification
 * @tc.desc: RegisterSwitchUserAnimationNotification test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, RegisterSwitchUserAnimationNotification, TestSize.Level1)
{
    sptr<ScreenSessionManagerClient> client = sptr<ScreenSessionManagerClient>::MakeSptr();
    ASSERT_NE(client, nullptr);
    client->ConnectToServer();

    std::string desc1 = "desc1";
    std::string desc2 = "desc2";
    client->RegisterSwitchUserAnimationNotification(desc1);
    EXPECT_NE(client->animateFinishDescriptionSet_.find(desc1), client->animateFinishDescriptionSet_.end());
    client->RegisterSwitchUserAnimationNotification(desc1);
    EXPECT_NE(client->animateFinishDescriptionSet_.find(desc1), client->animateFinishDescriptionSet_.end());
    EXPECT_EQ(client->animateFinishDescriptionSet_.size(), 1);
}

/**
 * @tc.name: NotifyIsFullScreenInForceSplitMode
 * @tc.desc: NotifyIsFullScreenInForceSplitMode test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, NotifyIsFullScreenInForceSplitMode, TestSize.Level3)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    screenSessionManagerClient_->screenSessionManager_ = new ScreenSessionManagerProxy(iRemoteObjectMocker);
    screenSessionManagerClient_->NotifyIsFullScreenInForceSplitMode(0, true);
    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    screenSessionManagerClient_->NotifyIsFullScreenInForceSplitMode(0, true);
    EXPECT_TRUE(logMsg.find("screenSessionManager_ is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
@tc.name: HandleSystemKeyboardOffPropertyChange01
@tc.desc: HandleSystemKeyboardOffPropertyChange01 Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleSystemKeyboardOffPropertyChange01, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    SuperFoldStatus currentState = SuperFoldStatus::HALF_FOLDED;
    ScreenProperty screenProperty;
    bool isKeyboardOn = false;
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleSystemKeyboardOffPropertyChange(screenProperty, currentState, isKeyboardOn);
}

/**
@tc.name: HandleSystemKeyboardOffPropertyChange02
@tc.desc: HandleSystemKeyboardOffPropertyChange02 Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleSystemKeyboardOffPropertyChange02, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    SuperFoldStatus currentState = SuperFoldStatus::FOLDED;
    ScreenProperty screenProperty;
    bool isKeyboardOn = true;
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleSystemKeyboardOffPropertyChange(screenProperty, currentState, isKeyboardOn);
}

/**
@tc.name: HandleSystemKeyboardOnPropertyChange01
@tc.desc: HandleSystemKeyboardOnPropertyChange01 Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleSystemKeyboardOnPropertyChange01, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    SuperFoldStatus currentState = SuperFoldStatus::HALF_FOLDED;
    bool isKeyboardOn = false;
    int32_t validHeight = 1620;
    ScreenProperty screenProperty;
    screenProperty.SetIsFakeInUse(true);
    RRect screenBounds;
    screenBounds.rect_.width_ = 1080;
    screenBounds.rect_.height_ = 1920;
    screenProperty.SetBounds(screenBounds);
    screenSession->SetScreenProperty(screenProperty);

    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleSystemKeyboardOnPropertyChange(screenProperty, currentState,
    isKeyboardOn, validHeight);

    EXPECT_EQ(screenSession->GetScreenProperty().GetIsFakeInUse(), false);
    EXPECT_EQ(screenSession->GetPointerActiveWidth(), 1080);
    EXPECT_EQ(screenSession->GetPointerActiveHeight(), 1620);
}

/**
@tc.name: HandleSystemKeyboardOnPropertyChange02
@tc.desc: HandleSystemKeyboardOnPropertyChange02 Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleSystemKeyboardOnPropertyChange02, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    SuperFoldStatus currentState = SuperFoldStatus::FOLDED;
    bool isKeyboardOn = false;
    int32_t validHeight = 1620;
    ScreenProperty screenProperty;
    screenProperty.SetIsFakeInUse(true);
    RRect screenBounds;
    screenBounds.rect_.width_ = 1920;
    screenBounds.rect_.height_ = 1080;
    screenProperty.SetBounds(screenBounds);
    screenSession->SetScreenProperty(screenProperty);

    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleSystemKeyboardOnPropertyChange(screenProperty, currentState,
    isKeyboardOn, validHeight);

    EXPECT_EQ(screenSession->GetScreenProperty().GetIsFakeInUse(), true);
    EXPECT_EQ(screenSession->GetPointerActiveWidth(), 1080);
    EXPECT_EQ(screenSession->GetPointerActiveHeight(), 1620);
}

/**
@tc.name: HandleKeyboardOnPropertyChange01
@tc.desc: HandleKeyboardOnPropertyChange01 Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleKeyboardOnPropertyChange01, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenProperty screenProperty;
    screenProperty.SetIsFakeInUse(true);
    RRect screenBounds;
    screenBounds.rect_.width_ = 100;
    screenBounds.rect_.height_ = 200;
    screenProperty.SetBounds(screenBounds);
    screenSession->SetScreenProperty(screenProperty);
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleKeyboardOnPropertyChange(screenProperty, 300);
    EXPECT_EQ(screenSession->GetScreenProperty().GetIsFakeInUse(), false);
    EXPECT_EQ(screenSession->GetValidWidth(), 100);
    EXPECT_EQ(screenSession->GetValidHeight(), 300);
    EXPECT_EQ(screenSession->GetScreenAreaHeight(), 1608);
}

/**
@tc.name: HandleKeyboardOnPropertyChange02
@tc.desc: HandleKeyboardOnPropertyChange02 Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleKeyboardOnPropertyChange02, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenProperty screenProperty;
    RRect screenBounds;
    screenBounds.rect_.width_ = 100;
    screenBounds.rect_.height_ = 200;
    screenProperty.SetBounds(screenBounds);
    screenSession->SetScreenProperty(screenProperty);
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleKeyboardOnPropertyChange(screenProperty, 300);
    EXPECT_EQ(screenSession->GetValidWidth(), 100);
    EXPECT_EQ(screenSession->GetValidHeight(), 300);
}

/**
@tc.name: HandleKeyboardOffPropertyChange
@tc.desc: HandleKeyboardOffPropertyChange Test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, HandleKeyboardOffPropertyChange, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenProperty screenProperty;
    RRect screenBounds;
    screenBounds.rect_.width_ = 100;
    screenBounds.rect_.height_ = 200;
    screenProperty.SetBounds(screenBounds);
    screenSession->SetScreenProperty(screenProperty);

    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSession->HandleKeyboardOffPropertyChange(screenProperty);

    EXPECT_EQ(screenSession->GetValidWidth(), 100);
    EXPECT_EQ(screenSession->GetValidHeight(), 200);
    EXPECT_EQ(screenSession->GetScreenProperty().GetIsFakeInUse(), true);
    EXPECT_EQ(screenSession->GetScreenAreaHeight(), 3296);
}

/**
@tc.name: OnScreenPropertyChanged
@tc.desc: OnScreenPropertyChanged test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, OnScreenPropertyChanged, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(screenSessionManagerClient_, nullptr);

    RRect bounds;
    bounds.rect_.width_ = 1344;
    bounds.rect_.height_ = 2772;
    float rotation = 0.0;
    sptr<ScreenSession> screenSession1 = sptr<ScreenSession>::MakeSptr(50, ScreenProperty(), 0));
    screenSessionManagerClient_->screenSessionMap_[50] = screenSession1;
    sptr<ScreenSession> screenSession2 = nullptr;
    screenSessionManagerClient_->screenSessionMap_[51] = screenSession2;

    screenSessionManagerClient_->OnScreenPropertyChanged(51, rotation, bounds);
    EXPECT_TRUE(logMsg.find("screenSession is null") != std::string::npos);
    logMsg.clear();

    screenSessionManagerClient_->screenSessionManager_ = nullptr;
    EXPECT_EQ(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->OnScreenPropertyChanged(50, rotation, bounds);
    EXPECT_TRUE(logMsg.find("screenSessionManager_ is null") != std::string::npos);
    logMsg.clear();

    screenSessionManagerClient_->ConnectToServer();
    EXPECT_NE(screenSessionManagerClient_->screenSessionManager_, nullptr);
    screenSessionManagerClient_->screenSessionManager_ = sptr::MakeSptr();
    screenSessionManagerClient_->OnScreenPropertyChanged(50, rotation, bounds);

    screenSessionManagerClient_->currentstate_ = SuperFoldStatus::KEYBOARD;
    screenSessionManagerClient_->OnScreenPropertyChanged(50, rotation, bounds);
    EXPECT_EQ(screenSession1->GetValidWidth(), 1344);
    EXPECT_EQ(screenSession1->GetValidWidth(), 2772);
    screenSessionManagerClient_->currentstate_ = SuperFoldStatus::UNKNOWN;
}

/**
@tc.name: OnPropertyChanged01
@tc.desc: OnPropertyChanged01 test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, OnPropertyChanged01, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_TRUE(screenSessionManagerClient_ != nullptr);

    ScreenId screenId = 1234;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = nullptr;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    screenSessionManagerClient_->screenSessionMap_.insert({screenId, screenSession});
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);
    EXPECT_TRUE(logMsg.find("screenSession is null") != std::string::npos);
    logMsg.clear();

    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::UNDEFINED);
    screenSession = sptr::MakeSptr(screenId, screenProperty, 0);
    screenSessionManagerClient_->screenSessionMap_.erase(screenId);
    screenSessionManagerClient_->screenSessionMap_.insert({screenId, screenSession});
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);
    EXPECT_FALSE(logMsg.find("nothing to handle") != std::string::npos);
    screenSessionManagerClient_->screenSessionMap_.erase(screenId);
    logMsg.clear();
}

/**
@tc.name: OnPropertyChanged02
@tc.desc: OnPropertyChanged02 test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, OnPropertyChanged02, TestSize.Level1)
{
    ScreenId screenId = 1234;
    ScreenProperty screenProperty;
    sptr<ScreenSession> screenSession = nullptr;
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::UNDEFINED;
    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED);
    screenSession = sptr::MakeSptr(screenId, screenProperty, 0);
    screenSessionManagerClient_->screenSessionMap_.erase(screenId);
    screenSessionManagerClient_->screenSessionMap_.insert({screenId, screenSession});
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);
    EXPECT_NE(screenSession->GetScreenProperty().GetIsFakeInUse(), true);

    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);
    EXPECT_EQ(screenSession->GetScreenProperty().GetIsFakeInUse(), false);

    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::KEYBOARD_ON);
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);

    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::KEYBOARD_OFF);
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);

    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON);
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);

    screenProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF);
    screenSessionManagerClient_->OnPropertyChanged(screenId, screenProperty, reason);
    screenSessionManagerClient_->screenSessionMap_.erase(screenId);
}

/**
 * @tc.name: SetInternalClipToBounds
 * @tc.desc: SetInternalClipToBounds test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, SetInternalClipToBounds, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(screenSessionManagerClient_, nullptr);

    RRect bounds;
    sptr<ScreenSession> screenSession1 = sptr<ScreenSession>::MakeSptr(50, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_[50] = screenSession1;

    screenSessionManagerClient_->SetInternalClipToBounds(51, true);
    EXPECT_TRUE(logMsg.find("screenSession is null") != std::string::npos);
    logMsg.clear();

    screenSessionManagerClient_->SetInternalClipToBounds(50, true);
    EXPECT_TRUE(logMsg.find("displayNode cliptobounds set to") != std::string::npos);
    logMsg.clear();

    sptr<ScreenSession> screenSession2 = sptr<ScreenSession>::MakeSptr(51, ScreenProperty(), 0);
    screenSession2->displayNode_ = nullptr;
    screenSessionManagerClient_->screenSessionMap_[51] = screenSession2;
    screenSessionManagerClient_->SetInternalClipToBounds(51, true);
    EXPECT_TRUE(logMsg.find("SetInternalClipToBounds end") != std::string::npos);
    logMsg.clear();

    screenSessionManagerClient_->screenSessionMap_.erase(50);
    screenSessionManagerClient_->screenSessionMap_.erase(51);
}

/**
 * @tc.name: GetSupportsFocus
 * @tc.desc: GetSupportsFocus test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerClientTest, GetSupportsFocus, TestSize.Level1)
{
    sptr<ScreenSessionManagerClient> client = sptr<ScreenSessionManagerClient>::MakeSptr();
    ASSERT_NE(client, nullptr);
    client->ConnectToServer();
    DisplayId displayId = 0;
    auto focus = client->GetSupportsFocus(displayId);
    EXPECT_EQ(focus, true);

    displayId = 10000;
    focus = client->GetSupportsFocus(displayId);
    EXPECT_EQ(focus, true);
}

/**
@tc.name: ProcPropertyChangedForSuperFold
@tc.desc: ProcPropertyChangedForSuperFold test
@tc.type: FUNC
*/
HWTEST_F(ScreenSessionManagerClientTest, ProcPropertyChangedForSuperFold, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ScreenSession session;
    ScreenProperty screenProperty;
    ScreenProperty eventPara;

    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }

    struct TestCase {
        SuperFoldStatusChangeEvents eventType;
        std::string expectedLog;
        bool isInfoLog;
    };

    std::vector testCases = {
        { SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED, "handle extend change to half fold", true },
        { SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED, "handle half fold change to expanded", true },
        { SuperFoldStatusChangeEvents::KEYBOARD_ON, "handle keyboard on", true },
        { SuperFoldStatusChangeEvents::KEYBOARD_OFF, "handle keyboard off", true },
        { SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_ON, "handle system keyboard on", true },
        { SuperFoldStatusChangeEvents::SYSTEM_KEYBOARD_OFF, "handle system keyboard off", true },
        { static_cast(999), "nothing to handle changeEvent=", false },
    };

    eventPara.SetCurrentValidHeight(1080);
    eventPara.SetIsKeyboardOn(true);
    eventPara.SetFoldStatus(SuperFoldStatus::FOLDED);

    for (const auto& tc : testCases) {
        logMsg.clear();
        eventPara.SetSuperFoldStatusChangeEvent(tc.eventType);
        session.ProcPropertyChangedForSuperFold(screenProperty, eventPara);

        if (tc.isInfoLog) {
            EXPECT_NE(logMsg.find(tc.expectedLog), std::string::npos)
                << "Expected log not found for event: " << static_cast<uint32_t>(tc.eventType);
            EXPECT_EQ(logMsg.find("failed"), std::string::npos)
                << "Unexpected error log found for info event: " << static_cast<uint32_t>(tc.eventType);
        } else {
            EXPECT_NE(logMsg.find(tc.expectedLog), std::string::npos)
                << "Expected error log not found for default case";
            EXPECT_NE(logMsg.find("nothing to handle"), std::string::npos) << "Default case log mismatch";
        }
    }

    logMsg.clear();
    LOG_SetCallback(nullptr);
}
} // namespace Rosen
} // namespace OHOS
