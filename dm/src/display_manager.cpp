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

#include "display_manager.h"

#include <chrono>
#include <cinttypes>
#include <transaction/rs_interfaces.h>
#include <ui/rs_surface_node.h>

#include "display_manager_adapter.h"
#include "display_manager_agent_default.h"
#include "dm_common.h"
#include "screen_manager.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "DisplayManager"};
const static uint32_t MAX_RETRY_NUM = 6;
const static uint32_t RETRY_WAIT_MS = 500;
const static uint32_t MAX_DISPLAY_SIZE = 32;
const static uint32_t MAX_INTERVAL_US = 25000;
std::atomic<bool> g_dmIsDestroyed = false;
std::mutex snapBypickerMutex;
}
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManager)

class DisplayManager::Impl : public RefBase {
public:
    Impl(std::recursive_mutex& mutex) : mutex_(mutex) {}
    ~Impl();

    static inline SingletonDelegator<DisplayManager> delegator;
    bool CheckRectValid(const Media::Rect& rect, int32_t oriHeight, int32_t oriWidth) const;
    bool CheckSizeValid(const Media::Size& size, int32_t oriHeight, int32_t oriWidth) const;
    sptr<Display> GetDefaultDisplay();
    sptr<Display> GetDefaultDisplaySync();
    std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution();
    sptr<Display> GetDisplayById(DisplayId displayId);
    DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow);
    bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId);
    bool IsFoldable();
    bool IsCaptured();
    FoldStatus GetFoldStatus();
    FoldDisplayMode GetFoldDisplayMode();
    void SetFoldDisplayMode(const FoldDisplayMode);
    DMError SetFoldDisplayModeFromJs(const FoldDisplayMode);
    void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX, float pivotY);
    void SetFoldStatusLocked(bool locked);
    DMError SetFoldStatusLockedFromJs(bool locked);
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    DMError RegisterDisplayListener(sptr<IDisplayListener> listener);
    DMError UnregisterDisplayListener(sptr<IDisplayListener> listener);
    bool SetDisplayState(DisplayState state, DisplayStateCallback callback);
    DMError RegisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener);
    DMError UnregisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener);
    DMError RegisterScreenshotListener(sptr<IScreenshotListener> listener);
    DMError UnregisterScreenshotListener(sptr<IScreenshotListener> listener);
    DMError RegisterPrivateWindowListener(sptr<IPrivateWindowListener> listener);
    DMError UnregisterPrivateWindowListener(sptr<IPrivateWindowListener> listener);
    DMError RegisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener);
    DMError UnregisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener);
    DMError RegisterFoldStatusListener(sptr<IFoldStatusListener> listener);
    DMError UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener);
    DMError RegisterFoldAngleListener(sptr<IFoldAngleListener> listener);
    DMError UnregisterFoldAngleListener(sptr<IFoldAngleListener> listener);
    DMError RegisterCaptureStatusListener(sptr<ICaptureStatusListener> listener);
    DMError UnregisterCaptureStatusListener(sptr<ICaptureStatusListener> listener);
    DMError RegisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener);
    DMError UnregisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener);
    DMError RegisterDisplayModeListener(sptr<IDisplayModeListener> listener);
    DMError UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener);
    DMError RegisterAvailableAreaListener(sptr<IAvailableAreaListener> listener);
    DMError UnregisterAvailableAreaListener(sptr<IAvailableAreaListener> listener);
    sptr<Display> GetDisplayByScreenId(ScreenId screenId);
    DMError ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy);
    DMError ResetAllFreezeStatus();
    DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid, std::vector<uint64_t>& windowIdList);
    void OnRemoteDied();
private:
    void ClearDisplayStateCallback();
    void ClearFoldStatusCallback();
    void ClearFoldAngleCallback();
    void ClearCaptureStatusCallback();
    void ClearDisplayModeCallback();
    void NotifyPrivateWindowStateChanged(bool hasPrivate);
    void NotifyPrivateStateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList);
    void NotifyScreenshot(sptr<ScreenshotInfo> info);
    void NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status);
    void NotifyDisplayStateChanged(DisplayId id, DisplayState state);
    void NotifyDisplayChangedEvent(sptr<DisplayInfo> info, DisplayChangeEvent event);
    void NotifyDisplayCreate(sptr<DisplayInfo> info);
    void NotifyDisplayDestroy(DisplayId);
    void NotifyDisplayChange(sptr<DisplayInfo> displayInfo);
    bool UpdateDisplayInfoLocked(sptr<DisplayInfo>);
    void NotifyFoldStatusChanged(FoldStatus foldStatus);
    void NotifyFoldAngleChanged(std::vector<float> foldAngles);
    void NotifyCaptureStatusChanged(bool isCapture);
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info);
    void NotifyDisplayModeChanged(FoldDisplayMode displayMode);
    void NotifyAvailableAreaChanged(DMRect rect);
    void Clear();
    std::string GetDisplayInfoSrting(sptr<DisplayInfo> displayInfo);

    DisplayId defaultDisplayId_ = DISPLAY_ID_INVALID;
    std::map<DisplayId, sptr<Display>> displayMap_;
    std::map<DisplayId, std::chrono::steady_clock::time_point> displayUptateTimeMap_;
    DisplayStateCallback displayStateCallback_;
    std::recursive_mutex& mutex_;
    std::set<sptr<IDisplayListener>> displayListeners_;
    std::set<sptr<IDisplayPowerEventListener>> powerEventListeners_;
    std::set<sptr<IScreenshotListener>> screenshotListeners_;
    std::set<sptr<IPrivateWindowListener>> privateWindowListeners_;
    std::set<sptr<IPrivateWindowListChangeListener>> privateWindowListChangeListeners_;
    std::set<sptr<IFoldStatusListener>> foldStatusListeners_;
    std::set<sptr<IFoldAngleListener>> foldAngleListeners_;
    std::set<sptr<ICaptureStatusListener>> captureStatusListeners_;
    std::set<sptr<IDisplayUpdateListener>> displayUpdateListeners_;
    std::set<sptr<IDisplayModeListener>> displayModeListeners_;
    std::set<sptr<IAvailableAreaListener>> availableAreaListeners_;
    class DisplayManagerListener;
    sptr<DisplayManagerListener> displayManagerListener_;
    class DisplayManagerAgent;
    sptr<DisplayManagerAgent> displayStateAgent_;
    sptr<DisplayManagerAgent> powerEventListenerAgent_;
    class DisplayManagerScreenshotAgent;
    sptr<DisplayManagerScreenshotAgent> screenshotListenerAgent_;
    class DisplayManagerPrivateWindowAgent;
    sptr<DisplayManagerPrivateWindowAgent> privateWindowListenerAgent_;
    class DisplayManagerPrivateWindowListAgent;
    sptr<DisplayManagerPrivateWindowListAgent> privateWindowListChangeListenerAgent_;
    class DisplayManagerFoldStatusAgent;
    sptr<DisplayManagerFoldStatusAgent> foldStatusListenerAgent_;
    class DisplayManagerFoldAngleAgent;
    sptr<DisplayManagerFoldAngleAgent> foldAngleListenerAgent_;
    class DisplayManagerCaptureStatusAgent;
    sptr<DisplayManagerCaptureStatusAgent> captureStatusListenerAgent_;
    class DisplayManagerDisplayUpdateAgent;
    sptr<DisplayManagerDisplayUpdateAgent> displayUpdateListenerAgent_;
    class DisplayManagerDisplayModeAgent;
    sptr<DisplayManagerDisplayModeAgent> displayModeListenerAgent_;
    class DisplayManagerAvailableAreaAgent;
    sptr<DisplayManagerAvailableAreaAgent> availableAreaListenerAgent_;
};

class DisplayManager::Impl::DisplayManagerListener : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerListener(sptr<Impl> impl) : pImpl_(impl)
    {
    }

    void OnDisplayCreate(sptr<DisplayInfo> displayInfo) override
    {
        if (displayInfo == nullptr || displayInfo->GetDisplayId() == DISPLAY_ID_INVALID) {
            WLOGFE("OnDisplayCreate, displayInfo is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("OnDisplayCreate, impl is nullptr.");
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
            WLOGFE("OnDisplayDestroy, displayId is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("OnDisplayDestroy, impl is nullptr.");
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
            WLOGFE("OnDisplayChange, displayInfo is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("OnDisplayChange, impl is nullptr.");
            return;
        }
        WLOGD("OnDisplayChange. display %{public}" PRIu64", event %{public}u", displayInfo->GetDisplayId(), event);
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

class DisplayManager::Impl::DisplayManagerAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerAgent() = default;

    virtual void NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status) override
    {
        pImpl_->NotifyDisplayPowerEvent(event, status);
    }

    virtual void NotifyDisplayStateChanged(DisplayId id, DisplayState state) override
    {
        pImpl_->NotifyDisplayStateChanged(id, state);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManager::Impl::DisplayManagerScreenshotAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerScreenshotAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerScreenshotAgent() = default;

    virtual void OnScreenshot(sptr<ScreenshotInfo> info) override
    {
        pImpl_->NotifyScreenshot(info);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManager::Impl::DisplayManagerPrivateWindowAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerPrivateWindowAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerPrivateWindowAgent() = default;

    virtual void NotifyPrivateWindowStateChanged(bool hasPrivate) override
    {
        pImpl_->NotifyPrivateWindowStateChanged(hasPrivate);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManager::Impl::DisplayManagerPrivateWindowListAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerPrivateWindowListAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerPrivateWindowListAgent() = default;

    virtual void NotifyPrivateStateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList) override
    {
        pImpl_->NotifyPrivateStateWindowListChanged(id, privacyWindowList);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManager::Impl::DisplayManagerFoldStatusAgent : public DisplayManagerAgentDefault {
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

class DisplayManager::Impl::DisplayManagerFoldAngleAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerFoldAngleAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerFoldAngleAgent() = default;

    virtual void NotifyFoldAngleChanged(std::vector<float> foldAngles) override
    {
        pImpl_->NotifyFoldAngleChanged(foldAngles);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManager::Impl::DisplayManagerCaptureStatusAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerCaptureStatusAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerCaptureStatusAgent() = default;

    virtual void NotifyCaptureStatusChanged(bool isCapture) override
    {
        pImpl_->NotifyCaptureStatusChanged(isCapture);
    }
private:
    sptr<Impl> pImpl_;
};

class DisplayManager::Impl::DisplayManagerDisplayUpdateAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerDisplayUpdateAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerDisplayUpdateAgent() = default;

    virtual void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info) override
    {
        pImpl_->NotifyDisplayChangeInfoChanged(info);
    }
private:
    sptr<Impl> pImpl_;
};


class DisplayManager::Impl::DisplayManagerDisplayModeAgent : public DisplayManagerAgentDefault {
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

class DisplayManager::Impl::DisplayManagerAvailableAreaAgent : public DisplayManagerAgentDefault {
public:
    explicit DisplayManagerAvailableAreaAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~DisplayManagerAvailableAreaAgent() = default;

    virtual void NotifyAvailableAreaChanged(DMRect area) override
    {
        pImpl_->NotifyAvailableAreaChanged(area);
    }
private:
    sptr<Impl> pImpl_;
};

bool DisplayManager::Impl::CheckRectValid(const Media::Rect& rect, int32_t oriHeight, int32_t oriWidth) const
{
    if (rect.left < 0) {
        return false;
    }
    if (rect.top < 0) {
        return false;
    }
    if (rect.width < 0) {
        return false;
    }
    if (rect.height < 0) {
        return false;
    }
    if (rect.width + rect.left > oriWidth) {
        return false;
    }
    if (rect.height + rect.top > oriHeight) {
        return false;
    }
    return true;
}

bool DisplayManager::Impl::CheckSizeValid(const Media::Size& size, int32_t oriHeight, int32_t oriWidth) const
{
    if (size.width < 0) {
        return false;
    }
    if (size.height < 0) {
        return false;
    }
    if (size.width > MAX_RESOLUTION_SIZE_SCREENSHOT) {
        return false;
    }
    if (size.height > MAX_RESOLUTION_SIZE_SCREENSHOT) {
        return false;
    }
    return true;
}

void DisplayManager::Impl::ClearDisplayStateCallback()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    WLOGFD("[UL_POWER]Clear displaystatecallback enter");
    displayStateCallback_ = nullptr;
    if (displayStateAgent_ != nullptr) {
        WLOGFI("[UL_POWER]UnregisterDisplayManagerAgent enter and displayStateAgent_ is cleared");
        SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(displayStateAgent_,
            DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
        displayStateAgent_ = nullptr;
    }
}

void DisplayManager::Impl::ClearFoldStatusCallback()
{
    DMError res = DMError::DM_OK;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (foldStatusListenerAgent_ != nullptr) {
        res = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(foldStatusListenerAgent_,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
        foldStatusListenerAgent_ = nullptr;
        WLOGI("ClearFoldStatusCallback foldStatusListenerAgent_ is nullptr !");
    }
    if (res != DMError::DM_OK) {
        WLOGFW("UnregisterDisplayManagerAgent FOLD_STATUS_CHANGED_LISTENER failed !");
    }
}

void DisplayManager::Impl::ClearFoldAngleCallback()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (foldAngleListenerAgent_ == nullptr) {
        return;
    }
    DMError res = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
        foldAngleListenerAgent_, DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER);
    foldAngleListenerAgent_ = nullptr;
    if (res != DMError::DM_OK) {
        WLOGFE("ClearFoldAngleCallback FOLD_ANGLE_CHANGED_LISTENER failed !");
    } else {
        WLOGI("ClearFoldAngleCallback foldAngleListenerAgent_!");
    }
}

void DisplayManager::Impl::ClearCaptureStatusCallback()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (captureStatusListenerAgent_ == nullptr) {
        return;
    }
    DMError res = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
        captureStatusListenerAgent_, DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
    captureStatusListenerAgent_ = nullptr;
    if (res != DMError::DM_OK) {
        WLOGFE("ClearCaptureStatusCallback FOLD_ANGLE_CHANGED_LISTENER failed !");
    } else {
        WLOGI("ClearCaptureStatusCallback captureStatusListenerAgent_!");
    }
}

void DisplayManager::Impl::ClearDisplayModeCallback()
{
    DMError res = DMError::DM_OK;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (displayModeListenerAgent_ != nullptr) {
        res = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(displayModeListenerAgent_,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
        displayModeListenerAgent_ = nullptr;
        WLOGI("ClearDisplayStateCallback displayModeListenerAgent_ is nullptr !");
    }
    if (res != DMError::DM_OK) {
        WLOGFW("UnregisterDisplayManagerAgent DISPLAY_MODE_CHANGED_LISTENER failed !");
    }
}

void DisplayManager::Impl::Clear()
{
    WLOGFI("Clear displaymanager listener");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError res = DMError::DM_OK;
    if (displayManagerListener_ != nullptr) {
        res = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            displayManagerListener_, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    }
    displayManagerListener_ = nullptr;
    if (res != DMError::DM_OK) {
        WLOGFW("UnregisterDisplayManagerAgent DISPLAY_EVENT_LISTENER failed !");
    }
    res = DMError::DM_OK;
    if (powerEventListenerAgent_ != nullptr) {
        res = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            powerEventListenerAgent_, DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    }
    powerEventListenerAgent_ = nullptr;
    if (res != DMError::DM_OK) {
        WLOGFW("UnregisterDisplayManagerAgent DISPLAY_POWER_EVENT_LISTENER failed !");
    }
    ClearDisplayStateCallback();
    ClearFoldStatusCallback();
    ClearFoldAngleCallback();
    ClearCaptureStatusCallback();
    ClearDisplayModeCallback();
}

DisplayManager::Impl::~Impl()
{
    Clear();
}

DisplayManager::DisplayManager() : pImpl_(new Impl(mutex_))
{
    WLOGFD("Create displaymanager instance");
    g_dmIsDestroyed = false;
}

DisplayManager::~DisplayManager()
{
    WLOGFI("Destroy displaymanager instance");
    g_dmIsDestroyed = true;
}

DisplayId DisplayManager::GetDefaultDisplayId()
{
    auto info = SingletonContainer::Get<DisplayManagerAdapter>().GetDefaultDisplayInfo();
    if (info == nullptr) {
        return DISPLAY_ID_INVALID;
    }
    return info->GetDisplayId();
}

sptr<Display> DisplayManager::Impl::GetDefaultDisplay()
{
    auto displayInfo = SingletonContainer::Get<DisplayManagerAdapter>().GetDefaultDisplayInfo();
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

sptr<Display> DisplayManager::Impl::GetDefaultDisplaySync()
{
    static std::chrono::steady_clock::time_point lastRequestTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRequestTime).count();
    if (defaultDisplayId_ != DISPLAY_ID_INVALID && interval < MAX_INTERVAL_US) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = displayMap_.find(defaultDisplayId_);
        if (iter != displayMap_.end()) {
            return displayMap_[defaultDisplayId_];
        }
    }

    uint32_t retryTimes = 0;
    sptr<DisplayInfo> displayInfo = nullptr;
    while (retryTimes < MAX_RETRY_NUM) {
        displayInfo = SingletonContainer::Get<DisplayManagerAdapter>().GetDefaultDisplayInfo();
        if (displayInfo != nullptr) {
            break;
        }
        retryTimes++;
        WLOGFW("Current get display info is null, retry %{public}u times", retryTimes);
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_MS));
    }
    if (retryTimes >= MAX_RETRY_NUM || displayInfo == nullptr) {
        WLOGFE("Get display info failed, please check whether the onscreenchange event is triggered");
        return nullptr;
    }

    auto displayId = displayInfo->GetDisplayId();
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateDisplayInfoLocked(displayInfo)) {
        displayMap_.erase(displayId);
        return nullptr;
    }
    lastRequestTime = currentTime;
    defaultDisplayId_ = displayId;
    return displayMap_[displayId];
}

sptr<Display> DisplayManager::Impl::GetDisplayById(DisplayId displayId)
{
    WLOGFD("GetDisplayById start, displayId: %{public}" PRIu64" ", displayId);
    auto currentTime = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto lastRequestIter = displayUptateTimeMap_.find(displayId);
        if (displayId != DISPLAY_ID_INVALID && lastRequestIter != displayUptateTimeMap_.end()) {
            auto interval = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRequestIter->second)
                .count();
            if (interval < MAX_INTERVAL_US) {
                auto iter = displayMap_.find(displayId);
                if (iter != displayMap_.end()) {
                    WLOGFW("update display from cache, Id: %{public}" PRIu64" ", displayId);
                    return displayMap_[displayId];
                }
            }
        }
    }
    sptr<DisplayInfo> displayInfo = SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayInfo(displayId);
    if (displayInfo == nullptr) {
        WLOGFW("display null id : %{public}" PRIu64" ", displayId);
        return nullptr;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateDisplayInfoLocked(displayInfo)) {
        displayMap_.erase(displayId);
        //map erase函数删除不存在key行为安全
        displayUptateTimeMap_.erase(displayId);
        return nullptr;
    }

    displayUptateTimeMap_[displayId] = currentTime;
    return displayMap_[displayId];
}

sptr<Display> DisplayManager::GetDisplayById(DisplayId displayId)
{
    if (g_dmIsDestroyed) {
        WLOGFI("DM has been destructed");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return pImpl_->GetDisplayById(displayId);
}

sptr<Display> DisplayManager::GetDisplayByScreen(ScreenId screenId)
{
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("screenId is invalid.");
        return nullptr;
    }
    sptr<Display> display = pImpl_->GetDisplayByScreenId(screenId);
    if (display == nullptr) {
        WLOGFE("get display by screenId failed. screen %{public}" PRIu64"", screenId);
    }
    return display;
}

sptr<Display> DisplayManager::Impl::GetDisplayByScreenId(ScreenId screenId)
{
    sptr<DisplayInfo> displayInfo = SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayInfoByScreenId(screenId);
    if (displayInfo == nullptr) {
        WLOGFE("get display by screenId: displayInfo is null");
        return nullptr;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("get display by screenId: invalid displayInfo");
        return nullptr;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateDisplayInfoLocked(displayInfo)) {
        displayMap_.erase(displayId);
        return nullptr;
    }
    return displayMap_[displayId];
}

std::shared_ptr<Media::PixelMap> DisplayManager::GetScreenshot(DisplayId displayId, DmErrorCode* errorCode)
{
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("displayId invalid!");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> screenShot =
        SingletonContainer::Get<DisplayManagerAdapter>().GetDisplaySnapshot(displayId, errorCode);
    if (screenShot == nullptr) {
        WLOGFE("DisplayManager::GetScreenshot failed!");
        return nullptr;
    }

    return screenShot;
}

std::shared_ptr<Media::PixelMap> DisplayManager::GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode)
{
    std::unique_lock<std::mutex> lock(snapBypickerMutex, std::defer_lock);
    if (!lock.try_lock()) {
        WLOGFE("DisplayManager::GetSnapshotByPicker try_lock failed!");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> screenShot =
        SingletonContainer::Get<DisplayManagerAdapter>().GetSnapshotByPicker(rect, errorCode);
    lock.unlock();
    if (screenShot == nullptr) {
        WLOGFE("DisplayManager::GetSnapshotByPicker failed!");
        return nullptr;
    }
    WLOGFI("snapshot area left:%{public}d, top:%{public}d, width:%{public}d, height:%{public}d",
        rect.left, rect.top, rect.width, rect.height);
    // create crop pixel map
    if (rect.width == 0 || rect.height == 0) {
        WLOGFE("width or height is invalid!");
        return nullptr;
    }
    Media::InitializationOptions opt;
    opt.size.width = rect.width;
    opt.size.height = rect.height;
    opt.scaleMode = Media::ScaleMode::FIT_TARGET_SIZE;
    opt.editable = false;
    auto pixelMap = Media::PixelMap::Create(*screenShot, rect, opt);
    if (pixelMap == nullptr) {
        WLOGFE("Media pixel map create failed");
        return nullptr;
    }
    return pixelMap;
}

std::shared_ptr<Media::PixelMap> DisplayManager::GetScreenshot(DisplayId displayId, const Media::Rect &rect,
    const Media::Size &size, int rotation, DmErrorCode* errorCode)
{
    std::shared_ptr<Media::PixelMap> screenShot = GetScreenshot(displayId, errorCode);
    if (screenShot == nullptr) {
        WLOGFE("DisplayManager::GetScreenshot failed!");
        return nullptr;
    }

    // check parameters
    int32_t oriHeight = screenShot->GetHeight();
    int32_t oriWidth = screenShot->GetWidth();
    if (!pImpl_->CheckRectValid(rect, oriHeight, oriWidth)) {
        WLOGFE("rect invalid! left %{public}d, top %{public}d, w %{public}d, h %{public}d",
            rect.left, rect.top, rect.width, rect.height);
        return nullptr;
    }
    if (!pImpl_->CheckSizeValid(size, oriHeight, oriWidth)) {
        WLOGFE("size invalid! w %{public}d, h %{public}d", rect.width, rect.height);
        return nullptr;
    }

    // create crop dest pixelmap
    Media::InitializationOptions opt;
    opt.size.width = size.width;
    opt.size.height = size.height;
    opt.scaleMode = Media::ScaleMode::FIT_TARGET_SIZE;
    opt.editable = false;
    auto pixelMap = Media::PixelMap::Create(*screenShot, rect, opt);
    if (pixelMap == nullptr) {
        WLOGFE("Media::PixelMap::Create failed!");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> dstScreenshot(pixelMap.release());

    return dstScreenshot;
}

sptr<Display> DisplayManager::GetDefaultDisplay()
{
    return pImpl_->GetDefaultDisplay();
}

void DisplayManager::AddDisplayIdFromAms(DisplayId displayId, const wptr<IRemoteObject>& abilityToken)
{
    if (abilityToken == nullptr || displayId == DISPLAY_ID_INVALID) {
        WLOGFE("abilityToken is nullptr or display id invalid.");
        return;
    }
    std::lock_guard<std::mutex> lock(displayOperateMutex_);
    auto iter = std::find_if(displayIdList_.begin(), displayIdList_.end(),
        [displayId, abilityToken](const auto &item) -> bool {
            return item.first == abilityToken && item.second == displayId;
    });
    if (iter != displayIdList_.end()) {
        WLOGFI("abilityToken and display[%{public}" PRIu64"] has been added.", displayId);
    } else {
        displayIdList_.push_back(std::make_pair(abilityToken, displayId));
    }
    ShowDisplayIdList(false);
}

void DisplayManager::RemoveDisplayIdFromAms(const wptr<IRemoteObject>& abilityToken)
{
    if (abilityToken == nullptr) {
        WLOGFE("abilityToken is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(displayOperateMutex_);
    if (displayIdList_.empty()) {
        WLOGFI("displayIdList_ is empty.");
        return;
    }
    auto iter = std::find_if(displayIdList_.begin(), displayIdList_.end(),
        [abilityToken](const auto &item) -> bool {
            return item.first == abilityToken;
    });
    if (iter != displayIdList_.end()) {
        displayIdList_.erase(iter);
    }
    ShowDisplayIdList(false);
}

DisplayId DisplayManager::GetCallingAbilityDisplayId()
{
    DisplayId displayId = DISPLAY_ID_INVALID;
    std::lock_guard<std::mutex> lock(displayOperateMutex_);
    if (displayIdList_.empty()) {
        WLOGFI("displayIdList is empty.");
        return displayId;
    }
    int displayCount = 0;
    for (const auto &iter : displayIdList_) {
        if (displayId == DISPLAY_ID_INVALID || displayId != iter.second) {
            displayCount++;
        }
        displayId = iter.second;
        if (displayCount > 1) {
            break;
        }
    }
    ShowDisplayIdList(false);
    return displayCount == 1 ? displayId : DISPLAY_ID_INVALID;
}

void DisplayManager::ShowDisplayIdList(bool isShowLog)
{
    std::ostringstream oss;
    oss << "current display id list:[";
    for (const auto &iter : displayIdList_) {
        oss << iter.second << ",";
    }
    if (isShowLog) {
        WLOGFI("%{public}s]", oss.str().c_str());
    } else {
        WLOGFD("%{public}s]", oss.str().c_str());
    }
}

sptr<Display> DisplayManager::GetDefaultDisplaySync(bool isFromNapi)
{
    if (isFromNapi) {
        sptr<Display> display = nullptr;
        DisplayId displayId = GetCallingAbilityDisplayId();
        if (displayId != DISPLAY_ID_INVALID) {
            WLOGFD("displayId:%{public}" PRIu64, displayId);
            display = pImpl_->GetDisplayById(displayId);
        }
        if (display != nullptr) {
            return display;
        } else {
            WLOGFI("get displayId:%{public}" PRIu64" info nullptr.", displayId);
        }
    }
    return pImpl_->GetDefaultDisplaySync();
}

std::vector<DisplayId> DisplayManager::GetAllDisplayIds()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetAllDisplayIds();
}

std::vector<DisplayPhysicalResolution> DisplayManager::Impl::GetAllDisplayPhysicalResolution()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetAllDisplayPhysicalResolution();
}

std::vector<DisplayPhysicalResolution> DisplayManager::GetAllDisplayPhysicalResolution()
{
    return pImpl_->GetAllDisplayPhysicalResolution();
}

std::vector<sptr<Display>> DisplayManager::GetAllDisplays()
{
    WLOGFD("GetAllDisplays start");
    std::vector<sptr<Display>> res;
    auto displayIds = GetAllDisplayIds();
    for (auto displayId : displayIds) {
        const sptr<Display> display = GetDisplayById(displayId);
        if (display != nullptr) {
            res.emplace_back(display);
        } else {
            WLOGFE("DisplayManager::GetAllDisplays display %" PRIu64" nullptr!", displayId);
        }
    }
    return res;
}

DMError DisplayManager::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    return pImpl_->HasPrivateWindow(displayId, hasPrivateWindow);
}

DMError DisplayManager::Impl::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().HasPrivateWindow(displayId, hasPrivateWindow);
}

bool DisplayManager::IsFoldable()
{
    return pImpl_->IsFoldable();
}

bool DisplayManager::Impl::IsFoldable()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().IsFoldable();
}

bool DisplayManager::IsCaptured()
{
    return pImpl_->IsCaptured();
}

bool DisplayManager::Impl::IsCaptured()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().IsCaptured();
}

FoldStatus DisplayManager::GetFoldStatus()
{
    return pImpl_->GetFoldStatus();
}

FoldStatus DisplayManager::Impl::GetFoldStatus()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetFoldStatus();
}

FoldDisplayMode DisplayManager::GetFoldDisplayMode()
{
    return pImpl_->GetFoldDisplayMode();
}

FoldDisplayMode DisplayManager::Impl::GetFoldDisplayMode()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetFoldDisplayMode();
}

void DisplayManager::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    pImpl_->SetFoldDisplayMode(mode);
}

DMError DisplayManager::SetFoldDisplayModeFromJs(const FoldDisplayMode mode)
{
    return pImpl_->SetFoldDisplayModeFromJs(mode);
}

void DisplayManager::SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX, float pivotY)
{
    pImpl_->SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
}

void DisplayManager::Impl::SetDisplayScale(ScreenId screenId,
    float scaleX, float scaleY, float pivotX, float pivotY)
{
    SingletonContainer::Get<DisplayManagerAdapter>().SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
}

void DisplayManager::Impl::SetFoldDisplayMode(const FoldDisplayMode mode)
{
    SingletonContainer::Get<DisplayManagerAdapter>().SetFoldDisplayMode(mode);
}

DMError DisplayManager::Impl::SetFoldDisplayModeFromJs(const FoldDisplayMode mode)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().SetFoldDisplayModeFromJs(mode);
}

void DisplayManager::SetFoldStatusLocked(bool locked)
{
    pImpl_->SetFoldStatusLocked(locked);
}

DMError DisplayManager::SetFoldStatusLockedFromJs(bool locked)
{
    return pImpl_->SetFoldStatusLockedFromJs(locked);
}

void DisplayManager::Impl::SetFoldStatusLocked(bool locked)
{
    SingletonContainer::Get<DisplayManagerAdapter>().SetFoldStatusLocked(locked);
}

DMError DisplayManager::Impl::SetFoldStatusLockedFromJs(bool locked)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().SetFoldStatusLockedFromJs(locked);
}

sptr<FoldCreaseRegion> DisplayManager::GetCurrentFoldCreaseRegion()
{
    return pImpl_->GetCurrentFoldCreaseRegion();
}

sptr<FoldCreaseRegion> DisplayManager::Impl::GetCurrentFoldCreaseRegion()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetCurrentFoldCreaseRegion();
}

DMError DisplayManager::Impl::RegisterDisplayListener(sptr<IDisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (displayManagerListener_ == nullptr) {
        displayManagerListener_ = new DisplayManagerListener(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            displayManagerListener_,
            DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayManagerAgent failed !");
        displayManagerListener_ = nullptr;
    } else {
        displayListeners_.insert(listener);
    }
    return ret;
}

void DisplayManager::Impl::NotifyPrivateWindowStateChanged(bool hasPrivate)
{
    std::set<sptr<IPrivateWindowListener>> privateWindowListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        privateWindowListeners = privateWindowListeners_;
    }
    for (auto& listener : privateWindowListeners) {
        listener->OnPrivateWindow(hasPrivate);
    }
}

void DisplayManager::Impl::NotifyPrivateStateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList)
{
    std::set<sptr<IPrivateWindowListChangeListener>> privateWindowListChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        privateWindowListChangeListeners = privateWindowListChangeListeners_;
    }
    for (auto& listener : privateWindowListChangeListeners) {
        listener->OnPrivateWindowListChange(id, privacyWindowList);
    }
}

DMError DisplayManager::RegisterPrivateWindowListener(sptr<IPrivateWindowListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterPrivateWindowListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterPrivateWindowListener(listener);
}

DMError DisplayManager::UnregisterPrivateWindowListener(sptr<IPrivateWindowListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterPrivateWindowListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterPrivateWindowListener(listener);
}

DMError DisplayManager::RegisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterPrivateWindowListChangeListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterPrivateWindowListChangeListener(listener);
}

DMError DisplayManager::UnregisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterPrivateWindowListChangeListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterPrivateWindowListChangeListener(listener);
}

DMError DisplayManager::Impl::RegisterPrivateWindowListener(sptr<IPrivateWindowListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (privateWindowListenerAgent_ == nullptr) {
        privateWindowListenerAgent_ = new DisplayManagerPrivateWindowAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            privateWindowListenerAgent_,
            DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayManagerAgent failed !");
        privateWindowListenerAgent_ = nullptr;
    } else {
        WLOGI("privateWindowListener register success");
        privateWindowListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::Impl::UnregisterPrivateWindowListener(sptr<IPrivateWindowListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(privateWindowListeners_.begin(), privateWindowListeners_.end(), listener);
    if (iter == privateWindowListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    privateWindowListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (privateWindowListeners_.empty() && privateWindowListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            privateWindowListenerAgent_,
            DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER);
        privateWindowListenerAgent_ = nullptr;
    }
    return ret;
}

DMError DisplayManager::Impl::RegisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (privateWindowListChangeListenerAgent_ == nullptr) {
        privateWindowListChangeListenerAgent_ = new DisplayManagerPrivateWindowListAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            privateWindowListChangeListenerAgent_,
            DisplayManagerAgentType::PRIVATE_WINDOW_LIST_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayManagerAgent failed !");
        privateWindowListChangeListenerAgent_ = nullptr;
    } else {
        WLOGI("privateWindowListChangeListener register success");
        privateWindowListChangeListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::Impl::UnregisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(privateWindowListChangeListeners_.begin(), privateWindowListChangeListeners_.end(), listener);
    if (iter == privateWindowListChangeListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    privateWindowListChangeListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (privateWindowListChangeListeners_.empty() && privateWindowListChangeListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            privateWindowListChangeListenerAgent_,
            DisplayManagerAgentType::PRIVATE_WINDOW_LIST_LISTENER);
        privateWindowListChangeListenerAgent_ = nullptr;
    }
    return ret;
}

DMError DisplayManager::RegisterDisplayListener(sptr<IDisplayListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterDisplayListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayListener(listener);
}

DMError DisplayManager::Impl::UnregisterDisplayListener(sptr<IDisplayListener> listener)
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
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            displayManagerListener_,
            DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
        displayManagerListener_ = nullptr;
    }
    return ret;
}

DMError DisplayManager::UnregisterDisplayListener(sptr<IDisplayListener> listener)
{
    if (listener == nullptr) {
        WLOGFW("UnregisterDisplayListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayListener(listener);
}

DMError DisplayManager::Impl::RegisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (powerEventListenerAgent_ == nullptr) {
        powerEventListenerAgent_ = new DisplayManagerAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            powerEventListenerAgent_,
            DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayManagerAgent failed !");
        powerEventListenerAgent_ = nullptr;
    } else {
        powerEventListeners_.insert(listener);
    }
    WLOGFD("RegisterDisplayPowerEventListener end");
    return ret;
}

DMError DisplayManager::RegisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayPowerEventListener(listener);
}

DMError DisplayManager::Impl::UnregisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(powerEventListeners_.begin(), powerEventListeners_.end(), listener);
    if (iter == powerEventListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    powerEventListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (powerEventListeners_.empty() && powerEventListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            powerEventListenerAgent_,
            DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
        powerEventListenerAgent_ = nullptr;
    }
    WLOGFD("UnregisterDisplayPowerEventListener end");
    return ret;
}

DMError DisplayManager::UnregisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayPowerEventListener(listener);
}

DMError DisplayManager::Impl::RegisterScreenshotListener(sptr<IScreenshotListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (screenshotListenerAgent_ == nullptr) {
        screenshotListenerAgent_ = new DisplayManagerScreenshotAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            screenshotListenerAgent_,
            DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayManagerAgent failed !");
        screenshotListenerAgent_ = nullptr;
    } else {
        screenshotListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::RegisterScreenshotListener(sptr<IScreenshotListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterScreenshotListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenshotListener(listener);
}

DMError DisplayManager::Impl::UnregisterScreenshotListener(sptr<IScreenshotListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(screenshotListeners_.begin(), screenshotListeners_.end(), listener);
    if (iter == screenshotListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenshotListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (screenshotListeners_.empty() && screenshotListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            screenshotListenerAgent_,
            DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
        screenshotListenerAgent_ = nullptr;
    }
    return ret;
}

DMError DisplayManager::UnregisterScreenshotListener(sptr<IScreenshotListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterScreenshotListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenshotListener(listener);
}

void DisplayManager::Impl::NotifyFoldAngleChanged(std::vector<float> foldAngles)
{
    std::set<sptr<IFoldAngleListener>> foldAngleListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        foldAngleListeners = foldAngleListeners_;
    }
    for (auto& listener : foldAngleListeners) {
        listener->OnFoldAngleChanged(foldAngles);
    }
}

DMError DisplayManager::RegisterFoldAngleListener(sptr<IFoldAngleListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("IFoldAngleListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterFoldAngleListener(listener);
}

DMError DisplayManager::Impl::RegisterFoldAngleListener(sptr<IFoldAngleListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (foldAngleListenerAgent_ == nullptr) {
        foldAngleListenerAgent_ = new DisplayManagerFoldAngleAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            foldAngleListenerAgent_,
            DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterFoldAngleListener failed !");
        foldAngleListenerAgent_ = nullptr;
    } else {
        WLOGD("IFoldAngleListener register success");
        foldAngleListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::UnregisterFoldAngleListener(sptr<IFoldAngleListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterFoldAngleListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterFoldAngleListener(listener);
}

DMError DisplayManager::Impl::UnregisterFoldAngleListener(sptr<IFoldAngleListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(foldAngleListeners_.begin(), foldAngleListeners_.end(), listener);
    if (iter == foldAngleListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    foldAngleListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (foldAngleListeners_.empty() && foldAngleListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            foldAngleListenerAgent_,
            DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER);
        foldAngleListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManager::Impl::NotifyCaptureStatusChanged(bool isCapture)
{
    std::set<sptr<ICaptureStatusListener>> captureStatusListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        captureStatusListeners = captureStatusListeners_;
    }
    for (auto& listener : captureStatusListeners) {
        listener->OnCaptureStatusChanged(isCapture);
    }
}

DMError DisplayManager::RegisterCaptureStatusListener(sptr<ICaptureStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("ICaptureStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterCaptureStatusListener(listener);
}

DMError DisplayManager::Impl::RegisterCaptureStatusListener(sptr<ICaptureStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (captureStatusListenerAgent_ == nullptr) {
        captureStatusListenerAgent_ = new DisplayManagerCaptureStatusAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            captureStatusListenerAgent_,
            DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFE("RegisterCaptureStatusListener failed !");
        captureStatusListenerAgent_ = nullptr;
    } else {
        WLOGD("ICaptureStatusListener register success");
        captureStatusListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::UnregisterCaptureStatusListener(sptr<ICaptureStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterCaptureStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterCaptureStatusListener(listener);
}

DMError DisplayManager::Impl::UnregisterCaptureStatusListener(sptr<ICaptureStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(captureStatusListeners_.begin(), captureStatusListeners_.end(), listener);
    if (iter == captureStatusListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    captureStatusListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (captureStatusListeners_.empty() && captureStatusListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            captureStatusListenerAgent_,
            DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
        captureStatusListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManager::Impl::NotifyFoldStatusChanged(FoldStatus foldStatus)
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

DMError DisplayManager::RegisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("IFoldStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterFoldStatusListener(listener);
}

DMError DisplayManager::Impl::RegisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (foldStatusListenerAgent_ == nullptr) {
        foldStatusListenerAgent_ = new DisplayManagerFoldStatusAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            foldStatusListenerAgent_,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterFoldStatusListener failed !");
        foldStatusListenerAgent_ = nullptr;
    } else {
        WLOGD("IFoldStatusListener register success");
        foldStatusListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterFoldStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterFoldStatusListener(listener);
}

DMError DisplayManager::Impl::UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener)
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
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            foldStatusListenerAgent_,
            DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
        foldStatusListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManager::Impl::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    std::set<sptr<IDisplayUpdateListener>> displayUpdateListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displayUpdateListeners = displayUpdateListeners_;
    }
    for (auto& listener : displayUpdateListeners) {
        listener->OnDisplayUpdate(info);
    }
}

DMError DisplayManager::RegisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("IDisplayUpdateListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayUpdateListener(listener);
}

DMError DisplayManager::Impl::RegisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (displayUpdateListenerAgent_ == nullptr) {
        displayUpdateListenerAgent_ = new DisplayManagerDisplayUpdateAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            displayUpdateListenerAgent_,
            DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayUpdateListener failed !");
        displayUpdateListenerAgent_ = nullptr;
    } else {
        WLOGI("IDisplayUpdateListener register success");
        displayUpdateListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::UnregisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterDisplayUpdateListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayUpdateListener(listener);
}

DMError DisplayManager::Impl::UnregisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(displayUpdateListeners_.begin(), displayUpdateListeners_.end(), listener);
    if (iter == displayUpdateListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    displayUpdateListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (displayUpdateListeners_.empty() && displayUpdateListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            displayUpdateListenerAgent_,
            DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER);
        displayUpdateListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManager::Impl::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    std::set<sptr<IDisplayModeListener>> displayModeListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displayModeListeners = displayModeListeners_;
    }
    auto displayInfo = SingletonContainer::Get<DisplayManagerAdapter>().GetDefaultDisplayInfo();
    NotifyDisplayChange(displayInfo);
    for (auto& listener : displayModeListeners) {
        listener->OnDisplayModeChanged(displayMode);
    }
}

void DisplayManager::Impl::NotifyAvailableAreaChanged(DMRect rect)
{
    std::set<sptr<IAvailableAreaListener>> availableAreaListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        availableAreaListeners = availableAreaListeners_;
    }
    for (auto& listener : availableAreaListeners) {
        listener->OnAvailableAreaChanged(rect);
    }
}

DMError DisplayManager::RegisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("IDisplayModeListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayModeListener(listener);
}

DMError DisplayManager::Impl::RegisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (displayModeListenerAgent_ == nullptr) {
        displayModeListenerAgent_ = new DisplayManagerDisplayModeAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            displayModeListenerAgent_,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterDisplayModeListener failed !");
        displayModeListenerAgent_ = nullptr;
    } else {
        WLOGD("IDisplayModeListener register success");
        displayModeListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterPrivateWindowListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayModeListener(listener);
}

DMError DisplayManager::Impl::UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener)
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
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            displayModeListenerAgent_,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
        displayModeListenerAgent_ = nullptr;
    }
    return ret;
}

DMError DisplayManager::RegisterAvailableAreaListener(sptr<IAvailableAreaListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterAvailableAreaListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterAvailableAreaListener(listener);
}

DMError DisplayManager::Impl::RegisterAvailableAreaListener(sptr<IAvailableAreaListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError ret = DMError::DM_OK;
    if (availableAreaListenerAgent_ == nullptr) {
        availableAreaListenerAgent_ = new DisplayManagerAvailableAreaAgent(this);
        ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
            availableAreaListenerAgent_,
            DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER);
    }
    if (ret != DMError::DM_OK) {
        WLOGFW("RegisterAvailableAreaListener failed !");
        availableAreaListenerAgent_ = nullptr;
    } else {
        WLOGD("IAvailableAreaListener register success");
        availableAreaListeners_.insert(listener);
    }
    return ret;
}

DMError DisplayManager::UnregisterAvailableAreaListener(sptr<IAvailableAreaListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterPrivateWindowListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterAvailableAreaListener(listener);
}

DMError DisplayManager::Impl::UnregisterAvailableAreaListener(sptr<IAvailableAreaListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(availableAreaListeners_.begin(), availableAreaListeners_.end(), listener);
    if (iter == availableAreaListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    availableAreaListeners_.erase(iter);
    DMError ret = DMError::DM_OK;
    if (availableAreaListeners_.empty() && availableAreaListenerAgent_ != nullptr) {
        ret = SingletonContainer::Get<DisplayManagerAdapter>().UnregisterDisplayManagerAgent(
            availableAreaListenerAgent_,
            DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER);
        availableAreaListenerAgent_ = nullptr;
    }
    return ret;
}

void DisplayManager::Impl::NotifyScreenshot(sptr<ScreenshotInfo> info)
{
    WLOGFI("NotifyScreenshot trigger:[%{public}s] displayId:%{public}" PRIu64" size:%{public}zu",
        info->GetTrigger().c_str(), info->GetDisplayId(), screenshotListeners_.size());
    std::set<sptr<IScreenshotListener>> screenshotListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        screenshotListeners = screenshotListeners_;
    }
    for (auto& listener : screenshotListeners) {
        listener->OnScreenshot(*info);
    }
}

void DisplayManager::Impl::NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status)
{
    WLOGFD("[UL_POWER]NotifyDisplayPowerEvent event:%{public}u, status:%{public}u, size:%{public}zu", event, status,
        powerEventListeners_.size());
    std::set<sptr<IDisplayPowerEventListener>> powerEventListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        powerEventListeners = powerEventListeners_;
    }
    for (auto& listener : powerEventListeners) {
        listener->OnDisplayPowerEvent(event, status);
    }
}

void DisplayManager::Impl::NotifyDisplayStateChanged(DisplayId id, DisplayState state)
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

void DisplayManager::Impl::NotifyDisplayCreate(sptr<DisplayInfo> info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UpdateDisplayInfoLocked(info);
}

void DisplayManager::Impl::NotifyDisplayDestroy(DisplayId displayId)
{
    WLOGFD("displayId:%{public}" PRIu64".", displayId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    displayMap_.erase(displayId);
}

void DisplayManager::Impl::NotifyDisplayChange(sptr<DisplayInfo> displayInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UpdateDisplayInfoLocked(displayInfo);
}

bool DisplayManager::Impl::UpdateDisplayInfoLocked(sptr<DisplayInfo> displayInfo)
{
    if (displayInfo == nullptr) {
        WLOGFW("displayInfo is null");
        return false;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    WLOGFD("displayId:%{public}" PRIu64".", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGFE("displayId is invalid.");
        return false;
    }
    auto iter = displayMap_.find(displayId);
    if (iter != displayMap_.end() && iter->second != nullptr) {
        WLOGFD("display Info Updated: %{public}s",
            GetDisplayInfoSrting(displayInfo).c_str());
        iter->second->UpdateDisplayInfo(displayInfo);
        return true;
    }
    sptr<Display> display = new (std::nothrow) Display("", displayInfo);
    if (display == nullptr) {
        WLOGFE("malloc display failed");
        return false;
    }
    displayMap_[displayId] = display;
    return true;
}

std::string DisplayManager::Impl::GetDisplayInfoSrting(sptr<DisplayInfo> displayInfo)
{
    if (displayInfo == nullptr) {
        WLOGFE("displayInfo nullptr.");
        return "";
    }
    std::ostringstream oss;
    oss <<  "Display ID: " << displayInfo->GetDisplayId() << ", ";
    oss <<  "Name: " << displayInfo->GetName() << ", ";
    oss <<  "RefreshRate: " << displayInfo->GetRefreshRate() << ", ";
    oss <<  "VirtualPixelRatio: " << displayInfo->GetVirtualPixelRatio() << ", ";
    oss <<  "DensityInCurResolution: " << displayInfo->GetDensityInCurResolution() << ", ";
    oss <<  "DefaultVirtualPixelRatio: " << displayInfo->GetDefaultVirtualPixelRatio() << ", ";
    oss <<  "Rotation: " << static_cast<int32_t>(displayInfo->GetRotation());
    return oss.str();
}

bool DisplayManager::WakeUpBegin(PowerStateChangeReason reason)
{
    WLOGFD("[UL_POWER]WakeUpBegin start, reason:%{public}u", reason);
    return SingletonContainer::Get<DisplayManagerAdapter>().WakeUpBegin(reason);
}

bool DisplayManager::WakeUpEnd()
{
    WLOGFD("[UL_POWER]WakeUpEnd start");
    return SingletonContainer::Get<DisplayManagerAdapter>().WakeUpEnd();
}

bool DisplayManager::SuspendBegin(PowerStateChangeReason reason)
{
    // dms->wms notify other windows to hide
    WLOGFD("[UL_POWER]SuspendBegin start, reason:%{public}u", reason);
    return SingletonContainer::Get<DisplayManagerAdapter>().SuspendBegin(reason);
}

bool DisplayManager::SuspendEnd()
{
    WLOGFD("[UL_POWER]SuspendEnd start");
    return SingletonContainer::Get<DisplayManagerAdapter>().SuspendEnd();
}

bool DisplayManager::Impl::SetDisplayState(DisplayState state, DisplayStateCallback callback)
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
            ret = SingletonContainer::Get<DisplayManagerAdapter>().RegisterDisplayManagerAgent(
                displayStateAgent_,
                DisplayManagerAgentType::DISPLAY_STATE_LISTENER) == DMError::DM_OK;
        }
    }
    ret = ret && SingletonContainer::Get<DisplayManagerAdapter>().SetDisplayState(state);
    if (!ret) {
        ClearDisplayStateCallback();
    }
    return ret;
}

bool DisplayManager::SetDisplayState(DisplayState state, DisplayStateCallback callback)
{
    return pImpl_->SetDisplayState(state, callback);
}

DisplayState DisplayManager::GetDisplayState(DisplayId displayId)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayState(displayId);
}

bool DisplayManager::TryToCancelScreenOff()
{
    WLOGFD("[UL_POWER]TryToCancelScreenOff start");
    return SingletonContainer::Get<DisplayManagerAdapter>().TryToCancelScreenOff();
}

bool DisplayManager::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
    WLOGFI("[UL_POWER]ScreenId:%{public}" PRIu64", level:%{public}u,", screenId, level);
    RSInterfaces::GetInstance().SetScreenBacklight(screenId, level);
    return true;
}

uint32_t DisplayManager::GetScreenBrightness(uint64_t screenId) const
{
    uint32_t level = static_cast<uint32_t>(RSInterfaces::GetInstance().GetScreenBacklight(screenId));
    WLOGFI("GetScreenBrightness screenId:%{public}" PRIu64", level:%{public}u,", screenId, level);
    return level;
}

void DisplayManager::NotifyDisplayEvent(DisplayEvent event)
{
    // Unlock event dms->wms restore other hidden windows
    WLOGFD("[UL_POWER]DisplayEvent:%{public}u", event);
    SingletonContainer::Get<DisplayManagerAdapter>().NotifyDisplayEvent(event);
}

bool DisplayManager::Freeze(std::vector<DisplayId> displayIds)
{
    WLOGFD("freeze display");
    if (displayIds.size() == 0) {
        WLOGFE("freeze display fail, num of display is 0");
        return false;
    }
    if (displayIds.size() > MAX_DISPLAY_SIZE) {
        WLOGFE("freeze display fail, displayIds size is bigger than %{public}u.", MAX_DISPLAY_SIZE);
        return false;
    }
    return SingletonContainer::Get<DisplayManagerAdapter>().SetFreeze(displayIds, true);
}

bool DisplayManager::Unfreeze(std::vector<DisplayId> displayIds)
{
    WLOGFD("unfreeze display");
    if (displayIds.size() == 0) {
        WLOGFE("unfreeze display fail, num of display is 0");
        return false;
    }
    if (displayIds.size() > MAX_DISPLAY_SIZE) {
        WLOGFE("unfreeze display fail, displayIds size is bigger than %{public}u.", MAX_DISPLAY_SIZE);
        return false;
    }
    return SingletonContainer::Get<DisplayManagerAdapter>().SetFreeze(displayIds, false);
}

DMError DisplayManager::AddSurfaceNodeToDisplay(DisplayId displayId,
    std::shared_ptr<class RSSurfaceNode>& surfaceNode)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().AddSurfaceNodeToDisplay(displayId, surfaceNode);
}

DMError DisplayManager::RemoveSurfaceNodeFromDisplay(DisplayId displayId,
    std::shared_ptr<class RSSurfaceNode>& surfaceNode)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().RemoveSurfaceNodeFromDisplay(displayId, surfaceNode);
}

void DisplayManager::Impl::OnRemoteDied()
{
    WLOGFI("dms is died");
    if (g_dmIsDestroyed) {
        WLOGFE("dm has been destructed, mutex_ is invalid");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    displayManagerListener_ = nullptr;
    displayStateAgent_ = nullptr;
    powerEventListenerAgent_ = nullptr;
    screenshotListenerAgent_ = nullptr;
    privateWindowListenerAgent_ = nullptr;
    privateWindowListChangeListenerAgent_ = nullptr;
    foldStatusListenerAgent_ = nullptr;
    foldAngleListenerAgent_ = nullptr;
    captureStatusListenerAgent_ = nullptr;
}

void DisplayManager::OnRemoteDied()
{
    if (g_dmIsDestroyed) {
        WLOGFE("dms is dying, pImpl_ is nullptr");
        return;
    }
    pImpl_->OnRemoteDied();
}

bool DisplayManager::ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId)
{
    return pImpl_->ConvertScreenIdToRsScreenId(screenId, rsScreenId);
}

bool DisplayManager::Impl::ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId)
{
    bool res = SingletonContainer::Get<DisplayManagerAdapter>().ConvertScreenIdToRsScreenId(screenId, rsScreenId);
    WLOGFD("Convert ScreenId %{public}" PRIu64" To RsScreenId %{public}" PRIu64"", screenId, rsScreenId);
    return res;
}

DMError DisplayManager::ProxyForFreeze(std::set<int32_t> pidList, bool isProxy)
{
    return pImpl_->ProxyForFreeze(pidList, isProxy);
}

DMError DisplayManager::Impl::ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().ProxyForFreeze(pidList, isProxy);
}

DMError DisplayManager::ResetAllFreezeStatus()
{
    return pImpl_->ResetAllFreezeStatus();
}

DMError DisplayManager::Impl::ResetAllFreezeStatus()
{
    return SingletonContainer::Get<DisplayManagerAdapter>().ResetAllFreezeStatus();
}

void DisplayManager::SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList)
{
    SingletonContainer::Get<DisplayManagerAdapter>().SetVirtualScreenBlackList(screenId, windowIdList);
}

void DisplayManager::DisablePowerOffRenderControl(ScreenId screenId)
{
    SingletonContainer::Get<DisplayManagerAdapter>().DisablePowerOffRenderControl(screenId);
}

DMError DisplayManager::SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
    std::vector<uint64_t>& windowIdList)
{
    return pImpl_->SetVirtualScreenSecurityExemption(screenId, pid, windowIdList);
}

DMError DisplayManager::Impl::SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
    std::vector<uint64_t>& windowIdList)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().SetVirtualScreenSecurityExemption(
        screenId, pid, windowIdList);
}
} // namespace OHOS::Rosen
