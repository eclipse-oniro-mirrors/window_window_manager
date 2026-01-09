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

#include "window_scene_session_impl.h"

#include <chrono>
#include <limits>
#include <ability_manager_client.h>
#include <modifier/rs_property.h>
#include <modifier_ng/appearance/rs_behind_window_filter_modifier.h>
#include <parameters.h>
#include <sstream>
#include <string>
#include <transaction/rs_transaction.h>
#include <hitrace_meter.h>
#include <hisysevent.h>

#include <application_context.h>
#include "color_parser.h"
#include "common/include/fold_screen_state_internel.h"
#include "common/include/fold_screen_common.h"
#include "configuration.h"
#include "display_manager.h"
#include "display_manager_adapter.h"
#include "dm_common.h"
#include "extension/extension_business_info.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "input_transfer_station.h"
#include "perform_reporter.h"
#include "rate_limited_logger.h"
#include "rs_adapter.h"
#include "session_helper.h"
#include "session_permission.h"
#include "session/container/include/window_event_channel.h"
#include "session_manager/include/session_manager.h"
#include "singleton_container.h"
#include "sys_cap_util.h"
#include "window_adapter.h"
#include "window_helper.h"
#include "window_inspector.h"
#include "window_manager_hilog.h"
#include "window_prepare_terminate.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "wm_math.h"
#include "session_manager_agent_controller.h"
#include <transaction/rs_interfaces.h>
#include "surface_capture_future.h"
#include "pattern_detach_callback.h"
#include "picture_in_picture_manager.h"
#include "window_session_impl.h"
#include "sys_cap_util.h"

namespace OHOS {
namespace Rosen {
union WSColorParam {
#if defined(BIG_ENDIANNESS) && BIG_ENDIANNESS
    struct {
        uint8_t alpha;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } argb;
#else
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } argb;
#endif
    uint32_t value;
};

#define CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession)                         \
    do {                                                                       \
        if ((hostSession) == nullptr) {                                        \
            TLOGE(WmsLogTag::DEFAULT, "hostSession is null");                  \
            return;                                                            \
        }                                                                      \
    } while (false)

#define CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, ret)              \
    do {                                                                       \
        if ((hostSession) == nullptr) {                                        \
            TLOGE(WmsLogTag::DEFAULT, "hostSession is null");                  \
            return ret;                                                        \
        }                                                                      \
    } while (false)

namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSceneSessionImpl"};
constexpr int32_t WINDOW_DETACH_TIMEOUT = 300;
constexpr int32_t WINDOW_LAYOUT_TIMEOUT = 30;
constexpr int32_t WINDOW_PAGE_ROTATION_TIMEOUT = 2000;
const std::string PARAM_DUMP_HELP = "-h";
constexpr float MIN_GRAY_SCALE = 0.0f;
constexpr float MAX_GRAY_SCALE = 1.0f;
constexpr int32_t DISPLAY_ID_C = 999;
constexpr int32_t MAX_POINTERS = 16;
constexpr int32_t TOUCH_SLOP_RATIO = 25;
const std::string BACK_WINDOW_EVENT = "scb_back_window_event";
const std::string COMPATIBLE_MAX_WINDOW_EVENT = "win_compatible_max_event";
const std::string COMPATIBLE_RECOVER_WINDOW_EVENT = "win_compatible_recover_event";
const std::string NAME_LANDSCAPE_2_3_CLICK = "win_change_to_2_3_landscape";
const std::string NAME_LANDSCAPE_1_1_CLICK = "win_change_to_1_1_landscape";
const std::string NAME_LANDSCAPE_18_9_CLICK = "win_change_to_18_9_landscape";
const std::string NAME_LANDSCAPE_SPLIT_CLICK = "win_change_to_split_landscape";
const std::string NAME_DEFAULT_LANDSCAPE_CLICK = "win_change_to_default_landscape";
const std::string EVENT_NAME_HOVER = "win_hover_event";
const std::string MOUSE_HOVER = "mouseHover";
const std::string TOUCH_HOVER = "touchHover";
const std::string EXIT_HOVER = "exitHover";
constexpr char SCENE_BOARD_UE_DOMAIN[] = "SCENE_BOARD_UE";
constexpr char HOVER_MAXIMIZE_MENU[] = "PC_HOVER_MAXIMIZE_MENU";
const std::unordered_set<WindowType> INVALID_SYSTEM_WINDOW_TYPE = {
    WindowType::WINDOW_TYPE_NEGATIVE_SCREEN,
    WindowType::WINDOW_TYPE_THEME_EDITOR,
    WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR,
    WindowType::WINDOW_TYPE_SCENE_BOARD,
    WindowType::WINDOW_TYPE_KEYBOARD_PANEL,
    WindowType::WINDOW_TYPE_APP_LAUNCHING,
    WindowType::WINDOW_TYPE_INCOMING_CALL,
    WindowType::WINDOW_TYPE_BOOT_ANIMATION,
    WindowType::WINDOW_TYPE_FREEZE_DISPLAY,
    WindowType::WINDOW_TYPE_PLACEHOLDER
};
const std::unordered_set<WindowType> INVALID_SCB_WINDOW_TYPE = {
    WindowType::WINDOW_TYPE_WALLPAPER,
    WindowType::WINDOW_TYPE_DESKTOP,
    WindowType::WINDOW_TYPE_DOCK_SLICE,
    WindowType::WINDOW_TYPE_STATUS_BAR,
    WindowType::WINDOW_TYPE_KEYGUARD,
    WindowType::WINDOW_TYPE_NAVIGATION_BAR,
    WindowType::WINDOW_TYPE_LAUNCHER_RECENT,
    WindowType::WINDOW_TYPE_LAUNCHER_DOCK
};
constexpr float INVALID_DEFAULT_DENSITY = 1.0f;
constexpr uint32_t FORCE_LIMIT_MIN_FLOATING_WIDTH = 40;
constexpr uint32_t FORCE_LIMIT_MIN_FLOATING_HEIGHT = 40;
constexpr int32_t API_VERSION_18 = 18;
constexpr uint32_t SNAPSHOT_TIMEOUT = 2000; // MS
constexpr uint32_t REASON_MAXIMIZE_MODE_CHANGE = 1;
constexpr int32_t SIDEBAR_BLUR_ANIMATION_DURATION = 150;

bool IsValueInRange(double value, double lowerBound, double upperBound)
{
    return !MathHelper::GreatNotEqual(lowerBound, value) &&
           !MathHelper::GreatNotEqual(value, upperBound);
}

void RecalculatePxLimitsByVp(const WindowLimits& RefreshLimitsVp, WindowLimits& RefreshLimitsPx, float vpr)
{
    RefreshLimitsPx.maxWidth_ = static_cast<uint32_t>(RefreshLimitsVp.maxWidth_ * vpr);
    RefreshLimitsPx.maxHeight_ = static_cast<uint32_t>(RefreshLimitsVp.maxHeight_ * vpr);
    RefreshLimitsPx.minWidth_ = static_cast<uint32_t>(RefreshLimitsVp.minWidth_ * vpr);
    RefreshLimitsPx.minHeight_ = static_cast<uint32_t>(RefreshLimitsVp.minHeight_ * vpr);
}

// Must ensure that vpr is non-zero
void RecalculateVpLimitsByPx(const WindowLimits& limits, WindowLimits& limitsVP, float vpr)
{
    if (MathHelper::NearZero(vpr)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "vpr is zero");
        return;
    }
    limitsVP.maxWidth_ = static_cast<uint32_t>(std::round(limits.maxWidth_ / vpr));
    limitsVP.maxHeight_ = static_cast<uint32_t>(std::round(limits.maxHeight_ / vpr));
    limitsVP.minWidth_ = static_cast<uint32_t>(std::round(limits.minWidth_ / vpr));
    limitsVP.minHeight_ = static_cast<uint32_t>(std::round(limits.minHeight_ / vpr));
}

uint32_t SafelyRoundToUint32(double value)
{
    return (value > static_cast<double>(UINT32_MAX)) ? UINT32_MAX : std::round(value);
}

void RecalculateLimits(double maxRatio, double minRatio, WindowLimits& limits)
{
    if (MathHelper::NearZero(maxRatio) || MathHelper::NearZero(minRatio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "ratio is zero");
        return;
    }
    const uint32_t newMaxWidth = SafelyRoundToUint32(limits.maxHeight_ * maxRatio);
    const uint32_t newMinWidth = SafelyRoundToUint32(limits.minHeight_ * minRatio);
    const uint32_t newMaxHeight = MathHelper::NearZero(minRatio) ?
        UINT32_MAX : SafelyRoundToUint32(limits.maxWidth_ / minRatio);
    const uint32_t newMinHeight = MathHelper::NearZero(maxRatio) ?
        UINT32_MAX : SafelyRoundToUint32(limits.minWidth_ / maxRatio);

    limits.maxWidth_ = std::min(newMaxWidth, limits.maxWidth_);
    limits.minWidth_ = std::max(newMinWidth, limits.minWidth_);
    limits.maxHeight_ = std::min(newMaxHeight, limits.maxHeight_);
    limits.minHeight_ = std::max(newMinHeight, limits.minHeight_);
}

void UpdateLimitIfInRange(uint32_t& currentLimit, uint32_t newLimit, uint32_t minBound, uint32_t maxBound)
{
    if (minBound <= newLimit && newLimit <= maxBound) {
        currentLimit = newLimit;
    }
}
}
std::mutex WindowSceneSessionImpl::keyboardPanelInfoChangeListenerMutex_;
using WindowSessionImplMap = std::map<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>>;
std::mutex WindowSceneSessionImpl::windowAttachStateChangeListenerMutex_;

WindowSceneSessionImpl::WindowSceneSessionImpl(const sptr<WindowOption>& option,
    const std::shared_ptr<RSUIContext>& rsUIContext) : WindowSessionImpl(option, rsUIContext)
{
    WLOGFI("[WMSCom] Constructor %{public}s", GetWindowName().c_str());
}

WindowSceneSessionImpl::~WindowSceneSessionImpl()
{
    WLOGFI("[WMSCom] Destructor %{public}d, %{public}s", GetPersistentId(), GetWindowName().c_str());
}

bool WindowSceneSessionImpl::IsValidSystemWindowType(const WindowType& type)
{
    if (INVALID_SYSTEM_WINDOW_TYPE.find(type) != INVALID_SYSTEM_WINDOW_TYPE.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "Invalid type: %{public}u", type);
        return false;
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "Valid type: %{public}u", type);
    return true;
}

sptr<WindowSessionImpl> WindowSceneSessionImpl::FindParentSessionByParentId(uint32_t parentId)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& [_, pair] : windowSessionMap_) {
        auto& window = pair.second;
        if (window && window->GetWindowId() == parentId) {
            if (WindowHelper::IsMainWindow(window->GetType()) || WindowHelper::IsSystemWindow(window->GetType())) {
                TLOGD(WmsLogTag::WMS_SUB, "Find parent window [%{public}s, %{public}u, %{public}d]",
                    window->GetProperty()->GetWindowName().c_str(), parentId,
                    window->GetProperty()->GetPersistentId());
                return window;
            } else if (WindowHelper::IsSubWindow(window->GetType()) &&
                       (IsSessionMainWindow(window->GetParentId()) ||
                        window->GetProperty()->GetIsUIExtFirstSubWindow() ||
                        window->GetProperty()->GetSubWindowLevel() < MAX_SUB_WINDOW_LEVEL)) {
                // subwindow's grandparent is mainwindow or subwindow's parent is an extension subwindow
                TLOGD(WmsLogTag::WMS_SUB, "Find parent window [%{public}s, %{public}u, %{public}d]",
                    window->GetProperty()->GetWindowName().c_str(), parentId,
                    window->GetProperty()->GetPersistentId());
                return window;
            }
        }
    }
    TLOGD(WmsLogTag::WMS_SUB, "Can not find parent window, id: %{public}d", parentId);
    return nullptr;
}

sptr<WindowSessionImpl> WindowSceneSessionImpl::FindParentMainSession(uint32_t parentId, const SessionMap& sessionMap)
{
    if (parentId == INVALID_SESSION_ID) {
        TLOGW(WmsLogTag::WMS_SUB, "invalid parent id");
        return nullptr;
    }
    for (const auto& [_, pair] : sessionMap) {
        auto& window = pair.second;
        if (window && window->GetWindowId() == parentId) {
            if (WindowHelper::IsMainWindow(window->GetType()) ||
                (WindowHelper::IsSystemWindow(window->GetType()) && window->GetParentId() == INVALID_SESSION_ID)) {
                TLOGD(WmsLogTag::WMS_SUB, "find main session, id:%{public}u", window->GetWindowId());
                return window;
            }
            return FindParentMainSession(window->GetParentId(), sessionMap);
        }
    }
    TLOGW(WmsLogTag::WMS_SUB, "don't find main session, parentId:%{public}u", parentId);
    return nullptr;
}

bool WindowSceneSessionImpl::IsSessionMainWindow(uint32_t parentId)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& [_, pair] : windowSessionMap_) {
        auto& window = pair.second;
        if (window && window->GetWindowId() == parentId && WindowHelper::IsMainWindow(window->GetType())) {
            return true;
        }
    }
    return false;
}

void WindowSceneSessionImpl::AddSubWindowMapForExtensionWindow()
{
    // update subWindowSessionMap_
    auto extensionWindow = FindExtensionWindowWithContext();
    if (extensionWindow != nullptr) {
        auto parentWindowId = extensionWindow->GetPersistentId();
        std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
        subWindowSessionMap_[parentWindowId].push_back(this);
    } else {
        TLOGE(WmsLogTag::WMS_SUB, "name: %{public}s not found parent extension window",
            property_->GetWindowName().c_str());
    }
}

bool WindowSceneSessionImpl::hasAncestorFloatSession(uint32_t parentId, const SessionMap& sessionMap)
{
    if (parentId == INVALID_SESSION_ID) {
        TLOGW(WmsLogTag::WMS_SUB, "invalid parent id");
        return false;
    }
    for (const auto& [_, pair] : sessionMap) {
        auto& window = pair.second;
        if (window && window->GetWindowId() == parentId) {
            if (window->GetType() == WindowType::WINDOW_TYPE_FLOAT) {
                TLOGD(WmsLogTag::WMS_SUB, "find float session, id:%{public}u", window->GetWindowId());
                return true;
            } else if (WindowHelper::IsSubWindow(window->GetType())) {
                return hasAncestorFloatSession(window->GetParentId(), sessionMap);
            }
            return false;
        }
    }
    TLOGD(WmsLogTag::WMS_SUB, "don't find float session, parentId:%{public}u", parentId);
    return false;
}

WMError WindowSceneSessionImpl::GetParentSessionAndVerify(bool isToast, sptr<WindowSessionImpl>& parentSession)
{
    if (isToast) {
        auto parentWindow = GetWindowWithId(property_->GetParentId());
        std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
        if (parentWindow && hasAncestorFloatSession(parentWindow->GetWindowId(), windowSessionMap_)) {
            TLOGI(WmsLogTag::WMS_SUB, "parentId: %{public}u exist ancestor float window", property_->GetParentId());
            parentSession = parentWindow;
        } else {
            parentSession = FindParentMainSession(property_->GetParentId(), windowSessionMap_);
        }
    } else {
        parentSession = FindParentSessionByParentId(property_->GetParentId());
    }
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "parent of sub window is nullptr, name: %{public}s, type: %{public}d",
            property_->GetWindowName().c_str(), GetType());
        return WMError::WM_ERROR_NULLPTR;
    }
    return WindowSceneSessionImpl::VerifySubWindowLevel(false, parentSession);
}

WMError WindowSceneSessionImpl::VerifySubWindowLevel(bool isToast, const sptr<WindowSessionImpl>& parentSession)
{
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "parent of sub window is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!isToast && !parentSession->GetIsUIExtFirstSubWindow() &&
        parentSession->GetProperty()->GetSubWindowLevel() >= 1 &&
        !parentSession->IsPcOrFreeMultiWindowCapabilityEnabled()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return WMError::WM_OK;
}

static void AdjustPropertySessionInfo(const std::shared_ptr<AbilityRuntime::Context>& context, SessionInfo& info)
{
    if (!context) {
        TLOGE(WmsLogTag::WMS_LIFE, "context is null");
        return;
    }
    info.moduleName_ = context->GetHapModuleInfo() ? context->GetHapModuleInfo()->moduleName : "";
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (abilityContext && abilityContext->GetAbilityInfo()) {
        info.abilityName_ = abilityContext->GetAbilityInfo()->name;
        info.bundleName_ = abilityContext->GetAbilityInfo()->bundleName;
    } else {
        info.bundleName_ = context->GetBundleName();
    }
}

WMError WindowSceneSessionImpl::CreateAndConnectSpecificSession()
{
    sptr<ISessionStage> iSessionStage(this);
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(iSessionStage);
    auto persistentId = INVALID_SESSION_ID;
    sptr<Rosen::ISession> session;
    auto context = GetContext();
    sptr<IRemoteObject> token = context ? context->GetToken() : nullptr;
    if (token) {
        property_->SetTokenState(true);
    }
    AdjustPropertySessionInfo(context, property_->EditSessionInfo());

    const WindowType type = GetType();
    bool hasToastFlag = property_->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TOAST);
    if (WindowHelper::IsSubWindow(type) && (property_->GetIsUIExtFirstSubWindow() ||
                                            (property_->GetIsUIExtAnySubWindow() && hasToastFlag))) {
        property_->SetParentPersistentId(property_->GetParentId());
        SetDefaultDisplayIdIfNeed();
        property_->SetIsUIExtensionAbilityProcess(isUIExtensionAbilityProcess_);
        // create sub session by parent session
        SingletonContainer::Get<WindowAdapter>().CreateAndConnectSpecificSession(iSessionStage, eventChannel,
            surfaceNode_, property_, persistentId, session, windowSystemConfig_, token);
        if (!hasToastFlag) {
            AddSubWindowMapForExtensionWindow();
        }
    } else if (WindowHelper::IsSubWindow(type)) {
        sptr<WindowSessionImpl> parentSession = nullptr;
        auto ret = WindowSceneSessionImpl::GetParentSessionAndVerify(hasToastFlag, parentSession);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        property_->SetDisplayId(parentSession->GetDisplayId());
        // set parent persistentId
        auto parentWindowId = parentSession->GetPersistentId();
        property_->SetParentPersistentId(parentWindowId);
        property_->SetIsPcAppInPad(parentSession->GetProperty()->GetIsPcAppInPad());
        property_->SetPcAppInpadCompatibleMode(parentSession->GetProperty()->GetPcAppInpadCompatibleMode());
        // creat sub session by parent session
        SingletonContainer::Get<WindowAdapter>().CreateAndConnectSpecificSession(iSessionStage, eventChannel,
            surfaceNode_, property_, persistentId, session, windowSystemConfig_, token);
        {
            std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
            // update subWindowSessionMap_
            subWindowSessionMap_[parentWindowId].push_back(this);
        }
        SetTargetAPIVersion(parentSession->GetTargetAPIVersion());
    } else { // system window
        WMError createSystemWindowRet = CreateSystemWindow(type);
        if (createSystemWindowRet != WMError::WM_OK) {
            return createSystemWindowRet;
        }
        auto parentSession = FindParentSessionByParentId(property_->GetParentPersistentId());
        if (parentSession != nullptr) {
            property_->SetIsPcAppInPad(parentSession->GetProperty()->GetIsPcAppInPad());
        }
        PreProcessCreate();
        SingletonContainer::Get<WindowAdapter>().CreateAndConnectSpecificSession(iSessionStage, eventChannel,
            surfaceNode_, property_, persistentId, session, windowSystemConfig_, token);
    }
    property_->SetPersistentId(persistentId);
    if (session == nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "create specific failed, session is nullptr, name: %{public}s",
            property_->GetWindowName().c_str());
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = session;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "name:%{public}s,id:%{public}d,parentId:%{public}d,type:%{public}u,"
        "touchable:%{public}d,displayId:%{public}" PRIu64, property_->GetWindowName().c_str(),
        property_->GetPersistentId(), property_->GetParentPersistentId(), GetType(),
        property_->GetTouchable(), property_->GetDisplayId());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::CreateSystemWindow(WindowType type)
{
    if (WindowHelper::IsAppFloatingWindow(type) || WindowHelper::IsPipWindow(type) ||
        type == WindowType::WINDOW_TYPE_TOAST || WindowHelper::IsFbWindow(type)) {
        property_->SetParentPersistentId(GetFloatingWindowParentId());
        TLOGI(WmsLogTag::WMS_SYSTEM, "parentId: %{public}d, type: %{public}d",
            property_->GetParentPersistentId(), type);
        auto mainWindow = FindMainWindowWithContext();
        property_->SetFloatingWindowAppType(mainWindow != nullptr ? true : false);
        if (mainWindow != nullptr) {
            if (property_->GetDisplayId() == DISPLAY_ID_INVALID) {
                property_->SetDisplayId(mainWindow->GetDisplayId());
            }
        }
    } else if (type == WindowType::WINDOW_TYPE_DIALOG) {
        if (auto mainWindow = FindMainWindowWithContext()) {
            property_->SetParentPersistentId(mainWindow->GetPersistentId());
            property_->SetDisplayId(mainWindow->GetDisplayId());
            TLOGI(WmsLogTag::WMS_DIALOG, "The parentId: %{public}d", mainWindow->GetPersistentId());
        }
    } else if (WindowHelper::IsSystemSubWindow(type)) {
        auto parentSession = FindParentSessionByParentId(property_->GetParentId());
        if (parentSession == nullptr || parentSession->GetHostSession() == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "parent of system sub window, name: %{public}s, type: %{public}d",
                property_->GetWindowName().c_str(), type);
            return WMError::WM_ERROR_NULLPTR;
        }
        if (WindowHelper::IsSystemSubWindow(parentSession->GetType())) {
            TLOGE(WmsLogTag::WMS_LIFE, "parent is system sub window, name: %{public}s, type: %{public}d",
                property_->GetWindowName().c_str(), type);
            return WMError::WM_ERROR_INVALID_TYPE;
        }
        // set parent persistentId
        property_->SetParentPersistentId(parentSession->GetPersistentId());
        property_->SetDisplayId(parentSession->GetDisplayId());
    }
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::RecoverSessionProperty()
{
    // recover colorMode
    if (auto hostSession = GetHostSession()) {
        hostSession->OnUpdateColorMode(colorMode_, hasDarkRes_);
    }
}

WMError WindowSceneSessionImpl::RecoverAndConnectSpecificSession()
{
    TLOGI(WmsLogTag::WMS_RECOVER, "windowName=%{public}s, windowMode=%{public}u, windowType=%{public}u, "
        "persistentId=%{public}d, windowState=%{public}d, requestWindowState=%{public}d, parentId=%{public}d",
        GetWindowName().c_str(), property_->GetWindowMode(), property_->GetWindowType(), GetPersistentId(), state_,
        requestState_, property_->GetParentId());

    const WindowType type = GetType();
    if (WindowHelper::IsSubWindow(type) && !property_->GetIsUIExtFirstSubWindow()) { // sub window
        TLOGD(WmsLogTag::WMS_RECOVER, "SubWindow");
        auto parentSession = FindParentSessionByParentId(property_->GetParentId());
        if (parentSession == nullptr || parentSession->GetHostSession() == nullptr) {
            TLOGE(WmsLogTag::WMS_RECOVER, "parentSession is null");
            return WMError::WM_ERROR_NULLPTR;
        }
    }
    if (WindowHelper::IsPipWindow(type)) {
        TLOGI(WmsLogTag::WMS_RECOVER, "pipWindow");
        PictureInPictureManager::DoClose(true, true);
        return WMError::WM_OK;
    }
    windowRecoverStateChangeFunc_(true, WindowRecoverState::WINDOW_START_RECONNECT);
    sptr<ISessionStage> iSessionStage(this);
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(iSessionStage);
    sptr<Rosen::ISession> session = nullptr;
    auto context = GetContext();
    sptr<IRemoteObject> token = context ? context->GetToken() : nullptr;
    windowRecoverStateChangeFunc_(true, WindowRecoverState::WINDOW_DOING_RECONNECT);
    SingletonContainer::Get<WindowAdapter>().RecoverAndConnectSpecificSession(
        iSessionStage, eventChannel, surfaceNode_, property_, session, token);

    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Recover failed, session is nullptr");
        windowRecoverStateChangeFunc_(true, WindowRecoverState::WINDOW_NOT_RECONNECT);
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = session;
    }
    windowRecoverStateChangeFunc_(true, WindowRecoverState::WINDOW_FINISH_RECONNECT);
    TLOGI(WmsLogTag::WMS_RECOVER,
        "over, windowName=%{public}s, persistentId=%{public}d",
        GetWindowName().c_str(), GetPersistentId());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::RecoverAndReconnectSceneSession()
{
    TLOGI(WmsLogTag::WMS_RECOVER, "windowName=%{public}s, windowMode=%{public}u, windowType=%{public}u, "
        "persistentId=%{public}d, windowState=%{public}d, requestWindowState=%{public}d", GetWindowName().c_str(),
        property_->GetWindowMode(), property_->GetWindowType(), GetPersistentId(), state_, requestState_);

    if (isFocused_) {
        UpdateFocusState(false);
    }
    auto context = GetContext();
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (context && context->GetHapModuleInfo() && abilityContext && abilityContext->GetAbilityInfo()) {
        if (!abilityContext->IsHook() || (abilityContext->IsHook() && abilityContext->GetHookOff())) {
            property_->EditSessionInfo().abilityName_ = abilityContext->GetAbilityInfo()->name;
            property_->EditSessionInfo().moduleName_ = context->GetHapModuleInfo()->moduleName;
        }
    } else {
        TLOGE(WmsLogTag::WMS_RECOVER, "context or abilityContext is null, recovered session failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto& info = property_->EditSessionInfo();
    if (auto want = abilityContext->GetWant()) {
        info.want = want;
    } else {
        TLOGE(WmsLogTag::WMS_RECOVER, "want is nullptr!");
    }
    property_->SetIsFullScreenInForceSplitMode(isFullScreenInForceSplit_.load());
    windowRecoverStateChangeFunc_(false, WindowRecoverState::WINDOW_START_RECONNECT);
    sptr<ISessionStage> iSessionStage(this);
    sptr<IWindowEventChannel> iWindowEventChannel = sptr<WindowEventChannel>::MakeSptr(iSessionStage);
    sptr<IRemoteObject> token = context ? context->GetToken() : nullptr;
    sptr<Rosen::ISession> session = nullptr;
    windowRecoverStateChangeFunc_(false, WindowRecoverState::WINDOW_DOING_RECONNECT);
    auto ret = SingletonContainer::Get<WindowAdapter>().RecoverAndReconnectSceneSession(
        iSessionStage, iWindowEventChannel, surfaceNode_, session, property_, token);
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "session is null, recovered session failed");
        windowRecoverStateChangeFunc_(false, WindowRecoverState::WINDOW_NOT_RECONNECT);
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = session;
    }
    windowRecoverStateChangeFunc_(false, WindowRecoverState::WINDOW_FINISH_RECONNECT);
    TLOGI(WmsLogTag::WMS_RECOVER, "Successful, persistentId=%{public}d", GetPersistentId());
    return static_cast<WMError>(ret);
}

std::string WindowSceneSessionImpl::TransferLifeCycleEventToString(LifeCycleEvent type) const
{
    std::string event;
    switch (type) {
        case LifeCycleEvent::CREATE_EVENT:
            event = "CREATE";
            break;
        case LifeCycleEvent::SHOW_EVENT:
            event = "SHOW";
            break;
        case LifeCycleEvent::HIDE_EVENT:
            event = "HIDE";
            break;
        case LifeCycleEvent::DESTROY_EVENT:
            event = "DESTROY";
            break;
        default:
            event = "UNDEFINE";
            break;
    }
    return event;
}

void WindowSceneSessionImpl::RecordLifeCycleExceptionEvent(LifeCycleEvent event, WMError errCode) const
{
    if (!(errCode > WMError::WM_ERROR_NEED_REPORT_BASE && errCode < WMError::WM_ERROR_PIP_REPEAT_OPERATION)) {
        return;
    }
    std::ostringstream oss;
    oss << "life cycle is abnormal: " << "window_name: " << GetWindowName().c_str()
        << ", id:" << GetWindowId() << ", event: " << TransferLifeCycleEventToString(event)
        << ", errCode: " << static_cast<int32_t>(errCode) << ";";
    std::string info = oss.str();
    TLOGI(WmsLogTag::WMS_LIFE, "window life cycle exception: %{public}s", info.c_str());
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "WINDOW_LIFE_CYCLE_EXCEPTION",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", getpid(),
        "UID", getuid(),
        "MSG", info);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "write HiSysEvent error, ret:%{public}d", ret);
    }
}

void WindowSceneSessionImpl::UpdateWindowState()
{
    {
        std::unique_lock<std::shared_mutex> lock(windowSessionMutex_);
        windowSessionMap_.insert(std::make_pair(property_->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(property_->GetPersistentId(), this)));
    }
    state_ = WindowState::STATE_CREATED;
    requestState_ = WindowState::STATE_CREATED;
    WindowType windowType = GetType();
    if (WindowHelper::IsMainWindow(windowType)) {
        if (property_->GetIsNeedUpdateWindowMode()) {
            WLOGFI("UpdateWindowMode %{public}u mode %{public}u",
                GetWindowId(), static_cast<uint32_t>(property_->GetWindowMode()));
            UpdateWindowModeImmediately(property_->GetWindowMode());
            property_->SetIsNeedUpdateWindowMode(false);
        } else {
            SetWindowMode(windowSystemConfig_.defaultWindowMode_);
        }
        NotifyWindowNeedAvoid(
            (property_->GetWindowFlags()) & (static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)));
        GetConfigurationFromAbilityInfo();
    } else {
        bool isSubWindow = WindowHelper::IsSubWindow(windowType);
        bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
        bool isSysytemWindow = WindowHelper::IsSystemWindow(windowType);
        UpdateWindowSizeLimits();
        if ((isSubWindow || isDialogWindow || isSysytemWindow) && IsWindowDraggable()) {
            WLOGFD("sync window limits to server side to make size limits work while resizing");
            UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
        }
    }
}

WMError WindowSceneSessionImpl::Create(const std::shared_ptr<AbilityRuntime::Context>& context,
    const sptr<Rosen::ISession>& iSession, const std::string& identityToken, bool isModuleAbilityHookEnd)
{
    TLOGI(WmsLogTag::WMS_LIFE, "Window Create name:%{public}s, state:%{public}u, mode:%{public}u",
        property_->GetWindowName().c_str(), state_, GetWindowMode());
    // allow iSession is nullptr when create window by innerkits
    if (!context) {
        TLOGW(WmsLogTag::WMS_LIFE, "context is nullptr");
    }
    WMError ret = WindowSessionCreateCheck();
    if (ret != WMError::WM_OK) {
        return ret;
    }
    // Since here is init of this window, no other threads will rw it.
    hostSession_ = iSession;
    SetContext(context);
    identityToken_ = identityToken;
    property_->SetIsAbilityHookOff(isModuleAbilityHookEnd);
    TLOGI(WmsLogTag::WMS_LIFE, "SetIsAbilityHookOff %{public}d", isModuleAbilityHookEnd);
    AdjustWindowAnimationFlag();
    if (context && context->GetApplicationInfo() &&
        context->GetApplicationInfo()->apiCompatibleVersion >= 9 && // 9: api version
        !SessionPermission::IsSystemCalling()) {
        WLOGI("Remove window flag WINDOW_FLAG_SHOW_WHEN_LOCKED");
        property_->SetWindowFlags(property_->GetWindowFlags() &
            (~(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED))));
    }
    int32_t zLevel = GetSubWindowZLevelByFlags(GetType(), GetWindowFlags(), IsTopmost());
    if (zLevel != NORMAL_SUB_WINDOW_Z_LEVEL) {
        property_->SetSubWindowZLevel(zLevel);
    }

    bool isSpecificSession = false;
    const auto& initRect = GetRequestRect();
    if (GetHostSession()) { // main window
        SetDefaultDisplayIdIfNeed();
        SetTargetAPIVersion(SysCapUtil::GetApiCompatibleVersion());
        ret = Connect();
        TLOGD(WmsLogTag::WMS_PC, "targeAPItVersion: %{public}d", GetTargetAPIVersion());
    } else { // system or sub window
        TLOGI(WmsLogTag::WMS_LIFE, "Create system or sub window with type=%{public}d", GetType());
        isSpecificSession = true;
        const auto& type = GetType();
        if (WindowHelper::IsSystemWindow(type)) {
            // Not valid system window type for session should return WMError::WM_OK;
            if (!IsValidSystemWindowType(type)) {
                return WMError::WM_ERROR_INVALID_CALLING;
            }
            if (INVALID_SCB_WINDOW_TYPE.find(type) != INVALID_SCB_WINDOW_TYPE.end()) {
                TLOGI(WmsLogTag::WMS_SYSTEM, "Invalid SCB type: %{public}u", type);
                return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
            }
            InitSystemSessionDragEnable();
        } else if (!WindowHelper::IsSubWindow(type)) {
            TLOGI(WmsLogTag::WMS_LIFE, "create failed not system or sub type, type: %{public}d", type);
            return WMError::WM_ERROR_INVALID_CALLING;
        }
        InitSubSessionDragEnable();
        isEnableDefaultDensityWhenCreate_ = windowOption_->IsDefaultDensityEnabled();
        ret = CreateAndConnectSpecificSession();
    }

    RecordLifeCycleExceptionEvent(LifeCycleEvent::CREATE_EVENT, ret);
    if (ret == WMError::WM_OK) {
        MakeSubOrDialogWindowDragableAndMoveble();
        UpdateWindowState();
        RegisterWindowRecoverStateChangeListener();
        RegisterSessionRecoverListener(isSpecificSession);
        UpdateDefaultStatusBarColor();
        AddSetUIContentTimeoutCheck();
        SetUIExtensionDestroyCompleteInSubWindow();
        SetSubWindowZLevelToProperty();
        InputTransferStation::GetInstance().AddInputWindow(this);
        if (WindowHelper::IsSubWindow(GetType()) && !initRect.IsUninitializedRect()) {
            auto hostSession = GetHostSession();
            if (IsFullScreenSizeWindow(initRect.width_, initRect.height_) && (hostSession != nullptr)) {
                // Full screen size sub window don't need to resize when dpi change
                TLOGI(WmsLogTag::WMS_LIFE, "Full screen size sub window set isDefaultDensityEnabled true");
                hostSession->OnDefaultDensityEnabled(true);
            }
            Resize(initRect.width_, initRect.height_);
        }
        RegisterWindowInspectorCallback();
        UpdateColorMode();
        SetPcAppInpadSpecificSystemBarInvisible();
        SetPcAppInpadOrientationLandscape();
        SetDefaultDensityEnabledValue(IsStageDefaultDensityEnabled());
        RegisterListenerForKeyboard();
    }
    UpdateAnimationSpeedIfEnabled();
    TLOGI(WmsLogTag::WMS_LIFE, "Window Create success [name:%{public}s, id:%{public}d], state:%{public}u, "
        "mode:%{public}u, enableDefaultDensity:%{public}d, displayId:%{public}" PRIu64,
        property_->GetWindowName().c_str(), property_->GetPersistentId(), state_, GetWindowMode(),
        isEnableDefaultDensityWhenCreate_, property_->GetDisplayId());
    return ret;
}

void WindowSceneSessionImpl::RegisterListenerForKeyboard()
{
    if (GetType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return;
    }
    SingletonContainer::Get<ScreenManager>().RegisterScreenListener(sptr<WindowScreenListener>::MakeSptr());
}

void WindowSceneSessionImpl::UpdateAnimationSpeedIfEnabled()
{
    if (!isEnableAnimationSpeed_.load()) {
        return;
    }

    TLOGI(WmsLogTag::WMS_ANIMATION, "isEnableAnimationSpeed_ is true");
    auto rsUIContext = WindowSessionImpl::GetRSUIContext();
    auto implicitAnimator = rsUIContext ? rsUIContext->GetRSImplicitAnimator() : nullptr;
    if (implicitAnimator != nullptr) {
        implicitAnimator->ApplyAnimationSpeedMultiplier(animationSpeed_.load());
        TLOGI(WmsLogTag::WMS_ANIMATION, "update animation speed success");
    }
}

WMError WindowSceneSessionImpl::SetPcAppInpadSpecificSystemBarInvisible()
{
    TLOGI(WmsLogTag::WMS_COMPAT, "isPcAppInpadSpecificSystemBarInvisible: %{public}d",
        property_->GetPcAppInpadSpecificSystemBarInvisible());
    if (WindowHelper::IsMainWindow(GetType()) && IsPadAndNotFreeMultiWindowCompatibleMode() &&
        property_->GetPcAppInpadSpecificSystemBarInvisible()) {
        SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        UpdateSpecificSystemBarEnabled(false, false, statusProperty);
        SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusProperty);

        SystemBarProperty NavigationIndicatorPorperty =
            GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
        UpdateSpecificSystemBarEnabled(false, false, NavigationIndicatorPorperty);
        SetSpecificBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, NavigationIndicatorPorperty);
        return WMError::WM_OK;
    }
    return WMError::WM_ERROR_INVALID_CALLING;
}

WMError WindowSceneSessionImpl::SetPcAppInpadOrientationLandscape()
{
    TLOGI(WmsLogTag::WMS_COMPAT, "isPcAppInpadOrientationLandscape: %{public}d",
        property_->GetPcAppInpadOrientationLandscape());
    if (WindowHelper::IsMainWindow(GetType()) && IsPadAndNotFreeMultiWindowCompatibleMode() &&
        property_->GetPcAppInpadOrientationLandscape()) {
        SetRequestedOrientation(Orientation::HORIZONTAL, false);
        return WMError::WM_OK;
    }
    return WMError::WM_ERROR_INVALID_CALLING;
}

bool WindowSceneSessionImpl::IsFullScreenSizeWindow(uint32_t width, uint32_t height)
{
    DisplayId displayId = property_->GetDisplayId();
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "display is null");
        return false;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "displayInfo is null");
        return false;
    }
    uint32_t displayWidth = static_cast<uint32_t>(displayInfo->GetWidth());
    uint32_t displayHeight = static_cast<uint32_t>(displayInfo->GetHeight());
    if (displayWidth == width && displayHeight == height) {
        return true;
    }
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice() ||
        DisplayManager::GetInstance().GetFoldStatus() != FoldStatus::HALF_FOLD) {
        return false;
    }
    // if is super fold device and in half fold state, check virtual screen
    auto virtualDisplay = SingletonContainer::Get<DisplayManager>().GetDisplayById(DISPLAY_ID_C);
    if (virtualDisplay == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "virtual display is null");
        return false;
    }
    auto virtualDisplayInfo = virtualDisplay->GetDisplayInfo();
    if (virtualDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "virtual displayInfo is null");
        return false;
    }
    displayWidth = static_cast<uint32_t>(virtualDisplayInfo->GetWidth());
    displayHeight = static_cast<uint32_t>(virtualDisplayInfo->GetHeight());
    if (displayWidth == width && displayHeight == height) {
        return true;
    }
    return false;
}

WMError WindowSceneSessionImpl::SetParentWindowInner(int32_t oldParentWindowId,
    const sptr<WindowSessionImpl>& newParentWindow)
{
    auto newParentWindowId = newParentWindow->GetPersistentId();
    auto subWindowId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_SUB, "subWindowId: %{public}d, oldParentWindowId: %{public}d, "
        "newParentWindowId: %{public}d", subWindowId, oldParentWindowId, newParentWindowId);
    WMError ret = SingletonContainer::Get<WindowAdapter>().SetParentWindow(subWindowId, newParentWindowId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d set parent window failed errCode: %{public}d",
            subWindowId, static_cast<int32_t>(ret));
        return ret;
    }
    RemoveSubWindow(oldParentWindowId);
    {
        std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
        subWindowSessionMap_[newParentWindowId].push_back(this);
    }
    property_->SetParentPersistentId(newParentWindowId);
    UpdateSubWindowInfo(newParentWindow->GetProperty()->GetSubWindowLevel() + 1,
        newParentWindow->GetContext());
    if (state_ == WindowState::STATE_SHOWN &&
        newParentWindow->GetWindowState() == WindowState::STATE_HIDDEN) {
        UpdateSubWindowStateAndNotify(newParentWindowId, WindowState::STATE_HIDDEN);
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetParentWindow(int32_t newParentWindowId)
{
    auto subWindowId = GetPersistentId();
    if (property_->GetPcAppInpadCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "This is PcAppInPad, not Supported");
        return WMError::WM_OK;
    }
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d device not support", subWindowId);
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d called by invalid window type %{public}d",
            subWindowId, GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto oldParentWindowId = property_->GetParentPersistentId();
    if (oldParentWindowId == newParentWindowId || subWindowId == newParentWindowId) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d newParentWindowId is the same as "
            "oldParentWindowId or subWindowId", subWindowId);
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    auto oldParentWindow = GetWindowWithId(oldParentWindowId);
    if (oldParentWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d can not find old parent window By Id: %{public}d",
            subWindowId, oldParentWindowId);
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    auto oldWindowType = oldParentWindow->GetType();
    if (!WindowHelper::IsMainWindow(oldWindowType) && !WindowHelper::IsFloatOrSubWindow(oldWindowType)) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d old parent window type invalid", subWindowId);
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    auto newParentWindow = GetWindowWithId(newParentWindowId);
    if (newParentWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d can not find new parent window By Id: %{public}d",
            subWindowId, newParentWindowId);
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    auto newWindowType = newParentWindow->GetType();
    if (!WindowHelper::IsMainWindow(newWindowType) && !WindowHelper::IsFloatOrSubWindow(newWindowType)) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d new parent window type invalid", subWindowId);
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    return SetParentWindowInner(oldParentWindowId, newParentWindow);
}

WMError WindowSceneSessionImpl::GetParentWindow(sptr<Window>& parentWindow)
{
    if (property_->GetPcAppInpadCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "This is PcAppInPad, not Supported");
        return WMError::WM_OK;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d called by invalid window type %{public}d",
            GetPersistentId(), GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (property_->GetIsUIExtFirstSubWindow()) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d UIExtension sub window not get parent window",
            GetPersistentId());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    parentWindow = FindWindowById(property_->GetParentPersistentId());
    if (parentWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "winId: %{public}d parentWindow is nullptr", GetPersistentId());
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::UpdateDefaultStatusBarColor()
{
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u not main window", GetPersistentId());
        return;
    }
    uint32_t contentColor = 0;
    auto ret = UpdateStatusBarColorByColorMode(contentColor);
    if (ret != WMError::WM_OK) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u no need update", GetPersistentId());
        return;
    }
    SystemBarProperty statusBarProp = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    statusBarProp.contentColor_ = contentColor;
    statusBarProp.settingFlag_ = static_cast<SystemBarSettingFlag>(
        static_cast<uint32_t>(statusBarProp.settingFlag_) |
        static_cast<uint32_t>(SystemBarSettingFlag::FOLLOW_SETTING));
    SystemBarPropertyFlag systemBarPropertyFlag = { false, false, true, false };
    property_->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusBarProp);
    if (!isAtomicServiceUseColor_) {
        UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, statusBarProp, systemBarPropertyFlag);
    }
}

void WindowSceneSessionImpl::InitSubSessionDragEnable()
{
    if (WindowHelper::IsSubWindow(GetType()) && !IsPcOrPadFreeMultiWindowMode()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "windId: %{public}d init subWindow dragEnable false",
            property_->GetPersistentId());
        property_->SetDragEnabled(false);
    }
}

void WindowSceneSessionImpl::InitSystemSessionDragEnable()
{
    if (WindowHelper::IsDialogWindow(GetType())) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "dialogWindow default draggable, should not init false, id: %{public}d",
            GetPersistentId());
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "windId: %{public}d init dragEnable false",
        GetPersistentId());
    property_->SetDragEnabled(false);
}

void WindowSceneSessionImpl::RegisterSessionRecoverListener(bool isSpecificSession)
{
    TLOGD(WmsLogTag::WMS_RECOVER, "Id=%{public}d, isSpecificSession=%{public}s",
        GetPersistentId(), isSpecificSession ? "true" : "false");

    if (GetType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        TLOGI(WmsLogTag::WMS_RECOVER, "input method window does not need to recover");
        return;
    }
    if (property_->GetCollaboratorType() != CollaboratorType::DEFAULT_TYPE) {
        TLOGI(WmsLogTag::WMS_RECOVER, "collaboratorType is %{public}" PRId32 ", not need to recover",
            property_->GetCollaboratorType());
        return;
    }

    wptr<WindowSceneSessionImpl> weakThis = this;
    auto callbackFunc = [weakThis, isSpecificSession] {
        auto promoteThis = weakThis.promote();
        if (promoteThis == nullptr) {
            TLOGW(WmsLogTag::WMS_RECOVER, "promoteThis is nullptr");
            return WMError::WM_ERROR_NULLPTR;
        }
        if (promoteThis->state_ == WindowState::STATE_DESTROYED) {
            TLOGW(WmsLogTag::WMS_RECOVER, "windowState is STATE_DESTROYED, no need to recover");
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }
        if (promoteThis->windowRecoverStateChangeFunc_ == nullptr) {
            TLOGW(WmsLogTag::WMS_RECOVER, "windowRecoverStateChangeFunc_ is nullptr");
            return WMError::WM_ERROR_NULLPTR;
        }

        auto ret = isSpecificSession ? promoteThis->RecoverAndConnectSpecificSession() :
			promoteThis->RecoverAndReconnectSceneSession();

        TLOGD(WmsLogTag::WMS_RECOVER, "Recover session over, ret=%{public}d", ret);
        return ret;
    };
    SingletonContainer::Get<WindowAdapter>().RegisterSessionRecoverCallbackFunc(GetPersistentId(), callbackFunc);
}

void WindowSceneSessionImpl::RegisterWindowRecoverStateChangeListener()
{
    windowRecoverStateChangeFunc_ = [weakThis = wptr(this)](bool isSpecificSession,
        const WindowRecoverState& state) THREAD_SAFETY_GUARD(SCENE_GUARD) {
        auto window = weakThis.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "window is null");
            return;
        }
        window->OnWindowRecoverStateChange(isSpecificSession, state);
    };
}

void WindowSceneSessionImpl::OnWindowRecoverStateChange(bool isSpecificSession, const WindowRecoverState& state)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "id: %{public}d, state:%{public}u", GetPersistentId(), state);
    switch (state) {
        case WindowRecoverState::WINDOW_START_RECONNECT:
            UpdateStartRecoverProperty(isSpecificSession);
            UpdateStartRecoverEventFlag();
            break;
        case WindowRecoverState::WINDOW_FINISH_RECONNECT:
            UpdateFinishRecoverProperty(isSpecificSession);
            RecoverSessionListener();
            RecoverSessionProperty();
            break;
        default:
            break;
    }
}

void WindowSceneSessionImpl::UpdateStartRecoverEventFlag()
{
    std::unique_lock<std::shared_mutex> lock(windowSessionMutex_);
    property_->EditSessionInfo().isReceiveDragEventEnabled_ = isReceiveDragEventEnabled_;
    property_->EditSessionInfo().isSeparationTouchEnabled_ = isSeparationTouchEnabled_;
}

void WindowSceneSessionImpl::UpdateStartRecoverProperty(bool isSpecificSession)
{
    if (isSpecificSession) {
        property_->SetWindowState(requestState_);
        if (GetContext() && GetContext()->GetToken()) {
            property_->SetTokenState(true);
        }
    } else {
        property_->SetWindowState(state_);
        property_->SetIsFullScreenWaterfallMode(isFullScreenWaterfallMode_.load());
    }
}

void WindowSceneSessionImpl::UpdateFinishRecoverProperty(bool isSpecificSession)
{
    property_->SetWindowState(state_);
}

bool WindowSceneSessionImpl::HandlePointDownEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const MMI::PointerEvent::PointerItem& pointerItem)
{
    bool needNotifyEvent = true;
    WindowType windowType = property_->GetWindowType();
    AreaType dragType = GetDragAreaByDownEvent(pointerEvent, pointerItem);
    TLOGD(WmsLogTag::WMS_EVENT, "dragType: %{public}d", dragType);
    bool isDecorDialog = windowType == WindowType::WINDOW_TYPE_DIALOG && property_->IsDecorEnable();
    bool isFixedSubWin = WindowHelper::IsSubWindow(windowType) && !IsWindowDraggable();
    bool isFixedSystemWin = WindowHelper::IsSystemWindow(windowType) && !IsWindowDraggable();
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, needNotifyEvent);
    TLOGD(WmsLogTag::WMS_EVENT, "isFixedSystemWin %{public}d, isFixedSubWin %{public}d, isDecorDialog %{public}d",
        isFixedSystemWin, isFixedSubWin, isDecorDialog);
    if ((isFixedSystemWin || isFixedSubWin) && !isDecorDialog) {
        hostSession->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise_);
    } else {
        if (dragType != AreaType::UNDEFINED) {
            hostSession->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise_);
            needNotifyEvent = false;
        } else if (WindowHelper::IsMainWindow(windowType) ||
                   WindowHelper::IsSubWindow(windowType) ||
                   WindowHelper::IsSystemWindow(windowType)) {
            hostSession->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise_);
        } else {
            hostSession->ProcessPointDownSession(pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
        }
    }
    return needNotifyEvent;
}

AreaType WindowSceneSessionImpl::GetDragAreaByDownEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const MMI::PointerEvent::PointerItem& pointerItem)
{
    AreaType dragType = AreaType::UNDEFINED;
    float vpr = WindowSessionImpl::GetVirtualPixelRatio();
    if (MathHelper::NearZero(vpr)) {
        return dragType;
    }
    const auto& sourceType = pointerEvent->GetSourceType();
    int outside = (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) ? static_cast<int>(HOTZONE_POINTER * vpr) :
        static_cast<int>(HOTZONE_TOUCH * vpr);
    int32_t winX = pointerItem.GetWindowX();
    int32_t winY = pointerItem.GetWindowY();
    WindowType windowType = property_->GetWindowType();
    bool isSystemDraggableType = WindowHelper::IsSystemWindow(windowType) && IsWindowDraggable();
    bool isFloatingDraggableType =
        property_->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING && IsWindowDraggable();
    const auto& rect = SessionHelper::TransferToWSRect(GetRect());
    auto limits = property_->GetWindowLimits();
    TLOGD(WmsLogTag::WMS_EVENT, "%{public}s [minWidth,maxWidth,minHeight,maxHeight]: %{public}d,"
        " %{public}d, %{public}d, %{public}d", GetWindowName().c_str(), limits.minWidth_,
        limits.maxWidth_, limits.minHeight_, limits.maxHeight_);
    if (isFloatingDraggableType || isSystemDraggableType) {
        dragType = SessionHelper::GetAreaType(winX, winY, sourceType, outside, vpr, rect, limits);
    }
    return dragType;
}

void WindowSceneSessionImpl::ResetSuperFoldDisplayY(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (superFoldOffsetY_ == -1) {
        auto foldCreaseRegion = DisplayManager::GetInstance().GetCurrentFoldCreaseRegion();
        if (foldCreaseRegion == nullptr) {
            TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "foldCreaseRegion is nullptr");
            return;
        }
        const auto& creaseRects = foldCreaseRegion->GetCreaseRects();
        if (creaseRects.empty()) {
            TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "creaseRects is empty");
            return;
        }
        const auto& rect = creaseRects.front();
        superFoldOffsetY_ = rect.height_ + rect.posY_;
        TLOGI(WmsLogTag::WMS_EVENT, "height: %{public}d, posY: %{public}d", rect.height_, rect.posY_);
    }
    std::vector<int32_t> pointerIds = pointerEvent->GetPointerIds();
    for (int32_t pointerId : pointerIds) {
        MMI::PointerEvent::PointerItem pointerItem;
        if (!pointerEvent->GetPointerItem(pointerId, pointerItem)) {
            TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "Get pointerItem failed");
            return;
        }
        if (auto displayYPos = pointerItem.GetDisplayYPos();
            !MathHelper::LessNotEqual(displayYPos, superFoldOffsetY_)) {
            pointerItem.SetDisplayYPos(displayYPos - superFoldOffsetY_);
            pointerEvent->UpdatePointerItem(pointerId, pointerItem);
            pointerEvent->SetTargetDisplayId(DISPLAY_ID_C);
            TLOGD(WmsLogTag::WMS_EVENT, "Calculated superFoldOffsetY:%{public}d,displayId:%{public}d,"
                "InputId:%{public}d,pointerId:%{public}d,displayXPos:%{private}f,displayYPos:%{private}f,",
                superFoldOffsetY_, pointerEvent->GetTargetDisplayId(), pointerEvent->GetId(),
                pointerId, pointerItem.GetDisplayXPos(), pointerItem.GetDisplayYPos());
        }
    }
}

void WindowSceneSessionImpl::ConsumePointerEventInner(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem, bool isHitTargetDraggable)
{
    const int32_t& action = pointerEvent->GetPointerAction();
    const auto& sourceType = pointerEvent->GetSourceType();
    const auto& rect = SessionHelper::TransferToWSRect(GetRect());
    bool isPointDown = (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    bool needNotifyEvent = true;
    if (property_->IsAdaptToEventMapping()) {
        HandleEventForCompatibleMode(pointerEvent, pointerItem);
    }
    lastPointerEvent_ = pointerEvent;
    if (isPointDown) {
        float vpr = WindowSessionImpl::GetVirtualPixelRatio();
        if (MathHelper::NearZero(vpr)) {
            WLOGFW("vpr is zero");
            pointerEvent->MarkProcessed();
            return;
        }
        if (IsWindowDelayRaiseEnabled() && isHitTargetDraggable) {
            isExecuteDelayRaise_ = true;
        }
        needNotifyEvent = HandlePointDownEvent(pointerEvent, pointerItem);
        RefreshNoInteractionTimeoutMonitor();
    }
    bool isPointUp = (action == MMI::PointerEvent::POINTER_ACTION_UP ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP ||
        action == MMI::PointerEvent::POINTER_ACTION_CANCEL);
    if (isPointUp) {
        if (auto hostSession = GetHostSession()) {
            hostSession->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise_);
        }
    }

    bool isPointPullUp = action == MMI::PointerEvent::POINTER_ACTION_PULL_UP;
    if (isExecuteDelayRaise_ && (isPointUp || isPointPullUp)) {
        isExecuteDelayRaise_ = false;
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        property_->GetDisplayId() == DISPLAY_ID_C &&
        DisplayManager::GetInstance().GetFoldStatus() == FoldStatus::HALF_FOLD) {
        ResetSuperFoldDisplayY(pointerEvent);
    }
    if (needNotifyEvent) {
        NotifyPointerEvent(pointerEvent);
    } else {
        pointerEvent->MarkProcessed();
    }
    if (isPointDown || isPointUp) {
        TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "InputId:%{public}d,wid:%{public}u,pointId:%{public}d"
            ",srcType:%{public}d,rect:[%{public}d,%{public}d,%{public}u,%{public}u]"
            ",notify:%{public}d",
            pointerEvent->GetId(), GetWindowId(), pointerEvent->GetPointerId(),
            sourceType, rect.posX_, rect.posY_, rect.width_, rect.height_,
            needNotifyEvent);
    }
}

void WindowSceneSessionImpl::ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        WLOGFE("PointerEvent is nullptr, windowId: %{public}d", GetWindowId());
        return;
    }

    if (GetHostSession() == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "hostSession is nullptr, windowId: %{public}d", GetWindowId());
        pointerEvent->MarkProcessed();
        return;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        TLOGW(WmsLogTag::WMS_INPUT_KEY_FLOW, "invalid pointerEvent, windowId: %{public}d", GetWindowId());
        pointerEvent->MarkProcessed();
        return;
    }

    bool isPointDown = pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN ||
        pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_DOWN;
    AreaType dragType = AreaType::UNDEFINED;
    if (isPointDown) {
        dragType = GetDragAreaByDownEvent(pointerEvent, pointerItem);
    }
    if (!IsWindowDelayRaiseEnabled() || dragType != AreaType::UNDEFINED) {
        ConsumePointerEventInner(pointerEvent, pointerItem, false);
        return;
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->ProcessPointerEvent(pointerEvent,
            [weakThis = wptr(this), pointerEvent, pointerItem](bool isHitTargetDraggable) mutable {
                auto window = weakThis.promote();
                if (window == nullptr) {
                    TLOGNE(WmsLogTag::WMS_FOCUS, "window is null");
                    return;
                }
                window->ConsumePointerEventInner(pointerEvent, pointerItem, isHitTargetDraggable);
            });
    } else {
        TLOGE(WmsLogTag::WMS_FOCUS, "uiContent is nullptr, windowId: %{public}u", GetWindowId());
        pointerEvent->MarkProcessed();
    }
}

bool WindowSceneSessionImpl::PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    bool isConsumed = false;
    if (auto uiContent = GetUIContentSharedPtr()) {
        isConsumed = uiContent->ProcessKeyEvent(keyEvent, true);
    }
    RefreshNoInteractionTimeoutMonitor();
    if ((keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_TAB ||
         keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_ENTER) && isConsumed &&
        keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_DOWN) {
        TLOGD(WmsLogTag::WMS_EVENT, "wid:%{public}u, isConsumed:%{public}d", GetWindowId(), isConsumed);
        NotifyWatchGestureConsumeResult(keyEvent->GetKeyCode(), isConsumed);
    }
    SetWatchGestureConsumed(isConsumed);
    return isConsumed;
}

static void GetWindowSizeLimits(std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo,
    WindowSizeLimits& windowSizeLimits)
{
    windowSizeLimits.maxWindowWidth = windowSizeLimits.maxWindowWidth > 0 ?
        windowSizeLimits.maxWindowWidth : abilityInfo->maxWindowWidth;
    windowSizeLimits.maxWindowHeight = windowSizeLimits.maxWindowHeight > 0 ?
        windowSizeLimits.maxWindowHeight : abilityInfo->maxWindowHeight;
    windowSizeLimits.minWindowWidth = windowSizeLimits.minWindowWidth > 0 ?
        windowSizeLimits.minWindowWidth : abilityInfo->minWindowWidth;
    windowSizeLimits.minWindowHeight = windowSizeLimits.minWindowHeight > 0 ?
        windowSizeLimits.minWindowHeight : abilityInfo->minWindowHeight;
}

std::vector<AppExecFwk::SupportWindowMode> WindowSceneSessionImpl::ExtractSupportWindowModeFromMetaData(
    const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = {};
    if (windowSystemConfig_.IsPcWindow() || IsFreeMultiWindowMode()) {
        auto metadata = abilityInfo->metadata;
        for (auto item : metadata) {
            if (item.name == "ohos.ability.window.supportWindowModeInFreeMultiWindow") {
                updateWindowModes = ParseWindowModeFromMetaData(item.value);
            }
        }
    }
    if (updateWindowModes.empty()) {
        updateWindowModes = abilityInfo->windowModes;
    }
    return updateWindowModes;
}

std::vector<AppExecFwk::SupportWindowMode> WindowSceneSessionImpl::ParseWindowModeFromMetaData(
    const std::string& supportModesInFreeMultiWindow)
{
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = {};
    std::stringstream supportModes(supportModesInFreeMultiWindow);
    std::string supportWindowMode;
    while (getline(supportModes, supportWindowMode, ',')) {
        if (supportWindowMode == "fullscreen") {
            updateWindowModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
        } else if (supportWindowMode == "split") {
            updateWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
        } else if (supportWindowMode == "floating") {
            updateWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
        }
    }
    return updateWindowModes;
}

void WindowSceneSessionImpl::GetConfigurationFromAbilityInfo()
{
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(GetContext());
    if (abilityContext == nullptr) {
        WLOGFE("abilityContext is nullptr");
        return;
    }
    auto abilityInfo = abilityContext->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        WindowSizeLimits windowSizeLimits = property_->GetWindowSizeLimits();
        GetWindowSizeLimits(abilityInfo, windowSizeLimits);
        property_->SetConfigWindowLimitsVP({
            windowSizeLimits.maxWindowWidth, windowSizeLimits.maxWindowHeight,
            windowSizeLimits.minWindowWidth, windowSizeLimits.minWindowHeight,
            static_cast<float>(abilityInfo->maxWindowRatio), static_cast<float>(abilityInfo->minWindowRatio),
            1.0f, PixelUnit::VP
        });
        UpdateWindowSizeLimits();
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
        // get support modes configuration
        uint32_t windowModeSupportType = 0;
        std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
        property_->GetSupportedWindowModes(supportedWindowModes);
        auto size = supportedWindowModes.size();
        if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
            windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(supportedWindowModes);
        } else {
            std::vector<AppExecFwk::SupportWindowMode> updateWindowModes =
                ExtractSupportWindowModeFromMetaData(abilityInfo);
            if (auto hostSession = GetHostSession()) {
                hostSession->NotifySupportWindowModesChange(updateWindowModes);
            };
            windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(updateWindowModes);
        }
        if (windowModeSupportType == 0) {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "mode config param is 0, all modes is supported");
            windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
        } else {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "winId: %{public}u, windowModeSupportType: %{public}u",
                GetWindowId(), windowModeSupportType);
        }
        property_->SetWindowModeSupportType(windowModeSupportType);
        TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, windowModeSupportType: %{public}u",
            GetWindowId(), windowModeSupportType);
        // update windowModeSupportType to server
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
        bool isWindowModeSupportFullscreen = GetTargetAPIVersion() < 15 ? // 15: isolated version
            (windowModeSupportType == WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN) :
            (WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN) &&
            !WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING));
        bool onlySupportFullScreen = isWindowModeSupportFullscreen && IsPcOrPadFreeMultiWindowMode();
        bool compatibleDisableFullScreen = property_->IsFullScreenDisabled();
        if ((onlySupportFullScreen || property_->GetFullScreenStart()) && !compatibleDisableFullScreen) {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "onlySupportFullScreen:%{public}d fullScreenStart:%{public}d",
                onlySupportFullScreen, property_->GetFullScreenStart());
            Maximize(MaximizePresentation::ENTER_IMMERSIVE);
        }
    }
}

/** @note @window.layout */
uint32_t WindowSceneSessionImpl::UpdateConfigVal(uint32_t minVal, uint32_t maxVal, uint32_t configVal,
                                                 uint32_t defaultVal, float vpr)
{
    bool validConfig = minVal < (configVal * vpr) && (configVal * vpr) < maxVal;
    return validConfig ? static_cast<uint32_t>(configVal * vpr) : static_cast<uint32_t>(defaultVal * vpr);
}

/** @note @window.layout */
uint32_t WindowSceneSessionImpl::UpdateConfigValInVP(uint32_t minVal, uint32_t maxVal, uint32_t configVal,
                                                     uint32_t defaultVal, float vpr)
{
    bool validConfig = minVal < (configVal * vpr) && (configVal * vpr) < maxVal;
    return validConfig ? configVal : defaultVal;
}

/** @note @window.layout */
std::pair<WindowLimits, WindowLimits> WindowSceneSessionImpl::GetSystemSizeLimits(uint32_t displayWidth,
                                                                                  uint32_t displayHeight,
                                                                                  float vpr)
{
    WindowLimits systemLimits;                                            // Physical pixels
    WindowLimits systemLimitsVP = WindowLimits::DEFAULT_VP_LIMITS();      // Virtual pixels

    // Set maximum dimensions
    const uint32_t configMaxSize = windowSystemConfig_.maxFloatingWindowSize_;
    systemLimits.maxWidth_ = static_cast<uint32_t>(configMaxSize * vpr);
    systemLimits.maxHeight_ = static_cast<uint32_t>(configMaxSize * vpr);
    systemLimitsVP.maxWidth_ = configMaxSize;
    systemLimitsVP.maxHeight_ = configMaxSize;

    SetMinimumDimensions(systemLimits, systemLimitsVP, displayWidth, displayHeight, vpr);

    TLOGI(WmsLogTag::WMS_LAYOUT, "px[%{public}u,%{public}u,%{public}u,%{public}u], "
        "vp[%{public}u,%{public}u,%{public}u,%{public}u], configMax:%{public}u, vpr:%{public}f, "
        "winType:%{public}u", systemLimits.maxWidth_, systemLimits.maxHeight_, systemLimits.minWidth_,
        systemLimits.minHeight_, systemLimitsVP.maxWidth_, systemLimitsVP.maxHeight_, systemLimitsVP.minWidth_,
        systemLimitsVP.minHeight_, configMaxSize, vpr, GetType());
    return std::make_pair(systemLimits, systemLimitsVP);
}

/** @note @window.layout */
void WindowSceneSessionImpl::SetMinimumDimensions(WindowLimits& systemLimits, WindowLimits& systemLimitsVP,
                                                  uint32_t displayWidth, uint32_t displayHeight, float vpr)
{
    const WindowType windowType = GetType();
    // Set minimum dimensions based on window type
    if (WindowHelper::IsMainWindow(windowType)) {
        ApplyConfiguredMinSizeToLimits(systemLimits, systemLimitsVP, displayWidth, displayHeight, vpr,
                                       windowSystemConfig_.miniWidthOfMainWindow_,
                                       windowSystemConfig_.miniHeightOfMainWindow_);
    } else if (WindowHelper::IsSubWindow(windowType)) {
        ApplyConfiguredMinSizeToLimits(systemLimits, systemLimitsVP, displayWidth, displayHeight, vpr,
                                       windowSystemConfig_.miniWidthOfSubWindow_,
                                       windowSystemConfig_.miniHeightOfSubWindow_);
    } else if (WindowHelper::IsDialogWindow(windowType)) {
        ApplyConfiguredMinSizeToLimits(systemLimits, systemLimitsVP, displayWidth, displayHeight, vpr,
                                       windowSystemConfig_.miniWidthOfDialogWindow_,
                                       windowSystemConfig_.miniHeightOfDialogWindow_);
    } else if (WindowHelper::IsSystemWindow(windowType)) {
        systemLimits.minWidth_ = 0;
        systemLimits.minHeight_ = 0;
        systemLimitsVP.minWidth_ = 0;
        systemLimitsVP.minHeight_ = 0;
    } else {
        systemLimits.minWidth_ = static_cast<uint32_t>(MIN_FLOATING_WIDTH * vpr);
        systemLimits.minHeight_ = static_cast<uint32_t>(MIN_FLOATING_HEIGHT * vpr);
        systemLimitsVP.minWidth_ = MIN_FLOATING_WIDTH;
        systemLimitsVP.minHeight_ = MIN_FLOATING_HEIGHT;
    }
}

/** @note @window.layout */
void WindowSceneSessionImpl::ApplyConfiguredMinSizeToLimits(WindowLimits& systemLimits, WindowLimits& systemLimitsVP,
                                                            uint32_t displayWidth, uint32_t displayHeight, float vpr,
                                                            uint32_t configMinWidth, uint32_t configMinHeight)
{
    systemLimits.minWidth_ = UpdateConfigVal(0, displayWidth, configMinWidth, MIN_FLOATING_WIDTH, vpr);
    systemLimits.minHeight_ = UpdateConfigVal(0, displayHeight, configMinHeight, MIN_FLOATING_HEIGHT, vpr);
    systemLimitsVP.minWidth_ = UpdateConfigValInVP(0, displayWidth, configMinWidth, MIN_FLOATING_WIDTH, vpr);
    systemLimitsVP.minHeight_ = UpdateConfigValInVP(0, displayHeight, configMinHeight, MIN_FLOATING_HEIGHT, vpr);
}

/** @note @window.layout */
void WindowSceneSessionImpl::CalculateNewLimitsByLimits(WindowLimits& newLimits,
                                                        WindowLimits& newLimitsVP,
                                                        WindowLimits& customizedLimits,
                                                        float& virtualPixelRatio)
{
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "display is null");
        return;
    }

    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "displayInfo is null");
        return;
    }

    uint32_t displayWidth = static_cast<uint32_t>(displayInfo->GetWidth());
    uint32_t displayHeight = static_cast<uint32_t>(displayInfo->GetHeight());
    if (displayWidth == 0 || displayHeight == 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "displayWidth or displayHeight is zero, displayId: %{public}" PRIu64,
            property_->GetDisplayId());
        return;
    }

    virtualPixelRatio = GetVirtualPixelRatio(displayInfo);
    if (MathHelper::NearZero(virtualPixelRatio)) {
        return;
    }

    // systemLimits: physical pixels, systemLimitsVP: virtual pixels
    const auto& [systemLimits, systemLimitsVP] = GetSystemSizeLimits(displayWidth, displayHeight, virtualPixelRatio);

    customizedLimits = userLimitsSet_ ? property_->GetUserWindowLimits() : property_->GetConfigWindowLimitsVP();

    newLimits = systemLimits;
    newLimitsVP = systemLimitsVP;

    // Processing limits based on the pixel unit type
    if (customizedLimits.pixelUnit_ == PixelUnit::VP) {
        ProcessVirtualPixelLimits(newLimits, newLimitsVP, customizedLimits, systemLimitsVP, virtualPixelRatio);
    } else {
        ProcessPhysicalPixelLimits(newLimits, newLimitsVP, customizedLimits, systemLimits, virtualPixelRatio);
    }
}

/** @note @window.layout */
void WindowSceneSessionImpl::ProcessVirtualPixelLimits(WindowLimits& newLimits, WindowLimits& newLimitsVP,
                                                       const WindowLimits& customizedLimits,
                                                       const WindowLimits& systemLimitsVP, float virtualPixelRatio)
{
    const uint32_t limitMinWidth = systemLimitsVP.minWidth_;
    const uint32_t limitMinHeight = systemLimitsVP.minHeight_;

    // Update the maximum width and height limitations
    UpdateLimitIfInRange(newLimitsVP.maxWidth_, customizedLimits.maxWidth_, limitMinWidth, systemLimitsVP.maxWidth_);
    UpdateLimitIfInRange(newLimitsVP.maxHeight_, customizedLimits.maxHeight_, limitMinHeight,
        systemLimitsVP.maxHeight_);

    // Update the minimum width and height limitations
    UpdateLimitIfInRange(newLimitsVP.minWidth_, customizedLimits.minWidth_, limitMinWidth, newLimitsVP.maxWidth_);
    UpdateLimitIfInRange(newLimitsVP.minHeight_, customizedLimits.minHeight_, limitMinHeight, newLimitsVP.maxHeight_);

    // Convert virtual pixels to physical pixels
    RecalculatePxLimitsByVp(newLimitsVP, newLimits, virtualPixelRatio);
}

/** @note @window.layout */
void WindowSceneSessionImpl::ProcessPhysicalPixelLimits(WindowLimits& newLimits, WindowLimits& newLimitsVP,
                                                        const WindowLimits& customizedLimits,
                                                        const WindowLimits& systemLimits, float virtualPixelRatio)
{
    uint32_t limitMinWidth = systemLimits.minWidth_;
    uint32_t limitMinHeight = systemLimits.minHeight_;

    // Breaking through system limitations
    if (forceLimits_ && IsPcOrFreeMultiWindowCapabilityEnabled()) {
        const uint32_t forceLimitMinWidth =
            static_cast<uint32_t>(FORCE_LIMIT_MIN_FLOATING_WIDTH * virtualPixelRatio);
        const uint32_t forceLimitMinHeight =
            static_cast<uint32_t>(FORCE_LIMIT_MIN_FLOATING_HEIGHT * virtualPixelRatio);

        limitMinWidth = std::min(forceLimitMinWidth, limitMinWidth);
        limitMinHeight = std::min(forceLimitMinHeight, limitMinHeight);

        newLimits.minWidth_ = limitMinWidth;
        newLimits.minHeight_ = limitMinHeight;
    }

    // Update the maximum width and height limitations
    UpdateLimitIfInRange(newLimits.maxWidth_, customizedLimits.maxWidth_, limitMinWidth, systemLimits.maxWidth_);
    UpdateLimitIfInRange(newLimits.maxHeight_, customizedLimits.maxHeight_, limitMinHeight, systemLimits.maxHeight_);

    // Update the minimum width and height limitations
    UpdateLimitIfInRange(newLimits.minWidth_, customizedLimits.minWidth_, limitMinWidth, newLimits.maxWidth_);
    UpdateLimitIfInRange(newLimits.minHeight_, customizedLimits.minHeight_, limitMinHeight, newLimits.maxHeight_);

    // Convert physical pixel to virtual pixels
    RecalculateVpLimitsByPx(newLimits, newLimitsVP, virtualPixelRatio);
}

/** @note @window.layout */
void WindowSceneSessionImpl::CalculateNewLimitsByRatio(WindowLimits& newLimits,
                                                       WindowLimits& newLimitsVP,
                                                       const WindowLimits& customizedLimits)
{
    // Copy ratio constraints from customized limits
    newLimits.maxRatio_ = customizedLimits.maxRatio_;
    newLimits.minRatio_ = customizedLimits.minRatio_;
    newLimitsVP.maxRatio_ = customizedLimits.maxRatio_;
    newLimitsVP.minRatio_ = customizedLimits.minRatio_;

    // Calculate initial ratio bounds based on current size limits
    double maxRatio = FLT_MAX;
    double minRatio = 0.0;
    CalculateInitialRatioBounds(newLimits, maxRatio, minRatio);

    // Apply customized ratio constraints if they are within feasible range
    ApplyCustomRatioConstraints(customizedLimits, maxRatio, minRatio);

    // Recalculate size limits using the final ratio values
    RecalculateSizeLimitsWithRatios(newLimits, newLimitsVP, maxRatio, minRatio, customizedLimits.pixelUnit_);
}

/** @note @window.layout */
void WindowSceneSessionImpl::CalculateInitialRatioBounds(const WindowLimits& currentLimits,
                                                         double& maxRatio, double& minRatio)
{
    // Calculate maximum possible ratio (maxWidth / minHeight)
    if (currentLimits.minHeight_ != 0) {
        maxRatio = static_cast<double>(currentLimits.maxWidth_) /
                   static_cast<double>(currentLimits.minHeight_);
    }

    // Calculate minimum possible ratio (minWidth / maxHeight)
    if (currentLimits.maxHeight_ != 0) {
        minRatio = static_cast<double>(currentLimits.minWidth_) /
                   static_cast<double>(currentLimits.maxHeight_);
    }
}

/** @note @window.layout */
void WindowSceneSessionImpl::ApplyCustomRatioConstraints(const WindowLimits& customizedLimits,
                                                         double& maxRatio, double& minRatio)
{
    // Apply customized max ratio if it's within the feasible range
    if (IsValueInRange(customizedLimits.maxRatio_, minRatio, maxRatio)) {
        maxRatio = customizedLimits.maxRatio_;
    }

    // Apply customized min ratio if it's within the feasible range
    if (IsValueInRange(customizedLimits.minRatio_, minRatio, maxRatio)) {
        minRatio = customizedLimits.minRatio_;
    }
}

/** @note @window.layout */
void WindowSceneSessionImpl::RecalculateSizeLimitsWithRatios(WindowLimits& limits,
                                                             WindowLimits& limitsVP,
                                                             double maxRatio,
                                                             double minRatio,
                                                             PixelUnit pixelUnit)
{
    const float vpr = limits.vpRatio_; // limits.vpRatio_ == limitsVP.vpRatio_
    if (pixelUnit == PixelUnit::VP) {
        RecalculateLimits(maxRatio, minRatio, limitsVP);
        RecalculatePxLimitsByVp(limitsVP, limits, vpr);
    } else {
        RecalculateLimits(maxRatio, minRatio, limits);
        RecalculateVpLimitsByPx(limits, limitsVP, vpr);
    }
}

/** @note @window.layout */
void WindowSceneSessionImpl::UpdateWindowSizeLimits()
{
    WindowLimits customizedLimits;
    WindowLimits newLimits;
    WindowLimits newLimitsVP = WindowLimits::DEFAULT_VP_LIMITS();
    float virtualPixelRatio = 0.0f;

    CalculateNewLimitsByLimits(newLimits, newLimitsVP, customizedLimits, virtualPixelRatio);
    if (MathHelper::NearZero(virtualPixelRatio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, virtual pixel ratio is zero",
            GetWindowId());
        return;
    }
    newLimits.vpRatio_ = virtualPixelRatio;
    newLimitsVP.vpRatio_ = virtualPixelRatio;
    CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);

    // When the system window has not been set with 'setWindowLimits',
    // manually change its minimum width and height limit to 1 px.
    if (WindowHelper::IsSystemWindowButNotDialog(GetType()) && !userLimitsSet_) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, set min limits to 1 px, preMinW:%{public}u, preMinH:%{public}u",
            GetPersistentId(), newLimits.minWidth_, newLimits.minHeight_);
        newLimits.minWidth_ = 1;
        newLimits.minHeight_ = 1;
    }

    property_->SetWindowLimits(newLimits);
    property_->SetWindowLimitsVP(newLimitsVP);
    property_->SetLastLimitsVpr(virtualPixelRatio);
}

void WindowSceneSessionImpl::PreLayoutOnShow(WindowType type, const sptr<DisplayInfo>& info)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "uiContent is null");
        return;
    }
    if (type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT && info != nullptr) {
        uint64_t screenId = info->GetScreenId();
        KeyboardLayoutParams params;
        property_->GetKeyboardLayoutParamsByScreenId(screenId, params);
        if (params.isEmpty()) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Update prelayout failed, %{public}" PRIu64, screenId);
        } else {
            std::string dispName = info->GetName();
            Rect newRect = (info->GetWidth() > info->GetHeight()) ?
                            params.LandscapeKeyboardRect_ : params.PortraitKeyboardRect_;
            property_->SetRequestRect(newRect);
        }
    }
    const auto& requestRect = GetRequestRect();
    TLOGI(WmsLogTag::WMS_LIFE, "name: %{public}s, id: %{public}d, type: %{public}u, requestRect:%{public}s",
        property_->GetWindowName().c_str(), GetPersistentId(), type, requestRect.ToString().c_str());
    if (requestRect.width_ != 0 && requestRect.height_ != 0) {
        UpdateViewportConfig(requestRect, WindowSizeChangeReason::RESIZE, nullptr, info);
        if (auto hostSession = GetHostSession()) {
            WSRect wsRect = { requestRect.posX_, requestRect.posY_, requestRect.width_, requestRect.height_ };
            // Set a flag when the window rect and the request rect are the same for size change notify
            // SetNotifySizeChangeFlag must be set before SetWindowRect
            SetNotifySizeChangeFlag(true);
            property_->SetWindowRect(requestRect);
            hostSession->UpdateClientRect(wsRect);
        } else {
            TLOGE(WmsLogTag::WMS_LAYOUT, "hostSession is null");
        }
    }
    uiContent->PreLayout();
}

WMError WindowSceneSessionImpl::Show(uint32_t reason, bool withAnimation, bool withFocus)
{
    return Show(reason, withAnimation, withFocus, false);
}

WMError WindowSceneSessionImpl::Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach)
{
    if (reason == static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH)) {
        TLOGI(WmsLogTag::WMS_MULTI_USER, "Switch to current user, NotifyAfterForeground");
        NotifyAfterForeground(true, false);
        NotifyAfterDidForeground(reason);
        return WMError::WM_OK;
    }
    const auto type = GetType();
    if (IsWindowSessionInvalid()) {
        TLOGI(WmsLogTag::WMS_LIFE, "Window show failed, session is invalid, name: %{public}s, id: %{public}d",
            property_->GetWindowName().c_str(), GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);

    TLOGI(WmsLogTag::WMS_LIFE, "Window show [name: %{public}s, id: %{public}d, type: %{public}u], reason: %{public}u,"
        " state:%{public}u, requestState:%{public}u", property_->GetWindowName().c_str(),
        property_->GetPersistentId(), type, reason, state_, requestState_);
    auto isDecorEnable = IsDecorEnable();
    UpdateDecorEnableToAce(isDecorEnable);
    property_->SetDecorEnable(isDecorEnable);

    if (state_ == WindowState::STATE_SHOWN) {
        TLOGI(WmsLogTag::WMS_LIFE, "window is already shown [name:%{public}s, id:%{public}d, type: %{public}u]",
            property_->GetWindowName().c_str(), property_->GetPersistentId(), type);
        if (WindowHelper::IsMainWindow(type)) {
            hostSession->RaiseAppMainWindowToTop();
        }
        NotifyAfterForeground(true, false);
        NotifyAfterDidForeground(reason);
        RefreshNoInteractionTimeoutMonitor();
        return WMError::WM_OK;
    }
    auto displayInfo = GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Window show failed, displayInfo is null, name: %{public}s, id: %{public}d",
            property_->GetWindowName().c_str(), GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    float density = GetVirtualPixelRatio(displayInfo);
    if (!MathHelper::NearZero(virtualPixelRatio_ - density) ||
        !MathHelper::NearZero(property_->GetLastLimitsVpr() - density)) {
        UpdateDensityInner(displayInfo);
    }

    WMError ret = UpdateAnimationFlagProperty(withAnimation);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "Window show failed, UpdateProperty failed, ret: %{public}d, name: %{public}s"
            ", id: %{public}d", static_cast<int32_t>(ret), property_->GetWindowName().c_str(), GetPersistentId());
        return ret;
    }
    UpdateTitleButtonVisibility();
    property_->SetFocusableOnShow(withFocus);
    if (WindowHelper::IsMainWindow(type)) {
        ret = static_cast<WMError>(hostSession->Foreground(property_, true, identityToken_));
    } else if (WindowHelper::IsSystemOrSubWindow(type)) {
        if (waitAttach && !lifecycleCallback_ && !WindowHelper::IsKeyboardWindow(type) &&
            SysCapUtil::GetBundleName() != AppExecFwk::Constants::SCENE_BOARD_BUNDLE_NAME) {
            lifecycleCallback_ = sptr<LifecycleFutureCallback>::MakeSptr();
            TLOGI(WmsLogTag::WMS_LIFE, "init lifecycleCallback, id:%{public}d", GetPersistentId());
        }
        if (waitAttach && lifecycleCallback_) {
            lifecycleCallback_->ResetAttachLock();
        }
        PreLayoutOnShow(type, displayInfo);
        // Add maintenance logs before the IPC process.
        TLOGD(WmsLogTag::WMS_LIFE, "Show session [name: %{public}s, id: %{public}d]",
            property_->GetWindowName().c_str(), GetPersistentId());
        ret = static_cast<WMError>(hostSession->Show(property_));
    } else {
        ret = WMError::WM_ERROR_INVALID_WINDOW;
    }
    RecordLifeCycleExceptionEvent(LifeCycleEvent::SHOW_EVENT, ret);
    if (ret == WMError::WM_OK) {
        // update sub window state
        UpdateSubWindowStateAndNotify(GetPersistentId(), WindowState::STATE_SHOWN);
        state_ = WindowState::STATE_SHOWN;
        requestState_ = WindowState::STATE_SHOWN;
        NotifyAfterForeground(true, true, waitAttach);
        NotifyAfterDidForeground(reason);
        NotifyFreeMultiWindowModeResume();
        RefreshNoInteractionTimeoutMonitor();
        TLOGI(WmsLogTag::WMS_LIFE, "Window show success [name:%{public}s, id:%{public}d, type:%{public}u]",
            property_->GetWindowName().c_str(), GetPersistentId(), type);
    } else {
        NotifyForegroundFailed(ret);
        TLOGI(WmsLogTag::WMS_LIFE, "Window show failed with errcode: %{public}d, name:%{public}s, id:%{public}d",
            static_cast<int32_t>(ret), property_->GetWindowName().c_str(), GetPersistentId());
    }
    NotifyWindowStatusChange(GetWindowMode());
    NotifyWindowStatusDidChangeAfterShowWindow();
    NotifyDisplayInfoChange(displayInfo);
    return ret;
}

void WindowSceneSessionImpl::NotifyWindowStatusDidChangeAfterShowWindow()
{
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->NotifyWindowStatusDidChangeAfterShowWindow();
}

sptr<DisplayInfo> WindowSceneSessionImpl::GetDisplayInfo() const
{
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "not find, displayId: %{public}" PRIu64 ", persistentId: %{public}d",
            property_->GetDisplayId(), GetPersistentId());
        display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplay();
        if (display != nullptr) {
            property_->SetDisplayId(display->GetId());
            TLOGI(WmsLogTag::WMS_LIFE, "use default display id: %{public}" PRIu64, display->GetId());
        } else {
            TLOGE(WmsLogTag::WMS_LIFE, "failed, display is null, name: %{public}s, id: %{public}d",
                property_->GetWindowName().c_str(), GetPersistentId());
            return nullptr;
        }
    }
    return display->GetDisplayInfo();
}

WMError WindowSceneSessionImpl::ShowKeyboard(
    uint32_t callingWindowId, uint64_t tgtDisplayId, KeyboardEffectOption effectOption)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "CallingWindowId: %{public}d, effect option: %{public}s, %{public}" PRIu64,
        callingWindowId, effectOption.ToString().c_str(), tgtDisplayId);
    if (effectOption.viewMode_ >= KeyboardViewMode::VIEW_MODE_END) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Invalid view mode: %{public}u. Use default mode",
            static_cast<uint32_t>(effectOption.viewMode_));
        effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    }
    if (effectOption.flowLightMode_ >= KeyboardFlowLightMode::END) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Invalid flow light mode: %{public}u. Use default mode",
            static_cast<uint32_t>(effectOption.flowLightMode_));
        effectOption.flowLightMode_ = KeyboardFlowLightMode::NONE;
    }
    if (effectOption.gradientMode_ >= KeyboardGradientMode::END) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Invalid gradient mode: %{public}u. Use default mode",
            static_cast<uint32_t>(effectOption.gradientMode_));
        effectOption.gradientMode_ = KeyboardGradientMode::NONE;
    }
    property_->SetKeyboardEffectOption(effectOption);
    property_->SetCallingSessionId(callingWindowId);
    property_->SetDisplayId(tgtDisplayId);
    return Show();
}

void WindowSceneSessionImpl::NotifyFreeMultiWindowModeResume()
{
    TLOGI(WmsLogTag::WMS_MAIN, "IsPcMode %{public}d, isColdStart %{public}d", IsPcOrFreeMultiWindowCapabilityEnabled(),
        isColdStart_);
    if (IsPcOrFreeMultiWindowCapabilityEnabled() && !isColdStart_) {
        isDidForeground_ = true;
        NotifyAfterLifecycleResumed();
    }
}

void WindowSceneSessionImpl::Resume()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in, isColdStart: %{public}d, isDidForeground: %{public}d",
        isColdStart_, isDidForeground_);
    isDidForeground_ = true;
    isColdStart_ = false;
    NotifyAfterLifecycleResumed();
}

void WindowSceneSessionImpl::Pause()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in, isColdStart: %{public}d", isColdStart_);
    isColdStart_ = false;
    NotifyAfterLifecyclePaused();
}

WMError WindowSceneSessionImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    return Hide(reason, withAnimation, isFromInnerkits, false);
}

WMError WindowSceneSessionImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach)
{
    if (reason == static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH)) {
        TLOGI(WmsLogTag::WMS_MULTI_USER, "Switch to another user, NotifyAfterBackground");
        NotifyAfterBackground(true, false);
        NotifyAfterDidBackground(reason);
        return WMError::WM_OK;
    }

    const auto type = GetType();
    TLOGI(WmsLogTag::WMS_LIFE, "Window hide [id:%{public}d, type: %{public}d, reason:%{public}u, state:%{public}u, "
        "requestState:%{public}u", GetPersistentId(), type, reason, state_, requestState_);
    if (IsWindowSessionInvalid()) {
        TLOGI(WmsLogTag::WMS_LIFE, "session is invalid, id:%{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);

    WindowState validState = state_;
    if (WindowHelper::IsSubWindow(type) || WindowHelper::IsDialogWindow(type)) {
        validState = requestState_;
    }
    if (validState == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        TLOGD(WmsLogTag::WMS_LIFE, "window is alreay hidden, id:%{public}d", property_->GetPersistentId());
        NotifyBackgroundFailed(WMError::WM_DO_NOTHING);
        return WMError::WM_OK;
    }

    WMError res = UpdateAnimationFlagProperty(withAnimation);
    if (res != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "UpdateProperty failed with errCode:%{public}d", static_cast<int32_t>(res));
        return res;
    }

    /*
     * main window no need to notify host, since host knows hide first
     * main window notify host temporarily, since host background may failed
     * need to SetActive(false) for host session before background
     */

    if (WindowHelper::IsMainWindow(type)) {
        res = static_cast<WMError>(SetActive(false));
        if (res != WMError::WM_OK) {
            return res;
        }
        res = static_cast<WMError>(hostSession->Background(true, identityToken_));
    } else if (WindowHelper::IsSystemOrSubWindow(type)) {
        if (waitDetach && !lifecycleCallback_ && !WindowHelper::IsKeyboardWindow(type) &&
            SysCapUtil::GetBundleName() != AppExecFwk::Constants::SCENE_BOARD_BUNDLE_NAME) {
            lifecycleCallback_ = sptr<LifecycleFutureCallback>::MakeSptr();
            TLOGI(WmsLogTag::WMS_LIFE, "init lifecycleCallback, id:%{public}d", GetPersistentId());
        }
        if (waitDetach && lifecycleCallback_) {
            lifecycleCallback_->ResetDetachLock();
        }
        res = static_cast<WMError>(hostSession->Hide());
    } else {
        res = WMError::WM_ERROR_INVALID_WINDOW;
    }

    RecordLifeCycleExceptionEvent(LifeCycleEvent::HIDE_EVENT, res);
    if (res == WMError::WM_OK) {
        // update sub window state if this is main window
        GetAttachStateSyncResult(waitDetach, false);
        UpdateSubWindowState(type);
        NotifyAfterDidBackground(reason);
        state_ = WindowState::STATE_HIDDEN;
        requestState_ = WindowState::STATE_HIDDEN;
        isDidForeground_ = false;
        if (!interactive_) {
            hasFirstNotifyInteractive_ = false;
        }
    }
    CustomHideAnimation();
    
    NotifyWindowStatusChange(GetWindowMode());
    NotifyWindowStatusDidChange(GetWindowMode());
    escKeyEventTriggered_ = false;
    TLOGI(WmsLogTag::WMS_LIFE, "Window hide success [id:%{public}d, type: %{public}d",
        property_->GetPersistentId(), type);
    return res;
}

void WindowSceneSessionImpl::CustomHideAnimation()
{
    uint32_t animationFlag = property_->GetAnimationFlag();
    if (animationFlag == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
        std::vector<sptr<IAnimationTransitionController>> animationTransitionControllers;
        {
            std::lock_guard<std::mutex> lockListener(transitionControllerMutex_);
            animationTransitionControllers = animationTransitionControllers_;
        }
        for (auto animationTransitionController : animationTransitionControllers) {
            if (animationTransitionController != nullptr) {
                animationTransitionController->AnimationForHidden();
            }
        }
        RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    }
}

WMError WindowSceneSessionImpl::NotifyDrawingCompleted()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is invalid, id:%{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    WMError res = WindowHelper::IsMainWindow(GetType()) ?
                  static_cast<WMError>(hostSession->DrawingCompleted()) :
                  WMError::WM_ERROR_INVALID_WINDOW;
    if (res == WMError::WM_OK) {
        TLOGI(WmsLogTag::WMS_LIFE, "success id:%{public}d", GetPersistentId());
    }
    return res;
}

WMError WindowSceneSessionImpl::NotifyRemoveStartingWindow()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "session is invalid, id:%{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    WMError res = WindowHelper::IsMainWindow(GetType()) ?
                  static_cast<WMError>(hostSession->RemoveStartingWindow()) :
                  WMError::WM_ERROR_INVALID_WINDOW;
    if (res == WMError::WM_OK) {
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "success id:%{public}d", GetPersistentId());
    }
    return res;
}

void WindowSceneSessionImpl::UpdateSubWindowState(const WindowType& type)
{
    UpdateSubWindowStateAndNotify(GetPersistentId(), WindowState::STATE_HIDDEN);
    if (WindowHelper::IsSubWindow(type)) {
        if (state_ == WindowState::STATE_SHOWN) {
            NotifyAfterBackground(true, true);
        }
    } else {
        NotifyAfterBackground(true, true);
    }
}

void WindowSceneSessionImpl::PreProcessCreate()
{
    SetDefaultProperty();
}

void WindowSceneSessionImpl::SetDefaultProperty()
{
    switch (property_->GetWindowType()) {
        case WindowType::WINDOW_TYPE_TOAST:
        case WindowType::WINDOW_TYPE_FLOAT:
        case WindowType::WINDOW_TYPE_SYSTEM_FLOAT:
        case WindowType::WINDOW_TYPE_FLOAT_CAMERA:
        case WindowType::WINDOW_TYPE_VOICE_INTERACTION:
        case WindowType::WINDOW_TYPE_SEARCHING_BAR:
        case WindowType::WINDOW_TYPE_SCREENSHOT:
        case WindowType::WINDOW_TYPE_GLOBAL_SEARCH:
        case WindowType::WINDOW_TYPE_DIALOG:
        case WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW:
        case WindowType::WINDOW_TYPE_PANEL:
        case WindowType::WINDOW_TYPE_LAUNCHER_DOCK:
        case WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD:
        case WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION:
        case WindowType::WINDOW_TYPE_MAGNIFICATION:
        case WindowType::WINDOW_TYPE_MAGNIFICATION_MENU:
        case WindowType::WINDOW_TYPE_SELECTION:
        case WindowType::WINDOW_TYPE_DYNAMIC: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            break;
        }
        case WindowType::WINDOW_TYPE_VOLUME_OVERLAY:
        case WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT:
        case WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR:
        case WindowType::WINDOW_TYPE_DOCK_SLICE:
        case WindowType::WINDOW_TYPE_STATUS_BAR:
        case WindowType::WINDOW_TYPE_NAVIGATION_BAR:
        case WindowType::WINDOW_TYPE_FLOAT_NAVIGATION: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_SYSTEM_TOAST: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetTouchable(false);
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_POINTER: {
            property_->SetFocusable(false);
            break;
        }
        case WindowType::WINDOW_TYPE_SCREEN_CONTROL: {
            property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
            property_->SetTouchable(false);
            property_->SetFocusable(false);
            SetAlpha(0);
            break;
        }
        default:
            break;
    }
}

WMError WindowSceneSessionImpl::DestroyInner(bool needNotifyServer)
{
    WMError ret = WMError::WM_OK;
    if (!WindowHelper::IsMainWindow(GetType()) && needNotifyServer) {
        if (WindowHelper::IsSystemWindow(GetType())) {
            // main window no need to notify host, since host knows hide first
            ret = SyncDestroyAndDisconnectSpecificSession(property_->GetPersistentId());
        } else if (WindowHelper::IsSubWindow(GetType()) && !property_->GetIsUIExtFirstSubWindow()) {
            auto parentSession = FindParentSessionByParentId(GetParentId());
            if (parentSession == nullptr || parentSession->GetHostSession() == nullptr) {
                return WMError::WM_ERROR_NULLPTR;
            }
            ret = SyncDestroyAndDisconnectSpecificSession(property_->GetPersistentId());
        } else if (property_->GetIsUIExtFirstSubWindow()) {
            ret = SyncDestroyAndDisconnectSpecificSession(property_->GetPersistentId());
        }
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "DestroyInner fail, ret:%{public}d", ret);
        return ret;
    }

    if (WindowHelper::IsMainWindow(GetType())) {
        if (auto hostSession = GetHostSession()) {
            ret = static_cast<WMError>(hostSession->Disconnect(true, identityToken_));
        }
    }
    return ret;
}

WMError WindowSceneSessionImpl::SyncDestroyAndDisconnectSpecificSession(int32_t persistentId)
{
    WMError ret = WMError::WM_OK;
    if (SysCapUtil::GetBundleName() == AppExecFwk::Constants::SCENE_BOARD_BUNDLE_NAME) {
        TLOGI(WmsLogTag::WMS_LIFE, "Destroy window is scb window");
        ret = SingletonContainer::Get<WindowAdapter>().DestroyAndDisconnectSpecificSession(persistentId);
        return ret;
    }
    sptr<PatternDetachCallback> callback = sptr<PatternDetachCallback>::MakeSptr();
    ret = SingletonContainer::Get<WindowAdapter>().DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId,
        callback->AsObject());
    if (ret != WMError::WM_OK) {
        return ret;
    }
    auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    callback->GetResult(WINDOW_DETACH_TIMEOUT);
    auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto waitTime = endTime - startTime;
    if (waitTime >= WINDOW_DETACH_TIMEOUT) {
        TLOGW(WmsLogTag::WMS_LIFE, "Destroy window timeout, persistentId:%{public}d", persistentId);
        callback->GetResult(std::numeric_limits<int>::max());
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Destroy window persistentId:%{public}d waitTime:%{public}lld", persistentId, waitTime);
    return ret;
}

WMError WindowSceneSessionImpl::DestroyHookWindow()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    InputTransferStation::GetInstance().RemoveInputWindow(GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "session invalid, id: %{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        state_ = WindowState::STATE_DESTROYED;
        requestState_ = WindowState::STATE_DESTROYED;
    }

    DestroySubWindow();
    {
        std::unique_lock<std::shared_mutex> lock(windowSessionMutex_);
        windowSessionMap_.erase(property_->GetWindowName());
    }
    {
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = nullptr;
    }
    NotifyAfterDestroy();
    ClearListenersById(GetPersistentId());
    ClearVsyncStation();
    SetUIContentComplete();
    TLOGI(WmsLogTag::WMS_LIFE, "Destroy hook window success, id: %{public}d", property_->GetPersistentId());
    return WMError::WM_OK;
}

WindowLifeCycleInfo WindowSceneSessionImpl::GetWindowLifecycleInfo() const
{
    WindowLifeCycleInfo lifeCycleInfo;
    lifeCycleInfo.windowId = GetPersistentId();
    lifeCycleInfo.windowType = GetType();
    lifeCycleInfo.windowName = GetWindowName();
    return lifeCycleInfo;
}

WMError WindowSceneSessionImpl::Destroy(bool needNotifyServer, bool needClearListener, uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "Destroy start, id:%{public}d, state:%{public}u, needNotifyServer:%{public}d, "
        "needClearListener:%{public}d, reason:%{public}u", GetPersistentId(), state_, needNotifyServer,
        needClearListener, reason);
    if (reason == static_cast<uint32_t>(WindowStateChangeReason::ABILITY_HOOK)) {
        return DestroyHookWindow();
    }
    InputTransferStation::GetInstance().RemoveInputWindow(GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "session invalid, id: %{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WindowInspector::GetInstance().UnregisterGetWMSWindowListCallback(GetWindowId());
    SingletonContainer::Get<WindowAdapter>().UnregisterSessionRecoverCallbackFunc(property_->GetPersistentId());

    auto ret = DestroyInner(needNotifyServer);
    RecordLifeCycleExceptionEvent(LifeCycleEvent::DESTROY_EVENT, ret);
    if (ret != WMError::WM_OK && ret != WMError::WM_ERROR_NULLPTR) { // nullptr means no session in server
        WLOGFW("Destroy window failed, id: %{public}d", GetPersistentId());
        return ret;
    }
    WindowLifeCycleInfo windowLifeCycleInfo = GetWindowLifecycleInfo();
    SingletonContainer::Get<WindowManager>().NotifyWMSWindowDestroyed(windowLifeCycleInfo);

    // delete after replace WSError with WMError
    NotifyBeforeDestroy(GetWindowName());
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        state_ = WindowState::STATE_DESTROYED;
        requestState_ = WindowState::STATE_DESTROYED;
    }

    DestroySubWindow();
    {
        std::unique_lock<std::shared_mutex> lock(windowSessionMutex_);
        windowSessionMap_.erase(property_->GetWindowName());
    }
    {
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = nullptr;
    }
    NotifyAfterDestroy();
    if (needClearListener) {
        ClearListenersById(GetPersistentId());
    }
    auto context = GetContext();
    if (context) {
        context.reset();
    }
    ClearVsyncStation();
    SetUIContentComplete();
    surfaceNode_ = nullptr;
    TLOGI(WmsLogTag::WMS_LIFE, "Destroy success, id: %{public}d", property_->GetPersistentId());
    return WMError::WM_OK;
}

/** @note @window.layout */
void WindowSceneSessionImpl::CheckMoveConfiguration(MoveConfiguration& moveConfiguration)
{
    std::vector<DisplayId> displayIds = SingletonContainer::Get<DisplayManagerAdapter>().GetAllDisplayIds();
    if (std::find(displayIds.begin(), displayIds.end(), moveConfiguration.displayId) ==
        displayIds.end()) { // need to be found in displayIds, otherwise the value is DISPLAY_ID_INVALID
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}d not find displayId moveConfiguration %{public}s",
            property_->GetPersistentId(), moveConfiguration.ToString().c_str());
        moveConfiguration.displayId = DISPLAY_ID_INVALID;
    }
}

/** @note @window.layout */
WMError WindowSceneSessionImpl::MoveTo(int32_t x, int32_t y, bool isMoveToGlobal, MoveConfiguration moveConfiguration)
{
    TLOGI_LMT(TEN_SECONDS, RECORD_200_TIMES, WmsLogTag::WMS_LAYOUT,
        "Id:%{public}d MoveTo:(%{public}d %{public}d) global:%{public}d cfg:%{public}s",
        property_->GetPersistentId(), x, y, isMoveToGlobal, moveConfiguration.ToString().c_str());
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Unsupported operation for pip window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    const auto& windowRect = GetRect();
    const auto& requestRect = GetRequestRect();
    if (WindowHelper::IsSubWindow(GetType())) {
        auto mainWindow = FindMainWindowWithContext();
        if (mainWindow != nullptr && (mainWindow->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
                                      mainWindow->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY)) {
            if (requestRect.posX_ == x && requestRect.posY_ == y) {
                TLOGW(WmsLogTag::WMS_LAYOUT, "Request same position in multiWindow will not update");
                return WMError::WM_OK;
            }
        }
    }
    Rect newRect = { x, y, requestRect.width_, requestRect.height_ }; // must keep x/y
    TLOGI_LMT(TEN_SECONDS, RECORD_200_TIMES, WmsLogTag::WMS_LAYOUT,
        "Id:%{public}d state:%{public}d type:%{public}d mode:%{public}d rect:"
        "%{public}s->%{public}s req=%{public}s", property_->GetPersistentId(), state_, GetType(), GetWindowMode(),
        windowRect.ToString().c_str(), newRect.ToString().c_str(), requestRect.ToString().c_str());
    property_->SetRequestRect(newRect);

    CheckMoveConfiguration(moveConfiguration);
    WSRect wsRect = { newRect.posX_, newRect.posY_, newRect.width_, newRect.height_ };
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto ret = hostSession->UpdateSessionRect(wsRect, SizeChangeReason::MOVE, isMoveToGlobal, false, moveConfiguration);
    return static_cast<WMError>(ret);
}

WMError WindowSceneSessionImpl::MoveToAsync(int32_t x, int32_t y, MoveConfiguration moveConfiguration)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "window should not move, winId:%{public}u, mode:%{public}u",
            GetWindowId(), GetWindowMode());
        return WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS;
    }
    auto ret = MoveTo(x, y, false, moveConfiguration);
    if (state_ == WindowState::STATE_SHOWN) {
        layoutCallback_->ResetMoveToLock();
        auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        layoutCallback_->GetMoveToAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto waitTime = endTime - startTime;
        if (waitTime >= WINDOW_LAYOUT_TIMEOUT) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "Layout timeout, Id:%{public}d", property_->GetPersistentId());
            layoutCallback_->GetMoveToAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        }
    }
    return static_cast<WMError>(ret);
}

WMError WindowSceneSessionImpl::MoveWindowToGlobal(int32_t x, int32_t y, MoveConfiguration moveConfiguration)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d MoveTo %{public}d %{public}d", property_->GetPersistentId(), x, y);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "window should not move, winId:%{public}u, mode:%{public}u",
            GetWindowId(), GetWindowMode());
        return WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS;
    }

    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Unsupported operation for pip window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    const auto& windowRect = GetRect();
    const auto& requestRect = GetRequestRect();
    Rect newRect = { x, y, requestRect.width_, requestRect.height_ }; // must keep x/y
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, state: %{public}d, type: %{public}d, mode: %{public}d, requestRect: "
        "%{public}s, windowRect: %{public}s, newRect: %{public}s",
        property_->GetPersistentId(), state_, GetType(), GetWindowMode(), requestRect.ToString().c_str(),
        windowRect.ToString().c_str(), newRect.ToString().c_str());

    property_->SetRequestRect(newRect);

    CheckMoveConfiguration(moveConfiguration);
    WSRect wsRect = { newRect.posX_, newRect.posY_, newRect.width_, newRect.height_ };
    auto hostSession = GetHostSession();
    RectAnimationConfig rectAnimationConfig = moveConfiguration.rectAnimationConfig;
    SizeChangeReason reason = rectAnimationConfig.duration > 0 ? SizeChangeReason::MOVE_WITH_ANIMATION :
        SizeChangeReason::MOVE;
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto ret = hostSession->UpdateSessionRect(wsRect, reason, false, true, moveConfiguration, rectAnimationConfig);
    if (state_ == WindowState::STATE_SHOWN) {
        layoutCallback_->ResetMoveToLock();
        auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        layoutCallback_->GetMoveToAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto waitTime = endTime - startTime;
        if (waitTime >= WINDOW_LAYOUT_TIMEOUT) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "Layout timeout, Id:%{public}d", property_->GetPersistentId());
            layoutCallback_->GetMoveToAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        }
    }
    return static_cast<WMError>(ret);
}

/** @note @window.layout */
WMError WindowSceneSessionImpl::MoveWindowToGlobalDisplay(
    int32_t x, int32_t y, MoveConfiguration /*moveConfiguration*/)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid session");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto mode = GetWindowMode();
    if (mode != WindowMode::WINDOW_MODE_FLOATING) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "window should not move, windowId: %{public}u, mode: %{public}u",
            GetWindowId(), mode);
        return WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS;
    }
    auto winId = GetPersistentId();
    const auto curGlobalDisplayRect = GetGlobalDisplayRect();
    if (curGlobalDisplayRect.IsSamePosition(x, y)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, request same position: [%{public}d, %{public}d]",
            winId, x, y);
        return WMError::WM_OK;
    }
    // Use RequestRect to quickly get width and height from Resize method.
    const auto requestRect = GetRequestRect();
    if (!Rect::IsRightBottomValid(x, y, requestRect.width_, requestRect.height_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, illegal position: [%{public}d, %{public}d]", winId, x, y);
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    WSRect newGlobalDisplayRect = { x, y, requestRect.width_, requestRect.height_ };
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, newGlobalDisplayRect: %{public}s",
        winId, newGlobalDisplayRect.ToString().c_str());
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto ret = hostSession->UpdateGlobalDisplayRectFromClient(newGlobalDisplayRect, SizeChangeReason::MOVE);
    // If the window is shown, wait for the layout result from the server, so that the
    // caller can directly obtain the updated globalDisplayRect from the window properties.
    if (state_ == WindowState::STATE_SHOWN) {
        layoutCallback_->ResetMoveWindowToGlobalDisplayLock();
        const auto now = [] {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        };
        const auto startTime = now();
        layoutCallback_->GetMoveWindowToGlobalDisplayAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        const auto waitDuration = now() - startTime;
        if (waitDuration >= WINDOW_LAYOUT_TIMEOUT) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "Window layout timeout, windowId: %{public}d", winId);
            layoutCallback_->GetMoveWindowToGlobalDisplayAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        }
    }
    return static_cast<WMError>(ret);
}

WMError WindowSceneSessionImpl::GetGlobalScaledRect(Rect& globalScaledRect)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    auto ret = hostSession->GetGlobalScaledRect(globalScaledRect);
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, globalScaledRect:%{public}s, ret:%{public}d",
        GetPersistentId(), globalScaledRect.ToString().c_str(), ret);
    if (WMError::WM_OK == static_cast<WMError>(ret)) {
        HookWindowSizeByHookWindowInfo(globalScaledRect);
    }
    return static_cast<WMError>(ret);
}

void WindowSceneSessionImpl::LimitCameraFloatWindowMininumSize(uint32_t& width, uint32_t& height, float& vpr)
{
    // Float camera window has a special limit:
    // if display sw <= 600dp, portrait: min width = display sw * 30%, landscape: min width = sw * 50%
    // if display sw > 600dp, portrait: min width = display sw * 12%, landscape: min width = sw * 30%
    if (GetType() != WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        return;
    }

    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        WLOGFE("get display failed displayId:%{public}" PRIu64, property_->GetDisplayId());
        return;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        WLOGFE("get displayInfo failed displayId:%{public}" PRIu64, property_->GetDisplayId());
        return;
    }
    uint32_t displayWidth = static_cast<uint32_t>(display->GetWidth());
    uint32_t displayHeight = static_cast<uint32_t>(display->GetHeight());
    if (displayWidth == 0 || displayHeight == 0) {
        return;
    }
    vpr = GetVirtualPixelRatio(displayInfo);
    uint32_t smallWidth = displayHeight <= displayWidth ? displayHeight : displayWidth;
    float hwRatio = static_cast<float>(displayHeight) / static_cast<float>(displayWidth);
    uint32_t minWidth;
    if (smallWidth <= static_cast<uint32_t>(600 * vpr)) { // sw <= 600dp
        if (displayWidth <= displayHeight) {
            minWidth = static_cast<uint32_t>(smallWidth * 0.3); // ratio : 0.3
        } else {
            minWidth = static_cast<uint32_t>(smallWidth * 0.5); // ratio : 0.5
        }
    } else {
        if (displayWidth <= displayHeight) {
            minWidth = static_cast<uint32_t>(smallWidth * 0.12); // ratio : 0.12
        } else {
            minWidth = static_cast<uint32_t>(smallWidth * 0.3); // ratio : 0.3
        }
    }
    width = (width < minWidth) ? minWidth : width;
    height = static_cast<uint32_t>(width * hwRatio);
}

/** @note @window.layout */
void WindowSceneSessionImpl::UpdateFloatingWindowSizeBySizeLimits(uint32_t& width, uint32_t& height) const
{
    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "float camera type window");
        return;
    }
    // get new limit config with the settings of system and app
    const auto& sizeLimits = property_->GetWindowLimits();
    // limit minimum size of floating (not system type) window
    if (!WindowHelper::IsSystemWindow(property_->GetWindowType()) ||
        property_->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
        width = std::max(sizeLimits.minWidth_, width);
        height = std::max(sizeLimits.minHeight_, height);
    }
    width = std::min(sizeLimits.maxWidth_, width);
    height = std::min(sizeLimits.maxHeight_, height);
    if (height == 0) {
        return;
    }
    float curRatio = static_cast<float>(width) / static_cast<float>(height);
    // there is no need to fix size by ratio if this is not main floating window
    if (!WindowHelper::IsMainFloatingWindow(property_->GetWindowType(), GetWindowMode()) ||
        (!MathHelper::GreatNotEqual(sizeLimits.minRatio_, curRatio) &&
         !MathHelper::GreatNotEqual(curRatio, sizeLimits.maxRatio_))) {
        return;
    }

    float newRatio = curRatio < sizeLimits.minRatio_ ? sizeLimits.minRatio_ : sizeLimits.maxRatio_;
    if (MathHelper::NearZero(newRatio)) {
        return;
    }
    if (sizeLimits.maxWidth_ == sizeLimits.minWidth_) {
        height = static_cast<uint32_t>(static_cast<float>(width) / newRatio);
        return;
    }
    if (sizeLimits.maxHeight_ == sizeLimits.minHeight_) {
        width = static_cast<uint32_t>(static_cast<float>(height) * newRatio);
        return;
    }
    WLOGFD("After limit by customize config: %{public}u %{public}u", width, height);
}

/** @note @window.layout */
void WindowSceneSessionImpl::LimitWindowSize(uint32_t& width, uint32_t& height)
{
    float vpr = 0.0f;

    // Float camera window has special limits
    LimitCameraFloatWindowMininumSize(width, height, vpr);

    if (!MathHelper::NearZero(vpr) || !MathHelper::NearZero(property_->GetLastLimitsVpr() - vpr)) {
        UpdateWindowSizeLimits();
    }
    UpdateFloatingWindowSizeBySizeLimits(width, height);
}

WMError WindowSceneSessionImpl::CheckAndModifyWindowRect(uint32_t& width, uint32_t& height)
{
    if (width == 0 || height == 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Unsupported operation for pip window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING && !IsFullScreenPcAppInPadMode()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Fullscreen window could not resize, winId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    if (IsFullScreenPcAppInPadMode() && IsFullScreenEnable()) {
        NotifyClientWindowSize();
        return WMError::WM_ERROR_INVALID_OPERATION;
    }

    LimitWindowSize(width, height);
    return WMError::WM_OK;
}

/** @note @window.layout */
WMError WindowSceneSessionImpl::Resize(uint32_t width, uint32_t height, const RectAnimationConfig& rectAnimationConfig)
{
    auto reason = SizeChangeReason::RESIZE;
    if (isResizedByLimit_) {
        reason = SizeChangeReason::RESIZE_BY_LIMIT;
        isResizedByLimit_ = false;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d resize %{public}u %{public}u",
        property_->GetPersistentId(), width, height);

    if (CheckAndModifyWindowRect(width, height) != WMError::WM_OK) {
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    const auto& windowRect = GetRect();
    const auto& requestRect = GetRequestRect();

    if (WindowHelper::IsSubWindow(GetType())) {
        auto mainWindow = FindMainWindowWithContext();
        if (mainWindow != nullptr && (mainWindow->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
                                      mainWindow->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY)) {
            if (width == requestRect.width_ && height == requestRect.height_) {
                TLOGW(WmsLogTag::WMS_LAYOUT, "Request same size in multiWindow will not update, return");
                return WMError::WM_OK;
            }
        }
    }

    Rect newRect = { requestRect.posX_, requestRect.posY_, width, height }; // must keep w/h
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, state: %{public}d, type: %{public}d, mode: %{public}d, requestRect: "
        "%{public}s, windowRect: %{public}s, newRect: %{public}s",
        property_->GetPersistentId(), state_, GetType(), GetWindowMode(), requestRect.ToString().c_str(),
        windowRect.ToString().c_str(), newRect.ToString().c_str());

    property_->SetRequestRect(newRect);
    property_->SetRectAnimationConfig(rectAnimationConfig);

    WSRect wsRect = { newRect.posX_, newRect.posY_, newRect.width_, newRect.height_ };
    auto hostSession = GetHostSession();
    if (rectAnimationConfig.duration > 0 && reason == SizeChangeReason::RESIZE) {
        reason = SizeChangeReason::RESIZE_WITH_ANIMATION;
    }

    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto ret = hostSession->UpdateSessionRect(wsRect, reason, false, false, {}, rectAnimationConfig);
    return static_cast<WMError>(ret);
}

WMError WindowSceneSessionImpl::ResizeAsync(uint32_t width, uint32_t height)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING && !property_->GetIsPcAppInPad()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "window should not resize, winId:%{public}u, mode:%{public}u",
            GetWindowId(), GetWindowMode());
        return WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS;
    }
    auto ret = Resize(width, height);
    if (state_ == WindowState::STATE_SHOWN) {
        layoutCallback_->ResetResizeLock();
        auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        layoutCallback_->GetResizeAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto waitTime = endTime - startTime;
        if (waitTime >= WINDOW_LAYOUT_TIMEOUT) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "Layout timeout, Id:%{public}d", property_->GetPersistentId());
            layoutCallback_->GetResizeAsyncResult(WINDOW_LAYOUT_TIMEOUT);
        }
    }
    return static_cast<WMError>(ret);
}

WMError WindowSceneSessionImpl::SetFrameRectForPartialZoomIn(const Rect& frameRect)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "set frame rect start, rect: %{public}s", frameRect.ToString().c_str());
    if (GetType() != WindowType::WINDOW_TYPE_MAGNIFICATION) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "window type is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return static_cast<WMError>(hostSession->SetFrameRectForPartialZoomIn(frameRect));
}

WMError WindowSceneSessionImpl::UpdateWindowModeForUITest(int32_t updateMode)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, updateMode: %{public}d", GetPersistentId(), updateMode);
    switch (updateMode) {
        case static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN):
            return Maximize();
        case static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING):
            return Recover();
        case static_cast<int32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY):
            return SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
        case static_cast<int32_t>(WindowMode::WINDOW_MODE_SPLIT_SECONDARY):
            return SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
        default:
            break;
    }
    return WMError::WM_DO_NOTHING;
}

WMError WindowSceneSessionImpl::GetTargetOrientationConfigInfo(Orientation targetOrientation,
    const std::map<WindowType, SystemBarProperty>& targetProperties,
    const std::map<WindowType, SystemBarProperty>& currentProperties,
    ViewportConfigAndAvoidArea& targetViewportConfigAndAvoidArea,
    ViewportConfigAndAvoidArea& currentViewportConfigAndAvoidArea)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::map<WindowType, SystemBarProperty> targetPageProperties;
    std::map<WindowType, SystemBarProperty> currentPageProperties;
    GetSystemBarPropertyForPage(targetProperties, targetPageProperties);
    GetSystemBarPropertyForPage(currentProperties, currentPageProperties);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);

    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "display is null, winId=%{public}u", GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<DisplayInfo> displayInfo = display ? display->GetDisplayInfo() : nullptr;
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "displayInfo is null!");
        return WMError::WM_ERROR_NULLPTR;
    }
    WSError ret;
    if (targetOrientation == Orientation::INVALID) {
        Orientation requestedOrientation = ConvertInvalidOrientation();
        ret = hostSession->GetTargetOrientationConfigInfo(requestedOrientation,
            targetPageProperties, currentPageProperties);
    } else {
        ret = hostSession->GetTargetOrientationConfigInfo(targetOrientation,
            targetPageProperties, currentPageProperties);
    }
    getTargetInfoCallback_->ResetGetTargetRotationLock();
    std::pair<OrientationInfo, OrientationInfo> infoResult =
        getTargetInfoCallback_->GetTargetOrientationResult(WINDOW_PAGE_ROTATION_TIMEOUT);
    OrientationInfo info = infoResult.first;
    OrientationInfo currentInfo = infoResult.second;
    Ace::ViewportConfig config = FillTargetOrientationConfig(info, displayInfo, GetDisplayId());
    targetViewportConfigAndAvoidArea.config = std::make_shared<Ace::ViewportConfig>(config);
    targetViewportConfigAndAvoidArea.avoidAreas = info.avoidAreas;
    Ace::ViewportConfig currentConfig = FillTargetOrientationConfig(currentInfo, displayInfo, GetDisplayId());
    currentViewportConfigAndAvoidArea.config = std::make_shared<Ace::ViewportConfig>(currentConfig);
    currentViewportConfigAndAvoidArea.avoidAreas = currentInfo.avoidAreas;
    TLOGI(WmsLogTag::WMS_ROTATION,
        "win:%{public}u, rotate:%{public}d, rect:%{public}s, avoidAreas:%{public}s,%{public}s,%{public}s,%{public}s",
        GetWindowId(), info.rotation, info.rect.ToString().c_str(),
        info.avoidAreas[AvoidAreaType::TYPE_SYSTEM].ToString().c_str(),
        info.avoidAreas[AvoidAreaType::TYPE_CUTOUT].ToString().c_str(),
        info.avoidAreas[AvoidAreaType::TYPE_KEYBOARD].ToString().c_str(),
        info.avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].ToString().c_str());
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "GetTargetOrientationConfigInfo: targetOrientation:%u, rotation:%d, rect:%s",
        static_cast<uint32_t>(targetOrientation), info.rotation, info.rect.ToString().c_str());

    return static_cast<WMError>(ret);
}

Ace::ViewportConfig WindowSceneSessionImpl::FillTargetOrientationConfig(
    const OrientationInfo& info, const sptr<DisplayInfo>& displayInfo, uint64_t displayId)
{
    Ace::ViewportConfig config;
    Rect targetRect = info.rect;
    uint32_t targetRotation = info.rotation;
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "displayInfo is null!");
        return config;
    }
    auto deviceRotation = static_cast<uint32_t>(displayInfo->GetDefaultDeviceRotationOffset());
    uint32_t transformHint = (targetRotation * ONE_FOURTH_FULL_CIRCLE_DEGREE + deviceRotation) % FULL_CIRCLE_DEGREE;
    float density = GetVirtualPixelRatio(displayInfo);
    virtualPixelRatio_ = density;
    config.SetSize(targetRect.width_, targetRect.height_);
    config.SetPosition(targetRect.posX_, targetRect.posY_);
    config.SetDensity(density);
    config.SetOrientation(targetRotation);
    config.SetTransformHint(transformHint);
    config.SetDisplayId(displayId);
    return config;
}

WSError WindowSceneSessionImpl::NotifyTargetRotationInfo(OrientationInfo& info, OrientationInfo& cuurentInfo)
{
    WSError ret = WSError::WS_OK;
    if (getTargetInfoCallback_) {
        ret = getTargetInfoCallback_->OnUpdateTargetOrientationInfo(info, cuurentInfo);
    }
    return ret;
}

WMError WindowSceneSessionImpl::SetAspectRatio(float ratio)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (IsAdaptToProportionalScale()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Window is in compatibility mode, windowId: %{public}u", GetWindowId());
        return WMError::WM_OK;
    }
    auto hostSession = GetHostSession();
    if (hostSession == nullptr) {
        WLOGFE("failed, because of nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (ratio == MathHelper::INF || ratio == MathHelper::NAG_INF || std::isnan(ratio) || MathHelper::NearZero(ratio)) {
        WLOGFE("failed, because of wrong value: %{public}f", ratio);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (hostSession->SetAspectRatio(ratio) != WSError::WS_OK) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    property_->SetAspectRatio(ratio);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::ResetAspectRatio()
{
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    WSError ret = hostSession->SetAspectRatio(0.0f);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed, windowId: %{public}u, ret: %{public}d",
            GetWindowId(), static_cast<int32_t>(ret));
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    property_->SetAspectRatio(0.0f);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetContentAspectRatio(float ratio, bool isPersistent, bool needUpdateRect)
{
    auto windowId = GetWindowId();
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Only allowed for the main window, windowId: %{public}u", windowId);
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid session, windowId: %{public}u", windowId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (IsAdaptToProportionalScale()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Window is in compatibility mode, windowId: %{public}u", windowId);
        return WMError::WM_OK;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    if (ratio == MathHelper::INF || ratio == MathHelper::NAG_INF || std::isnan(ratio) || MathHelper::NearZero(ratio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid ratio: %{public}f, windowId: %{public}u", ratio, windowId);
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT,
        "windowId: %{public}u, ratio: %{public}f, isPersistent: %{public}d, needUpdateRect: %{public}d",
        windowId, ratio, isPersistent, needUpdateRect);
    if (hostSession->SetContentAspectRatio(ratio, isPersistent, needUpdateRect) != WSError::WS_OK) {
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    property_->SetAspectRatio(ratio);
    return WMError::WM_OK;
}

/** @note @window.hierarchy */
WMError WindowSceneSessionImpl::RaiseToAppTop()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    WLOGFI("id: %{public}d", GetPersistentId());
    auto parentId = GetParentId();
    if (parentId == INVALID_SESSION_ID) {
        WLOGFE("Only the children of the main window can be raised!");
        return WMError::WM_ERROR_INVALID_PARENT;
    }

    if (!WindowHelper::IsSubWindow(GetType())) {
        WLOGFE("Must be app sub window window!");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    if (state_ != WindowState::STATE_SHOWN) {
        WLOGFE("The sub window must be shown!");
        return WMError::WM_DO_NOTHING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    WSError ret = hostSession->RaiseToAppTop();
    return static_cast<WMError>(ret);
}

/** @note @window.hierarchy */
WMError WindowSceneSessionImpl::RaiseAboveTarget(int32_t subWindowId)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    auto parentId = GetParentId();
    auto currentWindowId = GetWindowId();

    if (parentId == INVALID_SESSION_ID) {
        WLOGFE("Only the children of the main window can be raised!");
        return WMError::WM_ERROR_INVALID_PARENT;
    }

    auto subWindows = Window::GetSubWindow(parentId);
    auto targetWindow = find_if(subWindows.begin(), subWindows.end(), [subWindowId](sptr<Window>& window) {
        return static_cast<uint32_t>(subWindowId) == window->GetWindowId();
    });
    if (targetWindow == subWindows.end()) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (!WindowHelper::IsSubWindow(GetType())) {
        WLOGFE("Must be app sub window window!");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    if ((state_ != WindowState::STATE_SHOWN) ||
        ((*targetWindow)->GetWindowState() != WindowState::STATE_SHOWN)) {
        WLOGFE("The sub window must be shown!");
        return WMError::WM_DO_NOTHING;
    }
    if (currentWindowId == static_cast<uint32_t>(subWindowId)) {
        return WMError::WM_OK;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    WSError ret = hostSession->RaiseAboveTarget(subWindowId);
    return static_cast<WMError>(ret);
}

/** @note @window.hierarchy */
WMError WindowSceneSessionImpl::RaiseMainWindowAboveTarget(int32_t targetId)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "source id: %{public}u, target id: %{public}u", GetWindowId(), targetId);
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "device type not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    auto targetIter = std::find_if(windowSessionMap_.begin(), windowSessionMap_.end(),
        [targetId](const auto& windowInfoPair) {return windowInfoPair.second.first == targetId;});
    if (targetIter == windowSessionMap_.end()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "target id invalid or pid inconsistent with source window pid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto targetSessionImpl = targetIter->second.second;
    if (IsWindowSessionInvalid() || targetSessionImpl == nullptr || targetSessionImpl->GetHostSession() == nullptr ||
        targetSessionImpl->state_ == WindowState::STATE_DESTROYED) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "source window or target window is null or destroyed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (state_ != WindowState::STATE_SHOWN || targetSessionImpl->state_ != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "both source window and target window must be shown");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType()) || !WindowHelper::IsMainWindow(targetSessionImpl->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "window type not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (WindowHelper::IsModalMainWindow(GetType(), GetWindowFlags()) ||
        WindowHelper::IsModalMainWindow(targetSessionImpl->GetType(), targetSessionImpl->GetWindowFlags())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "both source window and target window must be not modal");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (IsApplicationModalSubWindowShowing(GetWindowId()) || IsApplicationModalSubWindowShowing(targetId)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "application sub window is not allowed");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (IsMainWindowTopmost() || targetSessionImpl->IsMainWindowTopmost() || IsTopmost() ||
        targetSessionImpl->IsTopmost()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "both source window and target window must be not topmost");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    WSError ret = hostSession->RaiseMainWindowAboveTarget(targetId);
    return static_cast<WMError>(ret);
}

/** @note @window.hierarchy */
WMError WindowSceneSessionImpl::SetSubWindowZLevel(int32_t zLevel)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "%{public}d", zLevel);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!windowSystemConfig_.supportZLevel_) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    if (!WindowHelper::IsNormalSubWindow(GetType(), property_->GetWindowFlags())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "must be normal app sub window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (GetParentId() == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "only the children of the main window can be raised");
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    if (zLevel > MAXIMUM_Z_LEVEL || zLevel < MINIMUM_Z_LEVEL) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "zLevel value %{public}d exceeds valid range [-10000, 10000]!", zLevel);
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    auto currentZLevel = property_->GetSubWindowZLevel();
    if (currentZLevel == static_cast<int32_t>(zLevel)) {
        return WMError::WM_OK;
    }
    property_->SetSubWindowZLevel(zLevel);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL);
}

/** @note @window.hierarchy */
WMError WindowSceneSessionImpl::GetSubWindowZLevel(int32_t& zLevel)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!windowSystemConfig_.supportZLevel_) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    if (!WindowHelper::IsSubWindow(GetType()) || !WindowHelper::IsDialogWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "must be app sub window!");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    zLevel = property_->GetSubWindowZLevel();
    TLOGI(WmsLogTag::WMS_HIERARCHY, "Id:%{public}u, zLevel:%{public}d", GetWindowId(), zLevel);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea,
    const Rect& rect, int32_t apiVersion)
{
    uint32_t currentApiVersion = GetTargetAPIVersionByApplicationInfo();
    apiVersion = (apiVersion == API_VERSION_INVALID) ? static_cast<int32_t>(currentApiVersion) : apiVersion;
    if (apiVersion < API_VERSION_18 && WindowHelper::IsSystemWindow(property_->GetWindowType())) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u type %{public}d api %{public}u not supported",
            GetWindowId(), type, apiVersion);
        return WMError::WM_OK;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    WSRect sessionRect = {
        rect.posX_, rect.posY_, static_cast<int32_t>(rect.width_), static_cast<int32_t>(rect.height_)
    };
    avoidArea = hostSession->GetAvoidAreaByType(type, sessionRect, apiVersion);
    getAvoidAreaCnt_++;
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] type %{public}d times %{public}u area %{public}s",
          GetWindowId(), GetWindowName().c_str(), type, getAvoidAreaCnt_.load(), avoidArea.ToString().c_str());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type,
    AvoidArea& avoidArea, const Rect& rect)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (type == AvoidAreaType::TYPE_KEYBOARD) {
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    WSRect sessionRect = {
        rect.posX_, rect.posY_, static_cast<int32_t>(rect.width_), static_cast<int32_t>(rect.height_)
    };
    avoidArea = hostSession->GetAvoidAreaByTypeIgnoringVisibility(type, sessionRect);
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] type %{public}d area %{public}s",
          GetWindowId(), GetWindowName().c_str(), type, avoidArea.ToString().c_str());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::NotifyWindowNeedAvoid(bool status)
{
    TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u status %{public}d",
        GetWindowId(), static_cast<int32_t>(status));
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->OnNeedAvoid(status);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetIgnoreSafeArea(bool isIgnoreSafeArea)
{
    return SetLayoutFullScreenByApiVersion(isIgnoreSafeArea);
}

WMError WindowSceneSessionImpl::SetLayoutFullScreenByApiVersion(bool status)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t version = 0;
    auto context = GetContext();
    if ((context != nullptr) && (context->GetApplicationInfo() != nullptr)) {
        version = context->GetApplicationInfo()->apiCompatibleVersion;
    }
    isIgnoreSafeArea_ = status;
    isIgnoreSafeAreaNeedNotify_ = true;
    // 10 ArkUI new framework support after API10
    if (version >= 10) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u status %{public}d",
            GetWindowId(), static_cast<int32_t>(status));
        if (auto uiContent = GetUIContentSharedPtr()) {
            uiContent->SetIgnoreViewSafeArea(status);
        }
    } else {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u status %{public}d",
            GetWindowId(), static_cast<int32_t>(status));
        WMError ret = WMError::WM_OK;
        if (status) {
            ret = RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_IMMS, "remove window flag, win %{public}u errCode %{public}d",
                    GetWindowId(), static_cast<int32_t>(ret));
                return ret;
            }
        } else {
            ret = AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_IMMS, "add window flag, win %{public}u errCode %{public}d",
                    GetWindowId(), static_cast<int32_t>(ret));
                return ret;
            }
        }
        ret = NotifyWindowNeedAvoid(!status);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_IMMS, "notify window need avoid, win %{public}u errCode %{public}d",
                GetWindowId(), static_cast<int32_t>(ret));
            return ret;
        }
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetLayoutFullScreen(bool status)
{
    TLOGD(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] status %{public}d",
        GetWindowId(), GetWindowName().c_str(), static_cast<int32_t>(status));
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsSystemWindow(GetType())) {
        TLOGI(WmsLogTag::WMS_IMMS, "system window not supported");
        return WMError::WM_OK;
    }
    if (IsPcOrPadFreeMultiWindowMode() && property_->IsAdaptToImmersive()) {
        SetLayoutFullScreenByApiVersion(status);
        // compatibleMode app may set statusBarColor before ignoreSafeArea
        auto systemBarProperties = property_->GetSystemBarProperty();
        if (status && systemBarProperties.find(WindowType::WINDOW_TYPE_STATUS_BAR) != systemBarProperties.end()) {
            auto statusBarProperty = systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR];
            HookDecorButtonStyleInCompatibleMode(statusBarProperty.contentColor_);
        }
        return WMError::WM_OK;
    }
    bool preStatus = property_->IsLayoutFullScreen();
    property_->SetIsLayoutFullScreen(status);
    auto hostSession = GetHostSession();
    if (hostSession != nullptr) {
        hostSession->OnLayoutFullScreenChange(status);
    }
    if (WindowHelper::IsMainWindow(GetType()) && !windowSystemConfig_.IsPhoneWindow() &&
        !windowSystemConfig_.IsPadWindow()) {
        if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
            WindowMode::WINDOW_MODE_FULLSCREEN)) {
            TLOGE(WmsLogTag::WMS_IMMS, "fullscreen window mode not supported");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        if (property_->IsFullScreenDisabled()) {
            TLOGE(WmsLogTag::WMS_IMMS, "compatible mode disable fullscreen");
            return WMError::WM_OK;
        }
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
        auto event = GetSessionEvent();
        hostSession->OnSessionEvent(event);
        SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    }
    WMError ret = SetLayoutFullScreenByApiVersion(status);
    if (ret != WMError::WM_OK) {
        property_->SetIsLayoutFullScreen(preStatus);
        TLOGE(WmsLogTag::WMS_IMMS, "failed, win %{public}u errCode %{public}d",
            GetWindowId(), static_cast<int32_t>(ret));
    }
    enableImmersiveMode_ = status;
    return ret;
}

SessionEvent WindowSceneSessionImpl::GetSessionEvent()
{
    auto event = SessionEvent::EVENT_MAXIMIZE;
    if (isFullScreenWaterfallMode_.load()) {
        event = SessionEvent::EVENT_MAXIMIZE_WATERFALL;
    } else if (isWaterfallToMaximize_.load()) {
        event = SessionEvent::EVENT_WATERFALL_TO_MAXIMIZE;
        isWaterfallToMaximize_.store(false);
    }
    return event;
}

WMError WindowSceneSessionImpl::SetTitleAndDockHoverShown(
    bool isTitleHoverShown, bool isDockHoverShown)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "winId:%{public}u %{public}s isTitleHoverShown:%{public}d, "
        "isDockHoverShown:%{public}d", GetWindowId(), GetWindowName().c_str(), isTitleHoverShown, isDockHoverShown);
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "window is not main window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "This is PcAppInPad, not supported");
        return WMError::WM_OK;
    }
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    titleHoverShowEnabled_ = isTitleHoverShown;
    dockHoverShowEnabled_ = isDockHoverShown;
    if (auto hostSession = GetHostSession()) {
        hostSession->OnTitleAndDockHoverShowChange(isTitleHoverShown, isDockHoverShown);
    }
    return WMError::WM_OK;
}

bool WindowSceneSessionImpl::IsLayoutFullScreen() const
{
    if (IsWindowSessionInvalid()) {
        return false;
    }
    WindowType winType = property_->GetWindowType();
    if (WindowHelper::IsMainWindow(winType)) {
        return (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN && isIgnoreSafeArea_);
    }
    if (WindowHelper::IsSubWindow(winType)) {
        return isIgnoreSafeAreaNeedNotify_ && isIgnoreSafeArea_;
    }
    return false;
}

SystemBarProperty WindowSceneSessionImpl::GetSystemBarPropertyByType(WindowType type) const
{
    auto curProperties = property_->GetSystemBarProperty();
    return curProperties[type];
}

WMError WindowSceneSessionImpl::NotifyWindowSessionProperty()
{
    TLOGD(WmsLogTag::WMS_IMMS, "windowId:%{public}u", GetWindowId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::NotifySpecificWindowSessionProperty(WindowType type, const SystemBarProperty& property)
{
    TLOGD(WmsLogTag::WMS_IMMS, "windowId:%{public}u", GetWindowId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (type == WindowType::WINDOW_TYPE_STATUS_BAR) {
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS);
        if (auto uiContent = GetUIContentSharedPtr()) {
            uiContent->SetStatusBarItemColor(property.contentColor_);
            AAFwk::Want want;
            want.SetParam(Extension::HOST_STATUS_BAR_CONTENT_COLOR, static_cast<int32_t>(property.contentColor_));
            uiContent->SendUIExtProprty(
                static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_STATUS_BAR_CONTENT_COLOR), want,
                static_cast<uint8_t>(SubSystemId::WM_UIEXT));
        }
        if (property_->IsAdaptToImmersive() && isIgnoreSafeArea_) {
            HookDecorButtonStyleInCompatibleMode(property.contentColor_);
        }
    } else if (type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS);
    } else if (type == WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR) {
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);
    }
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::HookDecorButtonStyleInCompatibleMode(uint32_t color)
{
    if (!property_->IsAdaptToImmersive()) {
        return;
    }
    // alpha Color Channel
    auto alpha = (color >> 24) & 0xFF;
    // R Color Channel
    auto red = (color >> 16) & 0xFF;
    // G Color Channel
    auto green = (color >> 8) & 0xFF;
    // B Color Channel
    auto blue = color & 0xFF;
    // calculate luminance, Identify whether a color is more black or more white.
    double luminance = 0.299 * red + 0.587 * green + 0.114 * blue;
    DecorButtonStyle style;
    style.colorMode = ((luminance > (0xFF>>1)) || (alpha == 0x00)) ? LIGHT_COLOR_MODE : DARK_COLOR_MODE;
    TLOGI(WmsLogTag::WMS_DECOR, "contentColor:%{public}d luminance:%{public}f colorMode:%{public}d",
        color, luminance, style.colorMode);
    SetDecorButtonStyle(style);
}

WMError WindowSceneSessionImpl::SetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGI(WmsLogTag::WMS_IMMS, "only main window support");
        return WMError::WM_OK;
    }
    if (!(state_ > WindowState::STATE_INITIAL && state_ < WindowState::STATE_BOTTOM)) {
        TLOGE(WmsLogTag::WMS_IMMS, "win %{public}u invalid state", GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    } else if (GetSystemBarPropertyByType(type) == property &&
        property.settingFlag_ == SystemBarSettingFlag::DEFAULT_SETTING) {
        setSameSystembarPropertyCnt_++;
        TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] set "
            "%{public}u %{public}u %{public}x %{public}x %{public}u, for %{public}u times",
            GetWindowId(), GetWindowName().c_str(), static_cast<uint32_t>(type), property.enable_,
            property.backgroundColor_, property.contentColor_, property.enableAnimation_,
            setSameSystembarPropertyCnt_);
        return WMError::WM_OK;
    }
    setSameSystembarPropertyCnt_ = 0;
    if (!(GetSystemBarPropertyByType(type) == property)) {
        TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] type %{public}u "
            "%{public}u %{public}x %{public}x %{public}u %{public}u",
            GetWindowId(), GetWindowName().c_str(), static_cast<uint32_t>(type), property.enable_,
            property.backgroundColor_, property.contentColor_, property.enableAnimation_, property.settingFlag_);
    }

    isSystembarPropertiesSet_ = true;
    property_->SetSystemBarProperty(type, property);
    WMError ret = NotifySpecificWindowSessionProperty(type, property);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "win %{public}u errCode %{public}d",
            GetWindowId(), static_cast<int32_t>(ret));
    }
    return ret;
}

WMError WindowSceneSessionImpl::UpdateSystemBarProperties(
    const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
{
    for (auto [systemBarType, systemBarPropertyFlag] : systemBarPropertyFlags) {
        if (systemBarProperties.find(systemBarType) == systemBarProperties.end()) {
            TLOGE(WmsLogTag::WMS_IMMS, "system bar type is invalid");
            return WMError::WM_DO_NOTHING;
        }
        auto property = GetSystemBarPropertyByType(systemBarType);
        property.enable_ = systemBarPropertyFlag.enableFlag ?
            systemBarProperties.at(systemBarType).enable_ : property.enable_;
        property.settingFlag_ |= systemBarPropertyFlag.enableFlag ?
            SystemBarSettingFlag::ENABLE_SETTING : SystemBarSettingFlag::DEFAULT_SETTING;
        property.backgroundColor_ = systemBarPropertyFlag.backgroundColorFlag ?
            systemBarProperties.at(systemBarType).backgroundColor_ : property.backgroundColor_;
        property.contentColor_ = systemBarPropertyFlag.contentColorFlag ?
            systemBarProperties.at(systemBarType).contentColor_ : property.contentColor_;
        property.settingFlag_ |=
            (systemBarPropertyFlag.backgroundColorFlag || systemBarPropertyFlag.contentColorFlag) ?
            SystemBarSettingFlag::COLOR_SETTING : SystemBarSettingFlag::DEFAULT_SETTING;
        property.enableAnimation_ = systemBarPropertyFlag.enableAnimationFlag ?
            systemBarProperties.at(systemBarType).enableAnimation_ : property.enableAnimation_;

        auto ret = UpdateSystemBarPropertyForPage(systemBarType, property, systemBarPropertyFlag);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_IMMS, "set failed");
            return ret;
        }
    }
    SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty navigationIndicatorPorperty =
        GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
    MobileAppInPadLayoutFullScreenChange(statusProperty.enable_, navigationIndicatorPorperty.enable_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::UpdateSystemBarPropertyForPage(WindowType type,
    const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag)
{
    if (systemBarPropertyFlag.contentColorFlag) {
        UpdateStatusBarColorHistory(StatusBarColorChangeReason::WINDOW_CONFIGURATION,
            std::optional<uint32_t>(systemBarProperty.contentColor_));
    }
    {
        std::lock_guard<std::mutex> lock(nowsystemBarPropertyMapMutex_);
        auto iter = nowsystemBarPropertyMap_.find(type);
        if (iter != nowsystemBarPropertyMap_.end()) {
            iter->second.enable_ = systemBarPropertyFlag.enableFlag ? systemBarProperty.enable_ : iter->second.enable_;
            iter->second.backgroundColor_ = systemBarPropertyFlag.backgroundColorFlag ?
                systemBarProperty.backgroundColor_ : iter->second.backgroundColor_;
            iter->second.contentColor_ = systemBarPropertyFlag.contentColorFlag ?
                systemBarProperty.contentColor_ : iter->second.contentColor_;
            iter->second.enableAnimation_ = systemBarPropertyFlag.enableAnimationFlag ?
                systemBarProperty.enableAnimation_ : iter->second.enableAnimation_;
            iter->second.settingFlag_ |= systemBarProperty.settingFlag_;
        } else {
            nowsystemBarPropertyMap_[type] = systemBarProperty;
        }
    }
    auto ret = SetSystemBarProperty(type, nowsystemBarPropertyMap_[type]);
    if (ret == WMError::WM_OK) {
        property_->SetSystemBarProperty(type, systemBarProperty);
    }
    return ret;
}

void WindowSceneSessionImpl::MobileAppInPadLayoutFullScreenChange(bool statusBarEnable, bool navigationEnable)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "isMobileAppInPadLayoutFullScreen %{public}d",
        property_->GetMobileAppInPadLayoutFullScreen());
    if (property_->GetMobileAppInPadLayoutFullScreen() && GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN) {
        TLOGI(WmsLogTag::WMS_COMPAT, "statusProperty %{public}d, navigationIndicatorPorperty %{public}d",
            statusBarEnable, navigationEnable);
        if (!statusBarEnable && !navigationEnable) {
            Maximize(MaximizePresentation::ENTER_IMMERSIVE);
        }
        if (statusBarEnable && navigationEnable) {
            Maximize(MaximizePresentation::EXIT_IMMERSIVE);
        }
    }
}

WMError WindowSceneSessionImpl::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    return SetSpecificBarProperty(type, property);
}

WMError WindowSceneSessionImpl::SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
    const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto type = WindowType::WINDOW_TYPE_STATUS_BAR;
    bool finshUpdate = false;
    {
        std::lock_guard<std::mutex> lock(nowsystemBarPropertyMapMutex_);
        auto iter = nowsystemBarPropertyMap_.find(type);
        if (iter == nowsystemBarPropertyMap_.end()) {
            nowsystemBarPropertyMap_[type] = GetSystemBarPropertyByType(type);
        }
        auto flagIter = propertyFlags.find(type);
        auto propertyIter = properties.find(type);
        if (flagIter != propertyFlags.end() && flagIter->second.contentColorFlag &&
            propertyIter != properties.end() && propertyIter->second.contentColor_ != iter->second.contentColor_) {
            iter->second.contentColor_ = propertyIter->second.contentColor_;
            iter->second.settingFlag_ |= SystemBarSettingFlag::COLOR_SETTING;
            UpdateStatusBarColorHistory(StatusBarColorChangeReason::NAVIGATION_CONFIGURATION,
                std::optional<uint32_t>(propertyIter->second.contentColor_));
            isNavigationUseColor_ = true;
            finshUpdate = true;
        }
    }
    if (finshUpdate) {
        SetSpecificBarProperty(type, nowsystemBarPropertyMap_[type]);
        property_->SetSystemBarProperty(type, GetSystemBarPropertyByType(type));
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties)
{
    {
        std::lock_guard<std::mutex> lock(nowsystemBarPropertyMapMutex_);
        auto iter = nowsystemBarPropertyMap_.find(WindowType::WINDOW_TYPE_STATUS_BAR);
        if (iter != nowsystemBarPropertyMap_.end()) {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR] = iter->second;
        } else {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR] =
                GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        }
    }
    return WMError::WM_OK;
}

uint32_t WindowSceneSessionImpl::UpdateStatusBarColorHistory(
    StatusBarColorChangeReason reason, std::optional<uint32_t> color)
{
    std::stack<std::pair<StatusBarColorChangeReason, uint32_t>> tmpStatusBarColorHistory;
    while (!statusBarColorHistory_.empty()) {
        auto top = statusBarColorHistory_.top();
        statusBarColorHistory_.pop();
        if (top.first == reason) {
            break;
        }
        tmpStatusBarColorHistory.push(top);
    }
    while (!tmpStatusBarColorHistory.empty()) {
        statusBarColorHistory_.push(tmpStatusBarColorHistory.top());
        tmpStatusBarColorHistory.pop();
    }
    if (color != std::nullopt) {
        statusBarColorHistory_.push(std::pair<StatusBarColorChangeReason, uint32_t>(reason, color.value()));
    }
    if (statusBarColorHistory_.empty()) {
        auto property = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        auto defaultConfig = std::pair<StatusBarColorChangeReason, uint32_t>(
            StatusBarColorChangeReason::WINDOW_CONFIGURATION, property.contentColor_);
        auto config = color == std::nullopt ?
            defaultConfig : std::pair<StatusBarColorChangeReason, uint32_t>(reason, color.value());
        statusBarColorHistory_.push(config);
    }
    return statusBarColorHistory_.top().second;
}

WMError WindowSceneSessionImpl::SetSystemBarPropertyForPage(WindowType type, std::optional<SystemBarProperty> property)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "win %{public}u invalid state", GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGI(WmsLogTag::WMS_IMMS, "only main window support, win %{public}u", GetWindowId());
        return WMError::WM_DO_NOTHING;
    }
    auto newProperty = GetSystemBarPropertyByType(type);
    {
        std::lock_guard<std::mutex> lock(nowsystemBarPropertyMapMutex_);
        if (property == std::nullopt) {
            nowsystemBarPropertyMap_[type].enable_ = newProperty.enable_;
            nowsystemBarPropertyMap_[type].enableAnimation_ = newProperty.enableAnimation_;
            auto flag = (static_cast<uint32_t>(nowsystemBarPropertyMap_[type].settingFlag_) &
                ~static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING)) |
                static_cast<uint32_t>(newProperty.settingFlag_);
            nowsystemBarPropertyMap_[type].settingFlag_ = static_cast<SystemBarSettingFlag>(flag);
        } else {
            nowsystemBarPropertyMap_[type].enable_ = property.value().enable_;
            nowsystemBarPropertyMap_[type].enableAnimation_ = newProperty.enableAnimation_;
            nowsystemBarPropertyMap_[type].settingFlag_ |= SystemBarSettingFlag::ENABLE_SETTING;
        }
        newProperty = nowsystemBarPropertyMap_[type];
    }
    return updateSystemBarproperty(type, newProperty);
}

WMError WindowSceneSessionImpl::SetStatusBarColorForPage(const std::optional<uint32_t> color)
{
    auto type = WindowType::WINDOW_TYPE_STATUS_BAR;
    auto newProperty = GetSystemBarPropertyByType(type);
    {
        std::lock_guard<std::mutex> lock(nowsystemBarPropertyMapMutex_);
        if (color == std::nullopt && !isAtomicServiceUseColor_) {
            return WMError::WM_DO_NOTHING;
        }
        if (color == std::nullopt) {
            if (!isNavigationUseColor_) {
                auto flag = (static_cast<uint32_t>(nowsystemBarPropertyMap_[type].settingFlag_) &
                    ~static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING)) |
                    static_cast<uint32_t>(newProperty.settingFlag_);
                nowsystemBarPropertyMap_[type].settingFlag_ = static_cast<SystemBarSettingFlag>(flag);
            }
            auto statusBarColor =
                UpdateStatusBarColorHistory(StatusBarColorChangeReason::ATOMICSERVICE_CONFIGURATION, color);
            if (!statusBarColorHistory_.empty() &&
                statusBarColorHistory_.top().first == StatusBarColorChangeReason::ATOMICSERVICE_CONFIGURATION) {
                nowsystemBarPropertyMap_[type].contentColor_ = statusBarColor;
            }
            isAtomicServiceUseColor_ = false;
        } else {
            nowsystemBarPropertyMap_[type].settingFlag_ |= SystemBarSettingFlag::COLOR_SETTING;
            nowsystemBarPropertyMap_[type].contentColor_ =
                UpdateStatusBarColorHistory(StatusBarColorChangeReason::ATOMICSERVICE_CONFIGURATION, color);
            isAtomicServiceUseColor_ = true;
        }
    }
    return updateSystemBarproperty(type, nowsystemBarPropertyMap_[type]);
}

WMError WindowSceneSessionImpl::updateSystemBarproperty(WindowType type, const SystemBarProperty& systemBarProperty)
{
    auto winProperty = GetSystemBarPropertyByType(type);
    property_->SetSystemBarProperty(type, systemBarProperty);
    auto ret = NotifySpecificWindowSessionProperty(type, systemBarProperty);
    property_->SetSystemBarProperty(type, winProperty);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "update prop fail, ret %{public}u", ret);
        return ret;
    }
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::GetSystemBarPropertyForPage(const std::map<WindowType, SystemBarProperty>& properties,
    std::map<WindowType, SystemBarProperty>& pageProperties)
{
    for (auto type : { WindowType::WINDOW_TYPE_STATUS_BAR, WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR }) {
        if (properties.find(type) != properties.end()) {
            pageProperties[type] = properties.at(type);
        } else {
            pageProperties[type] = GetSystemBarPropertyByType(type);
        }
        TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] type %{public}u "
            "%{public}u %{public}x %{public}x %{public}u %{public}u",
            GetWindowId(), GetWindowName().c_str(), static_cast<uint32_t>(type),
            pageProperties[type].enable_, pageProperties[type].backgroundColor_,
            pageProperties[type].contentColor_, pageProperties[type].enableAnimation_,
            pageProperties[type].settingFlag_);
    }
}

WMError WindowSceneSessionImpl::SetFullScreen(bool status)
{
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] status %{public}d",
        GetWindowId(), GetWindowName().c_str(), static_cast<int32_t>(status));
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsSystemWindow(GetType())) {
        TLOGI(WmsLogTag::WMS_IMMS, "system window not supported");
        return WMError::WM_OK;
    }

    if (WindowHelper::IsMainWindow(GetType()) && IsPcOrPadFreeMultiWindowMode()) {
        if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
            WindowMode::WINDOW_MODE_FULLSCREEN)) {
            TLOGE(WmsLogTag::WMS_IMMS, "fullscreen window not supported");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        if (property_->IsFullScreenDisabled()) {
            TLOGE(WmsLogTag::WMS_IMMS, "compatible mode disable fullscreen");
            return WMError::WM_OK;
        }
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
        hostSession->OnSessionEvent(SessionEvent::EVENT_MAXIMIZE);
        SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    };

    WMError ret = SetLayoutFullScreenByApiVersion(status);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed, win %{public}u errCode %{public}d",
            GetWindowId(), static_cast<int32_t>(ret));
    }

    UpdateDecorEnable(true);
    SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    statusProperty.enable_ = !status;
    ret = SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusProperty);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "SetSystemBarProperty win %{public}u errCode %{public}d",
            GetWindowId(), static_cast<int32_t>(ret));
    }

    return ret;
}

bool WindowSceneSessionImpl::IsFullScreen() const
{
    SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    return (IsLayoutFullScreen() && !statusProperty.enable_);
}

bool WindowSceneSessionImpl::IsDecorEnable() const
{
    WindowType windowType = GetType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    bool isValidWindow = isMainWindow ||
        ((isSubWindow || isDialogWindow || IsSubWindowMaximizeSupported()) && property_->IsDecorEnable());
    bool isWindowModeSupported = WindowHelper::IsWindowModeSupported(
        windowSystemConfig_.decorWindowModeSupportType_, GetWindowMode());
    if (windowSystemConfig_.freeMultiWindowSupport_) {
        return isValidWindow && windowSystemConfig_.isSystemDecorEnable_;
    }
    bool enable = isValidWindow && windowSystemConfig_.isSystemDecorEnable_ &&
        isWindowModeSupported;
    if ((isSubWindow || isDialogWindow) && property_->GetIsPcAppInPad() && property_->IsDecorEnable()) {
        enable = true;
    }
    if (IsSubWindowMaximizeSupported() && property_->IsDecorEnable()) {
        enable = true;
    }
    TLOGD(WmsLogTag::WMS_DECOR, "get decor enable %{public}d", enable);
    return enable;
}

WMError WindowSceneSessionImpl::SetWindowTitle(const std::string& title)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_DECOR, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!IsDecorEnable()) {
        TLOGE(WmsLogTag::WMS_DECOR, "DecorEnable is false");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(GetType())) {
        auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(GetContext());
        if (abilityContext == nullptr) {
            return WMError::WM_ERROR_NULLPTR;
        }
        abilityContext->SetMissionLabel(title);
    } else {
        return SetAPPWindowLabel(title);
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::Minimize()
{
    WLOGFI("id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    if (WindowHelper::IsMainWindow(GetType())) {
        hostSession->OnSessionEvent(SessionEvent::EVENT_MINIMIZE);
    } else {
        WLOGFE("This window state is abnormal.");
        return WMError::WM_DO_NOTHING;
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::MaximizeForCompatibleMode()
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "maximize fail, not main");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->OnSessionEvent(SessionEvent::EVENT_COMPATIBLE_TO_MAXIMIZE);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::RecoverForCompatibleMode()
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "recover fail, not main");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->OnSessionEvent(SessionEvent::EVENT_COMPATIBLE_TO_RECOVER);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SwitchCompatibleMode(CompatibleStyleMode styleMode)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "switch compatible style fail, not main");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    SessionEventParam param;
    param.compatibleStyleMode = static_cast<uint32_t>(styleMode);

    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->OnSessionEvent(SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE, param);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::Maximize()
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(GetType())) {
        UpdateIsShowDecorInFreeMultiWindow(true);
        SetLayoutFullScreen(enableImmersiveMode_);
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::Maximize(MaximizePresentation presentation)
{
    return Maximize(presentation, WaterfallResidentState::UNCHANGED);
}

bool WindowSceneSessionImpl::CheckWaterfallResidentState(WaterfallResidentState state) const
{
    if (WindowHelper::IsSubWindow(GetType())) {
        return state == WaterfallResidentState::UNCHANGED;
    }
    return true;
}

void WindowSceneSessionImpl::ApplyMaximizePresentation(MaximizePresentation presentation)
{
    titleHoverShowEnabled_ = true;
    dockHoverShowEnabled_ = true;
    switch (presentation) {
        case MaximizePresentation::ENTER_IMMERSIVE:
            enableImmersiveMode_ = true;
            break;
        case MaximizePresentation::EXIT_IMMERSIVE:
            enableImmersiveMode_ = false;
            break;
        case MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER:
            enableImmersiveMode_ = true;
            titleHoverShowEnabled_ = false;
            dockHoverShowEnabled_ = false;
            break;
        case MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING:
            break;
        default:
            TLOGE(WmsLogTag::WMS_LAYOUT_PC,
                  "Invalid presentation. windowId: %{public}u, presentation: %{public}u",
                  GetWindowId(), static_cast<uint32_t>(presentation));
            break;
    }
}

WMError WindowSceneSessionImpl::Maximize(MaximizePresentation presentation, WaterfallResidentState state)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!CheckWaterfallResidentState(state)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Invalid waterfallResidentState. windowId: %{public}u, state: %{public}u",
              GetWindowId(), static_cast<uint32_t>(state));
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        if (IsSubWindowMaximizeSupported()) {
            return MaximizeFloating();
        }
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "maximize fail, not main or sub window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
        WindowMode::WINDOW_MODE_FULLSCREEN)) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    // The device is not supported
    if (!IsPcOrPadFreeMultiWindowMode() || property_->IsFullScreenDisabled()) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "The device is not supported");
        return WMError::WM_OK;
    }
    ApplyMaximizePresentation(presentation);
    UpdateIsShowDecorInFreeMultiWindow(true);
    property_->SetIsLayoutFullScreen(enableImmersiveMode_);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
    hostSession->OnTitleAndDockHoverShowChange(titleHoverShowEnabled_, dockHoverShowEnabled_);
    SetLayoutFullScreenByApiVersion(enableImmersiveMode_);
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "present: %{public}d, enableImmersiveMode_:%{public}d!",
        presentation, enableImmersiveMode_.load());
    SessionEventParam param;
    param.waterfallResidentState = static_cast<uint32_t>(state);
    hostSession->OnSessionEvent(SessionEvent::EVENT_MAXIMIZE, param);
    return SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
}

WMError WindowSceneSessionImpl::MaximizeFloating()
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    if (!WindowHelper::IsMainWindow(property_->GetWindowType()) && !IsSubWindowMaximizeSupported()) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC,
              "SetGlobalMaximizeMode fail, not main or sub window, id %{public}d",
              GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
        WindowMode::WINDOW_MODE_FULLSCREEN)) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (property_->IsFullScreenDisabled()) {
        TLOGW(WmsLogTag::WMS_COMPAT, "diable fullScreen in compatibleMode window ,id:%{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (property_->GetMobileAppInPadLayoutFullScreen()) {
        SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        SystemBarProperty navigationIndicatorPorperty =
            GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
        TLOGI(WmsLogTag::WMS_COMPAT, "statusProperty %{public}d, navigationIndicatorPorperty %{public}d",
            statusProperty.enable_, navigationIndicatorPorperty.enable_);
        if (!statusProperty.enable_ && !navigationIndicatorPorperty.enable_) {
            Maximize(MaximizePresentation::ENTER_IMMERSIVE);
            return WMError::WM_OK;
        }
    }
    MaximizeEvent(hostSession);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
 
    return WMError::WM_OK;
}
 
void WindowSceneSessionImpl::MaximizeEvent(const sptr<ISession> &hostSession)
{
    if (hostSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "hostSession is nullptr");
        return;
    }
    if (GetGlobalMaximizeMode() != MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        const bool isPcMode = system::GetBoolParameter("persist.sceneboard.ispcmode", false);
        if (IsFreeMultiWindowMode() && !isPcMode && !property_->GetIsPcAppInPad() &&
            WindowHelper::IsMainWindow(GetType())) {
            UpdateIsShowDecorInFreeMultiWindow(false);
            hostSession->OnSessionEvent(SessionEvent::EVENT_MAXIMIZE_FULLSCREEN);
        } else {
            hostSession->OnSessionEvent(SessionEvent::EVENT_MAXIMIZE);
        }
        SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
        UpdateDecorEnable(true);
        property_->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    } else {
        hostSession->OnSessionEvent(SessionEvent::EVENT_MAXIMIZE_FLOATING);
        SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        property_->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
        UpdateDecorEnable(true);
        NotifyWindowStatusChange(GetWindowMode());
    }
}

WMError WindowSceneSessionImpl::Recover()
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d", GetPersistentId());
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && isFullScreenWaterfallMode_.load() &&
        lastWindowModeBeforeWaterfall_.load() == WindowMode::WINDOW_MODE_FULLSCREEN) {
        isWaterfallToMaximize_.store(true);
        SetFullScreenWaterfallMode(false);
        WMError ret = Maximize();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "recover to fullscreen failed");
            isWaterfallToMaximize_.store(false);
            SetFullScreenWaterfallMode(true);
        }
        return ret;
    }
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FLOATING)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "not support floating, can not Recover");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    if (WindowHelper::IsMainWindow(GetType()) || IsSubWindowMaximizeSupported()) {
        if (property_->GetMaximizeMode() == MaximizeMode::MODE_RECOVER &&
            property_->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING) {
            TLOGW(WmsLogTag::WMS_LAYOUT_PC, "Recover fail, already MODE_RECOVER");
            return WMError::WM_ERROR_REPEAT_OPERATION;
        }
        if (enableImmersiveMode_) {
            enableImmersiveMode_ = false;
            property_->SetIsLayoutFullScreen(enableImmersiveMode_);
            hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
        }
        hostSession->OnSessionEvent(SessionEvent::EVENT_RECOVER);
        SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        property_->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
        UpdateDecorEnable(true);
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
        NotifyWindowStatusChange(GetWindowMode());
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "recovery is invalid, window id: %{public}d", GetPersistentId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::Restore()
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Restore fail, not main window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!(windowSystemConfig_.IsPcWindow() || property_->GetIsPcAppInPad())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "This is not PC or PcAppInPad, not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    WSError ret = hostSession->OnRestoreMainWindow();
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "OnRestoreMainWindow result is: %{public}d", static_cast<int32_t>(ret));
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::Recover(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d, reason: %{public}u", GetPersistentId(), reason);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "The device is not supported");
        return WMError::WM_OK;
    }
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FLOATING)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "not support floating, can not Recover");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    if (WindowHelper::IsMainWindow(GetType()) || IsSubWindowMaximizeSupported()) {
        if (property_->GetMaximizeMode() == MaximizeMode::MODE_RECOVER &&
            property_->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING) {
            TLOGW(WmsLogTag::WMS_LAYOUT_PC, "Recover fail, already MODE_RECOVER");
            return WMError::WM_ERROR_REPEAT_OPERATION;
        }
        if (enableImmersiveMode_) {
            enableImmersiveMode_ = false;
            property_->SetIsLayoutFullScreen(enableImmersiveMode_);
            hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
        }
        hostSession->OnSessionEvent(SessionEvent::EVENT_RECOVER);
        // need notify arkui maximize mode change
        if (reason == REASON_MAXIMIZE_MODE_CHANGE &&
            property_->GetMaximizeMode() == MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
            UpdateMaximizeMode(MaximizeMode::MODE_RECOVER);
        }
        SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        property_->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
        UpdateDecorEnable(true);
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
        NotifyWindowStatusChange(GetWindowMode());
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "recovery is invalid on sub window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag)
{
    TLOGI(WmsLogTag::WMS_MAIN, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_MAIN, "This is not main window, not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    hostSession->OnSetWindowRectAutoSave(enabled, isSaveBySpecifiedFlag);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::IsWindowRectAutoSave(bool& enabled)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_MAIN, "This is not main window, not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "context is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    std::string bundleName = property_->GetSessionInfo().bundleName_;
    std::string moduleName = property_->GetSessionInfo().moduleName_;
    std::string abilityName = property_->GetSessionInfo().abilityName_;
    if (abilityContext && abilityContext->GetAbilityInfo()) {
        abilityName = abilityContext->GetAbilityInfo()->name;
        moduleName = context->GetHapModuleInfo() ? context->GetHapModuleInfo()->moduleName : "";
        bundleName = abilityContext->GetAbilityInfo()->bundleName;
    } else if (context) {
        moduleName = context->GetHapModuleInfo() ? context->GetHapModuleInfo()->moduleName : "";
        bundleName = context->GetBundleName();
    }
    std::string key = bundleName + moduleName + abilityName;
    int persistentId = GetPersistentId();
    auto ret = SingletonContainer::Get<WindowAdapter>().IsWindowRectAutoSave(key, enabled, persistentId);
    return ret;
}

WMError WindowSceneSessionImpl::SetSupportedWindowModes(
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes, bool grayOutMaximizeButton)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "This is PcAppInpad, not supported");
        return WMError::WM_OK;
    }

    if (!(windowSystemConfig_.IsPcWindow() || windowSystemConfig_.freeMultiWindowSupport_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Neither is Pc nor support free multi window, invalid calling");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "This is not main window, not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    if (grayOutMaximizeButton) {
        size_t size = supportedWindowModes.size();
        if (size == 0 || size > WINDOW_SUPPORT_MODE_MAX_SIZE) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "mode param is invalid");
            return WMError::WM_ERROR_ILLEGAL_PARAM;
        }
        if (std::find(supportedWindowModes.begin(), supportedWindowModes.end(),
            AppExecFwk::SupportWindowMode::FULLSCREEN) != supportedWindowModes.end()) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Supports full screen cannot be grayed out");
            return WMError::WM_ERROR_ILLEGAL_PARAM;
        }
        GrayOutMaximizeButton(true);
    } else if (grayOutMaximizeButton_) {
        GrayOutMaximizeButton(false);
    }

    return SetSupportedWindowModesInner(supportedWindowModes);
}

WMError WindowSceneSessionImpl::GrayOutMaximizeButton(bool isGrayOut)
{
    if (grayOutMaximizeButton_ == isGrayOut) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "Duplicate settings are gray out: %{public}d", isGrayOut);
        return WMError::WM_DO_NOTHING;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    grayOutMaximizeButton_ = isGrayOut;
    uiContent->OnContainerModalEvent(WINDOW_GRAY_OUT_MAXIMIZE_EVENT, isGrayOut ? "true" : "false");
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::UpdateWindowModeWhenSupportTypeChange(uint32_t windowModeSupportType)
{
    bool onlySupportFullScreen =
        WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN) &&
        !WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    bool disableFullScreen = property_->IsFullScreenDisabled();
    if (onlySupportFullScreen && !property_->IsLayoutFullScreen() && !disableFullScreen) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "onlySupportFullScreen:%{public}d IsLayoutFullScreen:%{public}d",
            onlySupportFullScreen, property_->IsLayoutFullScreen());
        Maximize(MaximizePresentation::ENTER_IMMERSIVE);
        return;
    }

    bool onlySupportFloating =
        !WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN) &&
        WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    if (onlySupportFloating) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "onlySupportFloating:%{public}d", onlySupportFloating);
        Recover(REASON_MAXIMIZE_MODE_CHANGE);
    }
}

WMError WindowSceneSessionImpl::SetSupportedWindowModesInner(
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes)
{
    auto size = supportedWindowModes.size();
    if (size <= 0 || size > WINDOW_SUPPORT_MODE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "mode param is invalid");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    uint32_t windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(supportedWindowModes);
    if (windowModeSupportType == 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "mode param is 0");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    bool onlySupportSplit = (windowModeSupportType ==
                            (WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                             WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY));
    if (onlySupportSplit) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "mode param is only support split");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "winId: %{public}u, windowModeSupportType: %{public}u",
        GetWindowId(), windowModeSupportType);

    // update windowModeSupportType to server
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    hostSession->NotifySupportWindowModesChange(supportedWindowModes);
    if (!IsPcOrPadFreeMultiWindowMode()) {
        pendingWindowModeSupportType_ = windowModeSupportType;
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "pending update, winId: %{public}u, windowModeSupportType: %{public}u",
            GetWindowId(), windowModeSupportType);
        return WMError::WM_OK;
    }

    haveSetSupportedWindowModes_ = true;
    property_->SetWindowModeSupportType(windowModeSupportType);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
    UpdateTitleButtonVisibility();
    UpdateWindowModeWhenSupportTypeChange(windowModeSupportType);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit)
{
    int32_t persistentId = GetPersistentId();
    return SingletonContainer::Get<WindowAdapter>().SetImageForRecent(imgResourceId, imageFit, persistentId);
}

WMError WindowSceneSessionImpl::SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap,
    ImageFit imageFit)
{
    int32_t persistentId = GetPersistentId();
    return SingletonContainer::Get<WindowAdapter>().SetImageForRecentPixelMap(pixelMap, imageFit, persistentId);
}

WMError WindowSceneSessionImpl::RemoveImageForRecent()
{
    int32_t persistentId = GetPersistentId();
    return SingletonContainer::Get<WindowAdapter>().RemoveImageForRecent(persistentId);
}

/** @note @window.drag */
void WindowSceneSessionImpl::StartMove()
{
    WLOGFI("id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return;
    }
    WindowType windowType = GetType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    bool isDecorDialog = isDialogWindow && property_->IsDecorEnable();
    bool isValidWindow = isMainWindow || (IsPcOrFreeMultiWindowCapabilityEnabled() && (isSubWindow || isDecorDialog));
    auto hostSession = GetHostSession();
    if (isValidWindow && hostSession) {
        hostSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
    }
}

bool WindowSceneSessionImpl::IsStartMoving()
{
    bool isMoving = false;
    if (auto hostSession = GetHostSession()) {
        isMoving = hostSession->IsStartMoving();
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "id: %{public}d, isMoving: %{public}d", GetPersistentId(), isMoving);
    return isMoving;
}

bool WindowSceneSessionImpl::CalcWindowShouldMove()
{
    if ((windowSystemConfig_.IsPhoneWindow() ||
        (windowSystemConfig_.IsPadWindow() && !IsFreeMultiWindowMode())) &&
        GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING) {
            return true;
    }

    if (IsPcOrFreeMultiWindowCapabilityEnabled()) {
        return true;
    }
    return false;
}

bool WindowSceneSessionImpl::CheckCanMoveWindowType()
{
    WindowType windowType = GetType();
    if (!WindowHelper::IsSystemWindow(windowType) &&
        !WindowHelper::IsMainWindow(windowType) &&
        !WindowHelper::IsSubWindow(windowType)) {
        return false;
    }
    return true;
}

bool WindowSceneSessionImpl::CheckCanMoveWindowTypeByDevice()
{
    WindowType windowType = GetType();
    bool isPcOrFreeMultiWindowCanMove = IsPcOrFreeMultiWindowCapabilityEnabled() &&
        (WindowHelper::IsSystemWindow(windowType) ||
         WindowHelper::IsMainWindow(windowType) ||
         WindowHelper::IsSubWindow(windowType));
    bool isPhoneWindowCanMove = (windowSystemConfig_.IsPhoneWindow() ||
        (windowSystemConfig_.IsPadWindow() && !IsFreeMultiWindowMode())) &&
        (WindowHelper::IsSystemWindow(windowType) || WindowHelper::IsSubWindow(windowType));
    if (isPcOrFreeMultiWindowCanMove || isPhoneWindowCanMove) {
        return true;
    }
    return false;
}

bool WindowSceneSessionImpl::CheckIsPcAppInPadFullScreenOnMobileWindowMode()
{
    auto mode = property_->GetWindowMode();
    if (property_->GetIsPcAppInPad() &&
        (mode == WindowMode::WINDOW_MODE_FULLSCREEN ||
         mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
         mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) &&
        !IsFreeMultiWindowMode()) {
        return true;
    }
    return false;
}

WmErrorCode WindowSceneSessionImpl::StartMoveWindow()
{
    if (!CheckCanMoveWindowTypeByDevice()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "invalid window type:%{public}u", GetType());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }
    if (!CalcWindowShouldMove()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "The device is not supported");
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (CheckIsPcAppInPadFullScreenOnMobileWindowMode()) {
        return WmErrorCode::WM_OK;
    }
    if (auto hostSession = GetHostSession()) {
        WSError errorCode = hostSession->SyncSessionEvent(SessionEvent::EVENT_START_MOVE);
        TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, errorCode:%{public}d",
            GetPersistentId(), static_cast<int>(errorCode));
        switch (errorCode) {
            case WSError::WS_ERROR_REPEAT_OPERATION: {
                return WmErrorCode::WM_ERROR_REPEAT_OPERATION;
            }
            case WSError::WS_ERROR_NULLPTR: {
                return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            }
            default: {
                return WmErrorCode::WM_OK;
            }
        }
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "hostSession is nullptr");
        return WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY;
    }
}

WmErrorCode WindowSceneSessionImpl::StartMoveWindowWithCoordinate(int32_t offsetX, int32_t offsetY)
{
    if (!CheckCanMoveWindowType()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "invalid window type:%{public}u", GetType());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }
    if (!IsPcOrFreeMultiWindowCapabilityEnabled()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "The device is not supported");
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (CheckIsPcAppInPadFullScreenOnMobileWindowMode()) {
        return WmErrorCode::WM_OK;
    }
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    const auto& windowRect = GetRect();
    if (offsetX < 0 || offsetX > static_cast<int32_t>(windowRect.width_) ||
        offsetY < 0 || offsetY > static_cast<int32_t>(windowRect.height_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "offset not in window");
        return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    WSError errorCode;
    MMI::PointerEvent::PointerItem pointerItem;
    if (lastPointerEvent_ != nullptr &&
        lastPointerEvent_->GetPointerItem(lastPointerEvent_->GetPointerId(), pointerItem)) {
        int32_t lastDisplayX = pointerItem.GetDisplayX();
        int32_t lastDisplayY = pointerItem.GetDisplayY();
        int32_t lastDisplayId = lastPointerEvent_->GetTargetDisplayId();
        TLOGD(WmsLogTag::WMS_LAYOUT_PC, "offset:[%{public}d,%{public}d] lastEvent:[%{private}d,%{private}d]"
            " lastDisplayId:%{public}d", offsetX, offsetY, lastDisplayX, lastDisplayY, lastDisplayId);
        errorCode = hostSession->StartMovingWithCoordinate(offsetX, offsetY, lastDisplayX, lastDisplayY, lastDisplayId);
    } else {
        errorCode = hostSession->SyncSessionEvent(SessionEvent::EVENT_START_MOVE);
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "id:%{public}d, errorCode:%{public}d",
          GetPersistentId(), static_cast<int>(errorCode));
    switch (errorCode) {
        case WSError::WS_ERROR_REPEAT_OPERATION: {
            return WmErrorCode::WM_ERROR_REPEAT_OPERATION;
        }
        case WSError::WS_ERROR_NULLPTR: {
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
        default: {
            return WmErrorCode::WM_OK;
        }
    }
}

WmErrorCode WindowSceneSessionImpl::StopMoveWindow()
{
    if (!CheckCanMoveWindowType()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "invalid window type:%{public}u", GetType());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }
    if (!IsPcOrFreeMultiWindowCapabilityEnabled()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "The device is not supported");
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "hostSession is nullptr");
        return WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    WSError errorCode = hostSession->SyncSessionEvent(SessionEvent::EVENT_END_MOVE);
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "id:%{public}d, errorCode:%{public}d",
          GetPersistentId(), static_cast<int>(errorCode));
    return errorCode == WSError::WS_ERROR_NULLPTR ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : WmErrorCode::WM_OK;
}

WMError WindowSceneSessionImpl::MainWindowCloseInner()
{
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(GetContext());
    if (!abilityContext) {
        return Destroy(true);
    }
    bool terminateCloseProcess = false;
    WMError res = NotifyMainWindowClose(terminateCloseProcess);
    if (res == WMError::WM_OK) {
        TLOGI(WmsLogTag::WMS_DECOR, "id: %{public}d, not close: %{public}d", GetPersistentId(),
            terminateCloseProcess);
        if (!terminateCloseProcess) {
            hostSession->OnSessionEvent(SessionEvent::EVENT_CLOSE);
        }
        return res;
    }
    auto handler = sptr<WindowPrepareTerminateHandler>::MakeSptr();
    PrepareTerminateFunc func = [hostSessionWptr = wptr<ISession>(hostSession)] {
        auto hostSession = hostSessionWptr.promote();
        if (hostSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "this session is nullptr");
            return;
        }
        hostSession->OnSessionEvent(SessionEvent::EVENT_CLOSE);
    };
    handler->SetPrepareTerminateFun(func);
    if (AAFwk::AbilityManagerClient::GetInstance()->PrepareTerminateAbility(abilityContext->GetToken(),
        handler) != ERR_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "PrepareTerminateAbility failed, do close window");
        hostSession->OnSessionEvent(SessionEvent::EVENT_CLOSE);
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::Close()
{
    TLOGI(WmsLogTag::WMS_DECOR, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_DECOR, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WindowType windowType = GetType();
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isSystemSubWindow = WindowHelper::IsSystemSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    if (WindowHelper::IsMainWindow(windowType) || isSubWindow) {
        WMError res = NotifyWindowWillClose(this);
        if (res == WMError::WM_OK) {
            TLOGI(WmsLogTag::WMS_DECOR, "id: %{public}d will close", GetPersistentId());
            return res;
        }
    }
    if (WindowHelper::IsMainWindow(windowType)) {
        return MainWindowCloseInner();
    } else if (isSubWindow || isSystemSubWindow || isDialogWindow) {
        TLOGI(WmsLogTag::WMS_DECOR, "subwindow or dialog");
        bool terminateCloseProcess = false;
        NotifySubWindowClose(terminateCloseProcess);
        if (!terminateCloseProcess || isDialogWindow) {
            return Destroy(true);
        }
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::CloseDirectly()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_DECOR, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_DECOR, "id: %{public}d", GetPersistentId());
    if (WindowHelper::IsMainWindow(GetType())) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
        hostSession->OnSessionEvent(SessionEvent::EVENT_CLOSE);
        return WMError::WM_OK;
    }
    return Destroy(true);
}

WMError WindowSceneSessionImpl::DisableAppWindowDecor()
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_DECOR, "disable app window decor permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "window decoration is invalid on sub window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    TLOGI(WmsLogTag::WMS_DECOR, "disable app window decoration.");
    windowSystemConfig_.isSystemDecorEnable_ = false;
    UpdateDecorEnable(true);
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::PerformBack()
{
    if (!WindowHelper::IsMainWindow(GetType())) {
        WLOGFI("PerformBack is not MainWindow, return");
        return;
    }
    if (auto hostSession = GetHostSession()) {
        bool needMoveToBackground = false;
        auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(GetContext());
        if (abilityContext != nullptr) {
            abilityContext->OnBackPressedCallBack(needMoveToBackground);
        }
        TLOGI(WmsLogTag::DEFAULT, "%{public}d", needMoveToBackground);
        hostSession->RequestSessionBack(needMoveToBackground);
    }
}

WMError WindowSceneSessionImpl::SetGlobalMaximizeMode(MaximizeMode mode)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "mode %{public}u", static_cast<uint32_t>(mode));
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    if (WindowHelper::IsMainWindow(GetType())) {
        hostSession->SetGlobalMaximizeMode(mode);
        return WMError::WM_OK;
    } else {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "fail, not main window");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
}

MaximizeMode WindowSceneSessionImpl::GetGlobalMaximizeMode() const
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    MaximizeMode mode = MaximizeMode::MODE_RECOVER;
    if (auto hostSession = GetHostSession()) {
        hostSession->GetGlobalMaximizeMode(mode);
    }
    return mode;
}

WMError WindowSceneSessionImpl::SetWindowMode(WindowMode mode)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, windowModeSupportType: %{public}u, mode: %{public}u",
        GetWindowId(), property_->GetWindowModeSupportType(), static_cast<uint32_t>(mode));
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(), mode)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "window %{public}u do not support mode: %{public}u",
            GetWindowId(), static_cast<uint32_t>(mode));
        return WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }
    WMError ret = UpdateWindowModeImmediately(mode);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Update window mode fail, ret:%{public}u", ret);
        return ret;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY) {
        hostSession->OnSessionEvent(SessionEvent::EVENT_SPLIT_PRIMARY);
    } else if (mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        hostSession->OnSessionEvent(SessionEvent::EVENT_SPLIT_SECONDARY);
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetGrayScale(float grayScale)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    constexpr float eps = 1e-6f;
    if (grayScale < (MIN_GRAY_SCALE - eps) || grayScale > (MAX_GRAY_SCALE + eps)) {
        WLOGFE("invalid grayScale value: %{public}f", grayScale);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        WLOGFE("uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent->SetContentNodeGrayScale(grayScale);
    WLOGI("Set window gray scale success, grayScale: %{public}f", grayScale);
    return WMError::WM_OK;
}

WindowMode WindowSceneSessionImpl::GetWindowMode() const
{
    return property_->GetWindowMode();
}

bool WindowSceneSessionImpl::IsTransparent() const
{
    if (IsWindowSessionInvalid()) {
        return false;
    }
    WSColorParam backgroundColor;
    backgroundColor.value = GetBackgroundColor();
    WLOGFD("color: %{public}u, alpha: %{public}u", backgroundColor.value, backgroundColor.argb.alpha);
    return backgroundColor.argb.alpha == 0x00; // 0x00: completely transparent
}

WMError WindowSceneSessionImpl::SetTransparent(bool isTransparent)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WSColorParam backgroundColor;
    backgroundColor.value = GetBackgroundColor();
    if (isTransparent) {
        backgroundColor.argb.alpha = 0x00; // 0x00: completely transparent
        return SetBackgroundColor(backgroundColor.value);
    } else {
        backgroundColor.value = GetBackgroundColor();
        if (backgroundColor.argb.alpha == 0x00) {
            backgroundColor.argb.alpha = 0xff; // 0xff: completely opaque
            return SetBackgroundColor(backgroundColor.value);
        }
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::AddWindowFlag(WindowFlag flag)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto context = GetContext();
    if (flag == WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED && context && context->GetApplicationInfo() &&
        context->GetApplicationInfo()->apiCompatibleVersion >= 9 && // 9: api version
        !SessionPermission::IsSystemCalling()) {
        TLOGI(WmsLogTag::DEFAULT, "Can not set show when locked, PersistentId: %{public}u", GetPersistentId());
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (flag == WindowFlag::WINDOW_FLAG_HANDWRITING && !SessionPermission::IsSystemCalling()) {
        TLOGI(WmsLogTag::DEFAULT, "Can not set handwritting, PersistentId: %{public}u", GetPersistentId());
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (flag == WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE && !SessionPermission::IsSystemCalling()) {
        TLOGI(WmsLogTag::DEFAULT, "Can not set forbid split move, PersistentId: %{public}u", GetPersistentId());
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    uint32_t updateFlags = property_->GetWindowFlags() | (static_cast<uint32_t>(flag));
    return SetWindowFlags(updateFlags);
}

WMError WindowSceneSessionImpl::RemoveWindowFlag(WindowFlag flag)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t updateFlags = property_->GetWindowFlags() & (~(static_cast<uint32_t>(flag)));
    return SetWindowFlags(updateFlags);
}

WMError WindowSceneSessionImpl::SetWindowFlags(uint32_t flags)
{
    TLOGI(WmsLogTag::DEFAULT, "Session %{public}u flags %{public}u", GetWindowId(), flags);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (property_->GetWindowFlags() == flags) {
        TLOGI(WmsLogTag::DEFAULT, "Session %{public}u set same flags %{public}u", GetWindowId(), flags);
        return WMError::WM_OK;
    }
    auto oriFlags = property_->GetWindowFlags();
    property_->SetWindowFlags(flags);
    WMError ret = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_FLAGS);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "SetWindowFlags errCode:%{public}d winId:%{public}u",
            static_cast<int32_t>(ret), GetWindowId());
        property_->SetWindowFlags(oriFlags);
    }
    return ret;
}

uint32_t WindowSceneSessionImpl::GetWindowFlags() const
{
    return property_->GetWindowFlags();
}

void WindowSceneSessionImpl::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent->UpdateConfiguration(configuration);
        UpdateColorMode();
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    UpdateDefaultStatusBarColor();
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(GetPersistentId(), subWindows);
    for (auto& subWindowSession : subWindows) {
        if (subWindowSession != nullptr) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene subWin=%{public}u, display=%{public}" PRIu64,
                subWindowSession->GetWindowId(), subWindowSession->GetDisplayId());
            subWindowSession->UpdateConfiguration(configuration);
        }
    }
}

WMError WindowSceneSessionImpl::UpdateColorMode(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    std::string colorMode;
    if (configuration != nullptr) {
        colorMode = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    }
    auto appContext = AbilityRuntime::Context::GetApplicationContext();
    if (appContext == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d app context is null", GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    if (colorMode.empty()) {
        std::shared_ptr<AppExecFwk::Configuration> config = appContext->GetConfiguration();
        if (config == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "config is null, winId: %{public}d", GetPersistentId());
            return WMError::WM_ERROR_NULLPTR;
        }
        colorMode = config->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, colorMode: %{public}s", GetPersistentId(), colorMode.c_str());
    if (colorMode_ == colorMode) {
        return WMError::WM_DO_NOTHING;
    }
    colorMode_ = colorMode;
    UpdateSidebarBlurStyleWhenColorModeChange();
    bool hasDarkRes = false;
    appContext->AppHasDarkRes(hasDarkRes);
    hasDarkRes_ = hasDarkRes;

    if (auto hostSession = GetHostSession()) {
        hostSession->OnUpdateColorMode(colorMode, hasDarkRes);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d hostSession is null", GetPersistentId());
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, colorMode: %{public}s, hasDarkRes: %{public}u",
          GetPersistentId(), colorMode.c_str(), hasDarkRes);
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::UpdateConfigurationForSpecified(
    const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent->UpdateConfiguration(configuration, resourceManager);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    if (configuration != nullptr) {
        specifiedColorMode_ = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "scene win=%{public}u, colorMode=%{public}s, display=%{public}" PRIu64,
            GetWindowId(), specifiedColorMode_.c_str(), GetDisplayId());
    }
    UpdateDefaultStatusBarColor();
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(GetPersistentId(), subWindows);
    if (subWindows.empty()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "no subSession, scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        return;
    }
    for (auto& subWindowSession : subWindows) {
        if (subWindowSession != nullptr) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene subWin=%{public}u, display=%{public}" PRIu64,
                subWindowSession->GetWindowId(), subWindowSession->GetDisplayId());
            subWindowSession->UpdateConfigurationForSpecified(configuration, resourceManager);
        }
    }
}

void WindowSceneSessionImpl::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    std::unordered_set<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowCtxSet(
        ignoreWindowContexts.begin(), ignoreWindowContexts.end());
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene map size: %{public}u", static_cast<uint32_t>(windowSessionMap_.size()));
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& winPair : windowSessionMap_) {
        auto window = winPair.second.second;
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "scene window is null");
            continue;
        }
        if (ignoreWindowCtxSet.count(window->GetContext()) == 0) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
            window->UpdateConfiguration(configuration);
        } else {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "skip scene win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
        }
    }
}

void WindowSceneSessionImpl::UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent->UpdateConfigurationSyncForAll(configuration);
        UpdateColorMode();
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(GetPersistentId(), subWindows);
    if (subWindows.empty()) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "no subSession, scene win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        return;
    }
    for (auto& subWindowSession : subWindows) {
        if (subWindowSession != nullptr) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene subWin=%{public}u, display=%{public}" PRIu64,
                subWindowSession->GetWindowId(), subWindowSession->GetDisplayId());
            subWindowSession->UpdateConfigurationSync(configuration);
        }
    }
}

void WindowSceneSessionImpl::UpdateConfigurationSyncForAll(
    const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene map size: %{public}u", static_cast<uint32_t>(windowSessionMap_.size()));
    for (const auto& winPair : windowSessionMap_) {
        if (auto window = winPair.second.second) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "scene win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
            window->UpdateConfigurationSync(configuration);
        }
    }
}

/** @note @window.hierarchy */
sptr<Window> WindowSceneSessionImpl::GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    uint32_t mainWinId = INVALID_WINDOW_ID;
    {
        std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
        if (windowSessionMap_.empty()) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "Please create mainWindow First!");
            return nullptr;
        }
        for (const auto& winPair : windowSessionMap_) {
            auto win = winPair.second.second;
            if (win && WindowHelper::IsMainWindow(win->GetType()) && context.get() == win->GetContext().get()) {
                mainWinId = win->GetWindowId();
                TLOGD(WmsLogTag::WMS_HIERARCHY, "Find MainWinId:%{public}u.", mainWinId);
                break;
            }
        }
    }
    TLOGI(WmsLogTag::WMS_HIERARCHY, "mainId:%{public}u!", mainWinId);
    if (mainWinId == INVALID_WINDOW_ID) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Cannot find topWindow!");
        return nullptr;
    }
    uint32_t topWinId = INVALID_WINDOW_ID;
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetTopWindowId(mainWinId, topWinId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    return FindWindowById(topWinId);
}

/** @note @window.hierarchy */
sptr<Window> WindowSceneSessionImpl::GetTopWindowWithId(uint32_t mainWinId)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "mainId:%{public}u", mainWinId);
    uint32_t topWinId = INVALID_WINDOW_ID;
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetTopWindowId(mainWinId, topWinId);
    if (ret != WMError::WM_OK) {
        WLOGFE("[GetTopWin] failed with errCode:%{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    return FindWindowById(topWinId);
}

sptr<Window> WindowSceneSessionImpl::GetMainWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    if (windowSessionMap_.empty()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Please create mainWindow First!");
        return nullptr;
    }
    for (const auto& winPair : windowSessionMap_) {
        auto win = winPair.second.second;
        if (win && WindowHelper::IsMainWindow(win->GetType()) && context.get() == win->GetContext().get()) {
            TLOGI(WmsLogTag::WMS_MAIN, "Find MainWinId:%{public}u.", win->GetWindowId());
            return win;
        }
    }
    TLOGE(WmsLogTag::WMS_MAIN, "Cannot find Window!");
    return nullptr;
}

sptr<WindowSessionImpl> WindowSceneSessionImpl::GetMainWindowWithId(uint32_t mainWinId)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    if (windowSessionMap_.empty()) {
        WLOGFE("Please create mainWindow First!");
        return nullptr;
    }
    for (const auto& winPair : windowSessionMap_) {
        auto win = winPair.second.second;
        if (win && WindowHelper::IsMainWindow(win->GetType()) && mainWinId == win->GetWindowId()) {
            WLOGI("GetTopWindow Find MainWinId:%{public}u.", mainWinId);
            return win;
        }
    }
    WLOGFE("Cannot find Window!");
    return nullptr;
}

static WMError GetParentMainWindowIdInner(WindowSessionImplMap& sessionMap, int32_t windowId, int32_t& mainWindowId)
{
    for (const auto& [_, pair] : sessionMap) {
        const auto& window = pair.second;
        if (window == nullptr) {
            return WMError::WM_ERROR_NULLPTR;
        }
        if (window->GetPersistentId() != windowId) {
            continue;
        }

        if (WindowHelper::IsMainWindow(window->GetType())) {
            TLOGI(WmsLogTag::WMS_SUB, "find main window, id:%{public}u", window->GetPersistentId());
            mainWindowId = window->GetPersistentId();
            return WMError::WM_OK;
        }
        if (WindowHelper::IsSubWindow(window->GetType()) || WindowHelper::IsDialogWindow(window->GetType())) {
            return GetParentMainWindowIdInner(sessionMap, window->GetParentId(), mainWindowId);
        }
        // not main window, sub window, dialog, set invalid id
        mainWindowId = INVALID_SESSION_ID;
        return WMError::WM_OK;
    }
    return WMError::WM_ERROR_INVALID_PARENT;
}

int32_t WindowSceneSessionImpl::GetParentMainWindowId(int32_t windowId)
{
    if (windowId == INVALID_SESSION_ID) {
        TLOGW(WmsLogTag::WMS_SUB, "invalid windowId id");
        return INVALID_SESSION_ID;
    }
    int32_t mainWindowId = INVALID_SESSION_ID;
    {
        std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
        WMError findRet = GetParentMainWindowIdInner(windowSessionMap_, windowId, mainWindowId);
        if (findRet == WMError::WM_OK) {
            return mainWindowId;
        }
    }
    // can't find in client, need to find in server find
    WMError ret = SingletonContainer::Get<WindowAdapter>().GetParentMainWindowId(windowId, mainWindowId);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "cant't find main window id, err:%{public}d", static_cast<uint32_t>(ret));
        return INVALID_SESSION_ID;
    }
    return mainWindowId;
}

WMError WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(uint32_t parentId, const std::string& windowName,
    WindowType parentWindowType, WindowType& windowType)
{
    auto window = WindowSceneSessionImpl::Find(windowName);
    if (window != nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "WindowName(%{public}s) already exists.", windowName.c_str());
        return WMError::WM_ERROR_REPEAT_OPERATION;
    }
    TLOGI(WmsLogTag::WMS_SUB, "parentWindowId:%{public}d, parentWindowType:%{public}d", parentId, parentWindowType);
    if (parentWindowType == WindowType::WINDOW_TYPE_SCENE_BOARD ||
        parentWindowType == WindowType::WINDOW_TYPE_DESKTOP) {
        windowType = WindowType::WINDOW_TYPE_SYSTEM_FLOAT;
    } else if (WindowHelper::IsUIExtensionWindow(parentWindowType)) {
        windowType = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    } else if (WindowHelper::IsSystemSubWindow(parentWindowType)) {
        TLOGE(WmsLogTag::WMS_SUB, "parent is system sub window, id: %{public}d, type: %{public}d",
            parentId, parentWindowType);
        return WMError::WM_ERROR_INVALID_TYPE;
    } else if (WindowHelper::IsSystemWindow(parentWindowType)) {
        windowType = WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW;
    } else {
        auto parentWindow = WindowSceneSessionImpl::GetWindowWithId(parentId);
        if (parentWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_SUB, "parentWindow is null, windowId:%{public}d", parentId);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        if (parentWindowType != parentWindow->GetType()) {
            TLOGE(WmsLogTag::WMS_SUB, "parentWindow does match type");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        auto ret = WindowSceneSessionImpl::VerifySubWindowLevel(false, parentWindow);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        windowType = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    }
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::SetNeedDefaultAnimation(bool needDefaultAnimation)
{
    enableDefaultAnimation_= needDefaultAnimation;
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->UpdateWindowAnimationFlag(needDefaultAnimation);
}

static float ConvertRadiusToSigma(float radius)
{
    return radius > 0.0f ? 0.57735f * radius + SK_ScalarHalf : 0.0f; // 0.57735f is blur sigma scale
}

WMError WindowSceneSessionImpl::CheckParmAndPermission()
{
    if (surfaceNode_ == nullptr) {
        WLOGFE("RSSurface node is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        WLOGFE("Check failed, permission denied");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetCornerRadius(float cornerRadius)
{
    if (surfaceNode_ == nullptr) {
        WLOGFE("RSSurface node is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WLOGFI("Set window %{public}s corner radius %{public}f", GetWindowName().c_str(), cornerRadius);
    surfaceNode_->SetCornerRadius(cornerRadius);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetWindowCornerRadius(float cornerRadius)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow() &&
        !windowSystemConfig_.IsPhoneWindow()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not pc, pad or phone, not supported.");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsFloatOrSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not sub window or float window.");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Set id %{public}u corner radius %{public}f.", GetWindowId(), cornerRadius);
    if (MathHelper::LessNotEqual(cornerRadius, 0.0f)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "The corner radius is less than zero.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    property_->SetWindowCornerRadius(cornerRadius);

    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    hostSession->SetWindowCornerRadius(cornerRadius);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetWindowCornerRadius(float& cornerRadius)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow() &&
            !windowSystemConfig_.IsPhoneWindow()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not PC, pad or phone, not supported.");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsFloatOrSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not sub window or float window.");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    cornerRadius = property_->GetWindowCornerRadius();
    if (MathHelper::LessNotEqual(cornerRadius, 0.0f)) {
        // Invalid corner radius means app has not set corner radius of the window, return the default corner radius
        TLOGI(WmsLogTag::WMS_ANIMATION, "System config radius: %{public}f, property radius: %{public}f, id: %{public}d",
            windowSystemConfig_.defaultCornerRadius_, cornerRadius, GetPersistentId());
        cornerRadius = MathHelper::LessNotEqual(windowSystemConfig_.defaultCornerRadius_, 0.0f) ?
            0.0f : windowSystemConfig_.defaultCornerRadius_;
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetShadowRadius(float radius)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s shadow radius %{public}f", GetWindowName().c_str(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    surfaceNode_->SetShadowRadius(ConvertRadiusToSigma(radius));
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SyncShadowsToComponent(const ShadowsInfo& shadowsInfo)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    TLOGI(WmsLogTag::WMS_ANIMATION, "Sync Shadows To Component %{public}s shadow radius: %{public}f,"
        " color: %{public}s, offsetX: %{public}f, offsetY: %{public}f", GetWindowName().c_str(),
        shadowsInfo.radius_, shadowsInfo.color_.c_str(), shadowsInfo.offsetX_, shadowsInfo.offsetY_);
    if (MathHelper::LessNotEqual(shadowsInfo.radius_, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    property_->SetWindowShadows(shadowsInfo);
    auto hostSession = GetHostSession();

    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    hostSession->SetWindowShadows(shadowsInfo);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetWindowShadowRadius(float radius)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow() &&
        !windowSystemConfig_.IsPhoneWindow() && !IsDeviceFeatureCapableForFreeMultiWindow()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "device not support.");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsFloatOrSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not sub window or float window.");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Set id %{public}u shadow radius %{public}f.", GetWindowId(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0f)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "The shadow radius is less than zero.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "RSSurface node is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    surfaceNode_->SetShadowRadius(ConvertRadiusToSigma(radius));
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetShadowColor(std::string color)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s shadow color %{public}s", GetWindowName().c_str(), color.c_str());
    uint32_t colorValue = 0;
    if (!ColorParser::Parse(color, colorValue)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    surfaceNode_->SetShadowColor(colorValue);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetShadowOffsetX(float offsetX)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s shadow offsetX %{public}f", GetWindowName().c_str(), offsetX);
    surfaceNode_->SetShadowOffsetX(offsetX);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetShadowOffsetY(float offsetY)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s shadow offsetY %{public}f", GetWindowName().c_str(), offsetY);
    surfaceNode_->SetShadowOffsetY(offsetY);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetBlur(float radius)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s blur radius %{public}f", GetWindowName().c_str(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    radius = ConvertRadiusToSigma(radius);
    WLOGFI("Set window %{public}s blur radius after conversion %{public}f", GetWindowName().c_str(), radius);
    surfaceNode_->SetFilter(RSFilter::CreateBlurFilter(radius, radius));
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetBackdropBlur(float radius)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s backdrop blur radius %{public}f", GetWindowName().c_str(), radius);
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    radius = ConvertRadiusToSigma(radius);
    WLOGFI("Set window %{public}s backdrop blur radius after conversion %{public}f", GetWindowName().c_str(), radius);
    surfaceNode_->SetBackgroundFilter(RSFilter::CreateBlurFilter(radius, radius));
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetBackdropBlurStyle(WindowBlurStyle blurStyle)
{
    WMError ret = CheckParmAndPermission();
    if (ret != WMError::WM_OK) {
        return ret;
    }

    WLOGFI("Set window %{public}s backdrop blur style %{public}u", GetWindowName().c_str(), blurStyle);
    if (blurStyle < WindowBlurStyle::WINDOW_BLUR_OFF || blurStyle > WindowBlurStyle::WINDOW_BLUR_THICK) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (blurStyle == WindowBlurStyle::WINDOW_BLUR_OFF) {
        surfaceNode_->SetBackgroundFilter(nullptr);
    } else {
        auto display = SingletonContainer::IsDestroyed() ? nullptr :
            SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
        if (display == nullptr) {
            WLOGFE("get display failed displayId:%{public}" PRIu64, property_->GetDisplayId());
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        auto displayInfo = display->GetDisplayInfo();
        if (displayInfo == nullptr) {
            WLOGFE("get displayInfo failed displayId:%{public}" PRIu64, property_->GetDisplayId());
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        surfaceNode_->SetBackgroundFilter(RSFilter::CreateMaterialFilter(
            static_cast<int>(blurStyle), GetVirtualPixelRatio(displayInfo)));
    }

    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetPrivacyMode(bool isPrivacyMode)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WLOGFD("id : %{public}u, %{public}u", GetWindowId(), isPrivacyMode);
    property_->SetPrivacyMode(isPrivacyMode);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
}

bool WindowSceneSessionImpl::IsPrivacyMode() const
{
    if (IsWindowSessionInvalid()) {
        return false;
    }
    return property_->GetPrivacyMode();
}

void WindowSceneSessionImpl::SetSystemPrivacyMode(bool isSystemPrivacyMode)
{
    WLOGFD("id : %{public}u, %{public}u", GetWindowId(), isSystemPrivacyMode);
    property_->SetSystemPrivacyMode(isSystemPrivacyMode);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
}

WMError WindowSceneSessionImpl::SetSnapshotSkip(bool isSkip)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    bool oldSkipValue = property_->GetSnapshotSkip();
    property_->SetSnapshotSkip(isSkip);
    auto resError = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    if (resError != WMError::WM_OK) {
        property_->SetSnapshotSkip(oldSkipValue);
    }
    return resError;
}

std::shared_ptr<Media::PixelMap> WindowSceneSessionImpl::Snapshot()
{
    if (IsWindowSessionInvalid()) {
        return nullptr;
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    auto isSucceeded = RSInterfaces::GetInstance().TakeSurfaceCapture(surfaceNode_, callback);
    if (!isSucceeded) {
        WLOGFE("Failed to TakeSurfaceCapture!");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = callback->GetResult(SNAPSHOT_TIMEOUT);
    if (pixelMap != nullptr) {
        WLOGFD("Snapshot succeed, save WxH=%{public}dx%{public}d", pixelMap->GetWidth(), pixelMap->GetHeight());
    } else {
        WLOGFE("Failed to get pixelmap, return nullptr!");
    }
    return pixelMap;
}

WMError WindowSceneSessionImpl::Snapshot(std::shared_ptr<Media::PixelMap>& pixelMap)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (state_ < WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d bounds not init, state: %{public}u",
            GetPersistentId(), state_);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    auto isSucceeded = RSInterfaces::GetInstance().TakeSurfaceCapture(surfaceNode_, callback);
    if (!isSucceeded) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, Failed to TakeSurfaceCapture", GetPersistentId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    pixelMap = callback->GetResult(SNAPSHOT_TIMEOUT);
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, Failed to get pixelmap", GetPersistentId());
        return WMError::WM_ERROR_TIMEOUT;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d snapshot end, WxH=%{public}dx%{public}d",
        GetPersistentId(), pixelMap->GetWidth(), pixelMap->GetHeight());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    auto isSucceeded = RSInterfaces::GetInstance().TakeSelfSurfaceCapture(surfaceNode_, callback);
    if (!isSucceeded) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowId:%{public}u, Failed to TakeSelfSurfaceCapture!", GetWindowId());
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    pixelMap = callback->GetResult(SNAPSHOT_TIMEOUT);
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to get pixelmap, windowId:%{public}u", GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "succeed, windowId:%{public}u, WxH=%{public}dx%{public}d",
        GetWindowId(), pixelMap->GetWidth(), pixelMap->GetHeight());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::NotifyMemoryLevel(int32_t level)
{
    TLOGI(WmsLogTag::DEFAULT, "id: %{public}u, level: %{public}d", GetWindowId(), level);
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        WLOGFE("Window %{public}s notify failed, uiContent is null.", GetWindowName().c_str());
        return WMError::WM_ERROR_NULLPTR;
    }
    // notify memory level
    uiContent->NotifyMemoryLevel(level);
    WLOGFD("End!");
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetTurnScreenOn(bool turnScreenOn)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetTurnScreenOn(turnScreenOn);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
}

bool WindowSceneSessionImpl::IsTurnScreenOn() const
{
    return property_->IsTurnScreenOn();
}

WMError WindowSceneSessionImpl::SetKeepScreenOn(bool keepScreenOn)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetKeepScreenOn(keepScreenOn);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
}

bool WindowSceneSessionImpl::IsKeepScreenOn() const
{
    return property_->IsKeepScreenOn();
}

WMError WindowSceneSessionImpl::SetViewKeepScreenOn(bool keepScreenOn)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d, enabled: %{public}u", GetPersistentId(), keepScreenOn);
    property_->SetViewKeepScreenOn(keepScreenOn);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_VIEW_KEEP_SCREEN_ON);
}

bool WindowSceneSessionImpl::IsViewKeepScreenOn() const
{
    return property_->IsViewKeepScreenOn();
}

WMError WindowSceneSessionImpl::SetWindowShadowEnabled(bool isEnabled)
{
    if (property_->GetPcAppInpadCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is PcAppInPad, not supported");
        return WMError::WM_OK;
    }
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow()) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetWindowShadowEnabled(isEnabled);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_SHADOW_ENABLED);
}

bool WindowSceneSessionImpl::GetWindowShadowEnabled() const
{
    return property_->GetWindowShadowEnabled();
}

WMError WindowSceneSessionImpl::SetTransform(const Transform& trans)
{
    TLOGI(WmsLogTag::DEFAULT, "Id: %{public}d", property_->GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    Transform oriTrans = property_->GetTransform();
    property_->SetTransform(trans);
    TransformSurfaceNode(trans);
    return WMError::WM_OK;
}

const Transform& WindowSceneSessionImpl::GetTransform() const
{
    return property_->GetTransform();
}

void WindowSceneSessionImpl::TransformSurfaceNode(const Transform& trans)
{
    if (surfaceNode_ == nullptr) {
        return;
    }
    surfaceNode_->SetPivotX(trans.pivotX_);
    surfaceNode_->SetPivotY(trans.pivotY_);
    surfaceNode_->SetScaleX(trans.scaleX_);
    surfaceNode_->SetScaleY(trans.scaleY_);
    surfaceNode_->SetTranslateX(trans.translateX_);
    surfaceNode_->SetTranslateY(trans.translateY_);
    surfaceNode_->SetTranslateZ(trans.translateZ_);
    surfaceNode_->SetRotationX(trans.rotationX_);
    surfaceNode_->SetRotationY(trans.rotationY_);
    surfaceNode_->SetRotation(trans.rotationZ_);
    uint32_t animationFlag = property_->GetAnimationFlag();
    if (animationFlag != static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
        RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    }
}

WMError WindowSceneSessionImpl::RegisterAnimationTransitionController(
    const sptr<IAnimationTransitionController>& listener)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lockListener(transitionControllerMutex_);
        if (std::find(animationTransitionControllers_.begin(), animationTransitionControllers_.end(), listener) ==
            animationTransitionControllers_.end()) {
            animationTransitionControllers_.push_back(listener);
        }
    }

    wptr<WindowSessionProperty> propertyWeak(property_);
    wptr<IAnimationTransitionController> animationTransitionControllerWeak(listener);

    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetNextFrameLayoutCallback([propertyWeak, animationTransitionControllerWeak]() {
            auto property = propertyWeak.promote();
            auto animationTransitionController = animationTransitionControllerWeak.promote();
            if (!property || !animationTransitionController) {
                TLOGE(WmsLogTag::WMS_SYSTEM, "property or animation transition controller is nullptr");
                return;
            }
            uint32_t animationFlag = property->GetAnimationFlag();
            if (animationFlag == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
                // CustomAnimation is enabled when animationTransitionController exists
                animationTransitionController->AnimationForShown();
            }
            TLOGI(WmsLogTag::WMS_SYSTEM, "AnimationForShown excute sucess %{public}d!", property->GetPersistentId());
        });
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "%{public}d!", property_->GetPersistentId());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::UpdateSurfaceNodeAfterCustomAnimation(bool isAdd)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "id:%{public}d, isAdd:%{public}u", property_->GetPersistentId(), isAdd);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsSystemWindow(property_->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "only system window can set");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    // set no custom after customAnimation
    WMError ret = UpdateAnimationFlagProperty(false);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "UpdateAnimationFlagProperty failed!");
        return ret;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    ret = static_cast<WMError>(hostSession->UpdateWindowSceneAfterCustomAnimation(isAdd));
    return ret;
}

void WindowSceneSessionImpl::AdjustWindowAnimationFlag(bool withAnimation)
{
    if (IsWindowSessionInvalid()) {
        WLOGFW("session invalid!");
        return;
    }
    // when show/hide with animation
    // use custom animation when transitionController exists; else use default animation
    WindowType winType = property_->GetWindowType();
    bool isAppWindow = WindowHelper::IsAppWindow(winType);
    if (withAnimation && !isAppWindow && !animationTransitionControllers_.empty()) {
        // use custom animation
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
    } else if ((isAppWindow && enableDefaultAnimation_) || (withAnimation && animationTransitionControllers_.empty())) {
        // use default animation
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::DEFAULT));
    } else {
        // with no animation
        property_->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::NONE));
    }
}

WMError WindowSceneSessionImpl::UpdateAnimationFlagProperty(bool withAnimation)
{
    if (!WindowHelper::IsSystemWindow(GetType())) {
        return WMError::WM_OK;
    }
    AdjustWindowAnimationFlag(withAnimation);
    // when show(true) with default, hide() with None, to adjust animationFlag to disabled default animation
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
}

WMError WindowSceneSessionImpl::SetAlpha(float alpha)
{
    WLOGFI("%{public}d alpha %{public}f", property_->GetPersistentId(), alpha);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        WLOGFE("permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    surfaceNode_->SetAlpha(alpha);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::BindDialogTarget(sptr<IRemoteObject> targetToken)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto persistentId = property_->GetPersistentId();
    TLOGI(WmsLogTag::WMS_DIALOG, "id: %{public}d", persistentId);
    WMError ret = SingletonContainer::Get<WindowAdapter>().BindDialogSessionTarget(persistentId, targetToken);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "bind window failed with errCode:%{public}d", static_cast<int32_t>(ret));
    }
    return ret;
}

WMError WindowSceneSessionImpl::SetDialogBackGestureEnabled(bool isEnabled)
{
    WindowType windowType = GetType();
    if (windowType != WindowType::WINDOW_TYPE_DIALOG) {
        TLOGE(WmsLogTag::WMS_DIALOG, "windowType not support. WinId:%{public}u, WindowType:%{public}u",
            GetWindowId(), static_cast<uint32_t>(windowType));
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto hostSession = GetHostSession();
    if (hostSession == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "set window failed because of nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    WMError ret = static_cast<WMError>(hostSession->SetDialogSessionBackGestureEnabled(isEnabled));
    dialogSessionBackGestureEnabled_ = isEnabled;
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_DIALOG, "set window failed with errCode:%{public}d", static_cast<int32_t>(ret));
    }
    return ret;
}

WMError WindowSceneSessionImpl::SetTouchHotAreas(const std::vector<Rect>& rects)
{
    std::vector<Rect> lastTouchHotAreas;
    property_->GetTouchHotAreas(lastTouchHotAreas);
    property_->SetTouchHotAreas(rects);
    WMError result = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    if (result != WMError::WM_OK) {
        property_->SetTouchHotAreas(lastTouchHotAreas);
        WLOGFE("errCode:%{public}d", static_cast<int32_t>(result));
        return result;
    }
    for (uint32_t i = 0; i < rects.size(); i++) {
        TLOGD(WmsLogTag::WMS_EVENT, "id:%{public}u xywh:[%{public}d %{public}d %{public}u %{public}u]",
            i, rects[i].posX_, rects[i].posY_, rects[i].width_, rects[i].height_);
    }
    return result;
}

WMError WindowSceneSessionImpl::SetKeyboardTouchHotAreas(const KeyboardTouchHotAreas& hotAreas)
{
    if (GetType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    std::vector<Rect> lastTouchHotAreas;
    property_->GetTouchHotAreas(lastTouchHotAreas);
    KeyboardTouchHotAreas lastKeyboardTouchHotAreas = property_->GetKeyboardTouchHotAreas();
    if (IsLandscape(hotAreas.displayId_)) {
        property_->SetTouchHotAreas(hotAreas.landscapeKeyboardHotAreas_);
    } else {
        property_->SetTouchHotAreas(hotAreas.portraitKeyboardHotAreas_);
    }
    property_->SetKeyboardTouchHotAreas(hotAreas);
    WMError result = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_KEYBOARD_TOUCH_HOT_AREA);
    if (result != WMError::WM_OK) {
        property_->SetTouchHotAreas(lastTouchHotAreas);
        property_->SetKeyboardTouchHotAreas(lastKeyboardTouchHotAreas);
        TLOGE(WmsLogTag::WMS_EVENT, "errCode:%{public}d", static_cast<int32_t>(result));
    }
    return result;
}

WmErrorCode WindowSceneSessionImpl::KeepKeyboardOnFocus(bool keepKeyboardFlag)
{
    property_->KeepKeyboardOnFocus(keepKeyboardFlag);
    return WmErrorCode::WM_OK;
}

WMError WindowSceneSessionImpl::ChangeCallingWindowId(uint32_t callingSessionId)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is invalid!");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WindowType type = GetType();
    uint32_t curCallingSessionId = property_->GetCallingSessionId();
    if (type != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT || callingSessionId == curCallingSessionId ||
        state_ != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "set calling window id failed, type: %{public}d, newId: %{public}u,"
            " state: %{public}d", type, callingSessionId, state_);
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "id: %{public}d, curId: %{public}u, newId: %{public}u",
        property_->GetPersistentId(), curCallingSessionId, callingSessionId);
    if (auto hostSession = GetHostSession()) {
        hostSession->SetCallingSessionId(callingSessionId);
    }
    property_->SetCallingSessionId(callingSessionId);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::ChangeKeyboardEffectOption(KeyboardEffectOption effectOption)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "effect option: %{public}s",
        effectOption.ToString().c_str());
    if (effectOption.viewMode_ >= KeyboardViewMode::VIEW_MODE_END ||
        effectOption.flowLightMode_ >= KeyboardFlowLightMode::END ||
        effectOption.gradientMode_ >= KeyboardGradientMode::END) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Invalid view or effect mode! effectOption: %{public}s",
            effectOption.ToString().c_str());
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    auto lastOption = property_->GetKeyboardEffectOption();
    if (effectOption == lastOption) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Is same option: %{public}s", lastOption.ToString().c_str());
        return WMError::WM_DO_NOTHING;
    }

    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is invalid!");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (state_ != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "The keyboard is not show status.");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    property_->SetKeyboardEffectOption(effectOption);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return static_cast<WMError>(hostSession->ChangeKeyboardEffectOption(effectOption));
}

void WindowSceneSessionImpl::DumpSessionElementInfo(const std::vector<std::string>& params)
{
    WLOGFD("in");
    std::vector<std::string> info;
    if (params.size() == 1 && params[0] == PARAM_DUMP_HELP) { // 1: params num
        WLOGFD("Dump ArkUI help Info");
        Ace::UIContent::ShowDumpHelp(info);
        SingletonContainer::Get<WindowAdapter>().NotifyDumpInfoResult(info);
        return;
    }

    WLOGFD("ArkUI:DumpInfo");
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->DumpInfo(params, info);
    }

    for (auto iter = info.begin(); iter != info.end();) {
        if ((*iter).size() == 0) {
            iter = info.erase(iter);
            continue;
        }
        WLOGFD("ElementInfo size: %{public}u", static_cast<uint32_t>((*iter).size()));
        iter++;
    }
    if (info.size() == 0) {
        WLOGFD("ElementInfo is empty");
        return;
    }
    SingletonContainer::Get<WindowAdapter>().NotifyDumpInfoResult(info);
}

WSError WindowSceneSessionImpl::NotifyLayoutFinishAfterWindowModeChange(WindowMode mode)
{
    NotifyWindowStatusDidChange(mode);
    return WSError::WS_OK;
}

bool WindowSceneSessionImpl::ShouldSkipSupportWindowModeCheck(uint32_t windowModeSupportType, WindowMode mode)
{
    bool isFreeMultiWindowMode = IsFreeMultiWindowMode();

    bool onlySupportSplitInFreeMultiWindow =
        isFreeMultiWindowMode && mode == WindowMode::WINDOW_MODE_FLOATING &&
        (WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY) ||
         WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY)) &&
        !WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    if (onlySupportSplitInFreeMultiWindow) {
        return true;
    }
    bool isMultiWindowMode = mode == WindowMode::WINDOW_MODE_FLOATING ||
                             mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
                             mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY;
    bool isAvailableDevice =
        (windowSystemConfig_.IsPhoneWindow() || windowSystemConfig_.IsPadWindow()) && !isFreeMultiWindowMode;
    if (isAvailableDevice && isMultiWindowMode) {
        return true;
    }

    // padMode to FreeMultiWindowMode, the compatible mode application supports MODE_FLOATING and MODE_FULLSCREEN.
    bool isCompatibleWindow = mode == WindowMode::WINDOW_MODE_FLOATING ||
                              mode == WindowMode::WINDOW_MODE_FULLSCREEN;
    bool isDragScale = property_->IsAdaptToDragScale();
    if (isFreeMultiWindowMode && isCompatibleWindow && isDragScale) {
        return true;
    }

    // the full screen mode must be supported in not pc window and not free multi window mode.
    bool isSupportedFullScreen = !windowSystemConfig_.IsPcWindow() && !isFreeMultiWindowMode &&
                                 mode == WindowMode::WINDOW_MODE_FULLSCREEN;
    if (isSupportedFullScreen) {
        return true;
    }
    return false;
}

WSError WindowSceneSessionImpl::UpdateWindowMode(WindowMode mode)
{
    if (IsWindowSessionInvalid()) {
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, windowModeSupportType: %{public}u, mode: %{public}u",
        GetWindowId(), property_->GetWindowModeSupportType(), static_cast<uint32_t>(mode));
    uint32_t windowModeSupportType = property_->GetWindowModeSupportType();
    if (!WindowHelper::IsWindowModeSupported(windowModeSupportType, mode) &&
        !ShouldSkipSupportWindowModeCheck(windowModeSupportType, mode)) {
        WLOGFE("%{public}u do not support mode: %{public}u",
            GetWindowId(), static_cast<uint32_t>(mode));
        return WSError::WS_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }
    if (mode != WindowMode::WINDOW_MODE_FULLSCREEN) {
        UpdateIsShowDecorInFreeMultiWindow(true);
    }
    WMError ret = UpdateWindowModeImmediately(mode);

    if (windowSystemConfig_.IsPcWindow()) {
        if (mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
            ret = SetLayoutFullScreenByApiVersion(true);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_IMMS, "SetLayoutFullScreenByApiVersion errCode:%{public}d winId:%{public}u",
                    static_cast<int32_t>(ret), GetWindowId());
            }
            SystemBarProperty statusProperty = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
            statusProperty.enable_ = false;
            ret = SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusProperty);
            if (ret != WMError::WM_OK) {
                WLOGFE("SetSystemBarProperty errCode:%{public}d winId:%{public}u",
                    static_cast<int32_t>(ret), GetWindowId());
            }
        }
    }
    return static_cast<WSError>(ret);
}

WSError WindowSceneSessionImpl::GetTopNavDestinationName(std::string& topNavDestName)
{
    auto uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null: winId=%{public}u", GetWindowId());
        return WSError::WS_DO_NOTHING;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "call uicontent: winId=%{public}u", GetWindowId());
    std::string navDestInfoJsonStr = uiContent->GetTopNavDestinationInfo(false, false);
    if (navDestInfoJsonStr.empty()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u, empty navDestInfoJsonStr", GetWindowId());
        return WSError::WS_OK;
    }
    nlohmann::json navDestInfoJson = nlohmann::json::parse(navDestInfoJsonStr, nullptr, false);
    if (navDestInfoJson.is_discarded()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "parse json error: winId=%{public}u, navDestInfoJsonStr=%{public}s",
            GetWindowId(), navDestInfoJsonStr.c_str());
        return WSError::WS_DO_NOTHING;
    }
    if (navDestInfoJson.contains("name") && navDestInfoJson["name"].is_string()) {
        navDestInfoJson["name"].get_to(topNavDestName);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u, navDestInfoJsonStr=%{public}s, topNavDestName=%{public}s",
        GetWindowId(), navDestInfoJsonStr.c_str(), topNavDestName.c_str());
    return WSError::WS_OK;
}

WMError WindowSceneSessionImpl::UpdateWindowModeImmediately(WindowMode mode)
{
    if (state_ == WindowState::STATE_CREATED || state_ == WindowState::STATE_HIDDEN) {
        property_->SetWindowMode(mode);
        UpdateTitleButtonVisibility();
        UpdateDecorEnable(true);
    } else if (state_ == WindowState::STATE_SHOWN) {
        // set client window mode if success.
        property_->SetWindowMode(mode);
        UpdateTitleButtonVisibility();
        UpdateDecorEnable(true, mode);
        if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
            property_->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
        }
    }
    NotifyWindowStatusChange(mode);
    return WMError::WM_OK;
}

WSError WindowSceneSessionImpl::UpdateMaximizeMode(MaximizeMode mode)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "%{public}u mode %{public}u", GetWindowId(), static_cast<uint32_t>(mode));
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "uiContent is null");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    uiContent->UpdateMaximizeMode(mode);
    property_->SetMaximizeMode(mode);
    if (mode == MaximizeMode::MODE_RECOVER && enableImmersiveMode_.load()) {
        enableImmersiveMode_.store(false);
        property_->SetIsLayoutFullScreen(false);
        if (auto hostSession = GetHostSession()) {
            hostSession->OnLayoutFullScreenChange(false);
        }
    }
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::NotifySessionFullScreen(bool fullScreen)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "winId: %{public}u", GetWindowId());
    Maximize(fullScreen ? MaximizePresentation::ENTER_IMMERSIVE : MaximizePresentation::EXIT_IMMERSIVE);
}

WSError WindowSceneSessionImpl::UpdateTitleInTargetPos(bool isShow, int32_t height)
{
    TLOGI(WmsLogTag::WMS_DECOR, "%{public}u isShow %{public}u, height %{public}u", GetWindowId(), isShow, height);
    if (IsWindowSessionInvalid()) {
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uiContent is null");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    NotifyTitleChange(isShow, height);
    uiContent->UpdateTitleInTargetPos(isShow, height);
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::UpdateSupportWindowModesWhenSwitchFreeMultiWindow()
{
    if (haveSetSupportedWindowModes_) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "SupportedWindowMode is already set, id: %{public}d", GetPersistentId());
        return;
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context_);
    if (abilityContext == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "abilityContext is nullptr");
        return;
    }
    auto abilityInfo = abilityContext->GetAbilityInfo();
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes =
        ExtractSupportWindowModeFromMetaData(abilityInfo);
    if (auto hostSession = GetHostSession()) {
        hostSession->NotifySupportWindowModesChange(updateWindowModes);
    };
    auto windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(updateWindowModes);
    property_->SetWindowModeSupportType(windowModeSupportType);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
}

void WindowSceneSessionImpl::UpdateEnableDragWhenSwitchMultiWindow(bool enable)
{
    if (hasSetEnableDrag_.load() || property_->IsDragResizeDisabled()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "EnableDrag is already set, id: %{public}d", GetPersistentId());
        return;
    }
    auto isSystemWindow = WindowHelper::IsSystemWindow(property_->GetWindowType());
    bool isDialog = WindowHelper::IsDialogWindow(property_->GetWindowType());
    bool isSystemCalling = property_->GetSystemCalling();
    TLOGI(WmsLogTag::WMS_LAYOUT, "windId: %{public}d, isSystemWindow: %{public}d, isDialog: %{public}d, "
        "isSystemCalling: %{public}d", GetPersistentId(), isSystemWindow, isDialog, isSystemCalling);
    if (!enable || (isSystemWindow && !isDialog && !isSystemCalling)) {
        property_->SetDragEnabled(false);
    } else {
        property_->SetDragEnabled(true);
    }
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
}

WSError WindowSceneSessionImpl::SwitchFreeMultiWindow(bool enable)
{
    if (IsWindowSessionInvalid()) {
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    if (windowSystemConfig_.freeMultiWindowEnable_ == enable) {
        UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
        return WSError::WS_ERROR_REPEAT_OPERATION;
    }
    NotifySwitchFreeMultiWindow(enable);
    NotifyFreeWindowModeChange(enable);
    // Switch process finish, update system config
    SetFreeMultiWindowMode(enable);
    if (enable) {
        PendingUpdateSupportWindowModesWhenSwitchMultiWindow();
    }
    UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
    UpdateEnableDragWhenSwitchMultiWindow(enable);
    if (!enable && !WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
        WindowMode::WINDOW_MODE_FULLSCREEN)) {
        UpdateDecorEnable(true);
    }
    auto mainWindow = FindMainWindowWithContext();
    bool isAnco = mainWindow != nullptr && mainWindow->IsAnco();
    if (WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(), WindowMode::WINDOW_MODE_FLOATING) &&
        !isAnco) {
        UpdateImmersiveBySwitchMode(enable);
    }
    SwitchSubWindow(enable, GetPersistentId());
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::PendingUpdateSupportWindowModesWhenSwitchMultiWindow()
{
    if (pendingWindowModeSupportType_ == WindowModeSupport::WINDOW_MODE_SUPPORT_ALL) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "pending data has not set, id: %{public}d", GetPersistentId());
        return;
    }

    uint32_t windowModeSupportType = pendingWindowModeSupportType_;
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d, windowModeSupportType: %{public}u",
        GetPersistentId(), windowModeSupportType);
    
    pendingWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    property_->SetWindowModeSupportType(windowModeSupportType);

    // update windowModeSupportType to server
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
    UpdateTitleButtonVisibility();
    haveSetSupportedWindowModes_ = true;

    // update window mode immediately when pending window support type take effect
    bool onlySupportFullScreen =
        WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN) &&
        !WindowHelper::IsWindowModeSupported(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    bool disableFullScreen = property_->IsFullScreenDisabled();
    if (onlySupportFullScreen && !disableFullScreen) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "only support fullscreen, enter immersive");
        Maximize(MaximizePresentation::ENTER_IMMERSIVE);
    }
}

void WindowSceneSessionImpl::UpdateImmersiveBySwitchMode(bool freeMultiWindowEnable)
{
    if (freeMultiWindowEnable && enableImmersiveMode_) {
        cacheEnableImmersiveMode_.store(true);
        enableImmersiveMode_.store(false);
        property_->SetIsLayoutFullScreen(enableImmersiveMode_);
        if (auto hostSession = GetHostSession()) {
            hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
        } else {
            TLOGE(WmsLogTag::WMS_LAYOUT, "host session is nullptr, id: %{public}d", GetPersistentId());
        }
    }
    if (!freeMultiWindowEnable && cacheEnableImmersiveMode_) {
        enableImmersiveMode_.store(true);
        cacheEnableImmersiveMode_.store(false);
        property_->SetIsLayoutFullScreen(enableImmersiveMode_);
        if (auto hostSession = GetHostSession()) {
            hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
        } else {
            TLOGE(WmsLogTag::WMS_LAYOUT, "host session is nullptr, id: %{public}d", GetPersistentId());
        }
    }
}

bool WindowSceneSessionImpl::GetFreeMultiWindowModeEnabledState()
{
    return windowSystemConfig_.freeMultiWindowEnable_ &&
        windowSystemConfig_.freeMultiWindowSupport_;
}

WSError WindowSceneSessionImpl::PcAppInPadNormalClose()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "window session invalid!");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    if (!property_->GetIsPcAppInPad()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "is not pcAppInPad, can not Close");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    Close();
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::NotifyIsFullScreenInForceSplitMode(bool isFullScreen)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "isFullScreen: %{public}u", isFullScreen);
    if (isFullScreenInForceSplit_.load() == isFullScreen) {
        TLOGI(WmsLogTag::WMS_COMPAT, "status has not changed");
        return;
    }
    isFullScreenInForceSplit_.store(isFullScreen);
    const auto& windowRect = GetRect();
    NotifySizeChange(windowRect, WindowSizeChangeReason::FULL_SCREEN_IN_FORCE_SPLIT);
    NotifyGlobalDisplayRectChange(windowRect, WindowSizeChangeReason::FULL_SCREEN_IN_FORCE_SPLIT);

    auto hostSession = GetHostSession();
    if (hostSession) {
        hostSession->NotifyIsFullScreenInForceSplitMode(isFullScreen);
    } else {
        TLOGW(WmsLogTag::WMS_COMPAT, "hostSession is nullptr");
    }
}

WSError WindowSceneSessionImpl::NotifyCompatibleModePropertyChange(const sptr<CompatibleModeProperty> property)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "window session invalid!");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    property_->SetCompatibleModeProperty(property);
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::NotifySessionForeground(uint32_t reason, bool withAnimation)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (!handler_) {
        TLOGE(WmsLogTag::WMS_LIFE, "handler is nullptr");
        return;
    }
    handler_->PostTask([weakThis = wptr(this), reason, withAnimation] {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            return;
        }
        window->Show(reason, withAnimation);
    }, __func__);
}

void WindowSceneSessionImpl::NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (!handler_) {
        TLOGE(WmsLogTag::WMS_LIFE, "handler is nullptr");
        return;
    }
    handler_->PostTask([weakThis = wptr(this), reason, withAnimation, isFromInnerkits] {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            return;
        }
        window->Hide(reason, withAnimation, isFromInnerkits);
    }, __func__);
}

WMError WindowSceneSessionImpl::NotifyPrepareClosePiPWindow()
{
    TLOGI(WmsLogTag::WMS_PIP, "type: %{public}u", GetType());
    if (!WindowHelper::IsPipWindow(GetType())) {
        return WMError::WM_DO_NOTHING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    hostSession->NotifyPiPWindowPrepareClose();
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetWindowLimits(WindowLimits& windowLimits, bool getVirtualPixel)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WindowLimits customizedLimits;
    if (getVirtualPixel) {
        customizedLimits = property_->GetWindowLimitsVP();
    } else {
        customizedLimits = property_->GetWindowLimits();
    }
    windowLimits.minWidth_ = customizedLimits.minWidth_;
    windowLimits.minHeight_ = customizedLimits.minHeight_;
    windowLimits.maxWidth_ = customizedLimits.maxWidth_;
    windowLimits.maxHeight_ = customizedLimits.maxHeight_;
    windowLimits.vpRatio_ = customizedLimits.vpRatio_;
    windowLimits.pixelUnit_ = customizedLimits.pixelUnit_;
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}u, minW:%{public}u, minH:%{public}u, "
        "maxW:%{public}u, maxH:%{public}u, vpRatio:%{public}f, pixelUnit:%{public}u", GetWindowId(),
        windowLimits.minWidth_, windowLimits.minHeight_, windowLimits.maxWidth_, windowLimits.maxHeight_,
        windowLimits.vpRatio_, windowLimits.pixelUnit_);
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::UpdateNewSize()
{
    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING || property_->IsWindowLimitDisabled()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "fullscreen of compatible mode could not update new size, Id: %{public}u",
            GetPersistentId());
        return;
    }
    bool needResize = false;
    Rect windowRect = GetRequestRect();
    if (windowRect.IsUninitializedSize()) {
        windowRect = GetRect();
        if (windowRect.IsUninitializedSize()) {
            TLOGW(WmsLogTag::WMS_LAYOUT, "The sizes of requestRect and rect are uninitialized. winId: %{public}u",
                GetWindowId());
            return;
        }
    }
    if (GetType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return;
    }

    uint32_t width = windowRect.width_;
    uint32_t height = windowRect.height_;
    const auto& newLimits = property_->GetWindowLimits();
    if (width < newLimits.minWidth_) {
        width = newLimits.minWidth_;
        needResize = true;
    }
    if (height < newLimits.minHeight_) {
        height = newLimits.minHeight_;
        needResize = true;
    }
    if (width > newLimits.maxWidth_) {
        width = newLimits.maxWidth_;
        needResize = true;
    }
    if (height > newLimits.maxHeight_) {
        height = newLimits.maxHeight_;
        needResize = true;
    }
    if (needResize) {
        if (IsPcOrPadFreeMultiWindowMode()) {
            isResizedByLimit_ = true;
        }
        Resize(width, height);
        TLOGI(WmsLogTag::WMS_LAYOUT, "Resize window by limits. Id: %{public}u, width: %{public}u,"
            " height: %{public}u", GetWindowId(), width, height);
    }
}

WMError WindowSceneSessionImpl::SetWindowLimits(WindowLimits& windowLimits, bool isForcible)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}u, minW:%{public}u, minH:%{public}u, "
        "maxW:%{public}u, maxH:%{public}u, pixelUnit:%{public}u, isForcible:%{public}u", GetWindowId(),
        windowLimits.minWidth_, windowLimits.minHeight_, windowLimits.maxWidth_, windowLimits.maxHeight_,
        windowLimits.pixelUnit_, isForcible);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    WindowType windowType = GetType();
    bool isDraggableSystemWin = WindowHelper::IsSystemWindow(windowType) && IsWindowDraggable();
    if (!WindowHelper::IsMainWindow(windowType) && !WindowHelper::IsSubWindow(windowType) &&
        windowType != WindowType::WINDOW_TYPE_DIALOG && !isDraggableSystemWin) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "type not support. Id:%{public}u, type:%{public}u",
            GetWindowId(), static_cast<uint32_t>(windowType));
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    WindowLimits customizedLimits;
    if (windowLimits.pixelUnit_ == PixelUnit::VP) {
        customizedLimits = property_->GetWindowLimitsVP();
        forceLimits_ = false;
    } else {
        customizedLimits = property_->GetWindowLimits();
        forceLimits_ = isForcible;
    }
    userLimitsSet_ = true;

    uint32_t minWidth = windowLimits.minWidth_ ? windowLimits.minWidth_ : customizedLimits.minWidth_;
    uint32_t minHeight = windowLimits.minHeight_ ? windowLimits.minHeight_ : customizedLimits.minHeight_;
    uint32_t maxWidth = windowLimits.maxWidth_ ? windowLimits.maxWidth_ : customizedLimits.maxWidth_;
    uint32_t maxHeight = windowLimits.maxHeight_ ? windowLimits.maxHeight_ : customizedLimits.maxHeight_;

    property_->SetUserWindowLimits({
        maxWidth, maxHeight, minWidth, minHeight, customizedLimits.maxRatio_, customizedLimits.minRatio_,
        customizedLimits.vpRatio_, windowLimits.pixelUnit_
    });

    UpdateWindowSizeLimits();
    WMError ret = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "update window proeprty failed! id: %{public}u.", GetWindowId());
        return ret;
    }
    UpdateNewSize();

    FillWindowLimits(windowLimits, windowLimits.pixelUnit_);
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::FillWindowLimits(WindowLimits& windowLimits, PixelUnit pixelUnit)
{
    WindowLimits newLimits;
    if (pixelUnit == PixelUnit::VP) {
        newLimits = property_->GetWindowLimitsVP();
    } else {
        newLimits = property_->GetWindowLimits();
    }
    windowLimits.minWidth_ = newLimits.minWidth_;
    windowLimits.minHeight_ = newLimits.minHeight_;
    windowLimits.maxWidth_ = newLimits.maxWidth_;
    windowLimits.maxHeight_ = newLimits.maxHeight_;
    windowLimits.pixelUnit_ = newLimits.pixelUnit_;
    TLOGI(WmsLogTag::WMS_LAYOUT, "success! Id:%{public}u, minW:%{public}u, minH:%{public}u, "
        "maxW:%{public}u, maxH:%{public}u, pixelUnit:%{public}u", GetWindowId(), windowLimits.minWidth_,
        windowLimits.minHeight_, windowLimits.maxWidth_, windowLimits.maxHeight_, windowLimits.pixelUnit_);
}

WSError WindowSceneSessionImpl::NotifyDialogStateChange(bool isForeground)
{
    const auto type = GetType();
    TLOGI(WmsLogTag::WMS_DIALOG, "state change [name:%{public}s, id:%{public}d, type:%{public}u], state:%{public}u,"
        " requestState:%{public}u, isForeground:%{public}d", property_->GetWindowName().c_str(), GetPersistentId(),
        type, state_, requestState_, static_cast<int32_t>(isForeground));
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "session is invalid, id:%{public}d", GetPersistentId());
        return WSError::WS_ERROR_INVALID_WINDOW;
    }

    if (isForeground) {
        if (state_ == WindowState::STATE_SHOWN) {
            return WSError::WS_OK;
        }
        if (state_ == WindowState::STATE_HIDDEN && requestState_ == WindowState::STATE_SHOWN) {
            state_ = WindowState::STATE_SHOWN;
            NotifyAfterForeground();
        }
    } else {
        if (state_ == WindowState::STATE_HIDDEN) {
            return WSError::WS_OK;
        }
        if (state_ == WindowState::STATE_SHOWN) {
            state_ = WindowState::STATE_HIDDEN;
            NotifyAfterBackground();
        }
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "success [name:%{public}s, id:%{public}d, type:%{public}u],"
        " state:%{public}u, requestState:%{public}u", property_->GetWindowName().c_str(), property_->GetPersistentId(),
        type, state_, requestState_);
    return WSError::WS_OK;
}

WMError WindowSceneSessionImpl::SetDefaultDensityEnabled(bool enabled)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Id=%{public}d, enabled=%{public}d", GetWindowId(), enabled);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "must be app main window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    if (IsStageDefaultDensityEnabled() == enabled) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "defaultDensityEnabledGlobalConfig not change");
        return WMError::WM_OK;
    }

    if (property_->IsAdaptToSimulationScale()) {
        TLOGI(WmsLogTag::WMS_COMPAT, "compat mode not use default density");
        return WMError::WM_OK;
    }

    if (auto hostSession = GetHostSession()) {
        hostSession->OnDefaultDensityEnabled(enabled);
    }

    defaultDensityEnabledStageConfig_.store(enabled);
    SetDefaultDensityEnabledValue(enabled);

    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& winPair : windowSessionMap_) {
        auto window = winPair.second.second;
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is nullptr");
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Id=%{public}u set=%{public}u",
            window->GetWindowId(), window->IsStageDefaultDensityEnabled());
        window->SetDefaultDensityEnabledValue(window->IsStageDefaultDensityEnabled());
        window->UpdateDensity();
    }
    return WMError::WM_OK;
}

bool WindowSceneSessionImpl::GetDefaultDensityEnabled()
{
    return isDefaultDensityEnabled_.load();
}

float WindowSceneSessionImpl::GetVirtualPixelRatio(const sptr<DisplayInfo>& displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "displayInfo is nullptr");
        return INVALID_DEFAULT_DENSITY;
    }
    if (IsDefaultDensityEnabled()) {
        return displayInfo->GetDefaultVirtualPixelRatio();
    }
    if (useUniqueDensity_) {
        return virtualPixelRatio_;
    }
    auto vpr = GetMainWindowCustomDensity();
    return vpr >= MINIMUM_CUSTOM_DENSITY && vpr <= MAXIMUM_CUSTOM_DENSITY ? vpr : displayInfo->GetVirtualPixelRatio();
}

WMError WindowSceneSessionImpl::HideNonSecureWindows(bool shouldHide)
{
    return SingletonContainer::Get<WindowAdapter>().AddOrRemoveSecureSession(property_->GetPersistentId(), shouldHide);
}

WMError WindowSceneSessionImpl::SetTextFieldAvoidInfo(double textFieldPositionY, double textFieldHeight)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "textFieldPositionY: %{public}f, textFieldHeight:%{public}f",
        textFieldPositionY, textFieldHeight);
    property_->SetTextFieldPositionY(textFieldPositionY);
    property_->SetTextFieldHeight(textFieldHeight);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO);
    return WMError::WM_OK;
}

std::unique_ptr<Media::PixelMap> WindowSceneSessionImpl::HandleWindowMask(
    const std::vector<std::vector<uint32_t>>& windowMask)
{
    const Rect& windowRect = GetRequestRect();
    uint32_t maskHeight = windowMask.size();
    if (maskHeight == 0) {
        WLOGFE("WindowMask is invalid");
        return nullptr;
    }
    uint32_t maskWidth = windowMask[0].size();
    if ((windowRect.height_ > 0 && windowRect.height_ != maskHeight) ||
        (windowRect.width_ > 0 && windowRect.width_ != maskWidth)) {
        WLOGFE("WindowMask is invalid");
        return nullptr;
    }
    constexpr uint32_t bgraChannel = 4;
    Media::InitializationOptions opts;
    opts.size.width = static_cast<int32_t>(maskWidth);
    opts.size.height = static_cast<int32_t>(maskHeight);
    uint32_t length = maskWidth * maskHeight * bgraChannel;
    uint8_t* data = static_cast<uint8_t*>(malloc(length));
    if (data == nullptr) {
        WLOGFE("data is nullptr");
        return nullptr;
    }
    constexpr uint32_t fullChannel = 255;
    constexpr uint32_t greenChannel = 1;
    constexpr uint32_t redChannel = 2;
    constexpr uint32_t alphaChannel = 3;
    for (uint32_t i = 0; i < maskHeight; i++) {
        for (uint32_t j = 0; j < maskWidth; j++) {
            uint32_t idx = i * maskWidth + j;
            uint32_t channel = windowMask[i][j] > 0 ? fullChannel : 0;
            uint32_t channelIndex = idx * bgraChannel;
            data[channelIndex] = channel; // blue channel
            data[channelIndex + greenChannel] = channel; // green channel
            data[channelIndex + redChannel] = fullChannel; // red channel
            data[channelIndex + alphaChannel] = channel; // alpha channel
        }
    }
    std::unique_ptr<Media::PixelMap> mask = Media::PixelMap::Create(reinterpret_cast<uint32_t*>(data), length, opts);
    free(data);
    return mask;
}

WMError WindowSceneSessionImpl::SetWindowMask(const std::vector<std::vector<uint32_t>>& windowMask)
{
    TLOGI(WmsLogTag::WMS_PC, "WindowId: %{public}u", GetWindowId());
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    std::shared_ptr<Media::PixelMap> mask = HandleWindowMask(windowMask);
    if (mask == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to create pixelMap of window mask");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    auto rsMask = RSMask::CreatePixelMapMask(mask);
    surfaceNode_->SetCornerRadius(0.0f);
    surfaceNode_->SetShadowRadius(0.0f);
    surfaceNode_->SetAbilityBGAlpha(0);
    surfaceNode_->SetMask(rsMask); // RS interface to set mask
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);

    property_->SetWindowMask(mask);
    property_->SetIsShaped(true);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);
}

WMError WindowSceneSessionImpl::SetFollowParentMultiScreenPolicy(bool enabled)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "This is PcAppInpad, not Suppored");
        return WMError::WM_OK;
    }
    if (!IsPhonePadOrPcWindow()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "called by invalid window type, type:%{public}d", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    hostSession->NotifyFollowParentMultiScreenPolicy(enabled);
    property_->EditSessionInfo().isFollowParentMultiScreenPolicy = enabled;
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::UseImplicitAnimation(bool useImplicit)
{
    TLOGI(WmsLogTag::WMS_PC, "WindowId: %{public}u", GetWindowId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return static_cast<WMError>(hostSession->UseImplicitAnimation(useImplicit));
}

WSError WindowSceneSessionImpl::UpdateBrightness(float brightness)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Wid: %{public}d, brightness: %{public}f", GetWindowId(), brightness);
    property_->SetBrightness(brightness);
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::UpdateDensity()
{
    UpdateDensityInner(nullptr);
}

void WindowSceneSessionImpl::UpdateDensityInner(const sptr<DisplayInfo>& info)
{
    if (property_->GetUserWindowLimits().pixelUnit_ == PixelUnit::VP) {
        UpdateWindowSizeLimits();
        UpdateNewSize();
    } else {
        WindowLimits limitsPx = property_->GetWindowLimits();
        WindowLimits limitsVp = WindowLimits::DEFAULT_VP_LIMITS();
        float vpr = 1.0f;
        WMError ret = WindowSessionImpl::GetVirtualPixelRatio(vpr);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::DEFAULT, "Id:%{public}d, get vpr failed", GetPersistentId());
            return;
        }
        RecalculateVpLimitsByPx(limitsPx, limitsVp, vpr);
        limitsPx.vpRatio_ = vpr;
        limitsVp.vpRatio_ = vpr;
        property_->SetWindowLimits(limitsPx);
        property_->SetWindowLimitsVP(limitsVp);
        property_->SetLastLimitsVpr(vpr);
    }
    WMError ret = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "update window property failed! id: %{public}u.", GetWindowId());
        return;
    }

    NotifyDisplayInfoChange(info);

    auto preRect = GetRect();
    UpdateViewportConfig(preRect, WindowSizeChangeReason::UNDEFINED, nullptr, info);
    TLOGD(WmsLogTag::DEFAULT, "[%{public}d, %{public}d, %{public}u, %{public}u]",
        preRect.posX_, preRect.posY_, preRect.width_, preRect.height_);
}

WMError WindowSceneSessionImpl::RegisterKeyboardPanelInfoChangeListener(
    const sptr<IKeyboardPanelInfoChangeListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(keyboardPanelInfoChangeListenerMutex_);
    if (keyboardPanelInfoChangeListeners_ == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "id: %{public}d",
            GetPersistentId());
        keyboardPanelInfoChangeListeners_ = listener;
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener already registered");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }

    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::RegisterWindowAttachStateChangeListener(
    const sptr<IWindowAttachStateChangeListner>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "id: %{public}d, listener is null", GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lockListener(windowAttachStateChangeListenerMutex_);
        windowAttachStateChangeListener_ = listener;
    }
    TLOGD(WmsLogTag::WMS_SUB, "id: %{public}d listener registered", GetPersistentId());
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->NotifyWindowAttachStateListenerRegistered(true);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::UnregisterWindowAttachStateChangeListener()
{
    {
        std::lock_guard<std::mutex> lockListener(windowAttachStateChangeListenerMutex_);
        windowAttachStateChangeListener_ = nullptr;
    }
    TLOGD(WmsLogTag::WMS_SUB, "id: %{public}d listener unregistered", GetPersistentId());
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    hostSession->NotifyWindowAttachStateListenerRegistered(false);
    return WMError::WM_OK;
}

WSError WindowSceneSessionImpl::NotifyWindowAttachStateChange(bool isAttach)
{
    TLOGI(WmsLogTag::WMS_SUB, "id: %{public}d", GetPersistentId());
    if (lifecycleCallback_) {
        TLOGI(WmsLogTag::WMS_SUB, "notifyAttachState id: %{public}d", GetPersistentId());
        lifecycleCallback_->OnNotifyAttachState(isAttach);
    }
    std::lock_guard<std::mutex> lockListener(windowAttachStateChangeListenerMutex_);
    if (!windowAttachStateChangeListener_) {
        TLOGW(WmsLogTag::WMS_SUB, "listener is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (isAttach) {
        windowAttachStateChangeListener_->AfterAttached();
    } else {
        windowAttachStateChangeListener_->AfterDetached();
    }
    return WSError::WS_OK;
}

WMError WindowSceneSessionImpl::UnregisterKeyboardPanelInfoChangeListener(
    const sptr<IKeyboardPanelInfoChangeListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(keyboardPanelInfoChangeListenerMutex_);
    keyboardPanelInfoChangeListeners_ = nullptr;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "id: %{public}d", GetPersistentId());

    return WMError::WM_OK;
}

void WindowSceneSessionImpl::NotifyKeyboardPanelInfoChange(const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "isShown: %{public}d, gravity: %{public}d"
        ", rect_: [%{public}d, %{public}d, %{public}d, %{public}d]", keyboardPanelInfo.isShowing_,
        keyboardPanelInfo.gravity_, keyboardPanelInfo.rect_.posX_, keyboardPanelInfo.rect_.posY_,
        keyboardPanelInfo.rect_.width_, keyboardPanelInfo.rect_.height_);
    std::lock_guard<std::mutex> lockListener(keyboardPanelInfoChangeListenerMutex_);
    if (keyboardPanelInfoChangeListeners_ && keyboardPanelInfoChangeListeners_.GetRefPtr()) {
        keyboardPanelInfoChangeListeners_.GetRefPtr()->OnKeyboardPanelInfoChanged(keyboardPanelInfo);
    } else {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "listener is unRegistered");
    }
}

WSError WindowSceneSessionImpl::UpdateDisplayId(DisplayId displayId)
{
    bool displayIdChanged = property_->GetDisplayId() != displayId;
    property_->SetDisplayId(displayId);
    NotifyDisplayInfoChange();
    if (displayIdChanged) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "wid: %{public}d, displayId: %{public}" PRIu64, GetPersistentId(), displayId);
        NotifyDisplayIdChange(displayId);
        NotifyDmsDisplayMove(displayId);
    }
    return WSError::WS_OK;
}

WSError WindowSceneSessionImpl::UpdateOrientation()
{
    TLOGD(WmsLogTag::DMS, "id: %{public}d", GetPersistentId());
    NotifyDisplayInfoChange();
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::NotifyDisplayInfoChange(const sptr<DisplayInfo>& info)
{
    TLOGD(WmsLogTag::DMS, "id: %{public}d", GetPersistentId());
    sptr<DisplayInfo> displayInfo = nullptr;
    DisplayId displayId = 0;
    if (info == nullptr) {
        displayId = property_->GetDisplayId();
        auto display = SingletonContainer::IsDestroyed() ? nullptr :
            SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId);
        if (display == nullptr) {
            TLOGE(WmsLogTag::DMS, "get display by displayId %{public}" PRIu64 " failed.", displayId);
            return;
        }
        displayInfo = display->GetDisplayInfo();
    } else {
        displayInfo = info;
    }
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "get display info %{public}" PRIu64 " failed.", displayId);
        return;
    }
    if (IsSystemDensityChanged(displayInfo)) {
        lastSystemDensity_ = displayInfo->GetVirtualPixelRatio();
        NotifySystemDensityChange(displayInfo->GetVirtualPixelRatio());
    }
    float density = GetVirtualPixelRatio(displayInfo);
    DisplayOrientation orientation = displayInfo->GetDisplayOrientation();

    // skip scb process
    auto context = GetContext();
    if (context == nullptr || context->GetBundleName() == AppExecFwk::Constants::SCENE_BOARD_BUNDLE_NAME) {
        TLOGE(WmsLogTag::DMS, "id:%{public}d failed, context is null.", GetPersistentId());
        return;
    }
    auto token = context->GetToken();
    if (token == nullptr) {
        TLOGE(WmsLogTag::DMS, "get token window:%{public}d failed.", GetPersistentId());
        return;
    }
    SingletonContainer::Get<WindowManager>().NotifyDisplayInfoChange(token, displayId, density, orientation);
}

bool WindowSceneSessionImpl::IsLandscape(uint64_t displayId)
{
    int32_t displayWidth = 0;
    int32_t displayHeight = 0;
    std::string dispName = "UNKNOWN";
    displayId = (displayId == DISPLAY_ID_INVALID) ? property_->GetDisplayId() : displayId;
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId);
    if (display != nullptr) {
        displayWidth = display->GetWidth();
        displayHeight = display->GetHeight();
        dispName = display->GetName();
    } else {
        auto defaultDisplayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
        if (defaultDisplayInfo != nullptr) {
            displayWidth = defaultDisplayInfo->GetWidth();
            displayHeight = defaultDisplayInfo->GetHeight();
        }
    }
    bool isLandscape = displayWidth > displayHeight;
    if (displayWidth == displayHeight && display != nullptr) {
        sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "displayInfo is nullptr");
            return false;
        }
        DisplayOrientation orientation = displayInfo->GetDisplayOrientation();
        if (orientation == DisplayOrientation::UNKNOWN) {
            TLOGW(WmsLogTag::WMS_KEYBOARD, "Display orientation is UNKNOWN");
        }
        isLandscape = (orientation == DisplayOrientation::LANDSCAPE ||
            orientation == DisplayOrientation::LANDSCAPE_INVERTED);
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "c-displayInfo: %{public}" PRIu64 ", %{public}d|%{public}d|%{public}d, %{public}s",
        displayId, displayWidth, displayHeight, isLandscape, dispName.c_str());
    return isLandscape;
}

WMError WindowSceneSessionImpl::MoveAndResizeKeyboard(const KeyboardLayoutParams& params)
{
    bool isLandscape = IsLandscape(params.displayId_);
    Rect newRect = isLandscape ? params.LandscapeKeyboardRect_ : params.PortraitKeyboardRect_;
    property_->SetRequestRect(newRect);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Id: %{public}d, newRect: %{public}s, isLandscape: %{public}d",
        GetPersistentId(), newRect.ToString().c_str(), isLandscape);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::AdjustKeyboardLayout(const KeyboardLayoutParams params)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "g: %{public}u, "
        "LAvoid: %{public}d, PAvoid: %{public}d, "
        "LRect: %{public}s, PRect: %{public}s, "
        "LPRect: %{public}s, PPRect: %{public}s, dispId: %{public}" PRIu64,
        static_cast<uint32_t>(params.gravity_), params.landscapeAvoidHeight_, params.portraitAvoidHeight_,
        params.LandscapeKeyboardRect_.ToString().c_str(), params.PortraitKeyboardRect_.ToString().c_str(),
        params.LandscapePanelRect_.ToString().c_str(), params.PortraitPanelRect_.ToString().c_str(), params.displayId_);
    property_->SetKeyboardLayoutParams(params);
    property_->AddKeyboardLayoutParams(params.displayId_, params);
    auto ret = MoveAndResizeKeyboard(params);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard move and resize failed");
        return ret;
    }
    if (auto hostSession = GetHostSession()) {
        return static_cast<WMError>(hostSession->AdjustKeyboardLayout(params));
    }
    return WMError::WM_OK;
}

void WindowSceneSessionImpl::WindowScreenListener::OnDisconnect(ScreenId screenId)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& [_, windPair] : windowSessionMap_) {
        if (windPair.second->GetType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            auto& window = windPair.second;
            window->GetProperty()->ClearCachedKeyboardParamsOnScreenDisconnected(screenId);
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Clear cached keyboardParams: %{public}d, dispId: %{public}" PRIu64,
                window->GetProperty()->GetPersistentId(), screenId);
        }
    }
}

WMError WindowSceneSessionImpl::SetImmersiveModeEnabledState(bool enable)
{
    TLOGD(WmsLogTag::WMS_IMMS, "id: %{public}u, enable: %{public}u", GetWindowId(), enable);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
        WindowMode::WINDOW_MODE_FULLSCREEN)) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const WindowType curWindowType = GetType();
    if (!WindowHelper::IsMainWindow(curWindowType) && !WindowHelper::IsSubWindow(curWindowType)) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    enableImmersiveMode_ = enable;
    hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
    WindowMode mode = GetWindowMode();
    if (!windowSystemConfig_.IsPcWindow() || mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
        return SetLayoutFullScreen(enableImmersiveMode_);
    }
    return WMError::WM_OK;
}

bool WindowSceneSessionImpl::GetImmersiveModeEnabledState() const
{
    TLOGD(WmsLogTag::WMS_IMMS, "id: %{public}u, enableImmersiveMode=%{public}u",
        GetWindowId(), enableImmersiveMode_.load());
    if (IsWindowSessionInvalid()) {
        return false;
    }
    return enableImmersiveMode_;
}

WMError WindowSceneSessionImpl::IsImmersiveLayout(bool& isImmersiveLayout) const
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (IsPcOrPadFreeMultiWindowMode()) {
        auto uiContent = GetUIContentSharedPtr();
        if (uiContent == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "uicontent is null");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        Rect drawableRect;
        uiContent->GetWindowPaintSize(drawableRect);
        isImmersiveLayout = drawableRect == GetRect();
    } else {
        isImmersiveLayout = isIgnoreSafeArea_;
    }
    return WMError::WM_OK;
}

template <typename K, typename V>
static V GetValueByKey(const std::unordered_map<K, V>& map, K key)
{
    auto it = map.find(key);
    return it != map.end() ? it->second : V{};
}

void WindowSceneSessionImpl::HandleEventForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem)
{
    int32_t action = pointerEvent->GetPointerAction();
    switch (action) {
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
            HandleDownForCompatibleMode(pointerEvent, pointerItem);
            break;
        case MMI::PointerEvent::POINTER_ACTION_MOVE:
            HandleMoveForCompatibleMode(pointerEvent, pointerItem);
            break;
        case MMI::PointerEvent::POINTER_ACTION_UP:
            HandleUpForCompatibleMode(pointerEvent, pointerItem);
            break;
        default:
            break;
    }
}

void WindowSceneSessionImpl::HandleDownForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem)
{
    int32_t displayX = pointerItem.GetDisplayX();
    int32_t displayY = pointerItem.GetDisplayY();
    int32_t displayId = property_->GetDisplayId();
    int32_t pointerCount = pointerEvent->GetPointerCount();
    if (pointerCount == 1) {
        eventMapTriggerByDisplay_[displayId] = std::vector<bool>(MAX_POINTERS);
        eventMapDeltaXByDisplay_[displayId] = std::vector<int32_t>(MAX_POINTERS);
        downPointerByDisplay_[displayId] = std::vector<PointInfo>(MAX_POINTERS);
        isOverTouchSlop_ = false;
        isDown_ = true;
    }

    if (IsInMappingRegionForCompatibleMode(displayX, displayY)) {
        int32_t pointerId = pointerEvent->GetPointerId();
        if (pointerId >= GetValueByKey(eventMapTriggerByDisplay_, displayId).size() ||
            pointerId >= GetValueByKey(eventMapDeltaXByDisplay_, displayId).size() ||
            pointerId >= GetValueByKey(downPointerByDisplay_, displayId).size()) {
            TLOGE(WmsLogTag::DEFAULT, "pointerId: %{public}d out of range", pointerId);
            return;
        }
        eventMapTriggerByDisplay_[displayId][pointerId] = true;
        downPointerByDisplay_[displayId][pointerId] = {displayX, displayY};
        const auto& windowRect = GetRect();
        float xMappingScale = 1.0f;
        if (windowRect.posX_ != 0) {
            xMappingScale = static_cast<float>(windowRect.width_) / windowRect.posX_;
        }
        int32_t windowLeft = windowRect.posX_;
        int32_t windowRight = windowRect.posX_ + windowRect.width_;
        int32_t transferX;
        if (displayX <= windowLeft) {
            transferX = windowRight - xMappingScale * (windowLeft - displayX);
        } else {
            transferX = windowLeft + xMappingScale * (displayX - windowRight);
        }
        if (transferX < 0) {
            transferX = 0;
        }
        TLOGD(WmsLogTag::DEFAULT, "DOWN in mapping region, displayX: %{private}d, transferX: %{public}d, "
            "pointerId: %{public}d", displayX, transferX, pointerId);
        eventMapDeltaXByDisplay_[displayId][pointerId] = transferX - displayX;
        ConvertPointForCompatibleMode(pointerEvent, pointerItem, transferX);
    }
}

void WindowSceneSessionImpl::HandleMoveForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem)
{
    if (!isDown_) {
        TLOGW(WmsLogTag::WMS_COMPAT, "receive move before down, skip");
        return;
    }
    int32_t displayId = property_->GetDisplayId();
    int32_t pointerId = pointerEvent->GetPointerId();
    if (pointerId >= GetValueByKey(eventMapTriggerByDisplay_, displayId).size() ||
        pointerId >= GetValueByKey(eventMapDeltaXByDisplay_, displayId).size() ||
        !GetValueByKey(eventMapTriggerByDisplay_, displayId)[pointerId]) {
        return;
    }

    int32_t displayX = pointerItem.GetDisplayX();
    int32_t displayY = pointerItem.GetDisplayY();
    const auto& windowRect = GetRect();
    if (!isOverTouchSlop_ && CheckTouchSlop(pointerId, displayX, displayY, windowRect.width_ / TOUCH_SLOP_RATIO)) {
        TLOGD(WmsLogTag::WMS_COMPAT, "reach touch slop, threshold: %{public}d", windowRect.width_ / TOUCH_SLOP_RATIO);
        isOverTouchSlop_ = true;
    }
    int32_t transferX = displayX + GetValueByKey(eventMapDeltaXByDisplay_, displayId)[pointerId];
    TLOGD(WmsLogTag::WMS_COMPAT, "MOVE, displayX: %{private}d, transferX: %{public}d, pointerId: %{public}d",
        displayX, transferX, pointerId);
    ConvertPointForCompatibleMode(pointerEvent, pointerItem, transferX);
}

void WindowSceneSessionImpl::HandleUpForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem)
{
    if (!isDown_) {
        TLOGW(WmsLogTag::WMS_COMPAT, "receive up before down, skip");
        return;
    }
    int32_t displayId = property_->GetDisplayId();
    int32_t pointerId = pointerEvent->GetPointerId();
    if (pointerId >= GetValueByKey(eventMapTriggerByDisplay_, displayId).size() ||
        pointerId >= GetValueByKey(eventMapDeltaXByDisplay_, displayId).size()) {
        return;
    }
    if (GetValueByKey(eventMapTriggerByDisplay_, displayId)[pointerId]) {
        int32_t displayX = pointerItem.GetDisplayX();
        int32_t transferX = displayX + GetValueByKey(eventMapDeltaXByDisplay_, displayId)[pointerId];
        ConvertPointForCompatibleMode(pointerEvent, pointerItem, transferX);
        TLOGD(WmsLogTag::WMS_COMPAT, "UP, displayX: %{private}d, transferX: %{public}d, pointerId: %{public}d",
            displayX, transferX, pointerId);
        GetValueByKey(eventMapDeltaXByDisplay_, displayId)[pointerId] = 0;
        GetValueByKey(eventMapTriggerByDisplay_, displayId)[pointerId] = false;
        IgnoreClickEvent(pointerEvent);
    }
    int32_t pointerCount = pointerEvent->GetPointerCount();
    if (pointerCount == 1) {
        eventMapDeltaXByDisplay_.erase(displayId);
        eventMapTriggerByDisplay_.erase(displayId);
        downPointerByDisplay_.erase(displayId);
        isDown_ = false;
    }
}

void WindowSceneSessionImpl::ConvertPointForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem, int32_t transferX)
{
    const auto& windowRect = GetRect();
    int32_t pointerId = pointerEvent->GetPointerId();

    pointerItem.SetDisplayX(transferX);
    pointerItem.SetDisplayXPos(static_cast<double>(transferX));
    pointerItem.SetWindowX(transferX - windowRect.posX_);
    pointerItem.SetWindowXPos(static_cast<double>(transferX - windowRect.posX_));
    pointerEvent->UpdatePointerItem(pointerId, pointerItem);
}

bool WindowSceneSessionImpl::IsInMappingRegionForCompatibleMode(int32_t displayX, int32_t displayY)
{
    Rect pointerRect = { displayX, displayY, 0, 0 };
    Rect globalRect {};
    if (WMError::WM_OK == GetGlobalScaledRect(globalRect)) {
        return !pointerRect.IsInsideOf(globalRect);
    }
    const auto& windowRect = GetRect();
    return !pointerRect.IsInsideOf(windowRect);
}

bool WindowSceneSessionImpl::CheckTouchSlop(int32_t pointerId, int32_t displayX, int32_t displayY, int32_t threshold)
{
    int32_t displayId = property_->GetDisplayId();
    if (downPointerByDisplay_.find(displayId) == downPointerByDisplay_.end()) {
        return false;
    }
    std::vector<PointInfo> downPointers = downPointerByDisplay_[displayId];
    return pointerId < downPointers.size() &&
        (std::abs(displayX - downPointers[pointerId].x) >= threshold ||
        std::abs(displayY - downPointers[pointerId].y) >= threshold);
}

void WindowSceneSessionImpl::IgnoreClickEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_UP) {
        return;
    }
    if (isOverTouchSlop_) {
        if (pointerEvent->GetPointerCount() == 1) {
            isOverTouchSlop_ = false;
        }
    } else {
        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
        TLOGI(WmsLogTag::DEFAULT, "transfer UP to CANCEL for not over touch slop");
    }
}

WMError WindowSceneSessionImpl::GetWindowStatus(WindowStatus& windowStatus)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PC, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    windowStatus = GetWindowStatusInner(GetWindowMode());
    TLOGD(WmsLogTag::WMS_PC, "Id:%{public}u, WindowStatus:%{public}u", GetWindowId(), windowStatus);
    return WMError::WM_OK;
}

bool WindowSceneSessionImpl::GetIsUIExtFirstSubWindow() const
{
    return property_->GetIsUIExtFirstSubWindow();
}

bool WindowSceneSessionImpl::GetIsUIExtAnySubWindow() const
{
    return property_->GetIsUIExtAnySubWindow();
}

WMError WindowSceneSessionImpl::SetGestureBackEnabled(bool enable)
{
    if (windowSystemConfig_.IsPcWindow()) {
        if (property_->IsAdaptToCompatibleDevice()) {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "phone app in pc compatible mode, enable=%{public}d", enable);
            return WMError::WM_OK;
        }
        TLOGI(WmsLogTag::WMS_IMMS, "device is not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u, enable %{public}u", GetWindowId(), enable);
    gestureBackEnabled_ = enable;
    AAFwk::Want want;
    want.SetParam(Extension::GESTURE_BACK_ENABLED, enable);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_GESTURE_BACK_ENABLED),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    return hostSession->SetGestureBackEnabled(enable);
}

WMError WindowSceneSessionImpl::GetGestureBackEnabled(bool& enable) const
{
    if (windowSystemConfig_.IsPcWindow()) {
        enable = false;
        if (property_->IsAdaptToCompatibleDevice()) {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "phone app in pc compatible mode, enable=%{public}d", enable);
            return WMError::WM_OK;
        }
        TLOGI(WmsLogTag::WMS_IMMS, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    enable = gestureBackEnabled_;
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u enable %{public}u", GetWindowId(), enable);
    return WMError::WM_OK;
}

WSError WindowSceneSessionImpl::SetFullScreenWaterfallMode(bool isWaterfallMode)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "prev: %{public}d, curr: %{public}d, winId: %{public}u",
        isFullScreenWaterfallMode_.load(), isWaterfallMode, GetWindowId());
    NotifyAcrossDisplaysChange(isWaterfallMode);
    if (isValidWaterfallMode_.load() && isFullScreenWaterfallMode_.load() == isWaterfallMode) {
        return WSError::WS_DO_NOTHING;
    }
    isFullScreenWaterfallMode_.store(isWaterfallMode);
    if (isWaterfallMode) {
        lastWindowModeBeforeWaterfall_.store(property_->GetWindowMode());
    } else {
        lastWindowModeBeforeWaterfall_.store(WindowMode::WINDOW_MODE_UNDEFINED);
    }
    isValidWaterfallMode_.store(true);
    NotifyWaterfallModeChange(isWaterfallMode);
    return WSError::WS_OK;
}

WSError WindowSceneSessionImpl::SetSupportEnterWaterfallMode(bool isSupportEnter)
{
    handler_->PostTask([weakThis = wptr(this), isSupportEnter, where = __func__] {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is null", where);
            return;
        }
        if (window->supportEnterWaterfallMode_ == isSupportEnter) {
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s prev: %{public}d, curr: %{public}d",
            where, window->supportEnterWaterfallMode_, isSupportEnter);
        window->supportEnterWaterfallMode_ = isSupportEnter;
        std::shared_ptr<Ace::UIContent> uiContent = window->GetUIContentSharedPtr();
        if (uiContent == nullptr || !window->IsDecorEnable()) {
            TLOGND(WmsLogTag::WMS_LAYOUT_PC, "%{public}s uiContent unavailable", where);
            return;
        }
        uiContent->OnContainerModalEvent(WINDOW_WATERFALL_VISIBILITY_EVENT, isSupportEnter ? "true" : "false");
    }, __func__);
    return WSError::WS_OK;
}

WMError WindowSceneSessionImpl::OnContainerModalEvent(const std::string& eventName, const std::string& value)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, name: %{public}s, value: %{public}s",
        GetPersistentId(), eventName.c_str(), value.c_str());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    hostSession->OnContainerModalEvent(eventName, value);
    if (eventName == WINDOW_WATERFALL_EVENT) {
        bool lastFullScreenWaterfallMode = isFullScreenWaterfallMode_.load();
        SetFullScreenWaterfallMode(true);
        auto ret = Maximize();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "maximize failed");
            SetFullScreenWaterfallMode(lastFullScreenWaterfallMode);
        }
        return ret;
    } else if (eventName == BACK_WINDOW_EVENT) {
        HandleBackEvent();
        return WMError::WM_OK;
    } else if (eventName == COMPATIBLE_MAX_WINDOW_EVENT) {
        MaximizeForCompatibleMode();
        return WMError::WM_OK;
    } else if (eventName == COMPATIBLE_RECOVER_WINDOW_EVENT) {
        RecoverForCompatibleMode();
        return WMError::WM_OK;
    }  else if (eventName == NAME_LANDSCAPE_18_9_CLICK) {
        SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_18_9);
        return WMError::WM_OK;
    } else if (eventName == NAME_LANDSCAPE_1_1_CLICK) {
        SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_1_1);
        return WMError::WM_OK;
    } else if (eventName == NAME_LANDSCAPE_2_3_CLICK) {
        SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_2_3);
        return WMError::WM_OK;
    } else if (eventName == NAME_DEFAULT_LANDSCAPE_CLICK) {
        SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_DEFAULT);
        return WMError::WM_OK;
    } else if (eventName == NAME_LANDSCAPE_SPLIT_CLICK) {
        SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_SPLIT);
        return WMError::WM_OK;
    } else if (eventName == EVENT_NAME_HOVER) {
        std::string bundleName = property_->GetSessionInfo().bundleName_;
        ReportHoverMaximizeMenu(bundleName, value);
        return WMError::WM_OK;
    }
    return WMError::WM_DO_NOTHING;
}

void WindowSceneSessionImpl::ReportHoverMaximizeMenu(const std::string& bundleName, const std::string& hoverType)
{
    HiSysEventWrite(SCENE_BOARD_UE_DOMAIN, HOVER_MAXIMIZE_MENU,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "BUNDLENAME", bundleName,
        "HOVERTYPE", hoverType);
}

bool WindowSceneSessionImpl::IsSystemDensityChanged(const sptr<DisplayInfo>& displayInfo)
{
    if (MathHelper::NearZero(lastSystemDensity_ - displayInfo->GetVirtualPixelRatio())) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "System density not change");
        return false;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "windowId: %{public}d, lastDensity: %{public}f, currDensity: %{public}f",
        GetPersistentId(), lastSystemDensity_, displayInfo->GetVirtualPixelRatio());
    return true;
}

bool WindowSceneSessionImpl::IsDefaultDensityEnabled()
{
    if (WindowHelper::IsMainWindow(GetType())) {
        return GetDefaultDensityEnabled();
    }
    if (isEnableDefaultDensityWhenCreate_) {
        return true;
    }
    if (auto mainWindow = FindMainWindowWithContext()) {
        CopyUniqueDensityParameter(mainWindow);
    }
    return GetDefaultDensityEnabled();
}

float WindowSceneSessionImpl::GetMainWindowCustomDensity()
{
    if (WindowHelper::IsMainWindow(GetType())) {
        return GetCustomDensity();
    }
    auto mainWindow = FindMainWindowWithContext();
    return mainWindow ? mainWindow->GetCustomDensity() : UNDEFINED_DENSITY;
}

float WindowSceneSessionImpl::GetCustomDensity() const
{
    return customDensity_;
}

WMError WindowSceneSessionImpl::SetWindowAnchorInfo(const WindowAnchorInfo& windowAnchorInfo)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const auto& property = GetProperty();
    if (!WindowHelper::IsSubWindow(property->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SUB, "only sub window is valid");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (WindowHelper::IsFullScreenWindow(property->GetWindowMode())) {
        TLOGE(WmsLogTag::WMS_SUB, "not support fullscreen sub window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (property->GetSubWindowLevel() > 1) {
        TLOGI(WmsLogTag::WMS_SUB, "not support more than 1 level window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!windowSystemConfig_.supportFollowRelativePositionToParent_) {
        TLOGI(WmsLogTag::WMS_SUB, "not support device");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGI(WmsLogTag::WMS_SUB, "session is nullptr");
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    WSError ret = hostSession->SetWindowAnchorInfo(windowAnchorInfo);
    if (ret == WSError::WS_ERROR_DEVICE_NOT_SUPPORT) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (ret == WSError::WS_OK) {
        property->SetWindowAnchorInfo(windowAnchorInfo);
    }
    return ret != WSError::WS_OK ? WMError::WM_ERROR_SYSTEM_ABNORMALLY : WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetFollowParentWindowLayoutEnabled(bool isFollow)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, window session is invalid", GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const auto& property = GetProperty();
    if (!WindowHelper::IsSubWindow(property->GetWindowType()) &&
        !WindowHelper::IsDialogWindow(property->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SUB, "only sub window and dialog is valid");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    if (property->GetSubWindowLevel() > 1) {
        TLOGI(WmsLogTag::WMS_SUB, "not support more than 1 level window");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    if (!windowSystemConfig_.supportFollowParentWindowLayout_) {
        TLOGI(WmsLogTag::WMS_SUB, "not support device");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!GetHostSession()) {
        TLOGI(WmsLogTag::WMS_SUB, "session is nullptr");
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    WSError ret = GetHostSession()->SetFollowParentWindowLayoutEnabled(isFollow);
    TLOGI(WmsLogTag::WMS_SUB, "id:%{public}d, isFollow:%{public}d", GetPersistentId(), isFollow);
    if (ret == WSError::WS_ERROR_DEVICE_NOT_SUPPORT) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, device not support", GetWindowId());
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return ret != WSError::WS_OK ? WMError::WM_ERROR_SYSTEM_ABNORMALLY : WMError::WM_OK;
}

WMError WindowSceneSessionImpl::IsTransitionAnimationSupported() const
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!(IsPcOrPadFreeMultiWindowMode() || property_->GetIsPcAppInPad())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Device is invalid");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Window type is invalid");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetWindowTransitionAnimation(WindowTransitionType transitionType,
    const TransitionAnimation& animation)
{
    WMError errorCode = IsTransitionAnimationSupported();
    if (errorCode != WMError::WM_OK) {
        return errorCode;
    }
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGI(WmsLogTag::WMS_ANIMATION, "session is nullptr");
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    WSError ret = hostSession->SetWindowTransitionAnimation(transitionType, animation);
    if (ret == WSError::WS_OK) {
        std::lock_guard<std::mutex> lockListener(transitionAnimationConfigMutex_);
        property_->SetTransitionAnimationConfig(transitionType, animation);
    }
    return ret != WSError::WS_OK ? WMError::WM_ERROR_SYSTEM_ABNORMALLY : WMError::WM_OK;
}

std::shared_ptr<TransitionAnimation> WindowSceneSessionImpl::GetWindowTransitionAnimation(WindowTransitionType
    transitionType)
{
    if (IsTransitionAnimationSupported() != WMError::WM_OK) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lockListener(transitionAnimationConfigMutex_);
    auto transitionAnimationConfig = property_->GetTransitionAnimationConfig();
    if (transitionAnimationConfig.find(transitionType) != transitionAnimationConfig.end()) {
        return transitionAnimationConfig[transitionType];
    } else {
        return nullptr;
    }
}

WMError WindowSceneSessionImpl::SetCustomDensity(float density, bool applyToSubWindow)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u, density=%{public}f", GetWindowId(), density);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if ((density < MINIMUM_CUSTOM_DENSITY && !MathHelper::NearZero(density - UNDEFINED_DENSITY)) ||
        density > MAXIMUM_CUSTOM_DENSITY) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u set invalid density=%{public}f", GetWindowId(), density);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u, must be app main window", GetWindowId());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (MathHelper::NearZero(customDensity_ - density) && !applyToSubWindow) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u set density not change", GetWindowId());
        return WMError::WM_OK;
    }
    defaultDensityEnabledStageConfig_.store(false);
    SetDefaultDensityEnabledValue(false);
    customDensity_ = density;
    UpdateDensity();
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& winPair : windowSessionMap_) {
        auto window = winPair.second.second;
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is nullptr");
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Id=%{public}d UpdateDensity", window->GetWindowId());
        window->SetDefaultDensityEnabledValue(false);
        if (applyToSubWindow) {
            window->UpdateDensity();
        }
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetWindowDensityInfo(WindowDensityInfo& densityInfo)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "display is null, winId=%{public}u", GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "displayInfo is null, winId=%{public}u", GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }
    densityInfo.systemDensity = displayInfo->GetVirtualPixelRatio();
    densityInfo.defaultDensity = displayInfo->GetDefaultVirtualPixelRatio();
    auto customDensity = UNDEFINED_DENSITY;
    if (IsDefaultDensityEnabled()) {
        customDensity = displayInfo->GetDefaultVirtualPixelRatio();
    } else {
        customDensity = GetCustomDensity();
        customDensity = MathHelper::NearZero(customDensity - UNDEFINED_DENSITY) ? displayInfo->GetVirtualPixelRatio()
                                                                                : customDensity;
    }
    densityInfo.customDensity = customDensity;
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::IsMainWindowFullScreenAcrossDisplays(bool& isAcrossDisplays)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    return hostSession->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
}

bool WindowSceneSessionImpl::IsFullScreenEnable() const
{
    if (!WindowHelper::IsWindowModeSupported(property_->GetWindowModeSupportType(),
        WindowMode::WINDOW_MODE_FULLSCREEN)) {
        return false;
    }
    const auto& sizeLimits = property_->GetWindowLimits();
    uint32_t displayWidth = 0;
    uint32_t displayHeight = 0;
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "display is null, winId=%{public}u", GetWindowId());
        return false;
    }
    displayWidth = static_cast<uint32_t>(display->GetWidth());
    displayHeight = static_cast<uint32_t>(display->GetHeight());
    if (property_->GetDragEnabled() && (sizeLimits.maxWidth_ < displayWidth || sizeLimits.maxHeight_ < displayHeight)) {
        return false;
    }
    return true;
}

WMError WindowSceneSessionImpl::GetWindowPropertyInfo(WindowPropertyInfo& windowPropertyInfo)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    windowPropertyInfo.windowRect = GetRect();
    auto uicontent = GetUIContentSharedPtr();
    if (uicontent == nullptr) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "uicontent is nullptr");
    } else {
        uicontent->GetWindowPaintSize(windowPropertyInfo.drawableRect);
    }
    windowPropertyInfo.globalDisplayRect = property_->GetGlobalDisplayRect();
    windowPropertyInfo.type = GetType();
    windowPropertyInfo.isLayoutFullScreen = IsLayoutFullScreen();
    windowPropertyInfo.isFullScreen = IsFullScreen();
    windowPropertyInfo.isTouchable = GetTouchable();
    windowPropertyInfo.isFocusable = GetFocusable();
    windowPropertyInfo.name = GetWindowName();
    windowPropertyInfo.isPrivacyMode = IsPrivacyMode();
    windowPropertyInfo.isKeepScreenOn = IsKeepScreenOn();
    windowPropertyInfo.brightness = GetBrightness();
    windowPropertyInfo.isTransparent = IsTransparent();
    windowPropertyInfo.id = GetWindowId();
    windowPropertyInfo.displayId = GetDisplayId();
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u, globalDisplayRect=%{public}s", GetWindowId(),
        windowPropertyInfo.globalDisplayRect.ToString().c_str());
    HookWindowSizeByHookWindowInfo(windowPropertyInfo.windowRect);
    HookWindowSizeByHookWindowInfo(windowPropertyInfo.globalDisplayRect);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetWindowStateSnapshot(std::string& winStateSnapshotJsonStr)
{
    auto persistentId = GetPersistentId();
    nlohmann::json winStateSnapshotJson = {
        {"isPcMode", system::GetBoolParameter("persist.sceneboard.ispcmode", false)},
    };
    winStateSnapshotJsonStr = winStateSnapshotJson.dump();
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, winStateSnapshot=%{public}s",
        persistentId, winStateSnapshotJsonStr.c_str());
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetRotationLocked(bool locked)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow() &&
        !windowSystemConfig_.IsPhoneWindow()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "window is not pc, pad or phone, not supported.");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!(WindowHelper::IsSystemWindow(GetType()) && !WindowHelper::IsWindowInApp(GetType()))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "is not system window.");
        return WMError::WM_ERROR_INVALID_WINDOW_TYPE;
    }
 
    TLOGI(WmsLogTag::WMS_ROTATION, "set id %{public}u rotation lock %{public}d.", GetWindowId(), locked);
    property_->SetRotationLocked(locked);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE);
}
 
WMError WindowSceneSessionImpl::GetRotationLocked(bool& locked)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow() &&
        !windowSystemConfig_.IsPhoneWindow()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "window is not pc, pad or phone, not supported.");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!(WindowHelper::IsSystemWindow(GetType()) && !WindowHelper::IsWindowInApp(GetType()))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "is not system window.");
        return WMError::WM_ERROR_INVALID_WINDOW_TYPE;
    }
 
    locked = property_->GetRotationLocked();
    TLOGI(WmsLogTag::WMS_ROTATION, "get id %{public}u rotation lock %{public}d.", GetWindowId(), locked);
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::SetHookTargetElementInfo(const AppExecFwk::ElementName& elementName)
{
    auto context = GetContext();
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (!abilityContext) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilityContext is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (abilityContext->IsHook() && !abilityContext->GetHookOff()) {
        property_->EditSessionInfo().bundleName_ = elementName.GetBundleName();
        property_->EditSessionInfo().moduleName_ = elementName.GetModuleName();
        property_->EditSessionInfo().abilityName_ = elementName.GetAbilityName();
    }
    return WMError::WM_OK;
}

WMError WindowSceneSessionImpl::GetAppForceLandscapeConfig(AppForceLandscapeConfig& config)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "HostSession is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    return hostSession->GetAppForceLandscapeConfig(config);
}

WMError WindowSceneSessionImpl::GetAppForceLandscapeConfigEnable(bool& enableForceSplit)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "HostSession is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    return hostSession->GetAppForceLandscapeConfigEnable(enableForceSplit);
}

WSError WindowSceneSessionImpl::NotifyAppForceLandscapeConfigUpdated()
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    WindowType winType = GetType();
    AppForceLandscapeConfig config = {};
    if (WindowHelper::IsMainWindow(winType) && GetAppForceLandscapeConfig(config) == WMError::WM_OK &&
        config.supportSplit_ > 0) {
        SetForceSplitConfig(config);
        return WSError::WS_OK;
    }
    return WSError::WS_DO_NOTHING;
}

WSError WindowSceneSessionImpl::NotifyAppForceLandscapeConfigEnableUpdated()
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    WindowType winType = GetType();
    bool enableForceSplit = false;
    if (WindowHelper::IsMainWindow(winType) &&
        GetAppForceLandscapeConfigEnable(enableForceSplit) == WMError::WM_OK) {
        SetForceSplitConfigEnable(enableForceSplit);
        return WSError::WS_OK;
    }
    return WSError::WS_DO_NOTHING;
}

void WindowSceneSessionImpl::SetForceSplitConfigEnable(bool enableForceSplit)
{
    WindowType winType = GetType();
    if (!WindowHelper::IsMainWindow(winType)) {
        return;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "uiContent is null!");
        return;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "SetForceSplitEnable, enableForceSplit: %{public}u",
        enableForceSplit);
    uiContent->SetForceSplitEnable(enableForceSplit);
}

WMError WindowSceneSessionImpl::GetAppHookWindowInfoFromServer(HookWindowInfo& hookWindowInfo)
{
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    return hostSession->GetAppHookWindowInfoFromServer(hookWindowInfo);
}

WSError WindowSceneSessionImpl::NotifyAppHookWindowInfoUpdated()
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "in");
    const WindowType windowType = GetType();
    if (!WindowHelper::IsMainWindow(windowType)) {
        return WSError::WS_DO_NOTHING;
    }

    HookWindowInfo hookWindowInfo{};
    if (GetAppHookWindowInfoFromServer(hookWindowInfo) != WMError::WM_OK) {
        return WSError::WS_DO_NOTHING;
    }

    SetAppHookWindowInfo(hookWindowInfo);
    return WSError::WS_OK;
}

WMError WindowSceneSessionImpl::SetSubWindowSource(SubWindowSource source)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const auto& property = GetProperty();
    if (!WindowHelper::IsSubWindow(property->GetWindowType()) &&
        !WindowHelper::IsDialogWindow(property->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SUB, "only sub window and dialog is valid");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGI(WmsLogTag::WMS_SUB, "session is nullptr");
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    WSError ret = hostSession->SetSubWindowSource(source);
    if (ret == WSError::WS_ERROR_INVALID_WINDOW) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return ret != WSError::WS_OK ? WMError::WM_ERROR_SYSTEM_ABNORMALLY : WMError::WM_OK;
}

WMError WindowSceneSessionImpl::RestoreMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParams)
{
    TLOGI(WmsLogTag::WMS_LIFE, "restore float main window id: %{public}d", GetPersistentId());
    if (GetType() != WindowType::WINDOW_TYPE_FLOAT) {
        TLOGE(WmsLogTag::WMS_LIFE, "only TYPE_FLOAT can use interface, type:%{public}u", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto parentWindow = FindWindowById(property_->GetParentPersistentId());
    if (parentWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "parentWindow is invalid");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (state_ != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_LIFE, "window is not foreground showing");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGI(WmsLogTag::WMS_LIFE, "session is nullptr");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WMError ret = hostSession->RestoreFloatMainWindow(wantParams);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "RestoreFloatMainWindow result is: %{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    return WMError::WM_OK;
}

WSError WindowSceneSessionImpl::CloseSpecificScene()
{
    if (!property_->IsDecorEnable()) {
        TLOGW(WmsLogTag::WMS_SUB, "specific scene can not close id: %{public}d, decor is not enable.",
            GetPersistentId());
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    TLOGI(WmsLogTag::WMS_SUB, "close specific scene id: %{public}d", GetPersistentId());
    handler_->PostTask([weakThis = wptr(this)] {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_SUB, "window is nullptr");
            return;
        }
        window->Close();
    }, __func__);
    return WSError::WS_OK;
}

/*
 * Window Event
 */
WMError WindowSceneSessionImpl::LockCursor(int32_t windowId, bool isCursorFollowMovement)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_EVENT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::vector<int32_t> parameters;
    parameters.emplace_back(LOCK_CURSOR_LENGTH);
    parameters.emplace_back(windowId);
    parameters.emplace_back(static_cast<int32_t>(isCursorFollowMovement));
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return hostSession->SendCommonEvent(static_cast<int32_t>(CommonEventCommand::LOCK_CURSOR), parameters);
}

WMError WindowSceneSessionImpl::UnlockCursor(int32_t windowId)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_EVENT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::vector<int32_t> parameters;
    parameters.emplace_back(UNLOCK_CURSOR_LENGTH);
    parameters.emplace_back(windowId);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return hostSession->SendCommonEvent(static_cast<int32_t>(CommonEventCommand::UNLOCK_CURSOR), parameters);
}

WMError WindowSceneSessionImpl::SetReceiveDragEventEnabled(bool enabled)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_EVENT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::vector<int32_t> parameters;
    parameters.emplace_back(SET_RECEIVE_DRAG_EVENT_LENGTH);
    parameters.emplace_back(static_cast<int32_t>(enabled));
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto result = hostSession->SendCommonEvent(static_cast<int32_t>(CommonEventCommand::SET_RECEIVE_DRAG_EVENT),
        parameters);
    if (result == WMError::WM_OK) {
        isReceiveDragEventEnabled_ = enabled;
    }
    return result;
}

bool WindowSceneSessionImpl::IsReceiveDragEventEnabled()
{
    return isReceiveDragEventEnabled_;
}

WMError WindowSceneSessionImpl::SetSeparationTouchEnabled(bool enabled)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_EVENT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::vector<int32_t> parameters;
    parameters.emplace_back(WINDOW_SEPARATION_TOUCH_ENABLED_LENGTH);
    parameters.emplace_back(static_cast<int32_t>(enabled));
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto result = hostSession->SendCommonEvent(
        static_cast<int32_t>(CommonEventCommand::SET_WINDOW_SEPARATION_TOUCH_ENABLED), parameters);
    if (result == WMError::WM_OK) {
        isSeparationTouchEnabled_ = enabled;
    }
    return result;
}

bool WindowSceneSessionImpl::IsSeparationTouchEnabled()
{
    return isSeparationTouchEnabled_;
}

bool WindowSceneSessionImpl::IsHitHotAreas(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
	
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (!IsPcOrPadFreeMultiWindowMode()) {
        return false;
    }
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uiContent is null, windowId: %{public}u", GetWindowId());
        return false;
    }
    Rect windowRect = property_->GetWindowRect();
    MMI::PointerEvent::PointerItem pointerItem;
    bool isValidPointItem = pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    
    int32_t width = static_cast<int32_t>(windowRect.width_);
    int32_t height = static_cast<int32_t>(windowRect.height_);
    int32_t posX = windowRect.posX_;
    int32_t posY = windowRect.posY_;
    float vpr = WindowSessionImpl::GetVirtualPixelRatio();
    float scaleX = compatScaleX_;
    float scaleY = compatScaleY_;
    float outsideArea = HOTZONE_TOUCH * vpr * scaleX;
    float insideArea = WINDOW_FRAME_WIDTH * vpr * scaleX;
 
    bool isHitTopHotArea = pointerItem.GetDisplayX() > posX - outsideArea * scaleX &&
        pointerItem.GetDisplayX() < posX + (width + outsideArea) * scaleX &&
        pointerItem.GetDisplayY() > posY - outsideArea * scaleY &&
        pointerItem.GetDisplayY() < posY + (insideArea + outsideArea) * scaleY;
 
    bool isHitLeftHotArea = pointerItem.GetDisplayX() > posX - outsideArea * scaleX &&
        pointerItem.GetDisplayX() < posX + (insideArea + outsideArea) * scaleX &&
        pointerItem.GetDisplayY() > posY - outsideArea * scaleY &&
        pointerItem.GetDisplayY() < posY + (height + outsideArea) * scaleY;
 
    bool isHitRightHotArea = pointerItem.GetDisplayX() > posX + (width - insideArea) * scaleX &&
        pointerItem.GetDisplayX() < posX + (width + outsideArea) * scaleX &&
        pointerItem.GetDisplayY() > posY - outsideArea * scaleY &&
        pointerItem.GetDisplayY() < posY + (height + outsideArea) * scaleY;
    
    bool isHitBottomHotArea = pointerItem.GetDisplayX() > posX - outsideArea * scaleX &&
        pointerItem.GetDisplayX() < posX + (width + outsideArea) * scaleX &&
        pointerItem.GetDisplayY() > posY + (height - insideArea) * scaleY &&
        pointerItem.GetDisplayY() < posY + (height + outsideArea) * scaleY;
 
    bool isHitHotAreas = isHitTopHotArea || isHitLeftHotArea || isHitRightHotArea || isHitBottomHotArea;
    if (isValidPointItem && isHitHotAreas) {
        TLOGI(WmsLogTag::WMS_DECOR, "hitHotAreas success");
        return true;
    }
    return false;
}

WSError WindowSceneSessionImpl::AddSidebarBlur()
{
    TLOGI(WmsLogTag::WMS_PC, "in, persistentId=%{public}d", GetPersistentId());
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "handler is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    handler_->PostTask(
        [weakThis = wptr(this), where = __func__] {
            auto window = weakThis.promote();
            if (!window) {
                TLOGNE(WmsLogTag::WMS_PC, "%{public}s: window is nullptr", where);
                return;
            }
            if (!window->surfaceNode_) {
                TLOGNE(WmsLogTag::WMS_PC, "%{public}s: surfaceNode is null", where);
                return;
            }
            auto rsNodeTemp = RSAdapterUtil::GetRSNode(window->GetRSUIContext(), window->surfaceNode_->GetId());
            if (rsNodeTemp) {
                bool isDark = (window->colorMode_ == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
                window->AddRSNodeModifier(isDark, rsNodeTemp);
            }
        }, __func__);
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::AddRSNodeModifier(bool isDark, const std::shared_ptr<RSBaseNode>& rsNode)
{
    TLOGI(WmsLogTag::WMS_PC, "persistentId=%{public}d, isDark=%{public}d", GetPersistentId(), isDark);
    if (blurRadiusValue_ && blurSaturationValue_ && blurBrightnessValue_ && blurMaskColorValue_) {
        TLOGE(WmsLogTag::WMS_PC, "RSAnimatableProperty has value");
        return;
    }
    if (!rsNode) {
        TLOGE(WmsLogTag::WMS_PC, "rsNode is nullptr");
        return;
    }
    blurRadiusValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(
        isDark ? SIDEBAR_DEFAULT_RADIUS_DARK : SIDEBAR_DEFAULT_RADIUS_LIGHT);
    blurSaturationValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(
        isDark ? SIDEBAR_DEFAULT_SATURATION_DARK : SIDEBAR_DEFAULT_SATURATION_LIGHT);
    blurBrightnessValue_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(
        isDark ? SIDEBAR_DEFAULT_BRIGHTNESS_DARK : SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT);
    blurMaskColorValue_ = std::make_shared<RSAnimatableProperty<Rosen::RSColor>>(
        Rosen::RSColor::FromArgbInt(isDark ? SIDEBAR_DEFAULT_MASKCOLOR_DARK : SIDEBAR_DEFAULT_MASKCOLOR_LIGHT));
    auto modifier = std::make_shared<Rosen::ModifierNG::RSBehindWindowFilterModifier>();
    modifier->AttachProperty(ModifierNG::RSPropertyType::BEHIND_WINDOW_FILTER_RADIUS, blurRadiusValue_);
    modifier->AttachProperty(ModifierNG::RSPropertyType::BEHIND_WINDOW_FILTER_SATURATION, blurSaturationValue_);
    modifier->AttachProperty(ModifierNG::RSPropertyType::BEHIND_WINDOW_FILTER_BRIGHTNESS, blurBrightnessValue_);
    modifier->AttachProperty(ModifierNG::RSPropertyType::BEHIND_WINDOW_FILTER_MASK_COLOR, blurMaskColorValue_);
    rsNode->AddModifier(modifier);
}

WSError WindowSceneSessionImpl::SetSidebarBlurStyleWithType(SidebarBlurType type)
{
    TLOGI(WmsLogTag::WMS_PC, "in, persistentId=%{public}d, type: %{public}u", GetPersistentId(),
        static_cast<uint32_t>(type));
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "handler is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    handler_->PostTask(
        [weakThis = wptr(this), type, where = __func__] {
            auto window = weakThis.promote();
            if (!window) {
                TLOGNE(WmsLogTag::WMS_PC, "%{public}s: window is nullptr", where);
                return;
            }
            if (!window->blurRadiusValue_ || !window->blurSaturationValue_ || !window->blurBrightnessValue_ ||
                !window->blurMaskColorValue_) {
                TLOGNW(WmsLogTag::WMS_PC, "%{public}s: RSAnimatableProperty is null", where);
                return;
            }
            bool isDark = (window->colorMode_ == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
            window->ModifySidebarBlurProperty(isDark, type);
        }, __func__);
    return WSError::WS_OK;
}

void WindowSceneSessionImpl::UpdateSidebarBlurStyleWhenColorModeChange()
{
    TLOGI(WmsLogTag::WMS_PC, "in, persistentId=%{public}d", GetPersistentId());
    if (!blurRadiusValue_ || !blurSaturationValue_ || !blurBrightnessValue_ || !blurMaskColorValue_) {
        TLOGW(WmsLogTag::WMS_PC, "RSAnimatableProperty is null");
        return;
    }
    bool isDark = (colorMode_ == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
    SidebarBlurType type = (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN) ?
        SidebarBlurType::DEFAULT_MAXIMIZE : SidebarBlurType::DEFAULT_FLOAT;
    ModifySidebarBlurProperty(isDark, type);
}

void WindowSceneSessionImpl::ModifySidebarBlurProperty(bool isDark, SidebarBlurType type)
{
    TLOGI(WmsLogTag::WMS_PC, "persistentId=%{public}d, isDark: %{public}d, type: %{public}u", GetPersistentId(),
        isDark, static_cast<uint32_t>(type));
    auto rsUIContext = GetRSUIContext();
    if (type == SidebarBlurType::DEFAULT_FLOAT || type == SidebarBlurType::DEFAULT_MAXIMIZE) {
        Rosen::RSAnimationTimingProtocol timingProtocol;
        timingProtocol.SetDuration(SIDEBAR_BLUR_ANIMATION_DURATION);
        timingProtocol.SetDirection(true);
        timingProtocol.SetFillMode(Rosen::FillMode::FORWARDS);
        timingProtocol.SetFinishCallbackType(Rosen::FinishCallbackType::LOGICALLY);
        RSNode::OpenImplicitAnimation(rsUIContext, timingProtocol, Rosen::RSAnimationTimingCurve::LINEAR, nullptr);
    }
    switch (type) {
        case SidebarBlurType::NONE:
            blurRadiusValue_->Set(SIDEBAR_BLUR_NUMBER_ZERO);
            blurSaturationValue_->Set(SIDEBAR_BLUR_NUMBER_ZERO);
            blurBrightnessValue_->Set(SIDEBAR_BLUR_NUMBER_ZERO);
            blurMaskColorValue_->Set(Rosen::RSColor::FromArgbInt(
                isDark ? SIDEBAR_SNAPSHOT_MASKCOLOR_DARK : SIDEBAR_SNAPSHOT_MASKCOLOR_LIGHT));
            break;
        case SidebarBlurType::INITIAL:
        case SidebarBlurType::DEFAULT_FLOAT: {
            blurRadiusValue_->Set(isDark ? SIDEBAR_DEFAULT_RADIUS_DARK : SIDEBAR_DEFAULT_RADIUS_LIGHT);
            blurSaturationValue_->Set(isDark ? SIDEBAR_DEFAULT_SATURATION_DARK : SIDEBAR_DEFAULT_SATURATION_LIGHT);
            blurBrightnessValue_->Set(isDark ? SIDEBAR_DEFAULT_BRIGHTNESS_DARK : SIDEBAR_DEFAULT_BRIGHTNESS_LIGHT);
            blurMaskColorValue_->Set(Rosen::RSColor::FromArgbInt(
                isDark ? SIDEBAR_DEFAULT_MASKCOLOR_DARK : SIDEBAR_DEFAULT_MASKCOLOR_LIGHT));
            break;
        }
        case SidebarBlurType::DEFAULT_MAXIMIZE: {
            blurRadiusValue_->Set(isDark ? SIDEBAR_MAXIMIZE_RADIUS_DARK : SIDEBAR_MAXIMIZE_RADIUS_LIGHT);
            blurSaturationValue_->Set(isDark ? SIDEBAR_MAXIMIZE_SATURATION_DARK : SIDEBAR_MAXIMIZE_SATURATION_LIGHT);
            blurBrightnessValue_->Set(isDark ? SIDEBAR_MAXIMIZE_BRIGHTNESS_DARK : SIDEBAR_MAXIMIZE_BRIGHTNESS_LIGHT);
            blurMaskColorValue_->Set(Rosen::RSColor::FromArgbInt(
                isDark ? SIDEBAR_MAXIMIZE_MASKCOLOR_DARK : SIDEBAR_MAXIMIZE_MASKCOLOR_LIGHT));
            break;
        }
        default:
            break;
    }
    if (type == SidebarBlurType::DEFAULT_FLOAT || type == SidebarBlurType::DEFAULT_MAXIMIZE) {
        RSNode::CloseImplicitAnimation(rsUIContext);
    }
}

bool WindowSceneSessionImpl::IsInFreeWindowMode() const
{
    return IsPcOrPadFreeMultiWindowMode();
}
} // namespace Rosen
} // namespace OHOS
