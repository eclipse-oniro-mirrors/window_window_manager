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
#include "string_util.h"
#include "permission.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
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
    DMError RegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener);
    DMError UnRegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener);
    DMError RegisterDisplayManagerAgent();
    DMError UnregisterDisplayManagerAgent();
    void OnRemoteDied();
    DMError SetVirtualScreenAutoRotation(ScreenId screenId, bool enable);

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
    std::set<sptr<IRecordDisplayListener>> recordDisplayListeners_;
    sptr<IDisplayManagerAgent> virtualScreenAgent_ = nullptr;
    std::mutex virtualScreenAgentMutex_;
};

class ScreenManager::Impl::ScreenManagerListener : public DisplayManagerAgentDefault {
public:
    explicit ScreenManagerListener(sptr<Impl> impl) : pImpl_(impl)
    {
    }

    void OnScreenConnect(sptr<ScreenInfo> screenInfo)
    {
        if (screenInfo == nullptr || screenInfo->GetScreenId() == SCREEN_ID_INVALID) {
            TLOGE(WmsLogTag::DMS, "screenInfo is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "impl is nullptr.");
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
            TLOGE(WmsLogTag::DMS, "screenId is invalid.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "impl is nullptr.");
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
            TLOGE(WmsLogTag::DMS, "screenInfo is null.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "impl is nullptr.");
            return;
        }
        TLOGD(WmsLogTag::DMS, "event %{public}u", event);
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
            TLOGE(WmsLogTag::DMS, "screenInfos is empty.");
            return;
        }
        if (pImpl_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "impl is nullptr.");
            return;
        }
        TLOGD(WmsLogTag::DMS, "trigger %{public}s, event %{public}u", trigger.c_str(), groupEvent);
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

    void NotifyRecordingDisplayChanged(const std::vector<DisplayId>& displayIds)
    {
        TLOGD(WmsLogTag::DMS, "begin");
        for (auto listener: pImpl_->recordDisplayListeners_) {
            listener->OnChange(displayIds);
        }
    };

private:
    void NotifyVirtualScreenGroupChanged(sptr<ScreenInfo> screenInfo,
        const std::string trigger, std::vector<ScreenId>& ids, ScreenGroupChangeEvent groupEvent)
    {
        if (screenInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenInfo is nullptr");
            return;
        }
        // check for invalid scene
        if (pImpl_->virtualScreenGroupListeners_.size() <= 0) {
            TLOGW(WmsLogTag::DMS, "no virtual screen group listeners");
            return;
        }
        if (screenInfo->GetType() != ScreenType::VIRTUAL) {
            TLOGW(WmsLogTag::DMS, "not virtual screen type: %{public}u", screenInfo->GetType());
            return;
        }

        // get the parent of screen
        ScreenId parent = groupEvent == ScreenGroupChangeEvent::ADD_TO_GROUP ?
            screenInfo->GetParentId() : screenInfo->GetLastParentId();
        TLOGD(WmsLogTag::DMS, "parentId=[%{public}llu], lastParentId=[%{public}llu]",
            (unsigned long long)screenInfo->GetParentId(), (unsigned long long)screenInfo->GetLastParentId());
        if (parent == INVALID_SCREEN_ID) {
            TLOGE(WmsLogTag::DMS, "parentId is invalid");
            return;
        }
        auto screenGroup = pImpl_->GetScreenGroup(parent);
        if (screenGroup == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenGroup is null");
            return;
        }

        // notify mirror
        ScreenCombination comb = screenGroup->GetCombination();
        TLOGD(WmsLogTag::DMS, "comb %{public}u", comb);
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
    TLOGD(WmsLogTag::DMS, "Create instance");
}

ScreenManager::~ScreenManager()
{
    TLOGI(WmsLogTag::DMS, "Destroy instance");
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
        TLOGE(WmsLogTag::DMS, "screenGroupInfo is null");
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
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenListener(listener);
}

DMError ScreenManager::Impl::UnregisterScreenListener(sptr<IScreenListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(screenListeners_.begin(), screenListeners_.end(), listener);
    if (iter == screenListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
}

DMError ScreenManager::UnregisterScreenListener(sptr<IScreenListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
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
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterScreenGroupListener(listener);
}

DMError ScreenManager::Impl::UnregisterScreenGroupListener(sptr<IScreenGroupListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(screenGroupListeners_.begin(), screenGroupListeners_.end(), listener);
    if (iter == screenGroupListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
}

DMError ScreenManager::UnregisterScreenGroupListener(sptr<IScreenGroupListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterScreenGroupListener(listener);
}

DMError ScreenManager::RegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "RegisterRecordDisplayListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterRecordDisplayListener(listener);
}

DMError ScreenManager::Impl::RegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DMError regSucc = RegisterDisplayManagerAgent();
    if (regSucc == DMError::DM_OK) {
        recordDisplayListeners_.insert(listener);
    }
    return regSucc;
}

DMError ScreenManager::UnRegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "UnRegisterRecordDisplayListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnRegisterRecordDisplayListener(listener);
}

DMError ScreenManager::Impl::UnRegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(recordDisplayListeners_.begin(), recordDisplayListeners_.end(), listener);
    if (iter == recordDisplayListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    recordDisplayListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
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
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterVirtualScreenGroupListener(listener);
}

DMError ScreenManager::Impl::UnregisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(virtualScreenGroupListeners_.begin(), virtualScreenGroupListeners_.end(), listener);
    if (iter == virtualScreenGroupListeners_.end()) {
        TLOGE(WmsLogTag::DMS, "could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    virtualScreenGroupListeners_.erase(iter);
    return isAllListenersRemoved() ? UnregisterDisplayManagerAgent() : DMError::DM_OK;
}

DMError ScreenManager::UnregisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::DMS, "listener is nullptr.");
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
            TLOGW(WmsLogTag::DMS, "RegisterDisplayManagerAgent failed !");
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
            TLOGW(WmsLogTag::DMS, "UnregisterDisplayManagerAgent failed!");
        }
    }
    return unRegSucc;
}

DMError ScreenManager::MakeExpand(const std::vector<ExpandOption>& options, ScreenId& screenGroupId)
{
    TLOGD(WmsLogTag::DMS, "Make expand");
    if (options.empty()) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (options.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "Make expand failed. Options size bigger than %{public}u.", MAX_SCREEN_SIZE);
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
        TLOGE(WmsLogTag::DMS, "Make expand failed");
    }
    return ret;
}

DMError ScreenManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
    const UniqueScreenRotationOptions& rotationOptions)
{
    std::vector<DisplayId> displayIds;
    TLOGD(WmsLogTag::DMS, "Start passing parameters from DMS side, isRotationLocked: %{public}d, rotation: %{public}d",
        rotationOptions.isRotationLocked_, rotationOptions.rotation_);
    return MakeUniqueScreen(screenIds, displayIds, rotationOptions);
}

DMError ScreenManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds)
{
    return MakeUniqueScreen(screenIds, UniqueScreenRotationOptions());
}

DMError ScreenManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds,
    const UniqueScreenRotationOptions& rotationOptions)
{
    TLOGD(WmsLogTag::DMS, "start make unique screen");
    if (screenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "screenIds is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screenIds.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "Make UniqueScreen failed. ScreenIds size bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeUniqueScreen(screenIds, displayIds,
        rotationOptions);
    return ret;
}

DMError ScreenManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds)
{
    return MakeUniqueScreen(screenIds, displayIds, UniqueScreenRotationOptions());
}

DMError ScreenManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, ScreenId& screenGroupId)
{
    TLOGI(WmsLogTag::DMS, "Make mirror for screen: %{public}" PRIu64"", mainScreenId);
    if (mirrorScreenId.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "Make Mirror failed. MirrorScreenId size bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirror(mainScreenId, mirrorScreenId,
        screenGroupId);
    if (screenGroupId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "create mirror failed");
    }
    return ret;
}

DMError ScreenManager::MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
    std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId)
{
    std::string mainScreenIdsStr = "";
    if (StringUtil::VectorToString(mainScreenIds, mainScreenIdsStr)) {
        TLOGI(WmsLogTag::DMS, "Make mirror for screens: %{public}s", mainScreenIdsStr.c_str());
    }
    if (mirrorScreenIds.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "Make Mirror failed. MirrorScreenId size bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirrorForRecord(mainScreenIds, mirrorScreenIds,
        screenGroupId);
    if (screenGroupId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "create mirror failed");
    }
    return ret;
}

DMError ScreenManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, DMRect mainScreenRegion,
    ScreenId& screenGroupId)
{
    TLOGI(WmsLogTag::DMS, "Make mirror with region for screen: %{public}" PRIu64"", mainScreenId);
    if (mirrorScreenId.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "Make Mirror failed. MirrorScreenId size bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirror(mainScreenId, mirrorScreenId,
        mainScreenRegion, screenGroupId);
    if (screenGroupId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "create mirror failed");
    }
    return ret;
}

DMError ScreenManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
    ScreenId& screenGroupId, Rotation rotation)
{
    TLOGI(WmsLogTag::DMS, "Make mirror for screen: %{public}" PRIu64"", mainScreenId);
    if (mirrorScreenId.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "Make mirror failed. MirrorScreenId size bigger than %{public}u.", MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().MakeMirror(mainScreenId, mirrorScreenId,
        screenGroupId, {rotation, true});
    if (screenGroupId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "Create mirror failed");
    }
    return ret;
}

DMError ScreenManager::SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
    TLOGI(WmsLogTag::DMS, "mainScreenId:%{public}" PRIu64",secondaryScreenId:%{public}" PRIu64","
        "screenMode:%{public}u", mainScreenId, secondaryScreenId, screenMode);
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().SetMultiScreenMode(mainScreenId,
        secondaryScreenId, screenMode);
    return ret;
}

DMError ScreenManager::SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
    TLOGI(WmsLogTag::DMS, "mId:%{public}" PRIu64", X:%{public}u, Y:%{public}u,sId:%{public}" PRIu64", "
        "X:%{public}u, Y:%{public}u", mainScreenOptions.screenId_, mainScreenOptions.startX_, mainScreenOptions.startY_,
        secondScreenOption.screenId_, secondScreenOption.startX_, secondScreenOption.startY_);
    DMError ret = SingletonContainer::Get<ScreenManagerAdapter>().SetMultiScreenRelativePosition(mainScreenOptions,
        secondScreenOption);
    return ret;
}

DMError ScreenManager::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    TLOGD(WmsLogTag::DMS, "Stop expand");
    if (expandScreenIds.empty()) {
        return DMError::DM_OK;
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().StopExpand(expandScreenIds);
}

DMError ScreenManager::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    TLOGD(WmsLogTag::DMS, "Stop mirror");
    if (mirrorScreenIds.empty()) {
        return DMError::DM_OK;
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().StopMirror(mirrorScreenIds);
}

DMError ScreenManager::DisableMirror(bool disableOrNot)
{
    TLOGI(WmsLogTag::DMS, "Disable mirror %{public}d", disableOrNot);
    return SingletonContainer::Get<ScreenManagerAdapter>().DisableMirror(disableOrNot);
}

DMError ScreenManager::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    TLOGI(WmsLogTag::DMS, "screens.size=%{public}llu", (unsigned long long)screens.size());
    if (screens.empty()) {
        TLOGW(WmsLogTag::DMS, "RemoveVirtualScreenFromGroup failed. screens is empty.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screens.size() > MAX_SCREEN_SIZE) {
        TLOGW(WmsLogTag::DMS, "RemoveVirtualScreenFromGroup failed. Screens size bigger than %{public}u.",
            MAX_SCREEN_SIZE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenFromGroup(screens);
    return DMError::DM_OK;
}

DMError ScreenManager::AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
{
    DMError errCode = SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenWhiteList(screenId, missionIds);
    return errCode;
}

DMError ScreenManager::RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
{
    DMError errCode = SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenWhiteList(screenId,
        missionIds);
    return errCode;
}

ScreenId ScreenManager::CreateVirtualScreen(VirtualScreenOption option)
{
    return pImpl_->CreateVirtualScreen(option);
}

ScreenId ScreenManager::Impl::CreateVirtualScreen(VirtualScreenOption option)
{
    //  After the process creating the virtual screen is killed, DMS needs to delete the virtual screen
    std::lock_guard<std::mutex> agentLock(virtualScreenAgentMutex_);
    if (virtualScreenAgent_ == nullptr) {
        virtualScreenAgent_ = new DisplayManagerAgentDefault();
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().CreateVirtualScreen(option, virtualScreenAgent_);
}

DMError ScreenManager::DestroyVirtualScreen(ScreenId screenId, bool isCallingByThirdParty)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().DestroyVirtualScreen(screenId, isCallingByThirdParty);
}

DMError ScreenManager::SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenSurface(screenId, surface);
}

DMError ScreenManager::AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenBlockList(persistentIds);
}

DMError ScreenManager::RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenBlockList(persistentIds);
}

DMError ScreenManager::SetScreenPrivacyMaskImage(ScreenId screenId,
    const std::shared_ptr<Media::PixelMap>& privacyMaskImg)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenPrivacyMaskImage(screenId, privacyMaskImg);
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
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", state:%{public}u, reason:%{public}u", screenId, state, reason);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetSpecifiedScreenPower(screenId, state, reason);
}

bool ScreenManager::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "state:%{public}u, reason:%{public}u", state, reason);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenPowerForAll(state, reason);
}

ScreenPowerState ScreenManager::GetScreenPower(ScreenId dmsScreenId)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenPower(dmsScreenId);
}

ScreenPowerState ScreenManager::GetScreenPower()
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenPower();
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
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64".", screenId);
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
        TLOGE(WmsLogTag::DMS, "displayInfo is null");
        return false;
    }
    ScreenId screenId = screenInfo->GetScreenId();
    TLOGD(WmsLogTag::DMS, "screenId:%{public}" PRIu64".", screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "displayId is invalid.");
        return false;
    }
    auto iter = screenMap_.find(screenId);
    if (iter != screenMap_.end() && iter->second != nullptr) {
        TLOGD(WmsLogTag::DMS, "Screen Info Updated: %{public}s",
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
        TLOGE(WmsLogTag::DMS, "screenInfo nullptr.");
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
    TLOGD(WmsLogTag::DMS, "dms is died");
    {
        std::lock_guard<std::mutex> agentLock(virtualScreenAgentMutex_);
        virtualScreenAgent_ = nullptr;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    screenManagerListener_ = nullptr;
}

void ScreenManager::OnRemoteDied()
{
    pImpl_->OnRemoteDied();
}

bool ScreenManager::SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenStatus(screenId, screenStatus);
}

DMError ScreenManager::SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
    uint32_t& actualRefreshRate)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenMaxRefreshRate(id,
        refreshRate, actualRefreshRate);
}

void ScreenManager::SetFoldStatusExpandAndLocked(bool locked)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetFoldStatusExpandAndLocked(locked);
}

DMError ScreenManager::SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable)
{
    if (screenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "screenIds is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenSkipProtectedWindow(screenIds, isEnable);
}

DMError ScreenManager::SetVirtualScreenAutoRotation(ScreenId screenId, bool enable)
{
    return pImpl_->SetVirtualScreenAutoRotation(screenId, enable);
}

DMError ScreenManager::Impl::SetVirtualScreenAutoRotation(ScreenId screenId, bool enable)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualScreenAutoRotation(screenId, enable);
}

DMError ScreenManager::SetScreenPrivacyWindowTagSwitch(ScreenId screenId,
    const std::vector<std::string>& privacyWindowTag, bool enable)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenPrivacyWindowTagSwitch(screenId,
        privacyWindowTag, enable);
}
} // namespace OHOS::Rosen
