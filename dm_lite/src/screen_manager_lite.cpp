/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "screen_manager_lite.h"
#include <cinttypes>
#include "display_manager_adapter_lite.h"
#include "display_manager_agent_default.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

/*
 * used by powermgr
 */
class ScreenManagerLite::Impl : public RefBase {
public:
    Impl() = default;
    ~Impl();
    
    static inline SingletonDelegator<ScreenManagerLite> delegator;
    DMError RegisterScreenListener(sptr<IScreenListener> listener);
    DMError UnregisterScreenListener(sptr<IScreenListener> listener);
    DMError RegisterDisplayManagerAgent();
    DMError UnregisterDisplayManagerAgent();
    DMError RegisterScreenModeChangeListener(sptr<IScreenModeChangeListener> listener);
    DMError UnregisterScreenModeChangeListener(sptr<IScreenModeChangeListener> listener);
    DMError RegisterScreenModeChangeManagerAgent();
    DMError UnregisterScreenModeChangeManagerAgent();
    DMError RegisterAbnormalScreenConnectChangeListener(sptr<IAbnormalScreenConnectChangeListener> listener);
    DMError UnregisterAbnormalScreenConnectChangeListener(sptr<IAbnormalScreenConnectChangeListener> listener);
    DMError GetPhysicalScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos);
    void OnRemoteDied();

private:
    void NotifyScreenConnect(sptr<ScreenInfo> info);
    void NotifyScreenDisconnect(ScreenId);
    void NotifyScreenChange(const sptr<ScreenInfo>& screenInfo);

    class ScreenManagerListener;
    sptr<ScreenManagerListener> screenManagerListener_;
    std::mutex mutex_;
    std::set<sptr<IScreenListener>> screenListeners_;

    class ScreenManagerScreenModeChangeAgent;
    std::set<sptr<IScreenModeChangeListener>> screenModeChangeListeners_;
    sptr<ScreenManagerScreenModeChangeAgent> screenModeChangeListenerAgent_;

    class ScreenManagerAbnormalScreenConnectChangeAgent;
    std::set<sptr<IAbnormalScreenConnectChangeListener>> abnormalScreenConnectChangeListeners_;
    sptr<ScreenManagerAbnormalScreenConnectChangeAgent> abnormalScreenConnectChangeListenerAgent_;
};

class ScreenManagerLite::Impl::ScreenManagerListener : public DisplayManagerAgentDefault {
public:
    explicit ScreenManagerListener(sptr<Impl> impl) : pImpl_(impl)
    {
    }

    void OnScreenConnect(sptr<ScreenInfo> screenInfo)
    {
        if (screenInfo == nullptr || screenInfo->GetScreenId() == SCREEN_ID_INVALID) {
            TLOGE(WmsLogTag::DMS_DM_LITE, "screenInfo is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS_DM_LITE, "impl is nullptr.");
            return;
        }
        std::lock_guard<std::mutex> lock(pImpl_->mutex_);
        for (auto listener : pImpl_->screenListeners_) {
            listener->OnConnect(screenInfo->GetScreenId());
        }
    };

    void OnScreenDisconnect(ScreenId screenId)
    {
        if (screenId == SCREEN_ID_INVALID) {
            TLOGE(WmsLogTag::DMS_DM_LITE, "screenId is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS_DM_LITE, "impl is nullptr.");
            return;
        }
        std::lock_guard<std::mutex> lock(pImpl_->mutex_);
        for (auto listener : pImpl_->screenListeners_) {
            listener->OnDisconnect(screenId);
        }
    };

    void OnScreenChange(const sptr<ScreenInfo>& screenInfo, ScreenChangeEvent event)
    {
        if (screenInfo == nullptr) {
            TLOGE(WmsLogTag::DMS_DM_LITE, "screenInfo is null.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS_DM_LITE, "impl is nullptr.");
            return;
        }
        TLOGD(WmsLogTag::DMS_DM_LITE, "event %{public}u", event);
        std::lock_guard<std::mutex> lock(pImpl_->mutex_);
        for (auto listener: pImpl_->screenListeners_) {
            listener->OnChange(screenInfo->GetScreenId());
        }
    };
private:
    sptr<Impl> pImpl_;
};

class ScreenManagerLite::Impl::ScreenManagerScreenModeChangeAgent : public DisplayManagerAgentDefault {
public:
    explicit ScreenManagerScreenModeChangeAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~ScreenManagerScreenModeChangeAgent() = default;

    virtual void NotifyScreenModeChange(const std::vector<sptr<ScreenInfo>>& screenInfos) override
    {
        std::lock_guard<std::mutex> lock(pImpl_->mutex_);
        for (auto listener: pImpl_->screenModeChangeListeners_) {
            listener->NotifyScreenModeChange(screenInfos);
        }
    }
private:
    sptr<Impl> pImpl_;
};

class ScreenManagerLite::Impl::ScreenManagerAbnormalScreenConnectChangeAgent : public DisplayManagerAgentDefault {
public:
    explicit ScreenManagerAbnormalScreenConnectChangeAgent(sptr<Impl> impl) : pImpl_(impl)
    {
    }
    ~ScreenManagerAbnormalScreenConnectChangeAgent() = default;

    virtual void NotifyAbnormalScreenConnectChange(ScreenId screenId) override
    {
        std::lock_guard<std::mutex> lock(pImpl_->mutex_);
        for (auto listener: pImpl_->abnormalScreenConnectChangeListeners_) {
            listener->NotifyAbnormalScreenConnectChange(screenId);
        }
    }
private:
    sptr<Impl> pImpl_;
};

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenManagerLite)

ScreenManagerLite::ScreenManagerLite()
{
    pImpl_ = new Impl();
    TLOGD(WmsLogTag::DMS_DM_LITE, "Create ScreenManagerLite instance");
}

ScreenManagerLite::~ScreenManagerLite()
{
    TLOGD(WmsLogTag::DMS_DM_LITE, "Destroy ScreenManagerLite instance");
}

ScreenManagerLite::Impl::~Impl()
{
    std::lock_guard<std::mutex> lock(mutex_);
    UnregisterDisplayManagerAgent();
}

DMError ScreenManagerLite::Impl::RegisterDisplayManagerAgent()
{
    DMError regSucc = DMError::DM_OK;
    if (screenManagerListener_ == nullptr) {
        screenManagerListener_ = new ScreenManagerListener(this);
        regSucc = SingletonContainer::Get<ScreenManagerAdapterLite>().RegisterDisplayManagerAgent(
            screenManagerListener_, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
        if (regSucc != DMError::DM_OK) {
            screenManagerListener_ = nullptr;
            TLOGW(WmsLogTag::DMS_DM_LITE, "RegisterDisplayManagerAgent failed !");
        }
    }
    return regSucc;
}

DMError ScreenManagerLite::Impl::UnregisterDisplayManagerAgent()
{
    DMError unRegSucc = DMError::DM_OK;
    if (screenManagerListener_ != nullptr) {
        unRegSucc = SingletonContainer::Get<ScreenManagerAdapterLite>().UnregisterDisplayManagerAgent(
            screenManagerListener_, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
        screenManagerListener_ = nullptr;
        if (unRegSucc != DMError::DM_OK) {
            TLOGW(WmsLogTag::DMS_DM_LITE, "UnregisterDisplayManagerAgent failed!");
        }
    }
    return unRegSucc;
}

DMError ScreenManagerLite::Impl::RegisterScreenListener(sptr<IScreenListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    DMError regSucc = RegisterDisplayManagerAgent();
    if (regSucc == DMError::DM_OK) {
        screenListeners_.insert(listener);
    }
    return regSucc;
}

DMError ScreenManagerLite::RegisterScreenListener(sptr<IScreenListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "RegisterScreenListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenListener(listener);
}

DMError ScreenManagerLite::Impl::UnregisterScreenListener(sptr<IScreenListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = std::find(screenListeners_.begin(), screenListeners_.end(), listener);
    if (iter == screenListeners_.end()) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenListeners_.erase(iter);
    return UnregisterDisplayManagerAgent();
}

DMError ScreenManagerLite::UnregisterScreenListener(sptr<IScreenListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "UnregisterScreenListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenListener(listener);
}

DMError ScreenManagerLite::Impl::RegisterScreenModeChangeManagerAgent()
{
    DMError regSucc = DMError::DM_OK;
    if (screenModeChangeListenerAgent_ == nullptr) {
        screenModeChangeListenerAgent_ = new ScreenManagerScreenModeChangeAgent(this);
        regSucc = SingletonContainer::Get<ScreenManagerAdapterLite>().RegisterDisplayManagerAgent(
            screenModeChangeListenerAgent_, DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER);
        if (regSucc != DMError::DM_OK) {
            screenModeChangeListenerAgent_ = nullptr;
            TLOGW(WmsLogTag::DMS_DM_LITE, "RegisterDisplayManagerAgent failed !");
        }
    }
    return regSucc;
}

DMError ScreenManagerLite::Impl::UnregisterScreenModeChangeManagerAgent()
{
    DMError unRegSucc = DMError::DM_OK;
    if (screenModeChangeListenerAgent_ != nullptr) {
        unRegSucc = SingletonContainer::Get<ScreenManagerAdapterLite>().UnregisterDisplayManagerAgent(
            screenModeChangeListenerAgent_, DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER);
        screenModeChangeListenerAgent_ = nullptr;
        if (unRegSucc != DMError::DM_OK) {
            TLOGW(WmsLogTag::DMS_DM_LITE, "UnregisterDisplayManagerAgent failed!");
        }
    }
    return unRegSucc;
}

DMError ScreenManagerLite::Impl::RegisterScreenModeChangeListener(sptr<IScreenModeChangeListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    DMError regSucc = RegisterScreenModeChangeManagerAgent();
    if (regSucc == DMError::DM_OK) {
        screenModeChangeListeners_.insert(listener);
    }
    std::vector<sptr<ScreenInfo>> screenInfos;
    DMError ret = GetPhysicalScreenInfos(screenInfos);
    if (ret == DMError::DM_OK) {
        TLOGI(WmsLogTag::DMS_DM_LITE, "RegisterScreenListener notify");
        listener->NotifyScreenModeChange(screenInfos);
    }
    return regSucc;
}

DMError ScreenManagerLite::RegisterScreenModeChangeListener(sptr<IScreenModeChangeListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "RegisterScreenListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenModeChangeListener(listener);
}

DMError ScreenManagerLite::Impl::UnregisterScreenModeChangeListener(sptr<IScreenModeChangeListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = std::find(screenModeChangeListeners_.begin(), screenModeChangeListeners_.end(), listener);
    if (iter == screenModeChangeListeners_.end()) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenModeChangeListeners_.erase(iter);
    return UnregisterScreenModeChangeManagerAgent();
}

DMError ScreenManagerLite::UnregisterScreenModeChangeListener(sptr<IScreenModeChangeListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "UnregisterScreenListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenModeChangeListener(listener);
}

DMError ScreenManagerLite::RegisterAbnormalScreenConnectChangeListener(
    sptr<IAbnormalScreenConnectChangeListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterAbnormalScreenConnectChangeListener(listener);
}

DMError ScreenManagerLite::Impl::RegisterAbnormalScreenConnectChangeListener(
    sptr<IAbnormalScreenConnectChangeListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    DMError regSucc = DMError::DM_OK;
    if (abnormalScreenConnectChangeListenerAgent_ == nullptr) {
        abnormalScreenConnectChangeListenerAgent_ =
            new (std::nothrow) ScreenManagerAbnormalScreenConnectChangeAgent(this);
        regSucc = SingletonContainer::Get<ScreenManagerAdapterLite>().RegisterDisplayManagerAgent(
            abnormalScreenConnectChangeListenerAgent_,
            DisplayManagerAgentType::ABNORMAL_SCREEN_CONNECT_CHANGE_LISTENER);
        if (regSucc != DMError::DM_OK) {
            abnormalScreenConnectChangeListenerAgent_ = nullptr;
            TLOGW(WmsLogTag::DMS_DM_LITE, "RegisterDisplayManagerAgent failed !");
        }
    }
    if (regSucc == DMError::DM_OK) {
        abnormalScreenConnectChangeListeners_.insert(listener);
    }
    return regSucc;
}

DMError ScreenManagerLite::UnregisterAbnormalScreenConnectChangeListener(
    sptr<IAbnormalScreenConnectChangeListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterAbnormalScreenConnectChangeListener(listener);
}

DMError ScreenManagerLite::Impl::UnregisterAbnormalScreenConnectChangeListener(
    sptr<IAbnormalScreenConnectChangeListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = std::find(abnormalScreenConnectChangeListeners_.begin(),
        abnormalScreenConnectChangeListeners_.end(), listener);
    if (iter == abnormalScreenConnectChangeListeners_.end()) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    abnormalScreenConnectChangeListeners_.erase(iter);
    DMError unregSucc = DMError::DM_OK;
    if (abnormalScreenConnectChangeListenerAgent_ != nullptr) {
        unregSucc = SingletonContainer::Get<ScreenManagerAdapterLite>().UnregisterDisplayManagerAgent(
            abnormalScreenConnectChangeListenerAgent_,
            DisplayManagerAgentType::ABNORMAL_SCREEN_CONNECT_CHANGE_LISTENER);
        abnormalScreenConnectChangeListenerAgent_ = nullptr;
        if (unregSucc != DMError::DM_OK) {
            TLOGW(WmsLogTag::DMS_DM_LITE, "UnregisterDisplayManagerAgent failed !");
        }
    }
    return unregSucc;
}

bool ScreenManagerLite::SetSpecifiedScreenPower(ScreenId screenId,
    ScreenPowerState state, PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS_DM_LITE, "screenId:%{public}" PRIu64 ", state:%{public}u, reason:%{public}u",
        screenId, state, reason);
    return SingletonContainer::Get<ScreenManagerAdapterLite>().SetSpecifiedScreenPower(screenId, state, reason);
}

bool ScreenManagerLite::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS_DM_LITE, "state:%{public}u, reason:%{public}u", state, reason);
    return SingletonContainer::Get<ScreenManagerAdapterLite>().SetScreenPowerForAll(state, reason);
}

ScreenPowerState ScreenManagerLite::GetScreenPower(ScreenId dmsScreenId)
{
    return SingletonContainer::Get<ScreenManagerAdapterLite>().GetScreenPower(dmsScreenId);
}

ScreenPowerState ScreenManagerLite::GetScreenPower()
{
    return SingletonContainer::Get<ScreenManagerAdapterLite>().GetScreenPower();
}

DMError ScreenManagerLite::GetPhysicalScreenIds(std::vector<uint64_t>& screenIds)
{
    std::vector<sptr<ScreenInfo>> screenInfos;
    DMError ret = SingletonContainer::Get<ScreenManagerAdapterLite>().GetAllScreenInfos(screenInfos);
    if (ret != DMError::DM_OK) {
        return ret;
    }

    for (const auto& screenInfo : screenInfos) {
        auto id = screenInfo->GetScreenId();
        if (screenInfo->GetType() == ScreenType::REAL && id != SCREEN_ID_INVALID) {
            screenIds.push_back(id);
        }
    }
    return DMError::DM_OK;
}

DMError ScreenManagerLite::Impl::GetPhysicalScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
    std::vector<sptr<ScreenInfo>> allScreenInfos;
    DMError ret = SingletonContainer::Get<ScreenManagerAdapterLite>().GetAllScreenInfos(allScreenInfos);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS_DM_LITE, "GetPhysicalScreenInfos failed");
        return ret;
    }
    for (const auto& screenInfo : allScreenInfos) {
        if (screenInfo->GetType() == ScreenType::REAL) {
            screenInfos.push_back(screenInfo);
        }
    }
    return DMError::DM_OK;
}

void ScreenManagerLite::Impl::OnRemoteDied()
{
    TLOGD(WmsLogTag::DMS_DM_LITE, "dms is died");
    std::lock_guard<std::mutex> lock(mutex_);
    screenManagerListener_ = nullptr;
    screenModeChangeListenerAgent_ = nullptr;
    abnormalScreenConnectChangeListenerAgent_ = nullptr;
}

void ScreenManagerLite::OnRemoteDied()
{
    pImpl_->OnRemoteDied();
}

} // namespace OHOS::Rosen