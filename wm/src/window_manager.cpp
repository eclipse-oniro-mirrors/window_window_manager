/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "window_manager.h"

#include <algorithm>
#include <cinttypes>

#include "event_handler.h"
#include "event_runner.h"
#include "marshalling_helper.h"

#include "window_adapter.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#ifdef EFFICIENCY_MANAGER_ENABLE
#include "suspend_manager_client.h"
#endif // EFFICIENCY_MANAGER_ENABLE

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManager"};
    const std::string WINDOW_MANAGER_CALLBACK_THREAD_NAME = "wm_listener";
}

bool WindowVisibilityInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteUint32(windowId_) && parcel.WriteInt32(pid_) &&
           parcel.WriteInt32(uid_) && parcel.WriteBool(isVisible_) &&
           parcel.WriteUint32(static_cast<uint32_t>(windowType_));
}

WindowVisibilityInfo* WindowVisibilityInfo::Unmarshalling(Parcel &parcel)
{
    auto windowVisibilityInfo = new (std::nothrow) WindowVisibilityInfo();
    if (windowVisibilityInfo == nullptr) {
        WLOGFE("window visibility info is nullptr.");
        return nullptr;
    }
    bool res = parcel.ReadUint32(windowVisibilityInfo->windowId_) && parcel.ReadInt32(windowVisibilityInfo->pid_) &&
    parcel.ReadInt32(windowVisibilityInfo->uid_) && parcel.ReadBool(windowVisibilityInfo->isVisible_);
    if (!res) {
        delete windowVisibilityInfo;
        return nullptr;
    }
    windowVisibilityInfo->windowType_ = static_cast<WindowType>(parcel.ReadUint32());
    return windowVisibilityInfo;
}

bool AccessibilityWindowInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(wid_) && parcel.WriteUint32(windowRect_.width_) &&
        parcel.WriteUint32(windowRect_.height_) && parcel.WriteInt32(windowRect_.posX_) &&
        parcel.WriteInt32(windowRect_.posY_) && parcel.WriteBool(focused_) && parcel.WriteBool(isDecorEnable_) &&
        parcel.WriteUint64(displayId_)  && parcel.WriteUint32(layer_) &&
        parcel.WriteUint32(static_cast<uint32_t>(mode_)) && parcel.WriteUint32(static_cast<uint32_t>(type_));
}

AccessibilityWindowInfo* AccessibilityWindowInfo::Unmarshalling(Parcel &parcel)
{
    auto info = new (std::nothrow) AccessibilityWindowInfo();
    if (info == nullptr) {
        WLOGFE("accessibility window info is nullptr.");
        return nullptr;
    }
    bool res = parcel.ReadInt32(info->wid_) && parcel.ReadUint32(info->windowRect_.width_) &&
        parcel.ReadUint32(info->windowRect_.height_) && parcel.ReadInt32(info->windowRect_.posX_) &&
        parcel.ReadInt32(info->windowRect_.posY_) && parcel.ReadBool(info->focused_) &&
        parcel.ReadBool(info->isDecorEnable_) && parcel.ReadUint64(info->displayId_) &&
        parcel.ReadUint32(info->layer_);
    if (!res) {
        delete info;
        return nullptr;
    }
    info->mode_ = static_cast<WindowMode>(parcel.ReadUint32());
    info->type_ = static_cast<WindowType>(parcel.ReadUint32());
    return info;
}

bool FocusChangeInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteUint32(windowId_) && parcel.WriteUint64(displayId_) &&
        parcel.WriteInt32(pid_) && parcel.WriteInt32(uid_) &&
        parcel.WriteUint32(static_cast<uint32_t>(windowType_));
}

FocusChangeInfo* FocusChangeInfo::Unmarshalling(Parcel &parcel)
{
    auto focusChangeInfo = new FocusChangeInfo();
    bool res = parcel.ReadUint32(focusChangeInfo->windowId_) && parcel.ReadUint64(focusChangeInfo->displayId_) &&
        parcel.ReadInt32(focusChangeInfo->pid_) && parcel.ReadInt32(focusChangeInfo->uid_);
    if (!res) {
        delete focusChangeInfo;
        return nullptr;
    }
    focusChangeInfo->windowType_ = static_cast<WindowType>(parcel.ReadUint32());
    return focusChangeInfo;
}

WM_IMPLEMENT_SINGLE_INSTANCE(WindowManager)

class WindowManager::Impl {
using ListenerTaskCallback = std::function<void()>;
using EventHandler = OHOS::AppExecFwk::EventHandler;
using EventPriority = OHOS::AppExecFwk::EventQueue::Priority;
public:
    void PostTask(ListenerTaskCallback &&callback, EventPriority priority, const std::string name);
    void InitListenerHandler();
    void NotifyFocused(uint32_t windowId, const sptr<IRemoteObject>& abilityToken,
        WindowType windowType, DisplayId displayId);
    void NotifyUnfocused(uint32_t windowId, const sptr<IRemoteObject>& abilityToken,
        WindowType windowType, DisplayId displayId);
    void NotifyFocused(const sptr<FocusChangeInfo>& focusChangeInfo);
    void NotifyUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo);
    void NotifySystemBarChanged(DisplayId displayId, const SystemBarRegionTints& tints);
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos, WindowUpdateType type);
    void NotifyWindowVisibilityInfoChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos);
    void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing);
    static inline SingletonDelegator<WindowManager> delegator_;

    bool isHandlerRunning_ = false;
    std::shared_ptr<EventHandler> listenerHandler_;
    std::recursive_mutex mutex_;
    std::vector<sptr<IFocusChangedListener>> focusChangedListeners_;
    sptr<WindowManagerAgent> focusChangedListenerAgent_;
    std::vector<sptr<ISystemBarChangedListener>> systemBarChangedListeners_;
    sptr<WindowManagerAgent> systemBarChangedListenerAgent_;
    std::vector<sptr<IWindowUpdateListener>> windowUpdateListeners_;
    sptr<WindowManagerAgent> windowUpdateListenerAgent_;
    std::vector<sptr<IVisibilityChangedListener>> windowVisibilityListeners_;
    sptr<WindowManagerAgent> windowVisibilityListenerAgent_;
    std::vector<sptr<ICameraFloatWindowChangedListener>> cameraFloatWindowChangedListeners_;
    sptr<WindowManagerAgent> cameraFloatWindowChangedListenerAgent_;
};

void WindowManager::Impl::PostTask(ListenerTaskCallback &&callback, EventPriority priority = EventPriority::LOW,
    const std::string name = "WINDOW_MANAGER_TASK")
{
    if (!isHandlerRunning_) {
        // Ensure that the callback thread is not used when it is initialized
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        InitListenerHandler();
    }
    if (listenerHandler_ == nullptr) {
        WLOGFE("listener handler is nullptr");
        return;
    }
    bool ret = listenerHandler_->PostTask([this, callback]() {
            callback();
        }, name, 0, priority); // 0 is task delay time
    if (!ret) {
        WLOGFE("post listener callback task failed.");
        return;
    }
    return;
}

void WindowManager::Impl::InitListenerHandler()
{
    auto runner = AppExecFwk::EventRunner::Create(WINDOW_MANAGER_CALLBACK_THREAD_NAME);
    if (runner == nullptr) {
        WLOGFE("init window manager callback runner failed.");
        return;
    }
    listenerHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (listenerHandler_ == nullptr) {
        WLOGFE("init window manager callback handler failed.");
        return;
    }
    isHandlerRunning_ = true;
    WLOGFD("init window manager callback runner success.");
}

void WindowManager::Impl::NotifyFocused(const sptr<FocusChangeInfo>& focusChangeInfo)
{
    WLOGFI("NotifyFocused [%{public}u; %{public}" PRIu64"; %{public}d; %{public}d; %{public}u; %{public}p]",
        focusChangeInfo->windowId_, focusChangeInfo->displayId_, focusChangeInfo->pid_, focusChangeInfo->uid_,
        static_cast<uint32_t>(focusChangeInfo->windowType_), focusChangeInfo->abilityToken_.GetRefPtr());
    std::vector<sptr<IFocusChangedListener>> focusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        focusChangeListeners = focusChangedListeners_;
    }
    PostTask([this, focusChangeInfo, focusChangeListeners]() mutable {
            for (auto& listener : focusChangeListeners) {
                listener->OnFocused(focusChangeInfo);
            }
        }, EventPriority::LOW, "FocusChangeInfo");
}

void WindowManager::Impl::NotifyUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo)
{
    WLOGFI("NotifyUnfocused [%{public}u; %{public}" PRIu64"; %{public}d; %{public}d; %{public}u; %{public}p]",
        focusChangeInfo->windowId_, focusChangeInfo->displayId_, focusChangeInfo->pid_, focusChangeInfo->uid_,
        static_cast<uint32_t>(focusChangeInfo->windowType_), focusChangeInfo->abilityToken_.GetRefPtr());
    std::vector<sptr<IFocusChangedListener>> focusChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        focusChangeListeners = focusChangedListeners_;
    }
    PostTask([this, focusChangeInfo, focusChangeListeners]() mutable {
            for (auto& listener : focusChangeListeners) {
                listener->OnUnfocused(focusChangeInfo);
            }
        }, EventPriority::LOW, "UnFocusChangeInfo");
}

void WindowManager::Impl::NotifySystemBarChanged(DisplayId displayId, const SystemBarRegionTints& tints)
{
    for (auto tint : tints) {
        WLOGFI("type:%{public}d, enable:%{public}d," \
            "backgroundColor:%{public}x, contentColor:%{public}x " \
            "region:[%{public}d, %{public}d, %{public}d, %{public}d]",
            tint.type_, tint.prop_.enable_, tint.prop_.backgroundColor_, tint.prop_.contentColor_,
            tint.region_.posX_, tint.region_.posY_, tint.region_.width_, tint.region_.height_);
    }
    std::vector<sptr<ISystemBarChangedListener>> systemBarChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        systemBarChangeListeners = systemBarChangedListeners_;
    }
    PostTask([this, displayId, tints, systemBarChangeListeners]() mutable {
            for (auto& listener : systemBarChangeListeners) {
                listener->OnSystemBarPropertyChange(displayId, tints);
            }
        }, EventPriority::LOW, "SystemBarChangeInfo");
}

void WindowManager::Impl::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    if (infos.empty()) {
        WLOGFE("infos is empty");
        return;
    }
    for (auto& info : infos) {
        WLOGFI("NotifyAccessibilityWindowInfo: wid[%{public}u], rect[%{public}d %{public}d %{public}d %{public}d]," \
            "isFocused[%{public}d], isDecorEnable[%{public}d], displayId[%{public}" PRIu64"], layer[%{public}u]," \
            "mode[%{public}u], type[%{public}u, updateType[%{public}d]",
            info->wid_, info->windowRect_.width_, info->windowRect_.height_, info->windowRect_.posX_,
            info->windowRect_.posY_, info->focused_, info->isDecorEnable_, info->displayId_, info->layer_,
            info->mode_, info->type_, type);
    }

    std::vector<sptr<IWindowUpdateListener>> windowUpdateListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        windowUpdateListeners = windowUpdateListeners_;
    }
    PostTask([this, infos, type, windowUpdateListeners]() mutable {
            for (auto& listener : windowUpdateListeners) {
                listener->OnWindowUpdate(infos, type);
            }
        }, EventPriority::LOW, "AccessibilityWindowInfo");
}

void WindowManager::Impl::NotifyWindowVisibilityInfoChanged(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos)
{
    std::vector<sptr<IVisibilityChangedListener>> visibilityChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        visibilityChangeListeners = windowVisibilityListeners_;
    }
    PostTask([this, windowVisibilityInfos, visibilityChangeListeners]() mutable {
            for (auto& listener : visibilityChangeListeners) {
                listener->OnWindowVisibilityChanged(windowVisibilityInfos);
            }
        }, EventPriority::LOW, "AccessibilityWindowInfo");
}

void WindowManager::Impl::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    WLOGFI("Camera float window, accessTokenId = %{public}u, isShowing = %{public}u", accessTokenId, isShowing);
    std::vector<sptr<ICameraFloatWindowChangedListener>> cameraFloatWindowChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        cameraFloatWindowChangeListeners = cameraFloatWindowChangedListeners_;
    }
    PostTask([this, accessTokenId, isShowing, cameraFloatWindowChangeListeners]() mutable {
            for (auto& listener : cameraFloatWindowChangeListeners) {
                listener->OnCameraFloatWindowChange(accessTokenId, isShowing);
            }
        }, EventPriority::LOW, "CameraFloatWindowStatus");
}

WindowManager::WindowManager() : pImpl_(std::make_unique<Impl>()) {}

void WindowManager::RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->focusChangedListeners_.begin(), pImpl_->focusChangedListeners_.end(), listener);
    if (iter != pImpl_->focusChangedListeners_.end()) {
        WLOGFI("Listener is already registered.");
        return;
    }
    pImpl_->focusChangedListeners_.push_back(listener);
    if (pImpl_->focusChangedListenerAgent_ == nullptr) {
        pImpl_->focusChangedListenerAgent_ = new WindowManagerAgent();
        SingletonContainer::Get<WindowAdapter>().RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
    }
}

void WindowManager::UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->focusChangedListeners_.begin(), pImpl_->focusChangedListeners_.end(), listener);
    if (iter == pImpl_->focusChangedListeners_.end()) {
        WLOGFE("could not find this listener");
        return;
    }
    pImpl_->focusChangedListeners_.erase(iter);
    if (pImpl_->focusChangedListeners_.empty() && pImpl_->focusChangedListenerAgent_ != nullptr) {
        SingletonContainer::Get<WindowAdapter>().UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS, pImpl_->focusChangedListenerAgent_);
        pImpl_->focusChangedListenerAgent_ = nullptr;
    }
}

void WindowManager::RegisterSystemBarChangedListener(const sptr<ISystemBarChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->systemBarChangedListeners_.begin(), pImpl_->systemBarChangedListeners_.end(),
        listener);
    if (iter != pImpl_->systemBarChangedListeners_.end()) {
        WLOGFI("Listener is already registered.");
        return;
    }
    pImpl_->systemBarChangedListeners_.push_back(listener);
    if (pImpl_->systemBarChangedListenerAgent_ == nullptr) {
        pImpl_->systemBarChangedListenerAgent_ = new WindowManagerAgent();
        SingletonContainer::Get<WindowAdapter>().RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR, pImpl_->systemBarChangedListenerAgent_);
    }
}

void WindowManager::UnregisterSystemBarChangedListener(const sptr<ISystemBarChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->systemBarChangedListeners_.begin(), pImpl_->systemBarChangedListeners_.end(),
        listener);
    if (iter == pImpl_->systemBarChangedListeners_.end()) {
        WLOGFE("could not find this listener");
        return;
    }
    pImpl_->systemBarChangedListeners_.erase(iter);
    if (pImpl_->systemBarChangedListeners_.empty() && pImpl_->systemBarChangedListenerAgent_ != nullptr) {
        SingletonContainer::Get<WindowAdapter>().UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR, pImpl_->systemBarChangedListenerAgent_);
        pImpl_->systemBarChangedListenerAgent_ = nullptr;
    }
}

void WindowManager::MinimizeAllAppWindows(DisplayId displayId)
{
    WLOGFI("displayId %{public}" PRIu64"", displayId);
    SingletonContainer::Get<WindowAdapter>().MinimizeAllAppWindows(displayId);
}

WMError WindowManager::ToggleShownStateForAllAppWindows()
{
    WLOGFI("ToggleShownStateForAllAppWindows");
    return SingletonContainer::Get<WindowAdapter>().ToggleShownStateForAllAppWindows();
}

WMError WindowManager::SetWindowLayoutMode(WindowLayoutMode mode)
{
    WLOGFI("set window layout mode: %{public}u", mode);
    WMError ret  = SingletonContainer::Get<WindowAdapter>().SetWindowLayoutMode(mode);
    if (ret != WMError::WM_OK) {
        WLOGFE("set layout mode failed");
    }
    return ret;
}

void WindowManager::RegisterWindowUpdateListener(const sptr<IWindowUpdateListener> &listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->windowUpdateListeners_.begin(), pImpl_->windowUpdateListeners_.end(), listener);
    if (iter != pImpl_->windowUpdateListeners_.end()) {
        WLOGFI("Listener is already registered.");
        return;
    }
    pImpl_->windowUpdateListeners_.emplace_back(listener);
    if (pImpl_->windowUpdateListenerAgent_ == nullptr) {
        pImpl_->windowUpdateListenerAgent_ = new WindowManagerAgent();
        SingletonContainer::Get<WindowAdapter>().RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
    }
}

void WindowManager::UnregisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->windowUpdateListeners_.begin(), pImpl_->windowUpdateListeners_.end(), listener);
    if (iter == pImpl_->windowUpdateListeners_.end()) {
        WLOGFE("could not find this listener");
        return;
    }
    pImpl_->windowUpdateListeners_.erase(iter);
    if (pImpl_->windowUpdateListeners_.empty() && pImpl_->windowUpdateListenerAgent_ != nullptr) {
        SingletonContainer::Get<WindowAdapter>().UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE, pImpl_->windowUpdateListenerAgent_);
        pImpl_->windowUpdateListenerAgent_ = nullptr;
    }
}

void WindowManager::RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->windowVisibilityListeners_.begin(), pImpl_->windowVisibilityListeners_.end(),
        listener);
    if (iter != pImpl_->windowVisibilityListeners_.end()) {
        WLOGFI("Listener is already registered.");
        return;
    }
    pImpl_->windowVisibilityListeners_.emplace_back(listener);
    if (pImpl_->windowVisibilityListenerAgent_ == nullptr) {
        pImpl_->windowVisibilityListenerAgent_ = new WindowManagerAgent();
        SingletonContainer::Get<WindowAdapter>().RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityListenerAgent_);
    }
}

void WindowManager::UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
{
    if (listener == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    pImpl_->windowVisibilityListeners_.erase(std::remove_if(pImpl_->windowVisibilityListeners_.begin(),
        pImpl_->windowVisibilityListeners_.end(), [listener](sptr<IVisibilityChangedListener> registeredListener) {
            return registeredListener == listener;
        }), pImpl_->windowVisibilityListeners_.end());

    if (pImpl_->windowVisibilityListeners_.empty() && pImpl_->windowVisibilityListenerAgent_ != nullptr) {
        SingletonContainer::Get<WindowAdapter>().UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY,
            pImpl_->windowVisibilityListenerAgent_);
        pImpl_->windowVisibilityListenerAgent_ = nullptr;
    }
}

void WindowManager::RegisterCameraFloatWindowChangedListener(const sptr<ICameraFloatWindowChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->cameraFloatWindowChangedListeners_.begin(),
        pImpl_->cameraFloatWindowChangedListeners_.end(), listener);
    if (iter != pImpl_->cameraFloatWindowChangedListeners_.end()) {
        WLOGFI("Listener is already registered.");
        return;
    }
    pImpl_->cameraFloatWindowChangedListeners_.push_back(listener);
    if (pImpl_->cameraFloatWindowChangedListenerAgent_ == nullptr) {
        pImpl_->cameraFloatWindowChangedListenerAgent_ = new WindowManagerAgent();
        SingletonContainer::Get<WindowAdapter>().RegisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT,
            pImpl_->cameraFloatWindowChangedListenerAgent_);
    }
}

void WindowManager::UnregisterCameraFloatWindowChangedListener(const sptr<ICameraFloatWindowChangedListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
    auto iter = std::find(pImpl_->cameraFloatWindowChangedListeners_.begin(),
        pImpl_->cameraFloatWindowChangedListeners_.end(), listener);
    if (iter == pImpl_->cameraFloatWindowChangedListeners_.end()) {
        WLOGFE("could not find this listener");
        return;
    }
    pImpl_->cameraFloatWindowChangedListeners_.erase(iter);
    if (pImpl_->cameraFloatWindowChangedListeners_.empty() &&
        pImpl_->cameraFloatWindowChangedListenerAgent_ != nullptr) {
        SingletonContainer::Get<WindowAdapter>().UnregisterWindowManagerAgent(
            WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT,
            pImpl_->cameraFloatWindowChangedListenerAgent_);
        pImpl_->cameraFloatWindowChangedListenerAgent_ = nullptr;
    }
}

void WindowManager::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) const
{
    if (focusChangeInfo == nullptr) {
        WLOGFE("focusChangeInfo is nullptr.");
        return;
    }
    WLOGFD("window focus change: %{public}d, id: %{public}u", focused, focusChangeInfo->windowId_);
    if (focused) {
#ifdef EFFICIENCY_MANAGER_ENABLE
        SuspendManager::SuspendManagerClient::GetInstance().ThawOneApplication(focusChangeInfo->uid_,
            "", "THAW_BY_FOCUS_CHANGED");
#endif // EFFICIENCY_MANAGER_ENABLE
        pImpl_->NotifyFocused(focusChangeInfo);
    } else {
        pImpl_->NotifyUnfocused(focusChangeInfo);
    }
}

void WindowManager::UpdateSystemBarRegionTints(DisplayId displayId,
    const SystemBarRegionTints& tints) const
{
    pImpl_->NotifySystemBarChanged(displayId, tints);
}

void WindowManager::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type) const
{
    pImpl_->NotifyAccessibilityWindowInfo(infos, type);
}

void WindowManager::UpdateWindowVisibilityInfo(
    const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos) const
{
    pImpl_->NotifyWindowVisibilityInfoChanged(windowVisibilityInfos);
}

WMError WindowManager::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const
{
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetAccessibilityWindowInfo(infos);
    if (ret != WMError::WM_OK) {
        WLOGFE("get window info failed");
    }
    return ret;
}

void WindowManager::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing) const
{
    pImpl_->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
}
} // namespace Rosen
} // namespace OHOS
