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

#include "picture_in_picture_controller_base.h"

#include <transaction/rs_sync_transaction_controller.h>
#include "parameters.h"
#include "picture_in_picture_manager.h"
#include "rs_adapter.h"
#include "singleton_container.h"
#include "window_adapter.h"
#include "window_manager_hilog.h"
#include "window_session_impl.h"

namespace OHOS {
namespace Rosen {

static const std::map<std::string, PiPControlType> CONTROL_TYPE_MAP = {
    {"playbackStateChanged", PiPControlType::VIDEO_PLAY_PAUSE},
    {"nextVideo", PiPControlType::VIDEO_NEXT},
    {"previousVideo", PiPControlType::VIDEO_PREVIOUS},
    {"hangUp", PiPControlType::HANG_UP_BUTTON},
    {"micStateChanged", PiPControlType::MICROPHONE_SWITCH},
    {"videoStateChanged", PiPControlType::CAMERA_SWITCH},
    {"voiceStateChanged", PiPControlType::MUTE_SWITCH},
    {"fastForward", PiPControlType::FAST_FORWARD},
    {"fastBackward", PiPControlType::FAST_BACKWARD}
};

PictureInPictureControllerBase::PictureInPictureControllerBase(sptr<PipOption> pipOption, sptr<Window> mainWindow,
    uint32_t windowId, napi_env env)
    : pipOption_(pipOption), mainWindow_(mainWindow), mainWindowId_(windowId), env_(env), weakRef_(this)
{
    curState_ = PiPWindowState::STATE_UNDEFINED;
}

PictureInPictureControllerBase::~PictureInPictureControllerBase()
{
    if (pipOption_) {
        pipOption_->ClearNapiRefs(env_);
    }
    TLOGI(WmsLogTag::WMS_PIP, "Destruction");
}

// LCOV_EXCL_START
void PictureInPictureControllerBase::NotifyOpretationError(WMError errCode, StartPipType startType)
{
    TLOGE(WmsLogTag::WMS_PIP, "window show failed, err: %{public}u", errCode);
    for (auto& listener : pipLifeCycleListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
        listener->OnPictureInPictureOperationError(static_cast<int32_t>(errCode));
        listener->OnPictureInPictureOperationError(controllerId_, static_cast<int32_t>(errCode));
    }
    SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(startType),
        pipOption_->GetPipTemplate(), PipConst::FAILED, "window show failed");
}

void PictureInPictureControllerBase::SetControllerId(uint32_t controllerId)
{
    controllerId_ = controllerId;
}

uint32_t PictureInPictureControllerBase::GetControllerId() const
{
    return controllerId_;
}

WMError PictureInPictureControllerBase::ShowPictureInPictureWindow(StartPipType startType)
{
    TLOGI(WmsLogTag::WMS_PIP, "startType:%{public}u", startType);
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Get PictureInPicture option failed");
        return WMError::WM_ERROR_PIP_CREATE_FAILED;
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is null when show pip");
        SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(startType),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "window is nullptr");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    for (auto& listener : pipLifeCycleListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
        listener->OnPreparePictureInPictureStart(GetStateChangeReason());
        listener->OnPreparePictureInPictureStart(controllerId_);
    }
    SetUIContent();
    WMError errCode = window_->Show(0, false);
    if (errCode != WMError::WM_OK) {
        NotifyOpretationError(errCode, startType);
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    uint32_t requestWidth = 0;
    uint32_t requestHeight = 0;
    pipOption_->GetContentSize(requestWidth, requestHeight);
    WindowSizeChangeReason reason = WindowSizeChangeReason::PIP_SHOW;
    if (startType == StartPipType::AUTO_START) {
        reason = WindowSizeChangeReason::PIP_AUTO_START;
    }
    if (requestWidth > 0 && requestHeight > 0) {
        Rect requestRect = {0, 0, requestWidth, requestHeight};
        window_->UpdatePiPRect(requestRect, reason);
    } else {
        window_->UpdatePiPRect(windowRect_, reason);
    }
    PictureInPictureManager::SetActiveController(this);
    SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(startType),
        pipOption_->GetPipTemplate(), PipConst::PIP_SUCCESS, "show pip success");
    isStoppedFromClient_ = false;
    return WMError::WM_OK;
}

WMError PictureInPictureControllerBase::StartPictureInPictureInner(StartPipType startType)
{
    WMError errCode = CreatePictureInPictureWindow(startType);
    if (errCode != WMError::WM_OK) {
        curState_ = PiPWindowState::STATE_UNDEFINED;
        TLOGE(WmsLogTag::WMS_PIP, "Create pip window failed, err: %{public}u", errCode);
        SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(startType),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "Create pip window failed");
        return errCode;
    }
    StartPipType type = startType;
    if (IsTypeNodeEnabled() && startType != StartPipType::AUTO_START) {
        type = StartPipType::AUTO_START;
    }
    errCode = ShowPictureInPictureWindow(type);
    if (errCode != WMError::WM_OK) {
        curState_ = PiPWindowState::STATE_UNDEFINED;
        TLOGE(WmsLogTag::WMS_PIP, "Show pip window failed, err: %{public}u", errCode);
        SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(type),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "Show pip window failed");
        return errCode;
    }
    curState_ = PiPWindowState::STATE_STARTED;
    SingletonContainer::Get<PiPReporter>().ReportPiPStartWindow(static_cast<int32_t>(type),
        pipOption_->GetPipTemplate(), PipConst::PIP_SUCCESS, "start pip success");
    return WMError::WM_OK;
}

WMError PictureInPictureControllerBase::StopPictureInPictureFromClient()
{
    if (!window_) {
        TLOGE(WmsLogTag::WMS_PIP, "window is null");
        SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(StopPipType::USER_STOP),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "window is null");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    if (curState_ == PiPWindowState::STATE_STOPPING || curState_ == PiPWindowState::STATE_STOPPED ||
        curState_ == PiPWindowState::STATE_RESTORING) {
        TLOGE(WmsLogTag::WMS_PIP, "Repeat stop request, curState: %{public}u", curState_);
        SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(StopPipType::USER_STOP),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "Repeat stop request");
        return WMError::WM_ERROR_PIP_REPEAT_OPERATION;
    }
    isStoppedFromClient_ = true;
    WMError res = window_->NotifyPrepareClosePiPWindow();
    if (res != WMError::WM_OK) {
        SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(StopPipType::USER_STOP),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "window destroy failed");
        return WMError::WM_ERROR_PIP_DESTROY_FAILED;
    }
    curState_ = PiPWindowState::STATE_STOPPING;
    return res;
}

WMError PictureInPictureControllerBase::StopPictureInPicture(bool destroyWindow, StopPipType stopPipType, bool withAnim)
{
    TLOGI(WmsLogTag::WMS_PIP, "destroyWindow: %{public}u anim: %{public}d", destroyWindow, withAnim);
    if ((!isStoppedFromClient_ && curState_ == PiPWindowState::STATE_STOPPING) ||
        curState_ == PiPWindowState::STATE_STOPPED) {
        TLOGE(WmsLogTag::WMS_PIP, "Repeat stop request, curState: %{public}u", curState_);
        SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(stopPipType),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "Repeat stop request");
        return WMError::WM_ERROR_PIP_REPEAT_OPERATION;
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr when stop pip");
        SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(stopPipType),
            pipOption_->GetPipTemplate(), PipConst::FAILED, "window_ is nullptr");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    curState_ = PiPWindowState::STATE_STOPPING;
    for (auto& listener : pipLifeCycleListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
        listener->OnPreparePictureInPictureStop(GetStateChangeReason());
        listener->OnPreparePictureInPictureStop(controllerId_);
    }
    if (!destroyWindow) {
        ResetExtController();
        curState_ = PiPWindowState::STATE_STOPPED;
        for (auto& listener : pipLifeCycleListeners_) {
            if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
            listener->OnPictureInPictureStop(GetStateChangeReason());
            listener->OnPictureInPictureStop(controllerId_);
        }
        PictureInPictureManager::RemoveActiveController(weakRef_);
        PictureInPictureManager::RemovePipControllerInfo(window_->GetWindowId());
        return WMError::WM_OK;
    }
    return StopPictureInPictureInner(stopPipType, withAnim);
}

WMError PictureInPictureControllerBase::StopPictureInPictureInner(StopPipType stopType, bool withAnim)
{
    uint32_t templateType = 0;
    if (pipOption_ != nullptr) {
        templateType = pipOption_->GetPipTemplate();
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr in stop pip inner");
        SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(stopType),
            templateType, PipConst::FAILED, "pipController is null");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    {
        AutoRSSyncTransaction syncTrans(window_->GetRSUIContext(), false);
        ResetExtController();
        if (!withAnim) {
            TLOGI(WmsLogTag::WMS_PIP, "DestroyPictureInPictureWindow without animation");
            DestroyPictureInPictureWindow();
        }
    }
    SingletonContainer::Get<PiPReporter>().ReportPiPStopWindow(static_cast<int32_t>(stopType),
        templateType, PipConst::PIP_SUCCESS, "pip window stop success");
    return WMError::WM_OK;
}

WMError PictureInPictureControllerBase::DestroyPictureInPictureWindow()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr when destroy pip");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window_->Destroy());
    if (ret != WmErrorCode::WM_OK) {
        curState_ = PiPWindowState::STATE_UNDEFINED;
        TLOGE(WmsLogTag::WMS_PIP, "window destroy failed, err:%{public}u", ret);
        for (auto& listener : pipLifeCycleListeners_) {
            if (listener == nullptr) {
                TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
                continue;
            }
            listener->OnPictureInPictureOperationError(static_cast<int32_t>(ret));
            listener->OnPictureInPictureOperationError(controllerId_, static_cast<int32_t>(ret));
        }
        return WMError::WM_ERROR_PIP_DESTROY_FAILED;
    }

    for (auto& listener : pipLifeCycleListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
        listener->OnPictureInPictureStop(GetStateChangeReason());
        listener->OnPictureInPictureStop(controllerId_);
    }
    curState_ = PiPWindowState::STATE_STOPPED;
    std::string navId = pipOption_ == nullptr ? "" : pipOption_->GetNavigationId();
    if (!navId.empty() && mainWindow_ && !IsTypeNodeEnabled()) {
        auto navController = NavigationController::GetNavigationController(mainWindow_->GetUIContent(), navId);
        if (navController) {
            navController->DeletePIPMode(handleId_);
            TLOGI(WmsLogTag::WMS_PIP, "Delete pip mode id: %{public}d", handleId_);
        }
    }
    if (mainWindow_ != nullptr) {
        mainWindow_->UnregisterLifeCycleListener(mainWindowLifeCycleListener_);
    }
    mainWindowLifeCycleListener_ = nullptr;
    PictureInPictureManager::RemovePipControllerInfo(window_->GetWindowId());
    window_ = nullptr;
    PictureInPictureManager::RemoveActiveController(this);
    return WMError::WM_OK;
}

sptr<Window> PictureInPictureControllerBase::GetPipWindow() const
{
    return window_;
}

uint32_t PictureInPictureControllerBase::GetMainWindowId() const
{
    return mainWindowId_;
}

void PictureInPictureControllerBase::SetPipWindow(sptr<Window> window)
{
    window_ = window;
}

PiPWindowState PictureInPictureControllerBase::GetControllerState() const
{
    return curState_;
}

void PictureInPictureControllerBase::UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    if (static_cast<int32_t>(status) < -1) {
        pipOption_->SetPiPControlEnabled(controlType, status);
    } else {
        pipOption_->SetPiPControlStatus(controlType, status);
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipWindow not exist");
        return;
    }
    window_->UpdatePiPControlStatus(controlType, status);
}

bool PictureInPictureControllerBase::IsContentSizeChanged(float width, float height, float posX, float posY)
{
    return windowRect_.width_ != static_cast<uint32_t>(width) ||
        windowRect_.height_ != static_cast<uint32_t>(height) ||
        windowRect_.posX_ != static_cast<int32_t>(posX) || windowRect_.posY_ != static_cast<int32_t>(posY);
}

void PictureInPictureControllerBase::WindowLifeCycleListener::AfterDestroyed()
{
    TLOGI(WmsLogTag::WMS_PIP, "stop picture_in_picture when attached window destroy");
    PictureInPictureManager::DoClose(true, true);
}

void PictureInPictureControllerBase::DoActionEvent(const std::string& actionName, int32_t status)
{
    TLOGI(WmsLogTag::WMS_PIP, "actionName: %{public}s", actionName.c_str());
    SingletonContainer::Get<PiPReporter>().ReportPiPActionEvent(pipOption_->GetPipTemplate(), actionName);
    for (auto& listener : pipActionObservers_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one action observer is nullptr");
            continue;
        }
        listener->OnActionEvent(actionName, status);
    }
    if (CONTROL_TYPE_MAP.find(actionName) != CONTROL_TYPE_MAP.end()) {
        pipOption_->SetPiPControlStatus(CONTROL_TYPE_MAP.at(actionName), static_cast<PiPControlStatus>(status));
    }
}

void PictureInPictureControllerBase::DoControlEvent(PiPControlType controlType, PiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, enabled:%{public}d", controlType, status);
    if (pipOption_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption_ is nullptr");
        return;
    }
    SingletonContainer::Get<PiPReporter>().ReportPiPControlEvent(pipOption_->GetPipTemplate(), controlType);
    for (auto& listener : pipControlObservers_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one control observer is nullptr");
            continue;
        }
        listener->OnControlEvent(controlType, status);
        listener->OnControlEvent(controllerId_, controlType, status);
    }
    pipOption_->SetPiPControlStatus(controlType, status);
}

void PictureInPictureControllerBase::PreRestorePictureInPicture()
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    curState_ = PiPWindowState::STATE_RESTORING;
    SetStateChangeReason(PiPStateChangeReason::PANEL_ACTION_RESTORE);
    for (auto& listener : pipLifeCycleListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
        listener->OnRestoreUserInterface(GetStateChangeReason());
        listener->OnRestoreUserInterface(controllerId_);
    }
}

void PictureInPictureControllerBase::LocateSource()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr");
        return;
    }
    window_->SetTransparent(true);
    UpdatePiPSourceRect();
}

void PictureInPictureControllerBase::PipSizeChange(double width, double height, double scale)
{
    PiPWindowSize windowSize;
    windowSize.width = std::round(width);
    windowSize.height = std::round(height);
    windowSize.scale = scale;
    TLOGI(WmsLogTag::WMS_PIP, "notify size info width: %{public}u, height: %{public}u scale: %{public}f",
          windowSize.width, windowSize.height, scale);
    for (auto& listener : pipWindowSizeListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one resize listener is nullptr");
            continue;
        }
        listener->OnPipSizeChange(windowSize);
        listener->OnPipSizeChange(controllerId_, windowSize);
    }
}

void PictureInPictureControllerBase::SetSurfaceId(uint64_t surfaceId)
{
    surfaceId_ = surfaceId;
}

uint64_t PictureInPictureControllerBase::GetSurfaceId() const
{
    return surfaceId_;
}

void PictureInPictureControllerBase::SetStateChangeReason(PiPStateChangeReason reason)
{
    stateChangeReason_ = reason;
}

PiPStateChangeReason PictureInPictureControllerBase::GetStateChangeReason() const
{
    return stateChangeReason_;
}

void PictureInPictureControllerBase::ActiveStatusChange(bool status)
{
    TLOGI(WmsLogTag::WMS_PIP, "notify active status: %{public}u", status);
    curActiveStatus_ = status;
    for (auto& listener : PiPActiveStatusObserver_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "screen active listener is nullptr");
            continue;
        }
        listener->OnActiveStatusChange(status);
    }
}

void PictureInPictureControllerBase::OnPictureInPictureStart()
{
    for (auto& listener : pipLifeCycleListeners_) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one lifecycle listener is nullptr");
            continue;
        }
        listener->OnPictureInPictureStart(GetStateChangeReason());
        listener->OnPictureInPictureStart(controllerId_);
    }
}

WMError PictureInPictureControllerBase::RegisterPiPLifecycle(const sptr<IPiPLifeCycle>& listener)
{
    return RegisterListener(pipLifeCycleListeners_, listener);
}

WMError PictureInPictureControllerBase::RegisterPiPActionObserver(const sptr<IPiPActionObserver>& listener)
{
    return RegisterListener(pipActionObservers_, listener);
}

WMError PictureInPictureControllerBase::RegisterPiPControlObserver(const sptr<IPiPControlObserver>& listener)
{
    return RegisterListener(pipControlObservers_, listener);
}

WMError PictureInPictureControllerBase::RegisterPiPWindowSize(const sptr<IPiPWindowSize>& listener)
{
    return RegisterListener(pipWindowSizeListeners_, listener);
}

WMError PictureInPictureControllerBase::RegisterPiPTypeNodeChange(const sptr<IPiPTypeNodeObserver>& listener)
{
    return RegisterListener(pipTypeNodeObserver_, listener);
}

WMError PictureInPictureControllerBase::RegisterPiPStart(const sptr<IPiPStartObserver>& listener)
{
    return RegisterListener(pipStartListeners_, listener);
}

WMError PictureInPictureControllerBase::RegisterPiPActiveStatusChange(const sptr<IPiPActiveStatusObserver>& listener)
{
    return RegisterListener(PiPActiveStatusObserver_, listener);
}

WMError PictureInPictureControllerBase::UnregisterPiPLifecycle(const sptr<IPiPLifeCycle>& listener)
{
    return UnregisterListener(pipLifeCycleListeners_, listener);
}

WMError PictureInPictureControllerBase::UnregisterPiPActionObserver(const sptr<IPiPActionObserver>& listener)
{
    return UnregisterListener(pipActionObservers_, listener);
}

WMError PictureInPictureControllerBase::UnregisterPiPControlObserver(const sptr<IPiPControlObserver>& listener)
{
    return UnregisterListener(pipControlObservers_, listener);
}

WMError PictureInPictureControllerBase::UnregisterPiPWindowSize(const sptr<IPiPWindowSize>& listener)
{
    return UnregisterListener(pipWindowSizeListeners_, listener);
}

WMError PictureInPictureControllerBase::UnRegisterPiPTypeNodeChange(const sptr<IPiPTypeNodeObserver>& listener)
{
    return UnregisterListener(pipTypeNodeObserver_, listener);
}

WMError PictureInPictureControllerBase::UnregisterPiPStart(const sptr<IPiPStartObserver>& listener)
{
    return UnregisterListener(pipStartListeners_, listener);
}

WMError PictureInPictureControllerBase::UnregisterPiPActiveStatusChange(const sptr<IPiPActiveStatusObserver>& listener)
{
    return UnregisterListener(PiPActiveStatusObserver_, listener);
}

void PictureInPictureControllerBase::UnregisterAllPiPLifecycle()
{
    pipLifeCycleListeners_.clear();
}

void PictureInPictureControllerBase::UnregisterAllPiPControlObserver()
{
    pipControlObservers_.clear();
}

void PictureInPictureControllerBase::UnregisterAllPiPWindowSize()
{
    pipWindowSizeListeners_.clear();
}

void PictureInPictureControllerBase::UnregisterAllPiPStart()
{
    pipStartListeners_.clear();
}

std::vector<sptr<IPiPLifeCycle>> PictureInPictureControllerBase::GetPictureInPictureLifecycle() const
{
    return pipLifeCycleListeners_;
}

std::vector<sptr<IPiPActionObserver>> PictureInPictureControllerBase::GetPictureInPictureActionObserver() const
{
    return pipActionObservers_;
}

std::vector<sptr<IPiPControlObserver>> PictureInPictureControllerBase::GetPictureInPictureControlObserver() const
{
    return pipControlObservers_;
}

std::vector<sptr<IPiPWindowSize>> PictureInPictureControllerBase::GetPictureInPictureSizeObserver() const
{
    return pipWindowSizeListeners_;
}

std::vector<sptr<IPiPStartObserver>> PictureInPictureControllerBase::GetPictureInPictureStartObserver() const
{
    return pipStartListeners_;
}

template<typename T>
WMError PictureInPictureControllerBase::RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "Listener already registered");
        return WMError::WM_OK;
    }
    holder.emplace_back(listener);
    return WMError::WM_OK;
}

template<typename T>
WMError PictureInPictureControllerBase::UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    holder.erase(std::remove_if(holder.begin(), holder.end(),
        [listener](const sptr<T>& registeredListener) {
            return registeredListener == listener;
        }), holder.end());
    return WMError::WM_OK;
}

void PictureInPictureControllerBase::GetPipPossible(bool& pipPossible)
{
    bool isPipEnabledFlag = false;
    SingletonContainer::Get<WindowAdapter>().GetIsPipEnabled(isPipEnabledFlag);
    pipPossible = isPipEnabledFlag;
    return;
}

bool PictureInPictureControllerBase::GetPipEnabled()
{
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    return multiWindowUIType == "HandsetSmartWindow" || multiWindowUIType == "FreeFormMultiWindow" ||
        multiWindowUIType == "TabletSmartWindow";
}

bool PictureInPictureControllerBase::GetPipSettingSwitchStatusEnabled()
{
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    return multiWindowUIType == "HandsetSmartWindow" || multiWindowUIType == "TabletSmartWindow";
}

bool PictureInPictureControllerBase::GetPiPSettingSwitchStatus()
{
    sptr<WindowSessionImpl> windowSessionImpl = WindowSessionImpl::GetWindowWithId(mainWindowId_);
    if (windowSessionImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "windowId not found.");
        return false;
    }
    bool switchStatus = false;
    WMError errcode = windowSessionImpl->GetPiPSettingSwitchStatus(switchStatus);
    if (errcode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "get switch error.");
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIP, "switchStatus: %{public}d", switchStatus);
    return switchStatus;
}

WMError PictureInPictureControllerBase::IsPiPActive(bool& status)
{
    if (curState_ != PiPWindowState::STATE_STARTED) {
        return WMError::WM_OK;
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "window is nullptr.");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    WMError ret = window_->IsPiPActive(status);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "get switch error.");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    TLOGI(WmsLogTag::WMS_PIP, "active status: %{public}d", status);
    return WMError::WM_OK;
}
// LCOV_EXCL_STOP
} // namespace Rosen
} // namespace OHOS