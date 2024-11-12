/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at.
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software,
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "display_manager_lite.h"

#include <chrono>
#include <cinttypes>

#include "display_manager_adapter_lite.h"
#include "display_manager_agent_default.h"
#include "dm_common.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "DisplayManagerLite"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerLite)

class DisplayManagerLite::Impl : public RefBase {
public:
    Impl(std::recursive_mutex& mutex) : mutex_(mutex) {}
    ~Impl();
    static inline SingletonDelegator<DisplayManagerLite> delegator;
    sptr<DisplayLite> GetDefaultDisplay();
    FoldStatus GetFoldStatus();
    FoldDisplayMode GetFoldDisplayMode();
    void SetFoldDisplayMode(const FoldDisplayMode);
    bool IsFoldable();

    DMError RegisterDisplayListener(sptr<IDisplayListener> listener);
    DMError UnregisterDisplayListener(sptr<IDisplayListener> listener);
    DMError RegisterFoldStatusListener(sptr<IFoldStatusListener> listener);
    DMError UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener);
    DMError RegisterDisplayModeListener(sptr<IDisplayModeListener> listener);
    DMError UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener);
    void OnRemoteDied();
    sptr<DisplayLite> GetDisplayById(DisplayId displayId);
    /*
     * used by powermgr
     */
    bool SetDisplayState(DisplayState state, DisplayStateCallback callback);
private:
    void NotifyDisplayCreate(sptr<DisplayInfo> info);
    void NotifyDisplayDestroy(DisplayId);
    void NotifyDisplayChange(sptr<DisplayInfo> displayInfo);
    bool UpdateDisplayInfoLocked(sptr<DisplayInfo>);
    void NotifyFoldStatusChanged(FoldStatus foldStatus);
    void NotifyDisplayModeChanged(FoldDisplayMode displayMode);
    /*
     * used by powermgr
     */
    void NotifyDisplayStateChanged(DisplayId id, DisplayState state);
    void ClearDisplayStateCallback();
    void Clear();

    std::map<DisplayId, sptr<DisplayLite>> displayMap_;
    DisplayStateCallback displayStateCallback_;
    std::recursive_mutex& mutex_;
    std::set<sptr<IDisplayListener>> displayListeners_;
    std::set<sptr<IFoldStatusListener>> foldStatusListeners_;
    std::set<sptr<IDisplayModeListener>> displayModeListeners_;
    class DisplayManagerListener;
    sptr<DisplayManagerListener> displayManagerListener_;
    class DisplayManagerFoldStatusAgent;
    sptr<DisplayManagerFoldStatusAgent> foldStatusListenerAgent_;
    class DisplayManagerDisplayModeAgent;
    sptr<DisplayManagerDisplayModeAgent> displayModeListenerAgent_;
    /*
     * used by powermgr
     */
    class DisplayManagerAgent;
    sptr<DisplayManagerAgent> displayStateAgent_;
};

class DisplayManagerLite::Impl::DisplayManagerListener : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerListener(sptr<Impl> impl) : pImpl_(impl)
    {
    }

    void OnDisplayCreate(sptr<DisplayInfo> displayInfo) override
    {
        if (displayInfo == nullptr || displayInfo->GetDisplayId() == DISPLAY_ID_INVALID) {
            WLOGFE("onDisplayCreate: displayInfo is nullptr");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("onDisplayCreate: pImpl_ is nullptr");
            return;
        }
        pImpl_->NotifyDisplayCreate(displayInfo);
        std::set<sptr<IDisplayListener>> displayListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
            displayListeners = pImpl_->displayListeners_;
        }
        for (auto listener : displayListeners) {
            listener->OnCreate(displayInfo->GetDisplayId());
        }
    };

    void OnDisplayDestroy(DisplayId displayId) override
    {
        if (displayId == DISPLAY_ID_INVALID) {
            WLOGFE("onDisplayDestroy: displayId is invalid");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("onDisplayDestroy: impl is nullptr");
            return;
        }
        pImpl_->NotifyDisplayDestroy(displayId);
        std::set<sptr<IDisplayListener>> displayListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
            displayListeners = pImpl_->displayListeners_;
        }
        for (auto listener : displayListeners) {
            listener->OnDestroy(displayId);
        }
    };

    void OnDisplayChange(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event) override
    {
        if (displayInfo == nullptr || displayInfo->GetDisplayId() == DISPLAY_ID_INVALID) {
            WLOGFE("onDisplayChange: displayInfo is nullptr");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("onDisplayChange: pImpl_ is nullptr");
            return;
        }
        WLOGD("onDisplayChange: display %{public}" PRIu64", event %{public}u", displayInfo->GetDisplayId(), event);
        pImpl_->NotifyDisplayChange(displayInfo);
        std::set<sptr<IDisplayListener>> displayListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
            displayListeners = pImpl_->displayListeners_;
        }
        for (auto listener : displayListeners) {
            listener->OnChange(displayInfo->GetDisplayId());
        }
    };
private:
    sptr<Impl> pImpl_;
};

class DisplayManagerLite::Impl::DisplayManagerFoldStatusAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerFoldStatusAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerFoldStatusAgent() = default;

    virtual void NotifyFoldStatusChanged(FoldStatus foldStatus) override
    {
        pImpl_->NotifyFoldStatusChanged(foldStatus);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManagerLite::Impl::DisplayManagerDisplayModeAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerDisplayModeAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerDisplayModeAgent() = default;

    virtual void NotifyDisplayModeChanged(FoldDisplayMode displayMode) override
    {
        pImpl_->NotifyDisplayModeChanged(displayMode);
    }
private:
    sptr<Impl> pImpl_;
};

/*
 * used by powermgr
 */
class DisplayManagerLite::Impl::DisplayManagerAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerAgent() = default;

    virtual void NotifyDisplayStateChanged(DisplayId id, DisplayState state) override
    {
        pImpl_->NotifyDisplayStateChanged(id, state);
    }
private:
    sptr<Impl> pImpl_;
};

void DisplayManagerLite::Impl::Clear()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError res = DMError::DM_OK;
    if (displayManagerListener_ != nullptr) {
        res = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            displayManagerListener_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    }
    displayManagerListener_ = nullptr;
    if (res != DMError::DM_OK) {
        WLOGFW("UnregisterDisplayManagerAgent DISPLAY_EVENT_LISTENER failed");
    }
    ClearDisplayStateCallback();
}

DisplayManagerLite::Impl::~Impl()
{
    Clear();
}

DisplayManagerLite::DisplayManagerLite() : pImpl_(new Impl(mutex_))
{
}

DisplayManagerLite::~DisplayManagerLite()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    destroyed_ = true;
}

DMError DisplayManagerLite::Impl::RegisterDisplayListener(sptr<IDisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (displayManagerListener_ == nullptr) {
        displayManagerListener_ = new DisplayManagerListener(this);
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().RegisterDisplayManagerAgent(
            displayManagerListener_,
            DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayManagerAgent failed");
        displayManagerListener_ = nullptr;
    } else {
        displayListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::RegisterDisplayListener(sptr<IDisplayListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterDisplayListener listener is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterDisplayListener(sptr<IDisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(displayListeners_.begin(), displayListeners_.end(), listener);
    if (iter == displayListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    displayListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (displayListeners_.empty() && displayManagerListener_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            displayManagerListener_,
            DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
        displayManagerListener_ = nullptr;
    }
    return ret;
}

DMError DisplayManagerLite::UnregisterDisplayListener(sptr<IDisplayListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterDisplayListener listener is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayListener(listener);
}

void DisplayManagerLite::Impl::NotifyDisplayCreate(sptr<DisplayInfo> info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UpdateDisplayInfoLocked(info);
}

void DisplayManagerLite::Impl::NotifyDisplayDestroy(DisplayId displayId)
{
    WLOGFD("displayId:%{public}" PRIu64".", displayId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    displayMap_.erase(displayId);
}

void DisplayManagerLite::Impl::NotifyDisplayChange(sptr<DisplayInfo> displayInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UpdateDisplayInfoLocked(displayInfo);
}

bool DisplayManagerLite::Impl::UpdateDisplayInfoLocked(sptr<DisplayInfo> displayInfo)
{
    if (displayInfo == nullptr) {
        WLOGFE("displayInfo is null");
        return false;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    WLOGFD("displayId:%{public}" PRIu64".", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("displayId is invalid");
        return false;
    }
    auto iter = displayMap_.find(displayId);
    if (iter != displayMap_.end() && iter->second != nullptr) {
        WLOGFD("get screen in screen map");
        iter->second->UpdateDisplayInfo(displayInfo);
        return true;
    }
    sptr<DisplayLite> display = new DisplayLite("", displayInfo);
    displayMap_[displayId] = display;
    return true;
}

DMError DisplayManagerLite::RegisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("IFoldStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterFoldStatusListener(listener);
}

DMError DisplayManagerLite::Impl::RegisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (foldStatusListenerAgent_ == nullptr) {
        foldStatusListenerAgent_ = new DisplayManagerFoldStatusAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().RegisterDisplayManagerAgent(
            foldStatusListenerAgent_,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterFoldStatusListener failed !");
        foldStatusListenerAgent_ = nullptr;
    } else {
        WLOGI("IFoldStatusListener register success");
        foldStatusListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterFoldStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterFoldStatusListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(foldStatusListeners_.begin(), foldStatusListeners_.end(), listener);
    if (iter == foldStatusListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    foldStatusListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (foldStatusListeners_.empty() && foldStatusListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            foldStatusListenerAgent_,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
        foldStatusListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManagerLite::Impl::NotifyFoldStatusChanged(FoldStatus foldStatus)
{
    std::set<sptr<IFoldStatusListener>> foldStatusListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        foldStatusListeners = foldStatusListeners_;
    }
    for (auto& listener : foldStatusListeners) {
        listener->OnFoldStatusChanged(foldStatus);
    }
}

DMError DisplayManagerLite::RegisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("IDisplayModeListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayModeListener(listener);
}

DMError DisplayManagerLite::Impl::RegisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (displayModeListenerAgent_ == nullptr) {
        displayModeListenerAgent_ = new DisplayManagerDisplayModeAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().RegisterDisplayManagerAgent(
            displayModeListenerAgent_,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayModeListener failed !");
        displayModeListenerAgent_ = nullptr;
    } else {
        WLOGI("IDisplayModeListener register success");
        displayModeListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterDisplayModeListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayModeListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(displayModeListeners_.begin(), displayModeListeners_.end(), listener);
    if (iter == displayModeListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    displayModeListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (displayModeListeners_.empty() && displayModeListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            displayModeListenerAgent_,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
        displayModeListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManagerLite::Impl::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    std::set<sptr<IDisplayModeListener>> displayModeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displayModeListeners = displayModeListeners_;
    }
    for (auto& listener : displayModeListeners) {
        listener->OnDisplayModeChanged(displayMode);
    }
}

FoldStatus DisplayManagerLite::GetFoldStatus()
{
    return pImpl_->GetFoldStatus();
}

FoldStatus DisplayManagerLite::Impl::GetFoldStatus()
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetFoldStatus();
}

sptr<DisplayLite> DisplayManagerLite::GetDefaultDisplay()
{
    return pImpl_->GetDefaultDisplay();
}

sptr<DisplayLite> DisplayManagerLite::Impl::GetDefaultDisplay()
{
    auto displayInfo = SingletonContainer::Get<DisplayManagerAdapterLite>().GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        return nullptr;
    }
    auto displayId = displayInfo->GetDisplayId();
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateDisplayInfoLocked(displayInfo)) {
        displayMap_.erase(displayId);
        return nullptr;
    }
    return displayMap_[displayId];
}

bool DisplayManagerLite::IsFoldable()
{
    return pImpl_->IsFoldable();
}

bool DisplayManagerLite::Impl::IsFoldable()
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().IsFoldable();
}

FoldDisplayMode DisplayManagerLite::GetFoldDisplayMode()
{
    return pImpl_->GetFoldDisplayMode();
}

FoldDisplayMode DisplayManagerLite::Impl::GetFoldDisplayMode()
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetFoldDisplayMode();
}

void DisplayManagerLite::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    return pImpl_->SetFoldDisplayMode(mode);
}

void DisplayManagerLite::Impl::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SetFoldDisplayMode(mode);
}

void DisplayManagerLite::Impl::OnRemoteDied()
{
    WLOGFI("dms is died");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    displayManagerListener_ = nullptr;
}

void DisplayManagerLite::OnRemoteDied()
{
    pImpl_->OnRemoteDied();
}

sptr<DisplayLite> DisplayManagerLite::Impl::GetDisplayById(DisplayId displayId)
{
    WLOGFD("GetDisplayById start, displayId: %{public}" PRIu64" ", displayId);
    auto displayInfo = SingletonContainer::Get<DisplayManagerAdapterLite>().GetDisplayInfo(displayId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateDisplayInfoLocked(displayInfo)) {
        displayMap_.erase(displayId);
        return nullptr;
    }
    return displayMap_[displayId];
}

sptr<DisplayLite> DisplayManagerLite::GetDisplayById(DisplayId displayId)
{
    if (destroyed_) {
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return pImpl_->GetDisplayById(displayId);
}

/*
 * used by powermgr
 */
bool DisplayManagerLite::WakeUpBegin(PowerStateChangeReason reason)
{
    WLOGFD("[UL_POWER]WakeUpBegin start, reason:%{public}u", reason);
    return SingletonContainer::Get<DisplayManagerAdapterLite>().WakeUpBegin(reason);
}

bool DisplayManagerLite::WakeUpEnd()
{
    WLOGFD("[UL_POWER]WakeUpEnd start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().WakeUpEnd();
}

bool DisplayManagerLite::SuspendBegin(PowerStateChangeReason reason)
{
    // dms->wms notify other windows to hide
    WLOGFD("[UL_POWER]SuspendBegin start, reason:%{public}u", reason);
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SuspendBegin(reason);
}

bool DisplayManagerLite::SuspendEnd()
{
    WLOGFD("[UL_POWER]SuspendEnd start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SuspendEnd();
}

bool DisplayManagerLite::SetDisplayState(DisplayState state, DisplayStateCallback callback)
{
    return pImpl_->SetDisplayState(state, callback);
}

DisplayState DisplayManagerLite::GetDisplayState(DisplayId displayId)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetDisplayState(displayId);
}

bool DisplayManagerLite::Impl::SetDisplayState(DisplayState state, DisplayStateCallback callback)
{
    WLOGFD("[UL_POWER]state:%{public}u", state);
    bool ret = true;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displayStateCallback_ != nullptr || callback == nullptr) {
            WLOGFI("[UL_POWER]previous callback not called or callback invalid");
            if (displayStateCallback_ != nullptr) {
                WLOGFI("[UL_POWER]previous callback not called, the displayStateCallback_ is not null");
            }
            if (callback == nullptr) {
                WLOGFI("[UL_POWER]Invalid callback received");
            }
            return false;
        }
        displayStateCallback_ = callback;

        if (displayStateAgent_ == nullptr) {
            displayStateAgent_ = new DisplayManagerAgent(this);
            ret = SingletonContainer::Get<DisplayManagerAdapterLite>().RegisterDisplayManagerAgent(
                displayStateAgent_,
                DisplayManagerAgentType::DISPLAY_STATE_LISTENER) == DMError::DM_OK;
        }
    }
    ret = ret && SingletonContainer::Get<DisplayManagerAdapterLite>().SetDisplayState(state);
    if (!ret) {
        ClearDisplayStateCallback();
    }
    return ret;
}

void DisplayManagerLite::Impl::NotifyDisplayStateChanged(DisplayId id, DisplayState state)
{
    WLOGFD("state:%{public}u", state);
    DisplayStateCallback displayStateCallback = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displayStateCallback = displayStateCallback_;
    }
    if (displayStateCallback) {
        displayStateCallback(state);
        ClearDisplayStateCallback();
        return;
    }
    WLOGFW("callback_ target is not set!");
}

void DisplayManagerLite::Impl::ClearDisplayStateCallback()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    WLOGFD("[UL_POWER]Clear displaystatecallback enter");
    displayStateCallback_ = nullptr;
    if (displayStateAgent_ != nullptr) {
        WLOGFI("[UL_POWER]UnregisterDisplayManagerAgent enter and displayStateAgent_ is cleared");
        SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(displayStateAgent_,
            DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
        displayStateAgent_ = nullptr;
    }
}

bool DisplayManagerLite::TryToCancelScreenOff()
{
    WLOGFD("[UL_POWER]TryToCancelScreenOff start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().TryToCancelScreenOff();
}

bool DisplayManagerLite::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
    WLOGFD("[UL_POWER]SetScreenBrightness screenId:%{public}" PRIu64", level:%{public}u,", screenId, level);
    SingletonContainer::Get<DisplayManagerAdapterLite>().SetScreenBrightness(screenId, level);
    return true;
}

uint32_t DisplayManagerLite::GetScreenBrightness(uint64_t screenId) const
{
    uint32_t level = SingletonContainer::Get<DisplayManagerAdapterLite>().GetScreenBrightness(screenId);
    WLOGFD("[UL_POWER]GetScreenBrightness screenId:%{public}" PRIu64", level:%{public}u,", screenId, level);
    return level;
}

DisplayId DisplayManagerLite::GetDefaultDisplayId()
{
    auto info = SingletonContainer::Get<DisplayManagerAdapterLite>().GetDefaultDisplayInfo();
    if (info == nullptr) {
        return DISPLAY_ID_INVALID;
    }
    return info->GetDisplayId();
}

std::vector<DisplayId> DisplayManagerLite::GetAllDisplayIds()
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetAllDisplayIds();
}

VirtualScreenFlag DisplayManagerLite::GetVirtualScreenFlag(ScreenId screenId)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetVirtualScreenFlag(screenId);
}

} // namespace OHOS::Rosen