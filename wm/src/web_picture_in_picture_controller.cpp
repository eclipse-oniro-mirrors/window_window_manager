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

#include "singleton_container.h"
#include "parameters.h"
#include "picture_in_picture_manager.h"
#include "web_picture_in_picture_controller.h"
#include "window_manager_hilog.h"
#include "window_scene_session_impl.h"

namespace OHOS {
namespace Rosen {
namespace {
    const std::string WEB_PIP_CONTENT_PATH = "/system/etc/window/resources/web_pip_content.abc";
}

WebPictureInPictureController::WebPictureInPictureController(const PiPConfig& config)
{
    pipOption_ = sptr<PipOption>::MakeSptr();
    pipOption_->SetPipTemplate(config.pipTemplateType);
    pipOption_->SetContentSize(config.width, config.height);
    pipOption_->SetControlGroup(config.controlGroup);
    mainWindowId_ = config.mainWindowId;
    mainWindow_ = WindowSceneSessionImpl::GetMainWindowWithId(config.mainWindowId);
    env_ = config.env;
    curState_ = PiPWindowState::STATE_UNDEFINED;
    isWeb_ = true;
    weakRef_ = this;
}

WMError WebPictureInPictureController::CreatePictureInPictureWindow(StartPipType startType)
{
    if (pipOption_ == nullptr || mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption or mainWindow is nullptr");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    TLOGI(WmsLogTag::WMS_PIP, "mainWindow:%{public}u, mainWindowState:%{public}u",
        mainWindowId_, mainWindow_->GetWindowState());
    if (mainWindow_->GetWindowState() != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_PIP, "mainWindow is not shown. create failed.");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    UpdateWinRectByComponent();
    auto windowOption = sptr<WindowOption>::MakeSptr();
    windowOption->SetWindowName(PIP_WINDOW_NAME);
    windowOption->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    windowOption->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    windowOption->SetWindowRect(windowRect_);
    windowOption->SetKeepScreenOn(true);
    windowOption->SetTouchable(false);
    WMError errCode = WMError::WM_OK;
    PiPTemplateInfo pipTemplateInfo;
    pipOption_->GetPiPTemplateInfo(pipTemplateInfo);
    auto context = mainWindow_->GetContext();
    SingletonContainer::Get<PiPReporter>().SetCurrentPackageName(context->GetApplicationInfo()->name);
    sptr<Window> window = Window::CreatePiP(windowOption, pipTemplateInfo, context, errCode);
    if (window == nullptr || errCode != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_PIP, "Window create failed, reason: %{public}d", errCode);
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    window_ = window;
    window_->UpdatePiPRect(windowRect_, WindowSizeChangeReason::PIP_START);
    PictureInPictureManager::PutPipControllerInfo(window_->GetWindowId(), this);
    return WMError::WM_OK;
}

WMError WebPictureInPictureController::StartPictureInPicture(StartPipType startType)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption is null");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    if (curState_ == PiPWindowState::STATE_STARTING || curState_ == PiPWindowState::STATE_STARTED) {
        TLOGW(WmsLogTag::WMS_PIP, "pipWindow is starting, state: %{public}u, id: %{public}u, mainWindow: %{public}u",
            curState_, (window_ == nullptr) ? INVALID_WINDOW_ID : window_->GetWindowId(), mainWindowId_);
        SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(startType),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "Pip window is starting");
        return WMError::WM_ERROR_PIP_REPEAT_OPERATION;
    }
    curState_ = PiPWindowState::STATE_STARTING;
    PictureInPictureManager::DoClose(true, true);
    return StartPictureInPictureInner(startType);
}

void WebPictureInPictureController::SetUIContent() const
{
    napi_value storage = nullptr;
    napi_ref storageRef = pipOption_->GetStorageRef();
    if (storageRef != nullptr) {
        napi_get_reference_value(env_, storageRef, &storage);
        TLOGI(WmsLogTag::WMS_PIP, "startPiP with localStorage");
    }
    window_->SetUIContentByAbc(WEB_PIP_CONTENT_PATH, env_, storage, nullptr);
}

void WebPictureInPictureController::UpdateContentSize(int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid size");
        return;
    }
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption is nullptr");
        return;
    }
    pipOption_->SetContentSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    if (curState_ != PiPWindowState::STATE_STARTED) {
        TLOGD(WmsLogTag::WMS_PIP, "UpdateContentSize is disabled when state: %{public}u", curState_);
        return;
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipWindow not exist");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "UpdateContentSize window: %{public}u width:%{public}u height:%{public}u",
        window_->GetWindowId(), width, height);
    Rect rect = {0, 0, width, height};
    window_->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RATIO_CHANGE);
    SingletonContainer::Get<PiPReporter>().ReportPiPRatio(width, height);
}

void WebPictureInPictureController::SetPipInitialSurfaceRect(int32_t positionX, int32_t positionY,
    uint32_t width, uint32_t height)
{
    initPositionX_ = positionX;
    initPositionY_ = positionY;
    initWidth_ = width;
    initHeight_ = height;
}

void WebPictureInPictureController::RestorePictureInPictureWindow()
{
    StopPictureInPicture(true, StopPipType::NULL_STOP, false);
    SingletonContainer::Get<PiPReporter>().ReportPiPRestore();
    TLOGI(WmsLogTag::WMS_PIP, "restore pip main window finished");
}

void WebPictureInPictureController::UpdateWinRectByComponent()
{
    uint32_t contentWidth = 0;
    uint32_t contentHeight = 0;
    pipOption_->GetContentSize(contentWidth, contentHeight);
    windowRect_.width_ = contentWidth;
    windowRect_.height_ = contentHeight;
    if (initWidth_ > 0 && initHeight_ > 0) {
        windowRect_.width_ = initWidth_;
        windowRect_.height_ = initHeight_;
    }
    windowRect_.posX_ = initPositionX_;
    windowRect_.posY_ = initPositionY_;
    TLOGD(WmsLogTag::WMS_PIP, "position width: %{public}u, height: %{public}u, posX: %{public}d, posY: %{public}d",
        windowRect_.width_, windowRect_.height_, windowRect_.posX_, windowRect_.posY_);
}

void WebPictureInPictureController::UpdatePiPSourceRect() const
{
    Rect rect = { 0, 0, 0, 0 };
    TLOGI(WmsLogTag::WMS_PIP, "result rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    window_->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RESTORE);
}

WMError WebPictureInPictureController::SetXComponentController(
    std::shared_ptr<XComponentController> xComponentController)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    pipXComponentController_ = xComponentController;
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr when set XComponentController");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    OnPictureInPictureStart();
    return WMError::WM_OK;
}

uint8_t WebPictureInPictureController::GetWebRequestId()
{
    uint8_t res = webRequestId_;
    if (webRequestId_ == UINT8_MAX) {
        webRequestId_ = 0;
    } else {
        webRequestId_++;
    }
    return res;
}

WMError WebPictureInPictureController::SetPipParentWindowId(uint32_t windowId)
{
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    if (multiWindowUIType != "FreeFormMultiWindow") {
        TLOGE(WmsLogTag::WMS_PIP, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "mainWindow is null");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    auto newMainWindow = WindowSceneSessionImpl::GetMainWindowWithId(windowId);
    if (newMainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "mainWindow not found: %{public}u", windowId);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    mainWindow_ = newMainWindow;
    mainWindowId_ = windowId;
    if (window_ == nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "window is null");
        return WMError::WM_OK;
    }
    TLOGI(WmsLogTag::WMS_PIP, "parentWindowId: %{public}u", windowId);
    return window_->SetPipParentWindowId(windowId);
}
} // namespace Rosen
} // namespace OHOS
