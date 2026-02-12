/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "picture_in_picture_controller.h"
#include "picture_in_picture_manager.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
    const std::string PIP_CONTENT_PATH = "/system/etc/window/resources/pip_content.abc";
    const std::string DESTROY_TIMEOUT_TASK = "PipDestroyTimeout";
    const std::string STATE_CHANGE = "stateChange";
    const std::string UPDATE_NODE = "nodeUpdate";
    const int32_t INVALID_HANDLE_ID = -1;
}

PictureInPictureController::PictureInPictureController(sptr<PipOption> pipOption, sptr<Window> mainWindow,
    uint32_t windowId, napi_env env)
{
    pipOption_ = pipOption;
    mainWindow_ = mainWindow;
    mainWindowId_ = windowId;
    env_ = env;
    curState_ = PiPWindowState::STATE_UNDEFINED;
    weakRef_ = this;
}

PictureInPictureController::~PictureInPictureController()
{
    if (!isAutoStartEnabled_) {
        return;
    }
    PictureInPictureManager::DetachAutoStartController(handleId_, weakRef_);
}

WMError PictureInPictureController::CreatePictureInPictureWindow(StartPipType startType)
{
    if (pipOption_ == nullptr || pipOption_->GetContext() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Create pip failed, invalid pipOption");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    mainWindowXComponentController_ = pipOption_->GetXComponentController();
    if ((mainWindowXComponentController_ == nullptr && !IsTypeNodeEnabled()) || mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "mainWindowXComponentController or mainWindow is nullptr");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    TLOGI(WmsLogTag::WMS_PIP, "mainWindow:%{public}u, mainWindowState:%{public}u",
        mainWindowId_, mainWindow_->GetWindowState());
    mainWindowLifeCycleListener_ = sptr<PictureInPictureController::WindowLifeCycleListener>::MakeSptr();
    mainWindow_->RegisterLifeCycleListener(mainWindowLifeCycleListener_);
    if (startType != StartPipType::AUTO_START && mainWindow_->GetWindowState() != WindowState::STATE_SHOWN) {
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
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(pipOption_->GetContext());
    const std::shared_ptr<AbilityRuntime::Context>& abilityContext = context->lock();
    SingletonContainer::Get<PiPReporter>().SetCurrentPackageName(abilityContext->GetApplicationInfo()->name);
    sptr<Window> window = Window::CreatePiP(windowOption, pipTemplateInfo, context->lock(), errCode);
    if (window == nullptr || errCode != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_PIP, "Window create failed, reason: %{public}d", errCode);
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    window_ = window;
    window_->UpdatePiPRect(windowRect_, WindowSizeChangeReason::PIP_START);
    PictureInPictureManager::PutPipControllerInfo(window_->GetWindowId(), this);
    return WMError::WM_OK;
}

WMError PictureInPictureController::StartPictureInPicture(StartPipType startType)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    if (pipOption_ == nullptr || pipOption_->GetContext() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption is null or Get PictureInPictureOption failed");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    if (curState_ == PiPWindowState::STATE_STARTING || curState_ == PiPWindowState::STATE_STARTED) {
        TLOGW(WmsLogTag::WMS_PIP, "pipWindow is starting, state: %{public}u, id: %{public}u, mainWindow: %{public}u",
            curState_, (window_ == nullptr) ? INVALID_WINDOW_ID : window_->GetWindowId(), mainWindowId_);
        SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(startType),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "Pip window is starting");
        return WMError::WM_ERROR_PIP_REPEAT_OPERATION;
    }
    if (!IsPullPiPAndHandleNavigation()) {
        TLOGE(WmsLogTag::WMS_PIP, "Navigation operate failed");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    curState_ = PiPWindowState::STATE_STARTING;
    if (PictureInPictureManager::HasActiveController() && !PictureInPictureManager::IsActiveController(weakRef_)) {
        // if current controller is not the active one, but belongs to the same mainWindow, reserve pipWindow
        if (PictureInPictureManager::IsAttachedToSameWindow(mainWindowId_)) {
            window_ = PictureInPictureManager::GetCurrentWindow();
            if (window_ == nullptr) {
                TLOGE(WmsLogTag::WMS_PIP, "Reuse pipWindow failed");
                curState_ = PiPWindowState::STATE_UNDEFINED;
                return WMError::WM_ERROR_PIP_CREATE_FAILED;
            }
            TLOGI(WmsLogTag::WMS_PIP, "Reuse pipWindow: %{public}u as attached to the same mainWindow: %{public}u",
                window_->GetWindowId(), mainWindowId_);
            PictureInPictureManager::DoClose(false, false);
            mainWindowXComponentController_ = IsTypeNodeEnabled() ? nullptr : pipOption_->GetXComponentController();
            UpdateWinRectByComponent();
            UpdateContentSize(windowRect_.width_, windowRect_.height_);
            PictureInPictureManager::PutPipControllerInfo(window_->GetWindowId(), this);
            WMError err = ShowPictureInPictureWindow(startType);
            if (err != WMError::WM_OK) {
                curState_ = PiPWindowState::STATE_UNDEFINED;
            } else {
                curState_ = PiPWindowState::STATE_STARTED;
            }
            return err;
        }
        // otherwise, stop the previous one
        PictureInPictureManager::DoClose(true, true);
    }

    WMError errCode = StartPictureInPictureInner(startType);
    if (errCode != WMError::WM_OK) {
        DeletePIPMode();
    }
    return errCode;
}

void PictureInPictureController::SetAutoStartEnabled(bool enable)
{
    TLOGI(WmsLogTag::WMS_PIP, "enable: %{public}u, mainWindow: %{public}u", enable, mainWindowId_);
    isAutoStartEnabled_ = enable;
    if (mainWindow_ == nullptr) {
        return;
    }
    if (!pipOption_) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption is null");
        return;
    }
    uint32_t priority = pipOption_->GetPipPriority(pipOption_->GetPipTemplate());
    uint32_t contentWidth = 0;
    uint32_t contentHeight = 0;
    pipOption_->GetContentSize(contentWidth, contentHeight);
    if (isAutoStartEnabled_) {
        // cache navigation here as we cannot get containerId while BG
        if (!IsPullPiPAndHandleNavigation()) {
            TLOGE(WmsLogTag::WMS_PIP, "Navigation operate failed");
            isAutoStartEnabled_ = false;
            mainWindow_->SetAutoStartPiP(false, priority, contentWidth, contentHeight);
            return;
        }
        mainWindow_->SetAutoStartPiP(true, priority, contentWidth, contentHeight);
        PictureInPictureManager::AttachAutoStartController(handleId_, weakRef_);
    } else {
        mainWindow_->SetAutoStartPiP(false, priority, contentWidth, contentHeight);
        PictureInPictureManager::DetachAutoStartController(handleId_, weakRef_);
        if (IsTypeNodeEnabled()) {
            TLOGI(WmsLogTag::WMS_PIP, "typeNode enabled");
            return;
        }
        std::string navId = pipOption_->GetNavigationId();
        if (!navId.empty()) {
            auto navController = NavigationController::GetNavigationController(mainWindow_->GetUIContent(), navId);
            if (navController) {
                navController->DeletePIPMode(handleId_);
                TLOGI(WmsLogTag::WMS_PIP, "Delete pip mode id: %{public}d", handleId_);
            }
        }
    }
}

void PictureInPictureController::IsAutoStartEnabled(bool& enable) const
{
    enable = isAutoStartEnabled_;
}

void PictureInPictureController::SetUIContent() const
{
    napi_value storage = nullptr;
    std::shared_ptr<NativeReference> storageRef = pipOption_->GetStorageRef();
    if (storageRef != nullptr) {
        storage = storageRef->GetNapiValue();
        TLOGI(WmsLogTag::WMS_PIP, "startPiP with localStorage");
    }
    window_->SetUIContentByAbc(PIP_CONTENT_PATH, env_, storage, nullptr);
}

void PictureInPictureController::UpdateContentSize(int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid size");
        return;
    }
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption_ is nullptr");
        return;
    }
    if (mainWindow_ != nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "mainWindow width:%{public}u height:%{public}u", width, height);
        uint32_t priority = pipOption_->GetPipPriority(pipOption_->GetPipTemplate());
        uint32_t contentWidth = static_cast<uint32_t>(width);
        uint32_t contentHeight = static_cast<uint32_t>(height);
        mainWindow_->SetAutoStartPiP(isAutoStartEnabled_, priority, contentWidth, contentHeight);
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
    if (mainWindowXComponentController_ && !IsTypeNodeEnabled()) {
        float posX = 0;
        float posY = 0;
        float newWidth = 0;
        float newHeight = 0;
        mainWindowXComponentController_->GetGlobalPosition(posX, posY);
        mainWindowXComponentController_->GetSize(newWidth, newHeight);
        bool isSizeChange = IsContentSizeChanged(newWidth, newHeight, posX, posY);
        if (isSizeChange) {
            Rect r = {static_cast<int32_t>(posX), static_cast<int32_t>(posY),
                static_cast<uint32_t>(newWidth), static_cast<uint32_t>(newHeight)};
            window_->UpdatePiPRect(r, WindowSizeChangeReason::TRANSFORM);
        }
    }
    TLOGI(WmsLogTag::WMS_PIP, "window: %{public}u width:%{public}u height:%{public}u",
        window_->GetWindowId(), width, height);
    Rect rect = {0, 0, width, height};
    window_->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RATIO_CHANGE);
    SingletonContainer::Get<PiPReporter>().ReportPiPRatio(width, height);
}

void PictureInPictureController::UpdateContentNodeRef(std::shared_ptr<NativeReference> nodeRef)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "option is null");
        SingletonContainer::Get<PiPReporter>().ReportPiPUpdateContent(static_cast<int32_t>(IsTypeNodeEnabled()),
            0, PipConst::FAILED, "option is null");
        return;
    }
    pipOption_->SetTypeNodeRef(nodeRef);
    if (IsTypeNodeEnabled()) {
        NotifyNodeUpdate(nodeRef);
        return;
    }
    ResetExtController();
    NotifyNodeUpdate(nodeRef);
    if (isAutoStartEnabled_) {
        std::string navId = pipOption_->GetNavigationId();
        if (!navId.empty()) {
            auto navController = NavigationController::GetNavigationController(mainWindow_->GetUIContent(), navId);
            if (navController) {
                navController->DeletePIPMode(handleId_);
                TLOGI(WmsLogTag::WMS_PIP, "Delete pip mode id: %{public}d", handleId_);
            }
        }
    }
    pipOption_->SetTypeNodeEnabled(true);
}

void PictureInPictureController::NotifyNodeUpdate(std::shared_ptr<NativeReference> nodeRef)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (nodeRef == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid nodeRef");
        SingletonContainer::Get<PiPReporter>().ReportPiPUpdateContent(static_cast<int32_t>(IsTypeNodeEnabled()),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "invalid nodeRef");
        return;
    }
    if (PictureInPictureManager::IsActiveController(weakRef_)) {
        std::shared_ptr<NativeReference> updateNodeCallbackRef = GetPipContentCallbackRef(UPDATE_NODE);
        if (updateNodeCallbackRef == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "updateNodeCallbackRef is null");
            SingletonContainer::Get<PiPReporter>().ReportPiPUpdateContent(static_cast<int32_t>(IsTypeNodeEnabled()),
                pipOption_->GetPipTemplate(), PipConst::FAILED, "updateNodeCallbackRef is null");
            return;
        }
        napi_value typeNode = nodeRef->GetNapiValue();
        napi_value value[] = { typeNode };
        CallJsFunction(env_, updateNodeCallbackRef->GetNapiValue(), value, 1);
        SingletonContainer::Get<PiPReporter>().ReportPiPUpdateContent(static_cast<int32_t>(IsTypeNodeEnabled()),
            pipOption_->GetPipTemplate(), PipConst::PIP_SUCCESS, "updateNode success");
    }
}

void PictureInPictureController::PrepareSource()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (IsTypeNodeEnabled()) {
        TLOGI(WmsLogTag::WMS_PIP, "typeNode enabled");
        return;
    }
    if (mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "mainWindow is nullptr");
        return;
    }
    std::string navId = pipOption_->GetNavigationId();
    if (navId != "") {
        auto navController = NavigationController::GetNavigationController(mainWindow_->GetUIContent(), navId);
        if (navController) {
            navController->PushInPIP(handleId_);
            TLOGI(WmsLogTag::WMS_PIP, "Push in pip handleId: %{public}d", handleId_);
        } else {
            TLOGE(WmsLogTag::WMS_PIP, "navController is nullptr");
        }
    }
}

void PictureInPictureController::RestorePictureInPictureWindow()
{
    StopPictureInPicture(true, StopPipType::NULL_STOP, true);
    SingletonContainer::Get<PiPReporter>().ReportPiPRestore();
    TLOGI(WmsLogTag::WMS_PIP, "restore pip main window finished");
}

void PictureInPictureController::UpdateWinRectByComponent()
{
    if (IsTypeNodeEnabled()) {
        uint32_t contentWidth = 0;
        uint32_t contentHeight = 0;
        pipOption_->GetContentSize(contentWidth, contentHeight);
        if (contentWidth == 0 || contentHeight == 0) {
            contentWidth = PipConst::DEFAULT_ASPECT_RATIOS[0];
            contentHeight = PipConst::DEFAULT_ASPECT_RATIOS[1];
        }
        windowRect_.posX_ = 0;
        windowRect_.posY_ = 0;
        windowRect_.width_ = contentWidth;
        windowRect_.height_ = contentHeight;
        return;
    }
    if (!mainWindowXComponentController_) {
        TLOGE(WmsLogTag::WMS_PIP, "main window xComponent not set");
        return;
    }
    float posX = 0;
    float posY = 0;
    float width = 0;
    float height = 0;
    mainWindowXComponentController_->GetGlobalPosition(posX, posY);
    mainWindowXComponentController_->GetSize(width, height);
    windowRect_.width_ = static_cast<uint32_t>(width);
    windowRect_.height_ = static_cast<uint32_t>(height);
    if (windowRect_.width_ == 0 || windowRect_.height_ == 0) {
        uint32_t contentWidth = 0;
        uint32_t contentHeight = 0;
        pipOption_->GetContentSize(contentWidth, contentHeight);
        windowRect_.width_ = contentWidth;
        windowRect_.height_ = contentHeight;
    }
    windowRect_.posX_ = static_cast<int32_t>(posX);
    windowRect_.posY_ = static_cast<int32_t>(posY);
    TLOGD(WmsLogTag::WMS_PIP, "position width: %{public}u, height: %{public}u, posX: %{public}d, posY: %{public}d",
        windowRect_.width_, windowRect_.height_, windowRect_.posX_, windowRect_.posY_);
}

void PictureInPictureController::UpdatePiPSourceRect() const
{
    if (IsTypeNodeEnabled() && window_ != nullptr) {
        Rect rect = {0, 0, 0, 0};
        TLOGI(WmsLogTag::WMS_PIP, "use typeNode, unable to locate source rect");
        window_->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RESTORE);
        return;
    }
    if (mainWindowXComponentController_ == nullptr || window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "xcomponent controller not valid");
        return;
    }
    float posX = 0;
    float posY = 0;
    float width = 0;
    float height = 0;
    mainWindowXComponentController_->GetGlobalPosition(posX, posY);
    mainWindowXComponentController_->GetSize(width, height);
    Rect rect = { posX, posY, width, height };
    TLOGI(WmsLogTag::WMS_PIP, "result rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    window_->UpdatePiPRect(rect, WindowSizeChangeReason::PIP_RESTORE);
}

void PictureInPictureController::ResetExtController()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    if (IsTypeNodeEnabled()) {
        TLOGI(WmsLogTag::WMS_PIP, "skip resetExtController as nodeController enabled");
        return;
    }
    if (mainWindowXComponentController_ == nullptr || pipXComponentController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "error when resetExtController, one of the xComponentController is null");
        return;
    }
    XComponentControllerErrorCode errorCode =
        mainWindowXComponentController_->ResetExtController(pipXComponentController_);
    if (errorCode != XComponentControllerErrorCode::XCOMPONENT_CONTROLLER_NO_ERROR) {
        TLOGE(WmsLogTag::WMS_PIP, "swap xComponent failed, errorCode: %{public}u", errorCode);
    }
}

WMError PictureInPictureController::SetXComponentController(std::shared_ptr<XComponentController> xComponentController)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    if (IsTypeNodeEnabled()) {
        TLOGI(WmsLogTag::WMS_PIP, "skip as nodeController enabled");
        return WMError::WM_OK;
    }
    pipXComponentController_ = xComponentController;
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr when set XComponentController");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    if (mainWindowXComponentController_ == nullptr || pipXComponentController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "error when setXController, one of the xComponentController is null");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    XComponentControllerErrorCode errorCode =
        mainWindowXComponentController_->SetExtController(pipXComponentController_);
    if (errorCode != XComponentControllerErrorCode::XCOMPONENT_CONTROLLER_NO_ERROR) {
        TLOGE(WmsLogTag::WMS_PIP, "swap xComponent failed, errorCode: %{public}u", errorCode);
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    OnPictureInPictureStart();
    return WMError::WM_OK;
}

WMError PictureInPictureController::RegisterPipContentListenerWithType(const std::string& type,
    std::shared_ptr<NativeReference> callbackRef)
{
    TLOGI(WmsLogTag::WMS_PIP, "Register type:%{public}s", type.c_str());
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Get PictureInPicture option failed");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    pipOption_->RegisterPipContentListenerWithType(type, callbackRef);
    return WMError::WM_OK;
}

WMError PictureInPictureController::UnRegisterPipContentListenerWithType(const std::string& type)
{
    TLOGI(WmsLogTag::WMS_PIP, "Unregister type:%{public}s", type.c_str());
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Get PictureInPicture option failed");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    pipOption_->UnRegisterPipContentListenerWithType(type);
    return WMError::WM_OK;
}

std::shared_ptr<NativeReference> PictureInPictureController::GetPipContentCallbackRef(const std::string& type)
{
    return pipOption_ == nullptr ? nullptr : pipOption_->GetPipContentCallbackRef(type);
}

void PictureInPictureController::NotifyStateChangeInner(PiPState state)
{
    NotifyStateChangeInner(env_, state);
}

void PictureInPictureController::NotifyStateChangeInner(napi_env env, PiPState state)
{
    std::shared_ptr<NativeReference> innerCallbackRef = GetPipContentCallbackRef(STATE_CHANGE);
    if (innerCallbackRef == nullptr) {
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        return;
    }
    napi_value value[] = {AbilityRuntime::CreateJsValue(env, static_cast<uint32_t>(state))};
    CallJsFunction(env, innerCallbackRef->GetNapiValue(), value, 1);
    napi_close_handle_scope(env, scope);
}

bool PictureInPictureController::IsTypeNodeEnabled() const
{
    return pipOption_ != nullptr ? pipOption_->IsTypeNodeEnabled() : false;
}

bool PictureInPictureController::IsPullPiPAndHandleNavigation()
{
    if (IsTypeNodeEnabled()) {
        TLOGI(WmsLogTag::WMS_PIP, "App use typeNode");
        return true;
    }
    if (pipOption_->GetNavigationId() == "") {
        TLOGI(WmsLogTag::WMS_PIP, "App not use navigation");
        return true;
    }
    if (mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Main window init error");
        return false;
    }
    std::string navId = pipOption_->GetNavigationId();
    int32_t pipOptionHandleId = pipOption_->GetHandleId();
    auto navController = GetNavigationController(navId);
    if (navController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Get navController error");
        return false;
    }

    if (pipOptionHandleId != INVALID_HANDLE_ID) {
        navController->SetInPIPMode(pipOptionHandleId);
        handleId_ = pipOptionHandleId;
        TLOGI(WmsLogTag::WMS_PIP, "SetInPIPMode handleId_: %{public}d, top result: %{public}d",
            handleId_, navController->GetTopHandle());
        return true;
    }

    if (!navController->IsNavDestinationInTopStack()) {
        TLOGE(WmsLogTag::WMS_PIP, "Top is not navDestination");
        return false;
    }

    TLOGI(WmsLogTag::WMS_PIP, "IsPullPiPAndHandleNavigation IsNavDestinationInTopStack");
    handleId_ = navController->GetTopHandle();
    if (handleId_ == INVALID_HANDLE_ID) {
        TLOGE(WmsLogTag::WMS_PIP, "Get top handle error");
        return false;
    }
    if (firstHandleId_ != INVALID_HANDLE_ID) {
        handleId_ = firstHandleId_;
        navController->SetInPIPMode(handleId_);
        TLOGI(WmsLogTag::WMS_PIP, "Cache first navigation");
    } else {
        TLOGI(WmsLogTag::WMS_PIP, "First top handle id: %{public}d", handleId_);
        firstHandleId_ = handleId_;
        navController->SetInPIPMode(handleId_);
    }
    return true;
}

std::string PictureInPictureController::GetPiPNavigationId() const
{
    return (pipOption_ != nullptr && !IsTypeNodeEnabled()) ? pipOption_->GetNavigationId() : "";
}

std::shared_ptr<NativeReference> PictureInPictureController::GetCustomNodeController()
{
    return pipOption_ == nullptr ? nullptr : pipOption_->GetNodeControllerRef();
}

std::shared_ptr<NativeReference> PictureInPictureController::GetTypeNode() const
{
    return pipOption_ == nullptr ? nullptr : pipOption_->GetTypeNodeRef();
}

NavigationController* PictureInPictureController::GetNavigationController(const std::string& navId)
{
    return NavigationController::GetNavigationController(mainWindow_->GetUIContent(), navId);
}

void PictureInPictureController::DeletePIPMode()
{
    if (mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Main window is null");
        return;
    }
    std::string navId = pipOption_->GetNavigationId();
    if (!navId.empty()) {
        auto navController = GetNavigationController(navId);
        if (navController) {
            navController->DeletePIPMode(handleId_);
            TLOGI(WmsLogTag::WMS_PIP, "Delete pip mode id: %{public}d", handleId_);
        }
    }
}

} // namespace Rosen
} // namespace OHOS