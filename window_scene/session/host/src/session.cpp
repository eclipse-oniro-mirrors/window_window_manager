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

#include <regex>

#include "ability_info.h"
#include "input_manager.h"
#include "key_event.h"
#include "pointer_event.h"
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>
#include "proxy/include/window_info.h"

#include "common/include/session_permission.h"
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
constexpr int64_t STATE_DETECT_DELAYTIME = 3 * 1000;
constexpr DisplayId VIRTUAL_DISPLAY_ID = 999;
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
} // namespace

std::shared_ptr<AppExecFwk::EventHandler> Session::mainHandler_;
bool Session::isScbCoreEnabled_ = false;
bool Session::isBackgroundUpdateRectNotifyEnabled_ = false;

Session::Session(const SessionInfo& info) : sessionInfo_(info)
{
    property_ = sptr<WindowSessionProperty>::MakeSptr();
    property_->SetWindowType(static_cast<WindowType>(info.windowType_));

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
        TLOGI(WmsLogTag::WMS_FOCUS, "focusedOnShow:%{public}d", focusedOnShow);
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
        mainHandler_->PostTask([surfaceNode = std::move(surfaceNode_)]() {
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

void Session::SetSurfaceNode(const std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    surfaceNode_ = surfaceNode;
}

std::shared_ptr<RSSurfaceNode> Session::GetSurfaceNode() const
{
    std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
    return surfaceNode_;
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
    if (g_enableForceUIFirst) {
        auto rsTransaction = RSTransactionProxy::GetInstance();
        if (rsTransaction) {
            rsTransaction->Begin();
        }
        if (!leashWinSurfaceNode && leashWinSurfaceNode_) {
            leashWinSurfaceNode_->SetForceUIFirst(false);
        }
        if (rsTransaction) {
            rsTransaction->Commit();
        }
    }
    std::lock_guard<std::mutex> lock(leashWinSurfaceNodeMutex_);
    leashWinSurfaceNode_ = leashWinSurfaceNode;
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
    sessionInfo_.callerPersistentId_ = info.callerPersistentId_;
    sessionInfo_.callingTokenId_ = info.callingTokenId_;
    sessionInfo_.uiAbilityId_ = info.uiAbilityId_;
    sessionInfo_.requestId = info.requestId;
    sessionInfo_.startSetting = info.startSetting;
    sessionInfo_.continueSessionId_ = info.continueSessionId_;
    sessionInfo_.isAtomicService_ = info.isAtomicService_;
    sessionInfo_.callState_ = info.callState_;
    sessionInfo_.processOptions = info.processOptions;
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
}

void Session::SetAppInstanceKey(const std::string& appInstanceKey)
{
    sessionInfo_.appInstanceKey_ = appInstanceKey;
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

void Session::NotifyAddSnapshot()
{
    /*
     * for blankness prolems, persist snapshot could conflict with background process,
     * thus no need to persist snapshot here
     */
    SaveSnapshot(false, false);
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnAddSnapshot();
        }
    }
}

void Session::NotifyRemoveSnapshot()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnRemoveSnapshot();
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
    return aspectRatio_;
}

WSError Session::SetAspectRatio(float ratio)
{
    aspectRatio_ = ratio;
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
    /* The state will be set background first when destroy keyboard, there is no need to notify scb if the state is
     * already background, which may cause performance deterioration.
     */
    if (GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT && state == state_ &&
        state == SessionState::STATE_BACKGROUND) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Keyboard is already hide");
        return;
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
    WLOGFI("SetFocusable id: %{public}d, focusable: %{public}d", GetPersistentId(), isFocusable);
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
    TLOGI(WmsLogTag::WMS_FOCUS, "SetFocusedOnShow:%{public}d, id: %{public}d", focusedOnShow, GetPersistentId());
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
        TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d, %{public}d", GetPersistentId(), touchable);
    }
    systemTouchable_ = touchable;
    NotifySessionInfoChange();
}

bool Session::GetSystemTouchable() const
{
    return forceTouchable_ && systemTouchable_ && GetTouchable();
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
    if (reason_ == reason) {
        return WSError::WS_DO_NOTHING;
    }
    reason_ = reason;
    return WSError::WS_OK;
}

WSError Session::UpdateClientDisplayId(DisplayId displayId)
{
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "sessionStage_ is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d move display %{public}" PRIu64 " from %{public}" PRIu64,
          GetPersistentId(), displayId, clientDisplayId_);
    clientDisplayId_ = displayId;
    sessionStage_->UpdateDisplayId(displayId);
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

void Session::UpdateClientRectPosYAndDisplayId(WSRect& rect)
{
    if (GetSessionProperty()->IsSystemKeyboard()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "skip update SystemKeyboard: %{public}d", GetPersistentId());
        return;
    }
    if (WindowHelper::IsUIExtensionWindow(GetWindowType())) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "skip update UIExtension: %{public}d, rect: %{public}s",
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
        winRect_ = rect;
        TLOGD(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    winRect_ = rect;
    if (!Session::IsBackgroundUpdateRectNotifyEnabled() && !IsSessionForeground()) {
        return WSError::WS_DO_NOTHING;
    }
    if (sessionStage_ != nullptr) {
        int32_t rotateAnimationDuration = GetRotateAnimationDuration();
        SceneAnimationConfig config { .rsTransaction_ = rsTransaction, .animationDuration_ = rotateAnimationDuration };
        WSRect updatedRect = rect;
        UpdateClientRectPosYAndDisplayId(updatedRect);
        sessionStage_->UpdateRect(updatedRect, reason, config, avoidAreas);
        SetClientRect(rect);
        RectCheckProcess();
    } else {
        WLOGFE("sessionStage_ is nullptr");
    }
    UpdatePointerArea(winRect_);
    return WSError::WS_OK;
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
    TLOGD(WmsLogTag::DMS, "update orientation: id: %{public}d.", GetPersistentId());
    if (!IsSessionValid()) {
        TLOGE(WmsLogTag::DMS, "update orientation failed because of session is invalid, id=%{public}d.",
            GetPersistentId());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "update orientation failed because of sessionStage_ is nullptr, id=%{public}d.",
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
    TLOGI(WmsLogTag::WMS_LIFE, "ConnectInner session, id: %{public}d, state: %{public}u,"
        "isTerminating:%{public}d, callingPid:%{public}d", GetPersistentId(),
        static_cast<uint32_t>(GetSessionState()), isTerminating_, pid);
    if (GetSessionState() != SessionState::STATE_DISCONNECT && !isTerminating_) {
        TLOGE(WmsLogTag::WMS_LIFE, "state is not disconnect state:%{public}u id:%{public}u!",
            GetSessionState(), GetPersistentId());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage == nullptr || eventChannel == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session stage or eventChannel is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_ = sessionStage;
    windowEventChannel_ = eventChannel;
    SetSurfaceNode(surfaceNode);
    abilityToken_ = token;
    systemConfig = systemConfig_;
    InitSessionPropertyWhenConnect(property);
    SetCallingPid(pid);
    callingUid_ = uid;
    UpdateSessionState(SessionState::STATE_CONNECT);
    WindowHelper::IsUIExtensionWindow(GetWindowType()) ? UpdateRect(winRect_, SizeChangeReason::UNDEFINED, "Connect") :
        NotifyClientToUpdateRect("Connect", nullptr);
    NotifyConnect();
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
    InitSystemSessionDragEnable(property);
    property->SetSessionPropertyChangeCallback(
        [weakThis = wptr(this)]() {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNE(WmsLogTag::DEFAULT, "session is null");
                return;
            }
            session->NotifySessionInfoChange();
        });

    Rect rect = {winRect_.posX_, winRect_.posY_, static_cast<uint32_t>(winRect_.width_),
        static_cast<uint32_t>(winRect_.height_)};
    property->SetWindowRect(rect);
    property->SetPersistentId(GetPersistentId());
    property->SetFullScreenStart(GetSessionInfo().fullScreenStart_);
    property->SetSupportedWindowModes(GetSessionInfo().supportedWindowModes);
    property->SetWindowSizeLimits(GetSessionInfo().windowSizeLimits);
    property->SetIsAtomicService(GetSessionInfo().isAtomicService_);
    property->SetRequestedOrientation(GetSessionProperty()->GetRequestedOrientation());
    property->SetDefaultRequestedOrientation(GetSessionProperty()->GetDefaultRequestedOrientation());
    TLOGI(WmsLogTag::WMS_MAIN, "Id: %{public}d, requestedOrientation: %{public}u,"
        " defaultRequestedOrientation: %{public}u", GetPersistentId(),
        static_cast<uint32_t>(GetSessionProperty()->GetRequestedOrientation()),
        static_cast<uint32_t>(GetSessionProperty()->GetDefaultRequestedOrientation()));
    property->SetCompatibleModeInPc(GetSessionProperty()->GetCompatibleModeInPc());
    property->SetIsSupportDragInPcCompatibleMode(GetSessionProperty()->GetIsSupportDragInPcCompatibleMode());
    if (GetSessionProperty()->GetCompatibleModeInPc()) {
        property->SetDragEnabled(GetSessionProperty()->GetIsSupportDragInPcCompatibleMode());
    }
    property->SetCompatibleModeEnableInPad(GetSessionProperty()->GetCompatibleModeEnableInPad());
    property->SetCompatibleWindowSizeInPc(GetSessionProperty()->GetCompatibleInPcPortraitWidth(),
        GetSessionProperty()->GetCompatibleInPcPortraitHeight(),
        GetSessionProperty()->GetCompatibleInPcLandscapeWidth(),
        GetSessionProperty()->GetCompatibleInPcLandscapeHeight());
    property->SetIsAppSupportPhoneInPc(GetSessionProperty()->GetIsAppSupportPhoneInPc());
    std::optional<bool> clientDragEnable = GetClientDragEnable();
    if (clientDragEnable.has_value()) {
        property->SetDragEnabled(clientDragEnable.value());
    }
    if (SessionHelper::IsMainWindow(GetWindowType())) {
        property->SetIsPcAppInPad(GetSessionProperty()->GetIsPcAppInPad());
    }
    SetSessionProperty(property);
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
    TLOGI(WmsLogTag::WMS_RECOVER, "id:%{public}d, state:%{public}u, pid:%{public}d",
        property->GetPersistentId(), static_cast<uint32_t>(property->GetWindowState()), pid);
    if (sessionStage == nullptr || eventChannel == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "session stage or eventChannel is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_ = sessionStage;
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
    return WSError::WS_OK;
}

WSError Session::Foreground(sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken)
{
    HandleDialogForeground();
    SessionState state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, state:%{public}u, isTerminating:%{public}d",
        GetPersistentId(), static_cast<uint32_t>(state), isTerminating_);
    if (state != SessionState::STATE_CONNECT && state != SessionState::STATE_BACKGROUND &&
        state != SessionState::STATE_INACTIVE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Foreground state invalid! state:%{public}u", state);
        return WSError::WS_ERROR_INVALID_SESSION;
    }

    UpdateSessionState(SessionState::STATE_FOREGROUND);
    if (!isActive_) {
        SetActive(true);
    }
    isStarting_ = false;

    NotifyForeground();

    isTerminating_ = false;
    isNeedSyncSessionRect_ = true;
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
    TLOGI(WmsLogTag::WMS_LIFE, "Background session, id: %{public}d, state: %{public}" PRIu32, GetPersistentId(),
        static_cast<uint32_t>(state));
    if (state == SessionState::STATE_ACTIVE && GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        UpdateSessionState(SessionState::STATE_INACTIVE);
        state = SessionState::STATE_INACTIVE;
        isActive_ = false;
    }
    isStarting_ = false;
    isStartingBeforeVisible_ = false;
    if (state != SessionState::STATE_INACTIVE) {
        TLOGW(WmsLogTag::WMS_LIFE, "Background state invalid! id: %{public}d, state: %{public}u",
            GetPersistentId(), state);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    UpdateSessionState(SessionState::STATE_BACKGROUND);
    SetIsPendingToBackgroundState(false);
    NotifyBackground();
    return WSError::WS_OK;
}

void Session::ResetSessionConnectState()
{
    TLOGI(WmsLogTag::WMS_LIFE, "ResetSessionState, id: %{public}d, state: %{public}u",
        GetPersistentId(), GetSessionState());
    SetSessionState(SessionState::STATE_DISCONNECT);
    SetCallingPid(-1);
}

void Session::ResetIsActive()
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d, isActive: %{public}u",
        GetPersistentId(), IsActive());
    isActive_ = false;
}

WSError Session::Disconnect(bool isFromClient, const std::string& identityToken)
{
    auto state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "Disconnect session, id: %{public}d, state: %{public}u", GetPersistentId(), state);
    isActive_ = false;
    isStarting_ = false;
    isStartingBeforeVisible_ = false;
    bufferAvailable_ = false;
    isNeedSyncSessionRect_ = true;
    if (mainHandler_) {
        std::shared_ptr<RSSurfaceNode> surfaceNode;
        {
            std::lock_guard<std::mutex> lock(surfaceNodeMutex_);
            surfaceNode_.swap(surfaceNode);
        }
        mainHandler_->PostTask([surfaceNode = std::move(surfaceNode)]() mutable {
            surfaceNode.reset();
        });
    }
    UpdateSessionState(SessionState::STATE_BACKGROUND);
    UpdateSessionState(SessionState::STATE_DISCONNECT);
    NotifyDisconnect();
    if (visibilityChangedDetectFunc_) {
        visibilityChangedDetectFunc_(GetCallingPid(), isVisible_, false);
    }
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
    if (active == isActive_) {
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
    if (clickModalWindowOutsideFunc_ && !winRect_.IsInRegion(posX, posY)) {
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
    PostTask([weakThis = wptr(this), isAttach]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGND(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        if (session->needNotifyAttachState_.load() && session->sessionStage_) {
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

void Session::SetNeedNotifyAttachState(bool needNotify)
{
    needNotifyAttachState_.store(needNotify);
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
    }
    StartLifeCycleTask(frontLifeCycleTask);
}

void Session::PostLifeCycleTask(Task&& task, const std::string& name, const LifeCycleTaskType& taskType)
{
    sptr<SessionLifeCycleTask> frontlifeCycleTask = nullptr;
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
        frontlifeCycleTask = lifeCycleTaskQueue_.front();
    }
    StartLifeCycleTask(frontlifeCycleTask);
}

void Session::StartLifeCycleTask(sptr<SessionLifeCycleTask> lifeCycleTask)
{
    if (lifeCycleTask->running) {
        return;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Execute LifeCycleTask %{public}s. PersistentId: %{public}d",
        lifeCycleTask->name.c_str(), persistentId_);
    lifeCycleTask->running = true;
    lifeCycleTask->startTime = std::chrono::steady_clock::now();
    PostTask(std::move(lifeCycleTask->task), lifeCycleTask->name);
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
            session->terminateSessionFuncNew_(info, needStartCaller, isFromBroker);
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
    return WSError::WS_OK;
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

WSError Session::Clear(bool needStartCaller)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, needStartCaller:%{public}u", GetPersistentId(), needStartCaller);
    auto task = [weakThis = wptr(this), needStartCaller]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->isTerminating_ = true;
        if (session->terminateSessionFuncNew_) {
            session->terminateSessionFuncNew_(session->GetSessionInfo(), needStartCaller, false);
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

bool Session::IsAncestorsSession(int ancestorsId) const
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
    WLOGFI("id: %{public}d,type: %{public}d", GetPersistentId(), GetWindowType());
    if (!isFocused_ && GetFocusable()) {
        FocusChangeReason reason = FocusChangeReason::CLICK;
        NotifyRequestFocusStatusNotifyManager(true, false, reason);
    }
    NotifyClick();
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

WSError Session::HandleSubWindowClick(int32_t action, bool isExecuteDelayRaise)
{
    auto parentSession = GetParentSession();
    if (parentSession && parentSession->CheckDialogOnForeground()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Its main window has dialog on foreground, id: %{public}d", GetPersistentId());
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    bool raiseEnabled = GetSessionProperty()->GetRaiseEnabled();
    bool isPointDown = action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN;
    if (isExecuteDelayRaise) {
        if (raiseEnabled && action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
            RaiseToAppTopForPointDown();
        }
        if (!raiseEnabled && parentSession) {
            parentSession->NotifyClick(!IsScbCoreEnabled());
        }
        return WSError::WS_OK;
    }
    if (raiseEnabled && isPointDown) {
        RaiseToAppTopForPointDown();
    } else if (parentSession) {
        // sub window is forbidden to raise to top after click, but its parent should raise
        parentSession->NotifyClick(!IsScbCoreEnabled());
    }
    return WSError::WS_OK;
}

WSError Session::TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool needNotifyClient, bool isExecuteDelayRaise)
{
    WLOGFD("Session TransferPointEvent, id: %{public}d", GetPersistentId());
    if (!IsSystemSession() && !IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (pointerEvent == nullptr) {
        WLOGFE("PointerEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto pointerAction = pointerEvent->GetPointerAction();
    bool isPointDown = (pointerAction == MMI::PointerEvent::POINTER_ACTION_DOWN) ||
        (pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    if (GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        if (CheckDialogOnForeground() && isPointDown) {
            HandlePointDownDialog();
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
    } else if (GetWindowType() == WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
        WSError ret = HandleSubWindowClick(pointerAction, isExecuteDelayRaise);
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
        PresentFoucusIfNeed(pointerAction);
    } else if (pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
        if (!isFocused_ && GetFocusable()) {
            NotifyRequestFocusStatusNotifyManager(true, false, FocusChangeReason::CLICK);
        }
        NotifyClick();
    }
    if (!windowEventChannel_) {
        if (!IsSystemSession()) {
            WLOGFE("windowEventChannel_ is null");
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
        WLOGFD("Action:%{public}s, eventId:%{public}d, report without timer",
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
    if (scenePersistence_ == nullptr) {
        return nullptr;
    }
    auto surfaceNode = GetSurfaceNode();
    if (!surfaceNode || !surfaceNode->IsBufferAvailable()) {
        scenePersistence_->SetHasSnapshot(false);
        return nullptr;
    }
    scenePersistence_->SetHasSnapshot(true);
    auto callback = std::make_shared<SurfaceCaptureFuture>();
    auto scaleValue = (scaleParam < 0.0f || std::fabs(scaleParam) < std::numeric_limits<float>::min()) ?
        snapshotScale_ : scaleParam;
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleValue,
        .scaleY = scaleValue,
        .useDma = true,
        .useCurWindow = useCurWindow,
    };
    bool ret = RSInterfaces::GetInstance().TakeSurfaceCapture(surfaceNode, callback, config);
    if (!ret) {
        TLOGE(WmsLogTag::WMS_MAIN, "TakeSurfaceCapture failed");
        return nullptr;
    }
    constexpr int32_t FFRT_SNAPSHOT_TIMEOUT_MS = 5000;
    auto pixelMap = callback->GetResult(runInFfrt ? FFRT_SNAPSHOT_TIMEOUT_MS : SNAPSHOT_TIMEOUT_MS);
    if (pixelMap != nullptr) {
        TLOGI(WmsLogTag::WMS_MAIN, "Save snapshot WxH=%{public}dx%{public}d, id: %{public}d",
            pixelMap->GetWidth(), pixelMap->GetHeight(), persistentId_);
        if (notifySessionSnapshotFunc_) {
            notifySessionSnapshotFunc_(persistentId_);
        }
        return pixelMap;
    }
    TLOGE(WmsLogTag::WMS_MAIN, "Save snapshot failed, id: %{public}d", persistentId_);
    return nullptr;
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

void Session::SaveSnapshot(bool useFfrt, bool needPersist)
{
    if (scenePersistence_ == nullptr) {
        return;
    }
    auto task = [weakThis = wptr(this), runInFfrt = useFfrt, requirePersist = needPersist]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->lastLayoutRect_ = session->layoutRect_;
        auto pixelMap = session->Snapshot(runInFfrt);
        if (pixelMap == nullptr) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(session->snapshotMutex_);
            session->snapshot_ = pixelMap;
        }
        {
            std::lock_guard lock(session->saveSnapshotCallbackMutex_);
            session->saveSnapshotCallback_();
        }
        if (!requirePersist) {
            return;
        }
        if (!session->scenePersistence_) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "scenePersistence_ is null");
            return;
        }
        Task removeSnapshotCallback = []() {};
        {
            std::lock_guard lock(session->removeSnapshotCallbackMutex_);
            removeSnapshotCallback = session->removeSnapshotCallback_;
        }
        session->scenePersistence_->SaveSnapshot(pixelMap, removeSnapshotCallback);
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

void Session::SetBufferAvailableChangeListener(const NotifyBufferAvailableChangeFunc& func)
{
    bufferAvailableChangeFunc_ = func;
    if (bufferAvailable_ && bufferAvailableChangeFunc_ != nullptr) {
        bufferAvailableChangeFunc_(bufferAvailable_);
    }
    WLOGFD("SetBufferAvailableChangeListener, id: %{public}d", GetPersistentId());
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
    updateSessionLabelAndIconFunc_ = nullptr;
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
            session->keyboardStateChangeFunc_(state, session->GetSessionProperty()->GetKeyboardViewMode());
        } else if (session->sessionStateChangeFunc_) {
            session->sessionStateChangeFunc_(state);
        } else {
            TLOGNI(WmsLogTag::WMS_LIFE, "sessionStateChangeFunc is null");
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
    TLOGD(WmsLogTag::WMS_FOCUS, "requestFocus: %{public}u, isClick: %{public}u", requestFocus, isClick);
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

void Session::PresentFoucusIfNeed(int32_t pointerAction)
{
    WLOGFD("OnClick down, id: %{public}d", GetPersistentId());
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        if (!isFocused_ && GetFocusable()) {
            FocusChangeReason reason = FocusChangeReason::CLICK;
            NotifyRequestFocusStatusNotifyManager(true, false, reason);
        }
        NotifyClick();
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

WSError Session::NotifyFocusStatus(bool isFocused)
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
    sessionStage_->UpdateFocus(isFocused);

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

WSError Session::UpdateHighlightStatus(bool isHighlight, bool needBlockHighlightNotify)
{
    TLOGD(WmsLogTag::WMS_FOCUS,
        "windowId: %{public}d, currHighlight: %{public}d, nextHighlight: %{public}d, needBlockNotify:%{public}d",
        persistentId_, isHighlight_, isHighlight, needBlockHighlightNotify);
    if (isHighlight_ == isHighlight) {
        return WSError::WS_DO_NOTHING;
    }
    isHighlight_ = isHighlight;
    if (needBlockHighlightNotify) {
        NotifyHighlightChange(isHighlight);
    }
    std::lock_guard lock(highlightChangeFuncMutex_);
    if (highlightChangeFunc_ != nullptr) {
        highlightChangeFunc_(isHighlight);
    }
    return WSError::WS_OK;
}

WSError Session::NotifyHighlightChange(bool isHighlight)
{
    if (IsSystemSession()) {
        TLOGW(WmsLogTag::WMS_FOCUS, "Session is invalid, id: %{public}d state: %{public}u",
            persistentId_, GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    sessionStage_->NotifyHighlightChange(isHighlight);
    return WSError::WS_OK;
}

WSError Session::SetCompatibleModeInPc(bool enable, bool isSupportDragInPcCompatibleMode)
{
    TLOGI(WmsLogTag::WMS_SCB, "SetCompatibleModeInPc enable: %{public}d, isSupportDragInPcCompatibleMode: %{public}d",
        enable, isSupportDragInPcCompatibleMode);
    GetSessionProperty()->SetCompatibleModeInPc(enable);
    GetSessionProperty()->SetIsSupportDragInPcCompatibleMode(isSupportDragInPcCompatibleMode);
    if (enable) {
        GetSessionProperty()->SetDragEnabled(isSupportDragInPcCompatibleMode);
    }
    return WSError::WS_OK;
}

WSError Session::SetCompatibleModeEnableInPad(bool enable)
{
    TLOGI(WmsLogTag::WMS_SCB, "id: %{public}d, enable: %{public}d", persistentId_, enable);
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_SCB, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    GetSessionProperty()->SetCompatibleModeEnableInPad(enable);

    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_SCB, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyCompatibleModeEnableInPad(enable);
}

WSError Session::SetAppSupportPhoneInPc(bool isSupportPhone)
{
    TLOGI(WmsLogTag::WMS_SCB, "isSupportPhone: %{public}d", isSupportPhone);
    GetSessionProperty()->SetIsAppSupportPhoneInPc(isSupportPhone);
    return WSError::WS_OK;
}

WSError Session::SetCompatibleWindowSizeInPc(int32_t portraitWidth, int32_t portraitHeight,
    int32_t landscapeWidth, int32_t landscapeHeight)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "compatible size: [%{public}d, %{public}d, %{public}d, %{public}d]",
        portraitWidth, portraitHeight, landscapeWidth, landscapeHeight);
    GetSessionProperty()->SetCompatibleWindowSizeInPc(portraitWidth, portraitHeight, landscapeWidth, landscapeHeight);
    return WSError::WS_OK;
}

WSError Session::SetIsPcAppInPad(bool enable)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "SetIsPcAppInPad enable: %{public}d", enable);
    GetSessionProperty()->SetIsPcAppInPad(enable);
    return WSError::WS_OK;
}

WSError Session::CompatibleFullScreenRecover()
{
    TLOGD(WmsLogTag::WMS_COMPAT, "recover compatible full screen windowId:%{public}d", GetPersistentId());
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
    return sessionStage_->CompatibleFullScreenRecover();
}

WSError Session::CompatibleFullScreenMinimize()
{
    TLOGD(WmsLogTag::WMS_MAIN, "minimize compatible full screen windowId:%{public}d", GetPersistentId());
    if (!IsSessionValid()) {
        TLOGD(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "session stage is nullptr id: %{public}d state: %{public}u",
              GetPersistentId(), GetSessionState());
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->CompatibleFullScreenMinimize();
}

WSError Session::CompatibleFullScreenClose()
{
    TLOGD(WmsLogTag::WMS_COMPAT, "close compatible full screen windowId:%{public}d", GetPersistentId());
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
    return sessionStage_->CompatibleFullScreenClose();
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
        GetSessionProperty()->SetIsNeedUpdateWindowMode(true);
        UpdateGestureBackEnabled();
    } else {
        GetSessionProperty()->SetWindowMode(mode);
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
        if ((ratio != 0) && !NearEqual(ratio, actRatio)) {
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
    if (winRect_ == rect) {
        WLOGFW("id: %{public}d skip same rect", persistentId_);
        return;
    }
    winRect_ = rect;
    dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    RectCheckProcess();
}

/** @note @window.layout */
WSRect Session::GetSessionRect() const
{
    return winRect_;
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
        WSRect scaledRect = session->GetSessionGlobalRect();
        scaledRect.width_ *= session->scaleX_;
        scaledRect.height_ *= session->scaleY_;
        globalScaledRect = { scaledRect.posX_, scaledRect.posY_, scaledRect.width_, scaledRect.height_ };
        TLOGNI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d globalScaledRect:%{public}s",
            session->GetPersistentId(), globalScaledRect.ToString().c_str());
        return WMError::WM_OK;
    };
    return PostSyncTask(task, __func__);
}

/** @note @window.layout */
WSRect Session::GetSessionGlobalRect() const
{
    if (IsScbCoreEnabled()) {
        std::lock_guard<std::mutex> lock(globalRectMutex_);
        return globalRect_;
    }
    return winRect_;
}

/** @note @window.layout */
void Session::SetSessionGlobalRect(const WSRect& rect)
{
    std::lock_guard<std::mutex> lock(globalRectMutex_);
    if (globalRect_ != rect) {
        dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::GLOBAL_RECT);
    }
    globalRect_ = rect;
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
    clientRect_ = rect;
    TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, update client rect:%{public}s",
        GetPersistentId(), rect.ToString().c_str());
}

/** @note @window.layout */
WSRect Session::GetClientRect() const
{
    return clientRect_;
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
    bool isPhoneOrPadNotFreeMultiWindow =
        systemConfig_.IsPhoneWindow() || (systemConfig_.IsPadWindow() && !systemConfig_.IsFreeMultiWindowMode());
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

void Session::NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
                                           const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (!sessionStage_) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "session stage is nullptr");
        return;
    }
    if (CheckEmptyKeyboardAvoidAreaIfNeeded()) {
        info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Occupied area needs to be empty when in floating mode");
    }
    sessionStage_->NotifyOccupiedAreaChangeInfo(info, rsTransaction);
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

void Session::SetBufferAvailable(bool bufferAvailable)
{
    TLOGD(WmsLogTag::DEFAULT, "Set:%{public}d", bufferAvailable);
    if (bufferAvailableChangeFunc_) {
        bufferAvailableChangeFunc_(bufferAvailable);
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
    scaleX_ = scaleX;
    scaleY_ = scaleY;
    pivotX_ = pivotX;
    pivotY_ = pivotY;
}

void Session::SetClientScale(float scaleX, float scaleY, float pivotX, float pivotY)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, preScaleX:%{public}f, preScaleY:%{public}f, "
        "newScaleX:%{public}f, newScaleY:%{public}f", GetPersistentId(), clientScaleX_, clientScaleY_, scaleX, scaleY);
    clientScaleX_ = scaleX;
    clientScaleY_ = scaleY;
    clientPivotX_ = pivotX;
    clientPivotY_ = pivotY;
}

float Session::GetScaleX() const
{
    return scaleX_;
}

float Session::GetScaleY() const
{
    return scaleY_;
}

float Session::GetPivotX() const
{
    return pivotX_;
}

float Session::GetPivotY() const
{
    return pivotY_;
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
    if (newRect != winRect_) {
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

WSError Session::SwitchFreeMultiWindow(bool enable)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "windowId:%{public}d enable: %{public}d", GetPersistentId(), enable);
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
        }
    }, "SetIsMidScene");
}

bool Session::GetIsMidScene() const
{
    return isMidScene_;
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
    return scenePersistence_->IsSavingSnapshot() ? GetSnapshot() :
        scenePersistence_->GetLocalSnapshotPixelMap(oriScale, newScale);
}

bool Session::IsVisibleForeground() const
{
    return isVisible_ && IsSessionForeground();
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
    RSSurfaceCaptureConfig config = {
        .scaleX = scaleValue,
        .scaleY = scaleValue,
        .useDma = true,
        .useCurWindow = true,
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
        windowDisplayInfo.displayId = GetSessionProperty()->GetDisplayId() ;
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
} // namespace OHOS::Rosen