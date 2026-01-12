/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "session/host/include/session.h"

#include <application_context.h>
#include <regex>
#include <string>

#include "ability_info.h"
#include "input_manager.h"
#include "key_event.h"
#include "pointer_event.h"
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>
#include "proxy/include/window_info.h"

#include "common/include/session_permission.h"
#include "fold_screen_state_internel.h"
#include "image_source.h"
#include "rs_adapter.h"
#include "session_coordinate_helper.h"
#include "session_helper.h"
#include "surface_capture_future.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "parameters.h"
#include <hisysevent.h>
#include "hitrace_meter.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/pc_fold_screen_manager.h"
#include "perform_reporter.h"
#include "session/host/include/scene_persistent_storage.h"
#include "screen_manager.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "Session" };
std::atomic<int32_t> g_persistentId = INVALID_SESSION_ID;
std::set<int32_t> g_persistentIdSet;
std::mutex g_persistentIdSetMutex;
constexpr float INNER_BORDER_VP = 5.0f;
constexpr float OUTSIDE_BORDER_VP = 4.0f;
constexpr float INNER_ANGLE_VP = 16.0f;
constexpr uint32_t MAX_LIFE_CYCLE_TASK_IN_QUEUE = 15;
constexpr int64_t LIFE_CYCLE_TASK_EXPIRED_TIME_LIMIT = 350;
static bool g_enableForceUIFirst = system::GetParameter("window.forceUIFirst.enabled", "1") == "1";
constexpr int64_t STATE_DETECT_DELAYTIME = 6 * 1000;
constexpr DisplayId VIRTUAL_DISPLAY_ID = 999;
constexpr int32_t TIMES_TO_WAIT_FOR_VSYNC_ONECE = 1;
constexpr int32_t TIMES_TO_WAIT_FOR_VSYNC_TWICE = 2;
const uint64_t PRELAUNCH_DONE_TIME = system::GetIntParameter("window.prelaunchDoneTime", 6000);

const std::map<SessionState, bool> ATTACH_MAP = {
    { SessionState::STATE_DISCONNECT, false },
    { SessionState::STATE_CONNECT, false },
    { SessionState::STATE_FOREGROUND, true },
    { SessionState::STATE_ACTIVE, true },
    { SessionState::STATE_INACTIVE, false },
    { SessionState::STATE_BACKGROUND, false },
};
const std::map<SessionState, bool> DETACH_MAP = {
    { SessionState::STATE_DISCONNECT, true },
    { SessionState::STATE_CONNECT, false },
    { SessionState::STATE_FOREGROUND, false },
    { SessionState::STATE_ACTIVE, false },
    { SessionState::STATE_INACTIVE, true },
    { SessionState::STATE_BACKGROUND, true },
};
const bool CORRECTION_ENABLE = system::GetIntParameter<int32_t>("const.system.sensor_correction_enable", 0) == 1;
const uint32_t ROTATION_90 = 90;
const uint32_t ROTATION_360 = 360;
const uint32_t ROTATION_LANDSCAPE_INVERTED = 3;
const std::string APP_CAST_SCREEN_NAME = "HwCast_AppModeDisplay";
constexpr float BLUR_SNAPSHOT_SCALE = 0.5f;
} // namespace

std::shared_ptr<AppExecFwk::EventHandler> Session::mainHandler_;
bool Session::isScbCoreEnabled_ = false;
bool Session::isBackgroundUpdateRectNotifyEnabled_ = false;

Session::Session(const SessionInfo& info) : sessionInfo_(info)
{
    property_ = sptr<WindowSessionProperty>::MakeSptr();
    property_->SetWindowType(static_cast<WindowType>(info.windowType_));
    layoutController_ = sptr<LayoutController>::MakeSptr(property_);
    layoutController_->SetSystemConfigFunc([this]() {
        return this->GetSystemConfig();
    });

    if (!mainHandler_) {
        auto runner = AppExecFwk::EventRunner::GetMainEventRunner();
        mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }

    using type = std::underlying_type_t<MMI::WindowArea>;
    for (type area = static_cast<type>(MMI::WindowArea::FOCUS_ON_TOP);
        area <= static_cast<type>(MMI::WindowArea::FOCUS_ON_BOTTOM_RIGHT); ++area) {
        auto ret = windowAreas_.insert(
            std::pair<MMI::WindowArea, WSRectF>(static_cast<MMI::WindowArea>(area), WSRectF()));
        if (!ret.second) {
            WLOGFE("Failed to insert area:%{public}d", area);
        }
    }

    if (info.want != nullptr) {
        auto focusedOnShow = info.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WINDOW_FOCUSED, true);
        TLOGD(WmsLogTag::WMS_FOCUS, "focusedOnShow:%{public}d", focusedOnShow);
        SetFocusedOnShow(focusedOnShow);
    }

    static const std::regex pattern(R"(^SCBScreenLock[0-9]+$)");
    if (std::regex_match(info.bundleName_, pattern)) {
        TLOGD(WmsLogTag::WMS_LIFE, "bundleName: %{public}s", info.bundleName_.c_str());
        isScreenLockWindow_ = true;
    }
}

Session::~Session()
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d", GetPersistentId());
    if (mainHandler_) {
        mainHandler_->PostTask([surfaceNode = std::move(surfaceNode_),
                                shadowSurfaceNode = std::move(shadowSurfaceNode_),
                                leashWinShadowSurfaceNode = std::move(leashWinShadowSurfaceNode_)]() {
            // do nothing
        });
    }
}

void Session::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler,
    const std::shared_ptr<AppExecFwk::EventHandler>& exportHandler)
{
    handler_ = handler;
    exportHandler_ = exportHandler;
}

void Session::PostTask(Task&& task, const std::string& name, int64_t delayTime)
{
    if (!handler_ || handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "s:%s", name.c_str());
        return task();
    }
    auto localTask = [task = std::move(task), name] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "s:%s", name.c_str());
        task();
    };
    handler_->PostTask(std::move(localTask), "wms:" + name, delayTime, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void Session::PostExportTask(Task&& task, const std::string& name, int64_t delayTime)
{
    if (!exportHandler_ || exportHandler_->GetEventRunner()->IsCurrentRunnerThread()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "s:%s", name.c_str());
        return task();
    }
    auto localTask = [task = std::move(task), name] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "s:%s", name.c_str());
        task();
    };
    exportHandler_->PostTask(std::move(localTask), "wms:" + name, delayTime,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

int32_t Session::GetPersistentId() const
{
    return persistentId_;
}

int32_t Session::GetCurrentRotation() const
{
    return currentRotation_;
}

void Session::SetSurfaceNode(const std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    RSAdapterUtil::SetRSUIContext(surfaceNode, GetRSUIContext(), true);
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    surfaceNode_ = surfaceNode;
    shadowSurfaceNode_ = RSAdapterUtil::IsClientMultiInstanceEnabled() && surfaceNode_ ?
        surfaceNode_->CreateShadowSurfaceNode() : nullptr;
}

std::shared_ptr<RSSurfaceNode> Session::GetSurfaceNode() const
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    return surfaceNode_;
}

std::shared_ptr<RSSurfaceNode> Session::GetSurfaceNode(bool isUpdateContextBeforeGet)
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    if (isUpdateContextBeforeGet) {
        TLOGI(WmsLogTag::WMS_SCB,
              "id: %{public}d, surfaceNode: %{public}s, original %{public}s",
              GetPersistentId(),
              RSAdapterUtil::RSNodeToStr(surfaceNode_).c_str(),
              RSAdapterUtil::RSUIContextToStr(GetRSUIContext()).c_str());
        RSAdapterUtil::SetRSUIContext(surfaceNode_, GetRSUIContext(), true);
    }
    return surfaceNode_;
}

std::shared_ptr<RSSurfaceNode> Session::GetShadowSurfaceNode() const
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    return RSAdapterUtil::IsClientMultiInstanceEnabled() ? shadowSurfaceNode_ : surfaceNode_;
}

std::optional<NodeId> Session::GetSurfaceNodeId() const
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    if (surfaceNode_ != nullptr) {
        return surfaceNode_->GetId();
    }
    return std::nullopt;
}

void Session::SetLeashWinSurfaceNode(std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode)
{
    auto rsUIContext = GetRSUIContext();
    RSAdapterUtil::SetRSUIContext(leashWinSurfaceNode, rsUIContext, true);
    if (g_enableForceUIFirst) {
        AutoRSTransaction trans(rsUIContext);
        if (!leashWinSurfaceNode && leashWinSurfaceNode_) {
            leashWinSurfaceNode_->SetForceUIFirst(false);
        }
    }
    std::lock_guard<std::mutex> lock(leashWinSurfaceNodeMutex_);
    leashWinSurfaceNode_ = leashWinSurfaceNode;
    leashWinShadowSurfaceNode_ = RSAdapterUtil::IsClientMultiInstanceEnabled() && leashWinSurfaceNode_ ?
        leashWinSurfaceNode_->CreateShadowSurfaceNode() : nullptr;
}

void Session::SetFrameLayoutFinishListener(const NotifyFrameLayoutFinishFunc& func)
{
    frameLayoutFinishFunc_ = func;
}

std::shared_ptr<RSSurfaceNode> Session::GetLeashWinSurfaceNode() const
{
    std::lock_guard<std::mutex> lock(leashWinSurfaceNodeMutex_);
    return leashWinSurfaceNode_;
}

std::shared_ptr<RSSurfaceNode> Session::GetLeashWinShadowSurfaceNode() const
{
    std::lock_guard<std::mutex> lock(leashWinSurfaceNodeMutex_);
    return RSAdapterUtil::IsClientMultiInstanceEnabled() ? leashWinShadowSurfaceNode_ : leashWinSurfaceNode_;
}

std::shared_ptr<RSSurfaceNode> Session::GetSurfaceNodeForMoveDrag() const
{
    auto movedSurfaceNode = GetLeashWinSurfaceNode();
    if (!movedSurfaceNode) {
        movedSurfaceNode = GetSurfaceNode();
    }
    return movedSurfaceNode;
}

std::shared_ptr<Media::PixelMap> Session::GetSnapshot() const
{
    std::lock_guard<std::mutex> lock(snapshotMutex_);
    return snapshot_;
}

std::shared_ptr<Media::PixelMap> Session::GetPreloadSnapshot() const
{
    std::lock_guard<std::mutex> lock(preloadSnapshotMutex_);
    return preloadSnapshot_;
}

void Session::SetSessionInfoAncoSceneState(int32_t ancoSceneState)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.ancoSceneState = ancoSceneState;
}

void Session::SetSessionInfoTime(const std::string& time)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.time = time;
}

void Session::SetSessionInfoAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.abilityInfo = abilityInfo;
}

void Session::SetSessionInfoSupportedWindowModes(const std::vector<AppExecFwk::SupportWindowMode>& updatedWindowModes)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.supportedWindowModes = updatedWindowModes;
}

void Session::SetSessionInfoWant(const std::shared_ptr<AAFwk::Want>& want)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.want = want;
}

void Session::ResetSessionInfoResultCode()
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.resultCode = -1; // -1: initial result code
}

void Session::SetSessionInfoPersistentId(int32_t persistentId)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.persistentId_ = persistentId;
}

void Session::SetSessionInfoCallerPersistentId(int32_t callerPersistentId)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.callerPersistentId_ = callerPersistentId;
}

void Session::SetSessionInfoContinueState(ContinueState state)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.continueState = state;
}

void Session::SetSessionInfoLockedState(bool lockedState)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.lockedState = lockedState;
    NotifySessionInfoLockedStateChange(lockedState);
}

void Session::SetSessionInfoIsClearSession(bool isClearSession)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.isClearSession = isClearSession;
}

void Session::SetSessionInfoAffinity(std::string affinity)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.sessionAffinity = affinity;
}

void Session::GetCloseAbilityWantAndClean(AAFwk::Want& outWant)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    if (sessionInfo_.closeAbilityWant != nullptr) {
        outWant = *sessionInfo_.closeAbilityWant;
        sessionInfo_.closeAbilityWant = nullptr;
    }
}

void Session::SetSessionInfo(const SessionInfo& info)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.want = info.want;
    sessionInfo_.callerToken_ = info.callerToken_;
    sessionInfo_.requestCode = info.requestCode;
    sessionInfo_.scenarios = info.scenarios;
    sessionInfo_.callerPersistentId_ = info.callerPersistentId_;
    sessionInfo_.callingTokenId_ = info.callingTokenId_;
    sessionInfo_.uiAbilityId_ = info.uiAbilityId_;
    sessionInfo_.requestId = info.requestId;
    sessionInfo_.startSetting = info.startSetting;
    if (!info.continueSessionId_.empty()) {
        sessionInfo_.continueSessionId_ = info.continueSessionId_;
    }
    sessionInfo_.isAtomicService_ = info.isAtomicService_;
    sessionInfo_.callState_ = info.callState_;
    sessionInfo_.processOptions = info.processOptions;
    sessionInfo_.disableDelegator = info.disableDelegator;
    sessionInfo_.reuseDelegatorWindow = info.reuseDelegatorWindow;
}

void Session::SetSessionInfoWindowInputType(uint32_t windowInputType)
{
    TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, type:%{public}u", GetPersistentId(), windowInputType);
    {
        std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
        sessionInfo_.windowInputType_ = windowInputType;
    }
    NotifySessionInfoChange();
}

void Session::SetSessionInfoExpandInputFlag(uint32_t expandInputFlag)
{
    {
        std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
        if (sessionInfo_.expandInputFlag_ != expandInputFlag) {
            sessionInfo_.expandInputFlag_ = expandInputFlag;
            TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, flag:%{public}u", GetPersistentId(), expandInputFlag);
        }
    }
    NotifySessionInfoChange();
}

uint32_t Session::GetSessionInfoExpandInputFlag() const
{
    return sessionInfo_.expandInputFlag_;
}

void Session::SetSessionInfoCursorDragCount(int32_t count)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.cursorDragCount_ = count;
}

int32_t Session::GetSessionInfoCursorDragCount()
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    return sessionInfo_.cursorDragCount_;
}

void Session::SetSessionInfoCursorDragFlag(bool value)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.cursorDragFlag_ = value;
}

bool Session::GetSessionInfoCursorDragFlag()
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    return sessionInfo_.cursorDragFlag_;
}

void Session::SetSessionInfoReceiveDragEventEnabled(bool value)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.isReceiveDragEventEnabled_ = value;
}

bool Session::GetSessionInfoReceiveDragEventEnabled()
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    return sessionInfo_.isReceiveDragEventEnabled_;
}

void Session::SetSessionInfoSeparationTouchEnabled(bool value)
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    sessionInfo_.isSeparationTouchEnabled_ = value;
}

bool Session::GetSessionInfoSeparationTouchEnabled()
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    return sessionInfo_.isSeparationTouchEnabled_;
}

void Session::SetSessionInfoAdvancedFeatureFlag(uint32_t bitPosition, bool value)
{
    if (bitPosition >= ADVANCED_FEATURE_BIT_MAX) {
        TLOGE(WmsLogTag::WMS_EVENT, "id:%{public}d, bitPosition:%{public}u", GetPersistentId(), bitPosition);
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
        sessionInfo_.advancedFeatureFlag_.set(bitPosition, value);
    }
}

bool Session::GetSessionInfoAdvancedFeatureFlag(uint32_t bitPosition)
{
    if (bitPosition >= ADVANCED_FEATURE_BIT_MAX) {
        TLOGE(WmsLogTag::WMS_EVENT, "id:%{public}d, bitPosition:%{public}u", GetPersistentId(), bitPosition);
        return false;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
        return sessionInfo_.advancedFeatureFlag_.test(bitPosition);
    }
}

void Session::SetSessionInfoWindowMode(int32_t windowMode)
{
    sessionInfo_.windowMode = windowMode;
}

DisplayId Session::GetScreenId() const
{
    return sessionInfo_.screenId_;
}

void Session::SetScreenId(uint64_t screenId)
{
    sessionInfo_.screenId_ = screenId;
    if (sessionStage_) {
        sessionStage_->UpdateDisplayId(screenId);
    }
    RSAdapterUtil::SetRSUIContext(GetSurfaceNode(), GetRSUIContext(), true);
}

void Session::SetAppInstanceKey(const std::string& appInstanceKey)
{
    sessionInfo_.appInstanceKey_ = appInstanceKey;
}

std::shared_ptr<AppExecFwk::AbilityInfo> Session::GetSessionInfoAbilityInfo()
{
    std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
    return sessionInfo_.abilityInfo;
}

std::string Session::GetAppInstanceKey() const
{
    return sessionInfo_.appInstanceKey_;
}

const SessionInfo& Session::GetSessionInfo() const
{
    return sessionInfo_;
}

SessionInfo& Session::EditSessionInfo()
{
    return sessionInfo_;
}

bool Session::RegisterLifecycleListener(const std::shared_ptr<ILifecycleListener>& listener)
{
    return RegisterListenerLocked(lifecycleListeners_, listener);
}

bool Session::UnregisterLifecycleListener(const std::shared_ptr<ILifecycleListener>& listener)
{
    return UnregisterListenerLocked(lifecycleListeners_, listener);
}

template<typename T>
bool Session::RegisterListenerLocked(std::vector<std::shared_ptr<T>>& holder, const std::shared_ptr<T>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(lifecycleListenersMutex_);
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        WLOGFE("Listener already registered");
        return false;
    }
    holder.emplace_back(listener);
    return true;
}

template<typename T>
bool Session::UnregisterListenerLocked(std::vector<std::shared_ptr<T>>& holder, const std::shared_ptr<T>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(lifecycleListenersMutex_);
    holder.erase(std::remove_if(holder.begin(), holder.end(),
        [listener](std::shared_ptr<T> registeredListener) { return registeredListener == listener; }),
        holder.end());
    return true;
}

void Session::NotifyActivation()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnActivation();
        }
    }
    snapshotNeedCancel_.store(true);
}

void Session::NotifyConnect()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnConnect();
        }
    }
}

void Session::NotifyForeground()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnForeground();
        }
    }
}

void Session::NotifyBackground()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnBackground();
        }
    }
}

void Session::NotifyDisconnect()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnDisconnect();
        }
    }
}

void Session::NotifyLayoutFinished()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnLayoutFinished();
        }
    }
}

void Session::NotifyRemoveBlank()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnRemoveBlank();
        }
    }
}

void Session::NotifyPreLoadStartingWindowFinished()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnPreLoadStartingWindowFinished();
        }
    }
}

void Session::NotifyRestart()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnRestart();
        }
    }
}

void Session::NotifyAddSnapshot(bool useFfrt, bool needPersist, bool needSaveSnapshot)
{
    /*
     * for blankness prolems, persist snapshot could conflict with background process,
     * thus no need to persist snapshot here
     */
    if (needSaveSnapshot) {
        SaveSnapshot(useFfrt, needPersist);
    }
    auto task = [weakThis = wptr(this), where = __func__]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s session is null", where);
            return;
        }
        auto lifecycleListeners = session->GetListeners<ILifecycleListener>();
        for (auto& listener : lifecycleListeners) {
            if (auto listenerPtr = listener.lock()) {
                listenerPtr->OnAddSnapshot();
            }
        }
    };

    if (useFfrt) {
        SetAddSnapshotCallback(task);
    } else {
        task();
    }
}

void Session::NotifyRemoveSnapshot()
{
    if (GetAppLockControl()) {
        TLOGI(WmsLogTag::WMS_PATTERN, "not allowed");
        return;
    }
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnRemoveSnapshot();
        }
    }
}

void Session::NotifyUpdateSnapshotWindow()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnUpdateSnapshotWindow();
        }
    }
}

void Session::NotifyExtensionDied()
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "permission denied!");
        return;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "NotifyExtensionDied called in session(persistentId:%{public}d).", persistentId_);
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnExtensionDied();
        }
    }
}

void Session::NotifyExtensionTimeout(int32_t errorCode)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "permission denied!");
        return;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "NotifyExtensionTimeout(errorCode:%{public}d) in session(persistentId:%{public}d).",
        errorCode, persistentId_);
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnExtensionTimeout(errorCode);
        }
    }
}

void Session::NotifyTransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
    int64_t uiExtensionIdLevel)
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnAccessibilityEvent(info, uiExtensionIdLevel);
        }
    }
}

void Session::NotifyExtensionDetachToDisplay()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "called");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "permission denied!");
        return;
    }

    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnExtensionDetachToDisplay();
        }
    }
}

float Session::GetAspectRatio() const
{
    return layoutController_->GetAspectRatio();
}

WSError Session::SetAspectRatio(float ratio)
{
    layoutController_->SetAspectRatio(ratio);
    return WSError::WS_OK;
}

SessionState Session::GetSessionState() const
{
    return state_;
}

void Session::SetSessionState(SessionState state)
{
    if (state < SessionState::STATE_DISCONNECT || state > SessionState::STATE_END) {
        WLOGFD("Invalid session state: %{public}u", state);
        return;
    }
    state_ = state;
    SetMainSessionUIStateDirty(true);
}

void Session::UpdateSessionState(SessionState state)
{
    // Remove unexecuted detection tasks when the window state changes to background or destroyed.
    if (state == SessionState::STATE_DISCONNECT ||
        state == SessionState::STATE_INACTIVE ||
        state == SessionState::STATE_BACKGROUND) {
        RemoveWindowDetectTask();
    }
    state_ = state;
    SetMainSessionUIStateDirty(true);
    NotifySessionStateChange(state);
}

void Session::UpdateSessionTouchable(bool touchable)
{
    GetSessionProperty()->SetTouchable(touchable);
    NotifySessionTouchableChange(touchable);
}

WSError Session::SetFocusable(bool isFocusable)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d, focusable: %{public}d", GetPersistentId(), isFocusable);
    GetSessionProperty()->SetFocusable(isFocusable);
    if (isFocused_ && !GetFocusable()) {
        FocusChangeReason reason = FocusChangeReason::FOCUSABLE;
        NotifyRequestFocusStatusNotifyManager(false, true, reason);
    }
    return WSError::WS_OK;
}

void Session::SetSystemFocusable(bool systemFocusable)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d, systemFocusable: %{public}d", GetPersistentId(), systemFocusable);
    systemFocusable_ = systemFocusable;
    if (isFocused_ && !systemFocusable) {
        FocusChangeReason reason = FocusChangeReason::FOCUSABLE;
        NotifyRequestFocusStatusNotifyManager(false, true, reason);
    }
}

WSError Session::SetFocusableOnShow(bool isFocusableOnShow)
{
    PostTask([weakThis = wptr(this), isFocusableOnShow]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "session is null");
            return;
        }
        TLOGND(WmsLogTag::WMS_FOCUS, "id: %{public}d, focusableOnShow: %{public}d",
            session->GetPersistentId(), isFocusableOnShow);
        session->focusableOnShow_ = isFocusableOnShow;
    }, __func__);
    return WSError::WS_OK;
}

bool Session::GetFocusable() const
{
    return GetSessionProperty()->GetFocusable();
}

bool Session::GetSystemFocusable() const
{
    if (parentSession_) {
        return systemFocusable_ && parentSession_->GetSystemFocusable();
    }
    return systemFocusable_;
}

bool Session::CheckFocusable() const
{
    return GetFocusable() && GetSystemFocusable();
}

bool Session::IsFocusableOnShow() const
{
    return focusableOnShow_;
}

bool Session::IsFocused() const
{
    return isFocused_;
}

void Session::SetNeedNotify(bool needNotify)
{
    needNotify_ = needNotify;
}

bool Session::NeedNotify() const
{
    return needNotify_;
}

void Session::SetFocusedOnShow(bool focusedOnShow)
{
    if (focusedOnShow == focusedOnShow_) {
        return;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "[%{public}d, %{public}d]", focusedOnShow, GetPersistentId());
    focusedOnShow_ = focusedOnShow;
}

bool Session::IsFocusedOnShow() const
{
    TLOGD(WmsLogTag::WMS_FOCUS, "IsFocusedOnShow:%{public}d, id: %{public}d", focusedOnShow_, GetPersistentId());
    return focusedOnShow_;
}

void Session::SetStartingBeforeVisible(bool isStartingBeforeVisible)
{
    isStartingBeforeVisible_ = isStartingBeforeVisible;
}

bool Session::GetStartingBeforeVisible() const
{
    return isStartingBeforeVisible_;
}

WSError Session::SetTouchable(bool touchable)
{
    SetSystemTouchable(touchable);
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (touchable != GetSessionProperty()->GetTouchable()) {
        TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, %{public}d", GetPersistentId(), touchable);
    }
    UpdateSessionTouchable(touchable);
    return WSError::WS_OK;
}

bool Session::GetTouchable() const
{
    return GetSessionProperty()->GetTouchable();
}

void Session::SetForceTouchable(bool forceTouchable)
{
    if (forceTouchable != forceTouchable_) {
        TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, %{public}d", GetPersistentId(), forceTouchable);
    }
    forceTouchable_ = forceTouchable;
}

void Session::SetSystemTouchable(bool touchable)
{
    if (touchable != systemTouchable_) {
        TLOGNI(WmsLogTag::WMS_EVENT, "sysTouch:%{public}d, %{public}d", GetPersistentId(), touchable);
    }
    systemTouchable_ = touchable;
    NotifySessionInfoChange();
}

bool Session::GetSystemTouchable() const
{
    return forceTouchable_ && systemTouchable_ && GetTouchable();
}

bool Session::GetWindowTouchableForMMI(DisplayId displayId) const
{
    bool isTouchable = true;
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    if (screenSession != nullptr) {
        if (!screenSession->IsTouchEnabled() || !GetSystemTouchable() ||
            !GetForegroundInteractiveStatus()) {
            isTouchable = false;
        }
    }
    return isTouchable;
}

bool Session::IsSystemActive() const
{
    return isSystemActive_;
}

void Session::SetSystemActive(bool systemActive)
{
    isSystemActive_ = systemActive;
    NotifySessionInfoChange();
}

WSError Session::SetRSVisible(bool isVisible)
{
    isRSVisible_ = isVisible;
    return WSError::WS_OK;
}

bool Session::GetRSVisible() const
{
    return isRSVisible_;
}

bool Session::GetFocused() const
{
    return isFocused_;
}

WSError Session::SetVisibilityState(WindowVisibilityState state)
{
    visibilityState_ = state;
    return WSError::WS_OK;
}

WindowVisibilityState Session::GetVisibilityState() const
{
    return visibilityState_;
}

WSError Session::SetDrawingContentState(bool isRSDrawing)
{
    isRSDrawing_ = isRSDrawing;
    return WSError::WS_OK;
}

bool Session::GetDrawingContentState() const
{
    return isRSDrawing_;
}

int32_t Session::GetWindowId() const
{
    return GetPersistentId();
}

void Session::SetCallingPid(int32_t id)
{
    TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, %{public}d", persistentId_, id);
    callingPid_ = id;
    if (visibilityChangedDetectFunc_ && isVisible_) {
        visibilityChangedDetectFunc_(callingPid_, false, isVisible_);
    }
}

void Session::SetCallingUid(int32_t id)
{
    callingUid_ = id;
}

int32_t Session::GetCallingPid() const
{
    return callingPid_;
}

int32_t Session::GetCallingUid() const
{
    return callingUid_;
}

void Session::SetAbilityToken(sptr<IRemoteObject> token)
{
    abilityToken_ = token;
}

sptr<IRemoteObject> Session::GetAbilityToken() const
{
    return abilityToken_;
}

WSError Session::UpdateBrightness(float brightness)
{
    if (sessionStage_) {
        sessionStage_->UpdateBrightness(brightness);
    }
    return WSError::WS_OK;
}

WSError Session::SetBrightness(float brightness)
{
    GetSessionProperty()->SetBrightness(brightness);
    return WSError::WS_OK;
}

float Session::GetBrightness() const
{
    return GetSessionProperty()->GetBrightness();
}

bool Session::IsSessionValid() const
{
    if (sessionInfo_.isSystem_) {
        WLOGFD("session is system, id: %{public}d, name: %{public}s, state: %{public}u",
            GetPersistentId(), sessionInfo_.bundleName_.c_str(), GetSessionState());
        return false;
    }
    bool res = state_ > SessionState::STATE_DISCONNECT && state_ < SessionState::STATE_END;
    return res;
}

bool Session::IsActive() const
{
    return isActive_;
}

bool Session::IsSystemSession() const
{
    return sessionInfo_.isSystem_;
}

bool Session::IsTerminated() const
{
    return (GetSessionState() == SessionState::STATE_DISCONNECT || isTerminating_);
}

bool Session::IsSessionForeground() const
{
    return state_ == SessionState::STATE_FOREGROUND || state_ == SessionState::STATE_ACTIVE;
}

bool Session::IsSessionNotBackground() const
{
    return state_ >= SessionState::STATE_DISCONNECT && state_ <= SessionState::STATE_ACTIVE;
}

WSRectF Session::UpdateTopBottomArea(const WSRectF& rect, MMI::WindowArea area)
{
    const float innerBorder = INNER_BORDER_VP * vpr_;
    const float outsideBorder = OUTSIDE_BORDER_VP * vpr_;
    const float innerAngle = INNER_ANGLE_VP * vpr_;
    const float horizontalBorderLength = outsideBorder + innerAngle;
    const float verticalBorderLength = outsideBorder + innerBorder;
    const size_t innerAngleCount = 2;
    WSRectF tbRect;
    tbRect.posX_ = rect.posX_ + horizontalBorderLength;
    tbRect.width_ = rect.width_ - horizontalBorderLength * innerAngleCount;
    tbRect.height_ = verticalBorderLength;
    if (area == MMI::WindowArea::FOCUS_ON_TOP) {
        tbRect.posY_ = rect.posY_;
    } else if (area == MMI::WindowArea::FOCUS_ON_BOTTOM) {
        tbRect.posY_ = rect.posY_ + rect.height_ - verticalBorderLength;
    } else {
        return WSRectF();
    }
    return tbRect;
}

WSRectF Session::UpdateLeftRightArea(const WSRectF& rect, MMI::WindowArea area)
{
    const float innerBorder = INNER_BORDER_VP * vpr_;
    const float outsideBorder = OUTSIDE_BORDER_VP * vpr_;
    const float innerAngle = INNER_ANGLE_VP * vpr_;
    const float verticalBorderLength = outsideBorder + innerAngle;
    const float horizontalBorderLength = outsideBorder + innerBorder;
    const size_t innerAngleCount = 2;
    WSRectF lrRect;
    lrRect.posY_ = rect.posY_ + verticalBorderLength;
    lrRect.width_ = horizontalBorderLength;
    lrRect.height_ = rect.height_ - verticalBorderLength * innerAngleCount;
    if (area == MMI::WindowArea::FOCUS_ON_LEFT) {
        lrRect.posX_ = rect.posX_;
    } else if (area == MMI::WindowArea::FOCUS_ON_RIGHT) {
        lrRect.posX_ = rect.posX_ + rect.width_ - horizontalBorderLength;
    } else {
        return WSRectF();
    }
    return lrRect;
}

WSRectF Session::UpdateInnerAngleArea(const WSRectF& rect, MMI::WindowArea area)
{
    const float outsideBorder = OUTSIDE_BORDER_VP * vpr_;
    const float innerAngle = INNER_ANGLE_VP * vpr_;
    WSRectF iaRect;
    iaRect.width_ = outsideBorder + innerAngle;
    iaRect.height_ = outsideBorder + innerAngle;
    if (area == MMI::WindowArea::FOCUS_ON_TOP_LEFT) {
        iaRect.posX_ = rect.posX_;
        iaRect.posY_ = rect.posY_;
    } else if (area == MMI::WindowArea::FOCUS_ON_TOP_RIGHT) {
        iaRect.posX_ = rect.posX_ + rect.width_ - iaRect.width_;
        iaRect.posY_ = rect.posY_;
    } else if (area == MMI::WindowArea::FOCUS_ON_BOTTOM_LEFT) {
        iaRect.posX_ = rect.posX_;
        iaRect.posY_ = rect.posY_ + rect.height_ - iaRect.height_;
    } else if (area == MMI::WindowArea::FOCUS_ON_BOTTOM_RIGHT) {
        iaRect.posX_ = rect.posX_ + rect.width_ - iaRect.width_;
        iaRect.posY_ = rect.posY_ + rect.height_ - iaRect.height_;
    } else {
        return WSRectF();
    }
    return iaRect;
}

WSRectF Session::UpdateHotRect(const WSRect& rect)
{
    WSRectF newRect;
    const float outsideBorder = OUTSIDE_BORDER_VP * vpr_;
    const size_t outsideBorderCount = 2;
    newRect.posX_ = rect.posX_ - outsideBorder;
    newRect.posY_ = rect.posY_ - outsideBorder;
    newRect.width_ = rect.width_ + outsideBorder * outsideBorderCount;
    newRect.height_ = rect.height_ + outsideBorder * outsideBorderCount;
    return newRect;
}

void Session::UpdatePointerArea(const WSRect& rect)
{
    if (preRect_ == rect) {
        WLOGFD("The window area does not change");
        return;
    }
    WSRectF hotRect = UpdateHotRect(rect);
    for (const auto &[area, _] : windowAreas_) {
        if (area == MMI::WindowArea::FOCUS_ON_TOP || area == MMI::WindowArea::FOCUS_ON_BOTTOM) {
            windowAreas_[area] = UpdateTopBottomArea(hotRect, area);
        } else if (area == MMI::WindowArea::FOCUS_ON_RIGHT || area == MMI::WindowArea::FOCUS_ON_LEFT) {
            windowAreas_[area] = UpdateLeftRightArea(hotRect, area);
        } else if (area == MMI::WindowArea::FOCUS_ON_TOP_LEFT || area == MMI::WindowArea::FOCUS_ON_TOP_RIGHT ||
            area == MMI::WindowArea::FOCUS_ON_BOTTOM_LEFT || area == MMI::WindowArea::FOCUS_ON_BOTTOM_RIGHT) {
            windowAreas_[area] = UpdateInnerAngleArea(hotRect, area);
        }
    }
    preRect_ = rect;
}

WSError Session::UpdateSizeChangeReason(SizeChangeReason reason)
{
    if (GetSizeChangeReason() == reason) {
        return WSError::WS_DO_NOTHING;
    }
    GetLayoutController()->UpdateSizeChangeReason(reason);
    return WSError::WS_OK;
}

bool Session::IsDraggingReason(SizeChangeReason reason) const
{
    return reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_MOVE ||
           reason == SizeChangeReason::DRAG_START;
}

void Session::SetClientDisplayIdChangeListener(const NotifyClientDisplayIdChangeFunc& func)
{
    clientDisplayIdChangeFunc_ = func;
}

WSError Session::UpdateClientDisplayId(DisplayId displayId)
{
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "sessionStage_ is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d move display %{public}" PRIu64 " from %{public}" PRIu64,
          GetPersistentId(), displayId, clientDisplayId_);
    sessionStage_->UpdateDisplayId(displayId);
    if (displayId != clientDisplayId_) {
        AddPropertyDirtyFlags(static_cast<uint32_t>(SessionPropertyFlag::DISPLAY_ID));
        NotifyDisplayIdChanged(GetPersistentId(), displayId);
    }
    clientDisplayId_ = displayId;
    return WSError::WS_OK;
}

DisplayId Session::TransformGlobalRectToRelativeRect(WSRect& rect) const
{
    const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
        PcFoldScreenManager::GetInstance().GetDisplayRects();
    int32_t lowerScreenPosY = defaultDisplayRect.height_ + foldCreaseRect.height_;
    TLOGI(WmsLogTag::WMS_LAYOUT, "lowerScreenPosY: %{public}d", lowerScreenPosY);
    auto screenHeight = defaultDisplayRect.height_ + foldCreaseRect.height_ + virtualDisplayRect.height_;
    if (rect.posY_ > screenHeight) {
        rect.posY_ -= lowerScreenPosY;
        return clientDisplayId_;
    }
    DisplayId updatedDisplayId = DEFAULT_DISPLAY_ID;
    if (rect.posY_ >= lowerScreenPosY) {
        updatedDisplayId = VIRTUAL_DISPLAY_ID;
        rect.posY_ -= lowerScreenPosY;
    }
    return updatedDisplayId;
}

void Session::TransformRelativeRectToGlobalRect(WSRect& rect) const
{
    auto currScreenFoldStatus = PcFoldScreenManager::GetInstance().GetScreenFoldStatus();
    auto needTransRect = currScreenFoldStatus != SuperFoldStatus::UNKNOWN &&
        currScreenFoldStatus != SuperFoldStatus::FOLDED && currScreenFoldStatus != SuperFoldStatus::EXPANDED;
    auto isSystemKeyboard = GetSessionProperty() != nullptr && GetSessionProperty()->IsSystemKeyboard();
    if (isSystemKeyboard || !needTransRect) {
        return;
    }
    const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
        PcFoldScreenManager::GetInstance().GetDisplayRects();
    int32_t lowerScreenPosY = defaultDisplayRect.height_ + foldCreaseRect.height_;
    if (GetSessionGlobalRect().posY_ >= lowerScreenPosY) {
        WSRect relativeRect = rect;
        rect.posY_ += lowerScreenPosY;
        TLOGI(WmsLogTag::WMS_LAYOUT, "Transform relativeRect: %{public}s to globalRect: %{public}s",
            relativeRect.ToString().c_str(), rect.ToString().c_str());
    }
}

void Session::UpdateClientRectPosYAndDisplayId(WSRect& rect)
{
    if (GetSessionProperty()->IsSystemKeyboard()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "skip update SystemKeyboard: %{public}d", GetPersistentId());
        return;
    }
    if (WindowHelper::IsUIExtensionWindow(GetWindowType())) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "skip update UIExtension: %{public}d, rect: %{public}s",
            GetPersistentId(), rect.ToString().c_str());
        return;
    }
    if (rect.IsInvalid()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "skip window: %{public}d invalid rect: %{public}s",
            GetPersistentId(), rect.ToString().c_str());
        return;
    }
    auto currScreenFoldStatus = PcFoldScreenManager::GetInstance().GetScreenFoldStatus();
    if (currScreenFoldStatus == SuperFoldStatus::UNKNOWN || currScreenFoldStatus == SuperFoldStatus::FOLDED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Error status");
        return;
    }
    if (GetScreenId() != DISPLAY_ID_INVALID &&
        !PcFoldScreenManager::GetInstance().IsPcFoldScreen(GetScreenId())) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "winId: %{public}d, displayId: %{public}" PRIu64 " not need",
            GetPersistentId(), GetScreenId());
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "lastStatus: %{public}d, curStatus: %{public}d",
        lastScreenFoldStatus_, currScreenFoldStatus);
    if (currScreenFoldStatus == SuperFoldStatus::EXPANDED || currScreenFoldStatus == SuperFoldStatus::KEYBOARD) {
        lastScreenFoldStatus_ = currScreenFoldStatus;
        auto ret = UpdateClientDisplayId(DEFAULT_DISPLAY_ID);
        TLOGI(WmsLogTag::WMS_LAYOUT, "JustUpdateId: winId: %{public}d, result: %{public}d",
            GetPersistentId(), ret);
        return;
    }
    WSRect lastRect = rect;
    auto updatedDisplayId = TransformGlobalRectToRelativeRect(rect);
    auto ret = UpdateClientDisplayId(updatedDisplayId);
    lastScreenFoldStatus_ = currScreenFoldStatus;
    configDisplayId_ = DISPLAY_ID_INVALID;
    TLOGI(WmsLogTag::WMS_LAYOUT, "CalculatedRect: winId: %{public}d, input: %{public}s, output: %{public}s,"
        " result: %{public}d, clientDisplayId: %{public}" PRIu64, GetPersistentId(), lastRect.ToString().c_str(),
        rect.ToString().c_str(), ret, updatedDisplayId);
}

void Session::SetSingleHandTransform(const SingleHandTransform& transform)
{
    singleHandTransform_ = transform;
}

SingleHandTransform Session::GetSingleHandTransform() const
{
    return singleHandTransform_;
}

void Session::SetSingleHandModeFlag(bool flag)
{
    singleHandModeFlag_ = flag;
}

bool Session::SessionIsSingleHandMode()
{
    return singleHandModeFlag_;
}


WSError Session::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction)
{
    return UpdateRectWithLayoutInfo(rect, reason, updateReason, rsTransaction, {});
}

WSError Session::UpdateRectWithLayoutInfo(const WSRect& rect, SizeChangeReason reason,
    const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "session update rect: id: %{public}d, rect:%{public}s, "
        "reason:%{public}u %{public}s", GetPersistentId(), rect.ToString().c_str(), reason, updateReason.c_str());
    if (!IsSessionValid()) {
        GetLayoutController()->SetSessionRect(rect);
        TLOGD(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    GetLayoutController()->SetSessionRect(rect);
    WSRect updateRect = IsNeedConvertToRelativeRect(reason) ?
        GetLayoutController()->ConvertGlobalRectToRelative(rect, GetDisplayId()) : rect;

    // Window Layout Global Coordinate System
    auto globalDisplayRect = SessionCoordinateHelper::RelativeToGlobalDisplayRect(GetScreenId(), updateRect);
    UpdateGlobalDisplayRect(globalDisplayRect, reason);

    if (!Session::IsBackgroundUpdateRectNotifyEnabled() && !IsSessionForeground()) {
        return WSError::WS_DO_NOTHING;
    }
    if (sessionStage_ != nullptr) {
        int32_t rotateAnimationDuration = GetRotateAnimationDuration();
        SceneAnimationConfig config;
        config.rsTransaction_ = rsTransaction;
        config.animationDuration_ = rotateAnimationDuration;
        if (reason == SizeChangeReason::SCENE_WITH_ANIMATION) {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "UpdateRectWithLayoutInfo %{public}d", reason);
            config = sceneAnimationConfig_;
        }
        UpdateClientRectPosYAndDisplayId(updateRect);
        sessionStage_->UpdateRect(updateRect, reason, config, avoidAreas);
        SetClientRect(rect);
        NotifyWindowStatusDidChangeIfNeedWhenUpdateRect(reason);
        RectCheckProcess();
    } else {
        WLOGFE("sessionStage_ is nullptr");
    }
    UpdatePointerArea(GetSessionRect());
    return WSError::WS_OK;
}

void Session::NotifyWindowStatusDidChangeIfNeedWhenUpdateRect(SizeChangeReason reason)
{
    if (!sessionStage_) {
        return;
    }
    if (reason == SizeChangeReason::MAXIMIZE || reason == SizeChangeReason::MAXIMIZE_IN_IMPLICT) {
        sessionStage_->NotifyLayoutFinishAfterWindowModeChange(GetWindowMode());
    }
}

WSError Session::UpdateDensity()
{
    WLOGFI("session update density: id: %{public}d.", GetPersistentId());
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ != nullptr) {
        sessionStage_->UpdateDensity();
    } else {
        WLOGFE("Session::UpdateDensity sessionStage_ is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    return WSError::WS_OK;
}

WSError Session::UpdateOrientation()
{
    TLOGD(WmsLogTag::DMS, "id: %{public}d.", GetPersistentId());
    if (!IsSessionValid()) {
        TLOGE(WmsLogTag::DMS, "session is invalid, id=%{public}d.",
            GetPersistentId());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "sessionStage_ is nullptr, id=%{public}d.",
            GetPersistentId());
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->UpdateOrientation();
}

__attribute__((no_sanitize("cfi"))) WSError Session::ConnectInner(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    SystemSessionConfig& systemConfig, sptr<WindowSessionProperty> property,
    sptr<IRemoteObject> token, int32_t pid, int32_t uid, const std::string& identityToken)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] ConnectInner session, state: %{public}u,"
        "isTerminating:%{public}d, callingPid:%{public}d, disableDelegator:%{public}d", GetPersistentId(),
        static_cast<uint32_t>(GetSessionState()), isTerminating_, pid, property->GetIsAbilityHookOff());
    if (GetSessionState() != SessionState::STATE_DISCONNECT && !isTerminating_ &&
        !GetSessionInfo().reuseDelegatorWindow) {
        TLOGE(WmsLogTag::WMS_LIFE, "state is not disconnect state:%{public}u id:%{public}u!, reuse %{public}d",
            GetSessionState(), GetPersistentId(), GetSessionInfo().reuseDelegatorWindow);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage == nullptr || eventChannel == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session stage or eventChannel is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_ = sessionStage;
    NotifyAppHookWindowInfoUpdated();
    sessionStage_->SetCurrentRotation(currentRotation_);
    windowEventChannel_ = eventChannel;
    SetSurfaceNode(surfaceNode);
    abilityToken_ = token;
    systemConfig = systemConfig_;
    InitSessionPropertyWhenConnect(property);
    SetCallingPid(pid);
    callingUid_ = uid;
    UpdateSessionState(SessionState::STATE_CONNECT);
    WindowHelper::IsUIExtensionWindow(GetWindowType()) ?
        UpdateRect(GetSessionRect(), SizeChangeReason::UNDEFINED, "Connect") :
        NotifyClientToUpdateRect("Connect", nullptr);
    EditSessionInfo().disableDelegator = property->GetIsAbilityHookOff();
    NotifyConnect();
    if (WindowHelper::IsSubWindow(GetWindowType()) && surfaceNode_ != nullptr) {
        surfaceNode_->SetFrameGravity(Gravity::TOP_LEFT);
    }
    return WSError::WS_OK;
}

void Session::InitSessionPropertyWhenConnect(const sptr<WindowSessionProperty>& property)
{
    if (property == nullptr) {
        return;
    }
    if (GetSessionProperty()->GetIsNeedUpdateWindowMode()) {
        property->SetIsNeedUpdateWindowMode(true);
        property->SetWindowMode(GetSessionProperty()->GetWindowMode());
    }
    if (SessionHelper::IsMainWindow(GetWindowType()) && GetSessionInfo().screenId_ != SCREEN_ID_INVALID) {
        property->SetDisplayId(GetSessionInfo().screenId_);
    }
    if (systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode()) {
        InitSystemSessionDragEnable(property);
    } else {
        property->SetDragEnabled(false);
    }
    property->SetSessionPropertyChangeCallback(
        [weakThis = wptr(this)]() {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNE(WmsLogTag::DEFAULT, "session is null");
                return;
            }
            session->NotifySessionInfoChange();
        });

    WSRect winRect = GetSessionRect();
    Rect rect = {winRect.posX_, winRect.posY_, static_cast<uint32_t>(winRect.width_),
        static_cast<uint32_t>(winRect.height_)};
    property->SetWindowRect(rect);
    property->SetPersistentId(GetPersistentId());
    property->SetAppIndex(GetSessionInfo().appIndex_);
    property->SetFullScreenStart(GetSessionInfo().fullScreenStart_);
    property->SetSupportedWindowModes(GetSessionInfo().supportedWindowModes);
    property->SetWindowSizeLimits(GetSessionInfo().windowSizeLimits);
    property->SetIsAtomicService(GetSessionInfo().isAtomicService_);
    property->SetRequestedOrientation(GetSessionProperty()->GetRequestedOrientation());
    property->SetDefaultRequestedOrientation(GetSessionProperty()->GetDefaultRequestedOrientation());
    property->SetUserRequestedOrientation(GetSessionProperty()->GetUserRequestedOrientation());
    TLOGI(WmsLogTag::WMS_MAIN, "[id: %{public}d] requestedOrientation: %{public}u,"
        " defaultRequestedOrientation: %{public}u,"
        " userRequestedOrientation: %{public}u", GetPersistentId(),
        static_cast<uint32_t>(GetSessionProperty()->GetRequestedOrientation()),
        static_cast<uint32_t>(GetSessionProperty()->GetDefaultRequestedOrientation()),
        static_cast<uint32_t>(GetSessionProperty()->GetUserRequestedOrientation()));
    property->SetCompatibleModeProperty(GetSessionProperty()->GetCompatibleModeProperty());
    if (property->GetCompatibleModeProperty()) {
        property->SetDragEnabled(!GetSessionProperty()->IsDragResizeDisabled());
    }
    if (property->IsAdaptToEventMapping()) {
        std::vector<Rect> touchHotAreas;
        GetSessionProperty()->GetTouchHotAreas(touchHotAreas);
        property->SetTouchHotAreas(touchHotAreas);
    }
    property->SetIsAppSupportPhoneInPc(GetSessionProperty()->GetIsAppSupportPhoneInPc());
    std::optional<bool> clientDragEnable = GetClientDragEnable();
    if (clientDragEnable.has_value()) {
        property->SetDragEnabled(clientDragEnable.value());
    }
    if (SessionHelper::IsMainWindow(GetWindowType())) {
        property->SetIsPcAppInPad(GetSessionProperty()->GetIsPcAppInPad());
    }
    property->SetSkipSelfWhenShowOnVirtualScreen(GetSessionProperty()->GetSkipSelfWhenShowOnVirtualScreen());
    property->SetSkipEventOnCastPlus(GetSessionProperty()->GetSkipEventOnCastPlus());
    property->SetIsAbilityHook(GetSessionInfo().isAbilityHook_);
    property->SetPcAppInpadCompatibleMode(GetSessionProperty()->GetPcAppInpadCompatibleMode());
    property->SetPcAppInpadSpecificSystemBarInvisible(GetSessionProperty()->GetPcAppInpadSpecificSystemBarInvisible());
    property->SetPcAppInpadOrientationLandscape(GetSessionProperty()->GetPcAppInpadOrientationLandscape());
    property->SetMobileAppInPadLayoutFullScreen(GetSessionProperty()->GetMobileAppInPadLayoutFullScreen());
    const bool isPcMode = system::GetBoolParameter("persist.sceneboard.ispcmode", false);
    const bool isShow = !(isScreenLockedCallback_ && isScreenLockedCallback_() &&
        systemConfig_.freeMultiWindowSupport_ && !isPcMode);
    property->SetIsShowDecorInFreeMultiWindow(isShow);
    SetSessionProperty(property);
    GetSessionProperty()->SetIsNeedUpdateWindowMode(false);
}

void Session::InitSystemSessionDragEnable(const sptr<WindowSessionProperty>& property)
{
    auto defaultDragEnable = false;
    auto isSystemWindow = WindowHelper::IsSystemWindow(property->GetWindowType());
    bool isDialog = WindowHelper::IsDialogWindow(property->GetWindowType());
    bool isSystemCalling = property->GetSystemCalling();
    TLOGI(WmsLogTag::WMS_LAYOUT, "windId: %{public}d, defaultDragEnable: %{public}d, isSystemWindow: %{public}d, "
        "isDialog: %{public}d, isSystemCalling: %{public}d", GetPersistentId(), defaultDragEnable,
        isSystemWindow, isDialog, isSystemCalling);
    if (isSystemWindow && !isDialog && !isSystemCalling) {
        property->SetDragEnabled(defaultDragEnable);
    }
}

WSError Session::Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    int32_t pid, int32_t uid)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "property is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_RECOVER, "id:%{public}d, state:%{public}u, pid:%{public}d, disableDelegator:%{public}d",
        property->GetPersistentId(), static_cast<uint32_t>(property->GetWindowState()), pid,
        property->GetIsAbilityHookOff());
    if (sessionStage == nullptr || eventChannel == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "session stage or eventChannel is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_ = sessionStage;
    NotifyAppHookWindowInfoUpdated();
    SetSurfaceNode(surfaceNode);
    windowEventChannel_ = eventChannel;
    abilityToken_ = token;
    SetSessionPropertyForReconnect(property);
    persistentId_ = property->GetPersistentId();
    SetCallingPid(pid);
    callingUid_ = uid;
    bufferAvailable_ = true;
    auto windowRect = property->GetWindowRect();
    layoutRect_ = { windowRect.posX_, windowRect.posY_,
        static_cast<int32_t>(windowRect.width_), static_cast<int32_t>(windowRect.height_) };
    UpdateSessionState(SessionState::STATE_CONNECT);
    EditSessionInfo().disableDelegator = property->GetIsAbilityHookOff();
    for (const auto& [transitionType, animation] : property->GetTransitionAnimationConfig()) {
        property_->SetTransitionAnimationConfig(transitionType, *animation);
    }
    return WSError::WS_OK;
}

WSError Session::Foreground(sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken)
{
    HandleDialogForeground();
    SessionState state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] state:%{public}u, isTerminating:%{public}d",
        GetPersistentId(), static_cast<uint32_t>(state), isTerminating_);
    if (state != SessionState::STATE_CONNECT && state != SessionState::STATE_BACKGROUND &&
        state != SessionState::STATE_INACTIVE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Foreground state invalid! state:%{public}u", state);
        return WSError::WS_ERROR_INVALID_SESSION;
    }

    UpdateSessionState(SessionState::STATE_FOREGROUND);
    if (!isActive_ || (isActive_ && GetSessionInfo().reuseDelegatorWindow)) {
        SetActive(true);
    }
    isStarting_ = false;
    NotifyForeground();

    isTerminating_ = false;
    PostSpecificSessionLifeCycleTimeoutTask(ATTACH_EVENT_NAME);

    // Window Layout Global Coordinate System
    // When the window enters foreground, notify the client to update GlobalDisplayRect once,
    // since background windows skip this notification to avoid IPC wake-up and power issues.
    NotifyClientToUpdateGlobalDisplayRect(GetGlobalDisplayRect(), SizeChangeReason::UNDEFINED);
    return WSError::WS_OK;
}

void Session::HandleDialogBackground()
{
    const auto& type = GetWindowType();
    if (type < WindowType::APP_MAIN_WINDOW_BASE || type >= WindowType::APP_MAIN_WINDOW_END) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Current session is not main window, id: %{public}d, type: %{public}d",
            GetPersistentId(), type);
        return;
    }

    auto dialogVec = GetDialogVector();
    for (const auto& dialog : dialogVec) {
        if (dialog == nullptr) {
            continue;
        }
        TLOGI(WmsLogTag::WMS_DIALOG, "Background dialog, id: %{public}d, dialogId: %{public}d",
            GetPersistentId(), dialog->GetPersistentId());
        if (!dialog->sessionStage_) {
            TLOGE(WmsLogTag::WMS_DIALOG, "dialog session stage is nullptr");
            return;
        }
        dialog->sessionStage_->NotifyDialogStateChange(false);
    }
}

void Session::HandleDialogForeground()
{
    const auto& type = GetWindowType();
    if (type < WindowType::APP_MAIN_WINDOW_BASE || type >= WindowType::APP_MAIN_WINDOW_END) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Current session is not main window, id: %{public}d, type: %{public}d",
            GetPersistentId(), type);
        return;
    }

    auto dialogVec = GetDialogVector();
    for (const auto& dialog : dialogVec) {
        if (dialog == nullptr) {
            continue;
        }
        TLOGI(WmsLogTag::WMS_DIALOG, "Foreground dialog, id: %{public}d, dialogId: %{public}d",
            GetPersistentId(), dialog->GetPersistentId());
        if (!dialog->sessionStage_) {
            TLOGE(WmsLogTag::WMS_DIALOG, "dialog session stage is nullptr");
            return;
        }
        dialog->sessionStage_->NotifyDialogStateChange(true);
    }
}

WSError Session::Background(bool isFromClient, const std::string& identityToken)
{
    HandleDialogBackground();
    SessionState state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] Background session, state: %{public}" PRIu32, GetPersistentId(),
        static_cast<uint32_t>(state));
    if (state == SessionState::STATE_ACTIVE && GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        UpdateSessionState(SessionState::STATE_INACTIVE);
        state = SessionState::STATE_INACTIVE;
        isActive_ = false;
    }
    isStarting_ = false;
    isStartingBeforeVisible_ = false;
    if (state != SessionState::STATE_INACTIVE) {
        TLOGW(WmsLogTag::WMS_LIFE, "[id: %{public}d] Background state invalid! state: %{public}u",
            GetPersistentId(), state);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    UpdateSessionState(SessionState::STATE_BACKGROUND);
    lastSnapshotScreen_ = WSSnapshotHelper::GetInstance()->GetScreenStatus();
    SetIsPendingToBackgroundState(false);
    NotifyBackground();
    PostSpecificSessionLifeCycleTimeoutTask(DETACH_EVENT_NAME);
    return WSError::WS_OK;
}

void Session::ResetSessionConnectState()
{
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] ResetSessionState, state: %{public}u",
        GetPersistentId(), GetSessionState());
    SetSessionState(SessionState::STATE_DISCONNECT);
    SetCallingPid(-1);
}

void Session::ResetIsActive()
{
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] isActive: %{public}u",
        GetPersistentId(), IsActive());
    isActive_ = false;
}

WSError Session::Disconnect(bool isFromClient, const std::string& identityToken)
{
    auto state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] Disconnect session, state: %{public}u", GetPersistentId(), state);
    isActive_ = false;
    isStarting_ = false;
    isStartingBeforeVisible_ = false;
    bufferAvailable_ = false;
    isNeedSyncSessionRect_ = true;
    if (mainHandler_) {
        std::shared_ptr<RSSurfaceNode> surfaceNode;
        std::shared_ptr<RSSurfaceNode> shadowSurfaceNode;
        {
            std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
            surfaceNode_.swap(surfaceNode);
            shadowSurfaceNode.swap(shadowSurfaceNode_);
        }
        mainHandler_->PostTask([surfaceNode = std::move(surfaceNode),
                                shadowSurfaceNode = std::move(shadowSurfaceNode)]() mutable {
            surfaceNode.reset();
            shadowSurfaceNode.reset();
        });
    }
    UpdateSessionState(SessionState::STATE_BACKGROUND);
    UpdateSessionState(SessionState::STATE_DISCONNECT);
    lastSnapshotScreen_ = WSSnapshotHelper::GetInstance()->GetScreenStatus();
    NotifyDisconnect();
    if (visibilityChangedDetectFunc_) {
        visibilityChangedDetectFunc_(GetCallingPid(), isVisible_, false);
    }
    // If session is disconnect, clear it's outline if need.
    OutlineStyleParams defaultParams;
    UpdateSessionOutline(false, defaultParams);
    return WSError::WS_OK;
}

WSError Session::Show(sptr<WindowSessionProperty> property)
{
    TLOGD(WmsLogTag::WMS_LIFE, "Show session, id: %{public}d", GetPersistentId());
    return WSError::WS_OK;
}

WSError Session::Hide()
{
    TLOGD(WmsLogTag::WMS_LIFE, "Hide session, id: %{public}d", GetPersistentId());
    return WSError::WS_OK;
}

WSError Session::DrawingCompleted()
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d", GetPersistentId());
    if (!SessionPermission::IsSystemAppCall()) {
        TLOGE(WmsLogTag::WMS_LIFE, "permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnDrawingCompleted();
        }
    }
    return WSError::WS_OK;
}

WSError Session::RemoveStartingWindow()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnAppRemoveStartingWindow();
        }
    }
    return WSError::WS_OK;
}

WSError Session::SetActive(bool active)
{
    SessionState state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "new active:%{public}d, id:%{public}d, state:%{public}u",
        active, GetPersistentId(), static_cast<uint32_t>(state));
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_LIFE, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (active == isActive_ && !GetSessionInfo().reuseDelegatorWindow) {
        TLOGD(WmsLogTag::WMS_LIFE, "Session active do not change: [%{public}d]", active);
        return WSError::WS_DO_NOTHING;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LIFE, "session stage is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (active && GetSessionState() == SessionState::STATE_FOREGROUND) {
        sessionStage_->SetActive(true);
        UpdateSessionState(SessionState::STATE_ACTIVE);
        isActive_ = active;
    }
    if (!active && GetSessionState() == SessionState::STATE_ACTIVE) {
        sessionStage_->SetActive(false);
        UpdateSessionState(SessionState::STATE_INACTIVE);
        isActive_ = active;
    }
    return WSError::WS_OK;
}

void Session::ProcessClickModalWindowOutside(int32_t posX, int32_t posY)
{
    if (clickModalWindowOutsideFunc_ && !GetSessionRect().IsInRegion(posX, posY)) {
        clickModalWindowOutsideFunc_();
    }
}

void Session::SetClickModalWindowOutsideListener(NotifyClickModalWindowOutsideFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session or func is null", where);
            return;
        }
        session->clickModalWindowOutsideFunc_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_DIALOG, "%{public}s id: %{public}d", where, session->GetPersistentId());
    }, __func__);
}

void Session::SetClientDragEnable(bool dragEnable)
{
    clientDragEnable_ = dragEnable;
}

std::optional<bool> Session::GetClientDragEnable() const
{
    return clientDragEnable_;
}

void Session::SetDragActivated(bool dragActivated)
{
    dragActivated_ = dragActivated;
}

bool Session::IsDragAccessible() const
{
    bool isDragEnabled = GetSessionProperty()->GetDragEnabled();
    TLOGD(WmsLogTag::WMS_LAYOUT, "PersistentId: %{public}d, dragEnabled: %{public}d, dragActivate: %{public}d",
        GetPersistentId(), isDragEnabled, dragActivated_);
    return isDragEnabled && dragActivated_;
}

bool Session::IsScreenLockWindow() const
{
    return isScreenLockWindow_;
}

void Session::NotifyForegroundInteractiveStatus(bool interactive)
{
    SetForegroundInteractiveStatus(interactive);
}

void Session::SetForegroundInteractiveStatus(bool interactive)
{
    if (interactive != GetForegroundInteractiveStatus()) {
        TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, %{public}d", GetPersistentId(),
            static_cast<int>(interactive));
    }
    foregroundInteractiveStatus_.store(interactive);
    if (Session::IsScbCoreEnabled()) {
        return;
    }
    NotifySessionInfoChange();
}

bool Session::GetForegroundInteractiveStatus() const
{
    return foregroundInteractiveStatus_.load();
}

bool Session::GetIsPendingToBackgroundState() const
{
    return isPendingToBackgroundState_.load();
}

void Session::SetIsPendingToBackgroundState(bool isPendingToBackgroundState)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d isPendingToBackgroundState:%{public}d",
        GetPersistentId(), isPendingToBackgroundState);
    return isPendingToBackgroundState_.store(isPendingToBackgroundState);
}

bool Session::IsActivatedAfterScreenLocked() const
{
    return isActivatedAfterScreenLocked_.load();
}

void Session::SetIsActivatedAfterScreenLocked(bool isActivatedAfterScreenLocked)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, isActivatedAfterScreenLocked:%{public}d",
        GetPersistentId(), isActivatedAfterScreenLocked);
    isActivatedAfterScreenLocked_.store(isActivatedAfterScreenLocked);
}

void Session::SetAttachState(bool isAttach, WindowMode windowMode)
{
    isAttach_ = isAttach;
    if (handler_ && IsNeedReportTimeout()) {
        handler_->RemoveTask(isAttach_ ? ATTACH_EVENT_NAME : DETACH_EVENT_NAME);
    }
    PostTask([weakThis = wptr(this), isAttach]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGND(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        auto mainSession = session->GetMainSession();
        if ((!mainSession || !mainSession->GetShowRecent()) &&
            session->sessionStage_ && WindowHelper::IsNeedWaitAttachStateWindow(session->GetWindowType())) {
            TLOGNI(WmsLogTag::WMS_LIFE, "NotifyWindowAttachStateChange, persistentId:%{public}d",
                session->GetPersistentId());
            session->sessionStage_->NotifyWindowAttachStateChange(isAttach);
        }
        TLOGND(WmsLogTag::WMS_LIFE, "isAttach:%{public}d persistentId:%{public}d", isAttach,
            session->GetPersistentId());
        if (!isAttach && session->detachCallback_ != nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "Session detach, persistentId:%{public}d", session->GetPersistentId());
            session->detachCallback_->OnPatternDetach(session->GetPersistentId());
            session->detachCallback_ = nullptr;
        }
    }, "SetAttachState");
    CreateDetectStateTask(isAttach, windowMode);
}

void Session::CreateDetectStateTask(bool isAttach, WindowMode windowMode)
{
    if (!IsSupportDetectWindow(isAttach)) {
        return;
    }
    if (showRecent_) {
        return;
    }
    if (!ShouldCreateDetectTask(isAttach, windowMode)) {
        RemoveWindowDetectTask();
        DetectTaskInfo detectTaskInfo;
        SetDetectTaskInfo(detectTaskInfo);
        return;
    }
    CreateWindowStateDetectTask(isAttach, windowMode);
}

void Session::RegisterDetachCallback(const sptr<IPatternDetachCallback>& callback)
{
    detachCallback_ = callback;
    if (!isAttach_ && detachCallback_ != nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "Session detach before register, persistentId:%{public}d", GetPersistentId());
        detachCallback_->OnPatternDetach(GetPersistentId());
        detachCallback_ = nullptr;
    }
}

void Session::SetChangeSessionVisibilityWithStatusBarEventListener(
    NotifyChangeSessionVisibilityWithStatusBarFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->changeSessionVisibilityWithStatusBarFunc_ = std::move(func);
    }, __func__);
}

void Session::SetPendingSessionActivationEventListener(NotifyPendingSessionActivationFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->pendingSessionActivationFunc_ = std::move(func);
    }, where);
}

void Session::SetBatchPendingSessionsActivationEventListener(NotifyBatchPendingSessionsActivationFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->batchPendingSessionsActivationFunc_ = std::move(func);
    }, where);
}

void Session::SetBackPressedListenser(NotifyBackPressedFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->backPressedFunc_ = std::move(func);
    }, where);
}

void Session::SetTerminateSessionListener(NotifyTerminateSessionFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->terminateSessionFunc_ = std::move(func);
    }, where);
}

void Session::SetRestartAppListener(NotifyRestartAppFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->restartAppFunc_ = std::move(func);
    }, where);
}

void Session::RemoveLifeCycleTask(const LifeCycleTaskType& taskType)
{
    sptr<SessionLifeCycleTask> frontLifeCycleTask = nullptr;
    {
        std::lock_guard<std::mutex> lock(lifeCycleTaskQueueMutex_);
        if (lifeCycleTaskQueue_.empty()) {
            return;
        }
        sptr<SessionLifeCycleTask> currLifeCycleTask = lifeCycleTaskQueue_.front();
        if (currLifeCycleTask->type != taskType) {
            TLOGW(WmsLogTag::WMS_LIFE,
                "not match, current running taskName=%{public}s, PersistentId=%{public}d",
                currLifeCycleTask->name.c_str(), persistentId_);
            return;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "Removed lifeCyleTask %{public}s. PersistentId=%{public}d",
            currLifeCycleTask->name.c_str(), persistentId_);
        lifeCycleTaskQueue_.pop_front();
        if (lifeCycleTaskQueue_.empty()) {
            return;
        }
        frontLifeCycleTask = lifeCycleTaskQueue_.front();
        if (!SetLifeCycleTaskRunning(frontLifeCycleTask)) {
            return;
        }
    }
    PostTask(std::move(frontLifeCycleTask->task), frontLifeCycleTask->name);
}

void Session::ClearLifeCycleTask()
{
    TLOGI(WmsLogTag::WMS_LIFE, "Clear LifeCycleTask, PersistentId=%{public}d", persistentId_);
    std::lock_guard<std::mutex> lock(lifeCycleTaskQueueMutex_);
    lifeCycleTaskQueue_.clear();
}

void Session::PostLifeCycleTask(Task&& task, const std::string& name, const LifeCycleTaskType& taskType)
{
    sptr<SessionLifeCycleTask> frontLifeCycleTask = nullptr;
    {
        std::lock_guard<std::mutex> lock(lifeCycleTaskQueueMutex_);
        if (!lifeCycleTaskQueue_.empty()) {
        // remove current running task if expired
            sptr<SessionLifeCycleTask> currLifeCycleTask = lifeCycleTaskQueue_.front();
            std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
            bool isCurrentTaskExpired =
                std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - currLifeCycleTask->startTime)
                    .count() > LIFE_CYCLE_TASK_EXPIRED_TIME_LIMIT;
            if (isCurrentTaskExpired) {
                TLOGE(WmsLogTag::WMS_LIFE, "Remove expired LifeCycleTask %{public}s. PersistentId=%{public}d",
                    currLifeCycleTask->name.c_str(), persistentId_);
                lifeCycleTaskQueue_.pop_front();
            }
        }

        if (lifeCycleTaskQueue_.size() == MAX_LIFE_CYCLE_TASK_IN_QUEUE) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to add task %{public}s to life cycle queue", name.c_str());
            return;
        }
        sptr<SessionLifeCycleTask> lifeCycleTask =
            sptr<SessionLifeCycleTask>::MakeSptr(std::move(task), name, taskType);
        lifeCycleTaskQueue_.push_back(lifeCycleTask);
        TLOGI(WmsLogTag::WMS_LIFE, "Add task %{public}s to life cycle queue, PersistentId=%{public}d",
            name.c_str(), persistentId_);
        frontLifeCycleTask = lifeCycleTaskQueue_.front();
        if (!SetLifeCycleTaskRunning(frontLifeCycleTask)) {
            return;
        }
    }
    PostTask(std::move(frontLifeCycleTask->task), frontLifeCycleTask->name);
}

bool Session::SetLifeCycleTaskRunning(const sptr<SessionLifeCycleTask>& lifeCycleTask)
{
    if (lifeCycleTask == nullptr || lifeCycleTask->running) {
        TLOGW(WmsLogTag::WMS_LIFE, "LifeCycleTask is running or null. PersistentId: %{public}d", persistentId_);
        return false;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Execute LifeCycleTask %{public}s. PersistentId: %{public}d",
        lifeCycleTask->name.c_str(), persistentId_);
    lifeCycleTask->running = true;
    lifeCycleTask->startTime = std::chrono::steady_clock::now();
    return true;
}

WSError Session::TerminateSessionNew(
    const sptr<AAFwk::SessionInfo> abilitySessionInfo, bool needStartCaller, bool isFromBroker)
{
    if (abilitySessionInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    auto task = [weakThis = wptr(this), abilitySessionInfo, needStartCaller, isFromBroker]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "session is null.");
            return;
        }
        session->isTerminating_ = true;
        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.persistentId_ = static_cast<int32_t>(abilitySessionInfo->persistentId);
        {
            std::lock_guard<std::recursive_mutex> lock(session->sessionInfoMutex_);
            session->sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
            session->sessionInfo_.resultCode = abilitySessionInfo->resultCode;
        }
        if (session->terminateSessionFuncNew_) {
            session->terminateSessionFuncNew_(info, needStartCaller, isFromBroker, false);
        }
        TLOGNI(WmsLogTag::WMS_LIFE,
            "TerminateSessionNew, id: %{public}d, needStartCaller: %{public}d, isFromBroker: %{public}d",
            session->GetPersistentId(), needStartCaller, isFromBroker);
    };
    PostLifeCycleTask(task, "TerminateSessionNew", LifeCycleTaskType::STOP);
    return WSError::WS_OK;
}

void Session::SetTerminateSessionListenerNew(NotifyTerminateSessionFuncNew&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->terminateSessionFuncNew_ = std::move(func);
    }, where);
}

WSError Session::TerminateSessionTotal(const sptr<AAFwk::SessionInfo> abilitySessionInfo, TerminateType terminateType)
{
    if (abilitySessionInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (isTerminating_) {
        TLOGE(WmsLogTag::WMS_LIFE, "is terminating, return!");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    isTerminating_ = true;
    SessionInfo info;
    info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
    info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
    info.callerToken_ = abilitySessionInfo->callerToken;
    info.persistentId_ = static_cast<int32_t>(abilitySessionInfo->persistentId);
    {
        std::lock_guard<std::recursive_mutex> lock(sessionInfoMutex_);
        sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
        sessionInfo_.resultCode = abilitySessionInfo->resultCode;
    }
    if (terminateSessionFuncTotal_) {
        terminateSessionFuncTotal_(info, terminateType);
    }
    return WSError::WS_OK;
}

void Session::SetTerminateSessionListenerTotal(NotifyTerminateSessionFuncTotal&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->terminateSessionFuncTotal_ = std::move(func);
    }, where);
}

WSError Session::SetSessionLabel(const std::string& label)
{
    WLOGFI("run Session::SetSessionLabel");
    if (updateSessionLabelFunc_) {
        updateSessionLabelFunc_(label);
    }
    label_ = label;
    return WSError::WS_OK;
}

void Session::UpdateSessionLabel(const std::string& label)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), label, where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        if (session->label_ == "") {
            session->label_ = label;
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s set label success label:%{public}s", where, label.c_str());
        }
        }, where);
}

std::string Session::GetSessionLabel() const
{
    return label_;
}

void Session::SetUpdateSessionLabelListener(const NofitySessionLabelUpdatedFunc& func)
{
    updateSessionLabelFunc_ = func;
}

WSError Session::SetSessionIcon(const std::shared_ptr<Media::PixelMap>& icon)
{
    WLOGFD("run Session::SetSessionIcon, id: %{public}d", GetPersistentId());
    if (scenePersistence_ == nullptr) {
        WLOGFE("scenePersistence_ is nullptr.");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    scenePersistence_->SaveUpdatedIcon(icon);
    std::string updatedIconPath = scenePersistence_->GetUpdatedIconPath();
    if (updateSessionIconFunc_) {
        updateSessionIconFunc_(updatedIconPath);
    }
    return WSError::WS_OK;
}

void Session::SetUpdateSessionIconListener(const NofitySessionIconUpdatedFunc& func)
{
    updateSessionIconFunc_ = func;
}

WSError Session::Clear(bool needStartCaller, bool isForceClean)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, needStartCaller:%{public}u, isForceClean:%{public}u",
        GetPersistentId(), needStartCaller, isForceClean);
    auto task = [weakThis = wptr(this), needStartCaller, isForceClean]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->isTerminating_ = true;
        if (session->terminateSessionFuncNew_) {
            session->terminateSessionFuncNew_(session->GetSessionInfo(), needStartCaller, false, isForceClean);
        }
    };
    PostLifeCycleTask(task, "Clear", LifeCycleTaskType::STOP);
    return WSError::WS_OK;
}

void Session::SetSessionExceptionListener(NotifySessionExceptionFunc&& func, bool fromJsScene)
{
    PostTask([weakThis = wptr(this), where = __func__, func = std::move(func), fromJsScene] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        if (fromJsScene) {
            session->jsSceneSessionExceptionFunc_ = std::move(func);
        } else {
            session->sessionExceptionFunc_ = std::move(func);
        }
    }, __func__);
}

void Session::SetSessionSnapshotListener(const NotifySessionSnapshotFunc& func)
{
    if (func == nullptr) {
        WLOGFE("func is nullptr");
        return;
    }
    notifySessionSnapshotFunc_ = func;
}

void Session::SetPendingSessionToForegroundListener(NotifyPendingSessionToForegroundFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->pendingSessionToForegroundFunc_ = std::move(func);
    }, __func__);
}

WSError Session::PendingSessionToForeground()
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d", GetPersistentId());
    if (pendingSessionActivationFunc_) {
        SessionInfo info = GetSessionInfo();
        pendingSessionActivationFunc_(info);
    }
    return WSError::WS_OK;
}

void Session::SetPendingSessionToBackgroundListener(NotifyPendingSessionToBackgroundFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->pendingSessionToBackgroundFunc_ = std::move(func);
    }, __func__);
}

WSError Session::PendingSessionToBackground(const BackgroundParams& params)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d, shouldBackToCaller: %{public}d",
        GetPersistentId(), params.shouldBackToCaller);
    if (pendingSessionToBackgroundFunc_) {
        pendingSessionToBackgroundFunc_(GetSessionInfo(), params);
    }
    return WSError::WS_OK;
}

void Session::SetPendingSessionToBackgroundForDelegatorListener(
    NotifyPendingSessionToBackgroundForDelegatorFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->pendingSessionToBackgroundForDelegatorFunc_ = std::move(func);
    }, __func__);
}

WSError Session::PendingSessionToBackgroundForDelegator(bool shouldBackToCaller)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d, shouldBackToCaller: %{public}d",
        GetPersistentId(), shouldBackToCaller);
    if (pendingSessionToBackgroundForDelegatorFunc_) {
        pendingSessionToBackgroundForDelegatorFunc_(GetSessionInfo(), shouldBackToCaller);
    }
    return WSError::WS_OK;
}

void Session::SetRaiseToAppTopForPointDownFunc(const NotifyRaiseToTopForPointDownFunc& func)
{
    raiseToTopForPointDownFunc_ = func;
}

void Session::NotifyScreenshot()
{
    if (!sessionStage_) {
        return;
    }
    sessionStage_->NotifyScreenshot();
}

WSError Session::NotifyScreenshotAppEvent(ScreenshotEventType type)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, event: %{public}d", GetPersistentId(), type);
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyScreenshotAppEvent(type);
}

WSError Session::NotifyCloseExistPipWindow()
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyCloseExistPipWindow();
}

WSError Session::NotifyDestroy()
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyDestroy();
}

WSError Session::NotifyAppForceLandscapeConfigUpdated()
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyAppForceLandscapeConfigUpdated();
}

WSError Session::NotifyAppHookWindowInfoUpdated()
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyAppHookWindowInfoUpdated();
}

void Session::SetParentSession(const sptr<Session>& session)
{
    if (session == nullptr) {
        WLOGFW("Session is nullptr");
        return;
    }
    {
        std::unique_lock<std::shared_mutex> lock(parentSessionMutex_);
        parentSession_ = session;
    }
    TLOGD(WmsLogTag::WMS_SUB, "[WMSDialog][WMSSub]Set parent success, parentId: %{public}d, id: %{public}d",
        session->GetPersistentId(), GetPersistentId());
}

sptr<Session> Session::GetParentSession() const
{
    std::shared_lock<std::shared_mutex> lock(parentSessionMutex_);
    return parentSession_;
}

sptr<Session> Session::GetMainSession() const
{
    if (SessionHelper::IsMainWindow(GetWindowType())) {
        return const_cast<Session*>(this);
    } else if (parentSession_) {
        return parentSession_->GetMainSession();
    } else {
        return nullptr;
    }
}

sptr<Session> Session::GetMainOrFloatSession() const
{
    auto windowType = GetWindowType();
    if (SessionHelper::IsMainWindow(windowType) || windowType == WindowType::WINDOW_TYPE_FLOAT) {
        return const_cast<Session*>(this);
    } else if (parentSession_) {
        return parentSession_->GetMainOrFloatSession();
    } else {
        return nullptr;
    }
}

bool Session::IsAncestorsSession(int32_t ancestorsId) const
{
    if (GetSessionProperty()->GetParentPersistentId() == ancestorsId) {
        return true;
    }
    auto parentSession = GetParentSession();
    if (parentSession != nullptr) {
        return parentSession->IsAncestorsSession(ancestorsId);
    }
    return false;
}

void Session::BindDialogToParentSession(const sptr<Session>& session)
{
    std::unique_lock<std::shared_mutex> lock(dialogVecMutex_);
    auto iter = std::find(dialogVec_.begin(), dialogVec_.end(), session);
    if (iter != dialogVec_.end()) {
        TLOGW(WmsLogTag::WMS_DIALOG, "Dialog is existed in parentVec, id: %{public}d, parentId: %{public}d",
            session->GetPersistentId(), GetPersistentId());
        return;
    }
    dialogVec_.push_back(session);
    TLOGD(WmsLogTag::WMS_DIALOG, "Bind dialog success, id: %{public}d, parentId: %{public}d",
        session->GetPersistentId(), GetPersistentId());
}

void Session::RemoveDialogToParentSession(const sptr<Session>& session)
{
    std::unique_lock<std::shared_mutex> lock(dialogVecMutex_);
    auto iter = std::find(dialogVec_.begin(), dialogVec_.end(), session);
    if (iter != dialogVec_.end()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Remove dialog success, id: %{public}d, parentId: %{public}d",
            session->GetPersistentId(), GetPersistentId());
        dialogVec_.erase(iter);
    }
    TLOGW(WmsLogTag::WMS_DIALOG, "Remove dialog failed, id: %{public}d, parentId: %{public}d",
        session->GetPersistentId(), GetPersistentId());
}

std::vector<sptr<Session>> Session::GetDialogVector() const
{
    std::shared_lock<std::shared_mutex> lock(dialogVecMutex_);
    return dialogVec_;
}

void Session::ClearDialogVector()
{
    std::unique_lock<std::shared_mutex> lock(dialogVecMutex_);
    dialogVec_.clear();
    TLOGD(WmsLogTag::WMS_DIALOG, "parentId: %{public}d", GetPersistentId());
    return;
}

bool Session::CheckDialogOnForeground()
{
    auto dialogVec = GetDialogVector();
    if (dialogVec.empty()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Dialog is empty, id: %{public}d", GetPersistentId());
        return false;
    }
    for (auto iter = dialogVec.rbegin(); iter != dialogVec.rend(); iter++) {
        auto dialogSession = *iter;
        if (dialogSession && (dialogSession->GetSessionState() == SessionState::STATE_ACTIVE ||
            dialogSession->GetSessionState() == SessionState::STATE_FOREGROUND)) {
            TLOGD(WmsLogTag::WMS_DIALOG, "Notify touch dialog window, id: %{public}d", GetPersistentId());
            return true;
        }
    }
    return false;
}

bool Session::CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    return true;
}

bool Session::IsTopDialog() const
{
    int32_t currentPersistentId = GetPersistentId();
    auto parentSession = GetParentSession();
    if (parentSession == nullptr) {
        TLOGW(WmsLogTag::WMS_DIALOG, "Dialog's Parent is NULL. id: %{public}d", currentPersistentId);
        return false;
    }
    auto parentDialogVec = parentSession->GetDialogVector();
    if (parentDialogVec.size() <= 1) {
        return true;
    }
    for (auto iter = parentDialogVec.rbegin(); iter != parentDialogVec.rend(); iter++) {
        auto dialogSession = *iter;
        if (dialogSession && (dialogSession->GetSessionState() == SessionState::STATE_ACTIVE ||
            dialogSession->GetSessionState() == SessionState::STATE_FOREGROUND)) {
            WLOGFI("Dialog id: %{public}d, current dialog id: %{public}d", dialogSession->GetPersistentId(),
                currentPersistentId);
            return dialogSession->GetPersistentId() == currentPersistentId;
        }
    }
    return false;
}

const char* Session::DumpPointerWindowArea(MMI::WindowArea area) const
{
    const std::map<MMI::WindowArea, const char*> areaMap = {
        { MMI::WindowArea::FOCUS_ON_INNER, "FOCUS_ON_INNER" },
        { MMI::WindowArea::FOCUS_ON_TOP, "FOCUS_ON_TOP" },
        { MMI::WindowArea::FOCUS_ON_BOTTOM, "FOCUS_ON_BOTTOM" },
        { MMI::WindowArea::FOCUS_ON_LEFT, "FOCUS_ON_LEFT" },
        { MMI::WindowArea::FOCUS_ON_RIGHT, "FOCUS_ON_RIGHT" },
        { MMI::WindowArea::FOCUS_ON_BOTTOM_LEFT, "FOCUS_ON_BOTTOM_LEFT" },
        { MMI::WindowArea::FOCUS_ON_BOTTOM_RIGHT, "FOCUS_ON_BOTTOM_RIGHT" },
        { MMI::WindowArea::FOCUS_ON_TOP_LEFT, "FOCUS_ON_TOP_LEFT" },
        { MMI::WindowArea::FOCUS_ON_TOP_RIGHT, "FOCUS_ON_TOP_RIGHT" },
        { MMI::WindowArea::EXIT, "EXIT" }
    };
    auto iter = areaMap.find(area);
    if (iter == areaMap.end()) {
        return "UNKNOWN";
    }
    return iter->second;
}

WSError Session::RaiseToAppTopForPointDown()
{
    if (raiseToTopForPointDownFunc_) {
        raiseToTopForPointDownFunc_();
        WLOGFD("RaiseToAppTopForPointDown, id: %{public}d, type: %{public}d", GetPersistentId(), GetWindowType());
    }
    return WSError::WS_OK;
}

void Session::PresentFocusIfPointDown()
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d,type: %{public}d", GetPersistentId(), GetWindowType());
    if (!isFocused_ && GetFocusable()) {
        FocusChangeReason reason = FocusChangeReason::CLICK;
        NotifyRequestFocusStatusNotifyManager(true, false, reason);
    }
    NotifyClickIfNeed();
}

void Session::HandlePointDownDialog()
{
    auto dialogVec = GetDialogVector();
    sptr<Session> lastValidDialog = nullptr;
    for (auto dialog : dialogVec) {
        if (dialog && (dialog->GetSessionState() == SessionState::STATE_FOREGROUND ||
            dialog->GetSessionState() == SessionState::STATE_ACTIVE)) {
            dialog->RaiseToAppTopForPointDown();
            lastValidDialog = dialog;
            TLOGD(WmsLogTag::WMS_DIALOG, "Point main window, raise to top and dialog need focus, "
                "id: %{public}d, dialogId: %{public}d", GetPersistentId(), dialog->GetPersistentId());
        }
    }
    if (lastValidDialog != nullptr) {
        lastValidDialog->PresentFocusIfPointDown();
    }
}

WSError Session::HandleSubWindowClick(int32_t action, int32_t sourceType, bool isExecuteDelayRaise)
{
    auto parentSession = GetParentSession();
    if (parentSession && parentSession->CheckDialogOnForeground()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Its main window has dialog on foreground, id: %{public}d", GetPersistentId());
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    const auto& property = GetSessionProperty();
    bool raiseEnabled = property->GetRaiseEnabled();
    bool isHoverDown = action == MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER &&
        sourceType ==  MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    bool isPointDown = action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN || isHoverDown;
    bool isPointMove = action == MMI::PointerEvent::POINTER_ACTION_MOVE;
    if (isExecuteDelayRaise) {
        if (raiseEnabled && action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
            RaiseToAppTopForPointDown();
        }
        if (!raiseEnabled && parentSession && !isPointMove) {
            parentSession->NotifyClick(!IsScbCoreEnabled());
        }
        return WSError::WS_OK;
    }
    bool isModal = WindowHelper::IsModalWindow(property->GetWindowFlags());
    TLOGD(WmsLogTag::WMS_EVENT,
          "id: %{public}d, raiseEnabled: %{public}d, isPointDown: %{public}d, isModal: %{public}d",
          GetPersistentId(), raiseEnabled, isPointDown, isModal);
    if (raiseEnabled && isPointDown) {
        if (!isModal) {
            RaiseToAppTopForPointDown();
            return WSError::WS_OK;
        }
        if (auto mainSession = GetMainSession()) {
            mainSession->NotifyClick(false);
        }
    } else if (parentSession && isPointDown) {
        // sub window is forbidden to raise to top after click, but its parent should raise
        parentSession->NotifyClick(!IsScbCoreEnabled());
    }
    return WSError::WS_OK;
}

WSError Session::HandlePointerEventForFocus(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool isExecuteDelayRaise)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "PointerEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "eventId:%{public}d, action:%{public}s, persistentId:%{public}d ",
        pointerEvent->GetId(), pointerEvent->DumpPointerAction(), persistentId_);
    auto pointerAction = pointerEvent->GetPointerAction();
    auto sourceType = pointerEvent->GetSourceType();
    bool isHoverDown = pointerAction == MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER &&
        sourceType ==  MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    bool isPointDown = (pointerAction == MMI::PointerEvent::POINTER_ACTION_DOWN) ||
        (pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) || isHoverDown;
    if (GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        if (CheckDialogOnForeground() && isPointDown) {
            HandlePointDownDialog();
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
    } else if (GetWindowType() == WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
        WSError ret = HandleSubWindowClick(pointerAction, sourceType, isExecuteDelayRaise);
        if (ret != WSError::WS_OK) {
            return ret;
        }
    } else if (GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
        auto parentSession = GetParentSession();
        if (parentSession && parentSession->CheckDialogOnForeground() && isPointDown) {
            parentSession->HandlePointDownDialog();
            if (!IsTopDialog()) {
                TLOGI(WmsLogTag::WMS_DIALOG, "There is at least one active dialog upon this dialog, id: %{public}d",
                    GetPersistentId());
                return WSError::WS_ERROR_INVALID_PERMISSION;
            }
        }
    }
    if (!isExecuteDelayRaise) {
        PresentFocusIfNeed(pointerAction, sourceType);
    } else if (pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
        if (!isFocused_ && GetFocusable()) {
            NotifyRequestFocusStatusNotifyManager(true, false, FocusChangeReason::CLICK);
        }
        NotifyClickIfNeed();
    }
    return WSError::WS_OK;
}

bool Session::HasParentSessionWithToken(const sptr<IRemoteObject>& token)
{
    auto parentSession = GetParentSession();
    if (parentSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "parent session is nullptr: %{public}d", GetPersistentId());
        return false;
    }
    if (parentSession->GetAbilityToken() == token) {
        return true;
    }
    return parentSession->HasParentSessionWithToken(token);
}

WSError Session::TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool needNotifyClient, bool isExecuteDelayRaise)
{
    TLOGD(WmsLogTag::WMS_EVENT, "id: %{public}d", GetPersistentId());
    if (!IsSystemSession() && !IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "PointerEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    WSError focusRet = HandlePointerEventForFocus(pointerEvent, isExecuteDelayRaise);
    if (focusRet != WSError::WS_OK) {
        return focusRet;
    }
    if (!windowEventChannel_) {
        if (!IsSystemSession()) {
            TLOGE(WmsLogTag::WMS_EVENT, "windowEventChannel_ is null");
        }
        return WSError::WS_ERROR_NULLPTR;
    }

    if (needNotifyClient) {
        WSError ret = windowEventChannel_->TransferPointerEvent(pointerEvent);
        if (ret != WSError::WS_OK) {
            WLOGFE("InputTracking id:%{public}d, TransferPointer failed, ret:%{public}d ",
                pointerEvent->GetId(), ret);
        }
        return ret;
    } else {
        pointerEvent->MarkProcessed();
    }
    auto pointerAction = pointerEvent->GetPointerAction();
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_MOVE ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) {
        TLOGD(WmsLogTag::WMS_EVENT, "eventId:%{public}d, action:%{public}s, persistentId:%{public}d, "
            "bundleName:%{public}s, pid:%{public}d", pointerEvent->GetId(), pointerEvent->DumpPointerAction(),
            persistentId_, callingBundleName_.c_str(), callingPid_);
    } else {
        TLOGD(WmsLogTag::WMS_EVENT, "eventId:%{public}d, action:%{public}s, "
            "persistentId:%{public}d, bundleName:%{public}s, pid:%{public}d",
            pointerEvent->GetId(), pointerEvent->DumpPointerAction(),
            persistentId_, callingBundleName_.c_str(), callingPid_);
    }
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_IN_WINDOW ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW) {
        TLOGD(WmsLogTag::WMS_EVENT, "Action:%{public}s, eventId:%{public}d, report without timer",
            pointerEvent->DumpPointerAction(), pointerEvent->GetId());
    }
    return WSError::WS_OK;
}

WSError Session::TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    WLOGFD("Session TransferKeyEvent eventId:%{public}d persistentId:%{public}d bundleName:%{public}s pid:%{public}d",
        keyEvent->GetId(), persistentId_, callingBundleName_.c_str(), callingPid_);
    if (!windowEventChannel_) {
        WLOGFE("windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    WLOGD("TransferKeyEvent, id: %{public}d", persistentId_);
    WSError ret = windowEventChannel_->TransferKeyEvent(keyEvent);
    if (ret != WSError::WS_OK) {
        WLOGFE("TransferKeyEvent failed, ret:%{public}d", ret);
        return ret;
    }
    return WSError::WS_OK;
}

WSError Session::TransferBackPressedEventForConsumed(bool& isConsumed)
{
    if (!windowEventChannel_) {
        WLOGFE("windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferBackpressedEventForConsumed(isConsumed);
}

WSError Session::TransferKeyEventForConsumed(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
    bool isPreImeEvent)
{
    if (!windowEventChannel_) {
        WLOGFE("windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (keyEvent == nullptr) {
        WLOGFE("KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, isPreImeEvent);
}

WSError Session::TransferFocusActiveEvent(bool isFocusActive)
{
    if (!windowEventChannel_) {
        WLOGFE("windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferFocusActiveEvent(isFocusActive);
}

WSError Session::TransferFocusStateEvent(bool focusState)
{
    if (!windowEventChannel_) {
        if (!IsSystemSession()) {
            WLOGFW("windowEventChannel_ is null");
        }
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferFocusState(focusState);
}

std::shared_ptr<Media::PixelMap> Session::Snapshot(bool runInFfrt, float scaleParam, bool useCurWindow) const
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Snapshot[%d][%s]", persistentId_, sessionInfo_.bundleName_.c_str());
    auto surfaceNode = GetSurfaceNode();
    if (!CheckSurfaceNodeForSnapshot(surfaceNode)) {
        return nullptr;
    }
    bool needBlurSnapshot = GetNeedUseBlurSnapshot();
    auto callback = std::make_shared<SurfaceCaptureFuture>();
    auto scaleValue = (scaleParam < 0.0f || std::fabs(scaleParam) < std::numeric_limits<float>::min()) ?
        snapshotScale_ : scaleParam;
    scaleValue = needBlurSnapshot ? scaleValue * BLUR_SNAPSHOT_SCALE : scaleValue;
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleValue,
        .scaleY = scaleValue,
        .useDma = true,
        .useCurWindow = useCurWindow,
        .backGroundColor = GetBackgroundColor(),
    };
    bool ret = false;
    if (needBlurSnapshot) {
        float blurRadius = 200.0f;
        ret = RSInterfaces::GetInstance().TakeSurfaceCaptureWithBlur(surfaceNode, callback, config, blurRadius);
    } else {
        ret = RSInterfaces::GetInstance().TakeSurfaceCapture(surfaceNode, callback, config);
    }
    if (!ret) {
        TLOGE(WmsLogTag::WMS_MAIN, "TakeSurfaceCapture failed");
        return nullptr;
    }
    constexpr int32_t FFRT_SNAPSHOT_TIMEOUT_MS = 5000;
    auto pixelMap = callback->GetResult(runInFfrt ? FFRT_SNAPSHOT_TIMEOUT_MS : SNAPSHOT_TIMEOUT_MS);
    if (pixelMap != nullptr) {
        TLOGI(WmsLogTag::WMS_MAIN, "Save snapshot WxH=%{public}dx%{public}d, id: %{public}d, uniqueId: %{public}d",
            pixelMap->GetWidth(), pixelMap->GetHeight(), persistentId_, pixelMap->GetUniqueId());
        if (notifySessionSnapshotFunc_) {
            notifySessionSnapshotFunc_(persistentId_);
        }
        return pixelMap;
    }
    TLOGE(WmsLogTag::WMS_MAIN, "Save snapshot failed, id: %{public}d", persistentId_);
    return nullptr;
}

bool Session::GetNeedUseBlurSnapshot() const
{
    bool isPrivacyMode = GetIsPrivacyMode();
    bool snapshotPrivacyMode = GetSnapshotPrivacyMode();
    ControlInfo controlInfo;
    bool isAppControl = GetAppControlInfo(ControlAppType::APP_LOCK, controlInfo);
    bool isAppUseControl = controlInfo.isNeedControl && !controlInfo.isControlRecentOnly;
    bool needUseBlurSnapshot = isPrivacyMode || snapshotPrivacyMode || (isAppControl && isAppUseControl);
    if (needUseBlurSnapshot) {
        TLOGI(WmsLogTag::WMS_PATTERN, "id: %{public}d, isPrivacyMode: %{public}d, snapshotPrivacyMode: %{public}d, "
            "isAppLock: %{public}d", persistentId_, isPrivacyMode, snapshotPrivacyMode, isAppUseControl);
    }
    return needUseBlurSnapshot;
}

bool Session::CheckSurfaceNodeForSnapshot(std::shared_ptr<RSSurfaceNode> surfaceNode) const
{
    if (scenePersistence_ == nullptr) {
        return false;
    }
    if (IsPersistentImageFit()) {
        return false;
    }
    auto key = GetSessionSnapshotStatus();
    if (!surfaceNode || !surfaceNode->IsBufferAvailable()) {
        scenePersistence_->SetHasSnapshot(false, key);
        scenePersistence_->SetHasSnapshotFreeMultiWindow(false);
        return false;
    }
    return true;
}

void Session::UpdateAppLockSnapshot(ControlAppType type, ControlInfo controlInfo)
{
    if (!WindowHelper::IsMainWindow(GetWindowType())) {
        return;
    }
    if (type != ControlAppType::APP_LOCK) {
        return;
    }
    bool isAppUseControl = controlInfo.isNeedControl && !controlInfo.isControlRecentOnly;
    TLOGI(WmsLogTag::WMS_PATTERN, "id: %{public}d, isAppLock: %{public}d", persistentId_, isAppUseControl);
    isAppLockControl_.store(isAppUseControl);
    if (isAppUseControl) {
        if (IsPersistentImageFit()) {
            NotifyAddSnapshot(false, false, false);
            return;
        }
        NotifyAddSnapshot(true, false, false);
        SaveSnapshot(true, true, nullptr, true);
        return;
    }
    if (IsSessionForeground()) {
        NotifyRemoveSnapshot();
        return;
    }
    if (!GetIsPrivacyMode() && !GetSnapshotPrivacyMode()) {
        SaveSnapshot(true, true, nullptr, true);
    }
}

bool Session::GetSnapshotPrivacyMode() const
{
    if (IsSessionForeground()) {
        return false;
    }
    return snapshotPrivacyMode_.load();
}

uint32_t Session::GetBackgroundColor() const
{
    std::shared_ptr<AbilityRuntime::ApplicationContext> appContext = AbilityRuntime::Context::GetApplicationContext();
    if (appContext != nullptr) {
        std::shared_ptr<AppExecFwk::Configuration> appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            std::string colorMode = appConfig->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
            TLOGI(WmsLogTag::WMS_PATTERN, ":%{public}s", colorMode.c_str());
            return (colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK) ? COLOR_BLACK : COLOR_WHITE;
        }
    } else {
        TLOGE(WmsLogTag::WMS_PATTERN, "app context is nullptr, use default backGroundColor WHITE");
    }
    return COLOR_WHITE;
}

void Session::RenameSnapshotFromOldPersistentId(int32_t oldPersistentId)
{
    auto task = [weakThis = wptr(this), oldPersistentId, where = __func__]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s session is nullptr", where);
            return;
        }
        std::map<std::string, std::string> renameMap;
        auto id = session->GetPersistentId();
        renameMap["SetImageForRecent_" + std::to_string(oldPersistentId)] = "SetImageForRecent_" + std::to_string(id);
        renameMap[session->GetSnapshotPersistentKey(oldPersistentId)] = session->GetSnapshotPersistentKey(id);
        for (uint32_t screenStatus = SCREEN_UNKNOWN; screenStatus < SCREEN_COUNT; screenStatus++) {
            renameMap[session->GetSnapshotPersistentKey(oldPersistentId, screenStatus)] =
                session->GetSnapshotPersistentKey(id, screenStatus);
        }
        ScenePersistentStorage::RenameKeys(renameMap, ScenePersistentStorageType::MAXIMIZE_STATE);
    };
    auto snapshotFfrtHelper = scenePersistence_->GetSnapshotFfrtHelper();
    std::string taskName = "RenameSnapshotFromOldPersistentId" + std::to_string(persistentId_);
    snapshotFfrtHelper->CancelTask(taskName);
    snapshotFfrtHelper->SubmitTask(std::move(task), taskName);
}

void Session::ResetSnapshot()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "id: %{public}d", persistentId_);
    std::lock_guard lock(snapshotMutex_);
    snapshot_ = nullptr;
    if (scenePersistence_ == nullptr) {
        TLOGI(WmsLogTag::WMS_PATTERN, "scenePersistence_ %{public}d nullptr", persistentId_);
        return;
    }
    scenePersistence_->ResetSnapshotCache();
}

void Session::ResetPreloadSnapshot()
{
    std::lock_guard<std::mutex> lock(preloadSnapshotMutex_);
    preloadSnapshot_ = nullptr;
}

void Session::SetEnableAddSnapshot(bool enableAddSnapshot)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "enableAddSnapshot: %{public}d", enableAddSnapshot);
    enableAddSnapshot_ = enableAddSnapshot;
}

bool Session::GetEnableAddSnapshot() const
{
    return enableAddSnapshot_;
}

void Session::SetBufferNameForPixelMap(const char* functionName, const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    std::string functionNameStr = (functionName != nullptr) ? functionName : "unknown";
    pixelMap->SetMemoryName(functionNameStr + '_' + std::to_string(GetPersistentId()));
}

void Session::SetPreloadingStartingWindow(bool preloading)
{
    preloadingStartingWindow_.store(preloading);
}

bool Session::GetPreloadingStartingWindow() const
{
    return preloadingStartingWindow_.load();
}

void Session::PreloadSnapshot()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "PreloadSnapshot[%d]", persistentId_);
    if (snapshot_ != nullptr) {
        std::lock_guard<std::mutex> lock(preloadSnapshotMutex_);
        preloadSnapshot_ = snapshot_;
        return;
    }
    if (scenePersistence_ == nullptr) {
        TLOGW(WmsLogTag::WMS_PATTERN, "scene persistence is null: %{public}d", GetPersistentId());
        return;
    }
    auto key = GetScreenSnapshotStatus();
    auto freeMultiWindow = freeMultiWindow_.load();
    auto hasSnapshot = scenePersistence_->HasSnapshot(key, freeMultiWindow);
    auto isSavingSnapshot = scenePersistence_->IsSavingSnapshot();
    const bool matchSnapshot = isSavingSnapshot || hasSnapshot;
    auto filePath = scenePersistence_->GetSnapshotFilePath(key, matchSnapshot, freeMultiWindow);
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    auto imageSource = Media::ImageSource::CreateImageSource(filePath, opts, errorCode);
    if (errorCode != 0 || imageSource == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "image source failed err %{public}d", errorCode);
        return;
    }
    Media::ImageInfo imageInfo;
    imageSource->GetImageInfo(imageInfo);
    TLOGI(WmsLogTag::WMS_PATTERN, "image size: %{public}d, %{public}d", imageInfo.size.width, imageInfo.size.height);
    Media::DecodeOptions decodeOpts;
    auto uniquePixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        TLOGE(WmsLogTag::WMS_PATTERN, "pixelMap failed err %{public}d", errorCode);
        return;
    }
    std::lock_guard<std::mutex> lock(preloadSnapshotMutex_);
    preloadSnapshot_ = std::move(uniquePixelMap);
}

void Session::SaveSnapshot(bool useFfrt, bool needPersist, std::shared_ptr<Media::PixelMap> persistentPixelMap,
    bool updateSnapshot, LifeCycleChangeReason reason)
{
    if (scenePersistence_ == nullptr) {
        return;
    }
    auto key = GetSessionSnapshotStatus(reason);
    auto rotation = currentRotation_;
    if (CORRECTION_ENABLE && key == 0) {
        rotation = (rotation + ROTATION_90) % ROTATION_360;
    }
    auto rotate = WSSnapshotHelper::GetDisplayOrientation(rotation);
    if (persistentPixelMap || !SupportSnapshotAllSessionStatus()) {
        key = defaultStatus;
        rotate = DisplayOrientation::PORTRAIT;
    }
    isSnapshotBlur_.store(GetNeedUseBlurSnapshot());
    bool needCacheSnapshot = (SupportCacheLockedSessionSnapshot() && reason == LifeCycleChangeReason::SCREEN_LOCK);
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), runInFfrt = useFfrt, requirePersist = needPersist, persistentPixelMap,
        updateSnapshot, key, rotate, needCacheSnapshot, reason, where]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        if (reason == LifeCycleChangeReason::QUICK_BATCH_BACKGROUND && session->snapshotNeedCancel_.load()) {
            TLOGNW(WmsLogTag::WMS_LIFE, "snapshot canceled id %{public}d", session->GetPersistentId());
            return;
        }
        session->lastLayoutRect_ = session->layoutRect_;
        auto pixelMap = persistentPixelMap ? persistentPixelMap : session->Snapshot(runInFfrt, 0.0f, updateSnapshot);
        if (pixelMap == nullptr) {
            return;
        }
        session->SetBufferNameForPixelMap(where, pixelMap);
        {
            std::lock_guard<std::mutex> lock(session->snapshotMutex_);
            session->snapshot_ = pixelMap;
        }
        {
            std::lock_guard<std::mutex> lock(session->addSnapshotCallbackMutex_);
            session->addSnapshotCallback_();
        }
        session->SetAddSnapshotCallback([]() {});
        if (!requirePersist) {
            return;
        }
        if (!session->scenePersistence_) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "scenePersistence_ is null");
            return;
        }
        if (updateSnapshot) {
            session->DeleteHasSnapshot();
            session->scenePersistence_->ClearSnapshot();
            session->NotifyUpdateSnapshotWindow();
        }
        session->SetHasSnapshot(key, rotate);
        Task saveSnapshotCallback = []() {};
        if (!needCacheSnapshot) {
            std::lock_guard lock(session->saveSnapshotCallbackMutex_);
            saveSnapshotCallback = session->saveSnapshotCallback_;
        }
        session->scenePersistence_->SaveSnapshot(pixelMap, saveSnapshotCallback, key, rotate,
            session->freeMultiWindow_.load());
    };
    if (!useFfrt) {
        task();
        return;
    }
    auto snapshotFfrtHelper = scenePersistence_->GetSnapshotFfrtHelper();
    std::string taskName = "Session::SaveSnapshot" + std::to_string(persistentId_);
    snapshotFfrtHelper->CancelTask(taskName);
    snapshotFfrtHelper->SubmitTask(std::move(task), taskName);
}

std::string Session::GetSnapshotPersistentKey(int32_t id)
{
    return "Snapshot_" + sessionInfo_.bundleName_ + "_" + std::to_string(id);
}

std::string Session::GetSnapshotPersistentKey(int32_t id, SnapshotStatus key)
{
    return "Snapshot_" + sessionInfo_.bundleName_ + "_"+ std::to_string(id) + "_" + std::to_string(key);
}

void Session::SetHasSnapshot(SnapshotStatus key, DisplayOrientation rotate)
{
    if (!scenePersistence_) {
        TLOGE(WmsLogTag::WMS_PATTERN, "scenePersistence is null");
        return;
    }
    if (freeMultiWindow_.load()) {
        scenePersistence_->SetHasSnapshotFreeMultiWindow(true);
        ScenePersistentStorage::Insert(GetSnapshotPersistentKey(persistentId_), EncodeSnapShotRecoverValue(rotate),
            ScenePersistentStorageType::MAXIMIZE_STATE);
    } else {
        scenePersistence_->SetHasSnapshot(true, key);
        ScenePersistentStorage::Insert(GetSnapshotPersistentKey(persistentId_, key),
            EncodeSnapShotRecoverValue(rotate), ScenePersistentStorageType::MAXIMIZE_STATE);
    }
}

int32_t Session::EncodeSnapShotRecoverValue(DisplayOrientation rotate)
{
    int32_t snapShotRecoverValue = 0;
    snapShotRecoverValue +=
        SessionHelper::ShiftDecimalDigit(static_cast<int32_t>(rotate),
            static_cast<int32_t>(SnapShotRecoverType::ROTATE));
    snapShotRecoverValue +=
        SessionHelper::ShiftDecimalDigit(static_cast<int32_t>(IsExitSplitOnBackgroundRecover()),
            static_cast<int32_t>(SnapShotRecoverType::EXIT_SPLIT_ON_BACKGROUND));
    return snapShotRecoverValue;
}

int32_t Session::DecodeSnapShotRecoverValue(int32_t snapShotRecoverValue, SnapShotRecoverType snapShotRecoverType)
{
    return (snapShotRecoverValue / static_cast<int32_t>(std::pow(DECIMAL_BASE,
        static_cast<int32_t>(snapShotRecoverType)))) % DECIMAL_BASE;
}

bool Session::IsExitSplitOnBackgroundRecover()
{
    return GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
           GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
           IsExitSplitOnBackground();
}

void Session::ResetLockedCacheSnapshot()
{
    if (!SupportCacheLockedSessionSnapshot()) {
        return;
    }
    if (snapshot_) {
        ResetSnapshot();
    }
}

bool Session::SupportCacheLockedSessionSnapshot() const
{
    return systemConfig_.supportCacheLockedSessionSnapshot_;
}

void Session::SetFreeMultiWindow()
{
    if (!SupportSnapshotAllSessionStatus()) {
        return;
    }
    if (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN) {
        freeMultiWindow_.store(false);
    } else {
        freeMultiWindow_.store(true);
    }
}

void Session::DeleteHasSnapshot()
{
    for (uint32_t screenStatus = SCREEN_UNKNOWN; screenStatus < SCREEN_COUNT; screenStatus++) {
        DeleteHasSnapshot(screenStatus);
    }
    DeleteHasSnapshotFreeMultiWindow();
}

void Session::DeleteHasSnapshot(SnapshotStatus key)
{
    auto hasSnapshot = ScenePersistentStorage::HasKey("Snapshot_" + sessionInfo_.bundleName_ +
        "_" + std::to_string(persistentId_) + "_" + std::to_string(key), ScenePersistentStorageType::MAXIMIZE_STATE);
    if (hasSnapshot) {
        ScenePersistentStorage::Delete("Snapshot_" + sessionInfo_.bundleName_ + "_" + std::to_string(persistentId_) +
            "_" + std::to_string(key), ScenePersistentStorageType::MAXIMIZE_STATE);
    }
}

void Session::DeleteHasSnapshotFreeMultiWindow()
{
    auto hasSnapshotFreeMultiWindow = ScenePersistentStorage::HasKey("Snapshot_" + sessionInfo_.bundleName_ +
        "_" + std::to_string(persistentId_), ScenePersistentStorageType::MAXIMIZE_STATE);
    if (hasSnapshotFreeMultiWindow) {
        ScenePersistentStorage::Delete("Snapshot_" + sessionInfo_.bundleName_ + "_" + std::to_string(persistentId_),
            ScenePersistentStorageType::MAXIMIZE_STATE);
    }
}

bool Session::HasSnapshot(SnapshotStatus key)
{
    auto hasSnapshot = ScenePersistentStorage::HasKey("Snapshot_" + sessionInfo_.bundleName_ +
        "_" + std::to_string(persistentId_) + "_" + std::to_string(key), ScenePersistentStorageType::MAXIMIZE_STATE);
    if (hasSnapshot && scenePersistence_) {
        scenePersistence_->SetHasSnapshot(true, key);
        int32_t snapShotRecoverValue = 0;
        ScenePersistentStorage::Get("Snapshot_" + sessionInfo_.bundleName_ +
            "_" + std::to_string(persistentId_) + "_" + std::to_string(key),
            snapShotRecoverValue, ScenePersistentStorageType::MAXIMIZE_STATE);
        scenePersistence_->rotate_[key] = static_cast<DisplayOrientation>(DecodeSnapShotRecoverValue(
            snapShotRecoverValue, SnapShotRecoverType::ROTATE));
        bool isExitSplitOnBackgroundRecover = static_cast<bool>(DecodeSnapShotRecoverValue(
            snapShotRecoverValue, SnapShotRecoverType::EXIT_SPLIT_ON_BACKGROUND));
        SetExitSplitOnBackground(isExitSplitOnBackgroundRecover);
    }
    return hasSnapshot;
}

bool Session::HasSnapshotFreeMultiWindow()
{
    auto hasSnapshotFreeMultiWindow = ScenePersistentStorage::HasKey("Snapshot_" + sessionInfo_.bundleName_ +
        "_"  + std::to_string(persistentId_), ScenePersistentStorageType::MAXIMIZE_STATE);
    if (hasSnapshotFreeMultiWindow && scenePersistence_) {
        freeMultiWindow_.store(true);
        scenePersistence_->SetHasSnapshotFreeMultiWindow(true);
        int32_t snapShotRecoverValue = 0;
        ScenePersistentStorage::Get("Snapshot_" + sessionInfo_.bundleName_ +
            "_" + std::to_string(persistentId_), snapShotRecoverValue, ScenePersistentStorageType::MAXIMIZE_STATE);
        bool isExitSplitOnBackgroundRecover = static_cast<bool>(DecodeSnapShotRecoverValue(
            snapShotRecoverValue, SnapShotRecoverType::EXIT_SPLIT_ON_BACKGROUND));
        SetExitSplitOnBackground(isExitSplitOnBackgroundRecover);
    }
    return hasSnapshotFreeMultiWindow;
}

bool Session::HasSnapshot()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "HasSnapshot[%d][%s]",
        persistentId_, sessionInfo_.bundleName_.c_str());
    bool hasSnapshot = false;
    for (uint32_t screenStatus = SCREEN_UNKNOWN; screenStatus < SCREEN_COUNT; screenStatus++) {
        hasSnapshot |= HasSnapshot(screenStatus);
    }
    return hasSnapshot || HasSnapshotFreeMultiWindow();
}

void Session::InitSnapshotCapacity()
{
    if (systemConfig_.supportSnapshotAllSessionStatus_) {
        capacity_ = maxCapacity;
    }
    if (scenePersistence_) {
        scenePersistence_->SetSnapshotCapacity(capacity_);
    }
}

bool Session::SupportSnapshotAllSessionStatus() const
{
    return (!IsPersistentImageFit() && (capacity_ != defaultCapacity));
}

SnapshotStatus Session::GetScreenSnapshotStatus() const
{
    if (!SupportSnapshotAllSessionStatus()) {
        return defaultStatus;
    }
    return WSSnapshotHelper::GetInstance()->GetScreenStatus();
}

SnapshotStatus Session::GetSessionSnapshotStatus(LifeCycleChangeReason reason) const
{
    if (!SupportSnapshotAllSessionStatus()) {
        return defaultStatus;
    }
    uint32_t snapshotScreen;
    if (state_ == SessionState::STATE_BACKGROUND || state_ == SessionState::STATE_DISCONNECT) {
        snapshotScreen = lastSnapshotScreen_;
    } else {
        snapshotScreen = WSSnapshotHelper::GetInstance()->GetScreenStatus();
    }
    if (reason == LifeCycleChangeReason::EXPAND_TO_FOLD_SINGLE_POCKET) {
        snapshotScreen = SCREEN_EXPAND;
    }
    return snapshotScreen;
}

uint32_t Session::GetWindowSnapshotOrientation() const
{
    if (!SupportSnapshotAllSessionStatus()) {
        return 0;
    }
    return WSSnapshotHelper::GetInstance()->GetWindowRotation();
}

uint32_t Session::GetLastOrientation() const
{
    if (!SupportSnapshotAllSessionStatus()) {
        return SNAPSHOT_PORTRAIT;
    }
    if (CORRECTION_ENABLE && WSSnapshotHelper::GetInstance()->GetScreenStatus() == 0) {
        return static_cast<uint32_t>(
            WSSnapshotHelper::GetDisplayOrientation((currentRotation_ + ROTATION_90) % ROTATION_360));
    }
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() &&
        WSSnapshotHelper::GetInstance()->GetScreenStatus() == SCREEN_FOLDED) {
        return ROTATION_LANDSCAPE_INVERTED;
    }
    return static_cast<uint32_t>(WSSnapshotHelper::GetDisplayOrientation(currentRotation_));
}

void Session::SetSessionStateChangeListenser(const NotifySessionStateChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            WLOGFE("session is null");
            return;
        }
        session->sessionStateChangeFunc_ = func;
        auto changedState = session->GetSessionState(); // read and write state should in one thread
        if (changedState == SessionState::STATE_ACTIVE) {
            changedState = SessionState::STATE_FOREGROUND;
        } else if (changedState == SessionState::STATE_INACTIVE) {
            changedState = SessionState::STATE_BACKGROUND;
        } else if (changedState == SessionState::STATE_DISCONNECT) {
            return;
        }
        session->NotifySessionStateChange(changedState);
        TLOGNI(WmsLogTag::WMS_LIFE, "id: %{public}d, state_: %{public}d, changedState: %{public}d",
            session->GetPersistentId(), session->GetSessionState(), changedState);
    }, "SetSessionStateChangeListenser");
}

void Session::SetClearSubSessionCallback(const NotifyClearSubSessionFunc& func)
{
    PostTask(
        [weakThis = wptr(this), func, where = __func__]() {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
                return;
            }
            session->clearSubSessionFunc_ = std::move(func);
        }, __func__);
}

bool Session::IsStatusBarVisible() const
{
    if (WindowHelper::IsMainWindow(Session::GetWindowType())) {
        return isStatusBarVisible_;
    } else if (WindowHelper::IsSubWindow(Session::GetWindowType())) {
        return GetMainSession() != nullptr ? GetMainSession()->isStatusBarVisible_ : false;
    }
    return false;
}

void Session::SetBufferAvailableChangeListener(const NotifyBufferAvailableChangeFunc& func)
{
    bufferAvailableChangeFunc_ = func;
    if (bufferAvailable_ && bufferAvailableChangeFunc_ != nullptr) {
        bufferAvailableChangeFunc_(bufferAvailable_, false);
    }
    WLOGFD("SetBufferAvailableChangeListener, id: %{public}d", GetPersistentId());
}

bool Session::UpdateWindowModeSupportType(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes =
        ExtractSupportWindowModeFromMetaData(abilityInfo);
    auto windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(updateWindowModes);
    const uint32_t noType = 0;
    if (windowModeSupportType != noType) {
        GetSessionProperty()->SetWindowModeSupportType(windowModeSupportType);
        return true;
    }
    return false;
}

void Session::SetGetRsCmdBlockingCountFunc(const GetRsCmdBlockingCountFunc& func)
{
    getRsCmdBlockingCountFunc_ = func;
}

void Session::SetAcquireRotateAnimationConfigFunc(const AcquireRotateAnimationConfigFunc& func)
{
    if (func == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "func is nullptr");
        return;
    }
    acquireRotateAnimationConfigFunc_ = func;
}

int32_t Session::GetRotateAnimationDuration()
{
    if (acquireRotateAnimationConfigFunc_) {
        RotateAnimationConfig rotateAnimationConfig;
        acquireRotateAnimationConfigFunc_(rotateAnimationConfig);
        return rotateAnimationConfig.duration_;
    }
    return ROTATE_ANIMATION_DURATION;
}

void Session::UnregisterSessionChangeListeners()
{
    sessionStateChangeFunc_ = nullptr;
    keyboardStateChangeFunc_ = nullptr;
    sessionFocusableChangeFunc_ = nullptr;
    sessionTouchableChangeFunc_ = nullptr;
    clickFunc_ = nullptr;
    jsSceneSessionExceptionFunc_ = nullptr;
    sessionExceptionFunc_ = nullptr;
    terminateSessionFunc_ = nullptr;
    pendingSessionActivationFunc_ = nullptr;
    changeSessionVisibilityWithStatusBarFunc_ = nullptr;
    bufferAvailableChangeFunc_ = nullptr;
    backPressedFunc_ = nullptr;
    terminateSessionFuncNew_ = nullptr;
    terminateSessionFuncTotal_ = nullptr;
    updateSessionLabelFunc_ = nullptr;
    updateSessionIconFunc_ = nullptr;
    pendingSessionToForegroundFunc_ = nullptr;
    pendingSessionToBackgroundForDelegatorFunc_ = nullptr;
    raiseToTopForPointDownFunc_ = nullptr;
    sessionInfoLockedStateChangeFunc_ = nullptr;
    contextTransparentFunc_ = nullptr;
    sessionRectChangeFunc_ = nullptr;
    sessionWindowLimitsChangeFunc_ = nullptr;
    updateSessionLabelAndIconFunc_ = nullptr;
    onRaiseMainWindowAboveTarget_ = nullptr;
    callingSessionIdChangeFunc_ = nullptr;
    WLOGFD("UnregisterSessionChangeListenser, id: %{public}d", GetPersistentId());
}

void Session::SetSessionStateChangeNotifyManagerListener(const NotifySessionStateChangeNotifyManagerFunc& func)
{
    sessionStateChangeNotifyManagerFunc_ = func;
    if (state_ == SessionState::STATE_DISCONNECT) {
        return;
    }
    NotifySessionStateChange(state_);
}

void Session::SetSessionInfoChangeNotifyManagerListener(const NotifySessionInfoChangeNotifyManagerFunc& func)
{
    sessionInfoChangeNotifyManagerFunc_ = func;
}

void Session::SetSessionPropertyChangeNotifyManagerListener(const NotifySessionPropertyChangeNotifyManagerFunc& func)
{
    sessionPropertyChangeNotifyManagerFunc_ = func;
}

void Session::SetDisplayIdChangedNotifyManagerListener(const NotifyDisplayIdChangedNotifyManagerFunc& func)
{
    displayIdChangedNotifyManagerFunc_ = func;
}

void Session::SetRequestFocusStatusNotifyManagerListener(const NotifyRequestFocusStatusNotifyManagerFunc& func)
{
    requestFocusStatusNotifyManagerFunc_ = func;
}

void Session::SetNotifyUIRequestFocusFunc(const NotifyUIRequestFocusFunc& func)
{
    std::unique_lock<std::shared_mutex> lock(uiRequestFocusMutex_);
    requestFocusFunc_ = func;
}

void Session::SetNotifyUILostFocusFunc(const NotifyUILostFocusFunc& func)
{
    std::unique_lock<std::shared_mutex> lock(uiLostFocusMutex_);
    lostFocusFunc_ = func;
}

void Session::SetGetStateFromManagerListener(const GetStateFromManagerFunc& func)
{
    getStateFromManagerFunc_ = func;
}

void Session::NotifySessionStateChange(const SessionState& state)
{
    PostTask([weakThis = wptr(this), state]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            WLOGFE("session is null");
            return;
        }
        TLOGND(WmsLogTag::WMS_LIFE, "NotifySessionStateChange, [state: %{public}u, persistent: %{public}d]",
            static_cast<uint32_t>(state), session->GetPersistentId());
        if (session->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT &&
            session->keyboardStateChangeFunc_) {
            const sptr<WindowSessionProperty>& property = session->GetSessionProperty();
            session->keyboardStateChangeFunc_(
                state, property->GetKeyboardEffectOption(), property->GetCallingSessionId(), property->GetDisplayId());
        } else if (session->sessionStateChangeFunc_) {
            session->sessionStateChangeFunc_(state);
        } else {
            TLOGNI(WmsLogTag::WMS_LIFE, "sessionStateChangeFunc is null");
        }
        if (!session->sessionStateChangeFunc_ && state == SessionState::STATE_DISCONNECT) {
            auto parentSession = session->GetParentSession();
            if (parentSession && parentSession->clearSubSessionFunc_) {
                parentSession->clearSubSessionFunc_(session->GetPersistentId());
                TLOGNI(WmsLogTag::WMS_LIFE, "notify clear subSession, parentId: %{public}d, "
                    "persistentId: %{public}d", parentSession->GetPersistentId(), session->GetPersistentId());
            }
        }

        if (session->sessionStateChangeNotifyManagerFunc_) {
            session->sessionStateChangeNotifyManagerFunc_(session->GetPersistentId(), state);
        } else {
            TLOGNI(WmsLogTag::WMS_LIFE, "sessionStateChangeNotifyManagerFunc is null");
        }
    }, "NotifySessionStateChange");
}

void Session::SetSessionFocusableChangeListener(const NotifySessionFocusableChangeFunc& func)
{
    sessionFocusableChangeFunc_ = func;
    NotifySessionFocusableChange(GetFocusable());
}

bool Session::GetSkipSelfWhenShowOnVirtualScreen() const
{
    return GetSessionProperty()->GetSkipSelfWhenShowOnVirtualScreen();
}

void Session::SetSessionTouchableChangeListener(const NotifySessionTouchableChangeFunc& func)
{
    sessionTouchableChangeFunc_ = func;
    sessionTouchableChangeFunc_(GetTouchable());
}

void Session::SetClickListener(const NotifyClickFunc& func)
{
    clickFunc_ = func;
}

void Session::NotifySessionFocusableChange(bool isFocusable)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d, focusable: %{public}u", GetPersistentId(), isFocusable);
    if (sessionFocusableChangeFunc_) {
        sessionFocusableChangeFunc_(isFocusable);
    }
}

void Session::NotifySessionTouchableChange(bool touchable)
{
    WLOGFD("Notify session touchable change: %{public}d", touchable);
    if (sessionTouchableChangeFunc_) {
        sessionTouchableChangeFunc_(touchable);
    }
}

void Session::NotifyClick(bool requestFocus, bool isClick)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d, focus: %{public}u, isClick: %{public}u",
        GetPersistentId(), requestFocus, isClick);
    if (clickFunc_) {
        clickFunc_(requestFocus, isClick);
    }
}

void Session::NotifyRequestFocusStatusNotifyManager(bool isFocused, bool byForeground, FocusChangeReason reason)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "NotifyRequestFocusStatusNotifyManager id: %{public}d, focused: %{public}d,\
        reason:  %{public}d", GetPersistentId(), isFocused, reason);
    if (requestFocusStatusNotifyManagerFunc_) {
        requestFocusStatusNotifyManagerFunc_(GetPersistentId(), isFocused, byForeground, reason);
    }
}

bool Session::GetStateFromManager(const ManagerState key)
{
    if (getStateFromManagerFunc_) {
        return getStateFromManagerFunc_(key);
    }
    switch (key) {
        case ManagerState::MANAGER_STATE_SCREEN_LOCKED:
            return false;
            break;
        default:
            return false;
    }
}

void Session::NotifyUIRequestFocus()
{
    WLOGFD("NotifyUIRequestFocus id: %{public}d", GetPersistentId());
    std::shared_lock<std::shared_mutex> lock(uiRequestFocusMutex_);
    if (requestFocusFunc_) {
        requestFocusFunc_();
    }
}

void Session::NotifyUILostFocus()
{
    WLOGFD("NotifyUILostFocus id: %{public}d", GetPersistentId());
    std::shared_lock<std::shared_mutex> lock(uiLostFocusMutex_);
    if (lostFocusFunc_) {
        lostFocusFunc_();
    }
}

void Session::PresentFocusIfNeed(int32_t pointerAction, int32_t sourceType)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "OnClick down, id: %{public}d", GetPersistentId());
    bool isHoverDown = pointerAction == MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER &&
        sourceType ==  MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN ||
        isHoverDown) {
        if (!isFocused_ && GetFocusable()) {
            FocusChangeReason reason = FocusChangeReason::CLICK;
            NotifyRequestFocusStatusNotifyManager(true, false, reason);
        }
        NotifyClickIfNeed();
    }
}

bool Session::IsNeedRequestToTop() const
{
    return WindowHelper::IsMainWindow(GetWindowType()) ? GetSessionProperty()->GetRaiseEnabled() : true;
}

void Session::NotifyClickIfNeed()
{
    if (IsNeedRequestToTop()) {
        NotifyClick(false);
    } else {
        TLOGI(WmsLogTag::WMS_FOCUS, "do not need to request to top");
    }
}

WSError Session::UpdateFocus(bool isFocused)
{
    if (isFocused_ == isFocused) {
        TLOGD(WmsLogTag::WMS_FOCUS, "Session focus do not change");
        return WSError::WS_DO_NOTHING;
    }
    isFocused_ = isFocused;
    UpdateGestureBackEnabled();
    // notify scb arkui focus
    if (!isFocused) {
        NotifyUILostFocus();
    }
    return WSError::WS_OK;
}

WSError Session::NotifyFocusStatus(const sptr<FocusNotifyInfo>& focusNotifyInfo, bool isFocused)
{
    if (!IsSessionValid()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Session stage is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_->UpdateFocus(focusNotifyInfo, isFocused);

    return WSError::WS_OK;
}

WSError Session::RequestFocus(bool isFocused)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;
    NotifyRequestFocusStatusNotifyManager(isFocused, false, reason);
    return WSError::WS_OK;
}

void Session::SetExclusivelyHighlighted(bool isExclusivelyHighlighted)
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "property is nullptr, windowId: %{public}d", persistentId_);
        return;
    }
    if (isExclusivelyHighlighted == property->GetExclusivelyHighlighted()) {
        return;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "windowId: %{public}d, isExclusivelyHighlighted: %{public}d", persistentId_,
        isExclusivelyHighlighted);
    property->SetExclusivelyHighlighted(isExclusivelyHighlighted);
}

WSError Session::UpdateHighlightStatus(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight,
    bool needBlockHighlightNotify)
{
    TLOGD(WmsLogTag::WMS_FOCUS,
        "windowId: %{public}d, currHighlight: %{public}d, nextHighlight: %{public}d, needBlockNotify:%{public}d",
        persistentId_, isHighlighted_, isHighlight, needBlockHighlightNotify);
    if (isHighlighted_ == isHighlight) {
        return WSError::WS_DO_NOTHING;
    }
    isHighlighted_ = isHighlight;
    if (!needBlockHighlightNotify) {
        NotifyHighlightChange(highlightNotifyInfo, isHighlight);
    }
    std::lock_guard lock(highlightChangeFuncMutex_);
    if (highlightChangeFunc_ != nullptr) {
        highlightChangeFunc_(isHighlight);
    }
    return WSError::WS_OK;
}

WSError Session::NotifyHighlightChange(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight)
{
    if (IsSystemSession()) {
        TLOGW(WmsLogTag::WMS_FOCUS, "Invalid [%{public}d, %{public}u]", persistentId_, GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_->NotifyHighlightChange(highlightNotifyInfo, isHighlight);
    return WSError::WS_OK;
}

WSError Session::GetIsHighlighted(bool& isHighlighted)
{
    isHighlighted = isHighlighted_;
    TLOGD(WmsLogTag::WMS_FOCUS, "windowId: %{public}d, isHighlighted: %{public}d",
        GetPersistentId(), isHighlighted_);
    return WSError::WS_OK;
}

WSError Session::SetAppSupportPhoneInPc(bool isSupportPhone)
{
    TLOGI(WmsLogTag::WMS_SCB, "isSupportPhone: %{public}d", isSupportPhone);
    GetSessionProperty()->SetIsAppSupportPhoneInPc(isSupportPhone);
    return WSError::WS_OK;
}

WSError Session::SetCompatibleModeProperty(const sptr<CompatibleModeProperty> compatibleModeProperty)
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "id: %{public}d property is nullptr", persistentId_);
        return WSError::WS_ERROR_NULLPTR;
    }
    property->SetCompatibleModeProperty(compatibleModeProperty);
    if (compatibleModeProperty && compatibleModeProperty->IsDragResizeDisabled()) {
        property->SetDragEnabled(false);
    }
    PostTask(
        [weakThis = wptr(this), where = __func__, weakProperty = wptr(compatibleModeProperty)]() {
            auto session = weakThis.promote();
            auto property = weakProperty.promote();
            if (!session || !session->sessionStage_) {
                TLOGNE(WmsLogTag::WMS_COMPAT, "session or sessionStage is null");
                return;
            }
            session->sessionStage_->NotifyCompatibleModePropertyChange(property);
        }, __func__);
    return WSError::WS_OK;
}

WSError Session::SetIsPcAppInPad(bool enable)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "SetIsPcAppInPad enable: %{public}d", enable);
    GetSessionProperty()->SetIsPcAppInPad(enable);
    return WSError::WS_OK;
}

WSError Session::SetPcAppInpadCompatibleMode(bool enabled)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "isPcAppInpadCompatibleMode: %{public}d", enabled);
    GetSessionProperty()->SetPcAppInpadCompatibleMode(enabled);
    return WSError::WS_OK;
}

WSError Session::SetPcAppInpadSpecificSystemBarInvisible(bool isPcAppInpadSpecificSystemBarInvisible)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "isPcAppInpadSpecificSystemBarInvisible: %{public}d",
        isPcAppInpadSpecificSystemBarInvisible);
    GetSessionProperty()->SetPcAppInpadSpecificSystemBarInvisible(isPcAppInpadSpecificSystemBarInvisible);
    return WSError::WS_OK;
}

WSError Session::SetPcAppInpadOrientationLandscape(bool isPcAppInpadOrientationLandscape)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "isPcAppInpadOrientationLandscape: %{public}d",
        isPcAppInpadOrientationLandscape);
    GetSessionProperty()->SetPcAppInpadOrientationLandscape(isPcAppInpadOrientationLandscape);
    return WSError::WS_OK;
}

WSError Session::SetMobileAppInPadLayoutFullScreen(bool isMobileAppInPadLayoutFullScreen)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "isMobileAppInPadLayoutFullScreen: %{public}d",
        isMobileAppInPadLayoutFullScreen);
    GetSessionProperty()->SetMobileAppInPadLayoutFullScreen(isMobileAppInPadLayoutFullScreen);
    return WSError::WS_OK;
}

WSError Session::PcAppInPadNormalClose()
{
    TLOGD(WmsLogTag::WMS_COMPAT, "windowId:%{public}d", GetPersistentId());
    if (!IsSessionValid()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "session stage is nullptr id: %{public}d state: %{public}u",
              GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->PcAppInPadNormalClose();
}

void Session::SetHasRequestedVsyncFunc(HasRequestedVsyncFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, func is null", GetPersistentId());
        return;
    }
    hasRequestedVsyncFunc_ = std::move(func);
}

void Session::SetRequestNextVsyncWhenModeChangeFunc(RequestNextVsyncWhenModeChangeFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, func is null", GetPersistentId());
        return;
    }
    requestNextVsyncWhenModeChangeFunc_ = std::move(func);
}

WSError Session::RequestNextVsyncWhenModeChange()
{
    if (!hasRequestedVsyncFunc_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, func is null", GetPersistentId());
        return WSError::WS_ERROR_NULLPTR;
    }
    isWindowModeDirty_.store(true);
    bool hasRequestedVsync = false;
    hasRequestedVsyncFunc_(hasRequestedVsync);
    timesToWaitForVsync_.store(hasRequestedVsync ? TIMES_TO_WAIT_FOR_VSYNC_TWICE : TIMES_TO_WAIT_FOR_VSYNC_ONECE);
    InitVsyncCallbackForModeChangeAndRequestNextVsync();
    return WSError::WS_OK;
}

void Session::OnVsyncReceivedAfterModeChanged()
{
    const char* const funcName = __func__;
    PostTask([weakThis = wptr(this), funcName] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", funcName);
            return;
        }
        if (!session->isWindowModeDirty_.load()) {
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: windowMode is not dirty, nothing to do, id:%{public}d",
                funcName, session->GetPersistentId());
            return;
        }
        session->timesToWaitForVsync_.fetch_sub(1);
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d, mode:%{public}d, waitVsyncTimes:%{public}d",
            funcName, session->GetPersistentId(), session->GetWindowMode(), session->timesToWaitForVsync_.load());
        bool isWindowModeDirty = true;
        if (session->timesToWaitForVsync_.load() > 0) {
            session->InitVsyncCallbackForModeChangeAndRequestNextVsync();
        } else if (session->timesToWaitForVsync_.load() < 0) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d, waitVsyncTimes:%{public}d",
                funcName, session->GetPersistentId(), session->timesToWaitForVsync_.load());
            session->timesToWaitForVsync_.store(0);
            session->isWindowModeDirty_.store(false);
        } else if (session->timesToWaitForVsync_.load() == 0 && session->sessionStage_ &&
                   session->isWindowModeDirty_.compare_exchange_strong(isWindowModeDirty, false)) {
            session->sessionStage_->NotifyLayoutFinishAfterWindowModeChange(session->GetWindowMode());
        } else {
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d, sessionStage is null or mode is not dirty",
                funcName, session->GetPersistentId());
            session->isWindowModeDirty_.store(false);
        }
        }, funcName);
}

void Session::InitVsyncCallbackForModeChangeAndRequestNextVsync()
{
    if (!requestNextVsyncWhenModeChangeFunc_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, func is null", GetPersistentId());
        return;
    }
    std::shared_ptr<VsyncCallback> nextVsyncCallback = std::make_shared<VsyncCallback>();
    nextVsyncCallback->onCallback = [weakThis = wptr(this), funcName = __func__](int64_t, int64_t) {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", funcName);
            return;
        }
        session->OnVsyncReceivedAfterModeChanged();
    };
    requestNextVsyncWhenModeChangeFunc_(nextVsyncCallback);
}

WSError Session::UpdateWindowMode(WindowMode mode)
{
    WLOGFD("Session update window mode, id: %{public}d, mode: %{public}d", GetPersistentId(),
        static_cast<int32_t>(mode));
    if (state_ == SessionState::STATE_END) {
        WLOGFI("session is already destroyed or property is nullptr! id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    } else if (state_ == SessionState::STATE_DISCONNECT) {
        GetSessionProperty()->SetWindowMode(mode);
        NotifySessionPropertyChange(WindowInfoKey::WINDOW_MODE);
        GetSessionProperty()->SetIsNeedUpdateWindowMode(true);
        UpdateGestureBackEnabled();
    } else {
        GetSessionProperty()->SetWindowMode(mode);
        NotifySessionPropertyChange(WindowInfoKey::WINDOW_MODE);
        RequestNextVsyncWhenModeChange();
        if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
            GetSessionProperty()->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
        }
        UpdateGestureBackEnabled();
        UpdateGravityWhenUpdateWindowMode(mode);
        if (!sessionStage_) {
            return WSError::WS_ERROR_NULLPTR;
        }
        return sessionStage_->UpdateWindowMode(mode);
    }
    return WSError::WS_OK;
}

void Session::UpdateGravityWhenUpdateWindowMode(WindowMode mode)
{
    auto surfaceNode = GetSurfaceNode();
    if ((systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode()) && surfaceNode != nullptr) {
        if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
            surfaceNode->SetFrameGravity(Gravity::LEFT);
        } else if (mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
            surfaceNode->SetFrameGravity(Gravity::RIGHT);
        } else if (mode == WindowMode::WINDOW_MODE_FLOATING || mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
            surfaceNode->SetFrameGravity(Gravity::RESIZE);
        }
    }
}

WSError Session::SetSystemSceneBlockingFocus(bool blocking)
{
    TLOGW(WmsLogTag::WMS_FOCUS, "Session set blocking focus, id: %{public}d, mode: %{public}d, Session is not system.",
        GetPersistentId(), blocking);
    return WSError::WS_ERROR_INVALID_SESSION;
}

bool Session::GetBlockingFocus() const
{
    return blockingFocus_;
}

WSError Session::SetSessionProperty(const sptr<WindowSessionProperty>& property)
{
    property_->CopyFrom(property);

    NotifySessionInfoChange();
    return WSError::WS_OK;
}

WSError Session::SetSessionPropertyForReconnect(const sptr<WindowSessionProperty>& property)
{
    property_->CopyFrom(property);

    auto hotAreasChangeCallback = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "session is nullptr");
            return;
        }
        session->NotifySessionInfoChange();
    };
    property_->SetSessionPropertyChangeCallback(hotAreasChangeCallback);
    NotifySessionInfoChange();
    return WSError::WS_OK;
}

/** @note @window.layout */
void Session::RectSizeCheckProcess(uint32_t curWidth, uint32_t curHeight, uint32_t minWidth,
    uint32_t minHeight, uint32_t maxFloatingWindowSize)
{
    constexpr uint32_t marginOfError = 2; // The max vp bias for preventing giant logs.
    if (curWidth + marginOfError < minWidth || curWidth > maxFloatingWindowSize + marginOfError ||
        curHeight + marginOfError < minHeight || curHeight > maxFloatingWindowSize + marginOfError) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "RectCheck err sessionID: %{public}d rect %{public}s",
            GetPersistentId(), GetSessionRect().ToString().c_str());
        std::ostringstream oss;
        oss << " RectCheck err size ";
        oss << " cur persistentId: " << GetPersistentId() << ",";
        oss << " windowType: " << static_cast<uint32_t>(GetWindowType()) << ",";
        oss << " windowName: " << GetSessionProperty()->GetWindowName() << ",";
        oss << " windowState: " << static_cast<uint32_t>(GetSessionProperty()->GetWindowState()) << ",";
        oss << " curWidth: " << curWidth << ",";
        oss << " curHeight: " << curHeight << ",";
        oss << " minWidth: " << minWidth << ",";
        oss << " minHeight: " << minHeight << ",";
        oss << " maxFloatingWindowSize: " << maxFloatingWindowSize << ",";
        oss << " sessionRect: " << GetSessionRect().ToString() << ";";
        WindowInfoReporter::GetInstance().ReportWindowException(
            static_cast<int32_t>(WindowDFXHelperType::WINDOW_RECT_CHECK), getpid(), oss.str());
    }
}

/** @note @window.layout */
void Session::RectCheckProcess()
{
    if (!(IsSessionForeground() || isVisible_)) {
        return;
    }
    auto displayId = GetSessionProperty()->GetDisplayId();
    std::map<ScreenId, ScreenProperty> screensProperties =
        Rosen::ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    if (screensProperties.find(displayId) == screensProperties.end()) {
        return;
    }
    auto screenProperty = screensProperties[displayId];
    float density = screenProperty.GetDensity();
    if (!NearZero(density) && !NearZero(GetSessionRect().height_)) {
        float curWidth = GetSessionRect().width_ / density;
        float curHeight = GetSessionRect().height_ / density;
        float ratio = GetAspectRatio();
        float actRatio = curWidth / curHeight;
        if (!NearZero(ratio) && !NearEqual(ratio, actRatio)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "RectCheck err ratio %{public}f != actRatio: %{public}f", ratio, actRatio);
            std::ostringstream oss;
            oss << " RectCheck err ratio ";
            oss << " cur persistentId: " << GetPersistentId() << ",";
            oss << " windowType: " << static_cast<uint32_t>(GetWindowType()) << ",";
            oss << " windowName: " << GetSessionProperty()->GetWindowName() << ",";
            oss << " windowState: " << static_cast<uint32_t>(GetSessionProperty()->GetWindowState()) << ",";
            oss << " curWidth: " << curWidth << ",";
            oss << " curHeight: " << curHeight << ",";
            oss << " setting ratio: " << ratio << ",";
            oss << " sessionRect: " << GetSessionRect().ToString() << ";";
            WindowInfoReporter::GetInstance().ReportWindowException(
                static_cast<int32_t>(WindowDFXHelperType::WINDOW_RECT_CHECK), getpid(), oss.str());
        }
        RectCheck(curWidth, curHeight);
    }
}

/** @note @window.layout */
void Session::SetSessionRect(const WSRect& rect)
{
    if (GetSessionRect() == rect) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id: %{public}d skip same rect", persistentId_);
        return;
    }
    layoutController_->SetSessionRect(rect);
    dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    RectCheckProcess();
}

/** @note @window.layout */
WSRect Session::GetSessionRect() const
{
    return layoutController_->GetSessionRect();
}

/** @note @window.layout */
WSRect Session::GetSessionScreenRelativeRect() const
{
    if (layoutController_->GetSizeChangeReason() == SizeChangeReason::DRAG_MOVE) {
        return layoutController_->ConvertGlobalRectToRelative(layoutController_->GetSessionRect(), GetDisplayId());
    }
    return layoutController_->GetSessionRect();
}

/** @note @window.layout */
WMError Session::GetGlobalScaledRect(Rect& globalScaledRect)
{
    auto task = [weakThis = wptr(this), &globalScaledRect] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "session is null");
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }
        session->GetLayoutController()->GetGlobalScaledRect(globalScaledRect);
        TLOGND(WmsLogTag::WMS_LAYOUT, "Id:%{public}d globalScaledRect:%{public}s",
            session->GetPersistentId(), globalScaledRect.ToString().c_str());
        return WMError::WM_OK;
    };
    return PostSyncTask(task, __func__);
}

/** @note @window.layout */
WSRect Session::GetSessionGlobalRect() const
{
    return layoutController_->GetSessionGlobalRect();
}

/** @note @window.layout */
WSRect Session::GetSessionGlobalRectInMultiScreen() const
{
    if (IsDragMoving()) {
        return GetLayoutController()->ConvertGlobalRectToRelative(GetSessionRect(), GetDisplayId());
    }
    return GetSessionGlobalRect();
}

/** @note @window.layout */
void Session::SetSessionGlobalRect(const WSRect& rect)
{
    if (layoutController_->SetSessionGlobalRect(rect)) {
        dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::GLOBAL_RECT);
        AddPropertyDirtyFlags(static_cast<uint32_t>(SessionPropertyFlag::WINDOW_GLOBAL_RECT));
    }
}

/** @note @window.layout */
WSRect Session::GetLastLayoutRect() const
{
    return lastLayoutRect_;
}

/** @note @window.layout */
WSRect Session::GetLayoutRect() const
{
    return layoutRect_;
}

void Session::SetSessionRequestRect(const WSRect& rect)
{
    GetSessionProperty()->SetRequestRect(SessionHelper::TransferToRect(rect));
    WLOGFD("is: %{public}d, rect: [%{public}d, %{public}d, %{public}u, %{public}u]", persistentId_,
        rect.posX_, rect.posY_, rect.width_, rect.height_);
}

WSRect Session::GetSessionRequestRect() const
{
    WSRect rect = SessionHelper::TransferToWSRect(GetSessionProperty()->GetRequestRect());
    WLOGFD("id: %{public}d, rect: %{public}s", persistentId_, rect.ToString().c_str());
    return rect;
}

/** @note @window.layout */
void Session::SetClientRect(const WSRect& rect)
{
    layoutController_->SetClientRect(rect);
}

/** @note @window.layout */
WSRect Session::GetClientRect() const
{
    return layoutController_->GetClientRect();
}

void Session::SetHidingStartingWindow(bool hidingStartWindow)
{
    hidingStartWindow_ = hidingStartWindow;
}

bool Session::GetHidingStartingWindow() const
{
    return hidingStartWindow_;
}

void Session::SetRestartApp(bool restartApp)
{
    sessionInfo_.isRestartApp_ = restartApp;
}

bool Session::GetRestartApp() const
{
    return sessionInfo_.isRestartApp_;
}

void Session::SetRestartInSameProcess(bool restartInSameProcess)
{
    sessionInfo_.isRestartInSameProcess_ = restartInSameProcess;
}

bool Session::GetRestartInSameProcess() const
{
    return sessionInfo_.isRestartInSameProcess_;
}

void Session::SetRestartCallerPersistentId(int32_t restartCallerPersistentId)
{
    sessionInfo_.restartCallerPersistentId_ = restartCallerPersistentId;
}

int32_t Session::GetRestartCallerPersistentId() const
{
    return sessionInfo_.restartCallerPersistentId_;
}

WSError Session::SetLeashWindowAlpha(bool hidingStartWindow)
{
    auto leashSurfaceNode = GetLeashWinSurfaceNode();
    if (leashSurfaceNode == nullptr) {
        TLOGI(WmsLogTag::WMS_PATTERN, "no leashWindow: %{public}d", hidingStartWindow);
        return WSError::WS_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_PATTERN, "hidingStartWindow: %{public}d", hidingStartWindow);
    hidingStartWindow ? leashSurfaceNode->SetAlpha(0) : leashSurfaceNode->SetAlpha(1);
    SetTouchable(!hidingStartWindow);
    return WSError::WS_OK;
}

void Session::SetEnableRemoveStartingWindow(bool enableRemoveStartingWindow)
{
    enableRemoveStartingWindow_ = enableRemoveStartingWindow;
}

bool Session::GetEnableRemoveStartingWindow() const
{
    return enableRemoveStartingWindow_;
}

void Session::SetAppBufferReady(bool appBufferReady)
{
    appBufferReady_ = appBufferReady;
}

bool Session::GetAppBufferReady() const
{
    return appBufferReady_;
}

void Session::SetUseStartingWindowAboveLocked(bool useStartingWindowAboveLocked)
{
    useStartingWindowAboveLocked_ = useStartingWindowAboveLocked;
}

bool Session::UseStartingWindowAboveLocked() const
{
    return useStartingWindowAboveLocked_;
}

void Session::SetRequestRectAnimationConfig(const RectAnimationConfig& rectAnimationConfig)
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "id: %{public}d property is nullptr", persistentId_);
        return;
    }
    property->SetRectAnimationConfig(rectAnimationConfig);
    TLOGI(WmsLogTag::WMS_LAYOUT, "id: %{public}d, animation duration: [%{public}u]", persistentId_,
        rectAnimationConfig.duration);
}

RectAnimationConfig Session::GetRequestRectAnimationConfig() const
{
    RectAnimationConfig rectAnimationConfig;
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "id: %{public}d property is nullptr", persistentId_);
        return rectAnimationConfig;
    }
    rectAnimationConfig = property->GetRectAnimationConfig();
    TLOGI(WmsLogTag::WMS_LAYOUT, "id: %{public}d, animation duration: [%{public}u]", persistentId_,
        rectAnimationConfig.duration);
    return rectAnimationConfig;
}

WindowType Session::GetWindowType() const
{
    return GetSessionProperty()->GetWindowType();
}

std::string Session::GetWindowName() const
{
    if (GetSessionInfo().isSystem_) {
        return GetSessionInfo().abilityName_;
    } else {
        return GetSessionProperty()->GetWindowName();
    }
}

void Session::SetSystemConfig(const SystemSessionConfig& systemConfig)
{
    systemConfig_ = systemConfig;
}

SystemSessionConfig Session::GetSystemConfig() const
{
    return systemConfig_;
}

void Session::SetSnapshotScale(const float snapshotScale)
{
    snapshotScale_ = snapshotScale;
}

WSError Session::ProcessBackEvent()
{
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_EVENT, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_EVENT, "session stage is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (auto remoteObject = sessionStage_->AsObject();
        remoteObject && !remoteObject->IsProxyObject()) {
        PostExportTask([sessionStage = sessionStage_] {
            sessionStage->HandleBackEvent();
        });
        return WSError::WS_OK;
    }
    return sessionStage_->HandleBackEvent();
}

void Session::GeneratePersistentId(bool isExtension, int32_t persistentId)
{
    std::lock_guard lock(g_persistentIdSetMutex);
    if (persistentId != INVALID_SESSION_ID  && !g_persistentIdSet.count(persistentId)) {
        g_persistentIdSet.insert(persistentId);
        persistentId_ = persistentId;
        return;
    }

    if (g_persistentId == INVALID_SESSION_ID) {
        g_persistentId++; // init non system session id from 2
    }

    g_persistentId++;
    while (g_persistentIdSet.count(g_persistentId)) {
        g_persistentId++;
    }
    if (isExtension) {
        constexpr uint32_t pidLength = 18;
        constexpr uint32_t pidMask = (1 << pidLength) - 1;
        constexpr uint32_t persistentIdLength = 12;
        constexpr uint32_t persistentIdMask = (1 << persistentIdLength) - 1;
        uint32_t assembledPersistentId = ((static_cast<uint32_t>(getpid()) & pidMask) << persistentIdLength) |
            (static_cast<uint32_t>(g_persistentId.load()) & persistentIdMask);
        persistentId_ = assembledPersistentId | 0x40000000;
    } else {
        persistentId_ = static_cast<uint32_t>(g_persistentId.load()) & 0x3fffffff;
    }
    g_persistentIdSet.insert(g_persistentId);
    TLOGI(WmsLogTag::WMS_LIFE,
        "persistentId: %{public}d, persistentId_: %{public}d", persistentId, persistentId_);
}

sptr<ScenePersistence> Session::GetScenePersistence() const
{
    return scenePersistence_;
}

bool Session::CheckEmptyKeyboardAvoidAreaIfNeeded() const
{
    bool isMainFloating =
        GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING && WindowHelper::IsMainWindow(GetWindowType());
    bool isParentFloating = SessionHelper::IsNonSecureToUIExtension(GetWindowType()) &&
        GetParentSession() != nullptr &&
        GetParentSession()->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
    bool isMidScene = GetIsMidScene();
    bool isPhoneNotFreeMultiWindow = systemConfig_.IsPhoneWindow() && !systemConfig_.IsFreeMultiWindowMode();
    bool isPadNotFreeMultiWindow = systemConfig_.IsPadWindow() && !systemConfig_.IsFreeMultiWindowMode();
    bool isPhoneOrPadNotFreeMultiWindow = isPhoneNotFreeMultiWindow || isPadNotFreeMultiWindow;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "check keyboard avoid area, isPhoneNotFreeMultiWindow: %{public}d, "
        "isPadNotFreeMultiWindow: %{public}d", isPhoneNotFreeMultiWindow, isPadNotFreeMultiWindow);
    auto screen = ScreenManager::GetInstance().GetScreenById(property_->GetDisplayId());
    if (screen != nullptr && screen->GetName() == APP_CAST_SCREEN_NAME) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "app cast screen");
        return true;
    }
    return (isMainFloating || isParentFloating) && !isMidScene && isPhoneOrPadNotFreeMultiWindow;
}

void Session::SetKeyboardStateChangeListener(const NotifyKeyboardStateChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__]() {
        auto session = weakThis.promote();
        if (session == nullptr || func == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session or func is null", where);
            return;
        }
        session->keyboardStateChangeFunc_ = func;
        auto newState = session->GetSessionState(); // read and write state should in one thread
        if (newState == SessionState::STATE_ACTIVE) {
            newState = SessionState::STATE_FOREGROUND;
        } else if (newState == SessionState::STATE_INACTIVE) {
            newState = SessionState::STATE_BACKGROUND;
        } else if (newState == SessionState::STATE_DISCONNECT) {
            return;
        }
        session->NotifySessionStateChange(newState);
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "%{public}s id: %{public}d, state_: %{public}d, newState: %{public}d",
            where, session->GetPersistentId(), session->GetSessionState(), newState);
    }, __func__);
}

void Session::SetCallingSessionIdSessionListenser(const ProcessCallingSessionIdChangeFunc&& func)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__]() {
        auto session = weakThis.promote();
        if (session == nullptr || func == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session or func is null", where);
            return;
        }
        session->callingSessionIdChangeFunc_ = std::move(func);
    }, __func__);
}

void Session::NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
    const std::shared_ptr<RSTransaction>& rsTransaction, const Rect& callingSessionRect,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if (!sessionStage_) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "session stage is nullptr");
        return;
    }
    if (CheckEmptyKeyboardAvoidAreaIfNeeded()) {
        info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Occupied area needs to be empty when in floating mode");
    }
    if (info != nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling id: %{public}d, callingSessionRect: %{public}s"
            ", safeRect: %{public}s, textFieldPositionY_: %{public}f, textFieldHeight_: %{public}f"
            ", size of avoidAreas: %{public}d", GetPersistentId(),
            callingSessionRect.ToString().c_str(), info->rect_.ToString().c_str(),
            info->textFieldPositionY_, info->textFieldHeight_, static_cast<int32_t>(avoidAreas.size()));
        for (const auto& [type, avoidArea] : avoidAreas) {
            TLOGI(WmsLogTag::WMS_KEYBOARD, "avoidAreaType: %{public}u, avoidArea: %{public}s",
                type, avoidArea.ToString().c_str());
        }
    } else {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "occupied area info is nullptr, id: %{public}d", GetPersistentId());
    }
    sessionStage_->NotifyOccupiedAreaChangeInfo(info, rsTransaction, callingSessionRect, avoidAreas);
}

WindowMode Session::GetWindowMode() const
{
    return GetSessionProperty()->GetWindowMode();
}

WSError Session::UpdateMaximizeMode(bool isMaximize)
{
    WLOGFD("Session update maximize mode, isMaximize: %{public}d", isMaximize);
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    MaximizeMode mode = MaximizeMode::MODE_RECOVER;
    if (isMaximize) {
        mode = MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    } else if (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN) {
        mode = MaximizeMode::MODE_FULL_FILL;
    }
    GetSessionProperty()->SetMaximizeMode(mode);
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_MAIN, "sessionStage_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->UpdateMaximizeMode(mode);
}

/** @note @window.hierarchy */
void Session::SetZOrder(uint32_t zOrder)
{
    lastZOrder_ = zOrder_;
    zOrder_ = zOrder;
    NotifySessionInfoChange();
}

/** @note @window.hierarchy */
uint32_t Session::GetZOrder() const
{
    return zOrder_;
}

/** @note @window.hierarchy */
uint32_t Session::GetLastZOrder() const
{
    return lastZOrder_;
}

void Session::SetUINodeId(uint32_t uiNodeId)
{
    if (uiNodeId_ != 0 && uiNodeId != 0 && !IsSystemSession() && SessionPermission::IsBetaVersion()) {
        int32_t eventRet = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "REPEAT_SET_UI_NODE_ID",
            OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "PID", getpid(),
            "UID", getuid());
        TLOGE(WmsLogTag::WMS_LIFE, " SetUINodeId: Repeat set UINodeId ret:%{public}d", eventRet);
        return;
    }
    uiNodeId_ = uiNodeId;
}

uint32_t Session::GetUINodeId() const
{
    return uiNodeId_;
}

void Session::SetShowRecent(bool showRecent)
{
    TLOGI(WmsLogTag::WMS_MAIN, "in recents: %{public}d, id: %{public}d", showRecent, persistentId_);
    bool isAttach = GetAttachState();
    if (!IsSupportDetectWindow(isAttach) ||
        !ShouldCreateDetectTaskInRecent(showRecent, showRecent_, isAttach)) {
        showRecent_ = showRecent;
        return;
    }
    showRecent_ = showRecent;
    WindowMode windowMode = GetWindowMode();
    if (!showRecent_ && ShouldCreateDetectTask(isAttach, windowMode)) {
        CreateWindowStateDetectTask(isAttach, windowMode);
    }
}

bool Session::GetShowRecent() const
{
    return showRecent_;
}

bool Session::GetAttachState() const
{
    return isAttach_;
}

DetectTaskInfo Session::GetDetectTaskInfo() const
{
    std::shared_lock<std::shared_mutex> lock(detectTaskInfoMutex_);
    return detectTaskInfo_;
}

void Session::SetDetectTaskInfo(const DetectTaskInfo& detectTaskInfo)
{
    std::unique_lock<std::shared_mutex> lock(detectTaskInfoMutex_);
    detectTaskInfo_ = detectTaskInfo;
}

bool Session::IsStateMatch(bool isAttach) const
{
    return isAttach ? ATTACH_MAP.at(GetSessionState()) : DETACH_MAP.at(GetSessionState());
}

bool Session::IsSupportDetectWindow(bool isAttach)
{
    if (!systemConfig_.IsPcWindow() && !systemConfig_.IsPhoneWindow()) {
        TLOGD(WmsLogTag::WMS_LIFE, "device type not support, id:%{public}d", persistentId_);
        return false;
    }
    if (isScreenLockedCallback_ && isScreenLockedCallback_()) {
        TLOGD(WmsLogTag::WMS_LIFE, "screen locked, id:%{public}d", persistentId_);
        return false;
    }
    if (!SessionHelper::IsMainWindow(GetWindowType())) {
        TLOGD(WmsLogTag::WMS_LIFE, "only support main window, id:%{public}d", persistentId_);
        return false;
    }
    // Only detecting cold start scenarios on PC
    if (systemConfig_.IsPcWindow() && (!isAttach || state_ != SessionState::STATE_DISCONNECT)) {
        TLOGD(WmsLogTag::WMS_LIFE, "pc only support cold start, id:%{public}d", persistentId_);
        RemoveWindowDetectTask();
        return false;
    }
    return true;
}

void Session::RemoveWindowDetectTask()
{
    if (handler_) {
        handler_->RemoveTask(GetWindowDetectTaskName());
    }
}

bool Session::ShouldCreateDetectTask(bool isAttach, WindowMode windowMode) const
{
    // Create detect task directy without pre-exiting tasks.
    if (GetDetectTaskInfo().taskState == DetectTaskState::NO_TASK) {
        return true;
    }
    // If the taskState matches the attach detach state, it will be create detect task directly.
    if ((GetDetectTaskInfo().taskState == DetectTaskState::ATTACH_TASK && isAttach) ||
        (GetDetectTaskInfo().taskState == DetectTaskState::DETACH_TASK && !isAttach)) {
        return true;
    } else {
        // Do not create detect task if the windowMode changes.
        return GetDetectTaskInfo().taskWindowMode == windowMode;
    }
}

bool Session::ShouldCreateDetectTaskInRecent(bool newShowRecent, bool oldShowRecent, bool isAttach) const
{
    if (newShowRecent) {
        return false;
    }
    return oldShowRecent ? isAttach : false;
}

void Session::RegisterIsScreenLockedCallback(const std::function<bool()>& callback)
{
    isScreenLockedCallback_ = callback;
}

std::string Session::GetWindowDetectTaskName() const
{
    return "wms:WindowStateDetect" + std::to_string(persistentId_);
}

void Session::RecordWindowStateAttachExceptionEvent(bool isAttached)
{
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "WINDOW_STATE_ERROR",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "TYPE", "WINDOW_STATE_ATTACH_EXCEPTION",
        "PERSISTENT_ID", GetPersistentId(),
        "WINDOW_NAME", GetWindowName().c_str(),
        "ATTACH_STATE", isAttached,
        "SESSION_STATE", static_cast<uint32_t>(GetSessionState()));
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "write HiSysEvent error, ret: %{public}d", ret);
    }
}

void Session::CreateWindowStateDetectTask(bool isAttach, WindowMode windowMode)
{
    if (!handler_) {
        return;
    }
    std::string taskName = GetWindowDetectTaskName();
    RemoveWindowDetectTask();
    auto detectTask = [weakThis = wptr(this), isAttach]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            if (isAttach) {
                TLOGNE(WmsLogTag::WMS_LIFE, "Window attach state and session"
                    "state mismatch, session is nullptr, attach:%{public}d", isAttach);
            }
            return;
        }
        // Skip state detect when screen locked.
        if (session->isScreenLockedCallback_ && !session->isScreenLockedCallback_()) {
            if (!session->IsStateMatch(isAttach)) {
                TLOGNE(WmsLogTag::WMS_LIFE, "Window attach state and session state mismatch, "
                    "attach:%{public}d, sessioniState:%{public}d, persistenId:%{public}d, bundleName:%{public}s",
                    isAttach, static_cast<uint32_t>(session->GetSessionState()),
                    session->GetPersistentId(), session->GetSessionInfo().bundleName_.c_str());
                session->RecordWindowStateAttachExceptionEvent(isAttach);
            }
        }
        DetectTaskInfo detectTaskInfo;
        session->SetDetectTaskInfo(detectTaskInfo);
    };
    handler_->PostTask(detectTask, taskName, STATE_DETECT_DELAYTIME);
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskWindowMode = windowMode;
    detectTaskInfo.taskState = isAttach ? DetectTaskState::ATTACH_TASK : DetectTaskState::DETACH_TASK;
    SetDetectTaskInfo(detectTaskInfo);
}

void Session::SetBufferAvailable(bool bufferAvailable, bool startWindowInvisible)
{
    TLOGD(WmsLogTag::DEFAULT, "Set:%{public}d %{public}d", bufferAvailable, startWindowInvisible);
    if (bufferAvailableChangeFunc_) {
        bufferAvailableChangeFunc_(bufferAvailable, startWindowInvisible);
    }
    bufferAvailable_ = bufferAvailable;
}

bool Session::GetBufferAvailable() const
{
    return bufferAvailable_;
}

void Session::SetNeedSnapshot(bool needSnapshot)
{
    needSnapshot_ = needSnapshot;
}

void Session::SetExitSplitOnBackground(bool isExitSplitOnBackground)
{
    TLOGW(WmsLogTag::WMS_MULTI_WINDOW, "id: %{public}d, SetExitSplitOnBackground not implement", persistentId_);
}

bool Session::IsExitSplitOnBackground() const
{
    TLOGW(WmsLogTag::WMS_MULTI_WINDOW, "id: %{public}d, IsExitSplitOnBackground not implement", persistentId_);
    return false;
}

void Session::SetFloatingScale(float floatingScale)
{
    floatingScale_ = floatingScale;
}

float Session::GetFloatingScale() const
{
    return floatingScale_;
}

void Session::SetScale(float scaleX, float scaleY, float pivotX, float pivotY)
{
    layoutController_->SetScale(scaleX, scaleY, pivotX, pivotY);
}

void Session::SetClientScale(float scaleX, float scaleY, float pivotX, float pivotY)
{
    layoutController_->SetClientScale(scaleX, scaleY, pivotX, pivotY);
}

float Session::GetScaleX() const
{
    return layoutController_->GetScaleX();
}

float Session::GetScaleY() const
{
    return layoutController_->GetScaleY();
}

float Session::GetPivotX() const
{
    return layoutController_->GetPivotX();
}

float Session::GetPivotY() const
{
    return layoutController_->GetPivotY();
}

void Session::SetSCBKeepKeyboard(bool scbKeepKeyboardFlag)
{
    scbKeepKeyboardFlag_ = scbKeepKeyboardFlag;
}

bool Session::GetSCBKeepKeyboardFlag() const
{
    return scbKeepKeyboardFlag_;
}

void Session::SetOffset(float x, float y)
{
    offsetX_ = x;
    offsetY_ = y;
    WSRect newRect {
        .posX_ = std::round(bounds_.posX_ + x),
        .posY_ = std::round(bounds_.posY_ + y),
        .width_ = std::round(bounds_.width_),
        .height_ = std::round(bounds_.height_),
    };
    if (newRect != GetSessionRect()) {
        UpdateRect(newRect, SizeChangeReason::UNDEFINED, "SetOffset");
    }
}

float Session::GetOffsetX() const
{
    return offsetX_;
}

float Session::GetOffsetY() const
{
    return offsetY_;
}

void Session::SetBounds(const WSRectF& bounds)
{
    bounds_ = bounds;
}

WSRectF Session::GetBounds()
{
    return bounds_;
}

void Session::SetRotation(Rotation rotation)
{
    rotation_ = rotation;
}

Rotation Session::GetRotation() const
{
    return rotation_;
}

WSError Session::UpdateTitleInTargetPos(bool isShow, int32_t height)
{
    WLOGFD("Session update title in target position, id: %{public}d, isShow: %{public}d, height: %{public}d",
        GetPersistentId(), isShow, height);
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_MAIN, "sessionStage_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->UpdateTitleInTargetPos(isShow, height);
}

void Session::SetSessionInfoLockedStateChangeListener(const NotifySessionInfoLockedStateChangeFunc& func)
{
    sessionInfoLockedStateChangeFunc_ = func;
}

void Session::NotifySessionInfoLockedStateChange(bool lockedState)
{
    WLOGFD("Notify sessioninfo lockedstate change: %{public}u", lockedState);
    if (sessionInfoLockedStateChangeFunc_) {
        sessionInfoLockedStateChangeFunc_(lockedState);
    }
}

std::vector<AppExecFwk::SupportWindowMode> Session::ExtractSupportWindowModeFromMetaData(
    const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = {};
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "id: %{public}d, abilityInfo is nullptr", GetPersistentId());
        return updateWindowModes;
    }
    if (IsPcWindow() || systemConfig_.IsFreeMultiWindowMode()) {
        auto metadata = abilityInfo->metadata;
        for (const auto& item : metadata) {
            if (item.name == "ohos.ability.window.supportWindowModeInFreeMultiWindow") {
                updateWindowModes = ParseWindowModeFromMetaData(item.value);
                return updateWindowModes;
            }
        }
    }
    if (updateWindowModes.empty()) {
        updateWindowModes = abilityInfo->windowModes;
    }
    return updateWindowModes;
}

std::vector<AppExecFwk::SupportWindowMode> Session::ParseWindowModeFromMetaData(
    const std::string& supportModesInFreeMultiWindow)
{
    static const std::unordered_map<std::string, AppExecFwk::SupportWindowMode> modeMap = {
        {"fullscreen", AppExecFwk::SupportWindowMode::FULLSCREEN},
        {"split", AppExecFwk::SupportWindowMode::SPLIT},
        {"floating", AppExecFwk::SupportWindowMode::FLOATING}
    };
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = {};
    for (auto iter : modeMap) {
        if (supportModesInFreeMultiWindow.find(iter.first) != std::string::npos) {
            updateWindowModes.push_back(iter.second);
        }
    }
    return updateWindowModes;
}

WSError Session::SwitchFreeMultiWindow(const SystemSessionConfig& config)
{
    bool enable = config.freeMultiWindowEnable_;
    systemConfig_.defaultWindowMode_ = config.defaultWindowMode_;
    systemConfig_.freeMultiWindowEnable_ = enable;
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sessionStage_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (!UpdateWindowModeSupportType(sessionInfo_.abilityInfo)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "not update WindowModeSupportType");
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "windowId: %{public}d enable: %{public}d defaultWindowMode: %{public}d",
        GetPersistentId(), enable, systemConfig_.defaultWindowMode_);
    bool isUiExtSubWindow = WindowHelper::IsSubWindow(GetSessionProperty()->GetWindowType()) &&
        GetSessionProperty()->GetIsUIExtFirstSubWindow();
    if (WindowHelper::IsMainWindow(GetWindowType()) || isUiExtSubWindow) {
        return sessionStage_->SwitchFreeMultiWindow(enable);
    }
    return WSError::WS_OK;
}

WSError Session::GetIsMidScene(bool& isMidScene)
{
    isMidScene = GetIsMidScene();
    return WSError::WS_OK;
}

WSError Session::GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj)
{
    if (!IsSessionValid()) {
        TLOGE(WmsLogTag::DEFAULT, "session %{public}d is invalid. Failed to get UIContentRemoteObj", GetPersistentId());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sessionStage_ is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->GetUIContentRemoteObj(uiContentRemoteObj);
}

void Session::SetNotifySystemSessionPointerEventFunc(const NotifySystemSessionPointerEventFunc& func)
{
    std::lock_guard<std::mutex> lock(pointerEventMutex_);
    systemSessionPointerEventFunc_ = func;
}

void Session::SetNotifySystemSessionKeyEventFunc(const NotifySystemSessionKeyEventFunc& func)
{
    std::unique_lock<std::shared_mutex> lock(keyEventMutex_);
    systemSessionKeyEventFunc_ = func;
}

void Session::NotifySessionInfoChange()
{
    if (sessionInfoChangeNotifyManagerFunc_) {
        sessionInfoChangeNotifyManagerFunc_(GetPersistentId());
    } else {
        TLOGD(WmsLogTag::WMS_EVENT, "sessionInfoChangeNotifyManagerFunc is nullptr");
    }
}

void Session::NotifySessionPropertyChange(WindowInfoKey windowInfoKey)
{
    if (sessionPropertyChangeNotifyManagerFunc_) {
        sessionPropertyChangeNotifyManagerFunc_(GetPersistentId(), windowInfoKey);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Func is invalid");
    }
}

void Session::NotifyDisplayIdChanged(int32_t persistentId, uint64_t displayId)
{
    if (displayIdChangedNotifyManagerFunc_) {
        displayIdChangedNotifyManagerFunc_(persistentId, displayId);
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "displayIdChangedNotifyManagerFunc is nullptr");
    }
}

bool Session::NeedCheckContextTransparent() const
{
    return contextTransparentFunc_ != nullptr;
}

void Session::SetContextTransparentFunc(const NotifyContextTransparentFunc& func)
{
    contextTransparentFunc_ = func;
}

void Session::NotifyContextTransparent()
{
    if (contextTransparentFunc_) {
        int32_t eventRet = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "SESSION_IS_TRANSPARENT",
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "PERSISTENT_ID", GetPersistentId(),
            "BUNDLE_NAME", sessionInfo_.bundleName_);
        WLOGFE("Session context is transparent, persistentId:%{public}d, eventRet:%{public}d",
            GetPersistentId(), eventRet);
        contextTransparentFunc_();
    }
}

bool Session::IsSystemInput()
{
    return sessionInfo_.sceneType_ == SceneType::INPUT_SCENE;
}

void Session::SetIsMidScene(bool isMidScene)
{
    PostTask([weakThis = wptr(this), isMidScene] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "session is null");
            return;
        }
        if (session->isMidScene_ != isMidScene) {
            TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "persistentId:%{public}d, isMidScene:%{public}d",
                session->GetPersistentId(), isMidScene);
            session->isMidScene_ = isMidScene;
            session->NotifySessionPropertyChange(WindowInfoKey::MID_SCENE);
        }
    }, "SetIsMidScene");
}

bool Session::GetIsMidScene() const
{
    if (isMidScene_) {
        return true;
    }
    if (GetParentSession() == nullptr) {
        return false;
    }
    return GetParentSession()->GetIsMidScene();
}

void Session::SetTouchHotAreas(const std::vector<Rect>& touchHotAreas)
{
    std::vector<Rect> lastTouchHotAreas;
    GetSessionProperty()->GetTouchHotAreas(lastTouchHotAreas);
    if (touchHotAreas == lastTouchHotAreas) {
        return;
    }

    dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::TOUCH_HOT_AREA);
    std::string rectStr;
    for (const auto& rect : touchHotAreas) {
        rectStr = rectStr + " " + rect.ToString();
    }
    TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d hot:%{public}s", GetPersistentId(), rectStr.c_str());
    GetSessionProperty()->SetTouchHotAreas(touchHotAreas);
}

std::shared_ptr<Media::PixelMap> Session::GetSnapshotPixelMap(const float oriScale, const float newScale)
{
    TLOGI(WmsLogTag::WMS_MAIN, "id %{public}d", GetPersistentId());
    if (scenePersistence_ == nullptr) {
        return nullptr;
    }
    auto key = GetScreenSnapshotStatus();
    return scenePersistence_->IsSavingSnapshot() ? GetSnapshot() :
        scenePersistence_->GetLocalSnapshotPixelMap(oriScale, newScale, key, freeMultiWindow_.load());
}

bool Session::IsVisibleForeground() const
{
    return isVisible_ && IsSessionForeground();
}

bool Session::IsVisibleNotBackground() const
{
    return isVisible_ && IsSessionNotBackground();
}

void Session::SetIsStarting(bool isStarting)
{
    isStarting_ = isStarting;
}

void Session::ResetDirtyFlags()
{
    if (!isVisible_) {
        dirtyFlags_ &= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
    } else {
        dirtyFlags_ = 0;
    }
}

void Session::SetUIStateDirty(bool dirty)
{
    mainUIStateDirty_.store(dirty);
}

bool Session::GetUIStateDirty() const
{
    return mainUIStateDirty_.load();
}

void Session::SetMainSessionUIStateDirty(bool dirty)
{
    if (GetParentSession() && WindowHelper::IsMainWindow(GetParentSession()->GetWindowType())) {
        GetParentSession()->SetUIStateDirty(dirty);
    }
}

bool Session::IsScbCoreEnabled()
{
    return isScbCoreEnabled_;
}

void Session::SetScbCoreEnabled(bool enabled)
{
    TLOGI(WmsLogTag::WMS_PIPELINE, "%{public}d", enabled);
    isScbCoreEnabled_ = enabled;
}

bool Session::IsBackgroundUpdateRectNotifyEnabled()
{
    return isBackgroundUpdateRectNotifyEnabled_;
}

void Session::SetBackgroundUpdateRectNotifyEnabled(const bool enabled)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "%{public}d", enabled);
    isBackgroundUpdateRectNotifyEnabled_ = enabled;
}

bool Session::IsVisible() const
{
    return isVisible_;
}

std::shared_ptr<AppExecFwk::EventHandler> Session::GetEventHandler() const
{
    return handler_;
}

std::shared_ptr<Media::PixelMap> Session::SetFreezeImmediately(float scale, bool isFreeze, float blur) const
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "GetSnapshotWithFreeze[%d][%s]",
        persistentId_, sessionInfo_.bundleName_.c_str());
    auto surfaceNode = GetSurfaceNode();
    if (!surfaceNode || !surfaceNode->IsBufferAvailable()) {
        return nullptr;
    }
    auto callback = std::make_shared<SurfaceCaptureFuture>();
    auto scaleValue = (!MathHelper::GreatNotEqual(scale, 0.0f) ||
        !MathHelper::GreatNotEqual(scale, std::numeric_limits<float>::min())) ? snapshotScale_ : scale;
    bool isNeedF16WindowShot = system::GetBoolParameter("persist.sys.graphic.scrgb.enabled", false);
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleValue,
        .scaleY = scaleValue,
        .useDma = true,
        .useCurWindow = true,
        .isHdrCapture = true,
        .needF16WindowCaptureForScRGB = isNeedF16WindowShot,
    };
    bool ret = RSInterfaces::GetInstance().SetWindowFreezeImmediately(surfaceNode, isFreeze, callback, config, blur);
    if (!ret) {
        TLOGE(WmsLogTag::WMS_PATTERN, "failed");
        return nullptr;
    }
    if (isFreeze) {
        auto pixelMap = callback->GetResult(SNAPSHOT_TIMEOUT_MS);
        TLOGI(WmsLogTag::WMS_PATTERN, "get result: %{public}d, id: %{public}d", pixelMap != nullptr, persistentId_);
        return pixelMap;
    }
    return nullptr;
}

bool Session::IsPcWindow() const
{
    return systemConfig_.IsPcWindow();
}

WindowUIInfo Session::GetWindowUIInfoForWindowInfo() const
{
    WindowUIInfo windowUIInfo;
    windowUIInfo.visibilityState = GetVisibilityState();
    return windowUIInfo;
}

WindowDisplayInfo Session::GetWindowDisplayInfoForWindowInfo() const
{
    WindowDisplayInfo windowDisplayInfo;
    if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(GetSessionProperty()->GetDisplayId())) {
        WSRect sessionGlobalRect = GetSessionGlobalRect();
        windowDisplayInfo.displayId = TransformGlobalRectToRelativeRect(sessionGlobalRect);
    } else {
        windowDisplayInfo.displayId = GetSessionProperty()->GetDisplayId();
    }
    return windowDisplayInfo;
}

WindowLayoutInfo Session::GetWindowLayoutInfoForWindowInfo() const
{
    WindowLayoutInfo windowLayoutInfo;
    WSRect sessionGlobalRect = GetSessionGlobalRect();
    sessionGlobalRect.width_ *= GetScaleX();
    sessionGlobalRect.height_ *= GetScaleY();
    if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(GetSessionProperty()->GetDisplayId())) {
        TransformGlobalRectToRelativeRect(sessionGlobalRect);
    }
    windowLayoutInfo.rect = { sessionGlobalRect.posX_, sessionGlobalRect.posY_,
                              sessionGlobalRect.width_, sessionGlobalRect.height_};
    windowLayoutInfo.zOrder = GetZOrder();
    return windowLayoutInfo;
}

WindowMetaInfo Session::GetWindowMetaInfoForWindowInfo() const
{
    WindowMetaInfo windowMetaInfo;
    windowMetaInfo.windowId = GetWindowId();
    if (GetSessionInfo().isSystem_) {
        windowMetaInfo.windowName = GetSessionInfo().abilityName_;
    } else {
        windowMetaInfo.windowName = GetSessionProperty()->GetWindowName();
    }
    windowMetaInfo.bundleName = GetSessionInfo().bundleName_;
    windowMetaInfo.abilityName = GetSessionInfo().abilityName_;
    windowMetaInfo.appIndex = GetSessionInfo().appIndex_;
    windowMetaInfo.pid = GetCallingPid();
    windowMetaInfo.windowType = GetWindowType();
    windowMetaInfo.windowMode = GetWindowMode();
    windowMetaInfo.isMidScene = GetIsMidScene();
    windowMetaInfo.isFocused = IsFocused();
    if (auto parentSession = GetParentSession()) {
        windowMetaInfo.parentWindowId = static_cast<uint32_t>(parentSession->GetWindowId());
    }
    if (auto surfaceNode = GetSurfaceNode()) {
        windowMetaInfo.surfaceNodeId = static_cast<uint64_t>(surfaceNode->GetId());
    }
    if (auto leashWinSurfaceNode = GetLeashWinSurfaceNode()) {
        windowMetaInfo.leashWinSurfaceNodeId = static_cast<uint64_t>(leashWinSurfaceNode->GetId());
    }
    auto property = GetSessionProperty();
    windowMetaInfo.isPrivacyMode = property->GetPrivacyMode() || property->GetSystemPrivacyMode();
    auto displayId = property->GetDisplayId();
    windowMetaInfo.isTouchable = GetWindowTouchableForMMI(displayId);
    TLOGD(WmsLogTag::WMS_EVENT, "id:%{public}d, %{public}d", windowMetaInfo.windowId,
        windowMetaInfo.isTouchable);
    return windowMetaInfo;
}

DisplayId Session::GetClientDisplayId() const
{
    return clientDisplayId_;
}

void Session::SetClientDisplayId(DisplayId displayid)
{
    clientDisplayId_ = displayid;
}

void Session::SetDragStart(bool isDragStart)
{
    isDragStart_ = isDragStart;
}

void Session::SetBorderUnoccupied(bool borderUnoccupied)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "borderUnoccupied: %{public}d", borderUnoccupied);
    borderUnoccupied_ = borderUnoccupied;
}

bool Session::GetBorderUnoccupied() const
{
    return borderUnoccupied_;
}

void Session::SetWindowAnimationDuration(int32_t duration)
{
    windowAnimationDuration_ = duration;
}

bool Session::IsNeedReportTimeout() const
{
    WindowType type = GetWindowType();
    return WindowHelper::IsSubWindow(type) || (WindowHelper::IsAboveSystemWindow(type) &&
        type != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT && type != WindowType::WINDOW_TYPE_PANEL);
}

void Session::PostSpecificSessionLifeCycleTimeoutTask(const std::string& eventName)
{
    const int32_t THRESHOLD = 20;
    if (!IsNeedReportTimeout()) {
        TLOGD(WmsLogTag::DEFAULT, "not specific window");
        return;
    }
    // if configured animation, don't report
    if (windowAnimationDuration_) {
        TLOGD(WmsLogTag::DEFAULT, "window configured animation, don't report");
        return;
    }
    if (!handler_) {
        TLOGE(WmsLogTag::DEFAULT, "handler is null");
        return;
    }
    handler_->RemoveTask(eventName == ATTACH_EVENT_NAME ?
        DETACH_EVENT_NAME : ATTACH_EVENT_NAME);
    auto task = [weakThis = wptr(this), eventName, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNI(WmsLogTag::DEFAULT, "%{public}s, session is null.", where);
            return;
        }
        bool isAttach = session->GetAttachState();
        if ((isAttach && eventName == ATTACH_EVENT_NAME) ||
            (!isAttach && eventName == DETACH_EVENT_NAME)) {
            TLOGND(WmsLogTag::DEFAULT, "%{public}s, detached or attached in time", where);
            return;
        }
        WindowLifeCycleReportInfo reportInfo {
            session->GetSessionInfo().bundleName_,
            static_cast<int32_t>(session->GetPersistentId()),
            static_cast<int32_t>(session->GetWindowType()),
            static_cast<int32_t>(session->GetWindowMode()),
            static_cast<int32_t>(session->GetSessionProperty()->GetWindowFlags()),
            eventName
        };
        TLOGNI(WmsLogTag::DEFAULT, "%{public}s report msg: %{public}s", where, reportInfo.ToString().c_str());
        WindowInfoReporter::GetInstance().ReportSpecWindowLifeCycleChange(reportInfo);
    };
    handler_->PostTask(task, "wms:" + eventName, THRESHOLD, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void Session::DeletePersistentImageFit()
{
    auto isPersistentImageFit = Rosen::ScenePersistentStorage::HasKey(
        "SetImageForRecent_" + std::to_string(GetPersistentId()), Rosen::ScenePersistentStorageType::MAXIMIZE_STATE);
    if (isPersistentImageFit) {
        TLOGI(WmsLogTag::WMS_PATTERN, "delete persistent ImageFit");
        isPersistentImageFit_.store(false);
        Rosen::ScenePersistentStorage::Delete("SetImageForRecent_" + std::to_string(GetPersistentId()),
            Rosen::ScenePersistentStorageType::MAXIMIZE_STATE);
    }
}

void Session::RecoverImageForRecent()
{
    auto isPersistentImageFit = ScenePersistentStorage::HasKey("SetImageForRecent_" +
        std::to_string(persistentId_), ScenePersistentStorageType::MAXIMIZE_STATE);
    if (isPersistentImageFit) {
        TLOGI(WmsLogTag::WMS_PATTERN, "%{public}d", persistentId_);
        isPersistentImageFit_.store(true);
        ScenePersistentStorage::Get("SetImageForRecent_" + std::to_string(persistentId_),
            persistentImageFit_, ScenePersistentStorageType::MAXIMIZE_STATE);
    }
}

void Session::SetPersistentImageFit(int32_t imageFit)
{
    persistentImageFit_ = imageFit;
    isPersistentImageFit_.store(true);
}

void Session::SetGlobalDisplayRect(const WSRect& rect)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, rect: %{public}s", GetPersistentId(), rect.ToString().c_str());
    GetSessionProperty()->SetGlobalDisplayRect(SessionHelper::TransferToRect(rect));
}

WSRect Session::GetGlobalDisplayRect() const
{
    WSRect rect = SessionHelper::TransferToWSRect(GetSessionProperty()->GetGlobalDisplayRect());
    TLOGD(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, rect: %{public}s", GetPersistentId(), rect.ToString().c_str());
    return rect;
}

WSError Session::UpdateGlobalDisplayRect(const WSRect& rect, SizeChangeReason reason)
{
    const int32_t windowId = GetWindowId();
    TLOGD(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, rect: %{public}s, reason: %{public}u",
        windowId, rect.ToString().c_str(), reason);
    if (rect == GetGlobalDisplayRect() && reason == globalDisplayRectSizeChangeReason_) {
        TLOGD(WmsLogTag::WMS_LAYOUT,
            "No change in rect or reason, windowId: %{public}d, rect: %{public}s, reason: %{public}u",
            windowId, rect.ToString().c_str(), reason);
        return WSError::WS_DO_NOTHING;
    }
    SetGlobalDisplayRect(rect);
    globalDisplayRectSizeChangeReason_ = reason;
    NotifyClientToUpdateGlobalDisplayRect(rect, reason);
    return WSError::WS_OK;
}

WSError Session::NotifyClientToUpdateGlobalDisplayRect(const WSRect& rect, SizeChangeReason reason)
{
    // Skip notifying client when the window is not in the foreground to avoid waking up
    // the application via IPC, which may cause unnecessary power consumption.
    if (!sessionStage_ || !IsSessionForeground()) {
        return WSError::WS_DO_NOTHING;
    }
    return sessionStage_->UpdateGlobalDisplayRectFromServer(rect, reason);
}

void Session::SetOutlineParamsChangeCallback(OutlineParamsChangeCallbackFunc&& func)
{
    PostTask([weakThis = wptr(this), where = __func__, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "session or onOutlineParamsChangeCallback is null.");
            return;
        }
        session->outlineParamsChangeCallback_ = std::move(func);
        session->outlineParamsChangeCallback_(session->isOutlineEnabled_, session->outlineStyleParams_);
    }, __func__);
}

void Session::UpdateSessionOutline(bool enabled, const OutlineStyleParams& params)
{
    if (enabled == isOutlineEnabled_ && params == outlineStyleParams_) {
        TLOGD(WmsLogTag::WMS_ANIMATION, "Same outline params.");
        return;
    }
    TLOGI(WmsLogTag::WMS_ANIMATION, "id: %{public}d, oldEnabled: %{public}d, oldParams: %{public}s, "
          "newEnabled: %{public}d, newParams: %{public}s", GetPersistentId(), isOutlineEnabled_,
          outlineStyleParams_.ToString().c_str(), enabled, params.ToString().c_str());
    isOutlineEnabled_ = enabled;
    outlineStyleParams_.outlineColor_ = params.outlineColor_;
    outlineStyleParams_.outlineWidth_ = params.outlineWidth_;
    outlineStyleParams_.outlineShape_ = params.outlineShape_;
    if (outlineParamsChangeCallback_) {
        outlineParamsChangeCallback_(isOutlineEnabled_, outlineStyleParams_);
    } else {
        TLOGI(WmsLogTag::WMS_ANIMATION, "Outline params change callback is null.");
    }
}

std::shared_ptr<RSUIContext> Session::GetRSUIContext(const char* caller)
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    auto screenId = GetScreenId();
    if (screenIdOfRSUIContext_ != screenId) {
        // Note: For the window corresponding to UIExtAbility, RSUIContext cannot be obtained
        // directly here because its server side is not SceneBoard. The acquisition of RSUIContext
        // is deferred to the UIExtensionPattern::OnConnect(ui_extension_pattern.cpp) method,
        // as ArkUI knows the host window for this type of window.
        rsUIContext_ = ScreenSessionManagerClient::GetInstance().GetRSUIContext(screenId);
        screenIdOfRSUIContext_ = screenId;
    }
    if (rsUIContext_ == nullptr) {
        TLOGI(WmsLogTag::WMS_SCB, "%{public}s: %{public}s, sessionId: %{public}d, screenId:%{public}" PRIu64,
            caller, RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str(), GetPersistentId(), screenId);
    }
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s: %{public}s, sessionId: %{public}d, screenId:%{public}" PRIu64,
            caller, RSAdapterUtil::RSUIContextToStr(rsUIContext_).c_str(), GetPersistentId(), screenId);
    return rsUIContext_;
}

std::shared_ptr<RSUIContext> Session::GetRSShadowContext()
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        if (!shadowSurfaceNode_) {
            TLOGE(WmsLogTag::WMS_SCB, "Shadow surface node is nullptr, id: %{public}d.", GetPersistentId());
            return nullptr;
        }
        return shadowSurfaceNode_->GetRSUIContext();
    }
    return GetRSUIContext();
}

std::shared_ptr<RSUIContext> Session::GetRSLeashWinShadowContext()
{
    std::lock_guard<std::mutex> lock(leashWinSurfaceNodeMutex_);
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        if (!leashWinShadowSurfaceNode_) {
            TLOGE(WmsLogTag::WMS_SCB, "Leash win shadow surface node is nullptr, id: %{public}d.", GetPersistentId());
            return nullptr;
        }
        return leashWinShadowSurfaceNode_->GetRSUIContext();
    }
    return GetRSUIContext();
}

WSError Session::SetIsShowDecorInFreeMultiWindow(bool isShow)
{
    if (!IsSessionValid()) {
        TLOGE(WmsLogTag::WMS_DECOR, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_DECOR, "sessionStage_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (WindowHelper::IsMainWindow(GetWindowType())) {
        TLOGI(WmsLogTag::WMS_DECOR, "id: %{public}d isShow: %{public}d",
            GetPersistentId(), isShow);
        return sessionStage_->UpdateIsShowDecorInFreeMultiWindow(isShow);
    }
    return WSError::WS_OK;
}

void Session::SetPrelaunch()
{
    prelaunchStart_ = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    TLOGI(WmsLogTag::WMS_LIFE, "SetPrelaunch timestamp: %{public}" PRIu64, prelaunchStart_);
    sessionInfo_.isPrelaunch_ = true;
}

bool Session::IsPrelaunch() const
{
    uint64_t nowTimeStamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    if (sessionInfo_.isPrelaunch_) {
        TLOGI(WmsLogTag::WMS_LIFE, "IsPrelaunch now - start: %{public}" PRIu64, nowTimeStamp - prelaunchStart_);
    }
    return sessionInfo_.isPrelaunch_ && nowTimeStamp - prelaunchStart_ > PRELAUNCH_DONE_TIME;
}
} // namespace OHOS::Rosen