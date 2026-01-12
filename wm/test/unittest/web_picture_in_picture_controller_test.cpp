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
#include "picture_in_picture_controller.h"
#include "picture_in_picture_manager.h"
#include "window.h"
#include "wm_common.h"
#include "xcomponent_controller.h"
#include "ability_context_impl.h"
#include "web_picture_in_picture_controller.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MockWindow : public Window {
public:
    MockWindow() {};
    ~MockWindow() {};
    MOCK_METHOD3(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus));
    MOCK_METHOD1(Destroy, WMError(uint32_t reason));
    MOCK_METHOD0(Destroy, WMError());
    MOCK_METHOD0(NotifyPrepareClosePiPWindow, WMError());
    MOCK_METHOD4(SetAutoStartPiP, void(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height));
    MOCK_CONST_METHOD0(GetWindowState, WindowState());
};

class MockXComponentController : public XComponentController {
public:
    MockXComponentController() {}
    ~MockXComponentController() {}
    MOCK_METHOD2(GetGlobalPosition, XComponentControllerErrorCode(float& offsetX, float& offsetY));
    MOCK_METHOD2(GetSize, XComponentControllerErrorCode(float& width, float& height));
    MOCK_METHOD1(SetExtController,
        XComponentControllerErrorCode(std::shared_ptr<XComponentController> xComponentController));
    MOCK_METHOD1(ResetExtController,
        XComponentControllerErrorCode(std::shared_ptr<XComponentController> xComponentController));
};

class WebPictureInPictureControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    PiPConfig config;
};

void WebPictureInPictureControllerTest::SetUpTestCase()
{
}

void WebPictureInPictureControllerTest::TearDownTestCase()
{
}

void WebPictureInPictureControllerTest::SetUp()
{
    uint32_t mainWindowId = 100;
    uint32_t pipTemplateType = 0;
    uint32_t width = 100;
    uint32_t height = 150;
    std::vector<uint32_t> controlGroup = {101};
    int num = 0;
    napi_env env = reinterpret_cast<napi_env>(&num);
    config = {mainWindowId, pipTemplateType, width, height, controlGroup, env};
}

void WebPictureInPictureControllerTest::TearDown()
{
}

namespace {
/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, CreatePictureInPictureWindow, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    auto option = webPipControl->pipOption_;
    webPipControl->pipOption_ = nullptr;
    StartPipType startType = StartPipType::NATIVE_START;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, webPipControl->CreatePictureInPictureWindow(startType));

    webPipControl->pipOption_ = option;
    webPipControl->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, webPipControl->CreatePictureInPictureWindow(startType));

    webPipControl->mainWindow_ = mw;
    EXPECT_CALL(*(mw), GetWindowState()).Times(2).WillOnce(Return(WindowState::STATE_CREATED));
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, webPipControl->CreatePictureInPictureWindow(startType));

    EXPECT_CALL(*(mw), GetWindowState()).Times(2).WillOnce(Return(WindowState::STATE_SHOWN));
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, webPipControl->CreatePictureInPictureWindow(startType));
}

/**
 * @tc.name: StartPictureInPicture
 * @tc.desc: StartPictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, StartPictureInPicture, TestSize.Level1)
{
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    auto option = webPipControl->pipOption_;
    webPipControl->pipOption_ = nullptr;
    StartPipType startType = StartPipType::NATIVE_START;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, webPipControl->StartPictureInPicture(startType));

    webPipControl->pipOption_ = option;
    webPipControl->curState_ = PiPWindowState::STATE_STARTING;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, webPipControl->StartPictureInPicture(startType));

    webPipControl->curState_ = PiPWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, webPipControl->StartPictureInPicture(startType));

    webPipControl->curState_ = PiPWindowState::STATE_UNDEFINED;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, webPipControl->StartPictureInPicture(startType));
}

/**
 * @tc.name: UpdateContentSize
 * @tc.desc: UpdateContentSize
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, UpdateContentSize, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    int32_t width = 0;
    int32_t height = 0;
    webPipControl->UpdateContentSize(width, height);
    height = 150;
    webPipControl->UpdateContentSize(width, height);
    height = 0;
    width = 100;
    webPipControl->UpdateContentSize(width, height);
    height = 150;
    webPipControl->UpdateContentSize(width, height);

    webPipControl->curState_ = PiPWindowState::STATE_UNDEFINED;
    webPipControl->UpdateContentSize(width, height);
    webPipControl->curState_ = PiPWindowState::STATE_STARTED;
    webPipControl->UpdateContentSize(width, height);

    webPipControl->window_ = nullptr;
    webPipControl->UpdateContentSize(width, height);
    webPipControl->window_ = mw;
    webPipControl->UpdateContentSize(width, height);
    EXPECT_EQ(webPipControl->pipOption_->contentWidth_, 100);
    EXPECT_EQ(webPipControl->pipOption_->contentHeight_, 150);
}

/**
 * @tc.name: RestorePictureInPictureWindow
 * @tc.desc: RestorePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, RestorePictureInPictureWindow, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    webPipControl->window_ = mw;
    webPipControl->RestorePictureInPictureWindow();
    EXPECT_EQ(webPipControl->curState_, PiPWindowState::STATE_STOPPED);
}

/**
 * @tc.name: SetXComponentController
 * @tc.desc: SetXComponentController
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, SetXComponentController, TestSize.Level1)
{
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    sptr<MockWindow> mw = new MockWindow();
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    webPipControl->window_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, webPipControl->SetXComponentController(xComponentController));
    webPipControl->window_ = mw;
    EXPECT_EQ(WMError::WM_OK, webPipControl->SetXComponentController(xComponentController));
}

/**
 * @tc.name: GetWebRequestId
 * @tc.desc: GetWebRequestId
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, GetWebRequestId, TestSize.Level1)
{
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    EXPECT_EQ(webPipControl->GetWebRequestId(), 0);
    EXPECT_EQ(webPipControl->GetWebRequestId(), 1);
    webPipControl->webRequestId_ = UINT8_MAX - 1;
    EXPECT_EQ(webPipControl->GetWebRequestId(), UINT8_MAX - 1);
    EXPECT_EQ(webPipControl->GetWebRequestId(), UINT8_MAX);
    EXPECT_EQ(webPipControl->GetWebRequestId(), 0);
}

/**
 * @tc.name: UpdateWinRectByComponent
 * @tc.desc: UpdateWinRectByComponent
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, UpdateWinRectByComponent, TestSize.Level1)
{
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    webPipControl->UpdateWinRectByComponent();
    EXPECT_EQ(webPipControl->windowRect_.width_, 100);
    EXPECT_EQ(webPipControl->windowRect_.height_, 150);

    webPipControl->pipOption_->contentWidth_ = 0;
    webPipControl->pipOption_->contentHeight_ = 0;
    webPipControl->UpdateWinRectByComponent();
    EXPECT_EQ(webPipControl->windowRect_.width_, 0);
    EXPECT_EQ(webPipControl->windowRect_.height_, 0);
    webPipControl->pipOption_->contentWidth_ = 10;
    webPipControl->pipOption_->contentHeight_ = 0;
    webPipControl->UpdateWinRectByComponent();
    EXPECT_EQ(webPipControl->windowRect_.width_, 10);
    EXPECT_EQ(webPipControl->windowRect_.height_, 0);
    webPipControl->UpdateWinRectByComponent();
    webPipControl->pipOption_->contentWidth_ = 0;
    webPipControl->UpdateWinRectByComponent();
    EXPECT_EQ(webPipControl->windowRect_.width_, 0);
    EXPECT_EQ(webPipControl->windowRect_.height_, 0);
    webPipControl->pipOption_->contentWidth_ = 10;
    webPipControl->pipOption_->contentHeight_ = 10;
    webPipControl->UpdateWinRectByComponent();
    EXPECT_EQ(webPipControl->windowRect_.posX_, 0);
    EXPECT_EQ(webPipControl->windowRect_.posY_, 0);
}

/**
 * @tc.name: SetPipParentWindowId
 * @tc.desc: SetPipParentWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WebPictureInPictureControllerTest, SetPipParentWindowId, TestSize.Level1)
{
    auto webPipControl = sptr<WebPictureInPictureController>::MakeSptr(config);
    uint32_t windowId = 10000;
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    bool isPC = multiWindowUIType == "FreeFormMultiWindow";
    if (!isPC) {
        EXPECT_EQ(webPipControl->SetPipParentWindowId(windowId), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
        GTEST_SKIP();
    }
    webPipControl->mainWindow_  = nullptr;
    EXPECT_EQ(webPipControl->SetPipParentWindowId(windowId), WMError::WM_ERROR_PIP_INTERNAL_ERROR);
 
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    webPipControl->mainWindow_ = mw;
    EXPECT_EQ(webPipControl->SetPipParentWindowId(windowId), WMError::WM_ERROR_INVALID_PARAM);
}
}
}
}