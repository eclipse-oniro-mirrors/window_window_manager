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
#include "web_picture_in_picture_controller_interface.h"
#include "native_pip_window_listener.h"
#include "window_manager_hilog.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WebPictureInPictureControllerInterfaceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    int num = 0;
    sptr<WebPictureInPictureControllerInterface> controller = nullptr;
    PiPConfig pipConfig{};
private:
    static constexpr uint32_t mainWindowId = 100;
    static constexpr uint32_t width = 100;
    static constexpr uint32_t height = 150;
};

void WebPictureInPictureControllerInterfaceTest::SetUpTestCase()
{
}

void WebPictureInPictureControllerInterfaceTest::TearDownTestCase()
{
}

void WebPictureInPictureControllerInterfaceTest::SetUp()
{
    pipConfig.mainWindowId = mainWindowId;
    pipConfig.pipTemplateType = 0;
    pipConfig.width = width;
    pipConfig.height = height;
    pipConfig.controlGroup = {101};
    pipConfig.env = reinterpret_cast<napi_env>(&num);
    controller = sptr<WebPictureInPictureControllerInterface>::MakeSptr();
}

void WebPictureInPictureControllerInterfaceTest::TearDown()
{
    pipConfig = {};
    controller = nullptr;
}

void PipStartPipCallback(uint32_t controllerId, uint8_t requestId, uint64_t surfaceId)
{
}

void PipLifeCycleCallback(uint32_t controllerId, PiPState state, int32_t errorCode)
{
}

void PipControlEventCallback(uint32_t controllerId, PiPControlType controlType, PiPControlStatus status)
{
}

void PipResizeCallback(uint32_t controllerId, uint32_t width, uint32_t height, double scale)
{
}
namespace {
/**
 * @tc.name: Create
 * @tc.desc: Create
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, Create, TestSize.Level1)
{
    pipConfig.mainWindowId = 0;
    WMError ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.mainWindowId = 100;
    pipConfig.width = 0;
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.height = 0;
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.width = 100;
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.height = 150;
    pipConfig.env = nullptr;
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    int num = 0;
    pipConfig.env = reinterpret_cast<napi_env>(&num);
    pipConfig.pipTemplateType = 5;
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.pipTemplateType = 0;
    pipConfig.controlGroup = {101, 102, 201, 202};
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.controlGroup = {201};
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    pipConfig.pipTemplateType = 1;
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_OK);
    pipConfig.pipTemplateType = 0;
    pipConfig.controlGroup = {101, 102};
    ret = controller->Create(pipConfig);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: StartPip
 * @tc.desc: StartPip
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, StartPip, TestSize.Level1)
{
    WMError ret = controller->StartPip(0);
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    controller->Create(pipConfig);
    ret = controller->StartPip(0);
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_CREATE_FAILED);
}

/**
 * @tc.name: StopPip
 * @tc.desc: StopPip
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, StopPip, TestSize.Level1)
{
    WMError ret = controller->StopPip();
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    controller->Create(pipConfig);
    ret = controller->StopPip();
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_STATE_ABNORMALLY);
}

/**
 * @tc.name: UpdateContentSize
 * @tc.desc: UpdateContentSize
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UpdateContentSize, TestSize.Level1)
{
    WMError ret =controller->UpdateContentSize(0, 0);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    ret =controller->UpdateContentSize(0, 10);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    ret =controller->UpdateContentSize(10, 0);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    ret =controller->UpdateContentSize(10, 10);
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    controller->Create(pipConfig);
    ret =controller->UpdateContentSize(10, 10);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UpdatePiPControlStatus
 * @tc.desc: UpdatePiPControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UpdatePiPControlStatus, TestSize.Level1)
{
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::PAUSE;
    WMError ret =controller->UpdatePiPControlStatus(controlType, status);
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    controller->Create(pipConfig);
    ret =controller->UpdatePiPControlStatus(controlType, status);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: setPiPControlEnabled
 * @tc.desc: setPiPControlEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, setPiPControlEnabled, TestSize.Level1)
{
    auto controlType = PiPControlType::VIDEO_PREVIOUS;
    WMError ret =controller->setPiPControlEnabled(controlType, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    controller->Create(pipConfig);
    ret = controller->setPiPControlEnabled(controlType, false);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetPipParentWindowId
 * @tc.desc: SetPipParentWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, SetPipParentWindowId, TestSize.Level1)
{
    uint32_t windowId = 200;
    WMError ret =controller->SetPipParentWindowId(windowId);
    EXPECT_EQ(ret, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    controller->Create(pipConfig);
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    if (!(multiWindowUIType == "FreeFormMultiWindow")) {
        EXPECT_EQ(controller->SetPipParentWindowId(windowId), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
        GTEST_SKIP();
    }
    ret = controller->SetPipParentWindowId(windowId);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: RegisterStartPipListener
 * @tc.desc: RegisterStartPipListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, RegisterStartPipListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterStartPipListener(PipStartPipCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->RegisterStartPipListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->RegisterStartPipListener(PipStartPipCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: UnregisterStartPipListener
 * @tc.desc: UnregisterStartPipListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterStartPipListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterStartPipListener(PipStartPipCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterStartPipListener(PipStartPipCallback);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = controller->UnregisterStartPipListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    ret = controller->UnregisterStartPipListener(PipStartPipCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: RegisterLifeCycleListener
 * @tc.desc: RegisterLifeCycleListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, RegisterLifeCycleListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->RegisterLifeCycleListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->RegisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: UnregisterLifeCycleListener
 * @tc.desc: UnregisterLifeCycleListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterLifeCycleListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = controller->UnregisterLifeCycleListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    ret = controller->UnregisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: RegisterControlEventListener
 * @tc.desc: RegisterControlEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, RegisterControlEventListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->RegisterControlEventListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->RegisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: UnregisterControlEventListener
 * @tc.desc: UnregisterControlEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterControlEventListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = controller->UnregisterControlEventListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    ret = controller->UnregisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: RegisterResizeListener
 * @tc.desc: RegisterResizeListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, RegisterResizeListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->RegisterResizeListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->RegisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: UnregisterResizeListener
 * @tc.desc: UnregisterResizeListener
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterResizeListener, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = controller->UnregisterResizeListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    ret = controller->UnregisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: UnregisterAllPiPStart
 * @tc.desc: UnregisterAllPiPStart
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterAllPiPStart, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterStartPipListener(PipStartPipCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterAllPiPStart();
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(controller->startPipCallbackSet_.size(), 0);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterAllPiPStart();
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: UnregisterAllPiPLifecycle
 * @tc.desc: UnregisterAllPiPLifecycle
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterAllPiPLifecycle, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterLifeCycleListener(PipLifeCycleCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterAllPiPLifecycle();
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(controller->lifeCycleCallbackSet_.size(), 0);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterAllPiPLifecycle();
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: UnregisterAllPiPControlObserver
 * @tc.desc: UnregisterAllPiPControlObserver
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterAllPiPControlObserver, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterControlEventListener(PipControlEventCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterAllPiPControlObserver();
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(controller->controlEventCallbackSet_.size(), 0);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterAllPiPControlObserver();
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: UnregisterAllPiPWindowSize
 * @tc.desc: UnregisterAllPiPWindowSize
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnregisterAllPiPWindowSize, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->RegisterResizeListener(PipResizeCallback);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnregisterAllPiPWindowSize();
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(controller->resizeCallbackSet_.size(), 0);

    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnregisterAllPiPWindowSize();
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: SetPipInitialSurfaceRect
 * @tc.desc: SetPipInitialSurfaceRect
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, SetPipInitialSurfaceRect, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->SetPipInitialSurfaceRect(10, 10, 0, 20);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->SetPipInitialSurfaceRect(10, 10, 20, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->SetPipInitialSurfaceRect(10, 10, 0, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = controller->SetPipInitialSurfaceRect(10, 10, 20, 20);
    EXPECT_EQ(WMError::WM_OK, ret);
    controller->sptrWebPipController_ = nullptr;
    ret = controller->SetPipInitialSurfaceRect(10, 10, 20, 20);
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}

/**
 * @tc.name: UnsetPipInitialSurfaceRect
 * @tc.desc: UnsetPipInitialSurfaceRect
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerInterfaceTest, UnsetPipInitialSurfaceRect, TestSize.Level1)
{
    controller->Create(pipConfig);
    WMError ret = controller->SetPipInitialSurfaceRect(10, 10, 20, 20);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = controller->UnsetPipInitialSurfaceRect();
    EXPECT_EQ(WMError::WM_OK, ret);
    controller->sptrWebPipController_ = nullptr;
    ret = controller->UnsetPipInitialSurfaceRect();
    EXPECT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, ret);
}
}
}
}