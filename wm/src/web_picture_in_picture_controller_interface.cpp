/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "web_picture_in_picture_controller_interface.h"

#include "native_pip_window_listener.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

using namespace Ace;

namespace {
    constexpr uint32_t MAX_CONTROL_GROUP_NUM = 3;
    std::shared_mutex cbSetMutex_;
    std::shared_mutex initRectMutex_;
    const std::set<PiPControlGroup> VIDEO_PLAY_CONTROLS {
        PiPControlGroup::VIDEO_PREVIOUS_NEXT,
        PiPControlGroup::FAST_FORWARD_BACKWARD,
    };
    const std::set<PiPControlGroup> VIDEO_CALL_CONTROLS {
        PiPControlGroup::VIDEO_CALL_MICROPHONE_SWITCH,
        PiPControlGroup::VIDEO_CALL_HANG_UP_BUTTON,
        PiPControlGroup::VIDEO_CALL_CAMERA_SWITCH,
        PiPControlGroup::VIDEO_CALL_MUTE_SWITCH,
    };
    const std::set<PiPControlGroup> VIDEO_MEETING_CONTROLS {
        PiPControlGroup::VIDEO_MEETING_HANG_UP_BUTTON,
        PiPControlGroup::VIDEO_MEETING_CAMERA_SWITCH,
        PiPControlGroup::VIDEO_MEETING_MUTE_SWITCH,
        PiPControlGroup::VIDEO_MEETING_MICROPHONE_SWITCH,
    };
    const std::set<PiPControlGroup> VIDEO_LIVE_CONTROLS {
        PiPControlGroup::VIDEO_PLAY_PAUSE,
        PiPControlGroup::VIDEO_LIVE_MUTE_SWITCH,
    };
    const std::map<PiPTemplateType, std::set<PiPControlGroup>> TEMPLATE_CONTROL_MAP {
        {PiPTemplateType::VIDEO_PLAY, VIDEO_PLAY_CONTROLS},
        {PiPTemplateType::VIDEO_CALL, VIDEO_CALL_CONTROLS},
        {PiPTemplateType::VIDEO_MEETING, VIDEO_MEETING_CONTROLS},
        {PiPTemplateType::VIDEO_LIVE, VIDEO_LIVE_CONTROLS},
    };
}

static int32_t checkControlsRules(uint32_t pipTemplateType, const std::vector<std::uint32_t>& controlGroups)
{
    auto iter = TEMPLATE_CONTROL_MAP.find(static_cast<PiPTemplateType>(pipTemplateType));
    if (iter == TEMPLATE_CONTROL_MAP.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "createPip param error, pipTemplateType not exists");
        return -1;
    }
    auto controls = iter->second;
    if (controlGroups.size() > MAX_CONTROL_GROUP_NUM) {
        return -1;
    }
    for (auto control : controlGroups) {
        if (controls.find(static_cast<PiPControlGroup>(control)) == controls.end()) {
            TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, controlGroup not matches, controlGroup: %{public}u",
                control);
            return -1;
        }
    }
    if (pipTemplateType != static_cast<uint32_t>(PiPTemplateType::VIDEO_PLAY)) {
        return 0;
    }
    auto iterFirst = std::find(controlGroups.begin(), controlGroups.end(),
        static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT));
    auto iterSecond = std::find(controlGroups.begin(), controlGroups.end(),
        static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
    if (iterFirst != controlGroups.end() && iterSecond != controlGroups.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, %{public}u conflicts with %{public}u in controlGroups",
            static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT),
            static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
        return -1;
    }
    return 0;
}

static int32_t checkCreatePipParams(const PiPConfig& config)
{
    if (config.mainWindowId == 0) {
        TLOGE(WmsLogTag::WMS_PIP, "mainWindowId could not be zero");
        return -1;
    }
    if (config.width == 0 || config.height == 0) {
        TLOGE(WmsLogTag::WMS_PIP, "width or height could not be zero");
        return -1;
    }
    if (config.env == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid env");
        return -1;
    }
    return checkControlsRules(config.pipTemplateType, config.controlGroup);
}

WMError WebPictureInPictureControllerInterface::Create(const PiPConfig& pipConfig)
{
    isPipEnabled_ = PictureInPictureControllerBase::GetPipEnabled();
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    int32_t ret = checkCreatePipParams(pipConfig);
    if (ret == -1) {
        TLOGE(WmsLogTag::WMS_PIP, "Invalid parameters when createPipController, "
            "please check if mainWindowId/width/height is zero, "
            "or env is nullptr, or controlGroup mismatch the corresponding pipTemplateType");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    sptrWebPipController_ = sptr<WebPictureInPictureController>::MakeSptr(pipConfig);
    return WMError::WM_OK;
}

WMError WebPictureInPictureControllerInterface::StartPip(uint32_t controllerId)
{
    if (auto pipController = sptrWebPipController_) {
        sptrWebPipController_->SetControllerId(controllerId);
        auto res = sptrWebPipController_->StartPictureInPicture(StartPipType::NATIVE_START);
        if (res == WMError::WM_OK) {
            isStarted_ = true;
        }
        return res;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::StopPip()
{
    if (auto pipController = sptrWebPipController_) {
        auto res = pipController->StopPictureInPictureFromClient();
        if (res == WMError::WM_OK) {
            isStarted_ = false;
        }
        return res;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::UpdateContentSize(int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid width or height");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (auto pipController = sptrWebPipController_) {
        pipController->UpdateContentSize(width, height);
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::UpdatePiPControlStatus(PiPControlType controlType,
    PiPControlStatus status)
{
    if (auto pipController = sptrWebPipController_) {
        pipController->UpdatePiPControlStatus(controlType, status);
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::setPiPControlEnabled(PiPControlType controlType, bool enabled)
{
    if (auto pipController = sptrWebPipController_) {
        pipController->UpdatePiPControlStatus(controlType,
            enabled ? PiPControlStatus::ENABLED : PiPControlStatus::DISABLED);
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::SetPipParentWindowId(uint32_t windowId)
{
    if (auto pipController = sptrWebPipController_) {
        return pipController->SetPipParentWindowId(windowId);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::SetPipInitialSurfaceRect(int32_t positionX, int32_t positionY,
    uint32_t width, uint32_t height)
{
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid initial rect");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (auto pipController = sptrWebPipController_) {
        std::unique_lock<std::shared_mutex> lock(initRectMutex_);
        pipController->SetPipInitialSurfaceRect(positionX, positionY, width, height);
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::UnsetPipInitialSurfaceRect()
{
    if (auto pipController = sptrWebPipController_) {
        std::unique_lock<std::shared_mutex> lock(initRectMutex_);
        pipController->SetPipInitialSurfaceRect(0, 0, 0, 0);
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

WMError WebPictureInPictureControllerInterface::RegisterStartPipListener(NativePipStartPipCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return RegisterListenerWithType(ListenerType::PIP_START_CB, listener);
}

WMError WebPictureInPictureControllerInterface::UnregisterStartPipListener(NativePipStartPipCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return UnregisterListenerWithType(ListenerType::PIP_START_CB, listener);
}


WMError WebPictureInPictureControllerInterface::RegisterLifeCycleListener(NativePipLifeCycleCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return RegisterListenerWithType(ListenerType::STATE_CHANGE_CB, listener);
}

WMError WebPictureInPictureControllerInterface::UnregisterLifeCycleListener(NativePipLifeCycleCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return UnregisterListenerWithType(ListenerType::STATE_CHANGE_CB, listener);
}

WMError WebPictureInPictureControllerInterface::RegisterControlEventListener(NativePipControlEventCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return RegisterListenerWithType(ListenerType::CONTROL_EVENT_CB, listener);
}

WMError WebPictureInPictureControllerInterface::UnregisterControlEventListener(NativePipControlEventCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return UnregisterListenerWithType(ListenerType::CONTROL_EVENT_CB, listener);
}

WMError WebPictureInPictureControllerInterface::RegisterResizeListener(NativePipResizeCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return RegisterListenerWithType(ListenerType::SIZE_CHANGE_CB, listener);
}

WMError WebPictureInPictureControllerInterface::UnregisterResizeListener(NativePipResizeCallback callback)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "callback is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto listener = sptr<NativePiPWindowListener>::MakeSptr(callback);
    return UnregisterListenerWithType(ListenerType::SIZE_CHANGE_CB, listener);
}

WMError WebPictureInPictureControllerInterface::CheckRegisterParam(ListenerType type,
    const sptr<NativePiPWindowListener>& listener)
{
    if (sptrWebPipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "WebPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "New NativePiPWindowListener failed");
        return WMError::WM_ERROR_PIP_STATE_ABNORMALLY;
    }
    return WMError::WM_OK;
}

WMError WebPictureInPictureControllerInterface::RegisterListenerWithType(ListenerType type,
    const sptr<NativePiPWindowListener>& listener)
{
    WMError ret = WMError::WM_OK;
    if ((ret = CheckRegisterParam(type, listener)) != WMError::WM_OK) {
        return ret;
    }
    if (IsRegistered(type, listener)) {
        TLOGE(WmsLogTag::WMS_PIP, "Listener already registered");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t cbMapSize = 0;
    {
        std::unique_lock<std::shared_mutex> lock(cbSetMutex_);
        switch (type) {
            case ListenerType::STATE_CHANGE_CB:
                ret = ProcessStateChangeRegister(listener);
                cbMapSize = lifeCycleCallbackSet_.size();
                break;
            case ListenerType::CONTROL_EVENT_CB:
                ret = ProcessControlEventRegister(listener);
                cbMapSize = controlEventCallbackSet_.size();
                break;
            case ListenerType::SIZE_CHANGE_CB:
                ret = ProcessSizeChangeRegister(listener);
                cbMapSize = resizeCallbackSet_.size();
                break;
            case ListenerType::PIP_START_CB:
                ret = ProcessPipStartRegister(listener);
                cbMapSize = startPipCallbackSet_.size();
                break;
            default:
                break;
        }
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Register type %{public}u failed", type);
        return ret;
    }
    TLOGI(WmsLogTag::WMS_PIP, "Register type %{public}u success! callback map size: %{public}d", type, cbMapSize);
    return WMError::WM_OK;
}

WMError WebPictureInPictureControllerInterface::UnregisterListenerWithType(ListenerType type,
    const sptr<NativePiPWindowListener>& listener)
{
    WMError ret = WMError::WM_OK;
    if ((ret = CheckRegisterParam(type, listener)) != WMError::WM_OK) {
        return ret;
    }
    if (!IsRegistered(type, listener)) {
        TLOGE(WmsLogTag::WMS_PIP, "Listener not registered");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t cbMapSize = 0;
    {
        std::unique_lock<std::shared_mutex> lock(cbSetMutex_);
        switch (type) {
            case ListenerType::STATE_CHANGE_CB:
                ret = ProcessStateChangeUnregister(listener);
                cbMapSize = lifeCycleCallbackSet_.size();
                break;
            case ListenerType::CONTROL_EVENT_CB:
                ret = ProcessControlEventUnregister(listener);
                cbMapSize = controlEventCallbackSet_.size();
                break;
            case ListenerType::SIZE_CHANGE_CB:
                ret = ProcessSizeChangeUnregister(listener);
                cbMapSize = resizeCallbackSet_.size();
                break;
            case ListenerType::PIP_START_CB:
                ret = ProcessPipStartUnregister(listener);
                cbMapSize = startPipCallbackSet_.size();
                break;
            default:
                break;
        }
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Unregister type %{public}u failed", type);
        return ret;
    }
    TLOGI(WmsLogTag::WMS_PIP, "Unregister type %{public}u success! callback map size: %{public}d", type, cbMapSize);
    return WMError::WM_OK;
}

WMError WebPictureInPictureControllerInterface::UnregisterAllPiPLifecycle()
{
    return UnregisterAll(ListenerType::STATE_CHANGE_CB);
}

WMError WebPictureInPictureControllerInterface::UnregisterAllPiPControlObserver()
{
    return UnregisterAll(ListenerType::CONTROL_EVENT_CB);
}

WMError WebPictureInPictureControllerInterface::UnregisterAllPiPWindowSize()
{
    return UnregisterAll(ListenerType::SIZE_CHANGE_CB);
}

WMError WebPictureInPictureControllerInterface::UnregisterAllPiPStart()
{
    return UnregisterAll(ListenerType::PIP_START_CB);
}

WMError WebPictureInPictureControllerInterface::UnregisterAll(ListenerType type)
{
    if (!isPipEnabled_) {
        TLOGE(WmsLogTag::WMS_PIP, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (auto pipController = sptrWebPipController_) {
        std::unique_lock<std::shared_mutex> lock(cbSetMutex_);
        switch (type) {
            case ListenerType::STATE_CHANGE_CB:
                lifeCycleCallbackSet_.clear();
                pipController->UnregisterAllPiPLifecycle();
                break;
            case ListenerType::CONTROL_EVENT_CB:
                controlEventCallbackSet_.clear();
                pipController->UnregisterAllPiPControlObserver();
                break;
            case ListenerType::SIZE_CHANGE_CB:
                resizeCallbackSet_.clear();
                pipController->UnregisterAllPiPWindowSize();
                break;
            case ListenerType::PIP_START_CB:
                startPipCallbackSet_.clear();
                pipController->UnregisterAllPiPStart();
                break;
            default:
                break;
        }
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "webPipController is nullptr");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
}

bool WebPictureInPictureControllerInterface::IsRegistered(ListenerType type,
    const sptr<NativePiPWindowListener>& listener)
{
    std::shared_lock<std::shared_mutex> lock(cbSetMutex_);
    switch (type) {
        case ListenerType::STATE_CHANGE_CB:
            for (const auto& it : lifeCycleCallbackSet_) {
                if (it == nullptr) {
                    continue;
                }
                if (it->GetLifeCycleCallbackRef() == listener->GetLifeCycleCallbackRef()) {
                    return true;
                }
            }
            break;
        case ListenerType::CONTROL_EVENT_CB:
            for (const auto& it : controlEventCallbackSet_) {
                if (it == nullptr) {
                    continue;
                }
                if (it->GetControlEventCallbackRef() == listener->GetControlEventCallbackRef()) {
                    return true;
                }
            }
            break;
        case ListenerType::SIZE_CHANGE_CB:
            for (const auto& it : resizeCallbackSet_) {
                if (it == nullptr) {
                    continue;
                }
                if (it->GetResizeCallbackRef() == listener->GetResizeCallbackRef()) {
                    return true;
                }
            }
            break;
        case ListenerType::PIP_START_CB:
            for (const auto& it : startPipCallbackSet_) {
                if (it == nullptr) {
                    continue;
                }
                if (it->GetStartPipCallbackRef() == listener->GetStartPipCallbackRef()) {
                    return true;
                }
            }
            break;
        default:
            break;
    }
    return false;
}

WMError WebPictureInPictureControllerInterface::ProcessStateChangeRegister(
    const sptr<NativePiPWindowListener>& listener)
{
    sptr<IPiPLifeCycle> thisListener(listener);
    WMError ret = sptrWebPipController_->RegisterPiPLifecycle(thisListener);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "register lifeCycleCallback failed");
        return ret;
    }
    lifeCycleCallbackSet_.insert(listener);
    return ret;
}

WMError WebPictureInPictureControllerInterface::ProcessStateChangeUnregister(
    const sptr<NativePiPWindowListener>& listener)
{
    for (const auto& it : lifeCycleCallbackSet_) {
        if (it == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one of lifeCycleCallbacks is nullptr");
            return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
        }
        if (it->GetLifeCycleCallbackRef() != listener->GetLifeCycleCallbackRef()) {
            continue;
        }
        sptr<IPiPLifeCycle> thisListener(it);
        WMError ret = sptrWebPipController_->UnregisterPiPLifecycle(thisListener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "Unregister lifeCycle callback failed");
            return ret;
        }
        lifeCycleCallbackSet_.erase(it);
        return ret;
    }
    TLOGE(WmsLogTag::WMS_PIP, "lifeCycle callback not found");
    return WMError::WM_ERROR_INVALID_PARAM;
}

WMError WebPictureInPictureControllerInterface::ProcessControlEventRegister(
    const sptr<NativePiPWindowListener>& listener)
{
    sptr<IPiPControlObserver> thisListener(listener);
    WMError ret = sptrWebPipController_->RegisterPiPControlObserver(thisListener);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "register controlEventCallback failed");
        return ret;
    }
    controlEventCallbackSet_.insert(listener);
    return ret;
}

WMError WebPictureInPictureControllerInterface::ProcessControlEventUnregister(
    const sptr<NativePiPWindowListener>& listener)
{
    for (const auto& it : controlEventCallbackSet_) {
        if (it == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one of controlEventCallbacks is nullptr");
            return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
        }
        if (it->GetControlEventCallbackRef() != listener->GetControlEventCallbackRef()) {
            continue;
        }
        sptr<IPiPControlObserver> thisListener(it);
        WMError ret = sptrWebPipController_->UnregisterPiPControlObserver(thisListener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "Unregister controlEvent callback failed");
            return ret;
        }
        controlEventCallbackSet_.erase(it);
        return ret;
    }
    TLOGE(WmsLogTag::WMS_PIP, "controlEvent callback not found");
    return WMError::WM_ERROR_INVALID_PARAM;
}

WMError WebPictureInPictureControllerInterface::ProcessSizeChangeRegister(
    const sptr<NativePiPWindowListener>& listener)
{
    sptr<IPiPWindowSize> thisListener(listener);
    WMError ret = sptrWebPipController_->RegisterPiPWindowSize(thisListener);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "register resizeCallback failed");
        return ret;
    }
    resizeCallbackSet_.insert(listener);
    return ret;
}

WMError WebPictureInPictureControllerInterface::ProcessSizeChangeUnregister(
    const sptr<NativePiPWindowListener>& listener)
{
    for (const auto& it : resizeCallbackSet_) {
        if (it == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one of resizeCallbacks is nullptr");
            return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
        }
        if (it->GetResizeCallbackRef() != listener->GetResizeCallbackRef()) {
            continue;
        }
        sptr<IPiPWindowSize> thisListener(it);
        WMError ret = sptrWebPipController_->UnregisterPiPWindowSize(thisListener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "Unregister resize callback failed");
            return ret;
        }
        resizeCallbackSet_.erase(it);
        TLOGI(WmsLogTag::WMS_PIP, "resize listener deleted");
        return ret;
    }
    TLOGE(WmsLogTag::WMS_PIP, "resize callback not found");
    return WMError::WM_ERROR_INVALID_PARAM;
}

WMError WebPictureInPictureControllerInterface::ProcessPipStartRegister(
    const sptr<NativePiPWindowListener>& listener)
{
    sptr<IPiPStartObserver> thisListener(listener);
    WMError ret =  sptrWebPipController_->RegisterPiPStart(thisListener);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "register pipStartCallback failed");
        return ret;
    }
    startPipCallbackSet_.insert(listener);
    return ret;
}

WMError WebPictureInPictureControllerInterface::ProcessPipStartUnregister(
    const sptr<NativePiPWindowListener>& listener)
{
    for (const auto& it : startPipCallbackSet_) {
        if (it == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one of startPipCallbacks is nullptr");
            return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
        }
        if (it->GetStartPipCallbackRef() != listener->GetStartPipCallbackRef()) {
            continue;
        }
        sptr<IPiPStartObserver> thisListener(it);
        WMError ret = sptrWebPipController_->UnregisterPiPStart(thisListener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "Unregister startPip callback failed");
            return ret;
        }
        startPipCallbackSet_.erase(it);
        TLOGI(WmsLogTag::WMS_PIP, "startPip listener deleted");
        return ret;
    }
    TLOGE(WmsLogTag::WMS_PIP, "startPip callback not found");
    return WMError::WM_ERROR_INVALID_PARAM;
}

} // namespace Rosen
} // namespace OHOS