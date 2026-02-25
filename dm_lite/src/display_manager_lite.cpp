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
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerLite)

class DisplayManagerLite::Impl : public RefBase {
public:
    Impl(std::recursive_mutex& mutex) : mutex_(mutex) {}
    ~Impl();
    static inline SingletonDelegator<DisplayManagerLite> delegator;
    sptr<DisplayLite> GetDefaultDisplay(int32_t userId = CONCURRENT_USER_ID_DEFAULT);
    FoldStatus GetFoldStatus();
    FoldDisplayMode GetFoldDisplayMode();
    FoldDisplayMode GetFoldDisplayModeForExternal();
    void SetFoldDisplayMode(const FoldDisplayMode);
    void SetFoldDisplayModeAsync(const FoldDisplayMode);
    bool IsFoldable();

    DMError RegisterDisplayListener(sptr<IDisplayListener> listener);
    DMError UnregisterDisplayListener(sptr<IDisplayListener> listener);
    DMError RegisterFoldStatusListener(sptr<IFoldStatusListener> listener);
    DMError UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener);
    DMError RegisterDisplayModeListener(sptr<IDisplayModeListener> listener);
    DMError UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener);
    DMError RegisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener);
    DMError UnregisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener);
    void DlcloseClearResource();
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
    void ClearFoldStatusCallback();
    void ClearDisplayModeCallback();
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
    void NotifyScreenMagneticStateChanged(bool isMagneticState);
    std::set<sptr<IScreenMagneticStateListener>> screenMagneticStateListeners_;
    class DisplayManagerScreenMagneticStateAgent;
    sptr<DisplayManagerScreenMagneticStateAgent> screenMagneticStateListenerAgent_;
};

class DisplayManagerLite::Impl::DisplayManagerListener : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerListener(sptr<Impl> impl) : pImpl_(impl)
    {
    }

    void OnDisplayCreate(sptr<DisplayInfo> displayInfo) override
    {
        if (displayInfo == nullptr || displayInfo->GetDisplayId() == DISPLAY_ID_INVALID) {
            TLOGE(WmsLogTag::DMS, "displayInfo is nullptr");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "pImpl_ is nullptr");
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
            TLOGE(WmsLogTag::DMS, "displayId is invalid");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "impl is nullptr");
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
            TLOGE(WmsLogTag::DMS, "displayInfo is nullptr");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "pImpl_ is nullptr");
            return;
        }
        TLOGD(WmsLogTag::DMS, "display %{public}" PRIu64", event %{public}u", displayInfo->GetDisplayId(), event);
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

class DisplayManagerLite::Impl::DisplayManagerScreenMagneticStateAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerScreenMagneticStateAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerScreenMagneticStateAgent() = default;

    virtual void NotifyScreenMagneticStateChanged(bool isMagneticState) override
    {
        pImpl_->NotifyScreenMagneticStateChanged(isMagneticState);
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
        TLOGW(WmsLogTag::DMS, "UnregisterDisplayManagerAgent DISPLAY_EVENT_LISTENER failed");
    }
    ClearDisplayStateCallback();
    ClearFoldStatusCallback();
    ClearDisplayModeCallback();
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
        TLOGW(WmsLogTag::DMS, "RegisterDisplayManagerAgent failed");
        displayManagerListener_ = nullptr;
    } else {
        displayListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::RegisterDisplayListener(sptr<IDisplayListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterDisplayListener(sptr<IDisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(displayListeners_.begin(), displayListeners_.end(), listener);
    if (iter == displayListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
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
        TLOGE(WmsLogTag::DMS, "listener is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayListener(listener);
}

void DisplayManagerLite::Impl::NotifyDisplayCreate(sptr<DisplayInfo> info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    static_cast<void>(UpdateDisplayInfoLocked(info));
}

void DisplayManagerLite::Impl::NotifyDisplayDestroy(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "displayId:%{public}" PRIu64".", displayId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    displayMap_.erase(displayId);
}

void DisplayManagerLite::Impl::NotifyDisplayChange(sptr<DisplayInfo> displayInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    static_cast<void>(UpdateDisplayInfoLocked(displayInfo));
}

bool DisplayManagerLite::Impl::UpdateDisplayInfoLocked(sptr<DisplayInfo> displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo is null");
        return false;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    TLOGD(WmsLogTag::DMS, "displayId:%{public}" PRIu64".", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "displayId is invalid");
        return false;
    }
    auto iter = displayMap_.find(displayId);
    if (iter != displayMap_.end() && iter->second != nullptr) {
        TLOGD(WmsLogTag::DMS, "get screen in screen map");
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
        TLOGE(WmsLogTag::DMS, "IFoldStatusListener listener is nullptr.");
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
        TLOGW(WmsLogTag::DMS, "failed !");
        foldStatusListenerAgent_ = nullptr;
    } else {
        TLOGI(WmsLogTag::DMS, "IFoldStatusListener register success");
        foldStatusListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterFoldStatusListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(foldStatusListeners_.begin(), foldStatusListeners_.end(), listener);
    if (iter == foldStatusListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
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
        TLOGE(WmsLogTag::DMS, "IDisplayModeListener listener is nullptr.");
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
        TLOGW(WmsLogTag::DMS, "RegisterDisplayModeListener failed !");
        displayModeListenerAgent_ = nullptr;
    } else {
        TLOGI(WmsLogTag::DMS, "IDisplayModeListener register success");
        displayModeListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayModeListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(displayModeListeners_.begin(), displayModeListeners_.end(), listener);
    if (iter == displayModeListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
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

sptr<DisplayLite> DisplayManagerLite::GetDefaultDisplay(int32_t userId)
{
    return pImpl_->GetDefaultDisplay(userId);
}

sptr<DisplayLite> DisplayManagerLite::Impl::GetDefaultDisplay(int32_t userId)
{
    auto displayInfo = SingletonContainer::Get<DisplayManagerAdapterLite>().GetDefaultDisplayInfo(userId);
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

FoldDisplayMode DisplayManagerLite::GetFoldDisplayModeForExternal()
{
    return pImpl_->GetFoldDisplayModeForExternal();
}

FoldDisplayMode DisplayManagerLite::Impl::GetFoldDisplayMode()
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetFoldDisplayMode();
}

FoldDisplayMode DisplayManagerLite::Impl::GetFoldDisplayModeForExternal()
{
    FoldDisplayMode displayMode = SingletonContainer::Get<DisplayManagerAdapterLite>().GetFoldDisplayMode();
    if (displayMode == FoldDisplayMode::GLOBAL_FULL) {
        return FoldDisplayMode::FULL;
    }
    return displayMode;
}

void DisplayManagerLite::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    return pImpl_->SetFoldDisplayMode(mode);
}

void DisplayManagerLite::SetFoldDisplayModeAsync(const FoldDisplayMode mode)
{
    return pImpl_->SetFoldDisplayModeAsync(mode);
}

void DisplayManagerLite::Impl::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SetFoldDisplayMode(mode);
}

void DisplayManagerLite::Impl::SetFoldDisplayModeAsync(const FoldDisplayMode mode)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SetFoldDisplayModeAsync(mode);
}

void DisplayManagerLite::Impl::OnRemoteDied()
{
    TLOGI(WmsLogTag::DMS, "dms is died");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    displayManagerListener_ = nullptr;
    foldStatusListenerAgent_ = nullptr;
    displayModeListenerAgent_ = nullptr;
}

void DisplayManagerLite::OnRemoteDied()
{
    pImpl_->OnRemoteDied();
}

sptr<DisplayLite> DisplayManagerLite::Impl::GetDisplayById(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "GetDisplayById start, displayId: %{public}" PRIu64" ", displayId);
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
    TLOGD(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin start, reason:%{public}u", reason);
    return SingletonContainer::Get<DisplayManagerAdapterLite>().WakeUpBegin(reason);
}

bool DisplayManagerLite::WakeUpEnd()
{
    TLOGD(WmsLogTag::DMS, "[UL_POWER]WakeUpEnd start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().WakeUpEnd();
}

bool DisplayManagerLite::SuspendBegin(PowerStateChangeReason reason)
{
    // dms->wms notify other windows to hide
    TLOGD(WmsLogTag::DMS, "[UL_POWER]SuspendBegin start, reason:%{public}u", reason);
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SuspendBegin(reason);
}

bool DisplayManagerLite::SuspendEnd()
{
    TLOGD(WmsLogTag::DMS, "[UL_POWER]SuspendEnd start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SuspendEnd();
}

ScreenId DisplayManagerLite::GetInternalScreenId()
{
    TLOGD(WmsLogTag::DMS, "[UL_POWER]GetInternalScreenId start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetInternalScreenId();
}

bool DisplayManagerLite::SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason)
{
    TLOGD(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerById start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SetScreenPowerById(screenId, state, reason);
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
    TLOGD(WmsLogTag::DMS, "[UL_POWER]state:%{public}u", state);
    bool ret = true;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displayStateCallback_ != nullptr || callback == nullptr) {
            if (displayStateCallback_ != nullptr) {
                TLOGI(WmsLogTag::DMS, "[UL_POWER]previous callback not called, the displayStateCallback_ is not null");
            }
            if (callback == nullptr) {
                TLOGI(WmsLogTag::DMS, "[UL_POWER]Invalid callback received");
                return false;
            }
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
    TLOGD(WmsLogTag::DMS, "state:%{public}u", state);
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
    TLOGW(WmsLogTag::DMS, "callback_ target is not set!");
}

void DisplayManagerLite::Impl::ClearDisplayStateCallback()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    TLOGD(WmsLogTag::DMS, "[UL_POWER]Clear displaystatecallback enter");
    displayStateCallback_ = nullptr;
    if (displayStateAgent_ != nullptr) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]cleared");
        SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(displayStateAgent_,
            DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
        displayStateAgent_ = nullptr;
    }
}

extern "C" __attribute__((destructor)) void DisplayManagerLite::Impl::DlcloseClearResource()
{
    // clear stub callback when dlclose
    TLOGI(WmsLogTag::DMS, "enter");
    SingletonContainer::Get<DisplayManagerAdapterLite>().Clear();
    DisplayManagerAdapterLite::DestroyInstance();
}

void DisplayManagerLite::Impl::ClearFoldStatusCallback()
{
    DMError ret = DMError::DM_OK;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (foldStatusListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            foldStatusListenerAgent_,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
        foldStatusListenerAgent_ = nullptr;
        TLOGI(WmsLogTag::DMS, "foldStatusListenerAgent_ is nullptr !");
    }
    if (ret != DMError::DM_OK) {
        TLOGW(WmsLogTag::DMS, "Unregister failed! Error code: %{public}d", ret);
    }
}

void DisplayManagerLite::Impl::ClearDisplayModeCallback()
{
    DMError ret = DMError::DM_OK;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (displayModeListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            displayModeListenerAgent_,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
        displayModeListenerAgent_ = nullptr;
        TLOGI(WmsLogTag::DMS, "displayModeListenerAgent_ is nullptr !");
    }
    if (ret != DMError::DM_OK) {
        TLOGW(WmsLogTag::DMS, "Unregister failed! Error code: %{public}d", ret);
    }
}

DMError DisplayManagerLite::RegisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "IScreenMagneticStateListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenMagneticStateListener(listener);
}

DMError DisplayManagerLite::Impl::RegisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (screenMagneticStateListenerAgent_ == nullptr) {
        screenMagneticStateListenerAgent_ = new DisplayManagerScreenMagneticStateAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().RegisterDisplayManagerAgent(
            screenMagneticStateListenerAgent_,
            DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        TLOGW(WmsLogTag::DMS, "RegisterScreenMagneticStateListener failed !");
        screenMagneticStateListenerAgent_ = nullptr;
    } else if (listener != nullptr) {
        TLOGD(WmsLogTag::DMS, "IScreenMagneticStateListener register success");
        bool isKeyboardOn = SingletonContainer::Get<DisplayManagerAdapterLite>().GetKeyboardState();
        TLOGI(WmsLogTag::DMS, "RegisterScreenMagneticStateListener isKeyboardOn : %{public}d", isKeyboardOn);
        listener->OnScreenMagneticStateChanged(isKeyboardOn);
        screenMagneticStateListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManagerLite::UnregisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "UnregisterScreenMagneticStateListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenMagneticStateListener(listener);
}

DMError DisplayManagerLite::Impl::UnregisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(screenMagneticStateListeners_.begin(), screenMagneticStateListeners_.end(), listener);
    if (iter == screenMagneticStateListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenMagneticStateListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (screenMagneticStateListeners_.empty() && screenMagneticStateListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapterLite>().UnregisterDisplayManagerAgent(
            screenMagneticStateListenerAgent_,
            DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER);
        screenMagneticStateListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManagerLite::Impl::NotifyScreenMagneticStateChanged(bool isMagneticState)
{
    std::set<sptr<IScreenMagneticStateListener>> screenMagneticStateListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        screenMagneticStateListeners = screenMagneticStateListeners_;
    }
    for (auto& listener : screenMagneticStateListeners) {
        listener->OnScreenMagneticStateChanged(isMagneticState);
    }
}

bool DisplayManagerLite::TryToCancelScreenOff()
{
    TLOGD(WmsLogTag::DMS, "[UL_POWER]TryToCancelScreenOff start");
    return SingletonContainer::Get<DisplayManagerAdapterLite>().TryToCancelScreenOff();
}

bool DisplayManagerLite::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
    TLOGD(WmsLogTag::DMS, "[UL_POWER]SetScreenBrightness screenId:%{public}" PRIu64", level:%{public}u,",
        screenId, level);
    SingletonContainer::Get<DisplayManagerAdapterLite>().SetScreenBrightness(screenId, level);
    return true;
}

uint32_t DisplayManagerLite::GetScreenBrightness(uint64_t screenId) const
{
    uint32_t level = SingletonContainer::Get<DisplayManagerAdapterLite>().GetScreenBrightness(screenId);
    TLOGD(WmsLogTag::DMS, "[UL_POWER]GetScreenBrightness screenId:%{public}" PRIu64", level:%{public}u,",
        screenId, level);
    return level;
}

DisplayId DisplayManagerLite::GetDefaultDisplayId(int32_t userId)
{
    auto info = SingletonContainer::Get<DisplayManagerAdapterLite>().GetDefaultDisplayInfo(userId);
    if (info == nullptr) {
        return DISPLAY_ID_INVALID;
    }
    return info->GetDisplayId();
}

std::vector<DisplayId> DisplayManagerLite::GetAllDisplayIds(int32_t userId)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetAllDisplayIds(userId);
}

VirtualScreenFlag DisplayManagerLite::GetVirtualScreenFlag(ScreenId screenId)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().GetVirtualScreenFlag(screenId);
}

DMError DisplayManagerLite::SetSystemKeyboardStatus(bool isTpKeyboardOn)
{
    return SingletonContainer::Get<DisplayManagerAdapterLite>().SetSystemKeyboardStatus(isTpKeyboardOn);
}

DMError DisplayManagerLite::IsOnboardDisplay(DisplayId displayId, bool& isOnboardDisplay)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "id invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return SingletonContainer::Get<DisplayManagerAdapterLite>().IsOnboardDisplay(displayId, isOnboardDisplay);
}
} // namespace OHOS::Rosen