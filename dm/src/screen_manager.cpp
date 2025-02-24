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
#include "screen_manager.h"

#include <map>
#include <vector>

#include <transaction/rs_interfaces.h>

#include "display_manager_adapter.h"
#include "display_manager_agent_default.h"
#include "permission.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "ScreenManager"};
const static uint32_t MAX_SCREEN_SIZE = 32;
const static uint32_t DLCLOSE_TIMEOUT = 300000;
}
class ScreenManager::Impl : public RefBase {
public:
    Impl() = default;
    ~Impl();
    
    static inline SingletonDelegator<ScreenManager> delegator;
    ScreenId CreateVirtualScreen(VirtualScreenOption option);
    sptr<Screen> GetScreen(ScreenId screenId);
    sptr<ScreenGroup> GetScreenGroup(ScreenId screenId);
    DMError GetAllScreens(std::vector<sptr<Screen>>& screens);
    DMError RegisterScreenListener(sptr<IScreenListener> listener);
    DMError UnregisterScreenListener(sptr<IScreenListener> listener);
    DMError RegisterScreenGroupListener(sptr<IScreenGroupListener> listener);
    DMError UnregisterScreenGroupListener(sptr<IScreenGroupListener> listener);
    DMError RegisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener);
    DMError UnregisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener);
    DMError RegisterDisplayManagerAgent();
    DMError UnregisterDisplayManagerAgent();
    void OnRemoteDied();

private:
    void NotifyScreenConnect(sptr<ScreenInfo> info);
    void NotifyScreenDisconnect(ScreenId);
    void NotifyScreenChange(const sptr<ScreenInfo>& screenInfo);
    void NotifyScreenChange(const std::vector<sptr<ScreenInfo>>& screenInfos);
    bool UpdateScreenInfoLocked(sptr<ScreenInfo>);
    std::string GetScreenInfoSrting(sptr<ScreenInfo> screenInfo);
    void DlcloseTimeout();

    bool isAllListenersRemoved() const;

    class ScreenManagerListener;
    sptr<ScreenManagerListener> screenManagerListener_;
    std::map<ScreenId, sptr<Screen>> screenMap_;
    std::map<ScreenId, sptr<ScreenGroup>> screenGroupMap_;
    std::recursive_mutex mutex_;
    std::set<sptr<IScreenListener>> screenListeners_;
    std::set<sptr<IScreenGroupListener>> screenGroupListeners_;
    std::set<sptr<IVirtualScreenGroupListener>> virtualScreenGroupListeners_;
    sptr<IDisplayManagerAgent> virtualScreenAgent_ = nullptr;
};

class ScreenManager::Impl::ScreenManagerListener : public DisplayManagerAgentDefault {
public:
    explicit ScreenManagerListener(sptr<Impl> impl) : pImpl_(impl)
    {
    }

    void OnScreenConnect(sptr<ScreenInfo> screenInfo)
    {
        if (screenInfo == nullptr || screenInfo->GetScreenId() == SCREEN_ID_INVALID) {
            WLOGFE("OnScreenConnect, screenInfo is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("OnScreenConnect, impl is nullptr.");
            return;
        }
        pImpl_->NotifyScreenConnect(screenInfo);
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        for (auto listener : pImpl_->screenListeners_) {
            listener->OnConnect(screenInfo->GetScreenId());
        }
    };

    void OnScreenDisconnect(ScreenId screenId)
    {
        if (screenId == SCREEN_ID_INVALID) {
            WLOGFE("OnScreenDisconnect, screenId is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("OnScreenDisconnect, impl is nullptr.");
            return;
        }
        pImpl_->NotifyScreenDisconnect(screenId);
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        for (auto listener : pImpl_->screenListeners_) {
            listener->OnDisconnect(screenId);
        }
    };

    void OnScreenChange(const sptr<ScreenInfo>& screenInfo, ScreenChangeEvent event)
    {
        if (screenInfo == nullptr) {
            WLOGFE("OnScreenChange, screenInfo is null.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("OnScreenChange, impl is nullptr.");
            return;
        }
        WLOGFD("OnScreenChange. event %{public}u", event);
        pImpl_->NotifyScreenChange(screenInfo);
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        for (auto listener: pImpl_->screenListeners_) {
            listener->OnChange(screenInfo->GetScreenId());
        }
    };

    void OnScreenGroupChange(const std::string& trigger, const std::vector<sptr<ScreenInfo>>& screenInfos,
        ScreenGroupChangeEvent groupEvent)
    {
        if (screenInfos.empty()) {
            WLOGFE("screenInfos is empty.");
            return;
        }
        if (pImpl_ == nullptr) {
            WLOGFE("impl is nullptr.");
            return;
        }
        WLOGFD("trigger %{public}s, event %{public}u", trigger.c_str(), groupEvent);
        pImpl_->NotifyScreenChange(screenInfos);
        std::vector<ScreenId> screenIds;
        for (auto screenInfo : screenInfos) {
            if (screenInfo->GetScreenId() != SCREEN_ID_INVALID) {
                screenIds.push_back(screenInfo->GetScreenId());
            }
        }
        std::lock_guard<std::recursive_mutex> lock(pImpl_->mutex_);
        for (auto listener: pImpl_->screenGroupListeners_) {
            listener->OnChange(screenIds, groupEvent);
        }
        NotifyVirtualScreenGroupChanged(screenInfos[0], trigger, screenIds, groupEvent);
    };
private:
    void NotifyVirtualScreenGroupChanged(sptr<ScreenInfo> screenInfo,
        const std::string trigger, std::vector<ScreenId>& ids, ScreenGroupChangeEvent groupEvent)
    {
        // check for invalid scene
        if (pImpl_->virtualScreenGroupListeners_.size() <= 0) {
            WLOGFW("no virtual screen group listeners");
            return;
        }
        if (screenInfo->GetType() != ScreenType::VIRTUAL) {
            WLOGFW("not virtual screen type: %{public}u", screenInfo->GetType());
            return;
        }

        // get the parent of screen
        ScreenId parent = groupEvent == ScreenGroupChangeEvent::ADD_TO_GROUP ?
            screenInfo->GetParentId() : screenInfo->GetLastParentId();
        WLOGFD("parentId=[%{public}llu], lastParentId=[%{public}llu]", (unsigned long long)screenInfo->GetParentId(),
            (unsigned long long)screenInfo->GetLastParentId());
        if (parent == INVALID_SCREEN_ID) {
            WLOGFE("parentId is invalid");
            return;
        }
        auto screenGroup = pImpl_->GetScreenGroup(parent);
        if (screenGroup == nullptr) {
            WLOGFE("screenGroup is null");
            return;
        }

        // notify mirror
        ScreenCombination comb = screenGroup->GetCombination();
        WLOGFD("comb %{public}u", comb);
        IVirtualScreenGroupListener::ChangeInfo changeInfo = {groupEvent, trigger, ids};
        for (auto listener: pImpl_->virtualScreenGroupListeners_) {
            if (comb == ScreenCombination::SCREEN_MIRROR) {
                listener->OnMirrorChange(changeInfo);
            }
        }
    }
    sptr<Impl> pImpl_;
};

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenManager)

extern "C" __attribute__((destructor)) void ScreenManager::Impl::DlcloseTimeout()
{
    usleep(DLCLOSE_TIMEOUT);
}

ScreenManager::ScreenManager()
{
    pImpl_ = new Impl();
    WLOGFD("Create screenmanager instance");
}

ScreenManager::~ScreenManager()
{
    WLOGFI("Destroy screenmanager instance");
}

ScreenManager::Impl::~Impl()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UnregisterDisplayManagerAgent();
}

sptr<Screen> ScreenManager::Impl::GetScreen(ScreenId screenId)
{
    auto screenInfo = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenInfo(screenId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateScreenInfoLocked(screenInfo)) {
        screenMap_.erase(screenId);
        return nullptr;
    }
    return screenMap_[screenId];
}

sptr<Screen> ScreenManager::GetScreenById(ScreenId screenId)
{
    return pImpl_->GetScreen(screenId);
}

sptr<ScreenGroup> ScreenManager::Impl::GetScreenGroup(ScreenId screenId)
{
    auto screenGroupInfo = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenGroupInfoById(screenId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (screenGroupInfo == nullptr) {
        WLOGFE("screenGroupInfo is null");
        screenGroupMap_.erase(screenId);
        return nullptr;
    }
    auto iter = screenGroupMap_.find(screenId);
    if (iter != screenGroupMap_.end() && iter->second != nullptr) {
        iter->second->UpdateScreenGroupInfo(screenGroupInfo);
        return iter->second;
    }
    sptr<ScreenGroup> screenGroup = new ScreenGroup(screenGroupInfo);
    screenGroupMap_[screenId] = screenGroup;
    return screenGroup;
}

sptr<ScreenGroup> ScreenManager::GetScreenGroup(ScreenId screenId)
{
    return pImpl_->GetScreenGroup(screenId);
}

DMError ScreenManager::Impl::GetAllScreens(std::vector<sptr<Screen>>& screens)
{
    std::vector<sptr<ScreenInfo>> screenInfos;
    DMError ret  = SingletonContainer::Get<ScreenManagerAdapter>().GetAllScreenInfos(screenInfos);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto info: screenInfos) {
        if (UpdateScreenInfoLocked(info)) {
            screens.emplace_back(screenMap_[info->GetScreenId()]);
        }
    }
    screenMap_.clear();
    for (auto screen: screens) {
        screenMap_.insert(std::make_pair(screen->GetId(), screen));
    }
    return ret;
}

DMError ScreenManager::GetAllScreens(std::vector<sptr<Screen>>& screens)
{
    return pImpl_->GetAllScreens(screens);
}

DMError ScreenManager::Impl::RegisterScreenListener(sptr<IScreenListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError regSucc = RegisterDisplayManagerAgent();
    if (regSucc == DMError::DM_OK) {
        screenListeners_.insert(listener);
    }
    return regSucc;
}

DMError ScreenManager::RegisterScreenListener(sptr<IScreenListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterScreenListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenListener(listener);
}

DMError ScreenManager::Impl::UnregisterScreenListener(sptr<IScreenListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(screenListeners_.begin(), screenListeners_.end(), listener);
    if (iter == screenListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
}

DMError ScreenManager::UnregisterScreenListener(sptr<IScreenListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterScreenListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenListener(listener);
}

DMError ScreenManager::Impl::RegisterScreenGroupListener(sptr<IScreenGroupListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError regSucc = RegisterDisplayManagerAgent();
    if (regSucc == DMError::DM_OK) {
        screenGroupListeners_.insert(listener);
    }
    return regSucc;
}

DMError ScreenManager::RegisterScreenGroupListener(sptr<IScreenGroupListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterScreenGroupListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenGroupListener(listener);
}

DMError ScreenManager::Impl::UnregisterScreenGroupListener(sptr<IScreenGroupListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(screenGroupListeners_.begin(), screenGroupListeners_.end(), listener);
    if (iter == screenGroupListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
}

DMError ScreenManager::UnregisterScreenGroupListener(sptr<IScreenGroupListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterScreenGroupListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenGroupListener(listener);
}

DMError ScreenManager::Impl::RegisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError regSucc = RegisterDisplayManagerAgent();
    if (regSucc == DMError::DM_OK) {
        virtualScreenGroupListeners_.insert(listener);
    }
    return regSucc;
}

DMError ScreenManager::RegisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("RegisterVirtualScreenGroupListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterVirtualScreenGroupListener(listener);
}

DMError ScreenManager::Impl::UnregisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(virtualScreenGroupListeners_.begin(), virtualScreenGroupListeners_.end(), listener);
    if (iter == virtualScreenGroupListeners_.end()) {
        WLOGFE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    virtualScreenGroupListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
}

DMError ScreenManager::UnregisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener)
{
    if (listener == nullptr) {
        WLOGFE("UnregisterVirtualScreenGroupListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterVirtualScreenGroupListener(listener);
}

DMError ScreenManager::Impl::RegisterDisplayManagerAgent()
{
    DMError regSucc = DMError::DM_OK;
    if (screenManagerListener_ == nullptr) {
        screenManagerListener_ = new ScreenManagerListener(this);
        regSucc = SingletonContainer::Get<ScreenManagerAdapter>().RegisterDisplayManagerAgent(
            screenManagerListener_, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
        if (regSucc != DMError::DM_OK) {
            screenManagerListener_ = nullptr;
            WLOGFW("RegisterDisplayManagerAgent failed !");
        }
    }
    return regSucc;
}

DMError ScreenManager::Impl::UnregisterDisplayManagerAgent()
{
    DMError unRegSucc = DMError::DM_OK;
    if (screenManagerListener_ != nullptr) {
        unRegSucc = SingletonContainer::Get<ScreenManagerAdapter>().UnregisterDisplayManagerAgent(
            screenManagerListener_, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
        screenManagerListener_ = nullptr;
        if (unRegSucc != DMError::DM_OK) {
            WLOGFW("UnregisterDisplayManagerAgent failed!");
        }
    }
    return unRegSucc;
}

DMError ScreenManager::MakeExpand(const std::vector<ExpandOption>& options, ScreenId& screenGroupId)
{
    WLOGFD("Make expand");
    if (options.empty()) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (options.size() > MAX_SCREEN_SIZE) {
        WLOGFW("Make expand failed. The options size is bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    std::vector<ScreenId> screenIds;
    std::vector<Point> startPoints;
    for (auto& option: options) {
        if (std::find(screenIds.begin(), screenIds.end(), option.screenId_) != screenIds.end()) {
            continue;
        }
        screenIds.emplace_back(option.screenId_);
        startPoints.emplace_back(Point(option.startX_, option.startY_));
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeExpand(screenIds, startPoints, screenGroupId);
    if (screenGroupId == SCREEN_ID_INVALID) {
        WLOGFE("Make expand failed");
    }
    return ret;
}

DMError ScreenManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds)
{
    WLOGFD("start Make UniqueScreen");
    if (screenIds.empty()) {
        WLOGFE("screenIds is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screenIds.size() > MAX_SCREEN_SIZE) {
        WLOGFW("Make UniqueScreen failed. The screenIds size is bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeUniqueScreen(screenIds);
    return ret;
}

DMError ScreenManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, ScreenId& screenGroupId)
{
    WLOGFI("Make mirror for screen: %{public}" PRIu64"", mainScreenId);
    if (mirrorScreenId.size() > MAX_SCREEN_SIZE) {
        WLOGFW("Make Mirror failed. The mirrorScreenId size is bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirror(mainScreenId, mirrorScreenId,
        screenGroupId);
    if (screenGroupId == SCREEN_ID_INVALID) {
        WLOGFE("create mirror failed");
    }
    return ret;
}

DMError ScreenManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, DMRect mainScreenRegion,
    ScreenId& screenGroupId)
{
    WLOGFI("Make mirror with region for screen: %{public}" PRIu64"", mainScreenId);
    if (mirrorScreenId.size() > MAX_SCREEN_SIZE) {
        WLOGFW("Make Mirror failed. MirrorScreenId size bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirror(mainScreenId, mirrorScreenId,
        mainScreenRegion, screenGroupId);
    if (screenGroupId == SCREEN_ID_INVALID) {
        WLOGFE("create mirror failed");
    }
    return ret;
}

DMError ScreenManager::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    WLOGFD("Stop expand");
    if (expandScreenIds.empty()) {
        return DMError::DM_OK;
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().StopExpand(expandScreenIds);
}

DMError ScreenManager::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    WLOGFD("Stop mirror");
    if (mirrorScreenIds.empty()) {
        return DMError::DM_OK;
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().StopMirror(mirrorScreenIds);
}

DMError ScreenManager::DisableMirror(bool disableOrNot)
{
    WLOGFI("Disable mirror %{public}d", disableOrNot);
    return SingletonContainer::Get<ScreenManagerAdapter>().DisableMirror(disableOrNot);
}

DMError ScreenManager::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    WLOGFI("screens.size=%{public}llu", (unsigned long long)screens.size());
    if (screens.empty()) {
        WLOGFW("RemoveVirtualScreenFromGroup failed. screens is empty.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screens.size() > MAX_SCREEN_SIZE) {
        WLOGFW("RemoveVirtualScreenFromGroup failed. The screens size is bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenFromGroup(screens);
    return DMError::DM_OK;
}

ScreenId ScreenManager::CreateVirtualScreen(VirtualScreenOption option)
{
    return pImpl_->CreateVirtualScreen(option);
}

ScreenId ScreenManager::Impl::CreateVirtualScreen(VirtualScreenOption option)
{
    //  After the process creating the virtual screen is killed, DMS needs to delete the virtual screen
    if (virtualScreenAgent_ == nullptr) {
        virtualScreenAgent_ = new DisplayManagerAgentDefault();
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().CreateVirtualScreen(option, virtualScreenAgent_);
}

DMError ScreenManager::DestroyVirtualScreen(ScreenId screenId)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().DestroyVirtualScreen(screenId);
}

DMError ScreenManager::SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenSurface(screenId, surface);
}

DMError ScreenManager::SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool rotation)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualMirrorScreenCanvasRotation(screenId, rotation);
}

DMError ScreenManager::ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().ResizeVirtualScreen(screenId, width, height);
}

DMError ScreenManager::SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualMirrorScreenScaleMode(screenId, scaleMode);
}

VirtualScreenFlag ScreenManager::GetVirtualScreenFlag(ScreenId screenId)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetVirtualScreenFlag(screenId);
}

DMError ScreenManager::SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenFlag(screenId, screenFlag);
}

DMError ScreenManager::SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenRefreshRate(screenId, refreshInterval);
}

bool ScreenManager::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason)
{
    WLOGFI("screenId:%{public}" PRIu64 ", state:%{public}u, reason:%{public}u", screenId, state, reason);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetSpecifiedScreenPower(screenId, state, reason);
}

bool ScreenManager::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    WLOGFI("state:%{public}u, reason:%{public}u", state, reason);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenPowerForAll(state, reason);
}

ScreenPowerState ScreenManager::GetScreenPower(ScreenId dmsScreenId)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenPower(dmsScreenId);
}

DMError ScreenManager::SetScreenRotationLocked(bool isLocked)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenRotationLocked(isLocked);
}

DMError ScreenManager::SetScreenRotationLockedFromJs(bool isLocked)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenRotationLockedFromJs(isLocked);
}

DMError ScreenManager::IsScreenRotationLocked(bool& isLocked)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().IsScreenRotationLocked(isLocked);
}

void ScreenManager::Impl::NotifyScreenConnect(sptr<ScreenInfo> info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UpdateScreenInfoLocked(info);
}

void ScreenManager::Impl::NotifyScreenDisconnect(ScreenId screenId)
{
    WLOGFI("screenId:%{public}" PRIu64".", screenId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    screenMap_.erase(screenId);
}

void ScreenManager::Impl::NotifyScreenChange(const sptr<ScreenInfo>& screenInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    UpdateScreenInfoLocked(screenInfo);
}

void ScreenManager::Impl::NotifyScreenChange(const std::vector<sptr<ScreenInfo>>& screenInfos)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto screenInfo : screenInfos) {
        UpdateScreenInfoLocked(screenInfo);
    }
}

bool ScreenManager::Impl::UpdateScreenInfoLocked(sptr<ScreenInfo> screenInfo)
{
    if (screenInfo == nullptr) {
        WLOGFE("displayInfo is null");
        return false;
    }
    ScreenId screenId = screenInfo->GetScreenId();
    WLOGFD("screenId:%{public}" PRIu64".", screenId);
    if (screenId == SCREEN_ID_INVALID) {
        WLOGFE("displayId is invalid.");
        return false;
    }
    auto iter = screenMap_.find(screenId);
    if (iter != screenMap_.end() && iter->second != nullptr) {
        WLOGFD("Screen Info Updated: %{public}s",
            GetScreenInfoSrting(screenInfo).c_str());
        iter->second->UpdateScreenInfo(screenInfo);
        return true;
    }
    sptr<Screen> screen = new Screen(screenInfo);
    screenMap_[screenId] = screen;
    return true;
}

std::string ScreenManager::Impl::GetScreenInfoSrting(sptr<ScreenInfo> screenInfo)
{
    if (screenInfo == nullptr) {
        WLOGFE("screenInfo nullptr.");
        return "";
    }
    std::ostringstream oss;
    oss <<  "Screen ID: " << screenInfo->GetScreenId() << ", ";
    oss <<  "Name: " << screenInfo->GetName() << ", ";
    oss <<  "VirtualWidth: " << screenInfo->GetVirtualWidth() << ", ";
    oss <<  "VirtualHeight: " << screenInfo->GetVirtualHeight() << ", ";
    oss <<  "VirtualPixelRatio: " << screenInfo->GetVirtualPixelRatio() << ", ";
    oss <<  "Rotation: " << static_cast<int32_t>(screenInfo->GetRotation());
    return oss.str();
}

bool ScreenManager::Impl::isAllListenersRemoved() const
{
    return screenListeners_.empty() && screenGroupListeners_.empty() && virtualScreenGroupListeners_.empty();
}

void ScreenManager::Impl::OnRemoteDied()
{
    WLOGFD("dms is died");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    screenManagerListener_ = nullptr;
    virtualScreenAgent_ = nullptr;
}

void ScreenManager::OnRemoteDied()
{
    pImpl_->OnRemoteDied();
}

DMError ScreenManager::SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
    uint32_t& actualRefreshRate)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenMaxRefreshRate(id,
        refreshRate, actualRefreshRate);
}
} // namespace OHOS::Rosen