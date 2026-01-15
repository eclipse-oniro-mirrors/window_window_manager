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

#include "session/host/include/scene_session.h"
#include <parameters.h>

#include <ability_manager_client.h>
#include <algorithm>
#include <app_mgr_client.h>
#include <atomic>
#include <climits>
#include "configuration.h"
#include <hitrace_meter.h>
#include "hitrace/hitracechain.h"
#include "hisysevent.h"
#include <type_traits>
#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include <ipc_skeleton.h>
#include <modifier_ng/appearance/rs_behind_window_filter_modifier.h>
#include <pointer_event.h>
#include <key_event.h>
#include <transaction/rs_sync_transaction_controller.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>
#include <ui_extension_utils.h>
#include <feature/window_keyframe/rs_window_keyframe_node.h>

#include "proxy/include/window_info.h"

#include "application_context.h"
#include "common/include/session_permission.h"
#ifdef DEVICE_STATUS_ENABLE
#include "interaction_manager.h"
#endif // DEVICE_STATUS_ENABLE
#include "interfaces/include/ws_common.h"
#include "pixel_map.h"
#include "process_options.h"
#include "rate_limited_logger.h"
#include "rs_adapter.h"
#include "session_coordinate_helper.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session/host/include/session_change_recorder.h"
#include "session/host/include/session_utils.h"
#include "display_manager.h"
#include "session_helper.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_math.h"
#include <running_lock.h>
#include "screen_manager.h"
#include "screen.h"
#include "fold_screen_state_internel.h"
#include "fold_screen_common.h"
#include "session/host/include/ability_info_manager.h"
#include "session/host/include/atomicservice_basic_engine_plugin.h"
#include "session/host/include/multi_instance_manager.h"
#include "session/host/include/pc_fold_screen_controller.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

#define RETURN_IF_NULL_IMPL(param, tag, fn, ...)                       \
    do {                                                               \
        if (!(param)) {                                                \
            TLOGNE(tag, "%{public}s: %{public}s is null", fn, #param); \
            return __VA_ARGS__;                                        \
        }                                                              \
    } while (0)

#define RETURN_IF_NULL_WITH_TAG(param, logTag, ...) \
    RETURN_IF_NULL_IMPL(param, logTag, __func__, __VA_ARGS__)

#define RETURN_IF_NULL_WITH_FUNC(param, funcName, ...) \
    RETURN_IF_NULL_IMPL(param, WmsLogTag::DEFAULT, funcName, __VA_ARGS__)

#define RETURN_IF_NULL(param, ...) RETURN_IF_NULL_WITH_FUNC(param, __func__, __VA_ARGS__)

namespace OHOS::Rosen {
namespace {
const std::string LOCK_WINDOW_CURSOR_PERMISSION = "ohos.permission.LOCK_WINDOW_CURSOR";
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSession" };
const std::string DLP_INDEX = "ohos.dlp.params.index";
const std::string ERROR_REASON_LOW_MEMORY_KILL = "LowMemoryKill";
constexpr const char* APP_CLONE_INDEX = "ohos.extra.param.key.appCloneIndex";
constexpr float MINI_FLOAT_SCALE = 0.3f;
constexpr float MOVE_DRAG_POSITION_Z = 100.5f;
constexpr DisplayId VIRTUAL_DISPLAY_ID = 999;
constexpr WSRectF VELOCITY_RELOCATION_TO_TOP = {0.0f, -10.0f, 0.0f, 0.0f};
constexpr WSRectF VELOCITY_RELOCATION_TO_BOTTOM = {0.0f, 10.0f, 0.0f, 0.0f};
constexpr int32_t API_VERSION_18 = 18;
constexpr int32_t HOOK_SYSTEM_BAR_HEIGHT = 40;
constexpr int32_t HOOK_AI_BAR_HEIGHT = 28;
constexpr int32_t POW_DOUBLE = 2;
constexpr int32_t MULTI_WINDOW_TITLE_BAR_DEFAULT_HEIGHT_VP = 32;
constexpr int32_t MAX_FLOAT_TITLE_BAR_HEIGHT_VP = 40;
constexpr uint32_t ROTATION_DEGREE = 90;
constexpr int32_t HALF_VALUE = 2;
constexpr char SCENE_BOARD_UE_DOMAIN[] = "SCENE_BOARD_UE";
constexpr char DRAG_RESIZE_WINDOW[] = "PC_DRAG_RESIZE_WINDOW";
const int32_t ROTATE_POLICY_WINDOW = 0;
const int32_t ROTATE_POLICY_SCREEN = 1;
const int32_t SCREEN_LOCK_Z_ORDER = 2000;
constexpr int32_t MIN_ROTATION_VALUE = 0;
constexpr int32_t MAX_ROTATION_VALUE = 3;
const std::string OPTIONAL_SHOW = "OPTIONAL_SHOW"; // startWindowType can be changed by startAbility option.
constexpr uint8_t MAX_DOWN_TIMES = 100;

bool CheckIfRectElementIsTooLarge(const WSRect& rect)
{
    int32_t largeNumber = static_cast<int32_t>(SHRT_MAX);
    return rect.posX_ >= largeNumber || rect.posY_ >= largeNumber ||
           rect.width_ >= largeNumber || rect.height_ >= largeNumber;
}

bool CmpExtensionWidowInfoByTimeStamp(const ExtensionWindowEventInfo& a, const ExtensionWindowEventInfo& b)
{
    return a.startModalExtensionTimeStamp < b.startModalExtensionTimeStamp;
}

bool isMainOrExtendScreenMode(const ScreenSourceMode& screenSourceMode)
{
    return screenSourceMode == ScreenSourceMode::SCREEN_MAIN ||
        screenSourceMode == ScreenSourceMode::SCREEN_EXTEND;
}
} // namespace

MaximizeMode SceneSession::maximizeMode_ = MaximizeMode::MODE_RECOVER;
std::shared_mutex SceneSession::windowDragHotAreaMutex_;
std::map<uint64_t, std::map<uint32_t, WSRect>> SceneSession::windowDragHotAreaMap_;
static bool g_enableForceUIFirst = system::GetParameter("window.forceUIFirst.enabled", "1") == "1";
GetConstrainedModalExtWindowInfoFunc SceneSession::onGetConstrainedModalExtWindowInfoFunc_;

SceneSession::SceneSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback, int32_t userId)
    : Session(info)
{
    GeneratePersistentId(false, info.persistentId_, userId);
    specificCallback_ = specificCallback;
    SetCollaboratorType(info.collaboratorType_);
    TLOGI(WmsLogTag::WMS_LIFE, "Create session, id: %{public}d", GetPersistentId());
    WindowHelper::SplitStringByDelimiter(
        system::GetParameter("const.window.containerColorLists", ""), ",", containerColorList_);
}

SceneSession::~SceneSession()
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d", GetPersistentId());
    // exclude when user deletes session in recent.
    if (SessionHelper::IsMainWindow(GetWindowType()) && notifySceneSessionDestructFunc_ && !isUserRequestedExit_) {
        notifySceneSessionDestructFunc_(GetPersistentId());
    }
}

WSError SceneSession::ConnectInner(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token, int32_t pid, int32_t uid,
    const std::string& identityToken)
{
    return PostSyncTask([weakThis = wptr(this), sessionStage, eventChannel, surfaceNode, &systemConfig,
        property, token, pid, uid, identityToken, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (SessionHelper::IsMainWindow(session->GetWindowType())) {
            if (!session->CheckIdentityTokenIfMatched(identityToken)) {
                TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s check failed", where);
                return WSError::WS_OK;
            }
        }
        if (property) {
            property->SetCollaboratorType(session->GetCollaboratorType());
            property->SetAppInstanceKey(session->GetAppInstanceKey());
            property->SetUseControlState(session->isAppUseControl_);
            property->SetAncoRealBundleName(session->IsAnco() ? session->GetSessionInfo().bundleName_ : "");
            if (session->GetSessionInfo().processOptions != nullptr) {
                MissionInfo missionInfo;
                missionInfo.startupInvisibility_ = session->GetSessionInfo().processOptions->startupVisibility ==
                    AAFwk::StartupVisibility::STARTUP_HIDE;
                property->SetMissionInfo(missionInfo);
            }
            property->SetPrelaunch(session->GetSessionInfo().isPrelaunch_);
            property->SetFrameNum(session->GetSessionInfo().frameNum_);
        }
        session->RetrieveStatusBarDefaultVisibility();
        auto ret = LOCK_GUARD_EXPR(SCENE_GUARD, session->Session::ConnectInner(
            sessionStage, eventChannel, surfaceNode, systemConfig, property, token, pid, uid));
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->NotifySingleHandTransformChange(session->GetSingleHandTransform());
        session->NotifyGlobalScaledRectChange();
        session->NotifyPropertyWhenConnect();
        if (session->pcFoldScreenController_) {
            session->pcFoldScreenController_->OnConnect();
        }
        return ret;
    }, __func__);
}

WSError SceneSession::Connect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    const std::string& identityToken)
{
    // Get pid and uid before posting task.
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    return ConnectInner(sessionStage, eventChannel, surfaceNode, systemConfig,
        property, token, pid, uid, identityToken);
}

WSError SceneSession::Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    int32_t pid, int32_t uid)
{
    return PostSyncTask([weakThis = wptr(this), sessionStage, eventChannel,
        surfaceNode, property, token, pid, uid, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WSError ret = LOCK_GUARD_EXPR(SCENE_GUARD,
            session->Session::Reconnect(sessionStage, eventChannel, surfaceNode, property, token, pid, uid));
        if (ret != WSError::WS_OK) {
            return ret;
        }
        return LOCK_GUARD_EXPR(SCENE_GUARD, session->ReconnectInner(property));
    });
}

WSError SceneSession::ReconnectInner(sptr<WindowSessionProperty> property)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "property is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    WindowState windowState = property->GetWindowState();
    TLOGI(WmsLogTag::WMS_RECOVER, "persistentId: %{public}d, windowState: %{public}d ", GetPersistentId(), windowState);
    WSError ret = WSError::WS_OK;
    switch (windowState) {
        case WindowState::STATE_INITIAL: {
            TLOGE(WmsLogTag::WMS_RECOVER, "persistentId: %{public}d, invalid window state: STATE_INITIAL",
                GetPersistentId());
            ret = WSError::WS_ERROR_INVALID_PARAM;
            break;
        }
        case WindowState::STATE_CREATED:
            break;
        case WindowState::STATE_SHOWN: {
            UpdateSessionState(SessionState::STATE_FOREGROUND);
            UpdateActiveStatus(true);
            break;
        }
        case WindowState::STATE_HIDDEN: {
            UpdateSessionState(SessionState::STATE_BACKGROUND);
            break;
        }
        default:
            TLOGE(WmsLogTag::WMS_RECOVER, "persistentId: %{public}d, invalid window state: %{public}u",
                GetPersistentId(), windowState);
            ret = WSError::WS_ERROR_INVALID_PARAM;
            break;
    }
    if (ret != WSError::WS_OK) {
        Session::Disconnect(false);
    }
    if (pcFoldScreenController_) {
        pcFoldScreenController_->OnConnect();
    }
    return ret;
}

bool SceneSession::IsShowOnLockScreen(uint32_t lockScreenZOrder)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: lockScreenZOrder: %{public}d, zOrder_: %{public}d", lockScreenZOrder,
        zOrder_);

    // must be default screen
    ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr && defaultScreenId != sessionProperty->GetDisplayId()) {
        TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: not default display");
        return false;
    }

    // current window on lock screen jurded by zorder
    if (zOrder_ >= lockScreenZOrder) {
        TLOGNI(WmsLogTag::WMS_UIEXT, "zOrder is bigger");
        return true;
    }

    if (zOrder_ == 0) {
        if (auto mainSession = GetMainSession()) {
            TLOGI(WmsLogTag::WMS_UIEXT, "mainSession zOrder=%{public}d", mainSession->GetZOrder());
            return mainSession->GetZOrder() >= lockScreenZOrder;
        }
    }
    return false;
}

void SceneSession::AddExtensionTokenInfo(const UIExtensionTokenInfo& tokenInfo)
{
    extensionTokenInfos_.push_back(tokenInfo);
    TLOGD(WmsLogTag::WMS_UIEXT, "can show:%{public}u, id: %{public}d",
        tokenInfo.canShowOnLockScreen, GetPersistentId());
}

void SceneSession::RemoveExtensionTokenInfo(const sptr<IRemoteObject>& abilityToken)
{
    auto persistentId = GetPersistentId();
    auto itr = std::remove_if(
        extensionTokenInfos_.begin(), extensionTokenInfos_.end(),
        [&abilityToken, persistentId, where = __func__](const auto& tokenInfo) {
            TLOGND(WmsLogTag::WMS_UIEXT,
                "%{public}s UIExtOnLock: need remove, token: %{public}u, persistentId: %{public}d",
                where, tokenInfo.callingTokenId, persistentId);
            return tokenInfo.abilityToken == abilityToken;
        });
    extensionTokenInfos_.erase(itr, extensionTokenInfos_.end());
}

void SceneSession::OnNotifyAboveLockScreen()
{
    CheckExtensionOnLockScreenToClose();
}

void SceneSession::CheckExtensionOnLockScreenToClose()
{
    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: %{public}d", GetPersistentId());

    // 1. check sub session
    for (auto& session : GetSubSession()) {
        if (!session) {
            TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: session is null");
            continue;
        }
        session->CheckExtensionOnLockScreenToClose();
    }

    // 2. check self permission
    std::vector<UIExtensionTokenInfo> tokenInfosToClose;
    for (auto& tokenInfo : extensionTokenInfos_) {
        if (tokenInfo.canShowOnLockScreen) {
            continue;
        }
        tokenInfosToClose.push_back(tokenInfo);
    }

    // 3. close ui extension without lock screen permisson
    std::for_each(tokenInfosToClose.rbegin(), tokenInfosToClose.rend(),
        [this](const UIExtensionTokenInfo& tokenInfo) { CloseExtensionSync(tokenInfo); });
}

void SceneSession::CloseExtensionSync(const UIExtensionTokenInfo& tokenInfo)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock");

    // hide sub window
    auto subSceneSessions = GetSubSession();
    for (auto& session : subSceneSessions) {
        if (!session) {
            TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: session is null");
            continue;
        }
        // hide sub window of ui extension
        if (session->GetAbilityToken() == tokenInfo.abilityToken) {
            TLOGI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: hide sub window %{public}u", session->GetWindowId());
            session->HideSync();
        }
    }

    TLOGI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: close ui extension, callerToken: %{public}u, persistent id %{public}d",
        tokenInfo.callingTokenId, GetPersistentId());

    // kill ui extension ability
    AAFwk::AbilityManagerClient::GetInstance()->CloseUIExtensionAbilityBySCB(tokenInfo.abilityToken);
}

WSError SceneSession::Foreground(
    sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken)
{
    if (!CheckPermissionWithPropertyAnimation(property)) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }

    // return when screen is locked and show without ShowWhenLocked flag
    ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    auto sessionProperty = GetSessionProperty();
    if (GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
        IsActivatedAfterScreenLocked() &&
        GetStateFromManager(ManagerState::MANAGER_STATE_SCREEN_LOCKED) &&
        (sessionProperty != nullptr && defaultScreenId == sessionProperty->GetDisplayId()) &&
        !IsShowWhenLocked()) {
        if (SessionPermission::VerifyCallingPermission("ohos.permission.CALLED_BELOW_LOCK_SCREEN")) {
            TLOGW(WmsLogTag::WMS_LIFE, "screen is locked, session %{public}d %{public}s permission verified",
                GetPersistentId(), sessionInfo_.bundleName_.c_str());
        } else if (SessionPermission::VerifyCallingPermission("ohos.permission.START_WINDOW_BELOW_LOCK_SCREEN")) {
            TLOGW(WmsLogTag::WMS_LIFE,
                "screen is locked, session %{public}d %{public}s start below lock screen permission verified",
                GetPersistentId(), sessionInfo_.bundleName_.c_str());
        } else {
            TLOGW(WmsLogTag::WMS_LIFE,
                "failed: screen is locked, session %{public}d %{public}s show without ShowWhenLocked flag",
                GetPersistentId(), sessionInfo_.bundleName_.c_str());
            return WSError::WS_ERROR_INVALID_SESSION;
        }
    }

    if (isFromClient && SessionHelper::IsMainWindow(GetWindowType())) {
        if (!CheckPidIfMatched() || !CheckIdentityTokenIfMatched(identityToken)) {
            TLOGW(WmsLogTag::WMS_LIFE, "check failed");
            return WSError::WS_OK;
        }
    }
    return ForegroundTask(property);
}

WSError SceneSession::ForegroundTask(const sptr<WindowSessionProperty>& property)
{
    PostTask([weakThis = wptr(this), property, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto sessionProperty = session->GetSessionProperty();
        if (property && sessionProperty) {
            sessionProperty->SetWindowMode(property->GetWindowMode());
            sessionProperty->SetDecorEnable(property->IsDecorEnable());
            session->SetFocusableOnShow(property->GetFocusableOnShow());
        }
        int32_t persistentId = session->GetPersistentId();
        auto ret = session->Session::Foreground(property);
        if (ret != WSError::WS_OK) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s ret=%{public}d win=%{public}d", where, ret, persistentId);
            return ret;
        }
        session->NotifySingleHandTransformChange(session->GetSingleHandTransform());
        session->NotifyGlobalScaledRectChange();
        session->SetSecurityLayerWhenEnterForeground();
        session->MarkAvoidAreaAsDirty();
        auto subSessions = session->GetSubSession();
        for (const auto& subSession : subSessions) {
            if (subSession) {
                subSession->MarkAvoidAreaAsDirty();
            }
        }
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onWindowInfoUpdate_(
                persistentId, WindowUpdateType::WINDOW_UPDATE_ADDED);
            session->specificCallback_->onHandleSecureSessionShouldHide_(session);
            session->UpdateGestureBackEnabled();
        } else {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s foreground specific callback is null", where);
        }
        session->DisableUIFirstIfNeed();
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

void SceneSession::DisableUIFirstIfNeed()
{
    if (!isUIFirstEnabled_) {
        TLOGI(WmsLogTag::WMS_ANIMATION, "UIFirst is disabled, id: %{public}d.", GetPersistentId());
        return;
    }
    auto leashWinShadowSurfaceNode = GetLeashWinShadowSurfaceNode();
    if (!leashWinShadowSurfaceNode) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "leashWinShadowSurfaceNode is null, id:%{public}d.", GetPersistentId());
        return;
    }
    if (RSAdapterUtil::IsClientMultiInstanceEnabled()) {
        // If client multi instance is enabled，use the shadowSurfaceNode's commit flush SetForceUIFirst command.
        AutoRSTransaction trans(GetRSLeashWinShadowContext());
        leashWinShadowSurfaceNode->SetForceUIFirst(false);
    } else {
        // If client multi instance is disabled，reuse the arkUI's commit flush SetForceUIFirst command.
        leashWinShadowSurfaceNode->SetForceUIFirst(false);
    }
    isUIFirstEnabled_ = false;
    TLOGI(WmsLogTag::WMS_ANIMATION, "leashWinShadowSurfaceNode disable UIFirst id:%{public}d!", GetPersistentId());
}

void SceneSession::CheckAndMoveDisplayIdRecursively(uint64_t displayId)
{
    if (GetSessionProperty()->GetDisplayId() == displayId || !shouldFollowParentWhenShow_) {
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "session id: %{public}d, Move display to %{public}" PRIu64,
        GetPersistentId(), displayId);
    SetScreenId(displayId); // notify client to update display id
    GetSessionProperty()->SetDisplayId(displayId); // set session property
    NotifySessionDisplayIdChange(displayId);
    for (const auto& session : subSession_) {
        if (session) {
            session->CheckAndMoveDisplayIdRecursively(displayId);
        }
    }
}

WSError SceneSession::Background(bool isFromClient, const std::string& identityToken)
{
    if (!CheckPermissionWithPropertyAnimation(GetSessionProperty())) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }

    if (isFromClient && SessionHelper::IsMainWindow(GetWindowType())) {
        if (!CheckPidIfMatched() || !CheckIdentityTokenIfMatched(identityToken)) {
            TLOGW(WmsLogTag::WMS_LIFE, "check failed");
            return WSError::WS_OK;
        }
    }
    return BackgroundTask(true);
}

WSError SceneSession::NotifyFrameLayoutFinishFromApp(bool notifyListener, const WSRect& rect)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "%{public}d, %{public}s", notifyListener, rect.ToString().c_str());
    bool isSaCalling = SessionPermission::IsSACalling();
    PostTask([weakThis = wptr(this), notifyListener, rect, where = __func__, isSaCalling] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        const bool isAncoSupportNotifyFrameLayoutFinish = system::GetBoolParameter("prop_to_param.support.notifyFrameLayoutFinish", false);
        if (session->IsAnco() && isAncoSupportNotifyFrameLayoutFinish) {
            if (!isSaCalling) {
                TLOGND(WmsLogTag::WMS_PATTERN, "%{public}s, not sa invoke, set rect and return.", where);
                session->layoutRect_ = rect;
                return WSError::WS_OK;
            }
        } else {
            TLOGND(WmsLogTag::WMS_PATTERN, "%{public}s, set rect and notifyListener.", where);
            session->layoutRect_ = rect;
        }
        session->NotifyLayoutFinished();
        if (notifyListener && session->frameLayoutFinishFunc_) {
            TLOGND(WmsLogTag::WMS_MULTI_WINDOW, "%{public}s id: %{public}d", where, session->GetPersistentId());
            session->frameLayoutFinishFunc_();
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::BackgroundTask(const bool isSaveSnapshot, LifeCycleChangeReason reason)
{
    auto needSaveSnapshot = !isPersistentImageFit_.load();
    PostTask([weakThis = wptr(this), isSaveSnapshot, needSaveSnapshot, reason, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto state = session->GetSessionState();
        if (state == SessionState::STATE_BACKGROUND) {
            return WSError::WS_OK;
        }
        session->snapshotNeedCancel_.store(false);
        auto ret = session->Session::Background();
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->SetSnapshotPrivacyMode(session->GetIsPrivacyMode());
        if (WindowHelper::IsMainWindow(session->GetWindowType()) && isSaveSnapshot && needSaveSnapshot) {
            session->SetFreeMultiWindow();
            session->SaveSnapshot(true, true, nullptr, false, reason);
        }
        if (session->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE);
        }
        session->MarkAvoidAreaAsDirty();
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onWindowInfoUpdate_(
                session->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
            session->specificCallback_->onHandleSecureSessionShouldHide_(session);
            session->UpdateGestureBackEnabled();
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WMError SceneSession::NotifySnapshotUpdate()
{
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s: session is null", where);
            return;
        }
        if (session->IsAnco()) {
            TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s: id: %{public}d, is anco", where, session->GetPersistentId());
            return;
        }
        TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s: id: %{public}d", where, session->GetPersistentId());
        if (WindowHelper::IsMainWindow(session->GetWindowType())) {
            session->SaveSnapshot(true, true, nullptr, true);
        }
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSession::NotifyRemovePrelaunchStartingWindow()
{
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s: session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s: id: %{public}d", where, session->GetPersistentId());
        if (WindowHelper::IsMainWindow(session->GetWindowType())) {
            session->RemovePrelaunchStartingWindow();
        }
    }, __func__);
    return WMError::WM_OK;
}

void SceneSession::ClearSpecificSessionCbMap()
{
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s: session is null", where);
            return;
        }
        session->ClearJsSceneSessionCbMap(true);
    }, __func__);
}

void SceneSession::ClearJsSceneSessionCbMap(bool needRemove)
{
    if (clearCallbackMapFunc_) {
        TLOGD(WmsLogTag::WMS_LIFE, "id: %{public}d, needRemove: %{public}d", GetPersistentId(), needRemove);
        clearCallbackMapFunc_(needRemove);
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "get callback failed, id: %{public}d", GetPersistentId());
    }
}

void SceneSession::RegisterShowWhenLockedCallback(NotifyShowWhenLockedFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->onShowWhenLockedFunc_ = std::move(callback);
        session->onShowWhenLockedFunc_(session->GetShowWhenLockedFlagValue());
    }, __func__);
}

void SceneSession::RegisterForceHideChangeCallback(NotifyForceHideChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->onForceHideChangeFunc_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterClearCallbackMapCallback(ClearCallbackMapFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->clearCallbackMapFunc_ = std::move(callback);
    }, __func__);
}

WSError SceneSession::Disconnect(bool isFromClient, const std::string& identityToken)
{
    if (isFromClient && SessionHelper::IsMainWindow(GetWindowType())) {
        if (!CheckPidIfMatched() || !CheckIdentityTokenIfMatched(identityToken)) {
            TLOGW(WmsLogTag::WMS_LIFE, "check failed");
            return WSError::WS_OK;
        }
    }
    return DisconnectTask(isFromClient, true);
}

WSError SceneSession::DisconnectTask(bool isFromClient, bool isSaveSnapshot)
{
    auto needSaveSnapshot = !isPersistentImageFit_.load();
    PostTask([weakThis = wptr(this), isFromClient, isSaveSnapshot, needSaveSnapshot, where = __func__]()
        THREAD_SAFETY_GUARD(SCENE_GUARD) {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto isMainWindow = SessionHelper::IsMainWindow(session->GetWindowType());
        if (isMainWindow) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s Notify scene session id: %{public}d paused", where,
                session->GetPersistentId());
            session->UpdateLifecyclePausedInner();
        }
        if (session->sessionInfo_.isPrelaunch_) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s Remove prelaunch session id: %{public}d", where,
                session->GetPersistentId());
            session->sessionInfo_.isPrelaunch_ = false;
            session->sessionInfo_.frameNum_ = 0;
        }
        if (isFromClient) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s Client need notify destroy session, id: %{public}d",
                where, session->GetPersistentId());
            session->SetSessionState(SessionState::STATE_DISCONNECT);
            return WSError::WS_OK;
        }
        session->SetSnapshotPrivacyMode(session->GetIsPrivacyMode());
        auto state = session->GetSessionState();
        if ((session->needSnapshot_ || (state == SessionState::STATE_ACTIVE && isMainWindow)) &&
            isSaveSnapshot && needSaveSnapshot) {
            session->SaveSnapshot(false);
        }
        session->Session::Disconnect(isFromClient);
        session->isTerminating_ = false;
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onHandleSecureSessionShouldHide_(session);
            session->isEnableGestureBack_ = true;
            session->UpdateGestureBackEnabled();
            session->isEnableGestureBackHadSet_ = false;
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::UpdateActiveStatus(bool isActive)
{
    PostTask([weakThis = wptr(this), isActive, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->IsSessionValid()) {
            TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s Session is invalid, id: %{public}d state: %{public}u",
                where, session->GetPersistentId(), session->GetSessionState());
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        if (isActive == session->isActive_) {
            TLOGND(WmsLogTag::WMS_LIFE, "%{public}s Session active do not change: %{public}d",
                where, isActive);
            return WSError::WS_DO_NOTHING;
        }

        WSError ret = WSError::WS_DO_NOTHING;
        if (isActive && session->GetSessionState() == SessionState::STATE_FOREGROUND) {
            session->UpdateSessionState(SessionState::STATE_ACTIVE);
            session->isActive_ = isActive;
            ret = WSError::WS_OK;
        }
        if (!isActive && session->GetSessionState() == SessionState::STATE_ACTIVE) {
            session->UpdateSessionState(SessionState::STATE_INACTIVE);
            session->isActive_ = isActive;
            ret = WSError::WS_OK;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s isActive: %{public}d, state: %{public}u",
            where, session->isActive_, session->GetSessionState());
        return ret;
    }, std::string(__func__) + ":" + std::to_string(isActive));
    return WSError::WS_OK;
}

DMRect SceneSession::CalcRectForStatusBar()
{
    DMRect statusBarRect = {0, 0, 0, 0};
    if (specificCallback_ == nullptr || specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "specificCallback is null");
        return statusBarRect;
    }
    const auto& statusBarVector = specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_(
        WindowType::WINDOW_TYPE_STATUS_BAR, GetSessionProperty()->GetDisplayId());
    for (auto& statusBar : statusBarVector) {
        if (statusBar == nullptr) {
            continue;
        }
        if (statusBar->IsVisible() &&
            static_cast<uint32_t>(statusBar->GetSessionRect().height_) > statusBarRect.height_) {
            statusBarRect.height_ = static_cast<uint32_t>(statusBar->GetSessionRect().height_);
        }
        if (static_cast<uint32_t>(statusBar->GetSessionRect().width_) > statusBarRect.width_) {
            statusBarRect.width_ = static_cast<uint32_t>(statusBar->GetSessionRect().width_);
        }
    }
    TLOGD(
        WmsLogTag::WMS_KEYBOARD, "width: %{public}d, height: %{public}d", statusBarRect.width_, statusBarRect.height_);
    return statusBarRect;
}

WSError SceneSession::SetMoveAvailableArea(DisplayId displayId)
{
    sptr<Display> display = DisplayManager::GetInstance().GetDisplayById(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "fail to get display");
        return WSError::WS_ERROR_INVALID_DISPLAY;
    }

    DMRect availableArea;
    DMError ret = display->GetAvailableArea(availableArea);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get available area, ret: %{public}d", ret);
        return WSError::WS_ERROR_INVALID_DISPLAY;
    }

    DMRect statusBarRect = CalcRectForStatusBar();
    if (systemConfig_.IsPadWindow() || systemConfig_.IsPhoneWindow()) {
        uint32_t statusBarHeight = statusBarRect.height_;
        if (static_cast<int32_t>(statusBarHeight) > availableArea.posY_) {
            availableArea.posY_ = static_cast<int32_t>(statusBarHeight);
        }

        sptr<ScreenSession> currentScreenSession =
            ScreenSessionManagerClient::GetInstance().GetScreenSessionById(GetSessionProperty()->GetDisplayId());
        if (currentScreenSession == nullptr) {
            TLOGW(WmsLogTag::WMS_KEYBOARD, "Screen session is null");
            return WSError::WS_ERROR_INVALID_DISPLAY;
        }
        uint32_t currentScreenHeight = currentScreenSession->GetScreenProperty().GetBounds().rect_.height_;
        availableArea.height_ = currentScreenHeight - static_cast<uint32_t>(availableArea.posY_);
    }

    bool isFoldable = ScreenSessionManagerClient::GetInstance().IsFoldable();
    if (systemConfig_.IsPhoneWindow() && isFoldable && statusBarRect.width_) {
        availableArea.width_ = statusBarRect.width_;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD,
          "the available area x is: %{public}d, y is: %{public}d, width is: %{public}d, height is: %{public}d",
          availableArea.posX_, availableArea.posY_, availableArea.width_, availableArea.height_);
    moveDragController_->SetMoveAvailableArea(availableArea);
    return WSError::WS_OK;
}

WSError SceneSession::InitializeMoveInputBar()
{
    auto property = GetSessionProperty();
    WindowType windowType = property->GetWindowType();
    if (WindowHelper::IsInputWindow(windowType)) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Start init move input bar param");
        DisplayId displayId = property->GetDisplayId();

        WSError ret = SetMoveAvailableArea(displayId);
        if (ret != WSError::WS_OK) {
            TLOGD(WmsLogTag::WMS_KEYBOARD, "set move availableArea error");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        moveDragController_->SetMoveInputBarStartDisplayId(displayId);
    }
    return WSError::WS_OK;
}

bool SceneSession::IsNeedConvertToRelativeRect(SizeChangeReason reason) const
{
    if (reason == SizeChangeReason::DRAG_MOVE) {
        return true;
    }
    if (WindowHelper::IsSubWindow(GetWindowType())) {
        return IsNeedCrossDisplayRendering() ||
               (GetWindowAnchorInfo().isAnchorEnabled_ && IsAnyParentSessionDragMoving());
    }
    return false;
}

bool SceneSession::IsDragMoving() const
{
    return moveDragController_ ? moveDragController_->GetStartMoveFlag() : false;
}

bool SceneSession::IsDragZooming() const
{
    return moveDragController_ ? moveDragController_->GetStartDragFlag() : false;
}

bool SceneSession::IsCrossDisplayDragSupported() const
{
    if (IsAnco()) {
        return false;
    }
    auto windowType = GetWindowType();
    return !WindowHelper::IsSystemWindow(windowType) ||
           windowType == WindowType::WINDOW_TYPE_FLOAT ||
           windowType == WindowType::WINDOW_TYPE_SCREENSHOT ||
           WindowHelper::IsInputWindow(windowType);
}

bool SceneSession::IsAnyParentSessionDragMoving() const
{
    if (SessionHelper::IsMainWindow(GetWindowType())) {
        return IsDragMoving();
    } else if (parentSession_) {
        return parentSession_->IsDragMoving() ? true : parentSession_->IsAnyParentSessionDragMoving();
    }
    return false;
}

bool SceneSession::IsAnyParentSessionDragZooming() const
{
    if (SessionHelper::IsMainWindow(GetWindowType())) {
        return IsDragZooming();
    } else if (parentSession_) {
        return parentSession_->IsDragZooming() ? true : parentSession_->IsAnyParentSessionDragZooming();
    }
    return false;
}

void SceneSession::SetParentRect()
{
    if ((systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode()) && moveDragController_ != nullptr) {
        moveDragController_->SetParentRect({0, 0, 0, 0});
        return;
    }
    auto mainSession = GetMainSession();
    if (mainSession != nullptr && moveDragController_ != nullptr) {
        Rect parentGlobalRect;
        WMError errorCode = mainSession->GetGlobalScaledRect(parentGlobalRect);
        if (errorCode != WMError::WM_OK) {
            TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, errorCode:%{public}d", GetPersistentId(), errorCode);
            return;
        }
        moveDragController_->SetParentRect(parentGlobalRect);
    }
}

WSRect SceneSession::GetGlobalOrWinRect()
{
    if (systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode()) {
        return GetSessionRect();
    }
    return GetSessionGlobalRect();
}

WindowLimits SceneSession::GetWindowLimits() const
{
    WindowLimits limits;
    if (auto property = GetSessionProperty()) {
        limits = property->GetWindowLimits();
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to get window limits, property is null");
    }
    return limits;
}

void SceneSession::ApplySessionEventParam(SessionEvent event, const SessionEventParam& param)
{
    switch (event) {
        case SessionEvent::EVENT_MAXIMIZE:
            sessionEventParam_.waterfallResidentState = param.waterfallResidentState;
            break;
        case SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE:
            sessionEventParam_.compatibleStyleMode = param.compatibleStyleMode;
            break;
        default:
            sessionEventParam_.waterfallResidentState = static_cast<uint32_t>(WaterfallResidentState::UNCHANGED);
            sessionEventParam_.compatibleStyleMode = static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_DEFAULT);
            break;
    }
}

void SceneSession::NotifyWindowStatusDidChangeIfNeedWhenSessionEvent(SessionEvent event)
{
    if (!ShouldNotifyWindowStatusChange(event)) {
        return;
    }
    constexpr uint32_t timesToWaitForVsyncs = 2;
    RunAfterNVsyncs(timesToWaitForVsyncs, [weakSession = wptr(this), where = __func__](int64_t, int64_t) {
        auto session = weakSession.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return;
        }

        session->ExecuteWindowStatusChangeNotification(where);
    });
}

bool SceneSession::ShouldNotifyWindowStatusChange(SessionEvent event) const
{
    const auto currentMode = GetWindowMode();
    bool isDockAutoHide = onGetIsDockAutoHideFunc_ ? onGetIsDockAutoHideFunc_() : false;

    return (event == SessionEvent::EVENT_MAXIMIZE &&
            currentMode == WindowMode::WINDOW_MODE_FULLSCREEN &&
            isDockAutoHide);
}

void SceneSession::ExecuteWindowStatusChangeNotification(const char* where)
{
    PostTask([weakSession = wptr(this), where] {
        auto session = weakSession.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return;
        }

        if (!session->sessionStage_) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: sessionStage is null", where);
            return;
        }

        session->sessionStage_->NotifyLayoutFinishAfterWindowModeChange(session->GetWindowMode());
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: Notify completed, windowId: %{public}d",
            where, session->GetPersistentId());
    }, __func__);
}

WSError SceneSession::OnSessionEvent(SessionEvent event, const SessionEventParam& param)
{
    PostTask([weakThis = wptr(this), event, param, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s event: %{public}d", where, static_cast<int32_t>(event));
        session->NotifyWindowStatusDidChangeIfNeedWhenSessionEvent(event);
        session->UpdateWaterfallMode(event);
        auto property = session->GetSessionProperty();
        bool proportionalScale = property->IsAdaptToProportionalScale();
        if (event == SessionEvent::EVENT_START_MOVE) {
            if (!session->IsMovable()) {
                return WSError::WS_OK;
            }
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::StartMove");
            WSError ret = session->InitializeMoveInputBar();
            if (ret != WSError::WS_OK) {
                return ret;
            }
            session->InitializeCrossMoveDrag();
            session->moveDragController_->InitMoveDragProperty();
            if (session->pcFoldScreenController_) {
                WSRect currRect;
                session->HookStartMoveRect(currRect, session->GetSessionRect());
                session->pcFoldScreenController_->RecordStartMoveRect(currRect, session->IsFullScreenMovable());
            }
            WSRect rect = session->GetMoveRectForWindowDrag();
            if (session->IsFullScreenMovable() || (session->IsSplitMovable() && !session->IsFreeMultiWindowMode())) {
                session->UpdateFullScreenWaterfallMode(false);
                rect = session->moveDragController_->GetFullScreenToFloatingRect(session->GetSessionRect(),
                    session->GetSessionRequestRect());
                session->Session::UpdateRect(rect, SizeChangeReason::RECOVER, where, nullptr);
                session->moveDragController_->SetStartMoveFlag(true);
                session->moveDragController_->CalcFirstMoveTargetRect(rect, true);
                session->ForceNotifyKeyboardOccupiedArea();
            } else {
                session->SetParentRect();
                session->moveDragController_->SetScale(session->GetScaleX(), session->GetScaleY());
                session->moveDragController_->SetStartMoveFlag(true);
                // use window rect when fullscreen or compatible mode
                session->moveDragController_->CalcFirstMoveTargetRect(rect, proportionalScale);
                if(session->IsSplitMovable()){
                    session->ForceNotifyKeyboardOccupiedArea();
                }
            }
            auto globalRect = session->moveDragController_->GetTargetRect(MoveDragController::TargetRectCoordinate::GLOBAL);
            session->SetSessionEventParam({session->moveDragController_->GetOriginalPointerPosX(),
                session->moveDragController_->GetOriginalPointerPosY(),
                rect.width_, rect.height_, 0, 0, 0, 0, globalRect.posX_, globalRect.posY_});
        }
        session->HandleSessionDragEvent(event);
        session->ApplySessionEventParam(event, param);
        if (session->onSessionEvent_) {
            session->onSessionEvent_(static_cast<uint32_t>(event), session->sessionEventParam_);
        }
        return WSError::WS_OK;
    }, std::string(__func__) + ":" + std::to_string(static_cast<uint32_t>(event)));
    return WSError::WS_OK;
}

void SceneSession::HandleSessionDragEvent(SessionEvent event)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    if (moveDragController_ &&
        (event == SessionEvent::EVENT_DRAG || event == SessionEvent::EVENT_DRAG_START)) {
        WSRect rect = moveDragController_->GetTargetRect(
            MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
        auto property = GetSessionProperty();
        if (event == SessionEvent::EVENT_DRAG_START && moveDragController_->GetStartDragFlag()) {
            if (property->IsAdaptToDragScale()) {
                dragResizeType = DragResizeType::RESIZE_SCALE;
            } else {
                dragResizeType = GetAppDragResizeType();
            }
            SetDragResizeTypeDuringDrag(dragResizeType);
        } else if (event == SessionEvent::EVENT_DRAG) {
            if (property->IsAdaptToDragScale()) {
                dragResizeType = DragResizeType::RESIZE_SCALE;
            } else {
                dragResizeType = GetDragResizeTypeDuringDrag();
            }
        }
        Gravity gravity = moveDragController_->GetGravity();
        SetSessionEventParam({rect.posX_, rect.posY_, rect.width_, rect.height_,
            static_cast<uint32_t>(dragResizeType), static_cast<uint32_t>(gravity)});
    } else if (moveDragController_ && event == SessionEvent::EVENT_END_MOVE) {
        const auto& lastDragEndRect = moveDragController_->GetLastDragEndRect();
        SetSessionEventParam({lastDragEndRect.posX_, lastDragEndRect.posY_,
            lastDragEndRect.width_, lastDragEndRect.height_,
            static_cast<uint32_t>(GetDragResizeTypeDuringDrag()),
            static_cast<uint32_t>(moveDragController_->GetGravity())});
        SetDragResizeTypeDuringDrag(dragResizeType);
    }
}

void SceneSession::UpdateWaterfallMode(SessionEvent event)
{
    if (pcFoldScreenController_ == nullptr) {
        return;
    }
    switch (event) {
        case SessionEvent::EVENT_MAXIMIZE_WATERFALL:
            UpdateFullScreenWaterfallMode(pcFoldScreenController_->IsHalfFolded(GetScreenId()));
            break;
        case SessionEvent::EVENT_WATERFALL_TO_MAXIMIZE:
        case SessionEvent::EVENT_RECOVER:
        case SessionEvent::EVENT_SPLIT_PRIMARY:
        case SessionEvent::EVENT_SPLIT_SECONDARY:
            UpdateFullScreenWaterfallMode(false);
            break;
        default:
            break;
    }
}

WSError SceneSession::SyncSessionEvent(SessionEvent event)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "the sync session event is: %{public}d", event);
    if (event != SessionEvent::EVENT_START_MOVE && event != SessionEvent::EVENT_END_MOVE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "This is not start move event or end move event, "
            "eventId=%{public}u windowId=%{public}d", event, GetPersistentId());
        return WSError::WS_ERROR_NULLPTR;
    }
    return PostSyncTask([weakThis = wptr(this), event, where = __func__] {
        auto session = weakThis.promote();
        if (!session || !session->moveDragController_) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s: session or moveDragController is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (event == SessionEvent::EVENT_END_MOVE) {
            if (!session->moveDragController_->GetStartMoveFlag()) {
                TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s Repeat operation, window is not moving", where);
                return WSError::WS_OK;
            }
            session->moveDragController_->StopMoving();
            return WSError::WS_OK;
        }
        if (session->moveDragController_->GetStartMoveFlag()) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s: Repeat operation, system window is moving", where);
            return WSError::WS_ERROR_REPEAT_OPERATION;
        }
        session->OnSessionEvent(event);
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSession::StartMovingWithCoordinate(int32_t offsetX, int32_t offsetY,
    int32_t pointerPosX, int32_t pointerPosY, DisplayId displayId)
{
    return PostSyncTask([weakThis = wptr(this), offsetX, offsetY, pointerPosX, pointerPosY,
        displayId, where = __func__] {
        auto session = weakThis.promote();
        if (!session || !session->moveDragController_) {
            TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: session or moveDragController is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (session->moveDragController_->GetStartMoveFlag()) {
            TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: Repeat operation, window is moving", where);
            return WSError::WS_ERROR_REPEAT_OPERATION;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: offset:[%{public}d,%{public}d] pointer:[%{public}d,%{public}d]"
            " displayId:%{public}" PRIu64, where, offsetX, offsetY, pointerPosX, pointerPosY, displayId);
        int32_t pointerY = pointerPosY;
        if (displayId == VIRTUAL_DISPLAY_ID) {
            pointerY += PcFoldScreenManager::GetInstance().GetVirtualDisplayPosY();
            TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: virtual display pointerY:%{public}d", where, pointerY);
        }
        WSRect winRect = {
            pointerPosX - offsetX,
            pointerY - offsetY,
            session->GetSessionRect().width_,
            session->GetSessionRect().height_
        };
        session->InitializeCrossMoveDrag();
        session->moveDragController_->InitMoveDragProperty();
        MoveDragController::MoveCoordinateProperty property = { offsetX, offsetY, pointerPosX,
            pointerY, displayId, winRect };
        session->moveDragController_->HandleStartMovingWithCoordinate(property, session->IsMovable());
        session->moveDragController_->SetSpecifyMoveStartDisplay(displayId);
        session->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
        return WSError::WS_OK;
    }, __func__);
}

uint32_t SceneSession::GetWindowDragHotAreaType(DisplayId displayId, uint32_t type, int32_t pointerX, int32_t pointerY)
{
    std::shared_lock<std::shared_mutex> lock(windowDragHotAreaMutex_);
    if (windowDragHotAreaMap_.find(displayId) == windowDragHotAreaMap_.end()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Display is invalid.");
        return type;
    }
    for (const auto& [key, rect] : windowDragHotAreaMap_[displayId]) {
        if (rect.IsInRegion(pointerX, pointerY)) {
            type |= key;
        }
    }
    return type;
}

void SceneSession::AddOrUpdateWindowDragHotArea(DisplayId displayId, uint32_t type, const WSRect& area)
{
    std::unique_lock<std::shared_mutex> lock(windowDragHotAreaMutex_);
    windowDragHotAreaMap_[displayId].insert_or_assign(type, area);
}

WSError SceneSession::NotifySubModalTypeChange(SubWindowModalType subWindowModalType)
{
    PostTask([weakThis = wptr(this), subWindowModalType, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s subWindowModalType: %{public}u",
            where, static_cast<uint32_t>(subWindowModalType));
        if (session->onSubModalTypeChange_) {
            session->onSubModalTypeChange_(subWindowModalType);
        }
    }, __func__);
    return WSError::WS_OK;
}

void SceneSession::RegisterSubModalTypeChangeCallback(NotifySubModalTypeChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session or SessionModalTypeChangeFunc is null", where);
            return;
        }
        session->onSubModalTypeChange_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s id: %{public}d",
            where, session->GetPersistentId());
    }, __func__);
}

void SceneSession::RegisterMainModalTypeChangeCallback(NotifyMainModalTypeChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s session or func is null", where);
            return;
        }
        session->onMainModalTypeChange_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d", where, session->GetPersistentId());
    }, __func__);
}

bool SceneSession::IsDialogWindow() const
{
    return WindowHelper::IsDialogWindow(GetSessionProperty()->GetWindowType());
}

SubWindowModalType SceneSession::GetSubWindowModalType() const
{
    SubWindowModalType modalType = SubWindowModalType::TYPE_UNDEFINED;
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "property is nullptr");
        return modalType;
    }
    auto windowType = property->GetWindowType();
    if (WindowHelper::IsToastSubWindow(windowType, property->GetWindowFlags())) {
        return SubWindowModalType::TYPE_TOAST;
    }
    if (WindowHelper::IsTextMenuSubWindow(windowType, property->GetWindowFlags())) {
        return SubWindowModalType::TYPE_TEXT_MENU;
    }
    if (WindowHelper::IsDialogWindow(windowType)) {
        modalType = SubWindowModalType::TYPE_DIALOG;
    } else if (WindowHelper::IsModalSubWindow(windowType, property->GetWindowFlags())) {
        if (WindowHelper::IsApplicationModalSubWindow(windowType, property->GetWindowFlags())) {
            modalType = SubWindowModalType::TYPE_APPLICATION_MODALITY;
        } else {
            modalType = SubWindowModalType::TYPE_WINDOW_MODALITY;
        }
    } else if (WindowHelper::IsSubWindow(windowType)) {
        modalType = SubWindowModalType::TYPE_NORMAL;
    }
    return modalType;
}

void SceneSession::SetSessionEventParam(SessionEventParam param)
{
    sessionEventParam_ = param;
}

void SceneSession::RegisterSessionEventCallback(NotifySessionEventFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        session->onSessionEvent_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterUpdateAppUseControlCallback(UpdateAppUseControlFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onUpdateAppUseControlFunc_ = std::move(callback);
        if (session->GetSessionInfo().hasPrivacyModeControl) {
            TLOGNI(WmsLogTag::WMS_LIFE, "has privacy mode control, id:%{public}d", session->GetPersistentId());
            ControlInfo controlInfo = { .isNeedControl = true, .isControlRecentOnly = true };
            session->NotifyUpdateAppUseControl(ControlAppType::PRIVACY_WINDOW, controlInfo);
        } else {
            session->UpdatePrivacyModeControlInfo();
        }
        if (!session->onGetAllAppUseControlMapFunc_) {
            TLOGNE(WmsLogTag::WMS_LIFE,
                "id: %{public}d session GetAllAppUseControlMapFunc is null", session->GetPersistentId());
            return;
        }
        auto& allAppUseControlMap = session->onGetAllAppUseControlMapFunc_();
        std::string key = SessionUtils::GetAppLockKey(session->GetSessionInfo().bundleName_,
            session->GetSessionInfo().appIndex_);
        if (allAppUseControlMap.find(key) == allAppUseControlMap.end()) {
            return;
        }
        bool appUseControlResult = false;
        for (const auto& [type, info] : allAppUseControlMap[key]) {
            TLOGNI(WmsLogTag::WMS_LIFE,
                "notify appUseControl when register, key: %{public}s, control: %{public}d, controlRecent: %{public}d",
                key.c_str(), info.isNeedControl, info.isControlRecentOnly);
            session->SetAppControlInfo(type, info);
            session->onUpdateAppUseControlFunc_(type, info.isNeedControl, info.isControlRecentOnly);
            if (info.isNeedControl && !info.isControlRecentOnly) {
                appUseControlResult = true;
            }
        }
        session->isAppUseControl_ = appUseControlResult;
    }, __func__);
}

void SceneSession::NotifyUpdateAppUseControl(ControlAppType type, const ControlInfo& controlInfo)
{
    PostTask([weakThis = wptr(this), type, controlInfo, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->SetAppControlInfo(type, controlInfo);
        session->UpdateAppLockSnapshot(type, controlInfo);
        if (session->onUpdateAppUseControlFunc_) {
            bool isAppUseControl = (controlInfo.isNeedControl && !controlInfo.isControlRecentOnly);
            bool isAppUseControlChanged = (session->isAppUseControl_ != isAppUseControl);
            session->isAppUseControl_ = isAppUseControl;
            session->onUpdateAppUseControlFunc_(type, controlInfo.isNeedControl, controlInfo.isControlRecentOnly);
            if (session->sessionStage_ == nullptr || type == ControlAppType::PRIVACY_WINDOW) {
                TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s sessionStage is nullptr or privacy mode control", where);
                return;
            }
            auto state = session->GetSessionState();
            TLOGNI(WmsLogTag::WMS_LIFE,
                "%{public}s isAppUseControl: %{public}d, persistentId: %{public}d, state: %{public}d, "
                "isAppUseControlChanged: %{public}d", where, isAppUseControl, session->GetPersistentId(), state,
                isAppUseControlChanged);
            if (state == SessionState::STATE_BACKGROUND && !isAppUseControlChanged) {
                return;
            }
            session->sessionStage_->NotifyAppUseControlStatus(isAppUseControl);
        }
    }, __func__);
}

void SceneSession::UpdatePrivacyModeControlInfo()
{
    bool isPrivacyMode = false;
    auto property = GetSessionProperty();
    if ((property && property->GetPrivacyMode()) || HasChildSessionInPrivacyMode()) {
        isPrivacyMode = true;
    }
    ControlInfo controlInfo;
    if (!isPrivacyMode && !GetAppControlInfo(ControlAppType::PRIVACY_WINDOW, controlInfo)) {
        TLOGI(WmsLogTag::WMS_LIFE, "no need to update privacy mode control info");
        return;
    }
    controlInfo = { .isNeedControl = isPrivacyMode, .isControlRecentOnly = true };
    NotifyUpdateAppUseControl(ControlAppType::PRIVACY_WINDOW, controlInfo);
}

bool SceneSession::HasChildSessionInPrivacyMode()
{
    for (const auto& subSession : GetSubSession()) {
        if (subSession == nullptr) {
            TLOGW(WmsLogTag::WMS_LIFE, "subSession is nullptr");
            continue;
        }
        auto property = subSession->GetSessionProperty();
        if (property && property->GetPrivacyMode()) {
            return true;
        }
        if (subSession->HasChildSessionInPrivacyMode()) {
            return true;
        }
    }
    for (const auto& dialogSession : GetDialogVector()) {
        if (dialogSession == nullptr) {
            TLOGW(WmsLogTag::WMS_LIFE, "dialogSession is nullptr");
            continue;
        }
        auto property = dialogSession->GetSessionProperty();
        if (property && property->GetPrivacyMode()) {
            return true;
        }
        auto dialogSceneSession = GetSceneSessionById(dialogSession->GetPersistentId());
        if (dialogSceneSession && dialogSceneSession->HasChildSessionInPrivacyMode()) {
            return true;
        }
    }
    return false;
}

void SceneSession::RegisterDefaultAnimationFlagChangeCallback(NotifyWindowAnimationFlagChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onWindowAnimationFlagChange_ = std::move(callback);
        session->onWindowAnimationFlagChange_(session->IsNeedDefaultAnimation());
    }, __func__);
}

void SceneSession::RegisterDefaultDensityEnabledCallback(NotifyDefaultDensityEnabledFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onDefaultDensityEnabledFunc_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterWindowShadowEnableChangeCallback(NotifyWindowShadowEnableChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !callback) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s session or callback is null", where);
            return;
        }
        session->onWindowShadowEnableChangeFunc_ = std::move(callback);
        session->onWindowShadowEnableChangeFunc_(session->GetSessionProperty()->GetWindowShadowEnabled());
    }, __func__);
}

void SceneSession::RegisterNeedAvoidCallback(NotifyNeedAvoidFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return;
        }
        session->onNeedAvoid_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterSystemBarPropertyChangeCallback(NotifySystemBarPropertyChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onSystemBarPropertyChange_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterTouchOutsideCallback(NotifyTouchOutsideFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onTouchOutside_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterFollowScreenChangeCallback(NotifyFollowScreenChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !session->specificCallback_ || !callback) {
            TLOGNE(WmsLogTag::DEFAULT, "%{public}s session or specific callback or callback is null", where);
            return;
        }
        session->specificCallback_->onUpdateFollowScreenChange_ = std::move(callback);
        session->specificCallback_->onUpdateFollowScreenChange_(session->GetFollowScreenChange());
    }, __func__);
}

void SceneSession::RegisterSnapshotSkipChangeCallback(NotifySnapshotSkipChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !callback) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s session or callback is null", where);
            return;
        }
        session->onSnapshotSkipChangeFunc_ = std::move(callback);
        session->onSnapshotSkipChangeFunc_(session->GetSessionProperty()->GetSnapshotSkip());
    }, __func__);
}

WSError SceneSession::SetGlobalMaximizeMode(MaximizeMode mode)
{
    return PostSyncTask([weakThis = wptr(this), mode, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGND(WmsLogTag::WMS_PC, "%{public}s mode: %{public}u", where, static_cast<uint32_t>(mode));
        session->maximizeMode_ = mode;
        ScenePersistentStorage::Insert("maximize_state", static_cast<int32_t>(session->maximizeMode_),
            ScenePersistentStorageType::MAXIMIZE_STATE);
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSession::GetGlobalMaximizeMode(MaximizeMode& mode)
{
    return PostSyncTask([weakThis = wptr(this), &mode, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        mode = maximizeMode_;
        TLOGND(WmsLogTag::WMS_PC, "%{public}s mode: %{public}u", where, static_cast<uint32_t>(mode));
        return WSError::WS_OK;
    }, __func__);
}

/** @note @window.layout */
WSError SceneSession::SetAspectRatio(float ratio)
{
    return SetContentAspectRatio(ratio, true, true);
}

WSError SceneSession::SetContentAspectRatio(float ratio, bool isPersistent, bool needUpdateRect)
{
    TLOGI(WmsLogTag::WMS_LAYOUT,
        "windowId: %{public}d, ratio: %{public}f, isPersistent: %{public}d, needUpdateRect: %{public}d",
        GetPersistentId(), ratio, isPersistent, needUpdateRect);
    return PostSyncTask([weakThis = wptr(this), ratio, isPersistent, needUpdateRect, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        auto property = session->GetSessionProperty();
        if (!property) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: property is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        WindowLimits limits = property->GetWindowLimits();
        WindowDecoration decoration = session->GetWindowDecoration();
        if (!SessionUtils::IsAspectRatioValid(ratio, limits, decoration)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "%{public}s: Invalid ratio: %{public}f", where, ratio);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        session->Session::SetAspectRatio(ratio);
        if (isPersistent) {
            session->SaveAspectRatio(ratio);
        }
        if (needUpdateRect) {
            WSRect originalRect = session->GetSessionRect();
            WSRect adjustedRect = session->layoutController_->AdjustRectByAspectRatio(originalRect, decoration);
            if (adjustedRect != originalRect) {
                session->NotifySessionRectChange(adjustedRect, SizeChangeReason::RESIZE);
                TLOGNI(WmsLogTag::WMS_LAYOUT,
                    "%{public}s: windowId: %{public}d, originalRect: %{public}s, adjustedRect: %{public}s",
                    where, session->GetPersistentId(),
                    originalRect.ToString().c_str(), adjustedRect.ToString().c_str());
            }
        }
        return WSError::WS_OK;
    }, __func__);
}

/** @note @window.layout */
bool SceneSession::ShouldSkipUpdateRect(const WSRect& rect)
{
    const auto persistentId = GetPersistentId();
    if (rect.IsInvalid()) {
        TLOGNE(WmsLogTag::WMS_LAYOUT, "id:%{public}d rect:%{public}s is invalid", persistentId,
            rect.ToString().c_str());
        return true;
    }
    // to solve the problem that setBounds is frequently invoked during the drag process
    if (GetSizeChangeReason() == SizeChangeReason::DRAG) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "skip drag reason update id:%{public}d rect:%{public}s",
            persistentId, rect.ToString().c_str());
        return true;
    }
    // the application can recongnize the end of the drag and perform some operations for layout refreshing
    if (GetSessionRect() == rect && GetSizeChangeReason() != SizeChangeReason::DRAG_END &&
        GetWindowType() != WindowType::WINDOW_TYPE_KEYBOARD_PANEL &&
        GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        // the session with an empty handle is not an application window and does not have a client. Therefore, clientRect does not need to be checked.
        if (!sessionStage_) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "sessionStage is null, skip same rect update id:%{public}d rect:%{public}s",
                persistentId, rect.ToString().c_str());
            return true;
        }
        if (GetClientRect() == rect) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "skip same rect update id:%{public}d rect:%{public}s clientRect:%{public}s",
                persistentId, rect.ToString().c_str(), GetClientRect().ToString().c_str());
            return true;
        }
    }
    return false;
}

/** @note @window.layout */
bool SceneSession::ShouldSkipUpdateRectNotify(const WSRect& rect)
{
    // position change no need to notify client, since frame layout finish will notify
    if (rect.IsSizeEqual(GetSessionRect()) && (GetSizeChangeReason() != SizeChangeReason::DRAG_MOVE ||
        !rectChangeListenerRegistered_)) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "position change no need notify client id:%{public}d, "
            "rect:%{public}s, preRect:%{public}s",
            GetPersistentId(), rect.ToString().c_str(), GetSessionRect().ToString().c_str());
        return true;
    }
    return false;
}

/** @note @window.layout */
WSError SceneSession::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction)
{
    PostTask([weakThis = wptr(this), rect, reason, rsTransaction, updateReason, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return;
        }
        // check whether to update rect
        if (session->ShouldSkipUpdateRect(rect)) {
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: no need to update rect", where);
            return;
        }

        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::UpdateRect %d [%d, %d, %u, %u]",
            session->GetPersistentId(), rect.posX_, rect.posY_, rect.width_, rect.height_);
        session->SetWinRectWhenUpdateRect(rect);
        // check whether to notify the client rect update
        if (session->ShouldSkipUpdateRectNotify(rect)) {
            session->NotifyClientToUpdateRect(updateReason, rsTransaction);
        }
        session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
        session->AddPropertyDirtyFlags(static_cast<uint32_t>(SessionPropertyFlag::WINDOW_RECT));
        TLOGI_LMTBYID(TEN_SECONDS, RECORD_100_TIMES, session->GetPersistentId(), WmsLogTag::WMS_LAYOUT,
            "%{public}s: id:%{public}d, reason:%{public}d %{public}s rect:win=%{public}s "
            "client=%{public}s", where, session->GetPersistentId(), session->GetSizeChangeReason(),
            updateReason.c_str(), rect.ToString().c_str(), session->GetClientRect().ToString().c_str());
    }, __func__ + GetRectInfo(rect));
    return WSError::WS_OK;
}

/** @note @window.layout */
void SceneSession::SetWinRectWhenUpdateRect(const WSRect& rect)
{
    WSRect newRect;
    if (GetIsMidScene() && rect.posX_ == 0 && rect.posY_ == 0) {
        WSRect curRect = GetSessionRect();
        newRect = { curRect.posX_, curRect.posY_, rect.width_, rect.height_ };
    } else {
        newRect = rect;
    }
    layoutController_->SetSessionRect(newRect);
}

WSError SceneSession::NotifyClientToUpdateRectTask(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    WSRect winRect = GetSessionRect();
    SizeChangeReason reason = GetSizeChangeReason();
    TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, reason:%{public}d, rect:%{public}s",
        GetPersistentId(), reason, winRect.ToString().c_str());
    bool isMoveOrDrag = moveDragController_ &&
        (moveDragController_->GetStartDragFlag() || moveDragController_->GetStartMoveFlag());
    if (isMoveOrDrag && reason == SizeChangeReason::UNDEFINED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "skip redundant rect update!");
        return WSError::WS_ERROR_REPEAT_OPERATION;
    }
    if (reason != SizeChangeReason::DRAG_MOVE) {
        UpdateCrossAxisOfLayout(winRect);
        if (reason != SizeChangeReason::DRAG) {
            UpdatePrivateStateOfLayout(winRect);
        }
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "SceneSession::NotifyClientToUpdateRect%d [%d, %d, %u, %u] reason:%u",
        GetPersistentId(), winRect.posX_, winRect.posY_, winRect.width_, winRect.height_, reason);

    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    if (GetForegroundInteractiveStatus()) {
        GetAllAvoidAreas(avoidAreas);
    } else {
        TLOGD(WmsLogTag::WMS_IMMS, "win [%{public}d] avoid area update rejected by recent", GetPersistentId());
    }
    if (winRect.IsInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "id:%{public}d rect:%{public}s is invalid",
            GetPersistentId(), winRect.ToString().c_str());
        return WSError::WS_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }
    WSError ret = WSError::WS_OK;
    // once reason is undefined, not use rsTransaction
    // when rotation, sync cnt++ in marshalling. Although reason is undefined caused by resize
    if (reason == SizeChangeReason::UNDEFINED || reason == SizeChangeReason::RESIZE || IsMoveToOrDragMove(reason)) {
        ret = Session::UpdateRectWithLayoutInfo(GetSessionRect(), reason, updateReason, nullptr, avoidAreas);
    } else {
        ret = Session::UpdateRectWithLayoutInfo(GetSessionRect(), reason, updateReason, rsTransaction, avoidAreas);
#ifdef DEVICE_STATUS_ENABLE
        // When the drag is in progress, the drag window needs to be notified to rotate.
        if (rsTransaction != nullptr) {
            RotateDragWindow(rsTransaction);
        }
#endif // DEVICE_STATUS_ENABLE
    }

    return ret;
}

WSError SceneSession::NotifyClientToUpdateRect(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    PostTask([weakThis = wptr(this), rsTransaction, updateReason, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WSError ret = session->NotifyClientToUpdateRectTask(updateReason, rsTransaction);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        return ret;
    }, __func__);
    return WSError::WS_OK;
}

bool SceneSession::GetScreenWidthAndHeightFromClient(const sptr<WindowSessionProperty>& sessionProperty,
    uint32_t& screenWidth, uint32_t& screenHeight)
{
    auto defaultDisplayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplayInfo != nullptr) {
        screenWidth = static_cast<uint32_t>(defaultDisplayInfo->GetWidth());
        screenHeight = static_cast<uint32_t>(defaultDisplayInfo->GetHeight());
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "defaultDisplayInfo is null, get screenWidthAndHeight failed");
        return false;
    }
    if (IsSystemKeyboard() && DisplayManager::GetInstance().GetFoldStatus() == FoldStatus::HALF_FOLD) {
        screenHeight = static_cast<uint32_t>(defaultDisplayInfo->GetPhysicalHeight());
        TLOGI(WmsLogTag::WMS_KEYBOARD, "id: %{public}d, display is half-fold", GetPersistentId());
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "screenSize: [%{public}d, %{public}d]", screenWidth, screenHeight);
    return true;
}

void SceneSession::SetSessionRectChangeCallback(const NotifySessionRectChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->sessionRectChangeFunc_ = func;
        if (session->sessionRectChangeFunc_ && session->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            auto reason = SizeChangeReason::UNDEFINED;
            auto rect = session->GetSessionRequestRect();
            if (rect.width_ == 0 && rect.height_ == 0) {
                reason = SizeChangeReason::MOVE;
            }
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s, winName:%{public}s, reason:%{public}d, rect:%{public}s",
                where, session->GetWindowName().c_str(), reason, rect.ToString().c_str());
            if (session->GetClientDisplayId() == VIRTUAL_DISPLAY_ID && rect.posY_ == 0) {
                rect.posY_ += PcFoldScreenManager::GetInstance().GetVirtualDisplayPosY();
            }
            auto rectAnimationConfig = session->GetRequestRectAnimationConfig();
            session->sessionRectChangeFunc_(rect, reason, DISPLAY_ID_INVALID, rectAnimationConfig);
        }
        return WSError::WS_OK;
    }, __func__);
}

void SceneSession::SetSessionWindowLimitsChangeCallback(const NotifySessionWindowLimitsChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        session->sessionWindowLimitsChangeFunc_ = func;
        if (session->sessionWindowLimitsChangeFunc_) {
            const auto sessionProperty = session->GetSessionProperty();
            WindowLimits windowLimits = sessionProperty->GetWindowLimits();
            WindowLimits windowLimitsVP = sessionProperty->GetWindowLimitsVP();
            WindowLimits userWindowLimits = sessionProperty->GetUserWindowLimits();
            bool useVPLimits = (userWindowLimits.pixelUnit_ == PixelUnit::VP);
            const WindowLimits& limitsToNotify = useVPLimits ? windowLimitsVP : windowLimits;
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s, id:%{public}d, px:%{public}s, vp:%{public}s, "
                "userLimitsUnit:%{public}u", where, session->GetPersistentId(),
                windowLimits.ToString().c_str(), windowLimitsVP.ToString().c_str(), userWindowLimits.pixelUnit_);
            session->sessionWindowLimitsChangeFunc_(limitsToNotify);
        }
    }, __func__);
}

void SceneSession::SetSessionDisplayIdChangeCallback(NotifySessionDisplayIdChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session or display id is null", where);
            return;
        }
        session->sessionDisplayIdChangeFunc_ = std::move(func);
    }, __func__);
}

void SceneSession::SetMainWindowTopmostChangeCallback(NotifyMainWindowTopmostChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s session or func is null", where);
            return;
        }
        session->mainWindowTopmostChangeFunc_ = std::move(func);
    }, __func__);
}

void SceneSession::SetTitleAndDockHoverShowChangeCallback(NotifyTitleAndDockHoverShowChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session or TitleAndDockHoverShowChangeFunc is null", where);
            return;
        }
        session->onTitleAndDockHoverShowChangeFunc_ = std::move(func);
        TLOGND(WmsLogTag::WMS_LAYOUT_PC, "%{public}s id: %{public}d",
            where, session->GetPersistentId());
    }, __func__);
}

void SceneSession::SetRestoreMainWindowCallback(NotifyRestoreMainWindowFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session or RestoreMainWindowFunc is null", where);
            return;
        }
        session->onRestoreMainWindowFunc_ = std::move(func);
        TLOGND(WmsLogTag::WMS_LAYOUT_PC, "%{public}s id: %{public}d",
            where, session->GetPersistentId());
    }, __func__);
}

void SceneSession::SetAdjustKeyboardLayoutCallback(const NotifyKeyboardLayoutAdjustFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session or keyboardLayoutFunc is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->adjustKeyboardLayoutFunc_ = func;
        auto property = session->GetSessionProperty();
        if (property == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s property is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        KeyboardLayoutParams params = property->GetKeyboardLayoutParams();
        session->adjustKeyboardLayoutFunc_(params);
        TLOGNI(WmsLogTag::WMS_KEYBOARD,
            "%{public}s Notify adjust keyboard layout when register, keyboardId: %{public}d, "
            "gravity: %{public}u, landscapeAvoidHeight: %{public}d, PortraitAvoidHeight: %{public}d, "
            "LandscapeKeyboardRect: %{public}s, PortraitKeyboardRect: %{public}s, "
            "LandscapePanelRect: %{public}s, PortraitPanelRect: %{public}s", where, session->GetPersistentId(),
            static_cast<uint32_t>(params.gravity_), params.landscapeAvoidHeight_, params.portraitAvoidHeight_,
            params.LandscapeKeyboardRect_.ToString().c_str(), params.PortraitKeyboardRect_.ToString().c_str(),
            params.LandscapePanelRect_.ToString().c_str(), params.PortraitPanelRect_.ToString().c_str());
        return WSError::WS_OK;
    }, __func__);
}

void SceneSession::SetSessionPiPControlStatusChangeCallback(const NotifySessionPiPControlStatusChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->sessionPiPControlStatusChangeFunc_ = func;
        return WSError::WS_OK;
    }, __func__);
}

void SceneSession::SetUpdatePiPTemplateInfoCallback(NotifyUpdatePiPTemplateInfoFunc&& func)
{
    auto task = [weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null", where);
            return;
        }
        session->updatePiPTemplateInfoCallbackFunc_ = std::move(func);
    };
    PostTask(task, __func__);
}

WSError SceneSession::UpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo)
{
    TLOGI(WmsLogTag::WMS_PIP, "UpdatePiPTemplateInfo, pipTemplateType: %{public}u, priority: %{public}d, "
        "defaultWindowSizeType: %{public}d, cornerAdsorptionEnabled: %{public}d", pipTemplateInfo.pipTemplateType,
        pipTemplateInfo.priority, pipTemplateInfo.defaultWindowSizeType, pipTemplateInfo.cornerAdsorptionEnabled);
    auto task = [weakThis = wptr(this), pipTemplateInfo = std::move(pipTemplateInfo), where = __func__]() mutable {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null or is terminating", where);
            return;
        }
        if (session->updatePiPTemplateInfoCallbackFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::UpdatePiPTemplateInfo");
            session->updatePiPTemplateInfoCallbackFunc_(pipTemplateInfo);
        }
    };
    PostTask(task, __func__);
    return WSError::WS_OK;
}

void SceneSession::SetAutoStartPiPStatusChangeCallback(const NotifyAutoStartPiPStatusChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null", where);
            return;
        }
        session->autoStartPiPStatusChangeFunc_ = func;
    }, __func__);
}

void SceneSession::SetPipParentWindowIdCallback(NotifySetPipParentWindowIdFunc&& func)
{
    auto task = [weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null", where);
            return;
        }
        session->setPipParentWindowIdFunc_ = std::move(func);
    };
    PostTask(task, __func__);
}

WSError SceneSession::SetPipParentWindowId(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "PipParentWindowId: %{public}u", windowId);
    auto task = [weakThis = wptr(this), windowId, where = __func__]() mutable {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null or is terminating", where);
            return;
        }
        if (session->setPipParentWindowIdFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetPipParentWindowId");
            session->setPipParentWindowIdFunc_(windowId);
        }
    };
    PostTask(task, __func__);
    return WSError::WS_OK;
}

WMError SceneSession::IsPiPActive(bool& status)
{
    status = pipActiveStatus_.load();
    return WMError::WM_OK;
}

/** @note @window.layout */
// set rect info and notify rect change who is register call back
void SceneSession::UpdateSessionRectInner(const WSRect& rect, SizeChangeReason reason,
    const MoveConfiguration& moveConfiguration, const RectAnimationConfig& rectAnimationConfig)
{
    auto newWinRect = GetSessionRect();
    auto newRequestRect = GetSessionRequestRect();
    bool isScbCoreEnabled = Session::IsScbCoreEnabled();
    if (reason == SizeChangeReason::MOVE || reason == SizeChangeReason::MOVE_WITH_ANIMATION) {
        newWinRect.posX_ = rect.posX_;
        newWinRect.posY_ = rect.posY_;
        newRequestRect.posX_ = rect.posX_;
        newRequestRect.posY_ = rect.posY_;
        if (!isScbCoreEnabled && !WindowHelper::IsMainWindow(GetWindowType())) {
            SetSessionRect(newWinRect);
        }
        SetSessionRequestRect(newRequestRect);
        SetRequestRectAnimationConfig(moveConfiguration.rectAnimationConfig);
        NotifySessionRectChange(newRequestRect, reason, moveConfiguration.displayId,
            moveConfiguration.rectAnimationConfig);
    } else if (reason == SizeChangeReason::RESIZE || reason == SizeChangeReason::RESIZE_WITH_ANIMATION) {
        if (rect.width_ > 0 && rect.height_ > 0) {
            newWinRect.width_ = rect.width_;
            newWinRect.height_ = rect.height_;
            newRequestRect.width_ = rect.width_;
            newRequestRect.height_ = rect.height_;
        }
        if (!isScbCoreEnabled && GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            SetSessionRect(newWinRect);
        }
        SetRequestRectAnimationConfig(rectAnimationConfig);
        DisplayId displayId = GetSessionProperty() != nullptr ? GetSessionProperty()->GetDisplayId() :
            DISPLAY_ID_INVALID;
        TLOGI(WmsLogTag::WMS_LAYOUT, "Get displayId: %{public}" PRIu64, displayId);
        auto notifyRect = newRequestRect;
        if (PcFoldScreenManager::GetInstance().IsHalfFolded(GetScreenId())) {
            notifyRect = rect;
        }
        SetSessionRequestRect(notifyRect);
        NotifySessionRectChange(notifyRect, reason, displayId, rectAnimationConfig);
    } else {
        if (!isScbCoreEnabled) {
            SetSessionRect(rect);
        }
        NotifySessionRectChange(rect, reason);
    }
    TLOGI_LMTBYID(TEN_SECONDS, RECORD_100_TIMES, GetPersistentId(), WmsLogTag::WMS_LAYOUT,
        "Id:%{public}d reason:%{public}d cfg:%{public}s rects:in=%{public}s "
        "newReq=%{public}s newWin=%{public}s", GetPersistentId(), reason,
        moveConfiguration.ToString().c_str(), rect.ToString().c_str(), newRequestRect.ToString().c_str(),
        newWinRect.ToString().c_str());
}

void SceneSession::UpdateSessionRectPosYFromClient(SizeChangeReason reason, DisplayId& configDisplayId, WSRect& rect)
{
    if (!PcFoldScreenManager::GetInstance().IsHalfFolded(GetScreenId()) ||
        PcFoldScreenManager::GetInstance().HasSystemKeyboard()) {
        TLOGD(
            WmsLogTag::WMS_LAYOUT, "winId: %{public}d, displayId: %{public}" PRIu64, GetPersistentId(), GetScreenId());
        return;
    }
    if (reason != SizeChangeReason::RESIZE) {
        configDisplayId_ = configDisplayId;
    }
    if (configDisplayId_ != DISPLAY_ID_INVALID &&
        !PcFoldScreenManager::GetInstance().IsPcFoldScreen(configDisplayId_)) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "winId: %{public}d, configDisplayId: %{public}" PRIu64,
            GetPersistentId(), configDisplayId_);
        return;
    }
    auto clientDisplayId = clientDisplayId_;
    TLOGI(WmsLogTag::WMS_LAYOUT, "winId: %{public}d, input: %{public}s, screenId: %{public}" PRIu64
        ", clientDisplayId: %{public}" PRIu64 ", configDisplayId: %{public}" PRIu64,
        GetPersistentId(), rect.ToString().c_str(), GetScreenId(), clientDisplayId, configDisplayId_);
    if (configDisplayId_ != VIRTUAL_DISPLAY_ID && clientDisplayId != VIRTUAL_DISPLAY_ID) {
        return;
    }
    if (rect.posY_ >= 0) {
        const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
            PcFoldScreenManager::GetInstance().GetDisplayRects();
        TLOGI(WmsLogTag::WMS_LAYOUT, "winId: %{public}d, defaultDisplayRect: %{public}s, virtualDisplayRect: %{public}s"
            ", foldCreaseRect: %{public}s", GetPersistentId(), defaultDisplayRect.ToString().c_str(),
            virtualDisplayRect.ToString().c_str(), foldCreaseRect.ToString().c_str());
        auto lowerScreenPosY = defaultDisplayRect.height_ + foldCreaseRect.height_;
        if (rect.posY_ < lowerScreenPosY) {
            rect.posY_ += lowerScreenPosY;
        }
    } else {
        rect.posY_ += GetSessionRect().posY_;
    }
    configDisplayId = DEFAULT_DISPLAY_ID;
    TLOGI(WmsLogTag::WMS_LAYOUT, "winId: %{public}d, output: %{public}s", GetPersistentId(), rect.ToString().c_str());
}

/** @note @window.layout */
WSError SceneSession::UpdateSessionRect(
    const WSRect& rect, SizeChangeReason reason, bool isGlobal, bool isFromMoveToGlobal,
    const MoveConfiguration& moveConfiguration, const RectAnimationConfig& rectAnimationConfig)
{
    bool isMoveOrResize = reason == SizeChangeReason::MOVE || reason == SizeChangeReason::RESIZE ||
        reason == SizeChangeReason::MOVE_WITH_ANIMATION || reason == SizeChangeReason::RESIZE_WITH_ANIMATION;
    if (isMoveOrResize && GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        return WSError::WS_DO_NOTHING;
    }
    WSRect newRect = rect;
    MoveConfiguration newMoveConfiguration = moveConfiguration;
    UpdateSessionRectPosYFromClient(reason, newMoveConfiguration.displayId, newRect);
    bool isAvailableWindow = (systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow()) && !IsFreeMultiWindowMode();
    bool needAdjust = WindowHelper::IsSubWindow(Session::GetWindowType()) && isAvailableWindow;
    // rect update for arkui pop-up window
    if (isGlobal && needAdjust) {
        if (auto mainSession = GetMainSession()) {
            auto mainRect = mainSession->GetSessionRect();
            if (!CheckIfRectElementIsTooLarge(mainRect)) {
                newRect.posX_ -= mainRect.posX_;
                newRect.posY_ -= mainRect.posY_;
            }
        }
    }
    // moving window based on screen coordinates
    if (isFromMoveToGlobal && needAdjust) {
        auto mainSession = GetMainSession();
        if (mainSession && mainSession->GetFloatingScale() != 0) {
            Rect mainGlobalRect;
            WMError errorCode = mainSession->GetGlobalScaledRect(mainGlobalRect);
            newRect.posX_ = (newRect.posX_ - mainGlobalRect.posX_) / mainSession->GetFloatingScale();
            newRect.posY_ = (newRect.posY_ - mainGlobalRect.posY_) / mainSession->GetFloatingScale();
        }
    }
    PostTask([weakThis = wptr(this), newRect, reason, newMoveConfiguration, rectAnimationConfig, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->SetRequestMoveConfiguration(newMoveConfiguration);
        session->UpdateSessionRectInner(newRect, reason, newMoveConfiguration, rectAnimationConfig);
        return WSError::WS_OK;
    }, __func__ + GetRectInfo(rect));
    return WSError::WS_OK;
}

WSError SceneSession::UpdateGlobalDisplayRectFromClient(const WSRect& rect, SizeChangeReason reason)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, rect: %{public}s, reason: %{public}u",
          GetPersistentId(), rect.ToString().c_str(), reason);
    PostTask([weakThis = wptr(this), rect, reason, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is nullptr", where);
            return;
        }
        if (rect == session->GetGlobalDisplayRect()) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s: windowId: %{public}d skip same rect",
                where, session->GetPersistentId());
            return;
        }
        // Convert global coordinates to screen-relative coordinates to be
        // compatible with the original logic of UpdateSessionRectInner.
        const auto& [screenId, screenRelativeRect] =
            SessionCoordinateHelper::GlobalToScreenRelativeRect(session->GetScreenId(), rect);
        RectAnimationConfig animConfig;
        MoveConfiguration moveConfig = { screenId, animConfig };
        session->SetRequestMoveConfiguration(moveConfig);
        session->UpdateSessionRectInner(screenRelativeRect, reason, moveConfig, animConfig);
    }, __func__ + GetRectInfo(rect));
    return WSError::WS_OK;
}

void SceneSession::HandleCrossMoveTo(WSRect& globalRect)
{
    HandleCrossMoveToSurfaceNode(globalRect);
    SetSurfaceBounds(globalRect, true, true);
}

CrossPlaneState SceneSession::UpdateCrossPlaneState(const WSRect &rect)
{
    if (!PcFoldScreenManager::GetInstance().IsHalfFolded(GetScreenId()) ||
        PcFoldScreenManager::GetInstance().HasSystemKeyboard()) {
        return CrossPlaneState::CROSS_DEFAULT_PLANE;
    }
    const auto &[defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
        PcFoldScreenManager::GetInstance().GetDisplayRects();
    if (rect.IsOverlap(defaultDisplayRect) && rect.IsOverlap(foldCreaseRect) && rect.IsOverlap(virtualDisplayRect)) {
        return CrossPlaneState::CROSS_ALL_PLANE;
    }
    if (rect.IsOverlap(defaultDisplayRect) && rect.IsOverlap(foldCreaseRect)) {
        return CrossPlaneState::CROSS_DEFAULT_CREASE_PLANE;
    }
    if (rect.IsOverlap(foldCreaseRect) && rect.IsOverlap(virtualDisplayRect)) {
        return CrossPlaneState::CROSS_VIRTUAL_CREASE_PLANE;
    }
    if (rect.IsOverlap(defaultDisplayRect)) {
        return CrossPlaneState::CROSS_DEFAULT_PLANE;
    }
    if (rect.IsOverlap(virtualDisplayRect)) {
        return CrossPlaneState::CROSS_VIRTUAL_PLANE;
    }
    if (rect.IsOverlap(foldCreaseRect)) {
        return CrossPlaneState::CROSS_CREASE_PLANE;
    }
    return CrossPlaneState::CROSS_DEFAULT_PLANE;
}

void SceneSession::UpdatePrivateStateOfLayout(const WSRect &rect)
{
    bool notifyPrivacy = false;
    CrossPlaneState crossPlaneState = UpdateCrossPlaneState(rect);
    notifyPrivacy = crossPlaneState_ != static_cast<uint32_t>(crossPlaneState);
    crossPlaneState_ = static_cast<uint32_t>(crossPlaneState);

    TLOGD(WmsLogTag::WMS_LAYOUT, "notifyPrivacy: %{public}u, winId: %{public}d, rect: %{public}s"
        ", crossPlaneState: %{public}u", notifyPrivacy, GetPersistentId(), rect.ToString().c_str(), crossPlaneState);
    
    if (notifyPrivacy && updatePrivateStateAndNotifyFunc_) {
        updatePrivateStateAndNotifyFunc_(GetPersistentId());
    } else if (updatePrivateStateAndNotifyFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "updatePrivateStateAndNotifyFunc_ is null, id: %{public}d", GetPersistentId() );
    }
}

void SceneSession::UpdateCrossAxisOfLayout(const WSRect& rect)
{
    const int FOLD_CREASE_TYPE = 2;
    isCrossAxisOfLayout_ = rect.IsOverlap(std::get<FOLD_CREASE_TYPE>(
        PcFoldScreenManager::GetInstance().GetDisplayRects()));
    UpdateCrossAxis();
}

void SceneSession::UpdateCrossAxis()
{
    CrossAxisState crossAxisState = CrossAxisState::STATE_INVALID;
    if (PcFoldScreenManager::GetInstance().GetDisplayId() == SCREEN_ID_INVALID ||
        PcFoldScreenManager::GetInstance().GetDisplayId() != GetSessionProperty()->GetDisplayId()) {
        return;
    }
    SuperFoldStatus foldStatus = PcFoldScreenManager::GetInstance().GetScreenFoldStatus();
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d, status %{public}d, cross %{public}d", GetPersistentId(),
        foldStatus, isCrossAxisOfLayout_.load());
    if (foldStatus != SuperFoldStatus::UNKNOWN) {
        if (foldStatus == SuperFoldStatus::HALF_FOLDED && isCrossAxisOfLayout_) {
            crossAxisState = CrossAxisState::STATE_CROSS;
        } else {
            crossAxisState = CrossAxisState::STATE_NO_CROSS;
        }
    }
    if (crossAxisState_ != static_cast<uint32_t>(crossAxisState) && sessionStage_ != nullptr) {
        crossAxisState_ = static_cast<uint32_t>(crossAxisState);
        sessionStage_->NotifyWindowCrossAxisChange(crossAxisState);
    } else if (sessionStage_ == nullptr) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "sessionStage_ is nullptr, id: %{public}d", GetPersistentId());
    }
}

WSError SceneSession::GetCrossAxisState(CrossAxisState& state)
{
    state = static_cast<CrossAxisState>(crossAxisState_.load());
    return WSError::WS_OK;
}

/** @note @window.layout */
WSError SceneSession::UpdateClientRect(const WSRect& rect)
{
    PostTask([weakThis = wptr(this), rect, funcName = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", funcName);
            return;
        }
        if (rect.IsInvalid()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d rect:%{public}s is invalid",
                funcName, session->GetPersistentId(), rect.ToString().c_str());
            return;
        }
        if (rect == session->GetClientRect()) {
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d skip same rect",
                funcName, session->GetPersistentId());
            return;
        }
        session->SetClientRect(rect);
    }, __func__ + GetRectInfo(rect));
    return WSError::WS_OK;
}

void SceneSession::NotifySingleHandTransformChange(const SingleHandTransform& singleHandTransform)
{
    if (!IsSessionForeground() && !IsVisible()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, session is not foreground and not visible!", GetPersistentId());
        return;
    }
    if (sessionStage_ != nullptr) {
        sessionStage_->NotifySingleHandTransformChange(singleHandTransform);
    }
}

void SceneSession::NotifyGlobalScaledRectChange()
{
    if (!IsSessionForeground()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, session is not foreground!", GetPersistentId());
        return;
    }
    if (sessionStage_ != nullptr) {
        Rect globalScaledRect;
        WMError errorCode = GetGlobalScaledRect(globalScaledRect);
        if (errorCode != WMError::WM_OK) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, errCode:%{public}d", GetPersistentId(), errorCode);
            return;
        }
        sessionStage_->NotifyGlobalScaledRectChange(globalScaledRect);
    }
}

void SceneSession::RegisterRaiseToTopCallback(NotifyRaiseToTopFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onRaiseToTop_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterRaiseAboveTargetCallback(NotifyRaiseAboveTargetFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onRaiseAboveTarget_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterRaiseMainWindowAboveTargetCallback(NotifyRaiseMainWindowAboveTargetFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onRaiseMainWindowAboveTarget_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterSessionTopmostChangeCallback(NotifySessionTopmostChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onSessionTopmostChange_ = std::move(callback);
        session->onSessionTopmostChange_(session->IsTopmost());
    }, __func__);
}

/** @note @window.hierarchy */
void SceneSession::RegisterSubSessionZLevelChangeCallback(NotifySubSessionZLevelChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onSubSessionZLevelChange_ = std::move(callback);
        session->onSubSessionZLevelChange_(session->GetSubWindowZLevel());
    }, __func__);
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseToAppTop()
{
    return PostSyncTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->onRaiseToTop_) {
            TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s id: %{public}d", where, session->GetPersistentId());
            session->onRaiseToTop_();
            session->SetMainSessionUIStateDirty(true);
        }
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSession::RestartApp(const std::shared_ptr<AAFwk::Want>& want)
{
    if (want == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "want is null");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    return PostSyncTask([weakThis = wptr(this), want, callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!SessionHelper::IsMainWindow(session->GetWindowType())) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: session is not main window, id:%{public}d",
                where, session->GetPersistentId());
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        if (!session->IsSessionForeground() || !session->GetForegroundInteractiveStatus()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: session is not foreground, id:%{public}d",
                where, session->GetPersistentId());
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (session->sessionInfo_.bundleName_ != want->GetElement().GetBundleName()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: not the same app, ability:%{public}s, target:%{public}s",
                where, session->sessionInfo_.bundleName_.c_str(), want->GetElement().GetBundleName().c_str());
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        AppExecFwk::AbilityInfo abilityInfo;
        if (!session->CheckAndGetAbilityInfoByWant(want, abilityInfo)) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: ability info is null, ability name:%{public}s",
                where, want->GetElement().GetAbilityName().c_str());
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        want->SetModuleName(abilityInfo.moduleName);
        SessionInfo info = GetSessionInfoByWant(want, session);
        if (info.isRestartApp_) {
            session->NotifyRestart();
        }
        if (session->restartAppFunc_) {
            session->restartAppFunc_(info, callingPid);
        }
        return WSError::WS_OK;
    }, __func__);
}

SessionInfo SceneSession::GetSessionInfoByWant(const std::shared_ptr<AAFwk::Want>& want,
    const sptr<SceneSession>& session)
{
    SessionInfo info;
    if ((session->sessionInfo_.moduleName_ == want->GetElement().GetModuleName() &&
        session->sessionInfo_.abilityName_ == want->GetElement().GetAbilityName()) ||
        session->sessionInfo_.isAtomicService_) {
        session->sessionInfo_.want = want;
        session->sessionInfo_.isRestartApp_ = true;
        session->sessionInfo_.restartCallerPersistentId_ = INVALID_SESSION_ID;
        info = session->sessionInfo_;
    } else {
        info.abilityName_ = want->GetElement().GetAbilityName();
        info.bundleName_ = want->GetElement().GetBundleName();
        info.moduleName_ = want->GetElement().GetModuleName();
        int32_t appCloneIndex = want->GetIntParam(APP_CLONE_INDEX, 0);
        info.appIndex_ = appCloneIndex == 0 ? want->GetIntParam(DLP_INDEX, 0) : appCloneIndex;
        info.appInstanceKey_ = want->GetStringParam(AAFwk::Want::APP_INSTANCE_KEY);
        TLOGI(WmsLogTag::WMS_LIFE, "the new session info, appindex:%{public}d, appInstanceKey:%{public}s",
            info.appIndex_, info.appInstanceKey_.c_str());
        info.callerPersistentId_ = session->GetPersistentId();
        info.want = want;
        info.restartCallerPersistentId_ = session->GetPersistentId();
    }
    return info;
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseAboveTarget(int32_t subWindowId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        WLOGFE("RaiseAboveTarget permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto subSession = std::find_if(subSession_.begin(), subSession_.end(), [subWindowId](sptr<SceneSession> session) {
        bool res = (session != nullptr && session->GetWindowId() == subWindowId) ? true : false;
        return res;
    });
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (subSession != subSession_.end() && callingPid != (*subSession)->GetCallingPid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "permission denied! id: %{public}d", subWindowId);
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    return PostSyncTask([weakThis = wptr(this), subWindowId, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->onRaiseAboveTarget_) {
            session->onRaiseAboveTarget_(subWindowId);
        }
        return WSError::WS_OK;
    }, __func__);
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseMainWindowAboveTarget(int32_t targetId)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[SceneSession] source id: %{public}u, target id: %{public}u ",
        GetWindowId(), targetId);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "server permission denied, require system application");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto targetSession = GetSceneSessionById(targetId);
    if (targetSession == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "target session is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (GetCallingPid() != targetSession->GetCallingPid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "calling pid inconsistecy");
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    if ((GetSessionProperty()->GetDisplayId() != targetSession->GetSessionProperty()->GetDisplayId()) &&
        !(PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(GetSessionProperty()->GetDisplayId()) &&
          PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(targetSession->
                                                                       GetSessionProperty()->GetDisplayId()))) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "source window and target window are not on the same screen");
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    if (GetZOrder() > SCREEN_LOCK_Z_ORDER || targetSession->GetZOrder() > SCREEN_LOCK_Z_ORDER) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "window above screenlock is not supported");
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    return PostSyncTask([weakThis = wptr(this), targetId, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->onRaiseMainWindowAboveTarget_) {
            TLOGNI(WmsLogTag::WMS_HIERARCHY, "id: %{public}d, raise main window above target: %{public}d",
                session->GetPersistentId(), targetId);
            session->onRaiseMainWindowAboveTarget_(targetId);
        }
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSession::BindDialogSessionTarget(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "dialog session is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (onBindDialogTarget_) {
        TLOGI(WmsLogTag::WMS_DIALOG, "id: %{public}d", sceneSession->GetPersistentId());
        onBindDialogTarget_(sceneSession);
    }
    return WSError::WS_OK;
}

WSError SceneSession::SetSystemBarProperty(WindowType type, SystemBarProperty systemBarProperty)
{
    TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u type %{public}u "
        "%{public}u %{public}x %{public}x %{public}u settingFlag %{public}u",
        GetPersistentId(), static_cast<uint32_t>(type),
        systemBarProperty.enable_, systemBarProperty.backgroundColor_, systemBarProperty.contentColor_,
        systemBarProperty.enableAnimation_, systemBarProperty.settingFlag_);
    auto property = GetSessionProperty();
    property->SetSystemBarProperty(type, systemBarProperty);
    if (type == WindowType::WINDOW_TYPE_STATUS_BAR && systemBarProperty.enable_) {
        SetIsDisplayStatusBarTemporarily(false);
    }
    if (onSystemBarPropertyChange_) {
        onSystemBarPropertyChange_(property->GetSystemBarProperty());
        if (specificCallback_ != nullptr && specificCallback_->onNotifyWindowSystemBarPropertyChangeFunc_ != nullptr) {
            specificCallback_->onNotifyWindowSystemBarPropertyChangeFunc_(type, systemBarProperty);
        }
    }
    return WSError::WS_OK;
}

void SceneSession::SetIsStatusBarVisible(bool isVisible)
{
    PostTask([weakThis = wptr(this), isVisible, where = __func__] {
        sptr<SceneSession> sceneSession = weakThis.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return;
        }
        sceneSession->SetIsStatusBarVisibleInner(isVisible);
    }, __func__);
}

WSError SceneSession::SetIsStatusBarVisibleInner(bool isVisible)
{
    bool isNeedNotify = IsStatusBarVisible() != isVisible;
    UpdateStatusBarVisible(isVisible);
    if (!isNeedNotify) {
        return WSError::WS_OK;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}d, %{public}s] visible %{public}u need notify",
        GetPersistentId(), GetWindowName().c_str(), isVisible);
    return HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_SYSTEM);
}

WSError SceneSession::HandleLayoutAvoidAreaUpdate(AvoidAreaType avoidAreaType)
{
    if (isLastFrameLayoutFinishedFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "isLastFrameLayoutFinishedFunc is null, win %{public}d", GetPersistentId());
        return WSError::WS_ERROR_NULLPTR;
    }
    bool isLayoutFinished = false;
    WSError ret = isLastFrameLayoutFinishedFunc_(isLayoutFinished);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "isLastFrameLayoutFinishedFunc failed, ret %{public}d", ret);
        return ret;
    }
    if (!isLayoutFinished) {
        MarkAvoidAreaAsDirty();
        return WSError::WS_OK;
    }
    if (avoidAreaType != AvoidAreaType::TYPE_END) {
        auto area = GetAvoidAreaByType(avoidAreaType);
        // code below aims to check if ai bar avoid area reaches window rect's bottom
        // it should not be removed until unexpected window rect update issues were solved
        if (avoidAreaType == AvoidAreaType::TYPE_NAVIGATION_INDICATOR && isAINavigationBarAvoidAreaValid_ &&
            !isAINavigationBarAvoidAreaValid_(GetSessionProperty()->GetDisplayId(), area, GetSessionRect().height_)) {
            TLOGE(WmsLogTag::WMS_IMMS, "ai bar avoid area dose not reach the bottom of the rect");
            return WSError::WS_OK;
        }
        UpdateAvoidArea(new AvoidArea(area), avoidAreaType);
        return WSError::WS_OK;
    } else {
        // avoidAreaType equal to TYPE_END means traversing and updating all avoid area types
        using T = std::underlying_type_t<AvoidAreaType>;
        for (T avoidType = static_cast<T>(AvoidAreaType::TYPE_START);
            avoidType < static_cast<T>(AvoidAreaType::TYPE_END); avoidType++) {
            auto type = static_cast<AvoidAreaType>(avoidType);
            auto area = GetAvoidAreaByType(type);
            // code below aims to check if ai bar avoid area reaches window rect's bottom
            // it should not be removed until unexpected window rect update issues were solved
            if (type == AvoidAreaType::TYPE_NAVIGATION_INDICATOR && isAINavigationBarAvoidAreaValid_ &&
                !isAINavigationBarAvoidAreaValid_(GetSessionProperty()->GetDisplayId(),
                    area, GetSessionRect().height_)) {
                TLOGE(WmsLogTag::WMS_IMMS, "ai bar avoid area dose not reach the bottom "
                    "of the rect while traversing all avoid area type");
                continue;
            }
            UpdateAvoidArea(new AvoidArea(area), type);
        }
    }
    return WSError::WS_OK;
}

void SceneSession::NotifyPropertyWhenConnect()
{
    TLOGI(WmsLogTag::DEFAULT, "In");
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "id: %{public}d property is nullptr", persistentId_);
        return;
    }
    NotifySessionFocusableChange(property->GetFocusable());
    NotifySessionTouchableChange(property->GetTouchable());
    OnShowWhenLocked(GetShowWhenLockedFlagValue());
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseAppMainWindowToTop()
{
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->IsFocusedOnShow()) {
            FocusChangeReason reason = FocusChangeReason::MOVE_UP;
            session->NotifyRequestFocusStatusNotifyManager(true, true, reason);
            session->NotifyClick(true, false);
        } else {
            session->SetFocusedOnShow(true);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::OnNeedAvoid(bool status)
{
    PostTask([weakThis = wptr(this), status, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s win %{public}d status %{public}d",
            where, session->GetPersistentId(), static_cast<int32_t>(status));
        if (session->onNeedAvoid_) {
            session->onNeedAvoid_(status);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::OnShowWhenLocked(bool showWhenLocked)
{
    WLOGFD("SceneSession ShowWhenLocked status:%{public}d", static_cast<int32_t>(showWhenLocked));
    if (onShowWhenLockedFunc_) {
        onShowWhenLockedFunc_(showWhenLocked);
    }
    return WSError::WS_OK;
}

bool SceneSession::IsShowWhenLocked() const
{
    return (GetSessionProperty()->GetWindowFlags() &
        static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) || IsTemporarilyShowWhenLocked();
}

bool SceneSession::GetShowWhenLockedFlagValue() const
{
    return GetSessionProperty()->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
}

void SceneSession::PrintAvoidAreaInfo(DisplayId displayId,
    AvoidAreaType type, const WSRect& rect, const WSRect& avoidRect) const
{
    std::tuple<DisplayId, WSRect, WSRect> inputParamters(displayId, rect, avoidRect);
    auto iter = lastAvoidAreaInputParamtersMap_.find(type);
    if (iter != lastAvoidAreaInputParamtersMap_.end() && iter->second == inputParamters) {
        return;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d dispaly %{public}" PRIu64 ""
        "type %{public}d rect %{public}s bar %{public}s",
        GetPersistentId(), displayId, type, rect.ToString().c_str(), avoidRect.ToString().c_str());
}

void SceneSession::CalculateAvoidAreaByType(AvoidAreaType type,
    const WSRect& winRect, const WSRect& avoidRect, AvoidArea& avoidArea)
{
    auto displayId = GetSessionProperty()->GetDisplayId();
    PrintAvoidAreaInfo(displayId, type, winRect, avoidRect);
    CalculateAvoidAreaRect(winRect, avoidRect, avoidArea);
    lastAvoidAreaInputParamtersMap_[type] = std::make_tuple(displayId, winRect, avoidRect);
}

void SceneSession::CalculateAvoidAreaRect(const WSRect& rect, const WSRect& avoidRect, AvoidArea& avoidArea) const
{
    if (SessionHelper::IsEmptyRect(rect) || SessionHelper::IsEmptyRect(avoidRect)) {
        return;
    }
    Rect avoidAreaRect = SessionHelper::TransferToRect(
        SessionHelper::GetOverlap(rect, avoidRect, rect.posX_, rect.posY_));
    if (WindowHelper::IsEmptyRect(avoidAreaRect)) {
        return;
    }

    uint32_t avoidAreaCenterX = static_cast<uint32_t>(avoidAreaRect.posX_) + (avoidAreaRect.width_ >> 1);
    uint32_t avoidAreaCenterY = static_cast<uint32_t>(avoidAreaRect.posY_) + (avoidAreaRect.height_ >> 1);
    float res1 = float(avoidAreaCenterY) - float(rect.height_) / float(rect.width_) *
        float(avoidAreaCenterX);
    float res2 = float(avoidAreaCenterY) + float(rect.height_) / float(rect.width_) *
        float(avoidAreaCenterX) - float(rect.height_);
    CalculateAvoidAreaByScale(avoidAreaRect);
    if (res1 < 0) {
        if (res2 < 0) {
            avoidArea.topRect_ = avoidAreaRect;
        } else {
            avoidArea.rightRect_ = avoidAreaRect;
        }
    } else {
        if (res2 < 0) {
            avoidArea.leftRect_ = avoidAreaRect;
        } else {
            avoidArea.bottomRect_ = avoidAreaRect;
        }
    }
}

void SceneSession::GetSystemAvoidArea(WSRect& rect, AvoidArea& avoidArea, bool ignoreVisibility)
{
    auto sessionProperty = GetSessionProperty();
    bool isNeedAvoid = sessionProperty->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    if (isNeedAvoid && WindowHelper::IsAppWindow(GetWindowType())) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d type %{public}u flag %{public}d",
            GetPersistentId(), static_cast<uint32_t>(GetWindowType()), sessionProperty->GetWindowFlags());
        return;
    }
    if (sessionProperty->IsAdaptToImmersive()) {
        HookAvoidAreaInCompatibleMode(rect, AvoidAreaType::TYPE_SYSTEM, avoidArea);
        return;
    }
    WindowMode windowMode = Session::GetWindowMode();
    bool isWindowFloatingOrSplit = windowMode == WindowMode::WINDOW_MODE_FLOATING ||
                                   windowMode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
                                   windowMode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY;
    WindowType windowType = Session::GetWindowType();
    bool isAvailableSystemWindow = WindowHelper::IsSystemWindow(windowType) &&
        (GetSessionProperty()->GetAvoidAreaOption() & static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW));
    bool isAvailableAppSubWindow = WindowHelper::IsSubWindow(windowType) &&
        (GetSessionProperty()->GetAvoidAreaOption() & static_cast<uint32_t>(AvoidAreaOption::ENABLE_APP_SUB_WINDOW));
    bool isAvailableWindowType = WindowHelper::IsMainWindow(windowType) || isAvailableSystemWindow ||
                                 isAvailableAppSubWindow;
    bool isAvailableDevice = (systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow()) &&
                              !IsFreeMultiWindowMode();
    DisplayId displayId = sessionProperty->GetDisplayId();
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    bool isAvailableScreen = !screenSession || (screenSession->GetName() != "HiCar");
    if (isWindowFloatingOrSplit && isAvailableWindowType && isAvailableDevice && isAvailableScreen) {
        float vpr = 3.5f; // 3.5f: default pixel ratio
        auto display = DisplayManager::GetInstance().GetDefaultDisplay();
        if (display == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "display is null");
            return;
        }
        vpr = display->GetVirtualPixelRatio();
        bool isFloat = windowMode == WindowMode::WINDOW_MODE_FLOATING && !GetIsMidScene();
        int32_t maxFloatTitleBarHeight = static_cast<int32_t>(vpr * MAX_FLOAT_TITLE_BAR_HEIGHT_VP);
        int32_t floatTitleBarHeight = std::min(maxFloatTitleBarHeight, GetStatusBarHeight());
        int32_t height =
            isFloat ? floatTitleBarHeight : static_cast<int32_t>(vpr * MULTI_WINDOW_TITLE_BAR_DEFAULT_HEIGHT_VP);
        avoidArea.topRect_.height_ = static_cast<uint32_t>(height);
        avoidArea.topRect_.width_ = static_cast<uint32_t>(display->GetWidth());
        return;
    }
    std::vector<sptr<SceneSession>> statusBarVector;
    if (specificCallback_ != nullptr && specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_) {
        statusBarVector = specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_(
            WindowType::WINDOW_TYPE_STATUS_BAR, sessionProperty->GetDisplayId());
    }
    for (auto& statusBar : statusBarVector) {
        if (statusBar == nullptr) {
            continue;
        }
        bool isVisible = statusBar->isVisible_;
        if (onGetStatusBarConstantlyShowFunc_) {
            onGetStatusBarConstantlyShowFunc_(displayId, isVisible);
            TLOGD(WmsLogTag::WMS_IMMS, "win %{public}d displayId %{public}" PRIu64 " constantly isVisible %{public}d",
                GetPersistentId(), displayId, isVisible);
        }
        bool isStatusBarVisible =
            WindowHelper::IsAppWindow(Session::GetWindowType()) ? IsStatusBarVisible() : isVisible;
        if (!isStatusBarVisible && !ignoreVisibility) {
            TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d status bar not visible", GetPersistentId());
            continue;
        }
        WSRect statusBarRect = statusBar->GetSessionRect();
        if (onGetStatusBarAvoidHeightFunc_) {
            onGetStatusBarAvoidHeightFunc_(displayId, statusBarRect);
        }
        CalculateAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM, rect, statusBarRect, avoidArea);
    }
}

void SceneSession::GetKeyboardAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    if (!keyboardAvoidAreaActive_) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "id: %{public}d, isSystemKeyboard: %{public}d, state: %{public}d, "
            "gravity: %{public}d", GetPersistentId(), IsSystemKeyboard(), GetSessionState(), GetKeyboardGravity());
        return;
    }
    if (Session::CheckEmptyKeyboardAvoidAreaIfNeeded()) {
        TLOGD(WmsLogTag::WMS_IMMS, "Keyboard avoid area needs to be empty in floating mode");
        return;
    }
    std::vector<sptr<SceneSession>> inputMethodVector;
    if (specificCallback_ != nullptr && specificCallback_->onGetSceneSessionVectorByType_) {
        inputMethodVector = specificCallback_->onGetSceneSessionVectorByType_(
            WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    }
    for (auto& inputMethod : inputMethodVector) {
        if (inputMethod->GetSessionState() != SessionState::STATE_FOREGROUND &&
            inputMethod->GetSessionState() != SessionState::STATE_ACTIVE) {
            continue;
        }
        SessionGravity gravity = inputMethod->GetKeyboardGravity();
        if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT || !inputMethod->IsKeyboardAvoidAreaActive()) {
            continue;
        }
        if (isKeyboardPanelEnabled_) {
            WSRect keyboardRect = {0, 0, 0, 0};
            if (inputMethod && inputMethod->GetKeyboardPanelSession()) {
                keyboardRect = inputMethod->GetKeyboardPanelSession()->GetSessionRect();
                inputMethod->RecalculatePanelRectForAvoidArea(keyboardRect);
            }
            CalculateAvoidAreaByType(AvoidAreaType::TYPE_KEYBOARD, rect, keyboardRect, avoidArea);
        } else {
            WSRect inputMethodRect = inputMethod->GetSessionRect();
            CalculateAvoidAreaByType(AvoidAreaType::TYPE_KEYBOARD, rect, inputMethodRect, avoidArea);
        }
    }
    return;
}

void SceneSession::GetKeyboardOccupiedAreaWithRotation(
    int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "In");
    if (!specificCallback_ || !specificCallback_->onKeyboardRotationChange_) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "specificCallback_ or onKeyboardRotationChange_ is nullptr");
        return;
    }
    specificCallback_->onKeyboardRotationChange_(persistentId, rotation, avoidAreas);
}

void SceneSession::RegisterNotifyOccupiedAreaChangeCallback(ForceNotifyOccupiedAreaChangeCallback&& callback){
    forceNotifyOccupiedAreaChangeFunc_ = std::move(callback);
}

void SceneSession::ForceNotifyKeyboardOccupiedArea(){
    if(forceNotifyOccupiedAreaChangeFunc_){
        auto displayId = GetSessionProperty()->GetDisplayId();
        forceNotifyOccupiedAreaChangeFunc_(displayId);
    } 
}

void SceneSession::GetCutoutAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    auto display = DisplayManager::GetInstance().GetDisplayById(GetSessionProperty()->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to get display");
        return;
    }
    sptr<CutoutInfo> cutoutInfo = display->GetCutoutInfo();
    if (cutoutInfo == nullptr) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutout info");
        return;
    }
    std::vector<DMRect> cutoutAreas = cutoutInfo->GetBoundingRects();
    if (cutoutAreas.empty()) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutout area");
        return;
    }
    for (auto& cutoutArea : cutoutAreas) {
        WSRect cutoutAreaRect = {
            cutoutArea.posX_,
            cutoutArea.posY_,
            cutoutArea.width_,
            cutoutArea.height_
        };
        CalculateAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, rect, cutoutAreaRect, avoidArea);
    }

    return;
}

void SceneSession::PatchAINavigationBarArea(AvoidArea& avoidArea)
{
    Rect areaEmpty = { 0, 0, 0, 0 };
    if (!avoidArea.leftRect_.IsUninitializedRect()) {
        avoidArea.bottomRect_ = avoidArea.leftRect_;
        avoidArea.leftRect_ = areaEmpty;
    } else if (!avoidArea.topRect_.IsUninitializedRect()) {
        avoidArea.bottomRect_ = avoidArea.topRect_;
        avoidArea.topRect_ = areaEmpty;
    } else if (!avoidArea.topRect_.IsUninitializedRect()) {
        avoidArea.bottomRect_ = avoidArea.rightRect_;
        avoidArea.rightRect_ = areaEmpty;
    }
}

void SceneSession::GetAINavigationBarArea(WSRect& rect, AvoidArea& avoidArea, bool ignoreVisibility)
{
    if (Session::GetWindowMode() == WindowMode::WINDOW_MODE_PIP) {
        TLOGD(WmsLogTag::WMS_IMMS, "window mode pip return");
        return;
    }
    // compatibleMode app need to hook avoidArea in pc
    if (GetSessionProperty()->IsAdaptToImmersive()) {
        HookAvoidAreaInCompatibleMode(rect, AvoidAreaType::TYPE_NAVIGATION_INDICATOR, avoidArea);
        return;
    }
    WSRect barArea;
    if (specificCallback_ != nullptr && specificCallback_->onGetAINavigationBarArea_) {
        barArea = specificCallback_->onGetAINavigationBarArea_(GetSessionProperty()->GetDisplayId(), ignoreVisibility);
    }
    CalculateAvoidAreaByType(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, rect, barArea, avoidArea);
    PatchAINavigationBarArea(avoidArea);
}

void SceneSession::HookAvoidAreaInCompatibleMode(const WSRect& rect, AvoidAreaType avoidAreaType,
    AvoidArea& avoidArea) const
{
    WindowMode mode = GetWindowMode();
    if (!GetSessionProperty()->IsAdaptToImmersive() || mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
        return;
    }
    float vpr = 1.9f; // 3.5f: default pixel ratio
    DMHookInfo hookInfo;
    ScreenSessionManagerClient::GetInstance().GetDisplayHookInfo(GetCallingUid(), hookInfo);
    if (hookInfo.density_) {
        vpr = hookInfo.density_;
    }
    switch (avoidAreaType) {
        case AvoidAreaType::TYPE_SYSTEM: {
            avoidArea.topRect_.posX_ = 0;
            avoidArea.topRect_.posY_ = 0;
            avoidArea.topRect_.height_ = HOOK_SYSTEM_BAR_HEIGHT * vpr;
            avoidArea.topRect_.width_ = rect.width_;
            return;
        }
        case AvoidAreaType::TYPE_NAVIGATION_INDICATOR: {
            avoidArea.bottomRect_.posX_ = 0;
            avoidArea.bottomRect_.posY_ = rect.height_ - HOOK_AI_BAR_HEIGHT * vpr;
            avoidArea.bottomRect_.width_ = rect.width_;
            avoidArea.bottomRect_.height_ = HOOK_AI_BAR_HEIGHT * vpr;
            return;
        }
        default: {
            TLOGE(WmsLogTag::WMS_IMMS, "cannot find win %{public}d type %{public}u", GetPersistentId(), avoidAreaType);
            return;
        }
    }
}

bool SceneSession::CheckGetSubWindowAvoidAreaAvailable(WindowMode winMode, AvoidAreaType type)
{
    if (GetSessionProperty()->GetAvoidAreaOption() & static_cast<uint32_t>(AvoidAreaOption::ENABLE_APP_SUB_WINDOW)) {
        return true;
    }
    if ((winMode == WindowMode::WINDOW_MODE_FLOATING && IsFreeMultiWindowMode()) || systemConfig_.IsPcWindow()) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}d not support", GetPersistentId());
        return false;
    }
    auto parentSession = GetParentSession();
    if (!parentSession) {
        TLOGE(WmsLogTag::WMS_IMMS, "win %{public}d parent session is nullptr", GetPersistentId());
        return false;
    }
    if (parentSession->GetSessionRect() != GetSessionRect()) {
        TLOGE(WmsLogTag::WMS_IMMS, "rect mismatch: win %{public}d parent %{public}d",
            GetPersistentId(), parentSession->GetPersistentId());
        return false;
    }
    return parentSession->CheckGetAvoidAreaAvailable(type);
}

bool SceneSession::CheckGetMainWindowAvoidAreaAvailable(WindowMode winMode, AvoidAreaType type)
{
    // compatibleMode app in pc,need use avoid Area
    if (GetSessionProperty()->IsAdaptToImmersive()) {
        return true;
    }
    if ((winMode == WindowMode::WINDOW_MODE_FLOATING && IsFreeMultiWindowMode()) || systemConfig_.IsPcWindow()) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}d not support", GetPersistentId());
        return false;
    }
    if (winMode == WindowMode::WINDOW_MODE_FLOATING && type != AvoidAreaType::TYPE_SYSTEM) {
        return false;
    }
    if (winMode != WindowMode::WINDOW_MODE_FLOATING || systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow()) {
        return true;
    }
    return false;
}

bool SceneSession::CheckGetSystemWindowAvoidAreaAvailable()
{
    if (GetSessionProperty()->GetAvoidAreaOption() &
        static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW)) {
        return systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow();
    }
    return false;
}

bool SceneSession::CheckGetAvoidAreaAvailable(AvoidAreaType type)
{
    if (type == AvoidAreaType::TYPE_KEYBOARD) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}d type 3, return 1", GetPersistentId());
        return true;
    }
    WindowMode winMode = GetWindowMode();
    WindowType winType = GetWindowType();
    bool isAvailable = false;
    if (WindowHelper::IsSubWindow(winType)) {
        isAvailable = CheckGetSubWindowAvoidAreaAvailable(winMode, type);
    } else if (WindowHelper::IsMainWindow(winType)) {
        isAvailable = CheckGetMainWindowAvoidAreaAvailable(winMode, type);
    } else if (WindowHelper::IsSystemWindow(winType)) {
        isAvailable = CheckGetSystemWindowAvoidAreaAvailable();
    }
    TLOGD(WmsLogTag::WMS_IMMS, "win %{public}d type %{public}u avoidAreaType %{public}u "
        "windowMode %{public}u avoidAreaOption %{public}u, return %{public}d",
        GetPersistentId(), static_cast<uint32_t>(winType), static_cast<uint32_t>(type),
        static_cast<uint32_t>(winMode), GetSessionProperty()->GetAvoidAreaOption(), isAvailable);
    return isAvailable;
}

void SceneSession::AddNormalModalUIExtension(const ExtensionWindowEventInfo& extensionInfo)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "parentId=%{public}d, persistentId=%{public}d, pid=%{public}d", GetPersistentId(),
        extensionInfo.persistentId, extensionInfo.pid);
    {
        std::unique_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
        modalUIExtensionInfoList_.push_back(extensionInfo);
        std::sort(modalUIExtensionInfoList_.begin(), modalUIExtensionInfoList_.end(),
            CmpExtensionWidowInfoByTimeStamp);
    }
    NotifySessionInfoChange();
}

void SceneSession::UpdateNormalModalUIExtension(const ExtensionWindowEventInfo& extensionInfo)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "persistentId=%{public}d,pid=%{public}d,"
        "Rect:[%{public}d %{public}d %{public}d %{public}d]",
        extensionInfo.persistentId, extensionInfo.pid, extensionInfo.windowRect.posX_,
        extensionInfo.windowRect.posY_, extensionInfo.windowRect.width_, extensionInfo.windowRect.height_);
    {
        std::unique_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
        auto iter = std::find_if(modalUIExtensionInfoList_.begin(), modalUIExtensionInfoList_.end(),
            [extensionInfo](const ExtensionWindowEventInfo& eventInfo) {
            return extensionInfo.persistentId == eventInfo.persistentId && extensionInfo.pid == eventInfo.pid;
        });
        if (iter == modalUIExtensionInfoList_.end()) {
            return;
        }
        iter->windowRect = extensionInfo.windowRect;
        iter->uiExtRect = extensionInfo.uiExtRect;
        iter->hasUpdatedRect = extensionInfo.hasUpdatedRect;
    }
    NotifySessionInfoChange();
}

void SceneSession::RemoveNormalModalUIExtension(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "parentId=%{public}d, persistentId=%{public}d", GetPersistentId(), persistentId);
    {
        std::unique_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
        auto iter = std::find_if(modalUIExtensionInfoList_.begin(), modalUIExtensionInfoList_.end(),
            [persistentId](const ExtensionWindowEventInfo& extensionInfo) {
            return extensionInfo.persistentId == persistentId;
        });
        if (iter == modalUIExtensionInfoList_.end()) {
            return;
        }
        modalUIExtensionInfoList_.erase(iter);
    }
    NotifySessionInfoChange();
}

void SceneSession::RegisterGetConstrainedModalExtWindowInfo(GetConstrainedModalExtWindowInfoFunc&& callback)
{
    onGetConstrainedModalExtWindowInfoFunc_ = std::move(callback);
}

std::optional<ExtensionWindowEventInfo> SceneSession::GetLastModalUIExtensionEventInfo()
{
    // Priority query constrained modal UIExt, if unavailable, then query normal modal UIExt
    if (onGetConstrainedModalExtWindowInfoFunc_) {
        if (auto constrainedExtEventInfo = onGetConstrainedModalExtWindowInfoFunc_(this)) {
            TLOGD(WmsLogTag::WMS_UIEXT, "get constrained UIExt eventInfo, id: %{public}d",
                constrainedExtEventInfo->persistentId);
            return constrainedExtEventInfo;
        }
    }
    std::shared_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
    return modalUIExtensionInfoList_.empty() ? std::nullopt :
        std::make_optional<ExtensionWindowEventInfo>(modalUIExtensionInfoList_.back());
}

Vector2f SceneSession::GetSessionGlobalPosition(bool useUIExtension)
{
    WSRect windowRect = GetSessionGlobalRectInMultiScreen();
    if (useUIExtension) {
        if (auto modalUIExtensionEventInfo = GetLastModalUIExtensionEventInfo()) {
            const auto& rect = modalUIExtensionEventInfo.value().windowRect;
            windowRect.posX_ = rect.posX_;
            windowRect.posY_ = rect.posY_;
        }
    }
    Vector2f position(windowRect.posX_, windowRect.posY_);
    return position;
}

WSRect SceneSession::GetSessionGlobalRectWithSingleHandScale()
{
    WSRect rectWithTransform = GetSessionGlobalRect();
    const SingleHandTransform& transform = GetSingleHandTransform();
    if (transform.posX == 0 && transform.posY == 0) {
        return rectWithTransform;
    }
    rectWithTransform.posX_ =
        static_cast<int32_t>(static_cast<float>(rectWithTransform.posX_) * transform.scaleX) + transform.posX;
    rectWithTransform.posY_ =
        static_cast<int32_t>(static_cast<float>(rectWithTransform.posY_) * transform.scaleY) + transform.posY;
    rectWithTransform.width_ =
        static_cast<int32_t>(static_cast<float>(rectWithTransform.width_) * transform.scaleX);
    rectWithTransform.height_ =
        static_cast<int32_t>(static_cast<float>(rectWithTransform.height_) * transform.scaleY);
    return rectWithTransform;
}

WSRect SceneSession::GetMoveRectForWindowDrag()
{
    auto property = GetSessionProperty();
    if (!property) {
        return GetGlobalOrWinRect();
    }
    if (property->GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return GetGlobalOrWinRect();
    }
    if (keyboardPanelSession_ != nullptr) {
        return keyboardPanelSession_->GetSessionRect();
    }
    return GetGlobalOrWinRect();
}

void SceneSession::AddUIExtSurfaceNodeId(uint64_t surfaceNodeId, int32_t persistentId)
{
    std::unique_lock<std::shared_mutex> lock(uiExtNodeIdToPersistentIdMapMutex_);
    TLOGI(WmsLogTag::WMS_UIEXT, "Add uiExtension pair surfaceNodeId=%{public}" PRIu64 ", persistentId=%{public}d",
        surfaceNodeId, persistentId);
    uiExtNodeIdToPersistentIdMap_.insert(std::make_pair(surfaceNodeId, persistentId));
}

void SceneSession::RemoveUIExtSurfaceNodeId(int32_t persistentId)
{
    std::unique_lock<std::shared_mutex> lock(uiExtNodeIdToPersistentIdMapMutex_);
    TLOGI(WmsLogTag::WMS_UIEXT, "Remove uiExtension by persistentId=%{public}d", persistentId);
    auto pairIter = std::find_if(uiExtNodeIdToPersistentIdMap_.begin(), uiExtNodeIdToPersistentIdMap_.end(),
        [persistentId](const auto& entry) { return entry.second == persistentId; });
    if (pairIter != uiExtNodeIdToPersistentIdMap_.end()) {
        TLOGI(WmsLogTag::WMS_UIEXT,
            "Successfully removed uiExtension pair surfaceNodeId=%{public}" PRIu64 ", persistentId=%{public}d",
            pairIter->first, persistentId);
        uiExtNodeIdToPersistentIdMap_.erase(pairIter);
        return;
    }
    TLOGE(WmsLogTag::WMS_UIEXT, "Failed to remove uiExtension by persistentId=%{public}d", persistentId);
}

int32_t SceneSession::GetUIExtPersistentIdBySurfaceNodeId(uint64_t surfaceNodeId) const
{
    std::shared_lock<std::shared_mutex> lock(uiExtNodeIdToPersistentIdMapMutex_);
    auto ret = uiExtNodeIdToPersistentIdMap_.find(surfaceNodeId);
    if (ret == uiExtNodeIdToPersistentIdMap_.end()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to find uiExtension by surfaceNodeId=%{public}" PRIu64 "", surfaceNodeId);
        return 0;
    }
    return ret->second;
}

WSError SceneSession::GetScaleInLSState(float& scaleX, float& scaleY) const
{
    if (GetWindowMode() != WindowMode::WINDOW_MODE_FULLSCREEN || GetIsMidScene() ||
        !specificCallback_ || !specificCallback_->onGetLSState_ || !specificCallback_->onGetLSState_()) {
        TLOGD(WmsLogTag::WMS_IMMS, "win: %{public}d, not in LS state", GetPersistentId());
        return WSError::WS_DO_NOTHING;
    }
    constexpr float invalidScale = 0;
    if (GetScaleX() <= invalidScale || GetScaleY() <= invalidScale) {
        TLOGE(WmsLogTag::WMS_IMMS, "win: %{public}d, invalid scale", GetPersistentId());
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    scaleX = GetScaleX();
    scaleY = GetScaleY();
    return WSError::WS_OK;
}

void SceneSession::CalculateWindowRectByScale(WSRect& winRect)
{
    float scaleX = 1;
    float scaleY = 1;
    if (GetScaleInLSState(scaleX, scaleY) != WSError::WS_OK) {
        return;
    }
    winRect.width_ *= scaleX;
    winRect.height_ *= scaleY;
}

void SceneSession::CalculateAvoidAreaByScale(Rect& avoidAreaRect) const
{
    float scaleX = 1;
    float scaleY = 1;
    if (GetScaleInLSState(scaleX, scaleY) != WSError::WS_OK) {
        return;
    }
    avoidAreaRect.posX_ = std::ceil(avoidAreaRect.posX_ / scaleX);
    avoidAreaRect.posY_ = std::ceil(avoidAreaRect.posY_ / scaleY);
    avoidAreaRect.width_ = std::ceil(avoidAreaRect.width_ / scaleX);
    avoidAreaRect.height_ = std::ceil(avoidAreaRect.height_ / scaleY);
}

AvoidArea SceneSession::GetAvoidAreaByTypeInner(AvoidAreaType type, const WSRect& rect, bool ignoreVisibility)
{
    if (!CheckGetAvoidAreaAvailable(type)) {
        return {};
    }

    AvoidArea avoidArea;
    WSRect sessionRect = rect.IsEmpty() ? GetSessionRect() : rect;
    CalculateWindowRectByScale(sessionRect);
    switch (type) {
        case AvoidAreaType::TYPE_SYSTEM: {
            GetSystemAvoidArea(sessionRect, avoidArea, ignoreVisibility);
            return avoidArea;
        }
        case AvoidAreaType::TYPE_CUTOUT: {
            GetCutoutAvoidArea(sessionRect, avoidArea);
            return avoidArea;
        }
        case AvoidAreaType::TYPE_SYSTEM_GESTURE: {
            return avoidArea;
        }
        case AvoidAreaType::TYPE_KEYBOARD: {
            GetKeyboardAvoidArea(sessionRect, avoidArea);
            return avoidArea;
        }
        case AvoidAreaType::TYPE_NAVIGATION_INDICATOR: {
            GetAINavigationBarArea(sessionRect, avoidArea, ignoreVisibility);
            return avoidArea;
        }
        default: {
            TLOGE(WmsLogTag::WMS_IMMS, "cannot find win %{public}d type %{public}u",
                GetPersistentId(), type);
            return avoidArea;
        }
    }
}

AvoidArea SceneSession::GetAvoidAreaByType(AvoidAreaType type, const WSRect& rect, int32_t apiVersion)
{
    return PostSyncTask([weakThis = wptr(this), type, rect, where = __func__]() -> AvoidArea {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return {};
        }
        return session->GetAvoidAreaByTypeInner(type, rect);
    }, __func__);
}

AvoidArea SceneSession::GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type, const WSRect& rect)
{
    return PostSyncTask([weakThis = wptr(this), type, rect, where = __func__]() -> AvoidArea {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return {};
        }
        return session->GetAvoidAreaByTypeInner(type, rect, true);
    }, __func__);
}

WSError SceneSession::GetAllAvoidAreas(std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    return PostSyncTask([weakThis = wptr(this), &avoidAreas, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }

        using T = std::underlying_type_t<AvoidAreaType>;
        for (T avoidType = static_cast<T>(AvoidAreaType::TYPE_START);
            avoidType < static_cast<T>(AvoidAreaType::TYPE_END); avoidType++) {
            auto type = static_cast<AvoidAreaType>(avoidType);
            auto area = session->GetAvoidAreaByTypeInner(type);
            // code below aims to check if ai bar avoid area reaches window rect's bottom
            // it should not be removed until unexpected window rect update issues were solved
            if (type == AvoidAreaType::TYPE_NAVIGATION_INDICATOR) {
                if (session->isAINavigationBarAvoidAreaValid_ &&
                    !session->isAINavigationBarAvoidAreaValid_(session->GetSessionProperty()->GetDisplayId(),
                        area, session->GetSessionRect().height_)) {
                    continue;
                }
            }
            avoidAreas[type] = area;
        }
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSession::GetAvoidAreasByRotation(Rotation rotation, const WSRect& rect,
    const std::map<WindowType, SystemBarProperty>& properties, std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if (!WindowHelper::IsMainFullScreenWindow(GetWindowType(), GetWindowMode())) {
        TLOGI(WmsLogTag::WMS_IMMS, "window is no support, type %{public}d, mode %{public}d",
            GetWindowType(), GetWindowMode());
        return WSError::WS_DO_NOTHING;
    }
    return PostSyncTask([weakThis = wptr(this), rotation, &rect, &properties, &avoidAreas, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        using T = std::underlying_type_t<AvoidAreaType>;
        for (T avoidType = static_cast<T>(AvoidAreaType::TYPE_START);
            avoidType < static_cast<T>(AvoidAreaType::TYPE_END); avoidType++) {
            auto type = static_cast<AvoidAreaType>(avoidType);
            avoidAreas[type] = session->GetAvoidAreaByRotation(rotation, rect, properties, type);
        }
        return WSError::WS_OK;
    }, __func__);
}

AvoidArea SceneSession::GetAvoidAreaByRotation(Rotation rotation, const WSRect& rect,
    const std::map<WindowType, SystemBarProperty>& properties, AvoidAreaType type)
{
    AvoidArea avoidArea;
    switch (type) {
        case AvoidAreaType::TYPE_SYSTEM:
        case AvoidAreaType::TYPE_NAVIGATION_INDICATOR: {
            GetSystemBarAvoidAreaByRotation(rotation, type, rect, properties, avoidArea);
            break;
        }
        case AvoidAreaType::TYPE_CUTOUT: {
            GetCutoutAvoidAreaByRotation(rotation, rect, avoidArea);
            break;
        }
        case AvoidAreaType::TYPE_KEYBOARD: {
            GetKeyboardAvoidAreaByRotation(rotation, rect, avoidArea);
            break;
        }
        default: {
            TLOGE(WmsLogTag::WMS_IMMS, "cannot find win %{public}d type %{public}u", GetPersistentId(), type);
            break;
        }
    }
    return avoidArea;
}

void SceneSession::GetSystemBarAvoidAreaByRotation(Rotation rotation, AvoidAreaType type, const WSRect& rect,
    const std::map<WindowType, SystemBarProperty>& properties, AvoidArea& avoidArea)
{
    DisplayId displayId = GetSessionProperty()->GetDisplayId();
    std::pair<WSRect, WSRect> nextSystemBarAvoidAreaRectInfo;
    if (specificCallback_ == nullptr ||
        specificCallback_->onGetNextAvoidAreaRectInfo_(
            displayId, type, nextSystemBarAvoidAreaRectInfo) != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get nextSystemBarAvoidAreaRectInfo failed.");
        return;
    }
    WindowType winType = (type == AvoidAreaType::TYPE_SYSTEM) ? WindowType::WINDOW_TYPE_STATUS_BAR :
                                                                WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR;
    if (auto iter = properties.find(winType); iter == properties.end()) {
        TLOGE(WmsLogTag::WMS_IMMS, "win [%{public}d] properties is empty, type %{public}d", GetPersistentId(), type);
        return;
    }
    bool isStatusBarAvoidAreaEmpty = winType == WindowType::WINDOW_TYPE_STATUS_BAR &&
        (rotation == Rotation::ROTATION_90 || rotation == Rotation::ROTATION_270) &&
        !(static_cast<bool>(static_cast<uint32_t>(properties.at(winType).settingFlag_) &
        static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING)));
    if (!properties.at(winType).enable_ || isStatusBarAvoidAreaEmpty) {
        TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}d] avoid area is empty, type %{public}d", GetPersistentId(), type);
        return;
    }
    WSRect avoidRect = (rotation == Rotation::ROTATION_0 || rotation == Rotation::ROTATION_180) ?
        nextSystemBarAvoidAreaRectInfo.first : nextSystemBarAvoidAreaRectInfo.second;
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d tyep %{public}d rect %{public}s bar %{public}s",
        GetPersistentId(), type, rect.ToString().c_str(), avoidRect.ToString().c_str());
    CalculateAvoidAreaRect(rect, avoidRect, avoidArea);
}

void SceneSession::GetCutoutAvoidAreaByRotation(Rotation rotation, const WSRect& rect, AvoidArea& avoidArea)
{
    auto cutoutInfo = DisplayManager::GetInstance().GetCutoutInfoWithRotation(rotation);
    if (cutoutInfo == nullptr) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutout info");
        return;
    }
    std::vector<DMRect> cutoutAreas = cutoutInfo->GetBoundingRects();
    if (cutoutAreas.empty()) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutout area");
        return;
    }
    for (auto& cutoutArea : cutoutAreas) {
        WSRect cutoutAreaRect = {
            cutoutArea.posX_,
            cutoutArea.posY_,
            cutoutArea.width_,
            cutoutArea.height_
        };
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}s cutout %{public}s",
              rect.ToString().c_str(), cutoutAreaRect.ToString().c_str());
        CalculateAvoidAreaRect(rect, cutoutAreaRect, avoidArea);
    }
}

void SceneSession::GetKeyboardAvoidAreaByRotation(Rotation rotation, const WSRect& rect, AvoidArea& avoidArea)
{
    std::vector<std::pair<bool, WSRect>> avoidInfoVector;
    GetKeyboardOccupiedAreaWithRotation(GetPersistentId(), rotation, avoidInfoVector);
    for (auto avoidInfo : avoidInfoVector) {
        if (!avoidInfo.first) {
            TLOGI(WmsLogTag::WMS_IMMS, "keyboard avoid area is empty id: %{public}d", GetPersistentId());
            continue;
        }
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}s keyboard %{public}s",
            rect.ToString().c_str(), avoidInfo.second.ToString().c_str());
            CalculateAvoidAreaRect(rect, avoidInfo.second, avoidArea);
    }
}

WSError SceneSession::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    if (!GetForegroundInteractiveStatus()) {
        TLOGD(WmsLogTag::WMS_IMMS, "win [%{public}d] avoid area update rejected by recent", GetPersistentId());
        return WSError::WS_DO_NOTHING;
    }
    return sessionStage_->UpdateAvoidArea(avoidArea, type);
}

WSError SceneSession::SetPipActionEvent(const std::string& action, int32_t status)
{
    TLOGI(WmsLogTag::WMS_PIP, "action: %{public}s, status: %{public}d", action.c_str(), status);
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->SetPipActionEvent(action, status);
}

WSError SceneSession::SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType: %{public}u, status: %{public}u", controlType, status);
    if (GetWindowType() != WindowType::WINDOW_TYPE_PIP || GetWindowMode() != WindowMode::WINDOW_MODE_PIP) {
        return WSError::WS_ERROR_INVALID_TYPE;
    }
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->SetPiPControlEvent(controlType, status);
}

WSError SceneSession::NotifyPipWindowSizeChange(double width, double height, double scale)
{
    TLOGI(WmsLogTag::WMS_PIP, "width: %{public}f, height: %{public}f scale: %{public}f", width, height, scale);
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyPipWindowSizeChange(width, height, scale);
}

WSError SceneSession::NotifyPiPActiveStatusChange(bool status)
{
    TLOGI(WmsLogTag::WMS_PIP, "status: %{public}u", status);
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_PIP, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    pipActiveStatus_.store(status);
    return sessionStage_->NotifyPiPActiveStatusChange(status);
}

void SceneSession::RegisterProcessPrepareClosePiPCallback(NotifyPrepareClosePiPSessionFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null", where);
            return;
        }
        session->onPrepareClosePiPSession_ = std::move(callback);
    }, __func__);
}

WSError SceneSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto& id = GetPersistentId();
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "id:%{public}d, type:%{public}d", id, GetWindowType());

    if (static_cast<MMI::WindowInputType>(GetSessionInfo().windowInputType_) == MMI::WindowInputType::TRANSMIT_ALL) {
        TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "Cancel the double_send mechanism window response touchoutside.");
        return WSError::WS_ERROR_INVALID_TYPE;
    }

    // notify touch outside
    if (specificCallback_ != nullptr && specificCallback_->onSessionTouchOutside_ &&
        sessionInfo_.bundleName_.find("SCBGestureBack") == std::string::npos) {
        specificCallback_->onSessionTouchOutside_(id, GetDisplayId());
    }

    // notify outside down event
    if (specificCallback_ != nullptr && specificCallback_->onOutsideDownEvent_) {
        specificCallback_->onOutsideDownEvent_(posX, posY);
    }
    return WSError::WS_OK;
}

WSError SceneSession::SendPointEventForMoveDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool isExecuteDelayRaise)
{
    NotifyOutsideDownEvent(pointerEvent);
    TransferPointerEvent(pointerEvent, false, isExecuteDelayRaise);
    return WSError::WS_OK;
}

WSError SceneSession::SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!IsSystemSession() && !IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    HandlePointerEventForFocus(pointerEvent, false);
    return WSError::WS_OK;
}

void SceneSession::NotifyOutsideDownEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    // notify touchOutside and touchDown event
    if (static_cast<MMI::WindowInputType>(GetSessionInfo().windowInputType_) == MMI::WindowInputType::TRANSMIT_ALL) {
        TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "Cancel the double_send mechanism window response touchoutside.");
        return;
    }

    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_DOWN &&
        action != MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        return;
    }

    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        return;
    }

    // notify touch outside
    if (specificCallback_ != nullptr && specificCallback_->onSessionTouchOutside_ &&
        sessionInfo_.bundleName_.find("SCBGestureBack") == std::string::npos) {
        specificCallback_->onSessionTouchOutside_(GetPersistentId(), GetDisplayId());
    }

    // notify outside down event
    if (specificCallback_ != nullptr && specificCallback_->onOutsideDownEvent_) {
        specificCallback_->onOutsideDownEvent_(pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
    }
}

WSError SceneSession::TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool needNotifyClient, bool isExecuteDelayRaise)
{
    PostTask([weakThis = wptr(this), pointerEvent, needNotifyClient, isExecuteDelayRaise, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::DEFAULT, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        return session->TransferPointerEventInner(pointerEvent, needNotifyClient, isExecuteDelayRaise);
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::TransferPointerEventInner(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool needNotifyClient, bool isExecuteDelayRaise)
{
    WLOGFD("[WMSCom] TransferPointEvent, id: %{public}d, type: %{public}d, needNotifyClient: %{public}d",
        GetPersistentId(), GetWindowType(), needNotifyClient);
    if (pointerEvent == nullptr) {
        WLOGFE("pointerEvent is null");
        return WSError::WS_ERROR_NULLPTR;
    }

    int32_t action = pointerEvent->GetPointerAction();

    if (!CheckPointerEventDispatch(pointerEvent)) {
        WLOGFI("Do not dispatch this pointer event");
        return WSError::WS_DO_NOTHING;
    }

    bool isPointDown = (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);

    {
        std::lock_guard<std::mutex> lock(floatWindowDownEventMutex_);
        if (isPointDown && floatWindowDownEventCnt_ < MAX_DOWN_TIMES) {
            ++floatWindowDownEventCnt_;
        }
    }

    auto property = GetSessionProperty();
    if (property == nullptr) {
        return Session::TransferPointerEvent(pointerEvent, needNotifyClient, isExecuteDelayRaise);
    }
    auto windowType = property->GetWindowType();
    bool isMovableWindowType = IsMovableWindowType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialog = WindowHelper::IsDialogWindow(windowType);
    bool isMaxModeAvoidSysBar = property->GetMaximizeMode() == MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    bool isDragAccessibleSystemWindow = WindowHelper::IsSystemWindow(windowType) && IsDragAccessible() &&
        !isDialog;
    bool isMovableSystemWindow = WindowHelper::IsSystemWindow(windowType) && !isDialog;
    TLOGD(WmsLogTag::WMS_EVENT, "%{public}s: %{public}d && %{public}d", property->GetWindowName().c_str(),
        WindowHelper::IsSystemWindow(windowType), IsDragAccessible());
    if (isMovableWindowType && !isMaxModeAvoidSysBar &&
        (isMainWindow || isSubWindow || isDialog || isDragAccessibleSystemWindow || isMovableSystemWindow)) {
        if (CheckDialogOnForeground() && isPointDown) {
            HandlePointDownDialog();
            pointerEvent->MarkProcessed();
            TLOGI(WmsLogTag::WMS_DIALOG, "There is dialog window foreground");
            return WSError::WS_OK;
        }
        if (!moveDragController_) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "moveDragController_ is null");
            return Session::TransferPointerEvent(pointerEvent, needNotifyClient, isExecuteDelayRaise);
        }
        moveDragController_->SetScale(GetScaleX(), GetScaleY()); // need scale ratio to calculate translate
        SetParentRect();
        if (IsDraggable() && moveDragController_->ConsumeDragEvent(pointerEvent)) {
            auto surfaceNode = GetSurfaceNode();
            moveDragController_->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
            if (isPointDown) {
                ReportDragEndDirection(GetSessionInfo().bundleName_, moveDragController_->GetAreaType());
            }
            PresentFocusIfNeed(pointerEvent->GetPointerAction());
            pointerEvent->MarkProcessed();
            return WSError::WS_OK;
        }
        if ((WindowHelper::IsMainWindow(windowType) ||
             WindowHelper::IsSubWindow(windowType) ||
             WindowHelper::IsSystemWindow(windowType)) && !isFollowParentLayout_ &&
            moveDragController_->ConsumeMoveEvent(pointerEvent)) {
            PresentFocusIfNeed(pointerEvent->GetPointerAction());
            pointerEvent->MarkProcessed();
            Session::TransferPointerEvent(pointerEvent, needNotifyClient, isExecuteDelayRaise);
            ProcessWindowMoving(pointerEvent);
            return WSError::WS_OK;
        }
    }
    return Session::TransferPointerEvent(pointerEvent, needNotifyClient, isExecuteDelayRaise);
}

void SceneSession::ReportDragEndDirection(const std::string& bundleName, AreaType dragType)
{
    HiSysEventWrite(SCENE_BOARD_UE_DOMAIN, DRAG_RESIZE_WINDOW,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "BUNDLENAME", bundleName,
        "DRAGDIRECTION", static_cast<int32_t>(dragType));
}

void SceneSession::NotifyUpdateGravity()
{
    std::unordered_map<int32_t, NotifySurfaceBoundsChangeFunc> funcMap;
    {
        std::lock_guard lock(registerNotifySurfaceBoundsChangeMutex_);
        funcMap = notifySurfaceBoundsChangeFuncMap_;
    }
    for (const auto& [sessionId, _] : funcMap) {
        auto subSession = GetSceneSessionById(sessionId);
        if (!subSession || !subSession->GetIsFollowParentLayout()) {
            return;
        }
        auto surfaceNode = subSession->GetSurfaceNode();
        auto subController = subSession->GetMoveDragController();
        if (subController && surfaceNode) {
            subController->UpdateSubWindowGravityWhenFollow(moveDragController_, surfaceNode);
        }
    }
}

void SceneSession::ProcessWindowMoving(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        return;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        return;
    }
    if (notifyWindowMovingFunc_) {
        notifyWindowMovingFunc_(static_cast<DisplayId>(pointerEvent->GetTargetDisplayId()),
            pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
    }
}

void SceneSession::SetWindowMovingCallback(NotifyWindowMovingFunc&& func)
{
    PostTask([weakThis = wptr(this), where = __func__, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: session or func is null", where);
            return;
        }
        session->notifyWindowMovingFunc_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d", where,
            session->GetPersistentId());
    }, __func__);
}

void SceneSession::SetTransitionAnimationCallback(UpdateTransitionAnimationFunc&& func)
{
    PostTask([weakThis = wptr(this), where = __func__, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: session or func is null", where);
            return;
        }
        session->updateTransitionAnimationFunc_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_ANIMATION, "%{public}s id: %{public}d", where, session->GetPersistentId());
    }, __func__);
}

bool SceneSession::IsMovableWindowType() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "property is null");
        return false;
    }

    return property->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING ||
        IsSplitMovable() || IsFullScreenMovable();
}

bool SceneSession::IsSplitMovable() const
{
    auto property = GetSessionProperty();
    return (property->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
        property->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) &&
        WindowHelper::IsWindowModeSupported(property->GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FLOATING);
}

bool SceneSession::IsFullScreenMovable() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "property is null");
        return false;
    }
    return property->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN &&
        WindowHelper::IsWindowModeSupported(property->GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FLOATING);
}

bool SceneSession::IsMovable() const
{
    if (!moveDragController_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "moveDragController_ is null, id: %{public}d", GetPersistentId());
        return false;
    }

    bool windowIsMovable = !moveDragController_->GetStartDragFlag() && IsMovableWindowType() &&
                           moveDragController_->HasPointDown() && moveDragController_->GetMovable() &&
                           !GetWindowAnchorInfo().isAnchorEnabled_;
    auto property = GetSessionProperty();
    if (property->GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR &&
        property->GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        windowIsMovable = windowIsMovable && IsFocused();
    }
    if (!windowIsMovable) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Window is not movable, id: %{public}d, startDragFlag: %{public}d, "
            "isFocused: %{public}d, movableWindowType: %{public}d, hasPointDown: %{public}d, movable: %{public}d",
            GetPersistentId(), moveDragController_->GetStartDragFlag(), IsFocused(), IsMovableWindowType(),
            moveDragController_->HasPointDown(), moveDragController_->GetMovable());
        return false;
    }
    return true;
}

bool SceneSession::IsDraggable() const
{
    if (!moveDragController_) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "moveDragController is null, id:%{public}d", GetPersistentId());
        return false;
    }
    auto windowType = GetWindowType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isSystemWindow = WindowHelper::IsSystemWindow(windowType);
    bool isDialog = WindowHelper::IsDialogWindow(windowType);
    bool isDragAccessibleSystemWindow = isSystemWindow && IsDragAccessible() && !isDialog;
    bool isFloatingDragAccessible = GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING && IsDragAccessible();

    bool isPcOrFreeMultiWindowCanDrag = (isFloatingDragAccessible || isDragAccessibleSystemWindow) &&
        (systemConfig_.IsPcWindow() || IsFreeMultiWindowMode() ||
        (GetSessionProperty()->GetIsPcAppInPad() && !isMainWindow));
    bool isPhoneWindowCanDrag = isFloatingDragAccessible && (isSystemWindow || isSubWindow) &&
        (systemConfig_.IsPhoneWindow() || (systemConfig_.IsPadWindow() && !IsFreeMultiWindowMode()));
    return isPcOrFreeMultiWindowCanDrag || isPhoneWindowCanDrag;
}

WSError SceneSession::RequestSessionBack(bool needMoveToBackground)
{
    PostTask([weakThis = wptr(this), needMoveToBackground, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->backPressedFunc_) {
            TLOGNW(WmsLogTag::WMS_EVENT, "%{public}s Session didn't register back event consumer!", where);
            return WSError::WS_DO_NOTHING;
        }
        if (g_enableForceUIFirst) {
            auto leashWinShadowSurfaceNode = session->GetLeashWinShadowSurfaceNode();
            if (leashWinShadowSurfaceNode) {
                AutoRSTransaction trans(session->GetRSLeashWinShadowContext());
                leashWinShadowSurfaceNode->SetForceUIFirst(true);
                session->isUIFirstEnabled_ = true;
                TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s leashWinShadowSurfaceNode_ SetForceUIFirst id:%{public}u!",
                    where, session->GetPersistentId());
            } else {
                TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s failed, leashWinShadowSurfaceNode_ null id:%{public}u",
                    where, session->GetPersistentId());
            }
        }
        session->backPressedFunc_(needMoveToBackground);
        return WSError::WS_OK;
    }, std::string(__func__) + ":" + std::to_string(needMoveToBackground));
    return WSError::WS_OK;
}

#ifdef DEVICE_STATUS_ENABLE
void SceneSession::RotateDragWindow(std::shared_ptr<RSTransaction> rsTransaction)
{
    Msdp::DeviceStatus::DragState state = Msdp::DeviceStatus::DragState::STOP;
    Msdp::DeviceStatus::InteractionManager::GetInstance()->GetDragState(state);
    if (state == Msdp::DeviceStatus::DragState::START) {
        Msdp::DeviceStatus::InteractionManager::GetInstance()->RotateDragWindowSync(rsTransaction);
    }
}
#endif // DEVICE_STATUS_ENABLE

/** @note @window.layout */
void SceneSession::NotifySessionRectChange(const WSRect& rect,
    SizeChangeReason reason, DisplayId displayId, const RectAnimationConfig& rectAnimationConfig)
{
    if (IsDragResizeScale(reason)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "compatiblemode drag scale no need notify rect change");
        return;
    }
    PostTask([weakThis = wptr(this), rect, reason, displayId, rectAnimationConfig, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        if (session->sessionRectChangeFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::NotifySessionRectChange");
            session->sessionRectChangeFunc_(rect, reason, displayId, rectAnimationConfig);
        }
    }, __func__ + GetRectInfo(rect));
}

/** @note @window.layout */
void SceneSession::NotifySessionWindowLimitsChange(const WindowLimits& windowlimits)
{
    PostTask([weakThis = wptr(this), windowlimits, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        if (session->sessionWindowLimitsChangeFunc_) {
            session->sessionWindowLimitsChangeFunc_(windowlimits);
        }
        }, __func__);
}

/** @note @window.layout */
void SceneSession::NotifySessionDisplayIdChange(uint64_t displayId)
{
    PostTask([weakThis = wptr(this), displayId] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "session is null");
            return;
        }
        if (session->sessionDisplayIdChangeFunc_) {
            session->sessionDisplayIdChangeFunc_(displayId);
            session->HandleLayoutAvoidAreaUpdate();
        }
    }, __func__);
}

void SceneSession::CheckSubSessionShouldFollowParent(uint64_t displayId)
{
    for (const auto& session : subSession_) {
        if (session && session->IsSessionForeground() && session->GetSessionProperty()->GetDisplayId() != displayId) {
            session->SetShouldFollowParentWhenShow(false);
        }
    }
}

bool SceneSession::IsDecorEnable() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        WLOGE("property is nullptr");
        return false;
    }
    auto windowType = property->GetWindowType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    bool isValidWindow = isMainWindow ||
        ((isSubWindow || isDialogWindow) && property->IsDecorEnable());
    bool isWindowModeSupported = WindowHelper::IsWindowModeSupported(
        systemConfig_.decorWindowModeSupportType_, property->GetWindowMode());
    bool enable = isValidWindow && systemConfig_.isSystemDecorEnable_ && isWindowModeSupported;
    return enable;
}

std::string SceneSession::GetRatioPreferenceKey()
{
    std::string key = sessionInfo_.bundleName_ + sessionInfo_.moduleName_ + sessionInfo_.abilityName_;
    if (key.length() > ScenePersistentStorage::MAX_KEY_LEN) {
        return key.substr(key.length() - ScenePersistentStorage::MAX_KEY_LEN);
    }
    return key;
}

bool SceneSession::SaveAspectRatio(float ratio)
{
    std::string key = GetRatioPreferenceKey();
    if (!key.empty()) {
        ScenePersistentStorage::Insert(key, ratio, ScenePersistentStorageType::ASPECT_RATIO);
        WLOGD("SceneSession save aspectRatio , key %{public}s, value: %{public}f", key.c_str(), GetAspectRatio());
        return true;
    }
    return false;
}

/** @note @window.drag */
void SceneSession::InitializeCrossMoveDrag()
{
    auto movedSurfaceNode = GetSurfaceNodeForMoveDrag();
    if (!movedSurfaceNode) {
        return;
    }
    auto parentNode = movedSurfaceNode->GetParent();
    if (!parentNode) {
        return;
    }
    auto property = GetSessionProperty();
    if (!property) {
        return;
    }
    auto originalPositionZ = movedSurfaceNode->GetStagingProperties().GetPositionZ();
    moveDragController_->SetOriginalPositionZ(originalPositionZ);
    moveDragController_->InitCrossDisplayProperty(property->GetDisplayId(), parentNode->GetId());
}

/** @note @window.drag */
void SceneSession::HandleMoveDragSurfaceBounds(
    WSRect& rect, WSRect& globalRect, SizeChangeReason reason, TargetRectUpdateMode mode)
{
    // NOTE: This method currently mixes multiple responsibilities (drag state
    // handling, slip animation logic, vsync scheduling, and resample dispatch).
    // It should be refactored to separate concerns and improve maintainability.

    bool isGlobal = (reason != SizeChangeReason::DRAG_END);
    bool needFlush = (reason != SizeChangeReason::DRAG_END);
    bool needSetBoundsNextVsync = false;

    throwSlipToFullScreenAnimCount_.store(0);
    UpdateSizeChangeReason(reason);

    const bool isTouchDrag = moveDragController_ &&
                             moveDragController_->GetPointerType() == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    const bool allowThrowSlip = pcFoldScreenController_ && pcFoldScreenController_->IsAllowThrowSlip(GetScreenId());
    const bool isDragMoveOrEnd = reason == SizeChangeReason::DRAG_MOVE || reason == SizeChangeReason::DRAG_END;

    if (isTouchDrag && allowThrowSlip && isDragMoveOrEnd) {
        // Foldable PC Slip Animation Path
        bool hasAnimation = reason == SizeChangeReason::DRAG_END && pcFoldScreenController_->NeedFollowHandAnimation();
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "reason=%{public}u, hasAnimation=%{public}d",
              static_cast<uint32_t>(reason), hasAnimation);
        if (hasAnimation) {
            auto movingPair = std::make_pair(pcFoldScreenController_->GetMovingTimingProtocol(),
                                             pcFoldScreenController_->GetMovingTimingCurve());
            SetSurfaceBoundsWithAnimation(movingPair, globalRect, nullptr, isGlobal);
        } else {
            SetSurfaceBounds(globalRect, isGlobal, needFlush);
        }
        pcFoldScreenController_->RecordMoveRects(rect);
    } else if (reason == SizeChangeReason::DRAG_START) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Drag-start event doesn't set surface bounds");
    } else {
        if (mode != TargetRectUpdateMode::RESAMPLE_SCHEDULED &&
            (reason != SizeChangeReason::DRAG || GetKeyFramePolicy().running_)) {
            SetSurfaceBounds(globalRect, isGlobal, needFlush);
        } else {
            needSetBoundsNextVsync = true;
        }
    }

    if (reason != SizeChangeReason::DRAG_MOVE && !KeyFrameNotifyFilter(rect, reason)) {
        UpdateRectForDrag(rect);
        RequestNextVsyncWhenDrag(globalRect, isGlobal, needFlush, needSetBoundsNextVsync);
    }

    // Request resample on next vsync if needed.
    if (mode == TargetRectUpdateMode::RESAMPLE_ACTIVATED) {
        RequestMoveResampleOnNextVsync();
    }
}

void SceneSession::RequestNextVsyncWhenDrag(
    const WSRect& globalRect, bool isGlobal, bool needFlush, bool needSetBoundsNextVsync)
{
    RunOnNextVsync([weakThis = wptr(this), globalRect, isGlobal,
                    needFlush, needSetBoundsNextVsync, where = __func__](int64_t, int64_t) {
        auto session = weakThis.promote();
        RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);
        session->OnNextVsyncReceivedWhenDrag(globalRect, isGlobal, needFlush, needSetBoundsNextVsync);
    });
}

void SceneSession::OnNextVsyncReceivedWhenDrag(
    const WSRect& globalRect, bool isGlobal, bool needFlush, bool needSetBoundsNextVsync)
{
    PostTask([weakThis = wptr(this), globalRect, isGlobal, needFlush, needSetBoundsNextVsync, where = __func__] {
        auto session = weakThis.promote();
        RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);
        if (session->IsDirtyDragWindow()) {
            WSRect winRect = session->GetSessionRect();
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                "SceneSession::OnNextVsyncReceivedWhenDrag id:%d [%d, %d, %d, %d] reason:%u",
                session->GetPersistentId(), winRect.posX_, winRect.posY_,
                winRect.width_, winRect.height_, session->GetSizeChangeReason());
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}u, winRect:%{public}s",
                where, session->GetPersistentId(), winRect.ToString().c_str());
            session->NotifyClientToUpdateRect("OnMoveDragCallback", nullptr);
            if (!session->moveDragController_) {
                TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session moveDragController is null", where);
            } else if (needSetBoundsNextVsync && session->moveDragController_->GetStartDragFlag()) {
                session->SetSurfaceBounds(globalRect, isGlobal, needFlush);
            }
            session->ResetDirtyDragFlags();
        }
    });
}

void SceneSession::RequestMoveResampleOnNextVsync()
{
    RunOnNextVsync([weakThis = wptr(this), where = __func__](int64_t timestamp, int64_t /*frameCount*/) {
        auto session = weakThis.promote();
        RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);

        int64_t vsyncTimeUs = timestamp / 1000; // ns to us
        session->PostTask([weakThis, vsyncTimeUs, where] {
            auto session = weakThis.promote();
            RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);

            session->PerformMoveResampleOnVsync(vsyncTimeUs);
        }, where);
    });
}

void SceneSession::PerformMoveResampleOnVsync(int64_t vsyncTimeUs)
{
    RETURN_IF_NULL_WITH_TAG(moveDragController_, WmsLogTag::WMS_LAYOUT);
    auto [mode, rect] = moveDragController_->ResampleTargetRectOnVsync(vsyncTimeUs);
    if (mode != TargetRectUpdateMode::UPDATED_IMMEDIATELY) {
        return;
    }
    SetSurfaceBounds(rect, true, true);

    // Continue the move resampling loop by scheduling the next vsync iteration.
    RequestMoveResampleOnNextVsync();
}

/** @note @window.drag */
void SceneSession::HandleMoveDragEnd(WSRect& rect, SizeChangeReason reason)
{
    if (GetOriPosYBeforeRaisedByKeyboard() != 0) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is moved and reset oriPosYBeforeRaisedByKeyboard");
        SetOriPosYBeforeRaisedByKeyboard(0);
    }
    if (moveDragController_->GetPointerType() == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN &&
        MoveUnderInteriaAndNotifyRectChange(rect, reason)) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "set full screen after throw slip");
    }

    uint64_t endDisplayId = moveDragController_->GetMoveDragEndDisplayId();
    if (endDisplayId == moveDragController_->GetMoveDragStartDisplayId() ||
        !IsCrossDisplayDragSupported()) {
        NotifySessionRectChange(rect, reason);
        HandleKeyboardMoveDragEnd(rect, reason);
        NotifySubSessionRectChangeByAnchor(rect, SizeChangeReason::UNDEFINED);
    } else {
        NotifySessionRectChange(rect, reason, endDisplayId);
        HandleKeyboardMoveDragEnd(rect, reason, endDisplayId);
        NotifySubSessionRectChangeByAnchor(rect, SizeChangeReason::UNDEFINED, endDisplayId);
        CheckSubSessionShouldFollowParent(endDisplayId);
    }
    moveDragController_->SetLastDragEndRect(rect);
    moveDragController_->ResetCrossMoveDragProperty();
    OnSessionEvent(SessionEvent::EVENT_END_MOVE);
    RestoreGravityWhenDragEnd();
}

/**
 * the window is transformed according to the scale ratio
 */
void SceneSession::WindowScaleTransfer(WSRect& rect, float scaleX, float scaleY)
{
    auto curWidth = rect.width_;
    auto curHeight = rect.height_;
    rect.width_ = static_cast<int32_t>(curWidth * scaleX);
    rect.height_ = static_cast<int32_t>(curHeight * scaleY);
    TLOGI(WmsLogTag::WMS_LAYOUT, "scaleX: %{public}f, scaleY: %{public}f, rect: %{public}s",
        scaleX, scaleY, rect.ToString().c_str());
}

/**
 * hook startMoveRect with showRect
 */
void SceneSession::HookStartMoveRect(WSRect& newRect, const WSRect& sessionRect)
{
    newRect = sessionRect;
    if (!WindowHelper::IsMainWindow(GetWindowType())) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "is not mainWindow");
        return;
    }
    auto scaleX = GetScaleX();
    auto scaleY = GetScaleY();
    if (IsCompatibilityModeScale(scaleX, scaleY)) {
        WindowScaleTransfer(newRect, scaleX, scaleY);
    }
}

/**
 * check compatible mode application that includes scale ratio
 * @return true: compatible mode application with scale ratio
 */
bool SceneSession::IsCompatibilityModeScale(float scaleX, float scaleY)
{
    auto property = GetSessionProperty();
    if (property->IsAdaptToProportionalScale() && MathHelper::GreatNotEqual(scaleX, 0.0f) &&
        MathHelper::GreatNotEqual(scaleY, 0.0f) && (!NearEqual(scaleX, 1.0f) || !NearEqual(scaleY, 1.0f))) {
        return true;
    }
    return false;
}

bool SceneSession::IsInCompatScaleStatus() const
{
    auto property = GetSessionProperty();
    if (property->IsAdaptToProportionalScale() || property->IsAdaptToSimulationScale()) {
        return !NearEqual(GetScaleX(), COMPACT_NORMAL_SCALE) || !NearEqual(GetScaleY(), COMPACT_NORMAL_SCALE);
    }
    return false;
}

bool SceneSession::IsInCompatScaleMode()
{
    if (WindowHelper::IsSubWindow(GetWindowType())) {
        auto mainSession = GetSceneSessionById(GetMainSessionId());
        if (mainSession && GetCallingPid() == mainSession->GetCallingPid()) {
            return mainSession->IsInCompatScaleMode();
        }
    }
    auto property = GetSessionProperty();
    return property->IsAdaptToProportionalScale() || property->IsAdaptToSimulationScale();
}

/**
 * throw slip to full screen
 */
void SceneSession::ThrowSlipToFullScreen(WSRect& endRect, WSRect& rect, int32_t statusBarHeight, int32_t dockHeight)
{
    if (pcFoldScreenController_ == nullptr) {
        return;
    }
    // maximize end rect and notify last rect
    throwSlipToFullScreenAnimCount_.fetch_add(1);
    pcFoldScreenController_->ResizeToFullScreen(endRect, statusBarHeight, dockHeight);
    if (pcFoldScreenController_->IsThrowSlipDirectly()) {
        pcFoldScreenController_->ThrowSlipFloatingRectDirectly(
            rect, GetSessionRequestRect(), statusBarHeight, dockHeight);
    }
}

/**
 * the compatible mode window is being scaled and transformed
 */
void SceneSession::CompatibilityModeWindowScaleTransfer(WSRect& rect, bool isScale)
{
    if (!WindowHelper::IsMainWindow(GetWindowType())) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "is not mainWindow");
        return;
    }
    auto scaleX = GetScaleX();
    auto scaleY = GetScaleY();
    if (MathHelper::NearZero(scaleX) || MathHelper::NearZero(scaleY)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "scale ratio is 0");
        return;
    }
    if (!isScale) {
        if (!MathHelper::NearZero(scaleX)) {
            scaleX = 1 / scaleX;
        }
        if (!MathHelper::NearZero(scaleY)) {
            scaleY = 1 / scaleY;
        }
    }
    if (IsCompatibilityModeScale(scaleX, scaleY)) {
        WindowScaleTransfer(rect, scaleX, scaleY);
    }
}

/**
 * move with init velocity
 * @return true: successfully throw slip
 */
bool SceneSession::MoveUnderInteriaAndNotifyRectChange(WSRect& rect, SizeChangeReason reason)
{
    if (pcFoldScreenController_ == nullptr) {
        return false;
    }
    CompatibilityModeWindowScaleTransfer(rect, true);
    bool isDockAutoHide = onGetIsDockAutoHideFunc_ ? onGetIsDockAutoHideFunc_() : false;
    int32_t statusBarHeight = (IsLayoutFullScreen() || isDockAutoHide) ? 0 : GetStatusBarHeight();
    int32_t dockHeight = (IsLayoutFullScreen() || isDockAutoHide) ? 0 : GetDockHeight();
    bool ret = pcFoldScreenController_->ThrowSlip(GetScreenId(), rect, statusBarHeight, dockHeight);
    if (!ret) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "no throw slip");
        pcFoldScreenController_->ResetRecords();
        CompatibilityModeWindowScaleTransfer(rect, false);
        return false;
    }
    CompatibilityModeWindowScaleTransfer(rect, false);
    WSRect endRect = rect;
    std::function<void()> finishCallback = nullptr;
    bool needSetFullScreen = pcFoldScreenController_->IsStartFullScreen();
    if (needSetFullScreen) {
        ThrowSlipToFullScreen(endRect, rect, statusBarHeight, dockHeight);
        finishCallback = [weakThis = wptr(this), rect, where = __func__] {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is nullptr", where);
                return;
            }
            session->OnThrowSlipAnimationStateChange(false, true);
            session->NotifyFullScreenAfterThrowSlip(rect);
        };
    } else {
        finishCallback = [weakThis = wptr(this), rect, where = __func__] {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is nullptr", where);
                return;
            }
            session->OnThrowSlipAnimationStateChange(false, false);
        };
    }
    auto throwSlipPair = std::make_pair(pcFoldScreenController_->GetThrowSlipTimingProtocol(),
        pcFoldScreenController_->GetThrowSlipTimingCurve());
    SetSurfaceBoundsWithAnimation(throwSlipPair, endRect, finishCallback);
    OnThrowSlipAnimationStateChange(true, needSetFullScreen);
    rect = endRect;
    pcFoldScreenController_->ResetRecords();
    return true;
}

void SceneSession::OnThrowSlipAnimationStateChange(bool isAnimating, bool isFullScreen)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "status: %{public}d", isAnimating);
    if (isAnimating) {
        StartAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(AsyncTraceTaskId::THROW_SLIP_ANIMATION),
            "SceneSession::ThrowSlipAnimation(id:%d)", GetPersistentId());
    } else {
        FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(AsyncTraceTaskId::THROW_SLIP_ANIMATION),
            "SceneSession::ThrowSlipAnimation(id:%d)", GetPersistentId());
    }
    SetUIFirstSwitch(isAnimating ? RSUIFirstSwitch::FORCE_DISABLE : RSUIFirstSwitch::NONE);
    if (onThrowSlipAnimationStateChangeFunc_) {
        onThrowSlipAnimationStateChangeFunc_(isAnimating, isFullScreen);
    }
}

void SceneSession::NotifyFullScreenAfterThrowSlip(const WSRect& rect)
{
    PostTask([weakThis = wptr(this), rect, where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s session is nullptr", where);
            return;
        }
        if (!session->IsVisibleForeground()) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s session go background when throw", where);
            return;
        }
        if (session->throwSlipToFullScreenAnimCount_.load() == 0) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s session moved when throw", where);
            return;
        }
        if (session->throwSlipToFullScreenAnimCount_.load() > 1) {
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s throw-slip fullscreen animation count: %{public}u",
                where, session->throwSlipToFullScreenAnimCount_.load());
            session->throwSlipToFullScreenAnimCount_.fetch_sub(1);
            return;
        }
        session->throwSlipToFullScreenAnimCount_.fetch_sub(1);
        if (!session->onSessionEvent_) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s invalid callback", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s rect: %{public}s", where, rect.ToString().c_str());
        session->onSessionEvent_(
            static_cast<uint32_t>(SessionEvent::EVENT_MAXIMIZE_WITHOUT_ANIMATION),
            SessionEventParam {rect.posX_, rect.posY_, rect.width_, rect.height_});
    }, __func__);
}

void SceneSession::ThrowSlipDirectly(ThrowSlipMode throwSlipMode, const WSRectF& velocity)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), throwSlipMode, velocity, where] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is nullptr", where);
            return;
        }
        auto controller = session->pcFoldScreenController_;
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session: %{public}d, velocity: %{public}s",
            where, session->GetPersistentId(), velocity.ToString().c_str());
        if (!(controller && controller->IsAllowThrowSlip(session->GetScreenId()))) {
            TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s not allow throw slip", where);
            return;
        }
        if (!session->IsMovableWindowType()) {
            TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s not movable", where);
            return;
        }
        bool isFullScreen = session->IsFullScreenMovable();
        WSRect currRect;
        session->HookStartMoveRect(currRect, session->GetSessionRect());
        controller->RecordStartMoveRectDirectly(currRect, throwSlipMode, velocity, isFullScreen);
        const WSRect& oriGlobalRect = session->GetSessionGlobalRect();
        WSRect globalRect = oriGlobalRect;
        if (!session->MoveUnderInteriaAndNotifyRectChange(globalRect, SizeChangeReason::UNDEFINED)) {
            TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s no throw", where);
            return;
        }
        if (isFullScreen) {
            session->UpdateFullScreenWaterfallMode(false);
        }
        WSRect rect = session->GetSessionRect();
        rect.posX_ += globalRect.posX_ - oriGlobalRect.posX_;
        rect.posY_ += globalRect.posY_ - oriGlobalRect.posY_;
        session->NotifySessionRectChange(globalRect, SizeChangeReason::UNDEFINED);
    }, __func__);
}

/** @note @window.drag */
void SceneSession::OnMoveDragCallback(SizeChangeReason reason, TargetRectUpdateMode mode)
{
    RETURN_IF_NULL_WITH_TAG(moveDragController_, WmsLogTag::WMS_LAYOUT);
    if (reason == SizeChangeReason::DRAG_START) {
        InitializeCrossMoveDrag();
    }
    WSRect rect = moveDragController_->GetTargetRect(
        MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "SceneSession::OnMoveDragCallback id:%d [%d, %d, %d, %d] reason:%u", GetPersistentId(), rect.posX_, rect.posY_,
        rect.width_, rect.height_, reason);
    if (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_END) {
        auto property = GetSessionProperty();
        if (property && !property->IsAdaptToDragScale()) {
            UpdateWinRectForSystemBar(rect);
        }
    }
    HandleSubSessionCrossNode(reason);
    moveDragController_->UpdateTargetRect(reason, rect);
    if (DragResizeWhenEndFilter(reason)) {
        return;
    }
    UpdateKeyFrameState(reason, rect);
    bool isRelatedToEndDisplay = reason == SizeChangeReason::DRAG_END && IsCrossDisplayDragSupported();
    WSRect relativeRect = moveDragController_->GetTargetRect(isRelatedToEndDisplay ?
        MoveDragController::TargetRectCoordinate::RELATED_TO_END_DISPLAY :
        MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    WSRect globalRect = moveDragController_->GetTargetRect(isRelatedToEndDisplay ?
        MoveDragController::TargetRectCoordinate::RELATED_TO_END_DISPLAY :
        MoveDragController::TargetRectCoordinate::GLOBAL);
    HandleMoveDragSurfaceNode(reason);
    HandleMoveDragSurfaceBounds(relativeRect, globalRect, reason, mode);
    if (reason == SizeChangeReason::DRAG_END) {
        HandleMoveDragEnd(relativeRect, reason);
        SetUIFirstSwitch(RSUIFirstSwitch::NONE);
    } else if (reason == SizeChangeReason::DRAG_START) {
        OnSessionEvent(SessionEvent::EVENT_DRAG_START);
        NotifyUpdateGravity();
        SetUIFirstSwitch(RSUIFirstSwitch::FORCE_DISABLE);
    }
}

void SceneSession::HandleSubSessionCrossNode(SizeChangeReason reason)
{
    if (IsDraggingReason(reason) && !Session::IsDragStart()) {
        Session::SetDragStart(true);
        if (!subSession_.empty()) {
            HandleSubSessionSurfaceNode(true, GetDisplayId());
        }
    } else if (reason == SizeChangeReason::DRAG_END) {
        Session::SetDragStart(false);
        if (!subSession_.empty()) {
            HandleSubSessionSurfaceNode(false, GetDisplayId());
        }
    }
}

bool SceneSession::DragResizeWhenEndFilter(SizeChangeReason reason)
{
    auto property = GetSessionProperty();
    if (property == nullptr || moveDragController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "property or controller is null");
        return true;
    }
    bool isPcOrPcModeMainWindow = (systemConfig_.IsPcWindow() || IsFreeMultiWindowMode()) &&
        WindowHelper::IsMainWindow(property->GetWindowType());
    bool isReasonMatched = reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_END;
    bool isResizeWhenEnd = (isReasonMatched && isPcOrPcModeMainWindow &&
        GetDragResizeTypeDuringDrag() == DragResizeType::RESIZE_WHEN_DRAG_END) || IsDragResizeScale(reason);
    if (isResizeWhenEnd) {
        UpdateSizeChangeReason(reason);
        if (reason == SizeChangeReason::DRAG) {
            NotifyDragEventOnNextVsync(reason);
        } else {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "trigger client rect change by scb, "
                "isPcOrPcMode: %{public}d", systemConfig_.IsPcWindow());
            bool isRelatedToStartDisplay = (systemConfig_.IsPcWindow() &&
                                            GetDragResizeTypeDuringDrag() == DragResizeType::RESIZE_WHEN_DRAG_END) ||
                                           !IsCrossDisplayDragSupported();
            WSRect relativeRect = moveDragController_->GetTargetRect(
                isRelatedToStartDisplay ?
                MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY :
                MoveDragController::TargetRectCoordinate::RELATED_TO_END_DISPLAY);
            HandleMoveDragEnd(relativeRect, reason);
            SetUIFirstSwitch(RSUIFirstSwitch::NONE);
        }
    }
    return isResizeWhenEnd;
}

void SceneSession::NotifyDragEventOnNextVsync(SizeChangeReason reason)
{
    needNotifyDragEventOnNextVsync_ = true;
    RunOnNextVsync([weakThis = wptr(this), reason, where = __func__](int64_t, int64_t) {
        auto session = weakThis.promote();
        RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);
        if (session->GetSizeChangeReason() == reason && session->IsNeedNotifyDragEventOnNextVsync()) {
            session->OnSessionEvent(SessionEvent::EVENT_DRAG);
            session->NotifiedDragEventOnNextVsync();
        }
    });
}

bool SceneSession::IsDragResizeScale(SizeChangeReason reason)
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "property is null");
        return false;
    }
    bool isPcOrPcModeMainWindow = (systemConfig_.IsPcWindow() || IsFreeMultiWindowMode()) &&
        WindowHelper::IsMainWindow(property->GetWindowType());
    bool result = (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_END) &&
        isPcOrPcModeMainWindow && GetDragResizeTypeDuringDrag() == DragResizeType::RESIZE_SCALE;
    return result;
}

WSError SceneSession::UpdateKeyFrameCloneNode(std::shared_ptr<RSWindowKeyFrameNode>& rsKeyFrameNode,
    std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "in");
    if (keyFrameCloneNode_) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "keyFrameCloneNode_ already exist");
        return WSError::WS_OK;
    }
    if (!rsKeyFrameNode || !sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "get nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto rsUIContext = GetRSUIContext();
    RSAdapterUtil::SetRSUIContext(rsKeyFrameNode, rsUIContext, true);
    RSAdapterUtil::SetRSTransactionHandler(rsTransaction, rsUIContext);
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "begin rsTransaction");
    }
    keyFrameCloneNode_ = rsKeyFrameNode;
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "change key frame node to %{public}" PRIu64 "", keyFrameCloneNode_->GetId());
    sessionStage_->LinkKeyFrameNode();
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "commit rsTransaction");
    }
    return WSError::WS_OK;
}

KeyFramePolicy SceneSession::GetKeyFramePolicy() const
{
    std::lock_guard<std::mutex> lock(keyFrameMutex_);
    return keyFramePolicy_;
}

void SceneSession::UpdateKeyFramePolicy(bool running, bool stopping)
{
    std::lock_guard<std::mutex> lock(keyFrameMutex_);
    keyFramePolicy_.running_ = running;
    keyFramePolicy_.stopping_ = stopping;
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "update keyframe policy: %{public}s", keyFramePolicy_.ToString().c_str());
}

WSError SceneSession::SetDragKeyFramePolicy(const KeyFramePolicy& keyFramePolicy)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "set keyframe policy from outside: %{public}s", keyFramePolicy.ToString().c_str());
    auto currentKeyFramePolicy = GetKeyFramePolicy();
    bool running = currentKeyFramePolicy.running_;
    if (!running && keyFramePolicy.enabled() &&
        moveDragController_ != nullptr && moveDragController_->GetStartDragFlag() &&
        GetAppDragResizeType() != DragResizeType::RESIZE_WHEN_DRAG_END) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "key frame policy is enabled during resize");
        {
            std::lock_guard<std::mutex> lock(keyFrameMutex_);
            keyFramePolicy_ = keyFramePolicy;
        }
        InitKeyFrameState(GetSessionRect());
        NotifyClientToUpdateRect("OnMoveDragCallback", nullptr);
    } else {
        bool stopping = currentKeyFramePolicy.stopping_;
        std::lock_guard<std::mutex> lock(keyFrameMutex_);
        keyFramePolicy_ = keyFramePolicy;
        keyFramePolicy_.running_ = running;
        keyFramePolicy_.stopping_ = stopping;
    }
    return WSError::WS_OK;
}

void SceneSession::UpdateKeyFrameState(SizeChangeReason reason, const WSRect& rect)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d rect: %{public}s reason: %{public}d",
        GetPersistentId(), rect.ToString().c_str(), reason);
    if (!moveDragController_ || !sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "no moveDragController or sessionStage");
        return;
    }
    if (reason == SizeChangeReason::DRAG_START && moveDragController_->GetStartDragFlag()) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "key frame start check");
        auto keyFramePolicy = GetKeyFramePolicy();
        if (!keyFramePolicy.enabled() || GetAppDragResizeType() == DragResizeType::RESIZE_WHEN_DRAG_END) {
            UpdateKeyFramePolicy(false, keyFramePolicy.stopping_);
            return;
        }
        return InitKeyFrameState(rect);
    }
    if (!GetKeyFramePolicy().running_) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "key frame not start");
        return;
    }
    if (reason == SizeChangeReason::DRAG && keyFrameCloneNode_ != nullptr) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "reset gravity and resize clone node");
        uint64_t timeStamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        if (keyFrameDragPauseNoticed_) {
            lastKeyFrameStamp_ = timeStamp;
        }
        lastKeyFrameDragStamp_ = timeStamp;
        lastKeyFrameDragRect_ = rect;
        keyFrameDragPauseNoticed_ = false;
        SetFrameGravity(Gravity::DEFAULT);
        keyFrameCloneNode_->SetBounds(0, 0, rect.width_, rect.height_);
        keyFrameCloneNode_->SetFrame(0, 0, rect.width_, rect.height_);
        RSTransactionAdapter::FlushImplicitTransaction(GetRSUIContext());
    } else if (reason == SizeChangeReason::DRAG_END) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "key frame stopping");
        UpdateKeyFramePolicy(false, true);
        sessionStage_->SetStageKeyFramePolicy(GetKeyFramePolicy());
        keyFrameCloneNode_ = nullptr;
    }
}

void SceneSession::InitKeyFrameState(const WSRect& rect)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "key frame start init");
    uint64_t timeStamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    keyFrameCloneNode_ = nullptr;
    UpdateKeyFramePolicy(true, false);
    keyFrameAnimating_ = false;
    lastKeyFrameStamp_ = timeStamp;
    lastKeyFrameRect_ = rect;
    lastKeyFrameDragRect_ = rect;
    keyFrameVsyncRequestStamp_ = timeStamp;
    lastKeyFrameDragStamp_ = timeStamp;
    keyFrameDragPauseNoticed_ = false;
    sessionStage_->SetStageKeyFramePolicy(GetKeyFramePolicy());
    RequestKeyFrameNextVsync(keyFrameVsyncRequestStamp_, 0);
}

void SceneSession::RequestKeyFrameNextVsync(uint64_t requestStamp, uint64_t count)
{
    if (GetKeyFramePolicy().running_ && requestStamp == keyFrameVsyncRequestStamp_) {
        RunOnNextVsync([weakThis = wptr(this), requestStamp, count, where = __func__](int64_t, int64_t) {
            auto session = weakThis.promote();
            RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);
            session->OnKeyFrameNextVsync(count);
            session->RequestKeyFrameNextVsync(requestStamp, count + 1);
        });
    } else {
        TLOGI(WmsLogTag::WMS_LAYOUT, "stop");
    }
}

void SceneSession::OnKeyFrameNextVsync(uint64_t count)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "get vsync %{public}" PRIu64, count);
    uint64_t nowTimeStamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    uint64_t duration = nowTimeStamp - lastKeyFrameDragStamp_;
    const uint64_t minDelay = 100;
    if (!keyFrameDragPauseNoticed_ && !keyFrameAnimating_ && duration >= minDelay) {
        bool isToNotice = true;
        if (KeyFrameRectAlmostSame(lastKeyFrameRect_, lastKeyFrameDragRect_)) {
            isToNotice = false;
        }
        keyFrameDragPauseNoticed_ = true;
        lastKeyFrameDragStamp_ = nowTimeStamp;
        lastKeyFrameStamp_ = nowTimeStamp;
        layoutController_->SetSessionRect(lastKeyFrameDragRect_);
        lastKeyFrameRect_ = lastKeyFrameDragRect_;
        if (isToNotice) {
            NotifyClientToUpdateRect("OnMoveDragCallback", nullptr);
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "to notice for key frame drag paused: %{public}d", isToNotice);
    }
}

bool SceneSession::KeyFrameNotifyFilter(const WSRect& rect, SizeChangeReason reason)
{
    KeyFramePolicy keyFramePolicy = GetKeyFramePolicy();
    if (!keyFramePolicy.running_) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "skip filter for not running");
        return false;
    }
    if (reason == SizeChangeReason::DRAG_START) {
        NotifyClientToUpdateRect("OnMoveDragCallback", nullptr);
        return true;
    }
    if (reason != SizeChangeReason::DRAG) {
        return false;
    }
    if (keyFrameAnimating_ || keyFrameCloneNode_ == nullptr) {
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "filter for animating or keyframe node not ready");
        return true;
    }
    bool isToFilter = true;
    uint64_t nowTimeStamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    uint64_t interval = nowTimeStamp - lastKeyFrameStamp_;
    bool intervalCheckPass = interval >= keyFramePolicy.interval_;
    bool distanceCheckPass = false;
    double distance = 0;
    if (keyFramePolicy.distance_ > 0) {
        distance = sqrt(pow(rect.width_ - lastKeyFrameRect_.width_, POW_DOUBLE) +
                        pow(rect.height_ - lastKeyFrameRect_.height_, POW_DOUBLE));
        distanceCheckPass = distance >= keyFramePolicy.distance_;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "key frame checking: %{public}" PRIu64 "[%{public}d], %{public}f[%{public}d]",
        interval, intervalCheckPass, distance, distanceCheckPass);
    if (intervalCheckPass || distanceCheckPass) {
        isToFilter = false;
    }
    if (!isToFilter && KeyFrameRectAlmostSame(lastKeyFrameRect_, rect)) {
        isToFilter = true;
        lastKeyFrameStamp_ = nowTimeStamp;
        lastKeyFrameRect_ = rect;
    }
    if (!isToFilter) {
        lastKeyFrameStamp_ = nowTimeStamp;
        lastKeyFrameRect_ = rect;
        keyFrameAnimating_ = true;
    }
    return isToFilter;
}

bool SceneSession::KeyFrameRectAlmostSame(const WSRect& rect1, const WSRect& rect2)
{
    const int32_t MAX_DIFF_THREDHOLD_FOR_TOUCH_EVENT = 3;
    if (rect1.isNearEqual(rect2, MAX_DIFF_THREDHOLD_FOR_TOUCH_EVENT)) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "rect almost same: %{public}s -> %{public}s",
            rect1.ToString().c_str(), rect2.ToString().c_str());
        return true;
    }
    return false;
}

WSError SceneSession::KeyFrameAnimateEnd()
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    keyFrameAnimating_ = false;
    return WSError::WS_OK;
}

std::shared_ptr<Rosen::RSNode> SceneSession::GetWindowDragMoveMountedNode(DisplayId displayId,
    uint32_t targetZOrder)
{
    if (!findScenePanelRsNodeByZOrderFunc_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "findScenePanelRsNodeByZOrderFunc_ is null");
        return nullptr;
    }
    return findScenePanelRsNodeByZOrderFunc_(displayId, targetZOrder);
}

/** @note @window.drag */
void SceneSession::HandleMoveDragSurfaceNode(SizeChangeReason reason)
{
    auto movedSurfaceNode = GetSurfaceNodeForMoveDrag();
    if (movedSurfaceNode == nullptr) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "SurfaceNode is null");
        return;
    }
    const auto startDisplayId = moveDragController_->GetMoveDragStartDisplayId();
    auto startScreenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(startDisplayId);
    if (startScreenSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "startScreenSession is null, startDisplayId: %{public}" PRIu64, startDisplayId);
        return;
    }
    bool isStartScreenMainOrExtend = isMainOrExtendScreenMode(startScreenSession->GetSourceMode());
    if (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_MOVE) {
        for (const auto displayId : moveDragController_->GetNewAddedDisplayIdsDuringMoveDrag()) {
            if (displayId == moveDragController_->GetMoveDragStartDisplayId()) {
                continue;
            }
            auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
            if (screenSession == nullptr) {
                TLOGD(WmsLogTag::WMS_LAYOUT, "ScreenSession is null");
                continue;
            }
            bool isDestScreenMainOrExtend = isMainOrExtendScreenMode(screenSession->GetSourceMode());
            // Not main to extend or extend to main or extend to extend, no need to add cross parent child
            if (!(isStartScreenMainOrExtend && isDestScreenMainOrExtend)) {
                TLOGD(WmsLogTag::WMS_LAYOUT, "No need to add cross-parent child elements for out-of-scope situations, "
                    "DisplayId: %{public}" PRIu64, displayId);
                continue;
            }
            auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
            if (dragMoveMountedNode == nullptr) {
                TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
                continue;
            }
            {
                AutoRSTransaction trans(movedSurfaceNode->GetRSUIContext());
                movedSurfaceNode->SetPositionZ(MOVE_DRAG_POSITION_Z);
                movedSurfaceNode->SetIsCrossNode(true);
            }

            {
                AutoRSTransaction trans(dragMoveMountedNode->GetRSUIContext());
                dragMoveMountedNode->AddCrossScreenChild(movedSurfaceNode, -1, true);
            }
            HandleSubSessionSurfaceNodeByWindowAnchor(reason, displayId);
            TLOGI(WmsLogTag::WMS_LAYOUT, "Add window to display: %{public}" PRIu64 "persistentId: %{public}d",
                displayId, GetPersistentId());
        }
    } else if (reason == SizeChangeReason::DRAG_END) {
        for (const auto displayId : moveDragController_->GetDisplayIdsDuringMoveDrag()) {
            if (displayId == moveDragController_->GetMoveDragStartDisplayId()) {
                continue;
            }
            auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
            if (dragMoveMountedNode == nullptr) {
                TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
                continue;
            }
            movedSurfaceNode->SetPositionZ(moveDragController_->GetOriginalPositionZ());
            dragMoveMountedNode->RemoveCrossScreenChild(movedSurfaceNode);
            // When the drag-to-move or drag-to-scale operation ends, if the window's current screen
            // is the same as the starting screen, the cloned node is removed immediately. Otherwise,
            // the removal of the cloned node is submitted along with the vsync refresh.
            if (!moveDragController_->IsWindowCrossScreenOnDragEnd()) {
                TLOGD(WmsLogTag::WMS_LAYOUT, "Cloned node removed immediately");
                RSTransactionAdapter::FlushImplicitTransaction(dragMoveMountedNode->GetRSUIContext());
            }

            movedSurfaceNode->SetIsCrossNode(false);
            HandleSubSessionSurfaceNodeByWindowAnchor(reason, displayId);
            TLOGI(WmsLogTag::WMS_LAYOUT, "Remove window from display: %{public}" PRIu64 "persistentId: %{public}d",
                displayId, GetPersistentId());
        }
    }
}

void SceneSession::HandleSubSessionSurfaceNode(bool isAdd, DisplayId draggingOrMovingParentDisplayId)
{
    for (const auto& session : subSession_) {
        if (session) {
            session->HandleSubSessionSurfaceNode(isAdd, draggingOrMovingParentDisplayId);
        }
    }

    if (WindowHelper::IsSubWindow(GetWindowType()) &&
        (IsFollowParentMultiScreenPolicy() || (!isAdd && cloneNodeCount_ > 0))) {
        WSRect targetRect = isAdd ? layoutController_->ConvertRelativeRectToGlobal(GetSessionRect(), GetScreenId()) :
            layoutController_->ConvertGlobalRectToRelative(GetSessionRect(), GetScreenId());
        TLOGI(WmsLogTag::WMS_LAYOUT, "isAdd:%{public}d targetRect:%{public}s", isAdd, targetRect.ToString().c_str());
        if (isAdd) {
            AddSurfaceNodeToScreen(draggingOrMovingParentDisplayId);
            SetSurfaceBounds(targetRect, true, true);
        } else {
            RemoveSurfaceNodeFromScreen();
            SetSurfaceBounds(targetRect, false, false);
            {
                std::lock_guard<std::mutex> lock(displayIdSetDuringMoveToMutex_);
                displayIdSetDuringMoveTo_.clear();
            }
            auto lastRect = GetRequestRectWhenFollowParent();
            if (lastRect.IsInvalid()) {
                return;
            }
            auto moveConfiguration = GetRequestMoveConfiguration();
            NotifySessionRectChange(lastRect, SizeChangeReason::UNDEFINED, moveConfiguration.displayId);
            SetRequestRectWhenFollowParent({0, 0, 0, 0});
            SetRequestMoveConfiguration({DISPLAY_ID_INVALID});
            cloneNodeCount_ = 0;
        }
    }
}

/** @note @window.drag */
WSError SceneSession::UpdateRectForDrag(const WSRect& rect)
{
    return PostSyncTask([weakThis = wptr(this), rect, where = __func__] {
        auto sceneSession = weakThis.promote();
        if (!sceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        sceneSession->GetLayoutController()->SetSessionRect(rect);
        sceneSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::DRAG_RECT);
        sceneSession->isDragging_ = true; // isDrag only reset by Vsync, not flushuiparam
        return WSError::WS_OK;
    }, __func__);
}

/** @note @window.drag */
void SceneSession::UpdateWinRectForSystemBar(WSRect& rect)
{
    if (!specificCallback_) {
        WLOGFE("specificCallback_ is null!");
        return;
    }
    auto sessionProperty = GetSessionProperty();
    float tmpPosY = 0.0;
    std::vector<sptr<SceneSession>> statusBarVector;
    if (specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_) {
        statusBarVector = specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_(
            WindowType::WINDOW_TYPE_STATUS_BAR, sessionProperty->GetDisplayId());
    }
    for (auto& statusBar : statusBarVector) {
        if (!(statusBar->isVisible_)) {
            continue;
        }
        WSRect statusBarRect = statusBar->GetSessionRect();
        if ((rect.posY_ < statusBarRect.posY_ + static_cast<int32_t>(statusBarRect.height_)) &&
            (rect.height_ != GetSessionRect().height_ || rect.width_ != GetSessionRect().width_)) {
            tmpPosY = rect.posY_ + rect.height_;
            rect.posY_ = statusBarRect.posY_ + statusBarRect.height_;
            rect.height_ = tmpPosY - rect.posY_;
        }
    }
    WLOGFD("after UpdateWinRectForSystemBar rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
}

void SceneSession::SetSurfaceBoundsWithAnimation(
    const std::pair<RSAnimationTimingProtocol, RSAnimationTimingCurve>& animationParam,
    const WSRect& rect, const std::function<void()>& finishCallback, bool isGlobal)
{
    RSNode::Animate(GetRSUIContext(), animationParam.first, animationParam.second,
        [weakThis = wptr(this), rect, isGlobal, where = __func__] {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s session is nullptr", where);
                return;
            }
            session->SetSurfaceBounds(rect, isGlobal, false);
            RSTransactionAdapter::FlushImplicitTransaction(session->GetRSUIContext());
        }, finishCallback);
}

void SceneSession::SetSurfaceBounds(const WSRect& rect, bool isGlobal, bool needFlush)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetSurfaceBounds id:%d [%d, %d, %d, %d] reason:%u",
        GetPersistentId(), rect.posX_, rect.posY_, rect.width_, rect.height_, GetSizeChangeReason());
    TLOGD(WmsLogTag::WMS_LAYOUT, "id: %{public}d, rect: %{public}s isGlobal: %{public}d needFlush: %{public}d",
        GetPersistentId(), rect.ToString().c_str(), isGlobal, needFlush);
    if (getRsCmdBlockingCountFunc_ != nullptr && getRsCmdBlockingCountFunc_() > 0) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "creating node, stop commit");
        return;
    }
    AutoRSTransaction trans(GetRSUIContext(), needFlush);
    auto surfaceNode = GetSurfaceNode();
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    NotifySubAndDialogFollowRectChange(rect, isGlobal, needFlush);
    SetSubWindowBoundsDuringCross(rect, isGlobal, needFlush);
    if (surfaceNode && leashWinSurfaceNode) {
        leashWinSurfaceNode->SetGlobalPositionEnabled(isGlobal);
        leashWinSurfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        leashWinSurfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode->SetBounds(0, 0, rect.width_, rect.height_);
        surfaceNode->SetFrame(0, 0, rect.width_, rect.height_);
    } else if (WindowHelper::IsPipWindow(GetWindowType()) && surfaceNode) {
        TLOGD(WmsLogTag::WMS_PIP, "PipWindow setSurfaceBounds");
        surfaceNode->SetGlobalPositionEnabled(isGlobal);
        surfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else if (WindowHelper::IsSubWindow(GetWindowType()) && surfaceNode) {
        WLOGFD("subwindow setSurfaceBounds");
        surfaceNode->SetGlobalPositionEnabled(isGlobal);
        surfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else if (WindowHelper::IsDialogWindow(GetWindowType()) && surfaceNode) {
        TLOGD(WmsLogTag::WMS_DIALOG, "dialogWindow setSurfaceBounds");
        surfaceNode->SetGlobalPositionEnabled(isGlobal);
        surfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else if (WindowHelper::IsSystemWindow(GetWindowType()) && surfaceNode) {
        TLOGD(WmsLogTag::WMS_SYSTEM, "system window setSurfaceBounds");
        surfaceNode->SetGlobalPositionEnabled(isGlobal);
        surfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else {
        WLOGE("SetSurfaceBounds surfaceNode is null!");
    }
}

void SceneSession::SetZOrder(uint32_t zOrder)
{
    PostTask([weakThis = wptr(this), zOrder, where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        if (session->zOrder_ != zOrder) {
            session->Session::SetZOrder(zOrder);
            if (session->specificCallback_ != nullptr) {
                session->specificCallback_->onWindowInfoUpdate_(session->GetPersistentId(),
                    WindowUpdateType::WINDOW_UPDATE_PROPERTY);
            }
        }
    }, __func__);
}

void SceneSession::SetFloatingScale(float floatingScale)
{
    if (floatingScale_ != floatingScale) {
        Session::SetFloatingScale(floatingScale);
        NotifySessionPropertyChange(WindowInfoKey::FLOATING_SCALE);
        if (specificCallback_ != nullptr) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
        }
        MarkAvoidAreaAsDirty();
    }
}

void SceneSession::SetParentPersistentId(int32_t parentId)
{
    auto property = GetSessionProperty();
    if (property) {
        property->SetParentPersistentId(parentId);
    }
}

int32_t SceneSession::GetParentPersistentId() const
{
    auto property = GetSessionProperty();
    if (property) {
        return property->GetParentPersistentId();
    }
    return INVALID_SESSION_ID;
}

int32_t SceneSession::GetMainSessionId()
{
    const auto& mainSession = GetMainSession();
    if (mainSession) {
        return mainSession->GetPersistentId();
    }
    return INVALID_SESSION_ID;
}

std::string SceneSession::GetWindowNameAllType() const
{
    if (GetSessionInfo().isSystem_) {
        return GetSessionInfo().abilityName_;
    } else {
        return GetWindowName();
    }
}

WSError SceneSession::SetTurnScreenOn(bool turnScreenOn)
{
    GetSessionProperty()->SetTurnScreenOn(turnScreenOn);
    return WSError::WS_OK;
}

bool SceneSession::IsTurnScreenOn() const
{
    return GetSessionProperty()->IsTurnScreenOn();
}

WSError SceneSession::SetKeepScreenOn(bool keepScreenOn)
{
    GetSessionProperty()->SetKeepScreenOn(keepScreenOn);
    return WSError::WS_OK;
}

bool SceneSession::IsKeepScreenOn() const
{
    return GetSessionProperty()->IsKeepScreenOn();
}

WSError SceneSession::SetViewKeepScreenOn(bool keepScreenOn)
{
    GetSessionProperty()->SetViewKeepScreenOn(keepScreenOn);
    return WSError::WS_OK;
}

bool SceneSession::IsViewKeepScreenOn() const
{
    return GetSessionProperty()->IsViewKeepScreenOn();
}

WSError SceneSession::SetWindowShadowEnabled(bool isEnabled)
{
    GetSessionProperty()->SetWindowShadowEnabled(isEnabled);
    PostTask([weakThis = wptr(this), isEnabled, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s isEnabled: %{public}u", where, isEnabled);
        if (session->onWindowShadowEnableChangeFunc_) {
            session->onWindowShadowEnableChangeFunc_(isEnabled);
        }
    }, __func__);
    return WSError::WS_OK;
}

bool SceneSession::GetWindowShadowEnabled() const
{
    return GetSessionProperty()->GetWindowShadowEnabled();
}

void SceneSession::SaveUpdatedIcon(const std::shared_ptr<Media::PixelMap>& icon)
{
    WLOGFI("run SaveUpdatedIcon");
    if (scenePersistence_ != nullptr) {
        scenePersistence_->SaveUpdatedIcon(icon);
    }
}

std::string SceneSession::GetUpdatedIconPath() const
{
    WLOGFI("run GetUpdatedIconPath");
    if (scenePersistence_ != nullptr) {
        return scenePersistence_->GetUpdatedIconPath();
    }
    return "";
}

void SceneSession::UpdateNativeVisibility(bool visible)
{
    PostTask([weakThis = wptr(this), visible, where = __func__]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        int32_t persistentId = session->GetPersistentId();
        TLOGNI(WmsLogTag::WMS_SCB, "%{public}s name: %{public}s, id: %{public}u, visible: %{public}u",
            where, session->sessionInfo_.bundleName_.c_str(), persistentId, visible);
        SceneSessionChangeInfo changeInfo {
            .persistentId_ = persistentId,
            .changeInfo_ = "Visibility change to " + std::to_string(visible),
            .logTag_ = WmsLogTag::WMS_ATTRIBUTE,
        };
        SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::VISIBLE_RECORD, changeInfo);
        bool oldVisibleState = session->isVisible_;
        session->isVisible_ = visible;
        if (session->visibilityChangedDetectFunc_) {
            session->visibilityChangedDetectFunc_(session->GetCallingPid(), oldVisibleState, visible);
        }
        if (session->specificCallback_ == nullptr) {
            TLOGNW(WmsLogTag::WMS_SCB, "%{public}s specific callback is null.", where);
            return;
        }
        if (visible) {
            session->specificCallback_->onWindowInfoUpdate_(persistentId, WindowUpdateType::WINDOW_UPDATE_ADDED);
        } else {
            session->specificCallback_->onWindowInfoUpdate_(persistentId, WindowUpdateType::WINDOW_UPDATE_REMOVED);
        }
        session->NotifyAccessibilityVisibilityChange();
        session->specificCallback_->onUpdateAvoidArea_(persistentId);
        // update private state
        if (!session->GetSessionProperty()) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s property is null", where);
            return;
        }
        if (session->updatePrivateStateAndNotifyFunc_ != nullptr) {
            session->updatePrivateStateAndNotifyFunc_(persistentId);
        }
    }, __func__);
}

void SceneSession::UpdateRotationAvoidArea()
{
    if (specificCallback_) {
        if (Session::IsScbCoreEnabled()) {
            MarkAvoidAreaAsDirty();
        } else {
            specificCallback_->onUpdateAvoidArea_(GetPersistentId());
        }
    }
}

void SceneSession::SetPrivacyMode(bool isPrivacy)
{
    auto property = GetSessionProperty();
    if (!property) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "property is null");
        return;
    }
    auto surfaceNode = GetShadowSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "surfaceNode_ is null");
        return;
    }
    bool lastPrivacyMode = property->GetPrivacyMode() || property->GetSystemPrivacyMode();
    if (lastPrivacyMode == isPrivacy) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "mode is same as: win=[%{public}d, %{public}s], isPrivacy=%{public}d",
            GetWindowId(), GetWindowName().c_str(), isPrivacy);
        return;
    }
    property->SetPrivacyMode(isPrivacy);
    property->SetSystemPrivacyMode(isPrivacy);
    {
        AutoRSTransaction trans(GetRSShadowContext());
        surfaceNode->SetSecurityLayer(isPrivacy);
        auto leashWinSurfaceNode = GetLeashWinShadowSurfaceNode();
        if (leashWinSurfaceNode != nullptr) {
            AutoRSTransaction leashTrans(GetRSLeashWinShadowContext());
            leashWinSurfaceNode->SetSecurityLayer(isPrivacy);
        }
    }
    NotifyPrivacyModeChange();
    auto mainSession = GetMainSession();
    if (mainSession) {
        ControlInfo controlInfo = { .isNeedControl = isPrivacy, .isControlRecentOnly = true };
        mainSession->NotifyUpdateAppUseControl(ControlAppType::PRIVACY_WINDOW, controlInfo);
    }
}

void SceneSession::NotifyPrivacyModeChange()
{
    bool isPrivacyMode = GetSessionProperty()->GetPrivacyMode();
    bool currExtPrivacyMode = combinedExtWindowFlags_.privacyModeFlag;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "win:[%{public}d, %{public}s], currExtPrivacyMode:%{public}d, "
        "session property privacyMode:%{public}d, last privacyMode:%{public}d",
        GetPersistentId(), GetWindowName().c_str(), currExtPrivacyMode, isPrivacyMode, isPrivacyMode_);
    bool mixedPrivacyMode = currExtPrivacyMode || isPrivacyMode;
    if (mixedPrivacyMode != isPrivacyMode_) {
        isPrivacyMode_ = mixedPrivacyMode;
        if (privacyModeChangeNotifyFunc_) {
            privacyModeChangeNotifyFunc_(isPrivacyMode_);
        }
    }
}

void SceneSession::NotifyExtensionSecureLimitChange(bool isLimit)
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "sessionStage is nullptr");
        return;
    }
    sessionStage_->NotifyExtensionSecureLimitChange(isLimit);
}

WMError SceneSession::SetSnapshotSkip(bool isSkip)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d, permission denied!", GetPersistentId());
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto property = GetSessionProperty();
    if (!property) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "property is null");
        return WMError::WM_ERROR_DESTROYED_OBJECT;
    }
    auto surfaceNode = GetShadowSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "surfaceNode_ is null");
        return WMError::WM_ERROR_DESTROYED_OBJECT;
    }
    bool lastSnapshotSkip = property->GetSnapshotSkip();
    if (lastSnapshotSkip == isSkip) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Snapshot skip does not change, do nothing, isSkip: %{public}d, "
            "id: %{public}d", isSkip, GetPersistentId());
        return WMError::WM_OK;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, isSkip: %{public}d", GetWindowId(), isSkip);
    property->SetSnapshotSkip(isSkip);
    {
        AutoRSTransaction trans(GetRSShadowContext());
        surfaceNode->SetSkipLayer(isSkip);
        auto leashWinSurfaceNode = GetLeashWinShadowSurfaceNode();
        if (leashWinSurfaceNode != nullptr) {
            AutoRSTransaction leashTrans(GetRSLeashWinShadowContext());
            leashWinSurfaceNode->SetSkipLayer(isSkip);
        }
    }
    return WMError::WM_OK;
}

void SceneSession::SetWatermarkEnabled(const std::string& watermarkName, bool isEnabled)
{
    auto surfaceNode = GetShadowSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::DEFAULT, "surfaceNode is null");
        return;
    }
    TLOGI(WmsLogTag::DEFAULT, "watermarkName:%{public}s, isEnabled:%{public}d, wid:%{public}d",
        watermarkName.c_str(), isEnabled, GetPersistentId());
    {
        AutoRSTransaction trans(GetRSShadowContext());
        surfaceNode->SetWatermarkEnabled(watermarkName, isEnabled);
        if (auto leashWinSurfaceNode = GetLeashWinShadowSurfaceNode()) {
            AutoRSTransaction leashTrans(GetRSLeashWinShadowContext());
            leashWinSurfaceNode->SetWatermarkEnabled(watermarkName, isEnabled);
        }
    }
}

void SceneSession::SetPiPTemplateInfo(const PiPTemplateInfo& pipTemplateInfo)
{
    pipTemplateInfo_ = pipTemplateInfo;
}

void SceneSession::SetSystemSceneOcclusionAlpha(double alpha)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetAbilityBGAlpha");
    if (alpha < 0 || alpha > 1.0) {
        WLOGFE("OnSetSystemSceneOcclusionAlpha property is null");
        return;
    }
    auto surfaceNode = GetSurfaceNode();
    if (!surfaceNode) {
        WLOGFE("surfaceNode_ is null");
        return;
    }
    uint8_t alpha8bit = static_cast<uint8_t>(alpha * 255);
    WLOGFI("SetAbilityBGAlpha alpha8bit=%{public}u.", alpha8bit);
    {
        AutoRSTransaction trans(GetRSUIContext());
        surfaceNode->SetAbilityBGAlpha(alpha8bit);
    }
}

void SceneSession::ResetOcclusionAlpha()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::ResetAbilityBGAlpha");
    auto surfaceNode = GetSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "surfaceNode_ is null");
        return;
    }
    uint8_t alpha8bit = GetSessionProperty()->GetBackgroundAlpha();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "alpha8bit=%{public}u, windowId=%{public}d", alpha8bit, GetPersistentId());
    {
        AutoRSTransaction trans(GetRSUIContext());
        surfaceNode->SetAbilityBGAlpha(alpha8bit);
    }
}

void SceneSession::SetSystemSceneForceUIFirst(bool forceUIFirst)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetForceUIFirst");
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    auto surfaceNode = GetSurfaceNode();
    if (leashWinSurfaceNode == nullptr && surfaceNode == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "leashWindow and surfaceNode are nullptr");
        return;
    }
    AutoRSTransaction trans(GetRSUIContext());
    if (leashWinSurfaceNode != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " forceUIFirst=%{public}d.",
            leashWinSurfaceNode->GetName().c_str(), leashWinSurfaceNode->GetId(), forceUIFirst);
        leashWinSurfaceNode->SetForceUIFirst(forceUIFirst);
    } else if (surfaceNode != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " forceUIFirst=%{public}d.",
            surfaceNode->GetName().c_str(), surfaceNode->GetId(), forceUIFirst);
        surfaceNode->SetForceUIFirst(forceUIFirst);
    }
}

void SceneSession::SetUIFirstSwitch(RSUIFirstSwitch uiFirstSwitch)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetUIFirstSwitch");
    AutoRSTransaction trans(GetRSUIContext());
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    auto surfaceNode = GetSurfaceNode();
    if (leashWinSurfaceNode != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " uiFirstSwitch=%{public}d",
            leashWinSurfaceNode->GetName().c_str(), leashWinSurfaceNode->GetId(), uiFirstSwitch);
        leashWinSurfaceNode->SetUIFirstSwitch(uiFirstSwitch);
    } else if (surfaceNode != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " uiFirstSwitch=%{public}d",
            surfaceNode->GetName().c_str(), surfaceNode->GetId(), uiFirstSwitch);
        surfaceNode->SetUIFirstSwitch(uiFirstSwitch);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "leashWindow and surfaceNode are nullptr");
    }
}

void SceneSession::CloneWindow(NodeId surfaceNodeId, bool needOffScreen)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::CloneWindow");
    AutoRSTransaction trans(GetRSUIContext());
    if (auto surfaceNode = GetSurfaceNode()) {
        TLOGI(WmsLogTag::WMS_PC, "%{public}s this: %{public}" PRIu64 " cloned: %{public}" PRIu64,
            surfaceNode->GetName().c_str(), surfaceNode->GetId(), surfaceNodeId);
        surfaceNode->SetClonedNodeInfo(surfaceNodeId, needOffScreen);
    }
}

void SceneSession::MarkSystemSceneUIFirst(bool isForced, bool isUIFirstEnabled)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::MarkSystemSceneUIFirst");
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    auto surfaceNode = GetSurfaceNode();
    if (leashWinSurfaceNode == nullptr && surfaceNode == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "leashWindow and surfaceNode are nullptr");
        return;
    }
    if (leashWinSurfaceNode != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " isForced=%{public}d. isUIFirstEnabled=%{public}d",
            leashWinSurfaceNode->GetName().c_str(), leashWinSurfaceNode->GetId(), isForced, isUIFirstEnabled);
        leashWinSurfaceNode->MarkUifirstNode(isForced, isUIFirstEnabled);
    } else {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " isForced=%{public}d. isUIFirstEnabled=%{public}d",
            surfaceNode->GetName().c_str(), surfaceNode->GetId(), isForced, isUIFirstEnabled);
        surfaceNode->MarkUifirstNode(isForced, isUIFirstEnabled);
    }
}

WSError SceneSession::UpdateWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    return PostSyncTask([weakThis = wptr(this), needDefaultAnimationFlag, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->needDefaultAnimationFlag_ = needDefaultAnimationFlag;
        if (session->onWindowAnimationFlagChange_) {
            session->onWindowAnimationFlagChange_(needDefaultAnimationFlag);
        }
        return WSError::WS_OK;
    }, __func__);
}

void SceneSession::SetWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    needDefaultAnimationFlag_ = needDefaultAnimationFlag;
    if (onWindowAnimationFlagChange_) {
        onWindowAnimationFlagChange_(needDefaultAnimationFlag);
    }
    return;
}

bool SceneSession::IsNeedDefaultAnimation() const
{
    return needDefaultAnimationFlag_;
}

bool SceneSession::GetFollowScreenChange() const
{
    return isFollowScreenChange_;
}

void SceneSession::SetFollowScreenChange(bool isFollowScreenChange)
{
    isFollowScreenChange_ = isFollowScreenChange;
}

bool SceneSession::IsAppSession() const
{
    if (GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        return true;
    }
    if (GetMainSession()) {
        return true;
    }
    return false;
}

/** @note @window.focus */
bool SceneSession::IsAppOrLowerSystemSession() const
{
    WindowType windowType = GetWindowType();
    if (windowType == WindowType::WINDOW_TYPE_NEGATIVE_SCREEN ||
        windowType == WindowType::WINDOW_TYPE_GLOBAL_SEARCH ||
        windowType == WindowType::WINDOW_TYPE_DESKTOP) {
        return true;
    }
    return IsAppSession();
}

/** @note @window.focus */
bool SceneSession::IsSystemSessionAboveApp() const
{
    WindowType windowType = GetWindowType();
    if (windowType == WindowType::WINDOW_TYPE_DIALOG || windowType == WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW) {
        return true;
    }
    if (windowType == WindowType::WINDOW_TYPE_PANEL &&
        sessionInfo_.bundleName_.find("SCBDropdownPanel") != std::string::npos) {
        return true;
    }
    if (windowType == WindowType::WINDOW_TYPE_FLOAT &&
        sessionInfo_.bundleName_.find("SCBGestureDock") != std::string::npos) {
        return true;
    }
    return false;
}

/** @note @window.focus */
bool SceneSession::IsSameMainSession(const sptr<SceneSession>& prevSession)
{
    if (prevSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "prevSession is nullptr");
        return false;
    }
    int32_t currSessionId = GetMainSessionId();
    int32_t prevSessionId = prevSession->GetMainSessionId();
    return currSessionId == prevSessionId && prevSessionId != INVALID_SESSION_ID;
}

/* @note @window.focus */
bool SceneSession::IsDelayFocusChange()
{
    return GetHidingStartingWindow();
}

void SceneSession::NotifyIsCustomAnimationPlaying(bool isPlaying)
{
    WLOGFI("id %{public}d %{public}u", GetPersistentId(), isPlaying);
    if (onIsCustomAnimationPlaying_) {
        onIsCustomAnimationPlaying_(isPlaying);
    }
}

WSError SceneSession::UpdateWindowSceneAfterCustomAnimation(bool isAdd)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to update with id:%{public}u!", GetPersistentId());
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    PostTask([weakThis = wptr(this), isAdd, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s id %{public}d, isAdd: %{public}d",
            where, session->GetPersistentId(), isAdd);
        if (isAdd) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s SetOpacityFunc not register %{public}d",
                where, session->GetPersistentId());
            return WSError::WS_ERROR_INVALID_OPERATION;
        } else {
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s background after custom animation id %{public}d",
                where, session->GetPersistentId());
            // since background will remove surfaceNode
            session->Background();
            session->NotifyIsCustomAnimationPlaying(false);
        }
        return WSError::WS_OK;
    }, std::string(__func__) + ":" + std::to_string(isAdd));
    return WSError::WS_OK;
}

bool SceneSession::IsFloatingWindowAppType() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        return false;
    }
    return property->IsFloatingWindowAppType();
}

std::vector<Rect> SceneSession::GetTouchHotAreas() const
{
    std::vector<Rect> touchHotAreas;
    auto property = GetSessionProperty();
    if (property) {
        property->GetTouchHotAreas(touchHotAreas);
    }
    return touchHotAreas;
}

PiPTemplateInfo SceneSession::GetPiPTemplateInfo() const
{
    return pipTemplateInfo_;
}

void SceneSession::DumpSessionElementInfo(const std::vector<std::string>& params)
{
    if (!sessionStage_) {
        return;
    }
    return sessionStage_->DumpSessionElementInfo(params);
}

void SceneSession::NotifyTouchOutside()
{
    WLOGFI("id: %{public}d, type: %{public}d", GetPersistentId(), GetWindowType());
    if (sessionStage_) {
        WLOGFD("Notify sessionStage TouchOutside");
        sessionStage_->NotifyTouchOutside();
    }
    if (onTouchOutside_) {
        WLOGFD("Notify sessionChangeCallback TouchOutside");
        onTouchOutside_();
    }
}

void SceneSession::NotifyWindowVisibility()
{
    if (sessionStage_) {
        sessionStage_->NotifyWindowVisibility(GetRSVisible());
    } else {
        WLOGFE("Notify window(id:%{public}d) visibility failed, for this session stage is nullptr", GetPersistentId());
    }
}

void SceneSession::NotifyOcclusionState()
{
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "no stage, win=[%{public}d, %{public}s]", GetWindowId(),
            GetWindowName().c_str());
        return;
    }
    auto visibilityState = GetVisibilityState();
    auto retCode = sessionStage_->NotifyWindowOcclusionState(visibilityState);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "win=[%{public}d, %{public}s], visibilityState=%{public}u, retCode=%{public}d",
        GetWindowId(), GetWindowName().c_str(), static_cast<uint32_t>(visibilityState), static_cast<int32_t>(retCode));
}

bool SceneSession::CheckTouchOutsideCallbackRegistered()
{
    return onTouchOutside_ != nullptr;
}

void SceneSession::SetRequestedOrientation(Orientation orientation, bool needAnimation)
{
    WLOGFI("id: %{public}d orientation: %{public}u", GetPersistentId(), static_cast<uint32_t>(orientation));
    GetSessionProperty()->SetRequestedOrientation(orientation, needAnimation);
    if (onRequestedOrientationChange_) {
        onRequestedOrientationChange_(static_cast<uint32_t>(orientation), needAnimation);
    }
}

WSError SceneSession::SetDefaultRequestedOrientation(Orientation orientation)
{
    return PostSyncTask([weakThis = wptr(this), orientation, where = __func__]() -> WSError {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::DEFAULT, "%{public}s session is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        TLOGNI(WmsLogTag::DEFAULT, "%{public}s id: %{public}d defaultRequestedOrientation: %{public}u",
            where, session->GetPersistentId(), static_cast<uint32_t>(orientation));
        auto property = session->GetSessionProperty();
        property->SetRequestedOrientation(orientation);
        property->SetDefaultRequestedOrientation(orientation);
        property->SetUserRequestedOrientation(orientation);
        return WSError::WS_OK;
    }, __func__);
}

void SceneSession::NotifyForceHideChange(bool hide)
{
    WLOGFI("id: %{public}d forceHide: %{public}u", persistentId_, hide);
    auto property = GetSessionProperty();
    if (property == nullptr) {
        WLOGFD("id: %{public}d property is nullptr", persistentId_);
        return;
    }
    property->SetForceHide(hide);
    if (onForceHideChangeFunc_) {
        onForceHideChangeFunc_(hide);
    }
    SetForceTouchable(!hide);
    if (hide) {
        if (isFocused_) {
            FocusChangeReason reason = FocusChangeReason::DEFAULT;
            NotifyRequestFocusStatusNotifyManager(false, true, reason);
            SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
        } else if (forceHideState_ == ForceHideState::NOT_HIDDEN) {
            SetForceHideState(ForceHideState::HIDDEN_WHEN_UNFOCUSED);
        }
    } else {
        if (forceHideState_ == ForceHideState::HIDDEN_WHEN_FOCUSED) {
            SetForceHideState(ForceHideState::NOT_HIDDEN);
            FocusChangeReason reason = FocusChangeReason::DEFAULT;
            NotifyRequestFocusStatusNotifyManager(true, true, reason);
        } else {
            SetForceHideState(ForceHideState::NOT_HIDDEN);
        }
    }
}

Orientation SceneSession::GetRequestedOrientation() const
{
    return GetSessionProperty()->GetRequestedOrientation();
}

bool SceneSession::IsAnco() const
{
    return collaboratorType_ == static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
}

void SceneSession::SetBlank(bool isAddBlank)
{
    isAddBlank_ = isAddBlank;
}

bool SceneSession::GetBlank() const
{
    return isAddBlank_;
}

void SceneSession::SetBufferAvailableCallbackEnable(bool enable)
{
    bufferAvailableCallbackEnable_ = enable;
}

bool SceneSession::GetBufferAvailableCallbackEnable() const
{
    return bufferAvailableCallbackEnable_;
}

int32_t SceneSession::GetCollaboratorType() const
{
    return collaboratorType_;
}

void SceneSession::SetCollaboratorType(int32_t collaboratorType)
{
    collaboratorType_ = collaboratorType;
    sessionInfo_.collaboratorType_ = collaboratorType;
}

std::string SceneSession::GetClientIdentityToken() const
{
    return clientIdentityToken_;
}

void SceneSession::SetClientIdentityToken(const std::string& clientIdentityToken)
{
    clientIdentityToken_ = clientIdentityToken;
}

void SceneSession::DumpSessionInfo(std::vector<std::string>& info) const
{
    std::string dumpInfo = "      Session ID #" + std::to_string(persistentId_);
    info.push_back(dumpInfo);
    dumpInfo = "        session name [" + SessionUtils::ConvertSessionName(sessionInfo_.bundleName_,
        sessionInfo_.abilityName_, sessionInfo_.moduleName_, sessionInfo_.appIndex_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        runningState [" + std::string(isActive_ ? "FOREGROUND" : "BACKGROUND") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        lockedState [" + std::to_string(sessionInfo_.lockedState) + "]";
    info.push_back(dumpInfo);
    auto abilityInfo = sessionInfo_.abilityInfo;
    dumpInfo = "        continuable [" + (abilityInfo ? std::to_string(abilityInfo->continuable) : " ") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        timeStamp [" + sessionInfo_.time + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        label [" + (abilityInfo ? abilityInfo->label : " ") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        iconPath [" + (abilityInfo ? abilityInfo->iconPath : " ") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        want [" + (sessionInfo_.want ? sessionInfo_.want->ToUri() : " ") + "]";
    info.push_back(dumpInfo);
}

std::shared_ptr<AppExecFwk::AbilityInfo> SceneSession::GetAbilityInfo() const
{
    return GetSessionInfo().abilityInfo;
}

void SceneSession::SetAbilitySessionInfo(std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo)
{
    SetSessionInfoAbilityInfo(abilityInfo);
}

void SceneSession::SetSessionState(SessionState state)
{
    Session::SetSessionState(state);
    NotifyAccessibilityVisibilityChange();
}

void SceneSession::UpdateSessionState(SessionState state)
{
    Session::UpdateSessionState(state);
    NotifyAccessibilityVisibilityChange();
}

bool SceneSession::IsVisibleForAccessibility() const
{
    if (Session::IsScbCoreEnabled()) {
        return GetSystemTouchable() && GetForegroundInteractiveStatus() && IsVisibleForeground();
    }
    return GetSystemTouchable() && GetForegroundInteractiveStatus() &&
        (IsVisible() || state_ == SessionState::STATE_ACTIVE || state_ == SessionState::STATE_FOREGROUND);
}

void SceneSession::SetForegroundInteractiveStatus(bool interactive)
{
    Session::SetForegroundInteractiveStatus(interactive);
    NotifyAccessibilityVisibilityChange();
    if (interactive) {
        return;
    }
    for (auto toastSession : toastSession_) {
        if (toastSession == nullptr) {
            TLOGD(WmsLogTag::WMS_TOAST, "toastSession session is nullptr");
            continue;
        }
        auto state = toastSession->GetSessionState();
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            continue;
        }
        toastSession->SetActive(false);
        toastSession->BackgroundTask();
    }
}

void SceneSession::NotifyAccessibilityVisibilityChange()
{
    bool isVisibleForAccessibilityNew = IsVisibleForAccessibility();
    if (isVisibleForAccessibilityNew == isVisibleForAccessibility_.load()) {
        return;
    }
    WLOGFD("[WMSAccess] NotifyAccessibilityVisibilityChange id: %{public}d, access: %{public}d ",
        GetPersistentId(), isVisibleForAccessibilityNew);
    isVisibleForAccessibility_.store(isVisibleForAccessibilityNew);
    if (specificCallback_ && specificCallback_->onWindowInfoUpdate_) {
        if (isVisibleForAccessibilityNew) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_ADDED);
        } else {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
        }
    } else {
        WLOGFD("specificCallback_->onWindowInfoUpdate_ not exist, persistent id: %{public}d", GetPersistentId());
    }
}

void SceneSession::SetSystemTouchable(bool touchable)
{
    Session::SetSystemTouchable(touchable);
    NotifyAccessibilityVisibilityChange();
}

WSError SceneSession::ChangeSessionVisibilityWithStatusBar(
    const sptr<AAFwk::SessionInfo> abilitySessionInfo, bool visible)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    PostTask([weakThis = wptr(this), abilitySessionInfo, visible, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s abilitySessionInfo is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s id:%{public}d is terminating", where, session->GetPersistentId());
            return WSError::WS_ERROR_INVALID_OPERATION;
        }

        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.moduleName_ = abilitySessionInfo->want.GetModuleName();
        int32_t appCloneIndex = abilitySessionInfo->want.GetIntParam(APP_CLONE_INDEX, 0);
        info.appIndex_ = appCloneIndex == 0 ? abilitySessionInfo->want.GetIntParam(DLP_INDEX, 0) : appCloneIndex;
        info.persistentId_ = abilitySessionInfo->persistentId;
        info.callerPersistentId_ = session->GetPersistentId();
        info.callerBundleName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        info.callerAbilityName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_ABILITY_NAME);
        info.callState_ = static_cast<uint32_t>(abilitySessionInfo->state);
        info.uiAbilityId_ = abilitySessionInfo->uiAbilityId;
        info.requestId = abilitySessionInfo->requestId;
        info.want = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
        info.requestCode = abilitySessionInfo->requestCode;
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.startSetting = abilitySessionInfo->startSetting;
        info.callingTokenId_ = abilitySessionInfo->callingTokenId;
        info.reuse = abilitySessionInfo->reuse;
        info.processOptions = abilitySessionInfo->processOptions;

        if (session->changeSessionVisibilityWithStatusBarFunc_) {
            session->changeSessionVisibilityWithStatusBarFunc_(info, visible);
        }

        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

static void SetAtomicServiceInfo(SessionInfo& sessionInfo)
{
#ifdef ACE_ENGINE_PLUGIN_PATH
    AtomicServiceInfo* atomicServiceInfo = AtomicServiceBasicEnginePlugin::GetInstance().
        GetParamsFromAtomicServiceBasicEngine(sessionInfo.bundleName_);
    if (atomicServiceInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Get atomicService info failed, bundleName:%{public}s.",
            sessionInfo.bundleName_.c_str());
        AtomicServiceBasicEnginePlugin::GetInstance().ReleaseData();
        return;
    }
    sessionInfo.atomicServiceInfo_.appNameInfo_ = atomicServiceInfo->GetAppName();
    sessionInfo.atomicServiceInfo_.circleIcon_ = atomicServiceInfo->GetCircleIcon();
    sessionInfo.atomicServiceInfo_.eyelashRingIcon_ = atomicServiceInfo->GetEyelashRingIcon();
    sessionInfo.atomicServiceInfo_.deviceTypes_ = atomicServiceInfo->GetDeviceTypes();
    sessionInfo.atomicServiceInfo_.resizable_ = atomicServiceInfo->GetResizable();
    sessionInfo.atomicServiceInfo_.supportWindowMode_ = atomicServiceInfo->GetSupportWindowMode();
    TLOGI(WmsLogTag::WMS_LIFE, "Get atomicService info success, bundleName:%{public}s.",
        sessionInfo.bundleName_.c_str());
    AtomicServiceBasicEnginePlugin::GetInstance().ReleaseData();
#endif
}

static SessionInfo MakeSessionInfoDuringPendingActivation(const sptr<AAFwk::SessionInfo>& abilitySessionInfo,
    const sptr<SceneSession>& session, bool isFoundationCall)
{
    SessionInfo info;
    info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
    info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
    info.moduleName_ = abilitySessionInfo->want.GetModuleName();
    int32_t appCloneIndex = abilitySessionInfo->want.GetIntParam(APP_CLONE_INDEX, 0);
    info.appIndex_ = appCloneIndex == 0 ? abilitySessionInfo->want.GetIntParam(DLP_INDEX, 0) : appCloneIndex;
    info.persistentId_ = abilitySessionInfo->persistentId;
    info.specifiedReason_ = static_cast<SpecifiedReason>(abilitySessionInfo->specifiedReason);
    info.callerPersistentId_ = session->GetPersistentId();
    info.callerBundleName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    info.callerAbilityName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_ABILITY_NAME);
    info.callState_ = static_cast<uint32_t>(abilitySessionInfo->state);
    info.uiAbilityId_ = abilitySessionInfo->uiAbilityId;
    info.requestId = abilitySessionInfo->requestId;
    info.want = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
    info.requestCode = abilitySessionInfo->requestCode;
    info.callerToken_ = abilitySessionInfo->callerToken;
    info.startSetting = abilitySessionInfo->startSetting;
    info.callingTokenId_ = abilitySessionInfo->callingTokenId;
    info.reuse = abilitySessionInfo->reuse;
    info.processOptions = abilitySessionInfo->processOptions;
    info.isAtomicService_ = abilitySessionInfo->isAtomicService;
    info.isBackTransition_ = abilitySessionInfo->isBackTransition;
    info.needClearInNotShowRecent_ = abilitySessionInfo->needClearInNotShowRecent;
    info.appInstanceKey_ = abilitySessionInfo->instanceKey;
    info.isPcOrPadEnableActivation_ = session->IsPcOrPadEnableActivation();
    info.canStartAbilityFromBackground_ = abilitySessionInfo->canStartAbilityFromBackground;
    info.isFoundationCall_ = isFoundationCall;
    info.specifiedFlag_ = abilitySessionInfo->specifiedFlag;
    info.reuseDelegatorWindow = abilitySessionInfo->reuseDelegatorWindow;
    if (session->IsPcOrPadEnableActivation()) {
        info.startWindowOption = abilitySessionInfo->startWindowOption;
        int32_t maxWindowWidth = abilitySessionInfo->want.GetIntParam(AAFwk::Want::PARAM_RESV_MAX_WINDOW_WIDTH, 0);
        info.windowSizeLimits.maxWindowWidth = static_cast<std::uint32_t>(maxWindowWidth > 0 ? maxWindowWidth : 0);
        int32_t minWindowWidth = abilitySessionInfo->want.GetIntParam(AAFwk::Want::PARAM_RESV_MIN_WINDOW_WIDTH, 0);
        info.windowSizeLimits.minWindowWidth = static_cast<std::uint32_t>(minWindowWidth > 0 ? minWindowWidth : 0);
        int32_t maxWindowHeight = abilitySessionInfo->want.GetIntParam(AAFwk::Want::PARAM_RESV_MAX_WINDOW_HEIGHT, 0);
        info.windowSizeLimits.maxWindowHeight = static_cast<std::uint32_t>(maxWindowHeight > 0 ? maxWindowHeight : 0);
        int32_t minWindowHeight = abilitySessionInfo->want.GetIntParam(AAFwk::Want::PARAM_RESV_MIN_WINDOW_HEIGHT, 0);
        info.windowSizeLimits.minWindowHeight = static_cast<std::uint32_t>(minWindowHeight > 0 ? minWindowHeight : 0);
        if (!abilitySessionInfo->supportWindowModes.empty()) {
            info.supportedWindowModes.assign(abilitySessionInfo->supportWindowModes.begin(),
                abilitySessionInfo->supportWindowModes.end());
        }
    }
    if (info.isAtomicService_) {
        SetAtomicServiceInfo(info);
    }
    if (info.want != nullptr) {
        info.windowMode = info.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, 0);
        info.sessionAffinity = info.want->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY);
        info.screenId_ = static_cast<uint64_t>(info.want->GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, -1));
        if (info.isBackTransition_) {
            info.screenId_ = session->GetSessionInfo().screenId_;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "want: screenId %{public}" PRIu64, info.screenId_);
    }
    if (info.windowMode == static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN)) {
        info.fullScreenStart_ = true;
    }
    info.scenarios = abilitySessionInfo->scenarios;
    session->CalculatedStartWindowType(info, abilitySessionInfo->hideStartWindow);
    if (abilitySessionInfo->windowCreateParams) {
        if (abilitySessionInfo->windowCreateParams->animationSystemParams &&
            SessionPermission::IsSystemAppCallByCallingTokenID(info.callingTokenId_)) {
            info.startAnimationSystemOptions = abilitySessionInfo->windowCreateParams->animationSystemParams;
        }
        info.startAnimationOptions = abilitySessionInfo->windowCreateParams->animationParams;
    }
    info.isPrelaunch_ = abilitySessionInfo->isPrelaunch;
    info.frameNum_ = abilitySessionInfo->frameNum;
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName:%{public}s, moduleName:%{public}s, abilityName:%{public}s,"
        "appIndex:%{public}d, affinity:%{public}s. callState:%{public}d, want persistentId:%{public}d,"
        "uiAbilityId:%{public}" PRIu64 ", windowMode:%{public}d, callerId:%{public}d,"
        "needClearInNotShowRecent:%{public}u, appInstanceKey: %{public}s,"
        "supportedWindowModes.size:%{public}zu, requestId:%{public}d,"
        "maxWindowWidth:%{public}d, minWindowWidth:%{public}d, maxWindowHeight:%{public}d, minWindowHeight:%{public}d,"
        "reuseDelegatorWindow:%{public}d, startWindowType:%{public}d, isPrelaunch:%{public}d, frameNum:%{public}d",
        info.bundleName_.c_str(), info.moduleName_.c_str(), info.abilityName_.c_str(), info.appIndex_,
        info.sessionAffinity.c_str(), info.callState_, info.persistentId_, info.uiAbilityId_, info.windowMode,
        info.callerPersistentId_, info.needClearInNotShowRecent_, info.appInstanceKey_.c_str(),
        info.supportedWindowModes.size(), info.requestId,
        info.windowSizeLimits.maxWindowWidth, info.windowSizeLimits.minWindowWidth,
        info.windowSizeLimits.maxWindowHeight, info.windowSizeLimits.minWindowHeight,
        info.reuseDelegatorWindow, info.startWindowType_, info.isPrelaunch_, info.frameNum_);
    return info;
}

void SceneSession::CalculatedStartWindowType(SessionInfo& info, bool hideStartWindow)
{
    if (getStartWindowConfigFunc_ == nullptr || GetSessionInfo().bundleName_ != info.bundleName_) {
        TLOGI(WmsLogTag::WMS_LIFE, "only same app in pc or pcMode.");
        return;
    }
    std::string startWindowType;
    getStartWindowConfigFunc_(info, startWindowType);
    if (startWindowType == OPTIONAL_SHOW) {
        info.startWindowType_ = StartWindowType::DEFAULT;
        if (hideStartWindow) {
            info.startWindowType_ = StartWindowType::RETAIN_AND_INVISIBLE;
            SetHidingStartingWindow(true);
        }
        info.isSetStartWindowType_ = true;
    }
}

void SceneSession::RegisterGetStartWindowConfigFunc(GetStartWindowTypeFunc&& func)
{
    getStartWindowConfigFunc_ = std::move(func);
}

WSError SceneSession::PendingSessionActivation(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    bool isFoundationCall = SessionPermission::IsFoundationCall();
    PostTask([weakThis = wptr(this), abilitySessionInfo, isFoundationCall, where = __func__] {
        OHOS::HiviewDFX::HiTraceId hiTraceId = OHOS::HiviewDFX::HiTraceChain::GetId();
        bool isValid = hiTraceId.IsValid();
        if (!isValid) {
            hiTraceId = OHOS::HiviewDFX::HiTraceChain::Begin("WindowCreateOnCall",
                HiTraceFlag::HITRACE_FLAG_INCLUDE_ASYNC | HiTraceFlag::HITRACE_FLAG_NO_BE_INFO |
                HiTraceFlag::HITRACE_FLAG_DONOT_CREATE_SPAN);
        }
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            if (!isValid) {
                OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
            }
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s abilitySessionInfo is null", where);
            if (!isValid) {
                OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
            }
            return WSError::WS_ERROR_NULLPTR;
        }
        bool isFromAncoAndToAnco = session->IsAnco() && AbilityInfoManager::GetInstance().IsAnco(
            abilitySessionInfo->want.GetElement().GetBundleName(),
            abilitySessionInfo->want.GetElement().GetAbilityName(), abilitySessionInfo->want.GetModuleName());
        if (session->DisallowActivationFromPendingBackground(session->IsPcOrPadEnableActivation(), isFoundationCall,
            abilitySessionInfo->canStartAbilityFromBackground, isFromAncoAndToAnco)) {
            if (!isValid) {
                OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
            }
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        session->sessionInfo_.startMethod = StartMethod::START_CALL;
        SessionInfo info = MakeSessionInfoDuringPendingActivation(abilitySessionInfo, session, isFoundationCall);
        if (MultiInstanceManager::IsSupportMultiInstance(session->systemConfig_) &&
            MultiInstanceManager::GetInstance().IsMultiInstance(info.bundleName_)) {
            if (!MultiInstanceManager::GetInstance().MultiInstancePendingSessionActivation(info)) {
                TLOGNE(WmsLogTag::WMS_LIFE,
                    "%{public}s multi instance start fail, id:%{public}d instanceKey:%{public}s",
                    where, session->GetPersistentId(), info.appInstanceKey_.c_str());
                if (!isValid) {
                    OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
                }
                return WSError::WS_ERROR_INVALID_PARAM;
            }
        }
        session->HandleCastScreenConnection(info, session);
        if (info.reuseDelegatorWindow) {
            if (!session->hookSceneSessionActivationFunc_) {
                TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s hookSceneSessionActivationFunc is null, id: %{public}d",
                    where, session->persistentId_);
                if (!isValid) {
                    OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
                }
                return WSError::WS_ERROR_NULLPTR;
            }
            session->hookSceneSessionActivationFunc_(session, false);
            if (!isValid) {
                OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
            }
            return WSError::WS_OK;
        }
        if (session->pendingSessionActivationFunc_) {
            session->pendingSessionActivationFunc_(info);
        }
        if (!isValid) {
            OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::DoBatchPendingSessionsActivation(
    const std::vector<sptr<AAFwk::SessionInfo>>& abilitySessionInfos,
    sptr<SceneSession>& session, bool isFoundationCall, const std::vector<PendingSessionActivationConfig>& configs)
{
    std::vector<std::shared_ptr<SessionInfo>> sessionInfos;
    std::vector<std::shared_ptr<PendingSessionActivationConfig>> abilitySessionActivationConfigs;
    for (const auto& config : configs) {
        auto info = std::make_shared<PendingSessionActivationConfig>();
        info->forceStart = config.forceStart;
        info->forceNewWant = config.forceNewWant;
        abilitySessionActivationConfigs.emplace_back(info);
    }
    for (auto& abilitySessionInfo : abilitySessionInfos) {
        if (abilitySessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        bool isFromAncoAndToAnco = session->IsAnco() && AbilityInfoManager::GetInstance().IsAnco(
            abilitySessionInfo->want.GetElement().GetBundleName(),
            abilitySessionInfo->want.GetElement().GetAbilityName(), abilitySessionInfo->want.GetModuleName());
        if (session->DisallowActivationFromPendingBackground(session->IsPcOrPadEnableActivation(), isFoundationCall,
            abilitySessionInfo->canStartAbilityFromBackground, isFromAncoAndToAnco)) {
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        std::shared_ptr<SessionInfo> info =
            std::make_shared<SessionInfo>(
            MakeSessionInfoDuringPendingActivation(abilitySessionInfo, session, isFoundationCall));
        sessionInfos.emplace_back(info);
        if (MultiInstanceManager::IsSupportMultiInstance(session->systemConfig_) &&
            MultiInstanceManager::GetInstance().IsMultiInstance(info->bundleName_)) {
            if (!MultiInstanceManager::GetInstance().MultiInstancePendingSessionActivation(*info)) {
                TLOGE(WmsLogTag::WMS_LIFE,
                    "multi instance start fail, id:%{public}d instanceKey:%{public}s",
                    session->GetPersistentId(), info->appInstanceKey_.c_str());
                return WSError::WS_ERROR_INVALID_PARAM;
            }
        }
        session->sessionInfo_.reuseDelegatorWindow = abilitySessionInfo->reuseDelegatorWindow;
        session->HandleCastScreenConnection(*info, session);
    }
    session->sessionInfo_.startMethod = StartMethod::START_CALL;
    if (session->batchPendingSessionsActivationFunc_) {
        session->batchPendingSessionsActivationFunc_(sessionInfos, abilitySessionActivationConfigs);
    }
    return WSError::WS_OK;
}


WSError SceneSession::BatchPendingSessionsActivation(const std::vector<sptr<AAFwk::SessionInfo>>& abilitySessionInfos,
    const std::vector<PendingSessionActivationConfig>& configs)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    if (!configs.empty() && abilitySessionInfos.size() != configs.size()) {
        TLOGE(WmsLogTag::WMS_LIFE,
            "The caller Param is illegal parameters. abilitySessionInfos: %{public}zu configs: %{public}zu",
            abilitySessionInfos.size(), configs.size());
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    bool isFoundationCall = SessionPermission::IsFoundationCall();
    PostTask([weakThis = wptr(this), abilitySessionInfos, configs, isFoundationCall, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfos.empty()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s abilitySessionInfo is null", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        return session->DoBatchPendingSessionsActivation(abilitySessionInfos, session, isFoundationCall, configs);
    }, __func__);
    return WSError::WS_OK;
}

bool SceneSession::DisallowActivationFromPendingBackground(bool isPcOrPadEnableActivation, bool isFoundationCall,
    bool canStartAbilityFromBackground, bool isFromAncoAndToAnco)
{
    if (isPcOrPadEnableActivation || !WindowHelper::IsMainWindow(GetWindowType())) {
        return false;
    }
    bool isPendingToBackgroundState = GetIsPendingToBackgroundState();
    bool foregroundInteractiveStatus = GetForegroundInteractiveStatus();
    TLOGI(WmsLogTag::WMS_LIFE, "session state:%{public}d, isFoundationCall:%{public}u, "
        "canStartAbilityFromBackground:%{public}u, foregroundInteractiveStatus:%{public}u, "
        "isPendingToBackgroundState:%{public}u, isFromAncoAndToAnco:%{public}u",
        GetSessionState(), isFoundationCall, canStartAbilityFromBackground, foregroundInteractiveStatus,
        isPendingToBackgroundState, isFromAncoAndToAnco);
    bool isSessionForeground = GetSessionState() == SessionState::STATE_FOREGROUND ||
        GetSessionState() == SessionState::STATE_ACTIVE;
    if (isSessionForeground) {
        if (isPendingToBackgroundState) {
            if (!(isFoundationCall && canStartAbilityFromBackground) && !isFromAncoAndToAnco) {
                TLOGW(WmsLogTag::WMS_LIFE, "no permission to start ability from PendingBackground, id:%{public}d",
                    GetPersistentId());
                return true;
            }
        } else if (!foregroundInteractiveStatus) {
            TLOGW(WmsLogTag::WMS_LIFE, "start ability invalid, session in a non interactive state");
            return true;
        }
    } else if (!(isFoundationCall && canStartAbilityFromBackground) && !isFromAncoAndToAnco) {
        TLOGW(WmsLogTag::WMS_LIFE, "no permission to start ability from Background, id:%{public}d",
            GetPersistentId());
        return true;
    }
    return false;
}

void SceneSession::HandleCastScreenConnection(SessionInfo& info, sptr<SceneSession> session)
{
    ScreenId defScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    if (defScreenId == info.screenId_) {
        return;
    }
    auto flag = Rosen::ScreenManager::GetInstance().GetVirtualScreenFlag(info.screenId_);
    if (flag != VirtualScreenFlag::CAST) {
        return;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Get exist session state :%{public}d persistentId:%{public}d",
        session->GetSessionState(), info.callerPersistentId_);
    if (session->GetSessionState() != SessionState::STATE_FOREGROUND &&
        session->GetSessionState() != SessionState::STATE_ACTIVE) {
        TLOGI(WmsLogTag::WMS_LIFE, "Get exist session state is not foreground");
        return;
    }
    info.isCastSession_ = true;
    std::vector<uint64_t> mirrorIds { info.screenId_ };
    Rosen::DMError ret = Rosen::ScreenManager::GetInstance().MakeUniqueScreen(mirrorIds);
    if (ret != Rosen::DMError::DM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "MakeUniqueScreen failed,ret: %{public}d", ret);
        return;
    }
}

static bool IsNeedSystemPermissionByAction(WSPropertyChangeAction action,
    const sptr<WindowSessionProperty>& property, const sptr<WindowSessionProperty>& sessionProperty)
{
    switch (action) {
        case WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON:
        case WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP:
        case WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS:
        case WSPropertyChangeAction::ACTION_UPDATE_TOPMOST:
        case WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE:
        case WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO:
        case WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE:
            return true;
        case WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG:
            return property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM);
        case WSPropertyChangeAction::ACTION_UPDATE_FLAGS: {
            uint32_t oldFlags = sessionProperty->GetWindowFlags();
            uint32_t flags = property->GetWindowFlags();
            if ((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK)) {
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

WMError SceneSession::UpdateSessionPropertyByAction(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "property is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "get session property failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (action == WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE) {
        if (!SessionPermission::VerifyCallingPermission("ohos.permission.PRIVACY_WINDOW")) {
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
    }
    if (action == WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST) {
        uint32_t accessTokenId = property->GetAccessTokenId();
        if (!SessionPermission::VerifyPermissionByCallerToken(accessTokenId,
            PermissionConstants::PERMISSION_MAIN_WINDOW_TOPMOST)) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "The caller has no permission granted.");
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
    }

    bool isSystemCalling = SessionPermission::IsSystemCalling() || SessionPermission::IsStartByHdcd();
    if (!isSystemCalling && IsNeedSystemPermissionByAction(action, property, sessionProperty)) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied! action: %{public}" PRIu64, action);
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    property->SetSystemCalling(isSystemCalling);
    auto task = [weak = wptr(this), property, action, where = __func__]() -> WMError {
        auto sceneSession = weak.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::DEFAULT, "%{public}s the session is nullptr", where);
            return WMError::WM_DO_NOTHING;
        }
        TLOGND(WmsLogTag::DEFAULT, "%{public}s Id: %{public}d, action: %{public}" PRIu64,
            where, sceneSession->GetPersistentId(), action);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession:UpdateProperty");
        return sceneSession->HandleUpdatePropertyByAction(property, action);
    };
    if (AppExecFwk::EventRunner::IsAppMainThread()) {
        PostTask(std::move(task), __func__);
        return WMError::WM_OK;
    }
    return PostSyncTask(std::move(task), __func__);
}

WMError SceneSession::SetGestureBackEnabled(bool isEnabled)
{
    PostTask([weakThis = wptr(this), isEnabled, where = __func__] {
        auto sceneSession = weakThis.promote();
        if (!sceneSession) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is invalid", where);
            return;
        }
        if (sceneSession->isEnableGestureBack_ == isEnabled) {
            TLOGND(WmsLogTag::WMS_IMMS, "%{public}s isEnabled equals last", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s win %{public}d isEnabled %{public}d",
            where, sceneSession->GetPersistentId(), isEnabled);
        sceneSession->isEnableGestureBack_ = isEnabled;
        sceneSession->isEnableGestureBackHadSet_ = true;
        sceneSession->UpdateGestureBackEnabled();
    }, __func__);
    return WMError::WM_OK;
}

bool SceneSession::GetGestureBackEnabled()
{
    return isEnableGestureBack_;
}

bool SceneSession::GetEnableGestureBackHadSet()
{
    return isEnableGestureBackHadSet_;
}

void SceneSession::UpdateFullScreenWaterfallMode(bool isWaterfallMode)
{
    PostTask([weakThis = wptr(this), isWaterfallMode, where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is nullptr", where);
            return;
        }
        if (session->pcFoldScreenController_ == nullptr) {
            return;
        }
        session->pcFoldScreenController_->UpdateFullScreenWaterfallMode(isWaterfallMode);
    }, __func__);
}

void SceneSession::OnWaterfallButtonChange(bool isShow)
{
    PostTask([weakThis = wptr(this), isShow, where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is nullptr", where);
            return;
        }
        if (session->pcFoldScreenController_ == nullptr) {
            return;
        }
        session->pcFoldScreenController_->UpdateSupportEnterWaterfallMode(isShow);
    }, __func__);
}

bool SceneSession::IsFullScreenWaterfallMode()
{
    if (pcFoldScreenController_ == nullptr) {
        return false;
    }
    return pcFoldScreenController_->IsFullScreenWaterfallMode();
}

void SceneSession::SetFullScreenWaterfallMode(bool isFullScreenWaterfallMode)
{
    if (pcFoldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d pcFoldScreenController is null", GetPersistentId());
        return;
    }
    return pcFoldScreenController_->SetFullScreenWaterfallMode(isFullScreenWaterfallMode);
}

WSError SceneSession::GetWaterfallMode(bool& isWaterfallMode)
{
    isWaterfallMode = IsFullScreenWaterfallMode();
    return WSError::WS_OK;
}

WMError SceneSession::IsMainWindowFullScreenAcrossDisplays(bool& isAcrossDisplays)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d permission denied!", GetPersistentId());
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!WindowHelper::IsAppWindow(GetWindowType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid window: %{public}d type %{public}u",
            GetPersistentId(), GetWindowType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto parentSession = GetSceneSessionById(GetMainSessionId());
    if (!parentSession) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d parent session is null", GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    isAcrossDisplays = parentSession->IsFullScreenWaterfallMode();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d isAcrossDisplays: %{public}u",
        GetPersistentId(), isAcrossDisplays);
    return WMError::WM_OK;
}

WMError SceneSession::NotifySubSessionAcrossDisplaysChange(bool isAcrossDisplays)
{
    std::vector<sptr<SceneSession>> subSessionVec = GetSubSession();
    for (const auto& subSession : subSessionVec) {
        if (subSession == nullptr) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "subsession is null");
            continue;
        }
        subSession->NotifySubSessionAcrossDisplaysChange(isAcrossDisplays);
        if (!subSession->GetIsRegisterAcrossMultiDisplayChanged()) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "subsession: %{public}d not register", subSession->GetPersistentId());
            continue;
        }
        NotifySessionAcrossDisplaysChange(subSession, isAcrossDisplays);
    }
    return WMError::WM_OK;
}

WMError SceneSession::NotifyFollowedParentWindowAcrossDisplaysChange(bool isAcrossDisplays)
{
    std::unordered_map<int32_t, NotifySurfaceBoundsChangeFunc> funcMap;
    {
        std::lock_guard lock(registerNotifySurfaceBoundsChangeMutex_);
        funcMap = notifySurfaceBoundsChangeFuncMap_;
    }
    for (const auto& [sessionId, _] : funcMap) {
        auto subSession = GetSceneSessionById(sessionId);
        if (subSession == nullptr) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "subsession is null");
            continue;
        }
        if (!subSession->GetIsFollowParentLayout()) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "subsession: %{public}d not follow", subSession->GetPersistentId());
            continue;
        }
        NotifySessionAcrossDisplaysChange(subSession, isAcrossDisplays);
    }
    return WMError::WM_OK;
}

void SceneSession::NotifySessionAcrossDisplaysChange(const sptr<SceneSession>& sceneSession, bool isAcrossDisplays)
{
    if (sceneSession->sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "%{public}d: sessionStage is null", sceneSession->GetPersistentId());
        return;
    }
    sceneSession->sessionStage_->SetFullScreenWaterfallMode(isAcrossDisplays);
    sceneSession->SetFullScreenWaterfallMode(isAcrossDisplays);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d isAcrossDisplays: %{public}u",
        sceneSession->GetPersistentId(), isAcrossDisplays);
}

void SceneSession::RegisterFullScreenWaterfallModeChangeCallback(std::function<void(bool isWaterfallMode)>&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__]() mutable {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is nullptr", where);
            return;
        }
        if (session->pcFoldScreenController_ == nullptr) {
            return;
        }
        session->pcFoldScreenController_->RegisterFullScreenWaterfallModeChangeCallback(std::move(func));
    }, __func__);
}

void SceneSession::RegisterThrowSlipAnimationStateChangeCallback(
    std::function<void(bool isAnimating, bool isFullScreen)>&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__]() mutable {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is nullptr", where);
            return;
        }
        session->onThrowSlipAnimationStateChangeFunc_ = std::move(func);
    }, __func__);
}

bool SceneSession::IsMissionHighlighted()
{
    if (!SessionHelper::IsMainWindow(GetWindowType())) {
        return false;
    }
    if (IsFocused()) {
        return true;
    }
    return std::any_of(subSession_.begin(), subSession_.end(),
        [](const sptr<SceneSession>& sceneSession) {
            return sceneSession != nullptr && sceneSession->IsMissionHighlighted();
        });
}

bool SceneSession::IsPcFoldDevice()
{
    return (pcFoldScreenController_ != nullptr);
}

void SceneSession::MaskSupportEnterWaterfallMode()
{
    if (pcFoldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "pcFoldScreenController is null");
        return;
    }
    pcFoldScreenController_->MaskSupportEnterWaterfallMode();
}

void SceneSession::SetSupportEnterWaterfallMode(bool isSupportEnter)
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sessionStage is nullptr");
        return;
    }
    if (!WindowHelper::IsMainWindow(GetWindowType())) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "only main window support enter water fall mode.");
        return;
    }
    sessionStage_->SetSupportEnterWaterfallMode(isSupportEnter);
}

void SceneSession::SetSessionChangeByActionNotifyManagerListener(const SessionChangeByActionNotifyManagerFunc& func)
{
    TLOGD(WmsLogTag::DEFAULT, "setListener success");
    sessionChangeByActionNotifyManagerFunc_ = func;
}

WMError SceneSession::HandleUpdatePropertyByAction(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "property is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    return ProcessUpdatePropertyByAction(property, action);
}

WMError SceneSession::ProcessUpdatePropertyByAction(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    switch (static_cast<uint64_t>(action)) {
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON):
            return HandleActionUpdateTurnScreenOn(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON):
            return HandleActionUpdateKeepScreenOn(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_VIEW_KEEP_SCREEN_ON):
            return HandleActionUpdateViewKeepScreenOn(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE):
            return HandleActionUpdateFocusable(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE):
            return HandleActionUpdateTouchable(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS):
            return HandleActionUpdateSetBrightness(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION):
            return HandleActionUpdateOrientation(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE):
            return HandleActionUpdatePrivacyMode(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE):
            return HandleActionUpdatePrivacyMode(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP):
            return HandleActionUpdateSnapshotSkip(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE):
            return HandleActionUpdateMaximizeState(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS):
            return HandleActionUpdateOtherProps(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS):
            return HandleActionUpdateStatusProps(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS):
            return HandleActionUpdateNavigationProps(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS):
            return HandleActionUpdateNavigationIndicatorProps(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FLAGS):
            return HandleActionUpdateFlags(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE):
            return HandleActionUpdateMode(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG):
            return HandleActionUpdateAnimationFlag(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA):
            return HandleActionUpdateTouchHotArea(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_KEYBOARD_TOUCH_HOT_AREA):
            return HandleActionUpdateKeyboardTouchHotArea(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE):
            return HandleActionUpdateDecorEnable(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS):
            return HandleActionUpdateWindowLimits(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED):
            return HandleActionUpdateDragenabled(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED):
            return HandleActionUpdateRaiseenabled(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS):
            return HandleActionUpdateHideNonSystemFloatingWindows(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO):
            return HandleActionUpdateTextfieldAvoidInfo(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK):
            return HandleActionUpdateWindowMask(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOPMOST):
            return HandleActionUpdateTopmost(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST):
            return HandleActionUpdateMainWindowTopmost(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL):
            return HandleActionUpdateSubWindowZLevel(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO):
            return HandleActionUpdateWindowModeSupportType(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_AVOID_AREA_OPTION):
            return HandleActionUpdateAvoidAreaOption(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_BACKGROUND_ALPHA):
            return HandleBackgroundAlpha(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_EXCLUSIVE_HIGHLIGHTED):
            return HandleActionUpdateExclusivelyHighlighted(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE):
            return HandleActionUpdateFollowScreenChange(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_SHADOW_ENABLED):
            return HandleActionUpdateWindowShadowEnabled(property, action);
        case static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE):
            return HandleActionUpdateRotationLockChange(property, action);
        default:
            TLOGE(WmsLogTag::DEFAULT, "Failed to find func handler!");
            return WMError::WM_DO_NOTHING;
    }
}

WMError SceneSession::HandleActionUpdateTurnScreenOn(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetTurnScreenOn(property->IsTurnScreenOn());
#ifdef POWER_MANAGER_ENABLE
    PostExportTask([weakThis = wptr(this), where = __func__] {
        auto sceneSession = weakThis.promote();
        if (!sceneSession) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s session is invalid", where);
            return;
        }
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s Win: %{public}s, is turn on: %{public}d",
            where, sceneSession->GetWindowName().c_str(), sceneSession->IsTurnScreenOn());
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        if (sceneSession->IsTurnScreenOn()) {
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s turn screen on", where);
            PowerMgr::PowerMgrClient::GetInstance().WakeupDevice();
        }
        // set ipc identity to raw
        IPCSkeleton::SetCallingIdentity(identity);
    }, __func__);
#else
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Can not found the sub system of PowerMgr");
#endif
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateKeepScreenOn(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetKeepScreenOn(property->IsKeepScreenOn());
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateViewKeepScreenOn(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d, enabled: %{public}u",
        GetPersistentId(), property->IsViewKeepScreenOn());
    SetViewKeepScreenOn(property->IsViewKeepScreenOn());
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateWindowShadowEnabled(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d, enabled: %{public}u",
        GetPersistentId(), property->GetWindowShadowEnabled());
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_WINDOW_TRANSPARENT) &&
        containerColorList_.count(GetSessionInfo().bundleName_) == 0) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d: permission denied", GetPersistentId());
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!WindowHelper::IsMainWindow(GetWindowType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, type is not supported", GetPersistentId());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    SetWindowShadowEnabled(property->GetWindowShadowEnabled());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateFocusable(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (!ScreenSessionManagerClient::GetInstance().GetSupportsFocus(GetSessionProperty()->GetDisplayId())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "screen of current window is not allowed to be focused");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    SetFocusable(property->GetFocusable());
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTouchable(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetTouchable(property->GetTouchable());
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateSetBrightness(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
        GetWindowType() != WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "only app main window or wallet swipe card can set brightness");
        return WMError::WM_OK;
    }
    if (!IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    float brightness = property->GetBrightness();
    if (std::abs(brightness - GetBrightness()) < std::numeric_limits<float>::epsilon()) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Session brightness do not change: [%{public}f]", brightness);
        return WMError::WM_OK;
    }
    SetBrightness(brightness);
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateOrientation(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetRequestedOrientation(property->GetRequestedOrientation(), property->GetRequestedAnimation());
    SceneSessionChangeInfo changeInfo {
        .persistentId_ = property->GetPersistentId(),
        .changeInfo_ = "Orientation change to " +
            std::to_string(static_cast<uint32_t>(property->GetRequestedOrientation())) + ", animation change to "+
            std::to_string(static_cast<uint32_t>(property->GetRequestedAnimation())),
        .logTag_ = WmsLogTag::WMS_ROTATION,
    };
    SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::ORIENTAION_RECORD, changeInfo);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdatePrivacyMode(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    bool isPrivacyMode = property->GetPrivacyMode() || property->GetSystemPrivacyMode();
    SetPrivacyMode(isPrivacyMode);
    NotifySessionChangeByActionNotifyManager(property, action);
    SceneSessionChangeInfo changeInfo {
        .persistentId_ = property->GetPersistentId(),
        .changeInfo_ = "Privacy mode change to " + std::to_string(isPrivacyMode),
        .logTag_ = WmsLogTag::WMS_ATTRIBUTE,
    };
    SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::PRIVACY_MODE, changeInfo);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateSnapshotSkip(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    PostTask([weakThis = wptr(this), isSkip = property->GetSnapshotSkip()] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "session is null");
            return;
        }
        if (session->onSnapshotSkipChangeFunc_) {
            session->onSnapshotSkipChangeFunc_(isSkip);
        }
    }, __func__);
    return SetSnapshotSkip(property->GetSnapshotSkip());
}

WMError SceneSession::HandleActionUpdateMaximizeState(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetMaximizeMode(property->GetMaximizeMode());
        sessionProperty->SetIsLayoutFullScreen(property->IsLayoutFullScreen());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateOtherProps(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto systemBarProperties = property->GetSystemBarProperty();
    for (auto iter : systemBarProperties) {
        SetSystemBarProperty(iter.first, iter.second);
    }
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateStatusProps(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    HandleSpecificSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property);
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateNavigationProps(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    HandleSpecificSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, property);
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateNavigationIndicatorProps(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    HandleSpecificSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, property);
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateFlags(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetWindowFlags(property);
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateMode(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowMode(property->GetWindowMode());
    }
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateAnimationFlag(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetAnimationFlag(property->GetAnimationFlag());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTouchHotArea(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    std::vector<Rect> touchHotAreas;
    property->GetTouchHotAreas(touchHotAreas);
    GetSessionProperty()->SetTouchHotAreas(touchHotAreas);
    if (specificCallback_ != nullptr && specificCallback_->onWindowInfoUpdate_ != nullptr) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "id=%{public}d", GetPersistentId());
        specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateDecorEnable(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (property != nullptr && !property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "update decor enable permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetDecorEnable(property->IsDecorEnable());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateWindowLimits(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowLimits(property->GetWindowLimits());
        sessionProperty->SetWindowLimitsVP(property->GetWindowLimitsVP());
        sessionProperty->SetUserWindowLimits(property->GetUserWindowLimits());

        WindowLimits windowLimits = sessionProperty->GetWindowLimits();
        WindowLimits windowLimitsVP = sessionProperty->GetWindowLimitsVP();
        WindowLimits userWindowLimits = sessionProperty->GetUserWindowLimits();
        TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, px:%{public}s, vp:%{public}s, userLimitsUnit:%{public}u",
            GetPersistentId(), windowLimits.ToString().c_str(), windowLimitsVP.ToString().c_str(),
            userWindowLimits.pixelUnit_);

        bool useVPLimits = (userWindowLimits.pixelUnit_ == PixelUnit::VP);
        const WindowLimits& limitsToNotify = useVPLimits ? windowLimitsVP : windowLimits;
        NotifySessionWindowLimitsChange(limitsToNotify);
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateDragenabled(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetDragEnabled(property->GetDragEnabled());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateRaiseenabled(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        TLOGI(WmsLogTag::WMS_HIERARCHY, "id: %{public}d, raise enabled: %{public}d", GetPersistentId(),
            property->GetRaiseEnabled());
        sessionProperty->SetRaiseEnabled(property->GetRaiseEnabled());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateHideNonSystemFloatingWindows(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Update property hideNonSystemFloatingWindows permission denied!");
        return WMError::WM_OK;
    }
    auto currentProperty = GetSessionProperty();
    if (currentProperty != nullptr) {
        NotifySessionChangeByActionNotifyManager(property, action);
        currentProperty->SetHideNonSystemFloatingWindows(property->GetHideNonSystemFloatingWindows());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTextfieldAvoidInfo(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetTextFieldPositionY(property->GetTextFieldPositionY());
        sessionProperty->SetTextFieldHeight(property->GetTextFieldHeight());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateWindowMask(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowMask(property->GetWindowMask());
        sessionProperty->SetIsShaped(property->GetIsShaped());
        NotifySessionChangeByActionNotifyManager(property, action);
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTopmost(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "UpdateTopmostProperty permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    SetTopmost(property->IsTopmost());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateMainWindowTopmost(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetMainWindowTopmost(property->IsMainWindowTopmost());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateSubWindowZLevel(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    SetSubWindowZLevel(property->GetSubWindowZLevel());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateAvoidAreaOption(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (!SessionHelper::IsSubWindow(sessionProperty->GetWindowType()) &&
        !SessionHelper::IsSystemWindow(sessionProperty->GetWindowType())) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    sessionProperty->SetAvoidAreaOption(property->GetAvoidAreaOption());
    return WMError::WM_OK;
}

WMError SceneSession::HandleBackgroundAlpha(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "property is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    sessionProperty->SetBackgroundAlpha(property->GetBackgroundAlpha());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateExclusivelyHighlighted(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_FOCUS, "property is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    sessionProperty->SetExclusivelyHighlighted(property->GetExclusivelyHighlighted());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateFollowScreenChange(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    UpdateFollowScreenChange(property->GetFollowScreenChange());
    NotifySessionChangeByActionNotifyManager(property, action);
    return WMError::WM_OK;
}

WSError SceneSession::UpdateFollowScreenChange(bool isFollowScreenChange)
{
    SetFollowScreenChange(isFollowScreenChange);
    auto task = [weakThis = wptr(this), isFollowScreenChange] {
        auto session = weakThis.promote();
        if (!session || !session->specificCallback_) {
            TLOGNE(WmsLogTag::DEFAULT, "session or specific callback is null");
            return;
        }
        if (session->specificCallback_->onUpdateFollowScreenChange_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession:UpdateFollowScreenChange");
            session->specificCallback_->onUpdateFollowScreenChange_(isFollowScreenChange);
        }
    };
    PostTask(std::move(task), "UpdateFollowScreenChange");
    return WSError::WS_OK;
}

void SceneSession::HandleSpecificSystemBarProperty(WindowType type, const sptr<WindowSessionProperty>& property)
{
    auto systemBarProperties = property->GetSystemBarProperty();
    if (auto iter = systemBarProperties.find(type); iter != systemBarProperties.end()) {
        SetSystemBarProperty(iter->first, iter->second);
        TLOGD(WmsLogTag::WMS_IMMS, "type %{public}d enable %{public}d",
            static_cast<int32_t>(iter->first), iter->second.enable_);
    }
}

void SceneSession::SetWindowFlags(const sptr<WindowSessionProperty>& property)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "get session property failed");
        return;
    }
    uint32_t flags = property->GetWindowFlags();
    uint32_t oldFlags = sessionProperty->GetWindowFlags();
    if (((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED) ||
         (oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK)) &&
        !property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Set window flags permission denied");
        return;
    }
    sessionProperty->SetWindowFlags(flags);
    if ((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) {
        OnShowWhenLocked(flags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    }
    TLOGI(WmsLogTag::DEFAULT, "%{public}u", flags);
}

void SceneSession::NotifySessionChangeByActionNotifyManager(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    TLOGD(WmsLogTag::DEFAULT, "id: %{public}d, action: %{public}" PRIu64, GetPersistentId(), action);
    if (sessionChangeByActionNotifyManagerFunc_ == nullptr) {
        TLOGW(WmsLogTag::DEFAULT, "func is null");
        return;
    }
    sessionChangeByActionNotifyManagerFunc_(this, property, action);
}

WSError SceneSession::TerminateSession(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    PostLifeCycleTask([weakThis = wptr(this), abilitySessionInfo, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s abilitySessionInfo is null", where);
            session->RemoveLifeCycleTask(LifeCycleTaskType::STOP);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s is terminating, return!", where);
            session->RemoveLifeCycleTask(LifeCycleTaskType::STOP);
            return WSError::WS_ERROR_INVALID_OPERATION;
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
        if (session->terminateSessionFunc_) {
            session->terminateSessionFunc_(info);
        }
        return WSError::WS_OK;
    }, __func__, LifeCycleTaskType::STOP);
    return WSError::WS_OK;
}

WSError SceneSession::NotifySessionExceptionInner(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
    const ExceptionInfo& exceptionInfo, bool isFromClient, bool startFail)
{
    PostLifeCycleTask([weakThis = wptr(this), abilitySessionInfo, exceptionInfo,
        isFromClient, startFail, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s abilitySessionInfo is null", where);
            session->RemoveLifeCycleTask(LifeCycleTaskType::STOP);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (SessionHelper::IsMainWindow(session->GetWindowType()) && !session->clientIdentityToken_.empty() &&
            isFromClient && (abilitySessionInfo->errorReason != ERROR_REASON_LOW_MEMORY_KILL &&
            session->clientIdentityToken_ != abilitySessionInfo->identityToken)) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s client exception not matched: %{public}s, %{public}s",
                where, session->clientIdentityToken_.c_str(), abilitySessionInfo->identityToken.c_str());
            session->RemoveLifeCycleTask(LifeCycleTaskType::STOP);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        session->isTerminating_ = true;
        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.errorCode = abilitySessionInfo->errorCode;
        info.errorReason = abilitySessionInfo->errorReason;
        info.persistentId_ = static_cast<int32_t>(abilitySessionInfo->persistentId);
        {
            std::lock_guard<std::recursive_mutex> lock(session->sessionInfoMutex_);
            session->sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
            session->sessionInfo_.errorCode = abilitySessionInfo->errorCode;
            session->sessionInfo_.errorReason = abilitySessionInfo->errorReason;
        }
        if (session->sessionExceptionFunc_) {
            session->sessionExceptionFunc_(info, exceptionInfo, false);
        }
        if (session->jsSceneSessionExceptionFunc_) {
            session->jsSceneSessionExceptionFunc_(info, exceptionInfo, startFail);
        }
        return WSError::WS_OK;
    }, __func__, LifeCycleTaskType::STOP);
    return WSError::WS_OK;
}

WSError SceneSession::NotifySessionException(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
    const ExceptionInfo& exceptionInfo)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "permission failed.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, true);
}

WSError SceneSession::NotifySessionExceptionWithOptions(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
    const std::string& reason, bool needRemoveSession)
{
    ExceptionInfo exceptionInfo;
    exceptionInfo.needRemoveSession = needRemoveSession;
    abilitySessionInfo->errorReason = reason;
    return NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false, false);
}

WSRect SceneSession::GetLastSafeRect() const
{
    return lastSafeRect;
}

void SceneSession::SetLastSafeRect(WSRect rect)
{
    lastSafeRect.posX_ = rect.posX_;
    lastSafeRect.posY_ = rect.posY_;
    lastSafeRect.width_ = rect.width_;
    lastSafeRect.height_ = rect.height_;
    return;
}

void SceneSession::SetMovable(bool movable)
{
    PostTask([weakThis = wptr(this), movable, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        if (session->moveDragController_) {
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s id: %{public}d, isMovable: %{public}d",
                where, session->GetPersistentId(), movable);
            session->moveDragController_->SetMovable(movable);
        }
    }, __func__);
}

WSError SceneSession::SetSplitButtonVisible(bool isVisible)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "isVisible: %{public}d", isVisible);
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->SetSplitButtonVisible(isVisible);
}

WSError SceneSession::SendContainerModalEvent(const std::string& eventName, const std::string& eventValue)
{
    TLOGI(WmsLogTag::WMS_EVENT, "name: %{public}s, value: %{public}s", eventName.c_str(), eventValue.c_str());
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_EVENT, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (eventName == EVENT_NAME_CHANGE_WATER_FALL_BUTTON) {
        OnWaterfallButtonChange(eventValue == WATER_FALL_BUTTON_SHOW);
        return WSError::WS_OK;
    }
    return sessionStage_->SendContainerModalEvent(eventName, eventValue);
}

WSError SceneSession::OnContainerModalEvent(const std::string& eventName, const std::string& eventValue)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "name: %{public}s, value: %{public}s", eventName.c_str(), eventValue.c_str());
    if (eventName == WINDOW_RELOCATION_EVENT) {
        if (eventValue == "true") {
            ThrowSlipDirectly(ThrowSlipMode::BUTTON, VELOCITY_RELOCATION_TO_TOP);
        } else {
            ThrowSlipDirectly(ThrowSlipMode::BUTTON, VELOCITY_RELOCATION_TO_BOTTOM);
        }
    }
    return WSError::WS_OK;
}

WSError SceneSession::SetSceneAnimationConfig(const SceneAnimationConfig& animationConfig)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "SetSceneAnimationConfig");
    sceneAnimationConfig_.animationDelay_ = animationConfig.animationDelay_;
    sceneAnimationConfig_.animationDuration_ = animationConfig.animationDuration_;
    sceneAnimationConfig_.animationParam_ = animationConfig.animationParam_;
    sceneAnimationConfig_.animationCurve_ = animationConfig.animationCurve_;
    return WSError::WS_OK;
}

void SceneSession::RegisterSetLandscapeMultiWindowFunc(NotifyLandscapeMultiWindowSessionFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "%{public}s session is null", where);
            return;
        }
        session->onSetLandscapeMultiWindowFunc_ = std::move(callback);
    }, __func__);
}

int32_t SceneSession::GetOriPosYBeforeRaisedByKeyboard() const
{
    return oriPosYBeforeRaisedByKeyboard_;
}

void SceneSession::SetOriPosYBeforeRaisedByKeyboard(int32_t posY)
{
    oriPosYBeforeRaisedByKeyboard_ = posY;
}

bool SceneSession::AddSubSession(const sptr<SceneSession>& subSession)
{
    if (subSession == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "subSession is nullptr");
        return false;
    }
    const auto& persistentId = subSession->GetPersistentId();
    auto iter = std::find_if(subSession_.begin(), subSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter != subSession_.end()) {
        TLOGE(WmsLogTag::WMS_SUB, "Sub ession is already exists, id: %{public}d, parentId: %{public}d",
            subSession->GetPersistentId(), GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_SUB, "Success, id: %{public}d, parentId: %{public}d",
        subSession->GetPersistentId(), GetPersistentId());
    subSession_.push_back(subSession);
    return true;
}

bool SceneSession::RemoveSubSession(int32_t persistentId)
{
    auto iter = std::find_if(subSession_.begin(), subSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter == subSession_.end()) {
        TLOGE(WmsLogTag::WMS_SUB, "Could not find subsession, id: %{public}d, parentId: %{public}d",
            persistentId, GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_SUB, "Success, id: %{public}d, parentId: %{public}d", persistentId, GetPersistentId());
    subSession_.erase(iter);
    return true;
}

bool SceneSession::AddToastSession(const sptr<SceneSession>& toastSession)
{
    if (toastSession == nullptr) {
        TLOGE(WmsLogTag::WMS_TOAST, "toastSession is nullptr");
        return false;
    }
    const auto& persistentId = toastSession->GetPersistentId();
    auto iter = std::find_if(toastSession_.begin(), toastSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter != toastSession_.end()) {
        TLOGE(WmsLogTag::WMS_TOAST, "Toast ession is already exists, id: %{public}d, parentId: %{public}d",
            toastSession->GetPersistentId(), GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_TOAST, "Success, id: %{public}d, parentId: %{public}d",
        toastSession->GetPersistentId(), GetPersistentId());
    toastSession_.push_back(toastSession);
    return true;
}

bool SceneSession::RemoveToastSession(int32_t persistentId)
{
    auto iter = std::find_if(toastSession_.begin(), toastSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter == toastSession_.end()) {
        TLOGE(WmsLogTag::WMS_TOAST, "Could not find toastSession, id: %{public}d, parentId: %{public}d",
            persistentId, GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_TOAST, "Success, id: %{public}d, parentId: %{public}d", persistentId, GetPersistentId());
    toastSession_.erase(iter);
    return true;
}

void SceneSession::NotifyPiPWindowPrepareClose()
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    PostTask([weakThis = wptr(this), callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null", where);
            return;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNW(WmsLogTag::WMS_PIP, "%{public}s permission denied, not call by the same process", where);
            return;
        }
        if (session->onPrepareClosePiPSession_) {
            session->onPrepareClosePiPSession_();
        }
        TLOGND(WmsLogTag::WMS_PIP, "%{public}s id: %{public}d", where, session->GetPersistentId());
    }, __func__);
}

WSError SceneSession::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "in");
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    PostTask([weakThis = wptr(this), isLandscapeMultiWindow, callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "%{public}s premission denied, not call by the same process", where);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (session->onSetLandscapeMultiWindowFunc_) {
            session->onSetLandscapeMultiWindowFunc_(isLandscapeMultiWindow);
        }
        TLOGND(WmsLogTag::WMS_MULTI_WINDOW, "%{public}s id: %{public}d, isLandscapeMultiWindow: %{public}u",
            where, session->GetPersistentId(), isLandscapeMultiWindow);
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

std::vector<sptr<SceneSession>> SceneSession::GetSubSession() const
{
    return subSession_;
}

std::vector<sptr<SceneSession>> SceneSession::GetToastSession() const
{
    return toastSession_;
}

WSRect SceneSession::GetSessionTargetRectByDisplayId(DisplayId displayId) const
{
    WSRect rect;
    if (moveDragController_) {
        rect = moveDragController_->GetTargetRectByDisplayId(displayId);
    } else {
        WLOGFI("moveDragController_ is null");
    }
    return rect;
}

void SceneSession::SetWindowDragHotAreaListener(const NotifyWindowDragHotAreaFunc& func)
{
    if (moveDragController_) {
        moveDragController_->SetWindowDragHotAreaFunc(func);
    }
}

void SceneSession::NotifySessionForeground(uint32_t reason, bool withAnimation)
{
    if (!sessionStage_) {
        return;
    }
    return sessionStage_->NotifySessionForeground(reason, withAnimation);
}

void SceneSession::NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    if (!sessionStage_) {
        return;
    }
    return sessionStage_->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
}

void SceneSession::NotifySessionFullScreen(bool fullScreen)
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sessionStage is null");
        return;
    }
    sessionStage_->NotifySessionFullScreen(fullScreen);
}

WSError SceneSession::UpdatePiPRect(const Rect& rect, SizeChangeReason reason)
{
    if (!WindowHelper::IsPipWindow(GetWindowType())) {
        return WSError::WS_DO_NOTHING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), rect, reason, callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_PIP,
                "%{public}s session is null or is terminating", where);
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNW(WmsLogTag::WMS_PIP, "%{public}s permission denied, not call by the same process", where);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        WSRect wsRect = SessionHelper::TransferToWSRect(rect);
        if (reason == SizeChangeReason::PIP_START) {
            session->SetSessionRequestRect(wsRect);
        }
        TLOGNI(WmsLogTag::WMS_PIP, "%{public}s rect:%{public}s, reason:%{public}u", where, wsRect.ToString().c_str(),
            static_cast<uint32_t>(reason));
        session->NotifySessionRectChange(wsRect, reason);
        return WSError::WS_OK;
    };
    if (mainHandler_ != nullptr) {
        mainHandler_->PostTask(std::move(task), __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        PostTask(std::move(task), __func__);
    }
    return WSError::WS_OK;
}

WSError SceneSession::UpdatePiPControlStatus(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    if (!WindowHelper::IsPipWindow(GetWindowType())) {
        return WSError::WS_DO_NOTHING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    PostTask([weakThis = wptr(this), controlType, status, callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null or is terminating", where);
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNW(WmsLogTag::WMS_PIP, "%{public}s permission denied, not call by the same process", where);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (session->sessionPiPControlStatusChangeFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::UpdatePiPControlStatus");
            session->sessionPiPControlStatusChangeFunc_(controlType, status);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::SetAutoStartPiP(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height)
{
    TLOGI(WmsLogTag::WMS_PIP, "isAutoStart:%{public}u priority:%{public}u width:%{public}u height:%{public}u",
        isAutoStart, priority, width, height);
    PostTask([weakThis = wptr(this), isAutoStart, priority, width, height, where = __func__] {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_PIP, "%{public}s session is null or is terminating", where);
            return;
        }
        if (session->autoStartPiPStatusChangeFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetAutoStartPiP");
            session->autoStartPiPStatusChangeFunc_(isAutoStart, priority, width, height);
        }
    }, __func__);
    return WSError::WS_OK;
}

void SceneSession::SendPointerEventToUI(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    NotifySystemSessionPointerEventFunc systemSessionPointerEventFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(pointerEventMutex_);
        systemSessionPointerEventFunc = systemSessionPointerEventFunc_;
    }
    if (systemSessionPointerEventFunc != nullptr) {
        systemSessionPointerEventFunc(pointerEvent);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "PointerEventFunc_ nullptr, id:%{public}d", pointerEvent->GetId());
        pointerEvent->MarkProcessed();
    }
}

bool SceneSession::SendKeyEventToUI(std::shared_ptr<MMI::KeyEvent> keyEvent, bool isPreImeEvent)
{
    NotifySystemSessionKeyEventFunc systemSessionKeyEventFunc = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(keyEventMutex_);
        systemSessionKeyEventFunc = systemSessionKeyEventFunc_;
    }
    if (systemSessionKeyEventFunc != nullptr) {
        return systemSessionKeyEventFunc(keyEvent, isPreImeEvent);
    }
    return false;
}

WSError SceneSession::UpdateSizeChangeReason(SizeChangeReason reason)
{
    PostTask([weakThis = wptr(this), reason, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->Session::UpdateSizeChangeReason(reason);
        if (reason != SizeChangeReason::UNDEFINED) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                "SceneSession::UpdateSizeChangeReason%d reason:%d",
                session->GetPersistentId(), static_cast<uint32_t>(reason));
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s Id: %{public}d, reason: %{public}d",
                where, session->GetPersistentId(), reason);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

void SceneSession::ResetSizeChangeReasonIfDirty()
{
    auto reason = GetSizeChangeReason();
    if (IsDirtyWindow() &&
        reason != SizeChangeReason::DRAG &&
        reason != SizeChangeReason::DRAG_END &&
        reason != SizeChangeReason::DRAG_START &&
        reason != SizeChangeReason::DRAG_MOVE) {
        UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    }
}

bool SceneSession::IsDirtyWindow()
{
    return dirtyFlags_ & static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
}

bool SceneSession::IsDirtyDragWindow()
{
    return dirtyFlags_ & static_cast<uint32_t>(SessionUIDirtyFlag::DRAG_RECT) || isDragging_;
}

void SceneSession::ResetDirtyDragFlags()
{
    dirtyFlags_ &= ~static_cast<uint32_t>(SessionUIDirtyFlag::DRAG_RECT);
    isDragging_ = false;
}

bool SceneSession::IsNeedNotifyDragEventOnNextVsync() const
{
    return needNotifyDragEventOnNextVsync_;
}

void SceneSession::NotifiedDragEventOnNextVsync()
{
    needNotifyDragEventOnNextVsync_ = false;
}

void SceneSession::NotifyUILostFocus()
{
    if (moveDragController_) {
        moveDragController_->OnLostFocus();
    }
    Session::NotifyUILostFocus();
}

void SceneSession::SetScale(float scaleX, float scaleY, float pivotX, float pivotY)
{
    if (GetScaleX() != scaleX || GetScaleY() != scaleY || GetPivotX() != pivotX || GetPivotY() != pivotY) {
        Session::SetScale(scaleX, scaleY, pivotX, pivotY);
        if (specificCallback_ != nullptr) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
        }
        if (sessionStage_ != nullptr) {
            Transform transform;
            transform.scaleX_ = scaleX;
            transform.scaleY_ = scaleY;
            transform.pivotX_ = pivotX;
            transform.pivotY_ = pivotY;
            sessionStage_->NotifyTransformChange(transform);
        } else {
            WLOGFE("sessionStage_ is nullptr");
        }
    }
}

void SceneSession::RequestHideKeyboard(bool isAppColdStart)
{
#ifdef IMF_ENABLE
    PostExportTask([weakThis = wptr(this), isAppColdStart, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD,
                "%{public}s Session is null, notify inputMethod framework hide keyboard failed!", where);
            return;
        }
        auto callingUid = IPCSkeleton::GetCallingUid();
        bool isBrokerCall = callingUid == ANCO_SERVICE_BROKER_UID;
        auto pid = IPCSkeleton::GetCallingRealPid();
        AppExecFwk::RunningProcessInfo info;
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(pid, info);
        bool isUIEProc = AAFwk::UIExtensionUtils::IsUIExtension(info.extensionType_);
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "%{public}s Notify inputMethod framework hide keyboard start, id: %{public}d,"
            "isAppColdStart: %{public}d, isBrokerCall: %{public}d, isUIEProc: %{public}d", where,
            session->GetPersistentId(), isAppColdStart, isBrokerCall, isUIEProc);
        if (MiscServices::InputMethodController::GetInstance() && !isBrokerCall && !isUIEProc) {
            MiscServices::InputMethodController::GetInstance()->RequestHideInput(
                static_cast<uint32_t>(session->GetPersistentId()));
            TLOGNI(WmsLogTag::WMS_KEYBOARD, "%{public}s Notify inputMethod framework hide keyboard end. id: %{public}d",
                where, session->GetPersistentId());
        }
    }, __func__);
#endif
}

bool SceneSession::IsStartMoving()
{
    return isStartMoving_.load();
}

void SceneSession::SetIsStartMoving(bool startMoving)
{
    isStartMoving_.store(startMoving);
}

void SceneSession::SetShouldHideNonSecureWindows(bool shouldHide)
{
    shouldHideNonSecureWindows_.store(shouldHide);
}

void SceneSession::CalculateCombinedExtWindowFlags()
{
    // Only correct when each flag is true when active, and once a uiextension is active, the host is active
    ExtensionWindowFlags combinedExtWindowOldFlag = combinedExtWindowFlags_;
    combinedExtWindowFlags_.bitData = 0;
    for (const auto& iter: extWindowFlagsMap_) {
        combinedExtWindowFlags_.bitData |= iter.second.bitData;
    }
    if (combinedExtWindowOldFlag.hideNonSecureWindowsFlag != combinedExtWindowFlags_.hideNonSecureWindowsFlag) {
        NotifyExtensionSecureLimitChange(static_cast<bool>(combinedExtWindowFlags_.hideNonSecureWindowsFlag));
    }
    
    NotifyPrivacyModeChange();
}

void SceneSession::UpdateExtWindowFlags(int32_t extPersistentId, const ExtensionWindowFlags& extWindowFlags,
    const ExtensionWindowFlags& extWindowActions)
{
    auto iter = extWindowFlagsMap_.find(extPersistentId);
    // Each flag is false when inactive, 0 means all flags are inactive
    auto oldFlags = iter != extWindowFlagsMap_.end() ? iter->second : ExtensionWindowFlags();
    ExtensionWindowFlags newFlags((extWindowFlags.bitData & extWindowActions.bitData) |
        (oldFlags.bitData & ~extWindowActions.bitData));
    if (newFlags.bitData == 0) {
        extWindowFlagsMap_.erase(extPersistentId);
    } else {
        extWindowFlagsMap_[extPersistentId] = newFlags;
    }
    CalculateCombinedExtWindowFlags();
}

ExtensionWindowFlags SceneSession::GetCombinedExtWindowFlags()
{
    auto combinedExtWindowFlags = combinedExtWindowFlags_;
    combinedExtWindowFlags.hideNonSecureWindowsFlag = IsSessionForeground() &&
        (combinedExtWindowFlags.hideNonSecureWindowsFlag || shouldHideNonSecureWindows_.load());
    return combinedExtWindowFlags;
}

void SceneSession::NotifyDisplayMove(DisplayId from, DisplayId to)
{
    if (sessionStage_) {
        sessionStage_->NotifyDisplayMove(from, to);
    } else {
        WLOGFE("Notify display move failed, sessionStage is null");
    }
}

void SceneSession::RemoveExtWindowFlags(int32_t extPersistentId)
{
    extWindowFlagsMap_.erase(extPersistentId);
    CalculateCombinedExtWindowFlags();
}

void SceneSession::ClearExtWindowFlags()
{
    extWindowFlagsMap_.clear();
    combinedExtWindowFlags_.bitData = 0;
}

WSError SceneSession::UpdateRectChangeListenerRegistered(bool isRegister)
{
    PostTask([weakThis = wptr(this), isRegister, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->rectChangeListenerRegistered_ = isRegister;
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

void SceneSession::SetIsLayoutFullScreen(bool isLayoutFullScreen)
{
    isLayoutFullScreen_ = isLayoutFullScreen;
}

bool SceneSession::IsLayoutFullScreen() const
{
    return isLayoutFullScreen_;
}

WSError SceneSession::OnLayoutFullScreenChange(bool isLayoutFullScreen)
{
    PostTask([weakThis = wptr(this), isLayoutFullScreen, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s isLayoutFullScreen: %{public}d", where, isLayoutFullScreen);
        if (session->onLayoutFullScreenChangeFunc_) {
            session->SetIsLayoutFullScreen(isLayoutFullScreen);
            session->onLayoutFullScreenChangeFunc_(isLayoutFullScreen);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::OnDefaultDensityEnabled(bool isDefaultDensityEnabled)
{
    PostTask([weakThis = wptr(this), isDefaultDensityEnabled, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s isDefaultDensityEnabled: %{public}d",
            where, isDefaultDensityEnabled);
        session->isDefaultDensityEnabled_ = isDefaultDensityEnabled;
        if (session->onDefaultDensityEnabledFunc_) {
            session->onDefaultDensityEnabledFunc_(isDefaultDensityEnabled);
        }
    }, __func__);
    return WSError::WS_OK;
}

WMError SceneSession::OnUpdateColorMode(const std::string& colorMode, bool hasDarkRes)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, colorMode: %{public}s, hasDarkRes: %{public}u",
        GetPersistentId(), colorMode.c_str(), hasDarkRes);
    std::lock_guard<std::mutex> lock(colorModeMutex_);
    colorMode_ = colorMode;
    hasDarkRes_ = hasDarkRes;
    return WMError::WM_OK;
}

std::string SceneSession::GetAbilityColorMode() const
{
    std::lock_guard<std::mutex> lock(colorModeMutex_);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, colorMode: %{public}s, hasDarkRes: %{public}u",
        GetPersistentId(), colorMode_.c_str(), hasDarkRes_);
    if (colorMode_ == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK && !hasDarkRes_) {
        return AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO;
    }
    return colorMode_;
}

/** @note @Window.Layout */
WMError SceneSession::UpdateWindowModeForUITest(int32_t updateMode)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "sessionStage is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, updateMode: %{public}d", GetPersistentId(), updateMode);
    return sessionStage_->UpdateWindowModeForUITest(updateMode);
}

void SceneSession::SetForceHideState(ForceHideState forceHideState)
{
    forceHideState_ = forceHideState;
}

ForceHideState SceneSession::GetForceHideState() const
{
    return forceHideState_;
}

void SceneSession::SetIsDisplayStatusBarTemporarily(bool isTemporary)
{
    isDisplayStatusBarTemporarily_.store(isTemporary);
}

bool SceneSession::GetIsDisplayStatusBarTemporarily() const
{
    return isDisplayStatusBarTemporarily_.load();
}

void SceneSession::RetrieveStatusBarDefaultVisibility()
{
    if (specificCallback_ && specificCallback_->onGetStatusBarDefaultVisibilityByDisplayId_) {
        UpdateStatusBarVisible(
            specificCallback_->onGetStatusBarDefaultVisibilityByDisplayId_(GetSessionProperty()->GetDisplayId()));
    }
}

void SceneSession::SetIsLastFrameLayoutFinishedFunc(IsLastFrameLayoutFinishedFunc&& func)
{
    isLastFrameLayoutFinishedFunc_ = std::move(func);
}

void SceneSession::SetIsAINavigationBarAvoidAreaValidFunc(IsAINavigationBarAvoidAreaValidFunc&& func)
{
    isAINavigationBarAvoidAreaValid_ = std::move(func);
}

void SceneSession::SetStartingWindowExitAnimationFlag(bool enable)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "SetStartingWindowExitAnimationFlag %{public}d", enable);
    needStartingWindowExitAnimation_.store(enable);
}

bool SceneSession::NeedStartingWindowExitAnimation() const
{
    return needStartingWindowExitAnimation_.load();
}

bool SceneSession::IsSystemSpecificSession() const
{
    return isSystemSpecificSession_;
}

void SceneSession::SetIsSystemSpecificSession(bool isSystemSpecificSession)
{
    isSystemSpecificSession_ = isSystemSpecificSession;
}

void SceneSession::SetTemporarilyShowWhenLocked(bool isTemporarilyShowWhenLocked)
{
    if (isTemporarilyShowWhenLocked_.load() == isTemporarilyShowWhenLocked) {
        return;
    }
    isTemporarilyShowWhenLocked_.store(isTemporarilyShowWhenLocked);
    TLOGI(WmsLogTag::WMS_SCB, "SetTemporarilyShowWhenLocked successfully, target:%{public}u",
        isTemporarilyShowWhenLocked);
    if (systemConfig_.freeMultiWindowSupport_) {
        const bool isPcMode = system::GetBoolParameter("persist.sceneboard.ispcmode", false);
        const bool isShow = !(isTemporarilyShowWhenLocked && !isPcMode);
        GetSessionProperty()->SetIsShowDecorInFreeMultiWindow(isShow);
        SetIsShowDecorInFreeMultiWindow(isShow);
    }
}

bool SceneSession::IsTemporarilyShowWhenLocked() const
{
    return isTemporarilyShowWhenLocked_.load();
}

void SceneSession::SetSkipDraw(bool skip)
{
    auto shadowSurfaceNode = GetShadowSurfaceNode();
    if (!shadowSurfaceNode) {
        WLOGFE("shadowSurfaceNode is null");
        return;
    }
    {
        AutoRSTransaction trans(GetRSShadowContext());
        shadowSurfaceNode->SetSkipDraw(skip);
    }
    if (auto leashShadowSurfaceNode = GetLeashWinShadowSurfaceNode()) {
        AutoRSTransaction trans(GetRSLeashWinShadowContext());
        leashShadowSurfaceNode->SetSkipDraw(skip);
    }
}

void SceneSession::SetSkipSelfWhenShowOnVirtualScreen(bool isSkip)
{
    TLOGD(WmsLogTag::WMS_SCB, "Set skip Self, isSkip: %{public}d", isSkip);
    PostTask([weakThis = wptr(this), isSkip, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s session is null", where);
            return;
        }
        session->GetSessionProperty()->SetSkipSelfWhenShowOnVirtualScreen(isSkip);
        std::shared_ptr<RSSurfaceNode> surfaceNode = session->GetSurfaceNode();
        if (!surfaceNode) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s surfaceNode_ is null", where);
            return;
        }
        std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode = session->GetLeashWinSurfaceNode();
        if (session->specificCallback_ != nullptr &&
            session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_ != nullptr) {
            if (WindowHelper::IsPipWindow(session->GetWindowType())) {
                TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s session is pip", where);
                session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_(surfaceNode->GetId(), isSkip);
            } else {
                session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_(session->GetMissionId(), isSkip);
            }
        }
        return;
    }, __func__);
}

void SceneSession::SetSkipEventOnCastPlus(bool isSkip)
{
    TLOGD(WmsLogTag::WMS_SCB, "Set skip event on cast plus, wid: %{public}d, isSkip: %{public}d",
        GetPersistentId(), isSkip);
    PostTask([weakThis = wptr(this), isSkip, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s session is null", where);
            return;
        }
        if (session->specificCallback_ != nullptr &&
            session->specificCallback_->onSetSkipEventOnCastPlus_ != nullptr) {
            session->specificCallback_->onSetSkipEventOnCastPlus_(session->GetPersistentId(), isSkip);
        }
        return;
    }, __func__);
}

WMError SceneSession::SetUniqueDensityDpi(bool useUnique, float dpi)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "SceneSession set unique dpi: id=%{public}d, dpi=%{public}f",
        GetPersistentId(), dpi);
    if (useUnique && (dpi > DOT_PER_INCH_MAXIMUM_VALUE || dpi < DOT_PER_INCH_MINIMUM_VALUE)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Invalid input dpi value, valid input range: %{public}u ~ %{public}u",
            DOT_PER_INCH_MINIMUM_VALUE, DOT_PER_INCH_MAXIMUM_VALUE);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    float density = static_cast<float>(dpi) / 160; // 160 is the coefficient between density and dpi;
    if (!IsSessionValid()) {
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sessionStage_ is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    sessionStage_->SetUniqueVirtualPixelRatio(useUnique, density);
    return WMError::WM_OK;
}

WMError SceneSession::UpdateAnimationSpeed(float speed)
{
    if (!IsSessionValid()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Session is invalid");
        return WMError::WM_ERROR_INVALID_SESSION;
    }

    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "sessionStage_ is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    sessionStage_->UpdateAnimationSpeed(speed);
    return WMError::WM_OK;
}

WMError SceneSession::SetSystemWindowEnableDrag(bool enableDrag)
{
    if (!WindowHelper::IsSubWindow(GetWindowType()) && !WindowHelper::IsSystemWindow(GetWindowType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "id:%{public}d, not set enable drag", GetPersistentId());
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    PostTask([weakThis = wptr(this), enableDrag, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d, enableDrag:%{public}d",
            where, session->GetPersistentId(), enableDrag);
        session->GetSessionProperty()->SetDragEnabled(enableDrag);
        session->NotifySessionInfoChange();
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSession::SetWindowEnableDragBySystem(bool enableDrag)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "enableDrag: %{public}d", enableDrag);
    PostTask([weakThis = wptr(this), enableDrag, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        session->SetClientDragEnable(enableDrag);
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s id: %{public}d, enableDrag: %{public}d",
            where, session->GetPersistentId(), enableDrag);
        session->GetSessionProperty()->SetDragEnabled(enableDrag);
        if (session->sessionStage_) {
            session->sessionStage_->SetEnableDragBySystem(enableDrag);
        }
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSession::ActivateDragBySystem(bool activateDrag)
{
    PostTask([weakThis = wptr(this), activateDrag, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        session->SetDragActivated(activateDrag);
        session->NotifySessionInfoChange();
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s id: %{public}d, activate drag: %{public}d",
            where, session->GetPersistentId(), activateDrag);
        if (session->sessionStage_) {
            session->sessionStage_->SetDragActivated(activateDrag);
        }
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateWindowModeSupportType(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (!property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowModeSupportType(property->GetWindowModeSupportType());
    }
    return WMError::WM_OK;
}

void SceneSession::RegisterForceSplitListener(const NotifyForceSplitFunc& func)
{
    forceSplitFunc_ = func;
}

void SceneSession::RegisterAppHookWindowInfoFunc(GetHookWindowInfoFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, func is null", GetPersistentId());
        return;
    }
    getHookWindowInfoFunc_ = std::move(func);
}

void SceneSession::RegisterRequestedOrientationChangeCallback(NotifyReqOrientationChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onRequestedOrientationChange_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterBindDialogSessionCallback(const NotifyBindDialogSessionFunc& callback)
{
    onBindDialogTarget_ = callback;
}

void SceneSession::RegisterIsCustomAnimationPlayingCallback(NotifyIsCustomAnimationPlayingCallback&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->onIsCustomAnimationPlaying_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterLayoutFullScreenChangeCallback(NotifyLayoutFullScreenChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        session->onLayoutFullScreenChangeFunc_ = std::move(callback);
    }, __func__);
}

void SceneSession::RegisterGetStatusBarAvoidHeightFunc(GetStatusBarAvoidHeightFunc&& callback)
{
    onGetStatusBarAvoidHeightFunc_ = std::move(callback);
}

void SceneSession::RegisterGetStatusBarConstantlyShowFunc(GetStatusBarConstantlyShowFunc&& callback)
{
    onGetStatusBarConstantlyShowFunc_ = std::move(callback);
}

void SceneSession::RegisterGetIsDockAutoHideFunc(GetIsDockAutoHideFunc&& func)
{
    onGetIsDockAutoHideFunc_ = std::move(func);
}

WMError SceneSession::GetAppForceLandscapeConfig(AppForceLandscapeConfig& config)
{
    if (forceSplitFunc_ == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    config = forceSplitFunc_(sessionInfo_.bundleName_);
    return WMError::WM_OK;
}

WMError SceneSession::GetAppHookWindowInfoFromServer(HookWindowInfo& hookWindowInfo)
{
    return PostSyncTask([weakThis = wptr(this), &hookWindowInfo, where = __func__]() -> WMError {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return WMError::WM_ERROR_INVALID_SESSION;
        }
        if (!session->getHookWindowInfoFunc_) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, func is null", session->GetPersistentId());
            return WMError::WM_ERROR_NULLPTR;
        }
        hookWindowInfo = session->getHookWindowInfoFunc_(session->GetSessionInfo().bundleName_);
        return WMError::WM_OK;
    }, __func__);
}

void SceneSession::NotifyWindowStatusDidChangeAfterShowWindow()
{
    PostTask([weakSession = wptr(this), where = __func__] {
        auto session = weakSession.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
            return;
        }
        constexpr uint32_t timesToWaitForVsyncs = 2;
        session->RunAfterNVsyncs(timesToWaitForVsyncs, [weakSession, where](int64_t, int64_t) {
            auto session = weakSession.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", where);
                return;
            }

            session->ExecuteWindowStatusChangeNotification(where);
            });
    }, __func__);
}

void SceneSession::SetUpdatePrivateStateAndNotifyFunc(const UpdatePrivateStateAndNotifyFunc& func)
{
    updatePrivateStateAndNotifyFunc_ = func;
}

void SceneSession::SetNotifyScreenshotAppEventRegisteredFunc(UpdateScreenshotAppEventRegisteredFunc&& func)
{
    updateScreenshotAppEventRegisteredFunc_ = std::move(func);
}

void SceneSession::SetNotifyVisibleChangeFunc(const NotifyVisibleChangeFunc& func)
{
    notifyVisibleChangeFunc_ = func;
}

void SceneSession::SetPrivacyModeChangeNotifyFunc(NotifyPrivacyModeChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s session is null", where);
            return;
        }
        session->privacyModeChangeNotifyFunc_ = std::move(func);
    }, __func__);
}

void SceneSession::SetHighlightChangeNotifyFunc(const NotifyHighlightChangeFunc& func)
{
    std::lock_guard lock(highlightChangeFuncMutex_);
    highlightChangeFunc_ = func;
}

void SceneSession::RegisterNotifySurfaceBoundsChangeFunc(int32_t sessionId, NotifySurfaceBoundsChangeFunc&& func)
{
    if (!func) {
        TLOGE(WmsLogTag::WMS_SUB, "func is null");
        return;
    }
    std::lock_guard lock(registerNotifySurfaceBoundsChangeMutex_);
    notifySurfaceBoundsChangeFuncMap_[sessionId] = func;
    NotifyFollowedParentWindowAcrossDisplaysChange(IsFullScreenWaterfallMode());
}

void SceneSession::UnregisterNotifySurfaceBoundsChangeFunc(int32_t sessionId)
{
    std::lock_guard lock(registerNotifySurfaceBoundsChangeMutex_);
    notifySurfaceBoundsChangeFuncMap_.erase(sessionId);
}

sptr<SceneSession> SceneSession::GetSceneSessionById(int32_t sessionId) const
{
    if (specificCallback_ == nullptr || specificCallback_->onGetSceneSessionByIdCallback_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "specificCallback or onGetSceneSessionByIdCallback is null");
        return nullptr;
    }
    return specificCallback_->onGetSceneSessionByIdCallback_(sessionId);
}

bool SceneSession::CheckAndGetAbilityInfoByWant(const std::shared_ptr<AAFwk::Want>& want,
    AppExecFwk::AbilityInfo& abilityInfo) const
{
    if (specificCallback_ == nullptr || specificCallback_->onCheckAndGetAbilityInfoByWantCallback_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "specificCallback or onCheckAndGetAbilityInfoByWantCallback is null");
        return false;
    }
    return specificCallback_->onCheckAndGetAbilityInfoByWantCallback_(want, abilityInfo);
}


void SceneSession::SetWindowAnchorInfoChangeFunc(NotifyWindowAnchorInfoChangeFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_SUB, "func is null");
        return;
    }
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is null", where);
            return;
        }
        func(session->windowAnchorInfo_);
        session->onWindowAnchorInfoChangeFunc_ = std::move(func);
    });
}

WSError SceneSession::SetWindowAnchorInfo(const WindowAnchorInfo& windowAnchorInfo)
{
    auto property = GetSessionProperty();
    if (!property || property->GetSubWindowLevel() > 1) {
        TLOGE(WmsLogTag::WMS_SUB, "property is null or not surppot more than 1 level window");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (!WindowHelper::IsSubWindow(property->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SUB, "only sub window is valid");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (GetIsFollowParentLayout()) {
        TLOGE(WmsLogTag::WMS_SUB, "current sub window follow parent layout");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (windowAnchorInfo.windowAnchor_ > WindowAnchor::BOTTOM_END ||
        windowAnchorInfo.windowAnchor_ < WindowAnchor::TOP_START) {
        TLOGE(WmsLogTag::WMS_SUB, "Unknown window anchor");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (!systemConfig_.supportFollowRelativePositionToParent_) {
        TLOGI(WmsLogTag::WMS_SUB, "not support device");
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }
    PostTask([weakThis = wptr(this), windowAnchorInfo, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is null", where);
            return;
        }
        session->windowAnchorInfo_ = windowAnchorInfo;
        if (session->onWindowAnchorInfoChangeFunc_) {
            session->onWindowAnchorInfoChangeFunc_(windowAnchorInfo);
        } else {
            TLOGI(WmsLogTag::WMS_SUB, "func is null");
        }
    });
    return WSError::WS_OK;
}

void SceneSession::CalcSubWindowRectByAnchor(const WSRect& parentRect, WSRect& subRect)
{
    if (SessionHelper::IsEmptyRect(parentRect) || SessionHelper::IsEmptyRect(subRect)) {
        TLOGE(WmsLogTag::WMS_SUB, "parent or sub window rect is invalid");
        return;
    }
    if (!windowAnchorInfo_.isAnchorEnabled_) {
        TLOGI(WmsLogTag::WMS_SUB, "sub window anchor disabled");
        return;
    }

    WindowAnchor anchorMode = windowAnchorInfo_.windowAnchor_;
    int32_t offsetX = windowAnchorInfo_.offsetX_;
    int32_t offsetY = windowAnchorInfo_.offsetY_;
    switch (anchorMode) {
        case WindowAnchor::TOP_START:
            subRect.posX_ = parentRect.posX_;
            subRect.posY_ = parentRect.posY_;
            break;
        case WindowAnchor::TOP:
            subRect.posX_ = parentRect.posX_ + (parentRect.width_ - subRect.width_) / HALF_VALUE;
            subRect.posY_ = parentRect.posY_;
            break;
        case WindowAnchor::TOP_END:
            subRect.posX_ = parentRect.posX_ + parentRect.width_ - subRect.width_;
            subRect.posY_ = parentRect.posY_;
            break;
        case WindowAnchor::START:
            subRect.posX_ = parentRect.posX_;
            subRect.posY_ = parentRect.posY_ + (parentRect.height_ - subRect.height_) / HALF_VALUE;
            break;
        case WindowAnchor::CENTER:
            subRect.posX_ = parentRect.posX_ + (parentRect.width_ - subRect.width_) / HALF_VALUE;
            subRect.posY_ = parentRect.posY_ + (parentRect.height_ - subRect.height_) / HALF_VALUE;
            break;
        case WindowAnchor::END:
            subRect.posX_ = parentRect.posX_ + parentRect.width_ - subRect.width_;
            subRect.posY_ = parentRect.posY_ + (parentRect.height_ - subRect.height_) / HALF_VALUE;
            break;
        case WindowAnchor::BOTTOM_START:
            subRect.posX_ = parentRect.posX_;
            subRect.posY_ = parentRect.posY_ + parentRect.height_ - subRect.height_;
            break;
        case WindowAnchor::BOTTOM:
            subRect.posX_ = parentRect.posX_ + (parentRect.width_ - subRect.width_) / HALF_VALUE;
            subRect.posY_ = parentRect.posY_ + parentRect.height_ - subRect.height_;
            break;
        case WindowAnchor::BOTTOM_END:
            subRect.posX_ = parentRect.posX_ + parentRect.width_ - subRect.width_;
            subRect.posY_ = parentRect.posY_ + parentRect.height_ - subRect.height_;
            break;
        default:
            TLOGE(WmsLogTag::WMS_SUB, "invalid anchor mode");
            return;
    }
    subRect.posX_ += offsetX;
    subRect.posY_ += offsetY;
    TLOGI(WmsLogTag::WMS_SUB, "win %{public}d, anchorMode %{public}u, rect %{public}s",
        GetPersistentId(), anchorMode, subRect.ToString().c_str());
}

void SceneSession::SetFollowParentRectFunc(NotifyFollowParentRectFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_SUB, "func is null");
        return;
    }
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is null", where);
            return;
        }
        func(session->isFollowParentLayout_);
        session->followParentRectFunc_ = std::move(func);
    });
}

WSError SceneSession::SetFollowParentWindowLayoutEnabled(bool isFollow)
{
    auto property = GetSessionProperty();
    if (!property || property->GetSubWindowLevel() > 1) {
        TLOGE(WmsLogTag::WMS_SUB, "property is null or not surppot more than 1 level window");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (!WindowHelper::IsSubWindow(property->GetWindowType()) &&
        !WindowHelper::IsDialogWindow(property->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SUB, "only sub window and dialog is valid");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (GetWindowAnchorInfo().isAnchorEnabled_) {
        TLOGE(WmsLogTag::WMS_SUB, "current sub window follow parent anchor");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (!systemConfig_.supportFollowParentWindowLayout_) {
        TLOGI(WmsLogTag::WMS_SUB, "not support device");
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }
    PostTask([weakThis = wptr(this), isFollow = isFollow,  where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is null", where);
            return;
        }
        session->isFollowParentLayout_ = isFollow;
        if (session->followParentRectFunc_) {
            session->followParentRectFunc_(isFollow);
        } else {
            TLOGI(WmsLogTag::WMS_SUB, "func is null");
        }
        // if parent is null, don't need follow move drag
        if (!session->parentSession_) {
            TLOGW(WmsLogTag::WMS_SUB, "parent is null");
            return;
        }
        if (!isFollow) {
            session->parentSession_->UnregisterNotifySurfaceBoundsChangeFunc(session->GetPersistentId());
            return;
        }
        auto task = [weak = weakThis](const WSRect& rect, bool isGlobal, bool needFlush) {
            auto session = weak.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_SUB, "session has been destroy");
                return;
            }
            session->SetSurfaceBounds(rect, isGlobal, needFlush);
        };
        session->parentSession_->RegisterNotifySurfaceBoundsChangeFunc(session->GetPersistentId(), std::move(task));
        if (auto parentSession = session->GetSceneSessionById(session->GetMainSessionId())) {
            session->NotifySessionAcrossDisplaysChange(session, parentSession->IsFullScreenWaterfallMode());
        }
    });
    return WSError::WS_OK;
}

int32_t SceneSession::GetStatusBarHeight()
{
    return PostSyncTask([weakThis = wptr(this), where = __func__]() -> int32_t {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", where);
            return 0;
        }
        int32_t height = 0;
        if (session->specificCallback_ == nullptr ||
            session->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s specificCallback_ or session property is null", where);
            return height;
        }
        const auto& statusBarVector = session->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_(
            WindowType::WINDOW_TYPE_STATUS_BAR, session->GetSessionProperty()->GetDisplayId());
        DisplayId displayId = session->GetSessionProperty()->GetDisplayId();
        for (auto& statusBar : statusBarVector) {
            if (statusBar == nullptr) {
                continue;
            }
            WSRect statusBarRect = statusBar->GetSessionRect();
            if (session->onGetStatusBarAvoidHeightFunc_) {
                session->onGetStatusBarAvoidHeightFunc_(displayId, statusBarRect);
            }
            height = statusBarRect.height_;
        }
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s win %{public}d displayId %{public}" PRIu64 " height %{public}d",
            where, session->GetPersistentId(), displayId, height);
        return height;
    }, __func__);
}

int32_t SceneSession::GetDockHeight()
{
    int32_t height = 0;
    if (specificCallback_ == nullptr || specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ == nullptr ||
        GetSessionProperty() == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "specificCallback_ or session property is null");
        return height;
    }
    const auto& dockVector = specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_(
        WindowType::WINDOW_TYPE_LAUNCHER_DOCK, GetSessionProperty()->GetDisplayId());
    for (auto& dock : dockVector) {
        if (dock != nullptr && dock->IsVisible() && dock->GetWindowName().find("SCBSmartDock") != std::string::npos) {
            int32_t dockHeight = dock->GetSessionRect().height_;
            if (dockHeight > height) {
                height = dockHeight;
            }
        }
    }
    return height;
}

bool SceneSession::CheckPermissionWithPropertyAnimation(const sptr<WindowSessionProperty>& property) const
{
    if (property && property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
        if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
            TLOGE(WmsLogTag::WMS_LIFE, "Not system app, no permission");
            return false;
        }
    }
    return true;
}

void SceneSession::UpdatePCZOrderAndMarkDirty(const uint32_t zOrder)
{
    dirtyFlags_ |= UpdateZOrderInner(zOrder) ? static_cast<uint32_t>(SessionUIDirtyFlag::Z_ORDER) : 0;
}

uint32_t SceneSession::UpdateUIParam(const SessionUIParam& uiParam)
{
    bool lastVisible = IsVisible();
    dirtyFlags_ |= UpdateInteractiveInner(uiParam.interactive_) ?
        static_cast<uint32_t>(SessionUIDirtyFlag::INTERACTIVE) : 0;
    if (!uiParam.interactive_) {
        // keep ui state in recent
        return dirtyFlags_;
    }
    dirtyFlags_ |= UpdateVisibilityInner(true) ? static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE) : 0;
    bool isUpdateRectDirty = UpdateRectInner(uiParam, GetSizeChangeReason());
    if (isUpdateRectDirty) {
        dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
        AddPropertyDirtyFlags(static_cast<uint32_t>(SessionPropertyFlag::WINDOW_RECT));
    }
    dirtyFlags_ |= UpdateScaleInner(uiParam.scaleX_, uiParam.scaleY_, uiParam.pivotX_, uiParam.pivotY_) ?
        static_cast<uint32_t>(SessionUIDirtyFlag::SCALE) : 0;
    if (dirtyFlags_ & static_cast<uint32_t>(SessionUIDirtyFlag::GLOBAL_RECT) ||
        dirtyFlags_ & static_cast<uint32_t>(SessionUIDirtyFlag::SCALE)) {
        NotifyGlobalScaledRectChange();
    }
    if (!isPcScenePanel_) {
        dirtyFlags_ |= UpdateZOrderInner(uiParam.zOrder_) ? static_cast<uint32_t>(SessionUIDirtyFlag::Z_ORDER) : 0;
    }
    if (!lastVisible && IsVisible() && !isFocused_ && !postProcessFocusState_.enabled_ &&
        GetForegroundInteractiveStatus()) {
        postProcessFocusState_.enabled_ = true;
        postProcessFocusState_.isFocused_ = true;
        postProcessFocusState_.reason_ = isStarting_ ?
            FocusChangeReason::SCB_START_APP : FocusChangeReason::FOREGROUND;
    }
    return dirtyFlags_;
}

uint32_t SceneSession::UpdateUIParam()
{
    bool lastVisible = IsVisible();
    dirtyFlags_ |= UpdateVisibilityInner(false) ? static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE) : 0;
    if (lastVisible && !IsVisible() && isFocused_) {
        postProcessFocusState_.enabled_ = true;
        postProcessFocusState_.isFocused_ = false;
        postProcessFocusState_.reason_ = FocusChangeReason::BACKGROUND;
    }
    return dirtyFlags_;
}

bool SceneSession::UpdateVisibilityInner(bool visibility)
{
    if (isVisible_ == visibility) {
        return false;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d, visibility: %{public}u -> %{public}u",
        GetPersistentId(), isVisible_, visibility);
    if (visibilityChangedDetectFunc_) {
        visibilityChangedDetectFunc_(GetCallingPid(), isVisible_, visibility);
    }
    isVisible_ = visibility;
    if (updatePrivateStateAndNotifyFunc_ != nullptr) {
        updatePrivateStateAndNotifyFunc_(GetPersistentId());
    }
    if (notifyVisibleChangeFunc_ != nullptr) {
        notifyVisibleChangeFunc_(GetPersistentId());
    }
    SceneSessionChangeInfo changeInfo {
        .persistentId_ = GetPersistentId(),
        .changeInfo_ = "Visibility change to " + std::to_string(visibility),
        .logTag_ = WmsLogTag::WMS_ATTRIBUTE,
    };
    SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::VISIBLE_RECORD, changeInfo);
    return true;
}

bool SceneSession::UpdateInteractiveInner(bool interactive)
{
    if (GetForegroundInteractiveStatus() == interactive) {
        return false;
    }
    NotifyAddOrRemoveSnapshotWindow(interactive && IsSessionForeground());
    SetForegroundInteractiveStatus(interactive);
    NotifyClientToUpdateInteractive(interactive);
    return true;
}

void SceneSession::NotifyAddOrRemoveSnapshotWindow(bool interactive)
{
    // persistent imageFit exist, add snapshot when interactive is false.
    if (isPersistentImageFit_.load()) {
        TLOGI(WmsLogTag::WMS_PATTERN, "Add or remove static image from window, interactive:%{public}d", interactive);
        PostTask([weakThis = wptr(this), interactive, where = __func__] {
            auto session = weakThis.promote();
            if (session == nullptr) {
                TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s session is null", where);
                return;
            }
            interactive ? session->NotifyRemoveSnapshot() : session->NotifyAddSnapshot(false, false, false);
        }, __func__);
    }
}

void SceneSession::UpdateLifecyclePausedInner()
{
    if (!sessionStage_) {
        return;
    }
    const auto state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "state: %{public}d", state);
    if (state == SessionState::STATE_ACTIVE || state == SessionState::STATE_FOREGROUND) {
        sessionStage_->NotifyLifecyclePausedStatus();
    }
}

bool SceneSession::PipelineNeedNotifyClientToUpdateRect() const
{
    return IsVisibleForeground() && GetForegroundInteractiveStatus();
}

bool SceneSession::UpdateRectInner(const SessionUIParam& uiParam, SizeChangeReason reason)
{
    if (reason == SizeChangeReason::PAGE_ROTATION) {
        dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    }
    if (reason == SizeChangeReason::DRAG_END) {
        SetSessionInfoCursorDragFlag(false);
        SetSessionInfoCursorDragCount(0);
    }
    // During the drag move, prohibit vSync from refreshing UI parameters to the server
    if (reason == SizeChangeReason::DRAG_MOVE ||
        !((NotifyServerToUpdateRect(uiParam, reason) || IsDirtyWindow()) && PipelineNeedNotifyClientToUpdateRect())) {
        return false;
    }
    if (WindowHelper::IsSubWindow(GetWindowType())) {
        isSubWindowResizingOrMoving_ = false;
    }
    NotifyClientToUpdateRect("WMSPipeline", RSSyncTransactionAdapter::GetRSTransaction(GetRSUIContext()));
    return true;
}

bool SceneSession::NotifyServerToUpdateRect(const SessionUIParam& uiParam, SizeChangeReason reason)
{
    if (!GetForegroundInteractiveStatus()) {
        TLOGD(WmsLogTag::WMS_PIPELINE, "skip recent, id:%{public}d", GetPersistentId());
        return false;
    }
    if (GetKeyFramePolicy().running_) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "skip for key frame running, id:%{public}d", GetPersistentId());
        return false;
    }
    if (uiParam.rect_.IsInvalid()) {
        TLOGW(WmsLogTag::WMS_PIPELINE, "id:%{public}d rect:%{public}s is invalid",
            GetPersistentId(), uiParam.rect_.ToString().c_str());
        return false;
    }
    auto globalRect = GetSessionGlobalRect();
    SetSessionGlobalRect(uiParam.rect_);
    if (globalRect != uiParam.rect_) {
        UpdateAllModalUIExtensions(uiParam.rect_);
    }
    if (!uiParam.needSync_ || !isNeedSyncSessionRect_) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, scenePanelNeedSync:%{public}u needSyncSessionRect:%{public}u "
            "rectAfter:%{public}s preRect:%{public}s preGlobalRect:%{public}s", GetPersistentId(), uiParam.needSync_,
            isNeedSyncSessionRect_, uiParam.rect_.ToString().c_str(), GetSessionRect().ToString().c_str(),
            globalRect.ToString().c_str());
        return false;
    }
    WSRect rect = { uiParam.rect_.posX_ - uiParam.transX_, uiParam.rect_.posY_ - uiParam.transY_,
        uiParam.rect_.width_, uiParam.rect_.height_ };
    if (GetSessionRect() == rect && (!sessionStage_ || GetClientRect() == rect) &&
        reason != SizeChangeReason::SPLIT_DRAG_END) {
        TLOGD(WmsLogTag::WMS_PIPELINE, "skip same rect update id:%{public}d rect:%{public}s preGlobalRect:%{public}s!",
            GetPersistentId(), rect.ToString().c_str(), globalRect.ToString().c_str());
        return false;
    }
    if (rect.IsInvalid()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "id:%{public}d rect:%{public}s is invalid, preGlobalRect:%{public}s",
            GetPersistentId(), rect.ToString().c_str(), globalRect.ToString().c_str());
        return false;
    }
    TLOGI_LMTBYID(TEN_SECONDS, RECORD_100_TIMES, GetPersistentId(), WmsLogTag::WMS_LAYOUT,
        "id:%{public}d, rect:%{public}s->%{public}s global=%{public}s client=%{public}s",
        GetPersistentId(), GetSessionRect().ToString().c_str(), rect.ToString().c_str(),
        globalRect.ToString().c_str(), GetClientRect().ToString().c_str());
    layoutController_->SetSessionRect(rect);
    RectCheckProcess();
    return true;
}

void SceneSession::PostProcessNotifyAvoidArea()
{
    if (PipelineNeedNotifyClientToUpdateAvoidArea(dirtyFlags_)) {
        NotifyClientToUpdateAvoidArea();
    }
}

bool SceneSession::PipelineNeedNotifyClientToUpdateAvoidArea(uint32_t dirty) const
{
    return ((dirty & static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE)) && IsImmersiveType()) ||
        ((dirty & static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA)) && isVisible_);
}

void SceneSession::NotifyClientToUpdateAvoidArea()
{
    if (specificCallback_ == nullptr) {
        return;
    }
    // flush avoid areas on (avoid area dirty && (normal session rect NOT dirty || avoid session))
    if ((IsImmersiveType() || !IsDirtyWindow()) && specificCallback_->onUpdateAvoidArea_) {
        specificCallback_->onUpdateAvoidArea_(GetPersistentId());
    }
}

bool SceneSession::UpdateScaleInner(float scaleX, float scaleY, float pivotX, float pivotY)
{
    if (!layoutController_->IsTransformNeedUpdate(scaleX, scaleY, pivotX, pivotY)) {
        return false;
    }
    Session::SetScale(scaleX, scaleY, pivotX, pivotY);
    if (!IsSessionForeground()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, session is not foreground!", GetPersistentId());
        return false;
    }
    if (sessionStage_ != nullptr) {
        Transform transform;
        transform.scaleX_ = scaleX;
        transform.scaleY_ = scaleY;
        transform.pivotX_ = pivotX;
        transform.pivotY_ = pivotY;
        sessionStage_->NotifyTransformChange(transform);
        Session::SetClientScale(scaleX, scaleY, pivotX, pivotY);
    } else {
        WLOGFE("sessionStage is nullptr");
    }
    HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END);
    return true;
}

bool SceneSession::UpdateZOrderInner(uint32_t zOrder)
{
    if (zOrder_ == zOrder) {
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "id: %{public}d, zOrder: %{public}u -> %{public}u, lastZOrder: %{public}u",
          GetPersistentId(), zOrder_, zOrder, lastZOrder_);
    lastZOrder_ = zOrder_;
    zOrder_ = zOrder;
    return true;
}

void SceneSession::SetPostProcessFocusState(PostProcessFocusState state)
{
    postProcessFocusState_ = state;
}

PostProcessFocusState SceneSession::GetPostProcessFocusState() const
{
    return postProcessFocusState_;
}

void SceneSession::ResetPostProcessFocusState()
{
    postProcessFocusState_.Reset();
}

void SceneSession::SetPostProcessProperty(bool state)
{
    postProcessProperty_ = state;
}

bool SceneSession::GetPostProcessProperty() const
{
    return postProcessProperty_;
}

bool SceneSession::IsImmersiveType() const
{
    WindowType type = GetWindowType();
    return type == WindowType::WINDOW_TYPE_STATUS_BAR ||
        type == WindowType::WINDOW_TYPE_NAVIGATION_BAR ||
        type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
}

bool SceneSession::IsPcOrPadEnableActivation() const
{
    auto property = GetSessionProperty();
    bool isPcAppInLargeScreenDevice = false;
    if (property != nullptr) {
        isPcAppInLargeScreenDevice = property->GetIsPcAppInPad();
    }
    return systemConfig_.IsPcWindow() || IsFreeMultiWindowMode() || isPcAppInLargeScreenDevice;
}

void SceneSession::SetMinimizedFlagByUserSwitch(bool isMinimized)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "winId: %{public}d, isMinimized: %{public}d", GetPersistentId(), isMinimized);
    isMinimizedByUserSwitch_ = isMinimized;
}

bool SceneSession::IsMinimizedByUserSwitch() const
{
    return isMinimizedByUserSwitch_;
}

void SceneSession::UnregisterSessionChangeListeners()
{
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->Session::UnregisterSessionChangeListeners();
    }, __func__);
}

void SceneSession::SetVisibilityChangedDetectFunc(VisibilityChangedDetectFunc&& func)
{
    LOCK_GUARD_EXPR(SCENE_GUARD, visibilityChangedDetectFunc_ = std::move(func));
}

void SceneSession::SetDefaultDisplayIdIfNeed()
{
    if (sessionInfo_.screenId_ == SCREEN_ID_INVALID) {
        auto defaultDisplayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
        sessionInfo_.screenId_ = defaultDisplayId;
        TLOGI(WmsLogTag::WMS_LIFE, "winId: %{public}d, update screen id %{public}" PRIu64,
            GetPersistentId(), defaultDisplayId);
        auto sessionProperty = GetSessionProperty();
        if (sessionProperty) {
            sessionProperty->SetDisplayId(defaultDisplayId);
        }
    }
}

int32_t SceneSession::GetCustomDecorHeight() const
{
    std::lock_guard lock(customDecorHeightMutex_);
    return customDecorHeight_;
}

void SceneSession::SetCustomDecorHeight(int32_t height)
{
    constexpr int32_t MIN_DECOR_HEIGHT = 37;
    constexpr int32_t MAX_DECOR_HEIGHT = 112;
    if (height < MIN_DECOR_HEIGHT || height > MAX_DECOR_HEIGHT) {
        return;
    }
    std::lock_guard lock(customDecorHeightMutex_);
    customDecorHeight_ = height;
}

WSError SceneSession::SetDecorVisible(bool isVisible)
{
    std::lock_guard lock(customDecorHeightMutex_);
    isDecorVisible_ = isVisible;
    return WSError::WS_OK;
}

bool SceneSession::IsDecorVisible() const
{
    std::lock_guard lock(customDecorHeightMutex_);
    return isDecorVisible_;
}

WindowDecoration SceneSession::GetWindowDecoration() const
{
    auto getTopDecorInPx = [&, where = __func__]() -> uint32_t {
        if (!IsDecorVisible() || !IsDecorEnable()) {
            TLOGW(WmsLogTag::WMS_DECOR, "%{public}s: decor not visible or not enable", where);
            return 0;
        }
        auto displayId = GetDisplayId();
        auto display = DisplayManager::GetInstance().GetDisplayById(displayId);
        if (!display) {
            TLOGW(WmsLogTag::WMS_DECOR, "%{public}s: display is null, displayId: %{public}" PRIu64, where, displayId);
            return 0;
        }
        float vpr = display->GetVirtualPixelRatio();
        constexpr int32_t defaultTopDecorHeightVp = 37;
        std::lock_guard lock(customDecorHeightMutex_);
        auto decorHeightVp = customDecorHeight_ != 0 ? customDecorHeight_ : defaultTopDecorHeightVp;
        TLOGD(WmsLogTag::WMS_DECOR, "%{public}s: decorHeight: %{public}d, vpr: %{public}f", where, decorHeightVp, vpr);
        return static_cast<uint32_t>(std::round(decorHeightVp * vpr));
    };
    // Only the top decoration (title bar) currently has height. Left, right, and bottom are set to 0 by default.
    // If future specifications introduce additional decorations, this return value should be updated accordingly.
    WindowDecoration decoration{0, getTopDecorInPx(), 0, 0};
    TLOGD(WmsLogTag::WMS_DECOR, "decoration: %{public}s", decoration.ToString().c_str());
    return decoration;
}

void SceneSession::UpdateGestureBackEnabled()
{
    if (specificCallback_ != nullptr &&
        specificCallback_->onUpdateGestureBackEnabled_ != nullptr) {
        specificCallback_->onUpdateGestureBackEnabled_(GetPersistentId());
    }
}

bool SceneSession::CheckIdentityTokenIfMatched(const std::string& identityToken)
{
    if (!identityToken.empty() && !clientIdentityToken_.empty() && identityToken != clientIdentityToken_) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "failed, clientIdentityToken: %{public}s, identityToken: %{public}s, bundleName: %{public}s",
            clientIdentityToken_.c_str(), identityToken.c_str(), GetSessionInfo().bundleName_.c_str());
        return false;
    }
    return true;
}

bool SceneSession::CheckPidIfMatched()
{
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (callingPid != -1 && callingPid != GetCallingPid()) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "failed, callingPid_: %{public}d, callingPid: %{public}d, bundleName: %{public}s",
            GetCallingPid(), callingPid, GetSessionInfo().bundleName_.c_str());
        return false;
    }
    return true;
}

void SceneSession::SetNeedSyncSessionRect(bool needSync)
{
    PostTask([weakThis = wptr(this), needSync, where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_PIPELINE, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_PIPELINE,
            "%{public}s: change applyToSubWindow from %{public}d to %{public}d, id:%{public}d",
            where, session->isNeedSyncSessionRect_, needSync, session->GetPersistentId());
        session->isNeedSyncSessionRect_ = needSync;
    }, __func__);
}

bool SceneSession::SetFrameGravity(Gravity gravity)
{
    auto surfaceNode = GetSurfaceNode();
    if (surfaceNode == nullptr) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "fail id:%{public}d gravity:%{public}d", GetPersistentId(), gravity);
        return false;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d gravity:%{public}d", GetPersistentId(), gravity);
    surfaceNode->SetFrameGravity(gravity);
    return true;
}

void SceneSession::SetWindowRectAutoSaveCallback(NotifySetWindowRectAutoSaveFunc&& func)
{
    PostTask([weakThis = wptr(this), where = __func__, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s session or onSetWindowRectAutoSaveFunc is null", where);
            return;
        }
        session->onSetWindowRectAutoSaveFunc_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d", where,
            session->GetPersistentId());
    }, __func__);
}

void SceneSession::SetIsSystemKeyboard(bool isSystemKeyboard)
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "property is nullptr");
        return;
    }
    sessionProperty->SetIsSystemKeyboard(isSystemKeyboard);
}

bool SceneSession::IsSystemKeyboard() const
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "property is nullptr");
        return false;
    }
    return sessionProperty->IsSystemKeyboard();
}

void SceneSession::RegisterSupportWindowModesCallback(NotifySetSupportedWindowModesFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session or func is null", where);
            return;
        }
        session->onSetSupportedWindowModesFunc_ = std::move(func);
        TLOGND(WmsLogTag::WMS_LAYOUT_PC, "%{public}s id: %{public}d", where, session->GetPersistentId());
    }, __func__);
}

void SceneSession::ActivateKeyboardAvoidArea(bool active, bool recalculateAvoid)
{
    if (recalculateAvoid && !active) {
        RestoreCallingSession(GetCallingSessionId(), nullptr);
    }
    keyboardAvoidAreaActive_ = active;
    if (recalculateAvoid && active) {
        EnableCallingSessionAvoidArea();
    }
}

bool SceneSession::IsKeyboardAvoidAreaActive() const
{
    return keyboardAvoidAreaActive_;
}

void SceneSession::MarkAvoidAreaAsDirty()
{
    dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
}

void SceneSession::SetMousePointerDownEventStatus(bool mousePointerDownEventStatus)
{
    isMousePointerDownEventStatus_ = mousePointerDownEventStatus;
}

bool SceneSession::GetMousePointerDownEventStatus() const
{
    return isMousePointerDownEventStatus_;
}

void SceneSession::SetFingerPointerDownStatus(int32_t fingerId)
{
    if (fingerPointerDownStatusList_.find(fingerId) != fingerPointerDownStatusList_.end()) {
        TLOGE(WmsLogTag::WMS_EVENT, "wid:%{public}d fingerId:%{public}d receive twice down event",
              GetPersistentId(), fingerId);
    } else {
        fingerPointerDownStatusList_.insert(fingerId);
    }
}

void SceneSession::RemoveFingerPointerDownStatus(int32_t fingerId)
{
    if (fingerPointerDownStatusList_.find(fingerId) == fingerPointerDownStatusList_.end()) {
        TLOGE(WmsLogTag::WMS_EVENT, "wid:%{public}d fingerId:%{public}d receive up without down event",
              GetPersistentId(), fingerId);
    } else {
        fingerPointerDownStatusList_.erase(fingerId);
    }
}

std::unordered_set<int32_t> SceneSession::GetFingerPointerDownStatusList() const
{
    return fingerPointerDownStatusList_;
}

void SceneSession::UpdateAllModalUIExtensions(const WSRect& globalRect)
{
    if (modalUIExtensionInfoList_.empty()) {
        return;
    }
    PostTask([weakThis = wptr(this), where = __func__, globalRect] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s session is null", where);
            return;
        }
        auto parentTransX = globalRect.posX_ - session->GetClientRect().posX_;
        auto parentTransY = globalRect.posY_ - session->GetClientRect().posY_;
        {
            std::unique_lock<std::shared_mutex> lock(session->modalUIExtensionInfoListMutex_);
            for (auto& extensionInfo : session->modalUIExtensionInfoList_) {
                if (!extensionInfo.hasUpdatedRect) {
                    continue;
                }
                extensionInfo.windowRect = extensionInfo.uiExtRect;
                extensionInfo.windowRect.posX_ += parentTransX;
                extensionInfo.windowRect.posY_ += parentTransY;
                WSRect transRect = { extensionInfo.windowRect.posX_, extensionInfo.windowRect.posY_,
                    extensionInfo.windowRect.width_, extensionInfo.windowRect.height_ };
                session->TransformRelativeRectToGlobalRect(transRect);
                extensionInfo.windowRect.posY_ = transRect.posY_;
            }
        }
        session->NotifySessionInfoChange();
        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: id: %{public}d, globalRect: %{public}s, parentTransX: %{public}d, "
            "parentTransY: %{public}d", where, session->GetPersistentId(), globalRect.ToString().c_str(),
            parentTransX, parentTransY);
    }, __func__);
}

void SceneSession::SetWindowCornerRadiusCallback(NotifySetWindowCornerRadiusFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), where, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "session or onSetWindowCornerRadiusFunc is null");
            return;
        }
        session->onSetWindowCornerRadiusFunc_ = std::move(func);
        auto property = session->GetSessionProperty();
        float cornerRadius = property ? property->GetWindowCornerRadius() : WINDOW_CORNER_RADIUS_INVALID;
        if (!MathHelper::LessNotEqual(cornerRadius, 0.0f)) {
            // Valid corner radius menas app has set corner radius of the window, need to update to scb.
            session->onSetWindowCornerRadiusFunc_(cornerRadius);
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s id: %{public}d, corner radius: %{public}f", where,
            session->GetPersistentId(), cornerRadius);
    }, __func__);
}

WSError SceneSession::SetWindowCornerRadius(float cornerRadius)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), cornerRadius, where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "session is null");
            return;
        }
        if (session->onSetWindowCornerRadiusFunc_) {
            TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s id %{public}d radius: %{public}f",
                where, session->GetPersistentId(), cornerRadius);
            session->onSetWindowCornerRadiusFunc_(cornerRadius);
        }
        session->GetSessionProperty()->SetWindowCornerRadius(cornerRadius);
    }, __func__);
    return WSError::WS_OK;
}

void SceneSession::SetWindowShadowsCallback(NotifySetWindowShadowsFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), where, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "session or onSetWindowShadowsFunc is null");
            return;
        }
        session->onSetWindowShadowsFunc_ = std::move(func);
        auto property = session->GetSessionProperty();
        session->onSetWindowShadowsFunc_(property->GetWindowShadows());
        TLOGNI(WmsLogTag::WMS_ANIMATION, "%{public}s id: %{public}d", where,
            session->GetPersistentId());
    }, __func__);
}

WSError SceneSession::SetWindowShadows(const ShadowsInfo& shadowsInfo)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        WLOGFE("RaiseAboveTarget permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), shadowsInfo, where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "session is null");
            return;
        }
        if (session->onSetWindowShadowsFunc_) {
            TLOGND(WmsLogTag::WMS_ANIMATION, "%{public}s id %{public}d shadow radius: %{public}f, "
                "color: %{public}s, offsetX: %{public}f, offsetY: %{public}f", where, session->GetPersistentId(),
                shadowsInfo.radius_, shadowsInfo.color_.c_str(), shadowsInfo.offsetX_, shadowsInfo.offsetY_);
            session->onSetWindowShadowsFunc_(shadowsInfo);
        }
        session->GetSessionProperty()->SetWindowShadows(shadowsInfo);
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::GetTopNavDestinationName(std::string& topNavDestName)
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session stage is null: win=%{public}d", GetWindowId());
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->GetTopNavDestinationName(topNavDestName);
}

void SceneSession::UpdateSubWindowLevel(uint32_t subWindowLevel)
{
    GetSessionProperty()->SetSubWindowLevel(subWindowLevel);
    for (const auto& session : GetSubSession()) {
        if (session != nullptr) {
            session->UpdateSubWindowLevel(subWindowLevel + 1);
        }
    }
}

uint32_t SceneSession::GetMaxSubWindowLevel() const
{
    uint32_t maxSubWindowLevel = 1;
    for (const auto& session : GetSubSession()) {
        if (session != nullptr) {
            maxSubWindowLevel = std::max(maxSubWindowLevel, session->GetMaxSubWindowLevel() + 1);
        }
    }
    return maxSubWindowLevel;
}

void SceneSession::SetColorSpace(ColorSpace colorSpace)
{
    auto surfaceNode = GetShadowSurfaceNode();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "surfaceNode is invalid");
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "SetColorSpace value=%{public}u", colorSpace);
    auto colorGamut = GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    if (colorSpace == ColorSpace::COLOR_SPACE_WIDE_GAMUT) {
        colorGamut = GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3;
    }
    {
        AutoRSTransaction trans(GetRSShadowContext());
        surfaceNode->SetColorSpace(colorGamut);
    }
}

void SceneSession::AddSidebarBlur()
{
    TLOGI(WmsLogTag::WMS_PC,"in");
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_PC, "sessionStage is null");
        return;
    }
    sessionStage_->AddSidebarBlur();
}

void SceneSession::SetSessionGetTargetOrientationConfigInfoCallback(
    const NotifySessionGetTargetOrientationConfigInfoFunc& func)
{
    PostTask(
        [weakThis = wptr(this), func, where = __func__] {
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
                return WSError::WS_ERROR_NULLPTR;
            }
            session->sessionGetTargetOrientationConfigInfoFunc_ = func;
            return WSError::WS_OK;
        }, __func__);
}

WSError SceneSession::GetTargetOrientationConfigInfo(Orientation targetOrientation,
    const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& targetProperties,
    const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& currentProperties)
{
    PostTask(
        [weakThis = wptr(this), targetOrientation, targetProperties, currentProperties, where = __func__] {
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
                return WSError::WS_ERROR_NULLPTR;
            }
            session->SetSystemBarPropertyForRotation(targetProperties);
            session->SetCurrentSystemBarPropertyForRotation(currentProperties);
            if (session->sessionGetTargetOrientationConfigInfoFunc_) {
                session->sessionGetTargetOrientationConfigInfoFunc_(static_cast<uint32_t>(targetOrientation));
            }
            return WSError::WS_OK;
        }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::ConvertOrientationAndRotation(const RotationInfoType from, const RotationInfoType to,
        const int32_t value, int32_t& convertedValue)
{
    WSError ret = WSError::WS_ERROR_INVALID_PARAM;
    if (value < MIN_ROTATION_VALUE || value > MAX_ROTATION_VALUE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "the value to be converted is invalid");
        return ret;
    }
    if (from == to) {
        convertedValue = value;
        return WSError::WS_OK;
    }
    if (from == RotationInfoType::DISPLAY_ORIENTATION && to == RotationInfoType::WINDOW_ORIENTATION) {
        ret = ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
        return ret;
    }
    if (from == RotationInfoType::WINDOW_ORIENTATION && to == RotationInfoType::DISPLAY_ORIENTATION) {
        ret = ConvertWindowOrientationToDisplayOrientation(value, convertedValue);
        return ret;
    }
    if (from == RotationInfoType::DISPLAY_ROTATION && to == RotationInfoType::DISPLAY_ORIENTATION) {
        ret = ConvertDisplayRotationToDisplayOrientation(value, convertedValue);
        return ret;
    }
    if (from == RotationInfoType::DISPLAY_ORIENTATION && to == RotationInfoType::DISPLAY_ROTATION) {
        ret = ConvertDisplayOrientationToDisplayRotation(value, convertedValue);
        return ret;
    }
    if (from == RotationInfoType::DISPLAY_ROTATION && to == RotationInfoType::WINDOW_ORIENTATION) {
        ret = ConvertDisplayRotationToWindowOrientation(value, convertedValue);
        return ret;
    }
    if (from == RotationInfoType::WINDOW_ORIENTATION && to == RotationInfoType::DISPLAY_ROTATION) {
        ret = ConvertWindowOrientationToDisplayRotation(value, convertedValue);
        return ret;
    }
    return ret;
}

WSError SceneSession::ConvertDisplayOrientationToWindowOrientation(const int32_t value, int32_t& convertedValue)
{
    DisplayOrientation displayOrientation = static_cast<DisplayOrientation>(value);
    if (displayOrientation == DisplayOrientation::UNKNOWN) {
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto it = DISPLAY_TO_WINDOW_MAP.find(displayOrientation);
    if (it != DISPLAY_TO_WINDOW_MAP.end()) {
        convertedValue = static_cast<int32_t>(it->second);
        return WSError::WS_OK;
    }
    return WSError::WS_ERROR_INVALID_PARAM;
}

WSError SceneSession::ConvertWindowOrientationToDisplayOrientation(const int32_t value, int32_t& convertedValue)
{
    WindowOrientation windowOrientation = static_cast<WindowOrientation>(value);
    auto it = WINDOW_TO_DISPLAY_MAP.find(windowOrientation);
    if (it != WINDOW_TO_DISPLAY_MAP.end()) {
        convertedValue = static_cast<int32_t>(it->second);
        return WSError::WS_OK;
    }
    return WSError::WS_ERROR_INVALID_PARAM;
}

WSError SceneSession::ConvertDisplayRotationToDisplayOrientation(const int32_t rotation, int32_t& orientation)
{
    sptr<ScreenSession> screenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(GetSessionProperty()->GetDisplayId());
    if (screenSession == nullptr) {
        TLOGW(WmsLogTag::WMS_ROTATION, "Screen session is null");
        return WSError::WS_ERROR_INVALID_DISPLAY;
    }
    FoldDisplayMode foldDisplayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    Rotation targetRotation = static_cast<Rotation>(rotation);
    RRect bounds = screenSession->CalcBoundsByRotation(targetRotation);
    DisplayOrientation displayOrientation =
        screenSession->CalcDeviceOrientationWithBounds(targetRotation, foldDisplayMode, bounds);
    orientation = static_cast<int32_t>(displayOrientation);
    TLOGI(WmsLogTag::WMS_ROTATION,
        "rotation: %{public}d, width: %{public}f, height: %{public}f, orientation: %{public}d",
        rotation, bounds.rect_.width_, bounds.rect_.height_, orientation);
    return WSError::WS_OK;
}

WSError SceneSession::ConvertDisplayOrientationToDisplayRotation(const int32_t orientation, int32_t& rotation)
{
    sptr<ScreenSession> screenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(GetSessionProperty()->GetDisplayId());
    if (screenSession == nullptr) {
        TLOGW(WmsLogTag::WMS_ROTATION, "Screen session is null");
        return WSError::WS_ERROR_INVALID_DISPLAY;
    }
    FoldDisplayMode foldDisplayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    DisplayOrientation displayOrientation = static_cast<DisplayOrientation>(orientation);
    RRect bounds = screenSession->CalcBoundsInRotationZero(foldDisplayMode);
    Rotation targetRotation =
        screenSession->CalcRotationByDeviceOrientation(displayOrientation, foldDisplayMode, bounds);
    rotation = static_cast<int32_t>(targetRotation);
    TLOGI(WmsLogTag::WMS_ROTATION,
        "rotation: %{public}d, width: %{public}f, height: %{public}f, orientation: %{public}d",
        rotation, bounds.rect_.width_, bounds.rect_.height_, orientation);
    return WSError::WS_OK;
}

WSError SceneSession::ConvertDisplayRotationToWindowOrientation(const int32_t value, int32_t& convertedValue)
{
    // convert displayRotation to displayorientation
    int32_t displayOrientation;
    WSError ret = ConvertDisplayRotationToDisplayOrientation(value, displayOrientation);
    if (ret != WSError::WS_OK) {
        TLOGNE(WmsLogTag::WMS_ROTATION, "failed to convert DisplayRotation to DisplayOrientation");
        return ret;
    }
    // convert displayorientation to windowOrientation
    ret = ConvertDisplayOrientationToWindowOrientation(displayOrientation, convertedValue);
    return ret;
}

WSError SceneSession::ConvertWindowOrientationToDisplayRotation(const int32_t value, int32_t& convertedValue)
{
    // convert windowOrientation to displayorientation
    int32_t displayOrientation;
    WSError ret = ConvertWindowOrientationToDisplayOrientation(value, displayOrientation);
    if (ret != WSError::WS_OK) {
        TLOGNE(WmsLogTag::WMS_ROTATION, "failed to convert WindowOrientation to DisplayOrientation");
        return ret;
    }
    // convert displayorientation to displayRotation
    ret = ConvertDisplayOrientationToDisplayRotation(displayOrientation, convertedValue);
    return ret;
}

WSError SceneSession::NotifyRotationProperty(uint32_t rotation, uint32_t width, uint32_t height)
{
    PostTask(
        [weakThis = wptr(this), rotation, width, height, where = __func__] {
            if (width == 0 || height == 0) {
                return WSError::WS_ERROR_INVALID_PARAM;
            }
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
                return WSError::WS_ERROR_NULLPTR;
            }
            WSRect wsrect = { 0, 0, width, height };
            auto properties = session->GetSystemBarPropertyForRotation();
            std::map<AvoidAreaType, AvoidArea> avoidAreas;
            uint32_t orientation = 0;
            WSError ret = session->ConvertRotationToOrientation(rotation, width, height, orientation);
            if (ret != WSError::WS_OK) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "failed to convert Rotation to Orientation");
                return ret;
            }
            // Orientation type is required here
            session->GetAvoidAreasByRotation(
                static_cast<Rotation>(orientation), wsrect, properties, avoidAreas);
            if (!session->sessionStage_) {
                return WSError::WS_ERROR_NULLPTR;
            }
            Rect rect = { wsrect.posX_, wsrect.posY_, wsrect.width_, wsrect.height_ };
            OrientationInfo info = { orientation, rect, avoidAreas };

            WSRect currentWsrect = session->GetSessionRect();
            auto currentProperties = session->GetCurrentSystemBarPropertyForRotation();
            std::map<AvoidAreaType, AvoidArea> currentAvoidAreas;
            uint32_t currentOrientation = 0;
            uint32_t currentRotation = static_cast<uint32_t>(session->GetCurrentRotation());
            WSError currentRet = session->ConvertRotationToOrientation(currentRotation,
                currentWsrect.width_, currentWsrect.height_, currentOrientation);
            if (currentRet != WSError::WS_OK) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "failed to convert currentWsrect Rotation to Orientation");
                return currentRet;
            }
            // Orientation type is required here
            session->GetAvoidAreasByRotation(
                static_cast<Rotation>(currentOrientation), currentWsrect, currentProperties, currentAvoidAreas);
            Rect currentRect =
                { currentWsrect.posX_, currentWsrect.posY_,currentWsrect.width_, currentWsrect.height_ };
            OrientationInfo currentInfo = { currentOrientation, currentRect, currentAvoidAreas };
            session->sessionStage_->NotifyTargetRotationInfo(info, currentInfo);
            return WSError::WS_OK;
        }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::NotifyPageRotationIsIgnored()
{
    PostTask(
        [weakThis = wptr(this), where = __func__] {
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
                return;
            }
            if (!session->sessionStage_) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s sessionStage is null", where);
                return;
            }
            session->sessionStage_->NotifyPageRotationIsIgnored();
        },
        __func__);
    return WSError::WS_OK;
}

WSError SceneSession::ConvertRotationToOrientation(uint32_t rotation,
    uint32_t width, uint32_t height, uint32_t& orientation)
{
    sptr<ScreenSession> screenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(GetSessionProperty()->GetDisplayId());
    if (screenSession == nullptr) {
        TLOGW(WmsLogTag::WMS_ROTATION, "Screen session is null");
        return WSError::WS_ERROR_INVALID_DISPLAY;
    }
    FoldDisplayMode foldDisplayMode = ScreenSessionManagerClient::GetInstance().GetFoldDisplayMode();
    Rotation targetRotation = static_cast<Rotation>(rotation / ROTATION_DEGREE);
    RRect bounds = RRect({ 0, 0, width, height }, 0.0f, 0.0f);
    DisplayOrientation displayOrientation =
        screenSession->CalcDeviceOrientationWithBounds(targetRotation, foldDisplayMode, bounds);
    orientation = static_cast<uint32_t>(displayOrientation);
    TLOGI(WmsLogTag::WMS_ROTATION,
        "rotation: %{public}d, width: %{public}d, height: %{public}d, orientation: %{public}d",
        rotation, width, height, orientation);
    return WSError::WS_OK;
}

void SceneSession::SetSystemBarPropertyForRotation(
    const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& properties)
{
    targetSystemBarProperty_ = properties;
}

std::map<Rosen::WindowType, Rosen::SystemBarProperty>& SceneSession::GetSystemBarPropertyForRotation()
{
    return targetSystemBarProperty_;
}

void SceneSession::SetCurrentSystemBarPropertyForRotation(
    const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& properties)
{
    currentSystemBarProperty_ = properties;
}

std::map<Rosen::WindowType, Rosen::SystemBarProperty>& SceneSession::GetCurrentSystemBarPropertyForRotation()
{
    return currentSystemBarProperty_;
}

void SceneSession::SetSidebarBlur(bool isDefaultSidebarBlur, bool isNeedAnimation)
{
    TLOGI(WmsLogTag::WMS_PC, "isDefaultSidebarBlur: %{public}d, isNeedAnimation: %{public}d",
        isDefaultSidebarBlur, isNeedAnimation);
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_PC, "sessionStage is null");
        return;
    }
    SidebarBlurType type = isDefaultSidebarBlur ?
        (isNeedAnimation ? SidebarBlurType::DEFAULT_FLOAT : SidebarBlurType::INITIAL) : SidebarBlurType::NONE;
    sessionStage_->SetSidebarBlurStyleWithType(type);
}

void SceneSession::NotifyWindowAttachStateListenerRegistered(bool registered)
{
    needNotifyAttachState_.store(registered);
}

void SceneSession::NotifyKeyboardAnimationCompleted(bool isShowAnimation,
    const WSRect& beginRect, const WSRect& endRect)
{
    PostTask([weakThis = wptr(this), isShowAnimation, beginRect, endRect, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session is null", where);
            return;
        }
        if (session->sessionStage_ == nullptr) {
            TLOGND(WmsLogTag::WMS_KEYBOARD, "%{public}s sessionStage_ is null, id: %{public}d",
                where, session->GetPersistentId());
            return;
        }
        if (isShowAnimation && !session->GetSessionProperty()->EditSessionInfo().isKeyboardDidShowRegistered_) {
            TLOGND(WmsLogTag::WMS_KEYBOARD, "keyboard did show listener is not registered");
            return;
        }
        if (!isShowAnimation && !session->GetSessionProperty()->EditSessionInfo().isKeyboardDidHideRegistered_) {
            TLOGND(WmsLogTag::WMS_KEYBOARD, "keyboard did hide listener is not registered");
            return;
        }

        KeyboardPanelInfo keyboardPanelInfo;
        keyboardPanelInfo.beginRect_ = SessionHelper::TransferToRect(beginRect);
        keyboardPanelInfo.endRect_ = SessionHelper::TransferToRect(endRect);
        keyboardPanelInfo.isShowing_ = isShowAnimation;
        session->sessionStage_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
    }, __func__);
}

void SceneSession::NotifyKeyboardAnimationWillBegin(bool isKeyboardShow, const WSRect& beginRect, const WSRect& endRect,
    bool withAnimation, const std::shared_ptr<RSTransaction>& rsTransaction)
{
    PostTask([weakThis = wptr(this), isKeyboardShow, beginRect, endRect, withAnimation,
        rsTransaction, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session is null", where);
            return;
        }
        if (session->sessionStage_ == nullptr) {
            TLOGND(WmsLogTag::WMS_KEYBOARD, "%{public}s sessionStage_ is null, id: %{public}d",
                where, session->GetPersistentId());
            return;
        }
        if (isKeyboardShow && !session->GetSessionProperty()->EditSessionInfo().isKeyboardWillShowRegistered_) {
            TLOGNI(WmsLogTag::WMS_KEYBOARD, "keyboard will show listener is not registered");
            return;
        }
        if (!isKeyboardShow && !session->GetSessionProperty()->EditSessionInfo().isKeyboardWillHideRegistered_) {
            TLOGNI(WmsLogTag::WMS_KEYBOARD, "keyboard will hide listener is not registered");
            return;
        }
        KeyboardAnimationInfo keyboardAnimationInfo;
        keyboardAnimationInfo.beginRect = SessionHelper::TransferToRect(beginRect);
        keyboardAnimationInfo.endRect = SessionHelper::TransferToRect(endRect);
        keyboardAnimationInfo.isShow = isKeyboardShow;
        keyboardAnimationInfo.withAnimation = withAnimation;
        session->sessionStage_->NotifyKeyboardAnimationWillBegin(keyboardAnimationInfo, rsTransaction);
        }, __func__);
}

void SceneSession::NotifyKeyboardWillShowRegistered(bool registered)
{
    GetSessionProperty()->EditSessionInfo().isKeyboardWillShowRegistered_ = registered;
}

void SceneSession::NotifyKeyboardWillHideRegistered(bool registered)
{
    GetSessionProperty()->EditSessionInfo().isKeyboardWillHideRegistered_ = registered;
}

void SceneSession::NotifyKeyboardDidShowRegistered(bool registered)
{
    GetSessionProperty()->EditSessionInfo().isKeyboardDidShowRegistered_ = registered;
}

void SceneSession::NotifyKeyboardDidHideRegistered(bool registered)
{
    GetSessionProperty()->EditSessionInfo().isKeyboardDidHideRegistered_ = registered;
}

void SceneSession::SaveLastDensity()
{
    auto property = GetSessionProperty();
    DisplayId displayId = property->GetDisplayId();
    if (auto display = DisplayManager::GetInstance().GetDisplayById(displayId)) {
        property->SetLastLimitsVpr(display->GetVirtualPixelRatio());
    }
}

void SceneSession::NotifyUpdateFlagCallback(NotifyUpdateFlagFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), where, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session or func is null");
            return;
        }
        session->onUpdateFlagFunc_ = std::move(func);
        session->onUpdateFlagFunc_(session->sessionInfo_.specifiedFlag_);
        TLOGND(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d specifiedFlag: %{public}s", where,
            session->GetPersistentId(), session->sessionInfo_.specifiedFlag_.c_str());
    }, __func__);
}

WSError SceneSession::UpdateRotationChangeRegistered(int32_t persistentId, bool isRegister)
{
    PostTask(
        [weakThis = wptr(this), persistentId, isRegister, where = __func__] {
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
                return;
            }
            TLOGI(
                WmsLogTag::WMS_ROTATION, "persistentId: %{public}d, isRegister: %{public}d", persistentId, isRegister);
            if (isRegister) {
                session->isRotationChangeCallbackRegistered = true;
            } else {
                session->isRotationChangeCallbackRegistered = false;
            }
        }, __func__);

    return WSError::WS_OK;
}

RotationChangeResult SceneSession::NotifyRotationChange(const RotationChangeInfo& rotationChangeInfo,
    bool isRestrictNotify)
{
    WindowType type = Session::GetWindowType();
    bool isSystemCalling = GetSessionProperty()->GetSystemCalling();
    return PostSyncTask(
        [weakThis = wptr(this), rotationChangeInfo, isRestrictNotify, type, isSystemCalling,
            where = __func__]() -> RotationChangeResult {
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
                return { RectType::RELATIVE_TO_SCREEN, { 0, 0, 0, 0 } };
            }
            if (!session->sessionStage_ || !session->isRotationChangeCallbackRegistered) {
                TLOGE(WmsLogTag::WMS_ROTATION, "sessionStage_ is null or isRotationChangeCallbackRegistered is false");
                return { RectType::RELATIVE_TO_SCREEN, { 0, 0, 0, 0 } };
            }
            if (isRestrictNotify && (!WindowHelper::IsSystemWindow(type) || !isSystemCalling)) {
                TLOGE(WmsLogTag::WMS_ROTATION,
                    "restrict NotifyRotationChange when not system window or not system calling");
                return { RectType::RELATIVE_TO_SCREEN, { 0, 0, 0, 0 } };
            }
            return session->sessionStage_->NotifyRotationChange(rotationChangeInfo);
        }, __func__);
}

WSError SceneSession::SetCurrentRotation(int32_t currentRotation)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "currentRotation: %{public}d", currentRotation);
    PostTask([weakThis = wptr(this), currentRotation, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s session is null", where);
            return;
        }
        session->currentRotation_ = currentRotation;
        if (!session->sessionStage_) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s sessionStage is null", where);
            return;
        }
        session->sessionStage_->SetCurrentRotation(currentRotation);
        }, __func__);
    return WSError::WS_OK;
}

WMError SceneSession::UpdateScreenshotAppEventRegistered(int32_t persistentId, bool isRegister)
{
    PostTask([weakThis = wptr(this), persistentId, isRegister, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s win %{public}d session is null", where, persistentId);
            return;
        }
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s win %{public}d isRegister %{public}uu",
            where, persistentId, isRegister);
        if (session->updateScreenshotAppEventRegisteredFunc_) {
            session->updateScreenshotAppEventRegisteredFunc_(persistentId, isRegister);
        }
        }, __func__);

    return WMError::WM_OK;
}

WMError SceneSession::UpdateAcrossDisplaysChangeRegistered(bool isRegister)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d permission denied!", GetPersistentId());
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!WindowHelper::IsAppWindow(GetWindowType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid window: %{public}d type %{public}u",
            GetPersistentId(), GetWindowType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    PostTask([weakThis = wptr(this), isRegister, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s session is null", where);
            return;
        }
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s winId: %{public}d, isRegister: %{public}d",
            where, session->GetPersistentId(), isRegister);
        session->isRegisterAcrossDisplaysChanged_.store(isRegister);
        }, __func__);

    return WMError::WM_OK;
}

WMError SceneSession::NotifyDisableDelegatorChange()
{
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "set session id %{public}d disableDelegator true", session->persistentId_);
        session->sessionInfo_.disableDelegator = true;
    }, __func__);
    return WMError::WM_OK;
}

void SceneSession::HookSceneSessionActivation(NotifyHookSceneSessionActivationFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), where, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        session->hookSceneSessionActivationFunc_ = std::move(func);
    }, where);
}

void SceneSession::SetSidebarBlurMaximize(bool isMaximize)
{
    TLOGI(WmsLogTag::WMS_PC, "isMaximize: %{public}d", isMaximize);
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_PC, "sessionStage is null");
        return;
    }
    sessionStage_->SetSidebarBlurStyleWithType(isMaximize ? SidebarBlurType::DEFAULT_MAXIMIZE :
        SidebarBlurType::DEFAULT_FLOAT);
}

void SceneSession::SetSceneSessionDestructNotificationFunc(NotifySceneSessionDestructFunc&& func)
{
    notifySceneSessionDestructFunc_ = std::move(func);
}

void SceneSession::SetIsUserRequestedExit(bool isUserRequestedExit)
{
    isUserRequestedExit_ = isUserRequestedExit;
}

void SceneSession::SetIsAncoForFloatingWindow(bool isAncoForFloatingWindow)
{
    isAncoForFloatingWindow_ = isAncoForFloatingWindow;
}

bool SceneSession::GetIsAncoForFloatingWindow() const
{
    return isAncoForFloatingWindow_;
}

WSError SceneSession::UseImplicitAnimation(bool useImplicit)
{
    return PostSyncTask([weakThis = wptr(this), useImplicit, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->useImplicitAnimationChangeFunc_) {
            session->useImplicitAnimationChangeFunc_(useImplicit);
        }
        return WSError::WS_OK;
    }, __func__);
}

void SceneSession::RegisterUseImplicitAnimationChangeCallback(NotifyUseImplicitAnimationChangeFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s session or func is null", where);
            return;
        }
        session->useImplicitAnimationChangeFunc_ = std::move(func);
        TLOGND(WmsLogTag::WMS_PC, "%{public}s id: %{public}d", where, session->GetPersistentId());
    }, __func__);
}

void SceneSession::SetSubWindowOutlineEnabled(bool subWindowOutlineEnabled)
{
    subWindowOutlineEnabled_ = subWindowOutlineEnabled;
}

bool SceneSession::IsSubWindowOutlineEnabled() const
{
    if (auto sessionProperty = GetSessionProperty()) {
        return sessionProperty->IsSubWindowOutlineEnabled();
    }
    return false;
}

/**
 * Window Transition Animation For PC and FreeMultiWindow device
 */
WSError SceneSession::SetWindowTransitionAnimation(WindowTransitionType transitionType,
    const TransitionAnimation& animation)
{
    if (!(IsPcWindow() || IsFreeMultiWindowMode() || GetSessionProperty()->GetIsPcAppInPad())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Not pc or pad device");
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }

    if (!WindowHelper::IsMainWindow(GetWindowType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Scene session is not main window");
        return WSError::WS_ERROR_INVALID_CALLING;
    }

    if (updateTransitionAnimationFunc_) {
        updateTransitionAnimationFunc_(transitionType, animation);
    }
    return WSError::WS_OK;
}

void SceneSession::SetSubWindowSourceFunc(NotifySetSubWindowSourceFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_SUB, "func is null");
        return;
    }
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is null", where);
            return;
        }
        func(session->subWindowSource_);
        session->subWindowSourceFunc_ = std::move(func);
    }, __func__);
}

WSError SceneSession::SetSubWindowSource(SubWindowSource source)
{
    auto property = GetSessionProperty();
    if (!WindowHelper::IsSubWindow(property->GetWindowType()) &&
        !WindowHelper::IsDialogWindow(property->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SUB, "only sub window and dialog is valid");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    PostTask([weakThis = wptr(this), source = source, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is null", where);
            return;
        }
        TLOGI(WmsLogTag::WMS_SUB, "SetSubWindowSource source: %{public}d", source);
        session->subWindowSource_ = source;
        if (session->subWindowSourceFunc_) {
            session->subWindowSourceFunc_(source);
        } else {
            TLOGNE(WmsLogTag::WMS_SUB, "func is null");
        }
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::CloseSpecificScene()
{
    TLOGI(WmsLogTag::WMS_EVENT, "close specific scene");
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_EVENT, "sessionStage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->CloseSpecificScene();
}

void SceneSession::RegisterAnimateToCallback(NotifyAnimateToFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: Session is null", where);
            return;
        }
        session->onAnimateTo_ = std::move(callback);
    }, __func__);
}

WMError SceneSession::AnimateTo(const WindowAnimationProperty& animationProperty,
    const WindowAnimationOption& animationOption)
{
    PostTask([weakThis = wptr(this), where = __func__, animationProperty, animationOption] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: Session is null", where);
            return;
        }
        if (session->onAnimateTo_) {
            session->onAnimateTo_(animationProperty, animationOption);
            return;
        }
        TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: onAnimateTo_ is nullptr", where);
    }, __func__);
    TLOGI(WmsLogTag::WMS_ANIMATION, "Post AnimateTo task success. windowId: %{public}d, targetScale: %{public}f, \
        animationOption: %{public}s", GetWindowId(), animationProperty.targetScale, \
        animationOption.ToString().c_str());
    return WMError::WM_OK;
}

void SceneSession::SetGetAllAppUseControlMapFunc(GetAllAppUseControlMapFunc&& func)
{
    onGetAllAppUseControlMapFunc_ = std::move(func);
}

WSError SceneSession::SetFrameRectForPartialZoomIn(const Rect& frameRect)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "permission denied.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    PostTask([weakThis = wptr(this), frameRect] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        return session->SetFrameRectForPartialZoomInInner(frameRect);
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSession::SetFrameRectForPartialZoomInInner(const Rect& frameRect)
{
    if (!mainHandler_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Main handler is nullptr, id: %{public}d.", GetPersistentId());
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    mainHandler_->PostTask([weakThis = wptr(this), frameRect, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s, session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto surfaceNode = session->GetSurfaceNode();
        if (surfaceNode == nullptr) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s, surface node is null", where);
            return WSError::WS_ERROR_INVALID_WINDOW;
        }
        AutoRSTransaction trans(session->GetRSUIContext(), true);
        surfaceNode->SetRegionToBeMagnified({ frameRect.posX_, frameRect.posY_, frameRect.width_, frameRect.height_ });
        TLOGNI(WmsLogTag::WMS_ANIMATION, "%{public}s, frameRect: %{public}s", where, frameRect.ToString().c_str());
        return WSError::WS_OK;
    }, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return WSError::WS_OK;
}

void SceneSession::SetFindScenePanelRsNodeByZOrderFunc(FindScenePanelRsNodeByZOrderFunc&& func)
{
    findScenePanelRsNodeByZOrderFunc_ = std::move(func);
}

void SceneSession::SetVsyncStation(const std::shared_ptr<VsyncStation>& vsyncStation)
{
    RETURN_IF_NULL(vsyncStation);
    vsyncStation_ = vsyncStation;
}

std::optional<uint32_t> SceneSession::GetFpsFromVsync() const
{
    RETURN_IF_NULL(vsyncStation_, std::nullopt);
    return vsyncStation_->GetFps();
}

void SceneSession::RunOnNextVsync(OnCallback&& callback)
{
    RETURN_IF_NULL(vsyncStation_);
    vsyncStation_->RunOnceOnNextVsync(std::move(callback));
}

void SceneSession::RunAfterNVsyncs(uint32_t delayVsyncCount, OnCallback&& callback)
{
    RETURN_IF_NULL(vsyncStation_);
    vsyncStation_->RunOnceAfterNVsyncs(delayVsyncCount, std::move(callback));
}

void SceneSession::SetSecurityLayerWhenEnterForeground()
{
    auto leashWinSurfaceNode = GetLeashWinShadowSurfaceNode();
    if (leashWinSurfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "leash node is null, win=[%{public}d, %{public}s]", GetWindowId(),
            GetWindowName().c_str());
        return;
    }
    auto sessionProperty = GetSessionProperty();
    bool lastPrivacyMode = sessionProperty->GetPrivacyMode() || sessionProperty->GetSystemPrivacyMode();
    bool multiInstanceEnabled = RSAdapterUtil::IsClientMultiInstanceEnabled();
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "win=[%{public}d, %{public}s], isMultiInstance=%{public}d, isPrivacy=%{public}d",
        GetWindowId(), GetWindowName().c_str(), multiInstanceEnabled, lastPrivacyMode);
    if (multiInstanceEnabled) {
        AutoRSTransaction trans(GetRSLeashWinShadowContext());
        leashWinSurfaceNode->SetSecurityLayer(lastPrivacyMode);
    } else {
        leashWinSurfaceNode->SetSecurityLayer(lastPrivacyMode);
    }
}

void SceneSession::RestoreGravityWhenDragEnd()
{
    // Ensure the last frame of drag rendering is completed before restoring the gravity to its pre-drag state.
    constexpr uint32_t gravityUpdateDelayVsyncs = 2;
    RunAfterNVsyncs(gravityUpdateDelayVsyncs, [weakSession = wptr(this), where = __func__](int64_t, int64_t) {
        auto session = weakSession.promote();
        RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);

        session->PostTask([weakSession, where] {
            auto session = weakSession.promote();
            RETURN_IF_NULL_IMPL(session, WmsLogTag::WMS_LAYOUT, where);
            RETURN_IF_NULL_IMPL(session->moveDragController_, WmsLogTag::WMS_LAYOUT, where);

            session->moveDragController_->RestoreToPreDragGravity(session->GetSurfaceNode());
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: Restore gravity completed, windowId: %{public}d",
                where, session->GetPersistentId());
        }, where);
    });
}

void SceneSession::RegisterRotationLockChangeCallback(NotifyRotationLockChangeFunc&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session || !session->specificCallback_ || !callback) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "session or specific callback or callback is null");
            return;
        }
        session->specificCallback_->onRotationLockChange_ = std::move(callback);
        session->specificCallback_->onRotationLockChange_(session->GetSessionProperty()->GetRotationLocked());
    }, __func__);
}
 
WMError SceneSession::HandleActionUpdateRotationLockChange(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    GetSessionProperty()->SetRotationLocked(property->GetRotationLocked());
    PostTask([weakThis = wptr(this), property] {
        auto session = weakThis.promote();
        if (!session || !session->specificCallback_) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "session or specific callback is null");
            return;
        }
        if (session->specificCallback_->onRotationLockChange_) {
            session->specificCallback_->onRotationLockChange_(property->GetRotationLocked());
        }
    }, __func__);
    return WMError::WM_OK;
}
/*
 * Window Event start
 */
bool SceneSession::CheckParameters(const std::vector<int32_t>& parameters, const int32_t length)
{
    if (parameters.size() < 1) {
        TLOGE(WmsLogTag::WMS_EVENT, "The format is incorrect(size<1).");
        return false;
    }
    if (parameters[0] != length) {
        TLOGE(WmsLogTag::WMS_EVENT, "The format is incorrect(length error).");
        return false;
    }
    if (parameters[0] != static_cast<int32_t>(parameters.size() - 1)) {
        TLOGE(WmsLogTag::WMS_EVENT, "The format is incorrect(size error).");
        return false;
    }
    return true;
}

WMError SceneSession::LockCursor(const std::vector<int32_t>& parameters)
{
    if (!SessionPermission::VerifyCallingPermission(LOCK_WINDOW_CURSOR_PERMISSION)) {
        TLOGE(WmsLogTag::WMS_EVENT, "The caller has not permission granted");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!CheckParameters(parameters, LOCK_CURSOR_LENGTH)) {
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    if (parameters[1] != GetWindowId()) {
        TLOGE(WmsLogTag::WMS_EVENT, "The windowId does not match.");
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    bool isCursorFollowMovement = static_cast<bool>(parameters[2]);
    SetSessionInfoAdvancedFeatureFlag(OHOS::Rosen::ADVANCED_FEATURE_BIT_LOCK_CURSOR, true);
    SetSessionInfoAdvancedFeatureFlag(OHOS::Rosen::ADVANCED_FEATURE_BIT_CURSOR_FOLLOW_MOVEMENT,
        isCursorFollowMovement);
    NotifySessionInfoChange();
    return WMError::WM_OK;
}

WMError SceneSession::UnlockCursor(const std::vector<int32_t>& parameters)
{
    if (!SessionPermission::VerifyCallingPermission(LOCK_WINDOW_CURSOR_PERMISSION)) {
        TLOGE(WmsLogTag::WMS_EVENT, "The caller has not permission granted");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!CheckParameters(parameters, UNLOCK_CURSOR_LENGTH)) {
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    if (parameters[1] != GetWindowId()) {
        TLOGE(WmsLogTag::WMS_EVENT, "The windowId does not match.");
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    SetSessionInfoAdvancedFeatureFlag(OHOS::Rosen::ADVANCED_FEATURE_BIT_LOCK_CURSOR, false);
    NotifySessionInfoChange();
    return WMError::WM_OK;
}

WMError SceneSession::SetReceiveDragEventEnabled(const std::vector<int32_t>& parameters)
{
    if (!CheckParameters(parameters, SET_RECEIVE_DRAG_EVENT_LENGTH)) {
        TLOGE(WmsLogTag::WMS_EVENT, "The param is illegal");
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    bool enabled = static_cast<bool>(parameters[1]);
    SetSessionInfoReceiveDragEventEnabled(enabled);
    SetSessionInfoAdvancedFeatureFlag(OHOS::Rosen::ADVANCED_FEATURE_BIT_RECEIVE_DRAG_EVENT, !enabled);
    NotifySessionInfoChange();
    return WMError::WM_OK;
}

WMError SceneSession::SetSeparationTouchEnabled(const std::vector<int32_t>& parameters)
{
    if (!CheckParameters(parameters, WINDOW_SEPARATION_TOUCH_ENABLED_LENGTH)) {
        TLOGE(WmsLogTag::WMS_EVENT, "The param is illegal");
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    bool enabled = static_cast<bool>(parameters[1]);
    SetSessionInfoSeparationTouchEnabled(enabled);
    SetSessionInfoAdvancedFeatureFlag(OHOS::Rosen::ADVANCED_FEATURE_BIT_WINDOW_SEPARATION_TOUCH_ENABLED, !enabled);
    NotifySessionInfoChange();
    return WMError::WM_OK;
}

void SceneSession::RegisterIsAppBoundSystemTrayCallback(
    const std::function<bool(int32_t callingPid, uint32_t callingToken, const std::string &instanceKey)>& callback)
{
    isAppBoundSystemTrayCallback_ = callback;
}
/*
 * Window Event end
 */
} // namespace OHOS::Rosen
