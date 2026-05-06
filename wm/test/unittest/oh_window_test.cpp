/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <configuration.h>
#include <gtest/gtest.h>

#include "ability_context_impl.h"
#include "mock_static_call.h"
#include "mock_session.h"
#include "oh_window.h"
#include "singleton_mocker.h"
#include "window_impl.h"
#include "window_scene.h"
#include "window_session_impl.h"

struct Input_TouchEvent {
    int32_t action;
    int32_t id;
    int32_t displayX;
    int32_t displayY;
    int64_t actionTime { -1 };
    int32_t windowId { -1 };
    int32_t displayId { -1 };
};

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<StaticCall, MockStaticCall>;
class OHWindowTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<WindowScene> scene_ = nullptr;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

void OHWindowTest::SetUpTestCase()
{
}

void OHWindowTest::TearDownTestCase()
{
}

void OHWindowTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = new WindowOption();
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _)).Times(1).WillOnce([this, &option](
        const std::string&, sptr<WindowOption>&, std::shared_ptr<AbilityRuntime::Context>) -> sptr<Window> {
        sptr<WindowImpl> window = new WindowImpl(option);
        EXPECT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID, abilityContext_));
        return window;
    });
    DisplayId displayId = 0;
    sptr<IWindowLifeCycle> listener = nullptr;
    scene_ = sptr<WindowScene>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, scene_->Init(displayId, abilityContext_, listener));
}

void OHWindowTest::TearDown()
{
    scene_->GoDestroy();
    scene_ = nullptr;
    abilityContext_ = nullptr;
}

namespace {
void FrameMetricsMeasuredCallback(int32_t windowId, const OH_WindowManager_FrameMetrics* metrics)
{
    (void)windowId;
    if (metrics == nullptr) {
        return;
    }
    bool isFirstDrawFrame = false;
    uint64_t inputHandlingDuration = 0;
    uint64_t layoutMeasureDuration = 0;
    uint64_t vsyncTimestamp = 0;
    (void)OH_WindowManager_FrameMetrics_IsFirstDrawFrame(metrics, &isFirstDrawFrame);
    (void)OH_WindowManager_FrameMetrics_GetInputHandlingDuration(metrics, &inputHandlingDuration);
    (void)OH_WindowManager_FrameMetrics_GetLayoutMeasureDuration(metrics, &layoutMeasureDuration);
    (void)OH_WindowManager_FrameMetrics_GetVsyncTimestamp(metrics, &vsyncTimestamp);
}

void FrameMetricsMeasuredCallback2(int32_t windowId, const OH_WindowManager_FrameMetrics* metrics)
{
    FrameMetricsMeasuredCallback(windowId, metrics);
}

void DensityInfoChangeCallback(int32_t windowId, const OH_WindowManager_DensityInfo* info)
{
    (void)windowId;
    (void)info;
}

/**
 * @tc.name: ShowWindow01
 * @tc.desc: return OK when show window
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, ShowWindow01, TestSize.Level1)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    auto ret = OH_WindowManager_ShowWindow(scene_->GetMainWindow()->GetWindowId());
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
}

/**
 * @tc.name: IsWindowShown01
 * @tc.desc: return OK when window is shown
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, IsWindowShowing01, TestSize.Level1)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    bool isShow;
    auto ret = OH_WindowManager_IsWindowShown(scene_->GetMainWindow()->GetWindowId(), &isShow);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
}

/**
 * @tc.name: GetDensityInfoCopy_InvalidParam
 * @tc.desc: get density info copy invalid param
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, GetDensityInfoCopy_InvalidParam, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    auto ret = OH_WindowManager_GetDensityInfoCopy(scene_->GetMainWindow()->GetWindowId(), nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: RegisterDensityInfoChangeCallback_NullCallback
 * @tc.desc: register density info change callback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, RegisterDensityInfoChangeCallback_NullCallback, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    auto ret = OH_WindowManager_RegisterDensityInfoChangeCallback(scene_->GetMainWindow()->GetWindowId(), nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: UnregisterDensityInfoChangeCallback_NullCallback
 * @tc.desc: unregister density info change callback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, UnregisterDensityInfoChangeCallback_NullCallback, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    auto ret = OH_WindowManager_UnregisterDensityInfoChangeCallback(scene_->GetMainWindow()->GetWindowId(), nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: OH_WindowManager_GetAllWindowLayoutInfoList
 * @tc.desc: OH_WindowManager_GetAllWindowLayoutInfoList test
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, OH_WindowManager_GetAllWindowLayoutInfoList, TestSize.Level0)
{
    int64_t displayId = -1;
    WindowManager_Rect** windowLayoutInfo = nullptr;
    size_t* windowLayoutInfoSize = nullptr;
    auto ret = OH_WindowManager_GetAllWindowLayoutInfoList(displayId, windowLayoutInfo, windowLayoutInfoSize);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
    displayId = 0;
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
    windowLayoutInfo = (WindowManager_Rect**)malloc(sizeof(WindowManager_Rect**));
    windowLayoutInfoSize = (size_t*)malloc(sizeof(size_t*));
    ret = OH_WindowManager_GetAllWindowLayoutInfoList(displayId, windowLayoutInfo, windowLayoutInfoSize);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    OH_WindowManager_ReleaseAllWindowLayoutInfoList(*windowLayoutInfo);
    *windowLayoutInfo = NULL;
    free(windowLayoutInfo);
    windowLayoutInfo = NULL;
    free(windowLayoutInfoSize);
    windowLayoutInfoSize = NULL;
}

/**
 * @tc.name: OH_WindowManager_InjectTouchEvent
 * @tc.desc: OH_WindowManager_InjectTouchEvent test
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, OH_WindowManager_InjectTouchEvent, TestSize.Level0)
{
    int32_t windowId = -1;
    int32_t windowX = 0;
    int32_t windowY = 0;
    auto ret = OH_WindowManager_InjectTouchEvent(windowId, nullptr, windowX, windowY);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
    Input_TouchEvent* touchEvent = OH_Input_CreateTouchEvent();
    ASSERT_NE(nullptr, touchEvent);
    touchEvent->actionTime = 100;
    touchEvent->id = 1;
    touchEvent->action = static_cast<Input_TouchEventAction>(10);
    touchEvent->displayX = 100;
    touchEvent->displayY = 200;
    touchEvent->windowId = -1;
    ret = OH_WindowManager_InjectTouchEvent(windowId, touchEvent, windowX, windowY);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
    windowId = 1;
    ret = OH_WindowManager_InjectTouchEvent(windowId, touchEvent, windowX, windowY);
    EXPECT_EQ(OH_Input_GetTouchEventWindowId(touchEvent), windowId);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    OH_Input_SetTouchEventWindowId(touchEvent, 2);
    ret = OH_WindowManager_InjectTouchEvent(windowId, touchEvent, windowX, windowY);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
    OH_Input_SetTouchEventWindowId(touchEvent, windowId);
    ret = OH_WindowManager_InjectTouchEvent(windowId, touchEvent, windowX, windowY);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    OH_Input_DestroyTouchEvent(&touchEvent);
}

/**
 * @tc.name: OH_WindowManager_GetAllMainWindowInfo
 * @tc.desc: OH_WindowManager_GetAllMainWindowInfo test
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, OH_WindowManager_GetAllMainWindowInfo, TestSize.Level0)
{
    WindowManager_MainWindowInfo** infoListNull = nullptr;
    size_t* mainWindowInfoSizeNull = nullptr;
    auto infoList = (WindowManager_MainWindowInfo**)malloc(sizeof(WindowManager_MainWindowInfo**));
    auto mainWindowInfoSize = (size_t*)malloc(sizeof(size_t*));
    auto ret = OH_WindowManager_GetAllMainWindowInfo(infoListNull, mainWindowInfoSizeNull);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
 
    ret = OH_WindowManager_GetAllMainWindowInfo(infoListNull, mainWindowInfoSize);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
 
    ret = OH_WindowManager_GetAllMainWindowInfo(infoList, mainWindowInfoSizeNull);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
 
    ret = OH_WindowManager_GetAllMainWindowInfo(infoList, mainWindowInfoSize);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    OH_WindowManager_ReleaseAllMainWindowInfo(*infoList);
    *infoList = nullptr;
    free(infoList);
    infoList = nullptr;
    free(mainWindowInfoSize);
    mainWindowInfoSize = nullptr;
}
 
/**
 * @tc.name: OH_WindowManager_GetMainWindowSnapshot
 * @tc.desc: OH_WindowManager_GetMainWindowSnapshot test
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, OH_WindowManager_GetMainWindowSnapshot, TestSize.Level0)
{
    int32_t* windowIdListNull = nullptr;
    size_t windowIdListSize = 1;
    WindowManager_WindowSnapshotConfig config;
    config.useCache = false;
    OH_WindowManager_WindowSnapshotCallback callback = nullptr;
    auto ret = OH_WindowManager_GetMainWindowSnapshot(windowIdListNull, windowIdListSize, config, callback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM), ret);
 
    auto windowIdList = (int32_t*)malloc(sizeof(int32_t*));
    ret = OH_WindowManager_GetMainWindowSnapshot(windowIdList, windowIdListSize, config, callback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    free(windowIdList);
    windowIdList = nullptr;
}

/**
 * @tc.name: RegisterFrameMetricsMeasuredCallback_NullCallback
 * @tc.desc: register frame metrics measured callback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, RegisterFrameMetricsMeasuredCallback_NullCallback, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    int32_t windowId = scene_->GetMainWindow()->GetWindowId();
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: UnregisterFrameMetricsMeasuredCallback_NullCallback
 * @tc.desc: unregister frame metrics measured callback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, UnregisterFrameMetricsMeasuredCallback_NullCallback, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    int32_t windowId = scene_->GetMainWindow()->GetWindowId();
    auto ret = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: UnregisterFrameMetricsMeasuredCallback_InvalidWindow
 * @tc.desc: unregister frame metrics measured callback with invalid window id
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, UnregisterFrameMetricsMeasuredCallback_InvalidWindow, TestSize.Level0)
{
    auto ret = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(-1, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
    ret = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(0, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
}

/**
 * @tc.name: RegisterFrameMetricsMeasuredCallback_InvalidWindow
 * @tc.desc: register frame metrics measured callback with invalid window id
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, RegisterFrameMetricsMeasuredCallback_InvalidWindow, TestSize.Level0)
{
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(-1, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
    ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(0, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
}

/**
 * @tc.name: RegisterUnregisterFrameMetricsMeasuredCallback_Normal
 * @tc.desc: register/unregister frame metrics measured callback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, RegisterUnregisterFrameMetricsMeasuredCallback_Normal, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    int32_t windowId = scene_->GetMainWindow()->GetWindowId();
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    if (ret == static_cast<int32_t>(WindowManager_ErrorCode::OK)) {
        auto ret2 = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
        EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret2);
        auto ret3 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
        EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret3);
        return;
    }
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
}

/**
 * @tc.name: RegisterUnregisterFrameMetricsMeasuredCallback_MultiCallbacks
 * @tc.desc: register/unregister two frame metrics measured callbacks for map erase branches
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, RegisterUnregisterFrameMetricsMeasuredCallback_MultiCallbacks, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    int32_t windowId = scene_->GetMainWindow()->GetWindowId();
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    if (ret != static_cast<int32_t>(WindowManager_ErrorCode::OK)) {
        EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
        return;
    }

    auto ret2 = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback2);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret2);
    auto ret3 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret3);
    auto ret4 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback2);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret4);
    auto ret5 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret5);
}

/**
 * @tc.name: UnregisterFrameMetricsMeasuredCallback_WindowDestroyed
 * @tc.desc: unregister frame metrics measured callback after window destroy
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, UnregisterFrameMetricsMeasuredCallback_WindowDestroyed, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    auto mainWindow = scene_->GetMainWindow();
    ASSERT_NE(nullptr, mainWindow);
    int32_t windowId = mainWindow->GetWindowId();
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    if (ret != static_cast<int32_t>(WindowManager_ErrorCode::OK)) {
        EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
        return;
    }

    EXPECT_EQ(WMError::WM_OK, mainWindow->Destroy());
    auto ret2 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret2);
}

/**
 * @tc.name: UnregisterFrameMetricsMeasuredCallback_WindowDestroyedCallbackNotRegistered
 * @tc.desc: unregister unregistered callback after window destroy
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, UnregisterFrameMetricsMeasuredCallback_WindowDestroyedCallbackNotRegistered, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    auto mainWindow = scene_->GetMainWindow();
    ASSERT_NE(nullptr, mainWindow);
    int32_t windowId = mainWindow->GetWindowId();
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    if (ret != static_cast<int32_t>(WindowManager_ErrorCode::OK)) {
        EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
        return;
    }

    EXPECT_EQ(WMError::WM_OK, mainWindow->Destroy());
    auto ret2 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback2);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret2);
}

/**
 * @tc.name: UnregisterFrameMetricsMeasuredCallback_CallbackNotRegistered
 * @tc.desc: unregister callback not registered while window callback map exists
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, UnregisterFrameMetricsMeasuredCallback_CallbackNotRegistered, TestSize.Level0)
{
    ASSERT_NE(nullptr, scene_);
    ASSERT_NE(nullptr, scene_->GetMainWindow());
    int32_t windowId = scene_->GetMainWindow()->GetWindowId();
    auto ret = OH_WindowManager_RegisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    if (ret != static_cast<int32_t>(WindowManager_ErrorCode::OK)) {
        EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL), ret);
        return;
    }

    auto ret2 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback2);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret2);
    auto ret3 = OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(windowId, FrameMetricsMeasuredCallback);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret3);
}

/**
 * @tc.name: FrameMetricsIsFirstDrawFrame_InvalidParam
 * @tc.desc: OH_WindowManager_FrameMetrics_IsFirstDrawFrame invalid param
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, FrameMetricsIsFirstDrawFrame_InvalidParam, TestSize.Level0)
{
    bool isFirstDrawFrame = false;
    auto ret = OH_WindowManager_FrameMetrics_IsFirstDrawFrame(nullptr, &isFirstDrawFrame);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);

    ret = OH_WindowManager_FrameMetrics_IsFirstDrawFrame(nullptr, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: FrameMetricsGetInputHandlingDuration_InvalidParam
 * @tc.desc: OH_WindowManager_FrameMetrics_GetInputHandlingDuration invalid param
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, FrameMetricsGetInputHandlingDuration_InvalidParam, TestSize.Level0)
{
    uint64_t duration = 0;
    auto ret = OH_WindowManager_FrameMetrics_GetInputHandlingDuration(nullptr, &duration);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);

    ret = OH_WindowManager_FrameMetrics_GetInputHandlingDuration(nullptr, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: FrameMetricsGetLayoutMeasureDuration_InvalidParam
 * @tc.desc: OH_WindowManager_FrameMetrics_GetLayoutMeasureDuration invalid param
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, FrameMetricsGetLayoutMeasureDuration_InvalidParam, TestSize.Level0)
{
    uint64_t duration = 0;
    auto ret = OH_WindowManager_FrameMetrics_GetLayoutMeasureDuration(nullptr, &duration);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);

    ret = OH_WindowManager_FrameMetrics_GetLayoutMeasureDuration(nullptr, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: FrameMetricsGetVsyncTimestamp_InvalidParam
 * @tc.desc: OH_WindowManager_FrameMetrics_GetVsyncTimestamp invalid param
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, FrameMetricsGetVsyncTimestamp_InvalidParam, TestSize.Level0)
{
    uint64_t timestamp = 0;
    auto ret = OH_WindowManager_FrameMetrics_GetVsyncTimestamp(nullptr, &timestamp);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);

    ret = OH_WindowManager_FrameMetrics_GetVsyncTimestamp(nullptr, nullptr);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM), ret);
}

/**
 * @tc.name: FrameMetricsGetters_ValidParam
 * @tc.desc: frame metrics getters valid param
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowTest, FrameMetricsGetters_ValidParam, TestSize.Level0)
{
    struct alignas(8) FrameMetricsLayoutForTest {
        bool firstDrawFrame;
        uint64_t inputHandlingDuration;
        uint64_t layoutMeasureDuration;
        uint64_t vsyncTimestamp;
    };
    FrameMetricsLayoutForTest metricsLayout {
        true,
        101,
        202,
        303,
    };
    auto metrics = reinterpret_cast<const OH_WindowManager_FrameMetrics*>(&metricsLayout);
    bool isFirstDrawFrame = false;
    uint64_t inputHandlingDuration = 0;
    uint64_t layoutMeasureDuration = 0;
    uint64_t vsyncTimestamp = 0;

    auto ret = OH_WindowManager_FrameMetrics_IsFirstDrawFrame(metrics, &isFirstDrawFrame);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_TRUE(isFirstDrawFrame);
    ret = OH_WindowManager_FrameMetrics_GetInputHandlingDuration(metrics, &inputHandlingDuration);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(metricsLayout.inputHandlingDuration, inputHandlingDuration);
    ret = OH_WindowManager_FrameMetrics_GetLayoutMeasureDuration(metrics, &layoutMeasureDuration);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(metricsLayout.layoutMeasureDuration, layoutMeasureDuration);
    ret = OH_WindowManager_FrameMetrics_GetVsyncTimestamp(metrics, &vsyncTimestamp);
    EXPECT_EQ(static_cast<int32_t>(WindowManager_ErrorCode::OK), ret);
    EXPECT_EQ(metricsLayout.vsyncTimestamp, vsyncTimestamp);
}
}
} // namespace Rosen
} // namespace OHOS