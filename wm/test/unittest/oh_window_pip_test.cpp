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
#include <gtest/gtest.h>

#include <gmock/gmock.h>
#include "parameters.h"
#include "oh_window_pip.h"

#include "oh_window_comm.h"
#include "web_picture_in_picture_controller_interface.h"
#include "window_manager_hilog.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class OHWindowPipTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    PictureInPicture_PipConfig pipConfig;
    uint32_t mainWindowId = 100;
    uint32_t width = 100;
    uint32_t height = 150;
    PictureInPicture_PipTemplateType pipTemplateType = PictureInPicture_PipTemplateType::VIDEO_PLAY;
    PictureInPicture_PipControlType pipControlType = PictureInPicture_PipControlType::VIDEO_PLAY_PAUSE;
    PictureInPicture_PipControlStatus pipControlStatus = PictureInPicture_PipControlStatus::PAUSE;
    PictureInPicture_PipControlGroup controlGroup[1] = {
        PictureInPicture_PipControlGroup::VIDEO_PLAY_VIDEO_PREVIOUS_NEXT };
    uint8_t controlGroupLength = 1;
    int num = 0;
    napi_env env = reinterpret_cast<napi_env>(&num);
};

void OHWindowPipTest::SetUpTestCase()
{
}

void OHWindowPipTest::TearDownTestCase()
{
}

void OHWindowPipTest::SetUp()
{
    OH_PictureInPicture_CreatePipConfig(&pipConfig);
    OH_PictureInPicture_SetPipMainWindowId(pipConfig, 100);
    OH_PictureInPicture_SetPipTemplateType(pipConfig, PictureInPicture_PipTemplateType::VIDEO_PLAY);
    OH_PictureInPicture_SetPipRect(pipConfig, 100, 150);
    OH_PictureInPicture_SetPipControlGroup(pipConfig, controlGroup, 1);
    OH_PictureInPicture_SetPipNapiEnv(pipConfig, env);
}

void OHWindowPipTest::TearDown()
{
    OH_PictureInPicture_DestroyPipConfig(&pipConfig);
}

void PipStartPipCallback(uint32_t controllerId, uint8_t requestId, uint64_t surfaceId)
{
}

void PipLifeCycleCallback(uint32_t controllerId, PictureInPicture_PipState state, int32_t errorCode)
{
}

void PipControlEventCallback(uint32_t controllerId, PictureInPicture_PipControlType controlType,
    PictureInPicture_PipControlStatus status)
{
}

void PipResizeCallback(uint32_t controllerId, uint32_t width, uint32_t height, double scale)
{
}

namespace {
/**
 * @tc.name: OH_PictureInPicture_CreatePipConfig
 * @tc.desc: OH_PictureInPicture_CreatePipConfig
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_CreatePipConfig, TestSize.Level1)
{
    PictureInPicture_PipConfig config;
    int32_t ret = OH_PictureInPicture_CreatePipConfig(nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_CreatePipConfig(&config);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DestroyPipConfig(&config);
}

/**
 * @tc.name: OH_PictureInPicture_DestroyPipConfig
 * @tc.desc: OH_PictureInPicture_DestroyPipConfig
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_DestroyPipConfig, TestSize.Level1)
{
    PictureInPicture_PipConfig config;
    int32_t ret = OH_PictureInPicture_DestroyPipConfig(nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_CreatePipConfig(&config);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_DestroyPipConfig(&config);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipMainWindowId
 * @tc.desc: OH_PictureInPicture_SetPipMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipMainWindowId, TestSize.Level1)
{
    int32_t ret = OH_PictureInPicture_SetPipMainWindowId(pipConfig, mainWindowId);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_SetPipMainWindowId(nullptr, mainWindowId);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipTemplateType
 * @tc.desc: OH_PictureInPicture_SetPipTemplateType
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipTemplateType, TestSize.Level1)
{
    int32_t ret = OH_PictureInPicture_SetPipTemplateType(pipConfig, pipTemplateType);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_SetPipTemplateType(nullptr, pipTemplateType);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipRect
 * @tc.desc: OH_PictureInPicture_SetPipRect
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipRect, TestSize.Level1)
{
    int32_t ret = OH_PictureInPicture_SetPipRect(pipConfig, 10, 10);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_SetPipRect(nullptr, 10, 10);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipControlGroup
 * @tc.desc: OH_PictureInPicture_SetPipControlGroup
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipControlGroup, TestSize.Level1)
{
    int32_t ret = OH_PictureInPicture_SetPipControlGroup(pipConfig, controlGroup, controlGroupLength);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_SetPipControlGroup(pipConfig, nullptr, 0);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_SetPipControlGroup(nullptr, controlGroup, controlGroupLength);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipNapiEnv
 * @tc.desc: OH_PictureInPicture_SetPipNapiEnv
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipNapiEnv, TestSize.Level1)
{
    int32_t ret = OH_PictureInPicture_SetPipNapiEnv(pipConfig, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_SetPipNapiEnv(nullptr, env);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_SetPipNapiEnv(nullptr, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_SetPipNapiEnv(pipConfig, env);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
}

/**
 * @tc.name: OH_PictureInPicture_CreatePip
 * @tc.desc: OH_PictureInPicture_CreatePip
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_CreatePip, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    int32_t ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    ret = OH_PictureInPicture_CreatePip(nullptr, &pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_CreatePip(nullptr, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_CreatePip(pipConfig, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_SetPipRect(pipConfig, 0, 0);
    ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_DeletePip
 * @tc.desc: OH_PictureInPicture_DeletePip
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_DeletePip, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_DeletePip(pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_DeletePip(pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
}

/**
 * @tc.name: OH_PictureInPicture_StartPip
 * @tc.desc: OH_PictureInPicture_StartPip
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_StartPip, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_StartPip(10000);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_StartPip(pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_CREATE_FAILED);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_StopPip
 * @tc.desc: OH_PictureInPicture_StopPip
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_StopPip, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_StopPip(pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_STATE_ABNORMAL);
    ret = OH_PictureInPicture_StartPip(pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_CREATE_FAILED);
    ret = OH_PictureInPicture_StopPip(pipControllerId_);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_PIP_STATE_ABNORMAL);
    ret = OH_PictureInPicture_StopPip(10000);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UpdatePipContentSize
 * @tc.desc: OH_PictureInPicture_UpdatePipContentSize
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UpdatePipContentSize, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_UpdatePipContentSize(10000, 50, 100);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    ret = OH_PictureInPicture_UpdatePipContentSize(pipControllerId_, 50, 100);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UpdatePipControlStatus
 * @tc.desc: OH_PictureInPicture_UpdatePipControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UpdatePipControlStatus, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_UpdatePipControlStatus(10000, pipControlType, pipControlStatus);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    PictureInPicture_PipControlType type = static_cast<PictureInPicture_PipControlType>(10);
    PictureInPicture_PipControlStatus status = static_cast<PictureInPicture_PipControlStatus>(2);
    ret = OH_PictureInPicture_UpdatePipControlStatus(pipControllerId_, type, pipControlStatus);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UpdatePipControlStatus(pipControllerId_, type, status);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UpdatePipControlStatus(pipControllerId_, pipControlType, status);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UpdatePipControlStatus(pipControllerId_, pipControlType, pipControlStatus);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipControlEnabled
 * @tc.desc: OH_PictureInPicture_SetPipControlEnabled
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipControlEnabled, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_SetPipControlEnabled(10000, pipControlType, false);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    PictureInPicture_PipControlType type = static_cast<PictureInPicture_PipControlType>(10);
    ret = OH_PictureInPicture_SetPipControlEnabled(pipControllerId_, type, false);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_SetPipControlEnabled(pipControllerId_, pipControlType, false);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_SetParentWindowId
 * @tc.desc: OH_PictureInPicture_SetParentWindowId
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetParentWindowId, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    uint32_t windowId = 200;
    auto ret = OH_PictureInPicture_SetParentWindowId(pipControllerId_, 0);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_SetParentWindowId(10000, windowId);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);

    ret = OH_PictureInPicture_SetParentWindowId(pipControllerId_, windowId);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_RegisterStartPipCallback
 * @tc.desc: OH_PictureInPicture_RegisterStartPipCallback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_RegisterStartPipCallback, TestSize.Level1)
{
    auto ret = OH_PictureInPicture_RegisterStartPipCallback(10000, PipStartPipCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    uint32_t pipControllerId_ = 0;
    ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_RegisterStartPipCallback(pipControllerId_, PipStartPipCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterStartPipCallback
 * @tc.desc: OH_PictureInPicture_UnregisterStartPipCallback
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterStartPipCallback, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret =  OH_PictureInPicture_UnregisterStartPipCallback(pipControllerId_ + 1, PipStartPipCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterStartPipCallback(pipControllerId_, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterStartPipCallback(pipControllerId_, PipStartPipCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_RegisterStartPipCallback(pipControllerId_, PipStartPipCallback);
    ret = OH_PictureInPicture_UnregisterStartPipCallback(pipControllerId_, PipStartPipCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterAllStartPipCallbacks
 * @tc.desc: OH_PictureInPicture_UnregisterAllStartPipCallbacks
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterAllStartPipCallbacks, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_UnregisterAllStartPipCallbacks(pipControllerId_ + 1);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterAllStartPipCallbacks(pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_RegisterLifecycleListener
 * @tc.desc: OH_PictureInPicture_RegisterLifecycleListener
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_RegisterLifecycleListener, TestSize.Level1)
{
    auto ret = OH_PictureInPicture_RegisterLifecycleListener(10000, PipLifeCycleCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    uint32_t pipControllerId_ = 0;
    ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_RegisterLifecycleListener(pipControllerId_, PipLifeCycleCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterLifecycleListener
 * @tc.desc: OH_PictureInPicture_UnregisterLifecycleListener
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterLifecycleListener, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret =  OH_PictureInPicture_UnregisterLifecycleListener(pipControllerId_ + 1, PipLifeCycleCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterLifecycleListener(pipControllerId_, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterLifecycleListener(pipControllerId_, PipLifeCycleCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_RegisterLifecycleListener(pipControllerId_, PipLifeCycleCallback);
    ret = OH_PictureInPicture_UnregisterLifecycleListener(pipControllerId_, PipLifeCycleCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterAllLifecycleListeners
 * @tc.desc: OH_PictureInPicture_UnregisterAllLifecycleListeners
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterAllLifecycleListeners, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_UnregisterAllLifecycleListeners(pipControllerId_ + 1);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterAllLifecycleListeners(pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_RegisterControlEventListener
 * @tc.desc: OH_PictureInPicture_RegisterControlEventListener
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_RegisterControlEventListener, TestSize.Level1)
{
    auto ret = OH_PictureInPicture_RegisterControlEventListener(10000, PipControlEventCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    uint32_t pipControllerId_ = 0;
    ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_RegisterControlEventListener(pipControllerId_, PipControlEventCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterControlEventListener
 * @tc.desc: OH_PictureInPicture_UnregisterControlEventListener
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterControlEventListener, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret =  OH_PictureInPicture_UnregisterControlEventListener(pipControllerId_ + 1, PipControlEventCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterControlEventListener(pipControllerId_, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterControlEventListener(pipControllerId_, PipControlEventCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_RegisterControlEventListener(pipControllerId_, PipControlEventCallback);
    ret = OH_PictureInPicture_UnregisterControlEventListener(pipControllerId_, PipControlEventCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterAllControlEventListeners
 * @tc.desc: OH_PictureInPicture_UnregisterAllControlEventListeners
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterAllControlEventListeners, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_UnregisterAllControlEventListeners(pipControllerId_ + 1);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterAllControlEventListeners(pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_RegisterResizeListener
 * @tc.desc: OH_PictureInPicture_RegisterResizeListener
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_RegisterResizeListener, TestSize.Level1)
{
    auto ret = OH_PictureInPicture_RegisterResizeListener(10000, PipResizeCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    uint32_t pipControllerId_ = 0;
    ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_RegisterResizeListener(pipControllerId_, PipResizeCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterResizeListener
 * @tc.desc: OH_PictureInPicture_UnregisterResizeListener
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterResizeListener, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret =  OH_PictureInPicture_UnregisterResizeListener(pipControllerId_ + 1, PipResizeCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterResizeListener(pipControllerId_, nullptr);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterResizeListener(pipControllerId_, PipResizeCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    OH_PictureInPicture_RegisterResizeListener(pipControllerId_, PipResizeCallback);
    ret = OH_PictureInPicture_UnregisterResizeListener(pipControllerId_, PipResizeCallback);
    EXPECT_EQ(ret, WindowManager_ErrorCode::OK);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnregisterAllResizeListeners
 * @tc.desc: OH_PictureInPicture_UnregisterAllResizeListeners
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnregisterAllResizeListeners, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_UnregisterAllResizeListeners(pipControllerId_ + 1);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_UnregisterAllResizeListeners(pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_SetAutoStartEnabled
 * @tc.desc: OH_PictureInPicture_SetAutoStartEnabled
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetAutoStartEnabled, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_SetAutoStartEnabled(pipControllerId_ + 1, true);
    EXPECT_EQ(ret, WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM);
    ret = OH_PictureInPicture_SetAutoStartEnabled(pipControllerId_, true);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_SetPipInitialSurfaceRect
 * @tc.desc: OH_PictureInPicture_SetPipInitialSurfaceRect
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_SetPipInitialSurfaceRect, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_SetPipInitialSurfaceRect(pipControllerId_ + 1, 10, 10, 100, 100);
    EXPECT_EQ(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM, ret);
    ret = OH_PictureInPicture_SetPipInitialSurfaceRect(pipControllerId_, 10, 10, 100, 100);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}

/**
 * @tc.name: OH_PictureInPicture_UnsetPipInitialSurfaceRect
 * @tc.desc: OH_PictureInPicture_UnsetPipInitialSurfaceRect
 * @tc.type: FUNC
 */
HWTEST_F(OHWindowPipTest, OH_PictureInPicture_UnsetPipInitialSurfaceRect, TestSize.Level1)
{
    uint32_t pipControllerId_ = 0;
    auto ret = OH_PictureInPicture_CreatePip(pipConfig, &pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    ret = OH_PictureInPicture_UnsetPipInitialSurfaceRect(pipControllerId_ + 1);
    EXPECT_EQ(WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM, ret);
    ret = OH_PictureInPicture_UnsetPipInitialSurfaceRect(pipControllerId_);
    EXPECT_EQ(WindowManager_ErrorCode::OK, ret);
    OH_PictureInPicture_DeletePip(pipControllerId_);
}
}
}
}