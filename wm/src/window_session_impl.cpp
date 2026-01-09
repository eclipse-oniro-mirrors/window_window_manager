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

#include "window_session_impl.h"

#include <cstdlib>

#include <ability_manager_client.h>
#include <app_mgr_client.h>
#include <common/rs_common_def.h>
#include <filesystem>
#include <float_wrapper.h>
#include <fstream>
#include <ipc_skeleton.h>
#include <hisysevent.h>
#include <parameters.h>
#include <int_wrapper.h>
#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include <singleton.h>
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>
#include <ui_extension_utils.h>

#include "application_context.h"
#include "color_parser.h"
#include "common/include/fold_screen_state_internel.h"
#include "common/include/fold_screen_common.h"
#include "display_info.h"
#include "display_manager.h"
#include "extension/extension_business_info.h"
#include "hitrace_meter.h"
#include "rate_limited_logger.h"
#include "rs_adapter.h"
#include "scene_board_judgement.h"
#include "session_helper.h"
#include "session_permission.h"
#include "key_event.h"
#include "session/container/include/window_event_channel.h"
#include "session_manager/include/session_manager.h"
#include "vsync_station.h"
#include "window_adapter.h"
#include "window_inspector.h"
#include "window_manager_hilog.h"
#include "window_helper.h"
#include "color_parser.h"
#include "singleton_container.h"
#include "sys_cap_util.h"
#include "perform_reporter.h"
#include "picture_in_picture_manager.h"
#include "parameters.h"
#include "floating_ball_manager.h"
#include "sys_cap_util.h"
#include "wm_common_inner.h"
#include "window_input_redistribute_impl.h"

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSessionImpl"};
constexpr int32_t FORCE_SPLIT_MODE = 5;
constexpr int32_t NAV_FORCE_SPLIT_MODE = 6;
constexpr int32_t API_VERSION_18 = 18;
constexpr uint32_t API_VERSION_MOD = 1000;
constexpr int32_t  WINDOW_ROTATION_CHANGE = 50;
constexpr uint32_t INVALID_TARGET_API_VERSION = 0;
constexpr uint32_t OPAQUE = 0xFF000000;
constexpr int32_t WINDOW_CONNECT_TIMEOUT = 1000;
constexpr int32_t WINDOW_LIFECYCLE_TIMEOUT = 100;
constexpr int32_t MIN_ROTATION_VALUE = 0;
constexpr int32_t MAX_ROTATION_VALUE = 3;

/*
 * DFX
 */
const std::string SET_UIEXTENSION_DESTROY_TIMEOUT_LISTENER_TASK_NAME = "SetUIExtDestroyTimeoutListener";
const std::string BUTTON_BACKGROUND_CORNER_RADIUS = "buttonBackgroundCornerRadius";
const std::string BUTTON_BACKGROUND_SIZE = "buttonBackgroundSize";
const std::string BUTTON_ICON_SIZE = "buttonIconSize";
const std::string CLOSE_BUTTON_RIGHT_MARGIN = "closeButtonRightMargin";
const std::string BUTTON_COLOR_MODE = "colorMode";
const std::string BUTTON_SPACING_BETWEEN = "spacingBetweenButtons";
const std::string DECOR_BUTTON_STYLE_CHANGE = "decor_button_style_change";
constexpr int64_t SET_UIEXTENSION_DESTROY_TIMEOUT_TIME_MS = 4000;

const std::string SCB_BACK_VISIBILITY = "scb_back_visibility";
const std::string SCB_COMPATIBLE_MAXIMIZE_VISIBILITY = "scb_compatible_maximize_visibility";
const std::string SCB_COMPATIBLE_MAXIMIZE_BTN_RES = "scb_compatible_maximize_btn_res";
const std::string SCB_COMPATIBLE_MENU_VISIBILITY = "scb_set_compatible_menu";
const std::string SCB_GET_COMPATIBLE_PRIMARY_MODE = "scb_get_compatible_primary_mode";

Ace::ContentInfoType GetAceContentInfoType(BackupAndRestoreType type)
{
    auto contentInfoType = Ace::ContentInfoType::NONE;
    switch (type) {
        case BackupAndRestoreType::CONTINUATION:
            contentInfoType = Ace::ContentInfoType::CONTINUATION;
            break;
        case BackupAndRestoreType::APP_RECOVERY:
            contentInfoType = Ace::ContentInfoType::APP_RECOVERY;
            break;
        case BackupAndRestoreType::RESOURCESCHEDULE_RECOVERY:
            contentInfoType = Ace::ContentInfoType::RESOURCESCHEDULE_RECOVERY;
            break;
        case BackupAndRestoreType::NONE:
            [[fallthrough]];
        default:
            break;
    }
    return contentInfoType;
}

Ace::ViewportConfig FillViewportConfig(
    Rect rect, float density, int32_t orientation, uint32_t transformHint, uint64_t displayId)
{
    Ace::ViewportConfig config;
    config.SetSize(rect.width_, rect.height_);
    config.SetPosition(rect.posX_, rect.posY_);
    config.SetDensity(density);
    config.SetOrientation(orientation);
    config.SetTransformHint(transformHint);
    config.SetDisplayId(displayId);
    return config;
}
}

std::map<int32_t, std::vector<sptr<ISystemBarPropertyListener>>> WindowSessionImpl::systemBarPropertyListeners_;
std::map<int32_t, std::vector<sptr<IWindowLifeCycle>>> WindowSessionImpl::lifecycleListeners_;
std::map<int32_t, std::vector<sptr<IWindowStageLifeCycle>>> WindowSessionImpl::windowStageLifecycleListeners_;
std::map<int32_t, std::vector<sptr<IDisplayMoveListener>>> WindowSessionImpl::displayMoveListeners_;
std::map<int32_t, std::vector<sptr<IWindowChangeListener>>> WindowSessionImpl::windowChangeListeners_;
std::map<int32_t, std::vector<sptr<IWindowCrossAxisListener>>> WindowSessionImpl::windowCrossAxisListeners_;
std::map<int32_t, std::vector<sptr<IAvoidAreaChangedListener>>> WindowSessionImpl::avoidAreaChangeListeners_;
std::map<int32_t, std::vector<sptr<IDialogDeathRecipientListener>>> WindowSessionImpl::dialogDeathRecipientListeners_;
std::map<int32_t, std::vector<sptr<IDialogTargetTouchListener>>> WindowSessionImpl::dialogTargetTouchListener_;
std::map<int32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> WindowSessionImpl::occupiedAreaChangeListeners_;
std::map<int32_t, std::vector<sptr<IKBWillShowListener>>> WindowSessionImpl::keyboardWillShowListeners_;
std::map<int32_t, std::vector<sptr<IKBWillHideListener>>> WindowSessionImpl::keyboardWillHideListeners_;
std::map<int32_t, std::vector<sptr<IKeyboardDidShowListener>>> WindowSessionImpl::keyboardDidShowListeners_;
std::map<int32_t, std::vector<sptr<IKeyboardDidHideListener>>> WindowSessionImpl::keyboardDidHideListeners_;
std::map<int32_t, std::vector<sptr<IScreenshotListener>>> WindowSessionImpl::screenshotListeners_;
std::unordered_map<int32_t, std::vector<IScreenshotAppEventListenerSptr>>
    WindowSessionImpl::screenshotAppEventListeners_;
std::map<int32_t, std::vector<sptr<ITouchOutsideListener>>> WindowSessionImpl::touchOutsideListeners_;
std::map<int32_t, std::vector<IWindowVisibilityListenerSptr>> WindowSessionImpl::windowVisibilityChangeListeners_;
std::mutex WindowSessionImpl::occlusionStateChangeListenerMutex_;
std::unordered_map<int32_t,
    std::vector<sptr<IOcclusionStateChangedListener>>> WindowSessionImpl::occlusionStateChangeListeners_;
std::mutex WindowSessionImpl::frameMetricsChangeListenerMutex_;
std::unordered_map<int32_t,
    std::vector<sptr<IFrameMetricsChangedListener>>> WindowSessionImpl::frameMetricsChangeListeners_;
std::mutex WindowSessionImpl::displayIdChangeListenerMutex_;
std::map<int32_t, std::vector<IDisplayIdChangeListenerSptr>> WindowSessionImpl::displayIdChangeListeners_;
std::mutex WindowSessionImpl::systemDensityChangeListenerMutex_;
std::unordered_map<int32_t, std::vector<ISystemDensityChangeListenerSptr>>
    WindowSessionImpl::systemDensityChangeListeners_;
std::recursive_mutex WindowSessionImpl::acrossDisplaysChangeListenerMutex_;
std::unordered_map<int32_t, std::vector<IAcrossDisplaysChangeListenerSptr>>
    WindowSessionImpl::acrossDisplaysChangeListeners_;
std::map<int32_t, std::vector<IWindowNoInteractionListenerSptr>> WindowSessionImpl::windowNoInteractionListeners_;
std::map<int32_t, std::vector<sptr<IWindowTitleButtonRectChangedListener>>>
    WindowSessionImpl::windowTitleButtonRectChangeListeners_;
std::map<int32_t, std::vector<sptr<IWindowRectChangeListener>>> WindowSessionImpl::windowRectChangeListeners_;
std::map<int32_t, std::vector<sptr<IWindowTitleChangeListener>>> WindowSessionImpl::windowTitleChangeListeners_;
std::map<int32_t, std::vector<sptr<IWindowTitleOrHotAreasListener>>>
    WindowSessionImpl::windowTitleOrHotAreasListeners_;
std::map<int32_t, std::vector<sptr<IRectChangeInGlobalDisplayListener>>>
    WindowSessionImpl::rectChangeInGlobalDisplayListeners_;
std::map<int32_t, std::vector<sptr<IExtensionSecureLimitChangeListener>>>
    WindowSessionImpl::secureLimitChangeListeners_;
std::map<int32_t, sptr<ISubWindowCloseListener>> WindowSessionImpl::subWindowCloseListeners_;
std::map<int32_t, sptr<IMainWindowCloseListener>> WindowSessionImpl::mainWindowCloseListeners_;
std::map<int32_t, sptr<IPreferredOrientationChangeListener>> WindowSessionImpl::preferredOrientationChangeListener_;
std::map<int32_t, sptr<IWindowOrientationChangeListener>> WindowSessionImpl::windowOrientationChangeListener_;
std::unordered_map<int32_t, std::vector<sptr<IWindowWillCloseListener>>> WindowSessionImpl::windowWillCloseListeners_;
std::map<int32_t, std::vector<sptr<ISwitchFreeMultiWindowListener>>> WindowSessionImpl::switchFreeMultiWindowListeners_;
std::map<int32_t, std::vector<sptr<IWindowHighlightChangeListener>>> WindowSessionImpl::highlightChangeListeners_;
std::map<int32_t, std::vector<sptr<IWindowRotationChangeListener>>> WindowSessionImpl::windowRotationChangeListeners_;
std::map<int32_t, std::vector<sptr<IFreeWindowModeChangeListener>>> WindowSessionImpl::freeWindowModeChangeListeners_;
std::recursive_mutex WindowSessionImpl::lifeCycleListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowStageLifeCycleListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowChangeListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowCrossAxisListenerMutex_;
std::recursive_mutex WindowSessionImpl::avoidAreaChangeListenerMutex_;
std::recursive_mutex WindowSessionImpl::dialogDeathRecipientListenerMutex_;
std::recursive_mutex WindowSessionImpl::dialogTargetTouchListenerMutex_;
std::recursive_mutex WindowSessionImpl::occupiedAreaChangeListenerMutex_;
std::recursive_mutex WindowSessionImpl::keyboardWillShowListenerMutex_;
std::recursive_mutex WindowSessionImpl::keyboardWillHideListenerMutex_;
std::recursive_mutex WindowSessionImpl::keyboardDidShowListenerMutex_;
std::recursive_mutex WindowSessionImpl::keyboardDidHideListenerMutex_;
std::recursive_mutex WindowSessionImpl::screenshotListenerMutex_;
std::recursive_mutex WindowSessionImpl::screenshotAppEventListenerMutex_;
std::recursive_mutex WindowSessionImpl::touchOutsideListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowVisibilityChangeListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowNoInteractionListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowStatusChangeListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowStatusDidChangeListenerMutex_;
std::recursive_mutex WindowSessionImpl::windowTitleButtonRectChangeListenerMutex_;
std::mutex WindowSessionImpl::displayMoveListenerMutex_;
std::mutex WindowSessionImpl::windowRectChangeListenerMutex_;
std::mutex WindowSessionImpl::windowTitleChangeListenerMutex_;
std::mutex WindowSessionImpl::windowTitleOrHotAreasListenerMutex_;
std::mutex WindowSessionImpl::rectChangeInGlobalDisplayListenerMutex_;
std::mutex WindowSessionImpl::secureLimitChangeListenerMutex_;
std::mutex WindowSessionImpl::subWindowCloseListenersMutex_;
std::mutex WindowSessionImpl::mainWindowCloseListenersMutex_;
std::recursive_mutex WindowSessionImpl::windowWillCloseListenersMutex_;
std::mutex WindowSessionImpl::switchFreeMultiWindowListenerMutex_;
std::mutex WindowSessionImpl::preferredOrientationChangeListenerMutex_;
std::mutex WindowSessionImpl::windowOrientationChangeListenerMutex_;
std::mutex WindowSessionImpl::highlightChangeListenerMutex_;
std::mutex WindowSessionImpl::systemBarPropertyListenerMutex_;
std::mutex WindowSessionImpl::waterfallModeChangeListenerMutex_;
std::unordered_map<int32_t, std::vector<sptr<IWaterfallModeChangeListener>>>
    WindowSessionImpl::waterfallModeChangeListeners_;
std::mutex WindowSessionImpl::windowRotationChangeListenerMutex_;
std::mutex WindowSessionImpl::freeWindowModeChangeListenerMutex_;
std::map<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>> WindowSessionImpl::windowSessionMap_;
std::shared_mutex WindowSessionImpl::windowSessionMutex_;
std::set<sptr<WindowSessionImpl>> g_windowExtensionSessionSet_;
std::atomic<int64_t> WindowSessionImpl::updateFocusTimeStamp_;
std::atomic<int64_t> WindowSessionImpl::updateHighlightTimeStamp_;
std::shared_mutex WindowSessionImpl::windowExtensionSessionMutex_;
std::recursive_mutex WindowSessionImpl::subWindowSessionMutex_;
std::map<int32_t, std::vector<sptr<WindowSessionImpl>>> WindowSessionImpl::subWindowSessionMap_;
std::map<int32_t, std::vector<sptr<IWindowStatusChangeListener>>> WindowSessionImpl::windowStatusChangeListeners_;
std::map<int32_t, std::vector<sptr<IWindowStatusDidChangeListener>>> WindowSessionImpl::windowStatusDidChangeListeners_;
bool WindowSessionImpl::isUIExtensionAbilityProcess_ = false;

#define CALL_LIFECYCLE_LISTENER(windowLifecycleCb, listeners) \
    do {                                                      \
        for (auto& listener : (listeners)) {                  \
            if (listener != nullptr) {            \
                listener->windowLifecycleCb();    \
            }                                                 \
        }                                                     \
    } while (0)

#define CALL_LIFECYCLE_LISTENER_WITH_PARAM(windowLifecycleCb, listeners, param) \
    do {                                                                        \
        for (auto& listener : (listeners)) {                                    \
            if (listener != nullptr) {                                         \
                listener->windowLifecycleCb(param);                 \
            }                                                                   \
        }                                                                       \
    } while (0)

#define CALL_UI_CONTENT(uiContentCb)                                           \
    do {                                                                       \
        std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();   \
        if (uiContent != nullptr) {                                            \
            uiContent->uiContentCb();                                          \
        }                                                                      \
    } while (0)

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

#define CHECK_UI_CONTENT_RETURN_IF_NULL(uiContent)                             \
    do {                                                                       \
        if ((uiContent) == nullptr) {                                          \
            TLOGE(WmsLogTag::WMS_LIFE, "uiContent is null");                   \
            return;                                                            \
        }                                                                      \
    } while (false)

WindowSessionImpl::WindowSessionImpl(const sptr<WindowOption>& option,
    const std::shared_ptr<RSUIContext>& rsUIContext)
{
    WLOGFD("[WMSCom] Constructor");
    property_ = sptr<WindowSessionProperty>::MakeSptr();
    windowOption_ = option;
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    WindowType optionWindowType = option->GetWindowType();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = option->GetBundleName();
    property_->SetSessionInfo(sessionInfo);
    property_->SetWindowType(optionWindowType);
    InitPropertyFromOption(option);
    isIgnoreSafeArea_ = WindowHelper::IsSubWindow(optionWindowType);
    followCreatorLifecycle_ = option->IsFollowCreatorLifecycle();

    RSAdapterUtil::InitRSUIDirector(rsUIDirector_, true, true, rsUIContext);
    if (WindowHelper::IsSubWindow(GetType())) {
        property_->SetDecorEnable(option->GetSubWindowDecorEnable());
    }
    surfaceNode_ = CreateSurfaceNode(property_->GetWindowName(), optionWindowType);
    if (surfaceNode_ != nullptr) {
        vsyncStation_ = std::make_shared<VsyncStation>(surfaceNode_->GetId());
    }
    WindowHelper::SplitStringByDelimiter(
        system::GetParameter("const.window.containerColorLists", ""), ",", containerColorList_);
}

void WindowSessionImpl::InitPropertyFromOption(const sptr<WindowOption>& option)
{
    property_->SetWindowName(option->GetWindowName());
    property_->SetRequestRect(option->GetWindowRect());
    property_->SetFocusable(option->GetFocusable());
    property_->SetTouchable(option->GetTouchable());
    property_->SetDisplayId(option->GetDisplayId());
    property_->SetParentId(option->GetParentId());
    property_->SetTurnScreenOn(option->IsTurnScreenOn());
    property_->SetKeepScreenOn(option->IsKeepScreenOn());
    property_->SetViewKeepScreenOn(option->IsViewKeepScreenOn());
    property_->SetWindowMode(option->GetWindowMode());
    property_->SetWindowFlags(option->GetWindowFlags());
    property_->SetCallingSessionId(option->GetCallingWindow());
    property_->SetIsUIExtFirstSubWindow(option->GetIsUIExtFirstSubWindow());
    property_->SetTopmost(option->GetWindowTopmost());
    property_->SetSubWindowZLevel(option->GetSubWindowZLevel());
    property_->SetZIndex(option->GetZIndex());
    property_->SetRealParentId(option->GetRealParentId());
    property_->SetParentWindowType(option->GetParentWindowType());
    property_->SetUIExtensionUsage(static_cast<UIExtensionUsage>(option->GetUIExtensionUsage()));
    property_->SetIsUIExtAnySubWindow(option->GetIsUIExtAnySubWindow());
    property_->SetIsSystemKeyboard(option->IsSystemKeyboard());
    property_->SetConstrainedModal(option->IsConstrainedModal());
    property_->SetSubWindowOutlineEnabled(option->IsSubWindowOutlineEnabled());
    layoutCallback_ = sptr<FutureCallback>::MakeSptr();
    getTargetInfoCallback_ = sptr<FutureCallback>::MakeSptr();
    getRotationResultFuture_ = sptr<FutureCallback>::MakeSptr();
    updateRectCallback_ = sptr<FutureCallback>::MakeSptr();
    isMainHandlerAvailable_ = option->GetMainHandlerAvailable();
}

bool WindowSessionImpl::IsPcWindow() const
{
    return windowSystemConfig_.IsPcWindow();
}

bool WindowSessionImpl::IsPadWindow() const
{
    return windowSystemConfig_.IsPadWindow();
}

bool WindowSessionImpl::IsPhonePadOrPcWindow() const
{
    return windowSystemConfig_.IsPhoneWindow() ||
        windowSystemConfig_.IsPadWindow() ||
        windowSystemConfig_.IsPcWindow();
}

bool WindowSessionImpl::IsPcOrFreeMultiWindowCapabilityEnabled() const
{
    return WindowSessionImpl::IsPcOrPadFreeMultiWindowMode() ||
        property_->GetIsPcAppInPad() ||
        IsDeviceFeatureCapableForFreeMultiWindow();
}

bool WindowSessionImpl::IsPcOrPadFreeMultiWindowMode() const
{
    return windowSystemConfig_.IsPcWindow() || IsFreeMultiWindowMode();
}

bool WindowSessionImpl::IsPadAndNotFreeMultiWindowCompatibleMode() const
{
    return property_->GetPcAppInpadCompatibleMode() &&
        !IsFreeMultiWindowMode();
}

bool WindowSessionImpl::IsSceneBoardEnabled() const
{
    return SceneBoardJudgement::IsSceneBoardEnabled();
}

// method will be instead after ace fix
bool WindowSessionImpl::GetCompatibleModeInPc() const
{
    return property_->IsAdaptToImmersive();
}

bool WindowSessionImpl::IsAdaptToCompatibleImmersive() const
{
    return property_->IsAdaptToImmersive();
}

bool WindowSessionImpl::IsAdaptToSimulationScale() const
{
    auto property = GetPropertyByContext();
    return property->IsAdaptToSimulationScale();
}

bool WindowSessionImpl::IsAdaptToProportionalScale() const
{
    auto property = GetPropertyByContext();
    return property->IsAdaptToProportionalScale();
}

bool WindowSessionImpl::IsInCompatScaleMode() const
{
    auto property = GetPropertyByContext();
    return property->IsAdaptToSimulationScale() || property->IsAdaptToProportionalScale();
}

bool WindowSessionImpl::IsInCompatScaleStatus() const
{
    return IsInCompatScaleMode() && (!NearEqual(compatScaleX_, 1.0f) || !NearEqual(compatScaleY_, 1.0f));
}

bool WindowSessionImpl::IsAdaptToSubWindow() const
{
    auto property = GetPropertyByContext();
    return property->IsAdaptToSubWindow();
}

void WindowSessionImpl::MakeSubOrDialogWindowDragableAndMoveble()
{
    if (IsPcOrFreeMultiWindowCapabilityEnabled() && windowOption_ != nullptr) {
        TLOGI(WmsLogTag::WMS_PC, "Called %{public}d.", GetPersistentId());
        // The context of the UEC child window is not the context of the main window
        auto mainWindow = FindMainWindowWithContext();
        // The child Windows created by compatible applications are mounted within the parent window,
        // with the same specifications as the mobile phone. No child Windows with title bars are created
        if (mainWindow && mainWindow->IsAdaptToSubWindow()) {
            TLOGE(WmsLogTag::WMS_COMPAT, "compat sub window not has title");
            return;
        }
        if (WindowHelper::IsSubWindow(property_->GetWindowType())) {
            TLOGI(WmsLogTag::WMS_PC, "create subwindow, title: %{public}s, decorEnable: %{public}d",
                windowOption_->GetSubWindowTitle().c_str(), windowOption_->GetSubWindowDecorEnable());
            property_->SetDecorEnable(windowOption_->GetSubWindowDecorEnable());
            property_->SetDragEnabled(windowOption_->GetSubWindowDecorEnable());
            UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
            subWindowTitle_ = windowOption_->GetSubWindowTitle();
        }
        bool isDialog = WindowHelper::IsDialogWindow(property_->GetWindowType());
        if (isDialog) {
            bool dialogDecorEnable = windowOption_->GetDialogDecorEnable();
            property_->SetDecorEnable(dialogDecorEnable);
            property_->SetDragEnabled(dialogDecorEnable);
            UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
            dialogTitle_ = windowOption_->GetDialogTitle();
            TLOGI(WmsLogTag::WMS_PC, "create dialogWindow, title: %{public}s, decorEnable: %{public}d",
                dialogTitle_.c_str(), dialogDecorEnable);
        }
    }
}

void WindowSessionImpl::SetSubWindowZLevelToProperty()
{
    if (WindowHelper::IsNormalSubWindow(property_->GetWindowType(), property_->GetWindowFlags())) {
        property_->SetSubWindowZLevel(windowOption_->GetSubWindowZLevel());
    }
}

RSSurfaceNode::SharedPtr WindowSessionImpl::CreateSurfaceNode(const std::string& name, WindowType type)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = name;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    switch (type) {
        case WindowType::WINDOW_TYPE_BOOT_ANIMATION:
        case WindowType::WINDOW_TYPE_POINTER:
            rsSurfaceNodeType = RSSurfaceNodeType::SELF_DRAWING_WINDOW_NODE;
            break;
        case WindowType::WINDOW_TYPE_APP_MAIN_WINDOW:
            rsSurfaceNodeType = RSSurfaceNodeType::APP_WINDOW_NODE;
            break;
        case WindowType::WINDOW_TYPE_UI_EXTENSION:
            TLOGD(WmsLogTag::WMS_UIEXT, "uiExtensionUsage=%{public}u", property_->GetUIExtensionUsage());
            if (SessionHelper::IsSecureUIExtension(property_->GetUIExtensionUsage())) {
                rsSurfaceNodeType = RSSurfaceNodeType::UI_EXTENSION_SECURE_NODE;
            } else {
                rsSurfaceNodeType = RSSurfaceNodeType::UI_EXTENSION_COMMON_NODE;
            }
            break;
        case WindowType::WINDOW_TYPE_PIP:
            rsSurfaceNodeType = RSSurfaceNodeType::APP_WINDOW_NODE;
            break;
        case WindowType::WINDOW_TYPE_MAGNIFICATION:
            rsSurfaceNodeType = RSSurfaceNodeType::ABILITY_MAGNIFICATION_NODE;
            break;
        default:
            rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
            break;
    }
    auto surfaceNode = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, rsSurfaceNodeType, true, property_->IsConstrainedModal(), GetRSUIContext());
    RSAdapterUtil::SetSkipCheckInMultiInstance(surfaceNode, true);
    TLOGD(WmsLogTag::WMS_SCB, "Create RSSurfaceNode: %{public}s, name: %{public}s",
          RSAdapterUtil::RSNodeToStr(surfaceNode).c_str(), name.c_str());
    return surfaceNode;
}

WindowSessionImpl::~WindowSessionImpl()
{
    WLOGFD("[WMSCom] id: %{public}d", GetPersistentId());
    Destroy(true, false);
}

uint32_t WindowSessionImpl::GetWindowId() const
{
    return static_cast<uint32_t>(GetPersistentId()) & 0xffffffff; // 0xffffffff: to get low 32 bits
}

uint64_t WindowSessionImpl::GetDisplayId() const
{
    return property_->GetDisplayId();
}

int32_t WindowSessionImpl::GetParentId() const
{
    // 0xffffffff: to get low 32 bits
    uint32_t parentID = static_cast<uint32_t>(property_->GetParentPersistentId()) & 0x7fffffff;
    return static_cast<int32_t>(parentID);
}

bool WindowSessionImpl::IsWindowSessionInvalid() const
{
    bool res = ((GetHostSession() == nullptr) || (GetPersistentId() == INVALID_SESSION_ID) ||
        (state_ == WindowState::STATE_DESTROYED));
    if (res) {
        TLOGW(WmsLogTag::WMS_LIFE, "already destroyed or not created! id: %{public}d state_: %{public}u",
            GetPersistentId(), state_);
    }
    return res;
}

bool WindowSessionImpl::IsMainHandlerAvailable() const
{
    TLOGI(WmsLogTag::DEFAULT, "id:%{public}d, isAvailable:%{public}u",
        GetPersistentId(), isMainHandlerAvailable_);
    return isMainHandlerAvailable_;
}

int32_t WindowSessionImpl::GetPersistentId() const
{
    return property_->GetPersistentId();
}

sptr<WindowSessionProperty> WindowSessionImpl::GetProperty() const
{
    return property_;
}

SystemSessionConfig WindowSessionImpl::GetSystemSessionConfig() const
{
    return windowSystemConfig_;
}

sptr<ISession> WindowSessionImpl::GetHostSession() const
{
    std::lock_guard<std::mutex> lock(hostSessionMutex_);
    return hostSession_;
}

ColorSpace WindowSessionImpl::GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut)
{
    if (colorGamut == GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB) {
        return ColorSpace::COLOR_SPACE_DEFAULT;
    } else if (colorGamut == GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3) {
        return ColorSpace::COLOR_SPACE_WIDE_GAMUT;
    } else {
        WLOGFE("try to get not exist ColorSpace");
        return ColorSpace::COLOR_SPACE_DEFAULT;
    }
}

GraphicColorGamut WindowSessionImpl::GetSurfaceGamutFromColorSpace(ColorSpace colorSpace)
{
    if (colorSpace == ColorSpace::COLOR_SPACE_DEFAULT) {
        return GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    } else if (colorSpace == ColorSpace::COLOR_SPACE_WIDE_GAMUT) {
        return GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3;
    } else {
        WLOGFE("try to get not exist colorGamut");
        return GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    }
}

bool WindowSessionImpl::IsSupportWideGamut()
{
    return true;
}

void WindowSessionImpl::SetColorSpace(ColorSpace colorSpace)
{
    if (IsWindowSessionInvalid() || surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is invalid");
        return;
    }
    auto colorGamut = GetSurfaceGamutFromColorSpace(colorSpace);
    surfaceNode_->SetColorSpace(colorGamut);
}

ColorSpace WindowSessionImpl::GetColorSpace()
{
    if (IsWindowSessionInvalid() || surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is invalid");
        return ColorSpace::COLOR_SPACE_DEFAULT;
    }
    GraphicColorGamut colorGamut = surfaceNode_->GetColorSpace();
    return GetColorSpaceFromSurfaceGamut(colorGamut);
}

WMError WindowSessionImpl::WindowSessionCreateCheck()
{
    if (vsyncStation_ == nullptr || !vsyncStation_->IsVsyncReceiverCreated()) {
        return WMError::WM_ERROR_NULLPTR;
    }
    const auto& name = property_->GetWindowName();
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    // check window name, same window names are forbidden
    if (windowSessionMap_.find(name) != windowSessionMap_.end()) {
        WLOGFE("WindowName(%{public}s) already exists.", name.c_str());
        return WMError::WM_ERROR_REPEAT_OPERATION;
    }

    // check if camera floating window is already exists
    if (property_->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA ||
        property_->GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        for (const auto& item : windowSessionMap_) {
            if (item.second.second && item.second.second->property_ &&
                item.second.second->property_->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
                    WLOGFE("Camera floating window is already exists.");
                return WMError::WM_ERROR_REPEAT_OPERATION;
            }
        }
        uint32_t accessTokenId = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
        property_->SetAccessTokenId(accessTokenId);
        TLOGI(WmsLogTag::DEFAULT, "Create camera float window, TokenId=%{private}u", accessTokenId);
    }
    return WMError::WM_OK;
}

void WindowSessionImpl::SetDefaultDisplayIdIfNeed()
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    auto displayId = property_->GetDisplayId();
    if (displayId == DISPLAY_ID_INVALID) {
        auto defaultDisplayId = SingletonContainer::IsDestroyed() ? DISPLAY_ID_INVALID :
            SingletonContainer::Get<DisplayManager>().GetDefaultDisplayId();
        defaultDisplayId = (defaultDisplayId == DISPLAY_ID_INVALID) ? 0 : defaultDisplayId;
        property_->SetDisplayId(defaultDisplayId);
        property_->SetIsFollowParentWindowDisplayId(true);
        TLOGI(WmsLogTag::DEFAULT, "%{public}" PRIu64, defaultDisplayId);
    }
}

/** @note @window.hierarchy */
int32_t WindowSessionImpl::GetSubWindowZLevelByFlags(WindowType type, uint32_t windowFlags, bool isTopmost)
{
    if (WindowHelper::IsApplicationModalSubWindow(type, windowFlags)) {
        if (isTopmost) {
            return APPLICATION_MODALITY_SUB_WINDOW_Z_LEVEL + TOPMOST_SUB_WINDOW_Z_LEVEL;
        }
        return APPLICATION_MODALITY_SUB_WINDOW_Z_LEVEL;
    } else if (WindowHelper::IsModalSubWindow(type, windowFlags)) {
        if (isTopmost) {
            return MODALITY_SUB_WINDOW_Z_LEVEL + TOPMOST_SUB_WINDOW_Z_LEVEL;
        }
        return MODALITY_SUB_WINDOW_Z_LEVEL;
    } else if (WindowHelper::IsToastSubWindow(type, windowFlags)) {
        return TOAST_SUB_WINDOW_Z_LEVEL;
    } else if (WindowHelper::IsTextMenuSubWindow(type, windowFlags)) {
        return TEXT_MENU_SUB_WINDOW_Z_LEVEL;
    } else if (WindowHelper::IsDialogWindow(type)) {
        return DIALOG_SUB_WINDOW_Z_LEVEL;
    }
    return NORMAL_SUB_WINDOW_Z_LEVEL;
}

WMError WindowSessionImpl::Connect()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    sptr<ISessionStage> iSessionStage(this);
    auto windowEventChannel = sptr<WindowEventChannel>::MakeSptr(iSessionStage);
    windowEventChannel->SetIsUIExtension(property_->GetWindowType() == WindowType::WINDOW_TYPE_UI_EXTENSION);
    windowEventChannel->SetUIExtensionUsage(property_->GetUIExtensionUsage());
    sptr<IWindowEventChannel> iWindowEventChannel(windowEventChannel);
    auto context = GetContext();
    sptr<IRemoteObject> token = context ? context->GetToken() : nullptr;
    uint64_t originDisplayId = GetDisplayId();
    if (token) {
        property_->SetTokenState(true);
    }
    auto ret = hostSession->Connect(
        iSessionStage, iWindowEventChannel, surfaceNode_, windowSystemConfig_, property_,
        token, identityToken_);
    if (SysCapUtil::GetBundleName() != AppExecFwk::Constants::SCENE_BOARD_BUNDLE_NAME &&
        WindowHelper::IsMainWindow(GetType()) && !property_->GetMissionInfo().startupInvisibility_) {
        auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        updateRectCallback_->GetUpdateRectResult(WINDOW_CONNECT_TIMEOUT);
        auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto waitTime = endTime - startTime;
        if (waitTime >= WINDOW_CONNECT_TIMEOUT) {
            TLOGW(WmsLogTag::WMS_LIFE, "Connect timeout, persistentId:%{public}d", GetPersistentId());
        }
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Window Connect [name:%{public}s, id:%{public}d, type:%{public}u], ret:%{public}u",
        property_->GetWindowName().c_str(), GetPersistentId(), property_->GetWindowType(), ret);
    if (originDisplayId != property_->GetDisplayId()) {
        NotifyDmsDisplayMove(property_->GetDisplayId());
    }
    if (IsInCompatScaleMode() || WindowHelper::IsUIExtensionWindow(GetType())) {
        RegisterWindowScaleCallback();
    }
    return static_cast<WMError>(ret);
}

sptr<WindowSessionImpl> WindowSessionImpl::GetWindowWithId(uint32_t windowId)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    if (windowSessionMap_.empty()) {
        TLOGD(WmsLogTag::WMS_SYSTEM, "Please create mainWindow First!");
        return nullptr;
    }
    for (const auto& [_, pair] : windowSessionMap_) {
        auto& window = pair.second;
        if (window && windowId == window->GetWindowId()) {
            TLOGD(WmsLogTag::WMS_SYSTEM, "find window %{public}s, id %{public}d",
                window->GetWindowName().c_str(), windowId);
            return window;
        }
    }
    TLOGD(WmsLogTag::WMS_SYSTEM, "Cannot find Window!");
    return nullptr;
}

sptr<WindowSessionImpl> WindowSessionImpl::GetScaleWindow(uint32_t windowId)
{
    sptr<WindowSessionImpl> window = GetWindowWithId(windowId);
    if (window) {
        TLOGD(WmsLogTag::WMS_COMPAT, "find window id:%{public}d", windowId);
        return window;
    }
    if (isUIExtensionAbilityProcess_) {
        std::shared_lock<std::shared_mutex> lock(windowExtensionSessionMutex_);
        for (const auto& window : GetWindowExtensionSessionSet()) {
            if (window && static_cast<uint32_t>(window->GetProperty()->GetParentId()) == windowId) {
                TLOGD(WmsLogTag::WMS_COMPAT, "find extension window id:%{public}d", window->GetPersistentId());
                return window;
            }
        }
    }
    {
        std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
        for (const auto& [_, pair] : windowSessionMap_) {
            auto& window = pair.second;
            if (window && window->IsFocused()) {
                TLOGD(WmsLogTag::WMS_COMPAT, "find focus window id:%{public}d", window->GetPersistentId());
                return window;
            }
        }
    }
    return nullptr;
}

WMError WindowSessionImpl::GetWindowScaleCoordinate(uint32_t windowId, CursorInfo& cursorInfo)
{
    if (cursorInfo.isInvalid()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "id:%{public}d cursorInfo is invalid", windowId);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    sptr<WindowSessionImpl> window = GetScaleWindow(windowId);
    if (!window) {
        TLOGE(WmsLogTag::WMS_COMPAT, "find window id:%{public}d failed", windowId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto windowType = window->GetType();
    if (WindowHelper::IsSubWindow(windowType)) {
        if (window->GetProperty()->GetIsUIExtensionAbilityProcess()) {
            TLOGD(WmsLogTag::WMS_COMPAT, "id:%{public}d extension sub window", windowId);
            return WMError::WM_OK;
        }
        window = window->FindMainWindowWithContext();
    }
    if (!window || !window->IsInCompatScaleStatus()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "window id:%{public}d not scale", windowId);
        return WMError::WM_OK;
    }
    Rect windowRect = window->GetRect();
    if (WindowHelper::IsUIExtensionWindow(windowType)) {
        windowRect = window->GetHostWindowRect(windowId);
    }
    float scaleX = window->compatScaleX_;
    float scaleY = window->compatScaleY_;
    int32_t cursorX = cursorInfo.left - windowRect.posX_;
    int32_t cursorY = cursorInfo.top - windowRect.posY_;
    // 2: x scale computational formula
    cursorInfo.left = round(windowRect.posX_ + scaleX * cursorX);
    // 2: y scale computational formula
    cursorInfo.top = round(windowRect.posY_ + scaleY * cursorY);
    cursorInfo.width *= scaleX;
    cursorInfo.height *= scaleY;
    return WMError::WM_OK;
}

void WindowSessionImpl::RegisterWindowScaleCallback()
{
    static bool isRegister = false;
    if (isRegister) {
        TLOGD(WmsLogTag::WMS_COMPAT, "not registered");
        return;
    }
#ifdef IMF_ENABLE
    auto instance = MiscServices::InputMethodController::GetInstance();
    if (!instance) {
        TLOGD(WmsLogTag::WMS_COMPAT, "get inputMethod instance failed");
        return;
    }
    auto callback = [] (uint32_t windowId, MiscServices::CursorInfo& cursorInfo) {
        CursorInfo info = { cursorInfo.left, cursorInfo.top, cursorInfo.width, cursorInfo.height };
        WMError ret = GetWindowScaleCoordinate(windowId, info);
        cursorInfo = { info.left, info.top, info.width, info.height };
        return static_cast<int32_t>(ret);
    };
    instance->RegisterWindowScaleCallbackHandler(std::move(callback));
#endif
    isRegister = true;
}

void WindowSessionImpl::ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    NotifyPointerEvent(pointerEvent);
}

void WindowSessionImpl::ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    bool isConsumed = false;
    NotifyKeyEvent(keyEvent, isConsumed, false);
}

void WindowSessionImpl::ConsumeBackEvent()
{
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    HandleBackEvent();
}

bool WindowSessionImpl::IsDialogSessionBackGestureEnabled()
{
    return dialogSessionBackGestureEnabled_;
}

bool WindowSessionImpl::PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TLOGI(WmsLogTag::WMS_EVENT, "id: %{public}d", keyEvent->GetId());
    if (auto uiContent = GetUIContentSharedPtr()) {
        return uiContent->ProcessKeyEvent(keyEvent, true);
    }
    return false;
}

bool WindowSessionImpl::NotifyOnKeyPreImeEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    return PreNotifyKeyEvent(keyEvent);
}

void WindowSessionImpl::GetSubWindows(int32_t parentPersistentId, std::vector<sptr<WindowSessionImpl>>& subWindows)
{
    std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
    auto iter = subWindowSessionMap_.find(parentPersistentId);
    if (iter == subWindowSessionMap_.end()) {
        TLOGD(WmsLogTag::WMS_SUB, "parent window: %{public}d has no child node", parentPersistentId);
        return;
    }
    subWindows = iter->second;
}

void WindowSessionImpl::UpdateSubWindowStateAndNotify(int32_t parentPersistentId, const WindowState newState)
{
    StateChangeOption option(parentPersistentId, newState);
    UpdateSubWindowStateWithOptions(option);
}

void WindowSessionImpl::UpdateSubWindowStateWithOptions(const StateChangeOption& option)
{
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(option.parentPersistentId_, subWindows);
    if (subWindows.empty()) {
        TLOGD(WmsLogTag::WMS_SUB, "parent window: %{public}d, its subWindowMap is empty", option.parentPersistentId_);
        return;
    }
    // when parent window hide and subwindow whose state is shown should hide and notify user
    if (option.newState_ == WindowState::STATE_HIDDEN) {
        for (auto subwindow : subWindows) {
            if (subwindow == nullptr || subwindow->GetWindowState() != WindowState::STATE_SHOWN) {
                continue;
            }
            // If the subwindow of uiextension follows the uiextension lifecycle, the subWindow will hide
            if (subwindow->followCreatorLifecycle_) {
                TLOGI(WmsLogTag::WMS_SUB, "subwindow hide follow uec, id: %{public}d", subwindow->GetPersistentId());
                subwindow->Hide(option.reason_, option.withAnimation_, option.isFromInnerkits_, option.waitDetach_);
                subwindow->SetIsHiddenFollowingUIExtension(true);
            } else {
                subwindow->state_ = WindowState::STATE_HIDDEN;
                subwindow->NotifyAfterBackground();
                TLOGD(WmsLogTag::WMS_SUB, "Notify subWindow background, id:%{public}d", subwindow->GetPersistentId());
                UpdateSubWindowStateAndNotify(subwindow->GetPersistentId(), option.newState_);
            }
        }
    // when parent window show and subwindow whose state is shown should show and notify user
    } else if (option.newState_ == WindowState::STATE_SHOWN) {
        for (auto subwindow : subWindows) {
            if (subwindow == nullptr || subwindow->GetWindowState() != WindowState::STATE_HIDDEN) {
                continue;
            }
            // If the subwindow has been hidden due to uiextension hiding, The subWindow will show
            if (subwindow->followCreatorLifecycle_ && subwindow->IsHiddenFollowingUIExtension()) {
                TLOGI(WmsLogTag::WMS_SUB, "subwindow show follow uec, id: %{public}d", subwindow->GetPersistentId());
                subwindow->Show(option.reason_, option.withAnimation_, option.withFocus_, option.waitAttach_);
                subwindow->SetIsHiddenFollowingUIExtension(false);
            } else if (subwindow->GetRequestWindowState() == WindowState::STATE_SHOWN) {
                subwindow->state_ = WindowState::STATE_SHOWN;
                subwindow->NotifyAfterForeground();
                TLOGD(WmsLogTag::WMS_SUB, "Notify subWindow foreground, id:%{public}d", subwindow->GetPersistentId());
                UpdateSubWindowStateAndNotify(subwindow->GetPersistentId(), option.newState_);
            }
        }
    }
}

WMError WindowSessionImpl::Show(uint32_t reason, bool withAnimation, bool withFocus)
{
    return Show(reason, withAnimation, withFocus, false);
}

WMError WindowSessionImpl::Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach)
{
    TLOGI(WmsLogTag::WMS_LIFE, "name:%{public}s, id:%{public}d, type:%{public}u, reason:%{public}u, state:%{public}u",
        property_->GetWindowName().c_str(), property_->GetPersistentId(), GetType(), reason, state_);
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (state_ == WindowState::STATE_SHOWN) {
        TLOGD(WmsLogTag::WMS_LIFE, "alreay shown [name:%{public}s, id:%{public}d, type: %{public}u]",
            property_->GetWindowName().c_str(), GetPersistentId(), property_->GetWindowType());
        NotifyAfterForeground(true, false);
        return WMError::WM_OK;
    }

    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    WSError ret = hostSession->Foreground(property_);
    // delete after replace WSError with WMError
    WMError res = static_cast<WMError>(ret);
    if (res == WMError::WM_OK) {
        StateChangeOption option(GetPersistentId(), WindowState::STATE_SHOWN, reason, withAnimation, withFocus,
            waitAttach, false, false);
        UpdateSubWindowStateWithOptions(option);
        state_ = WindowState::STATE_SHOWN;
        requestState_ = WindowState::STATE_SHOWN;
        NotifyAfterForeground();
    } else {
        NotifyForegroundFailed(res);
    }
    return res;
}

WMError WindowSessionImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    return Hide(reason, withAnimation, isFromInnerkits, false);
}

WMError WindowSessionImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d Hide, reason:%{public}u, state:%{public}u",
        GetPersistentId(), reason, state_);
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        TLOGD(WmsLogTag::WMS_LIFE, "alreay hidden [name:%{public}s, id:%{public}d, type: %{public}u]",
            property_->GetWindowName().c_str(), GetPersistentId(), property_->GetWindowType());
        NotifyBackgroundFailed(WMError::WM_DO_NOTHING);
        return WMError::WM_OK;
    }
    UpdateSubWindowStateAndNotify(GetPersistentId(), WindowState::STATE_HIDDEN);
    state_ = WindowState::STATE_HIDDEN;
    requestState_ = WindowState::STATE_HIDDEN;
    NotifyAfterBackground();
    return WMError::WM_OK;
}

void WindowSessionImpl::RemoveSubWindow(int32_t parentPersistentId)
{
    const int32_t persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_SUB, "Id: %{public}d, parentId: %{public}d", persistentId, parentPersistentId);
    std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
    auto subIter = subWindowSessionMap_.find(parentPersistentId);
    if (subIter == subWindowSessionMap_.end()) {
        return;
    }
    auto& subWindows = subIter->second;
    for (auto iter = subWindows.begin(); iter != subWindows.end(); iter++) {
        auto subWindow = *iter;
        if (subWindow != nullptr && subWindow->GetPersistentId() == persistentId) {
            TLOGD(WmsLogTag::WMS_SUB, "erase persistentId: %{public}d", persistentId);
            subWindows.erase(iter);
            break;
        } else {
            TLOGD(WmsLogTag::WMS_SUB, "Exists other sub window, persistentId: %{public}d", persistentId);
        }
    }
}

void WindowSessionImpl::DestroySubWindow()
{
    int32_t parentPersistentId = property_->GetParentPersistentId();
    const int32_t persistentId = GetPersistentId();
    if (property_->GetIsUIExtFirstSubWindow()) {
        auto extensionWindow = FindExtensionWindowWithContext();
        if (extensionWindow != nullptr) {
            parentPersistentId = extensionWindow->GetPersistentId();
        }
    }
    // remove from subWindowMap_ when destroy sub window
    RemoveSubWindow(parentPersistentId);
    {
        std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
        auto subIter = subWindowSessionMap_.find(parentPersistentId);
        if (subIter != subWindowSessionMap_.end() && property_->GetIsUIExtFirstSubWindow() &&
            subWindowSessionMap_.empty()) {
            auto extensionWindow = FindExtensionWindowWithContext();
            if (extensionWindow != nullptr && extensionWindow->GetUIContentSharedPtr() == nullptr) {
                extensionWindow->AddSetUIExtensionDestroyTimeoutCheck();
            }
        }
    }
    // remove from subWindowMap_ when destroy parent window
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(persistentId, subWindows);
    for (auto iter = subWindows.begin(); iter != subWindows.end(); iter = subWindows.begin()) {
        auto subWindow = *iter;
        if (subWindow == nullptr) {
            TLOGW(WmsLogTag::WMS_SUB, "Destroy sub window which is nullptr");
            subWindows.erase(iter);
            continue;
        }
        bool isExtDestroyed = subWindow->property_->GetIsUIExtFirstSubWindow();
        TLOGD(WmsLogTag::WMS_SUB, "Destroy sub window, persistentId: %{public}d, isExtDestroyed: %{public}d",
            subWindow->GetPersistentId(), isExtDestroyed);
        auto ret = subWindow->Destroy(isExtDestroyed);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SUB, "Destroy failed. persistentId: %{public}d", subWindow->GetPersistentId());
            subWindows.erase(iter);
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
        auto mainIter = subWindowSessionMap_.find(persistentId);
        if (mainIter != subWindowSessionMap_.end()) {
            mainIter->second.clear();
            subWindowSessionMap_.erase(mainIter);
        }
    }
}

WMError WindowSessionImpl::Destroy(bool needNotifyServer, bool needClearListener, uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d Destroy, state:%{public}u, needNotifyServer:%{public}d, "
        "needClearListener:%{public}d, reason:%{public}u", GetPersistentId(), state_, needNotifyServer,
        needClearListener, reason);
    if (IsWindowSessionInvalid()) {
        WLOGFW("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (auto hostSession = GetHostSession()) {
        hostSession->Disconnect();
    }
    NotifyBeforeDestroy(GetWindowName());
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        state_ = WindowState::STATE_DESTROYED;
        requestState_ = WindowState::STATE_DESTROYED;
    }
    DestroySubWindow();
    {
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = nullptr;
    }
    {
        std::unique_lock<std::shared_mutex> lock(windowSessionMutex_);
        windowSessionMap_.erase(property_->GetWindowName());
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
    return WMError::WM_OK;
}

WMError WindowSessionImpl::Destroy(uint32_t reason)
{
    return Destroy(true, true, reason);
}

WSError WindowSessionImpl::SetActive(bool active)
{
    WLOGFD("active status: %{public}d", active);
    if (active) {
        NotifyAfterActive();
    } else {
        NotifyAfterInactive();
    }
    return WSError::WS_OK;
}

/** @note @window.layout */
WSError WindowSessionImpl::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const SceneAnimationConfig& config, const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    // delete after replace ws_common.h with wm_common.h
    auto wmReason = static_cast<WindowSizeChangeReason>(reason);
    Rect wmRect = { rect.posX_, rect.posY_, rect.width_, rect.height_ };
    auto preRect = GetRect();
    if (preRect.width_ != wmRect.width_ || preRect.height_ != wmRect.height_) {
        windowSizeChanged_ = true;
    }
    property_->SetWindowRect(wmRect);
    property_->SetRequestRect(wmRect);

    TLOGI_LMT(TEN_SECONDS, RECORD_100_TIMES, WmsLogTag::WMS_LAYOUT,
        "id:%{public}d name:%{public}s rect:%{public}s->%{public}s reason:%{public}u displayId:%{public}"
        PRIu64, GetPersistentId(), GetWindowName().c_str(), preRect.ToString().c_str(), rect.ToString().c_str(),
        wmReason, property_->GetDisplayId());
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "WindowSessionImpl::UpdateRect id: %d [%d, %d, %u, %u] reason: %u hasRSTransaction: %u", GetPersistentId(),
        wmRect.posX_, wmRect.posY_, wmRect.width_, wmRect.height_, wmReason, config.rsTransaction_ != nullptr);
    if (handler_ != nullptr && (wmReason == WindowSizeChangeReason::ROTATION ||
        wmReason == WindowSizeChangeReason::SNAPSHOT_ROTATION)) {
        postTaskDone_ = false;
        UpdateRectForRotation(wmRect, preRect, wmReason, config, avoidAreas);
    } else if (handler_ != nullptr && wmReason == WindowSizeChangeReason::PAGE_ROTATION) {
        UpdateRectForPageRotation(wmRect, preRect, wmReason, config, avoidAreas);
    } else if (handler_ != nullptr && wmReason == WindowSizeChangeReason::SCENE_WITH_ANIMATION) {
        UpdateRectForResizeAnimation(wmRect, preRect, wmReason, config, avoidAreas);
    } else {
        UpdateRectForOtherReason(wmRect, preRect, wmReason, config.rsTransaction_, avoidAreas);
    }

    if (wmReason == WindowSizeChangeReason::MOVE || wmReason == WindowSizeChangeReason::RESIZE ||
        wmReason == WindowSizeChangeReason::MOVE_WITH_ANIMATION ||
        wmReason == WindowSizeChangeReason::RESIZE_WITH_ANIMATION) {
        layoutCallback_->OnUpdateSessionRect(wmRect, wmReason, GetPersistentId());
    }
    NotifyFirstValidLayoutUpdate(preRect, wmRect);
    return WSError::WS_OK;
}

/** @note @window.layout */
void WindowSessionImpl::UpdateVirtualPixelRatio(const sptr<Display>& display)
{
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "display is null when rotation!");
        return;
    }
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "displayInfo is null when rotation!");
        return;
    }
    virtualPixelRatio_ = GetVirtualPixelRatio(displayInfo);
    TLOGD(WmsLogTag::WMS_LAYOUT, "virtualPixelRatio: %{public}f", virtualPixelRatio_);
}

void WindowSessionImpl::UpdateRectForResizeAnimation(const Rect& wmRect, const Rect& preRect,
    WindowSizeChangeReason wmReason, const SceneAnimationConfig& config,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    handler_->PostTask([weak = wptr(this), wmReason, wmRect, preRect, config, avoidAreas]() mutable {
        HITRACE_METER_NAME(HITRACE_TAG_WINDOW_MANAGER, "WindowSessionImpl::UpdateRectForResizeAnimation");
        auto window = weak.promote();
        if (!window) {
            return;
        }
        auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window->property_->GetDisplayId());
        sptr<DisplayInfo> displayInfo = display ? display->GetDisplayInfo() : nullptr;
        auto rsUIContext = window->GetRSUIContext();
        std::array<float, ANIMATION_PARAM_SIZE> param = config.animationParam_;
        RSAnimationTimingCurve curve = window->updateConfigCurve(config.animationCurve_, param);
        std::shared_ptr<RSTransaction> rsTransaction = config.rsTransaction_;
        if (rsTransaction) {
            RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
            rsTransaction->Begin();
        }

        RSAnimationTimingProtocol protocol;
        protocol.SetDuration(config.animationDuration_);
        protocol.SetStartDelay(config.animationDelay_);
        RSNode::OpenImplicitAnimation(rsUIContext, protocol, curve, nullptr);
        if ((wmRect != preRect) || (wmReason != window->lastSizeChangeReason_)) {
            window->NotifySizeChange(wmRect, wmReason);
            window->lastSizeChangeReason_ = wmReason;
        }
        window->UpdateViewportConfig(wmRect, wmReason, rsTransaction, displayInfo, avoidAreas);
        RSNode::CloseImplicitAnimation(rsUIContext);
        if (rsTransaction) {
            rsTransaction->Commit();
        } else {
            RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
        }
        window->postTaskDone_ = true;
    }, "WMS_WindowSessionImpl_UpdateRectForResizeAnimation");
}

RSAnimationTimingCurve WindowSessionImpl::updateConfigCurve(const WindowAnimationCurve& configCurve,
    const std::array<float, ANIMATION_PARAM_SIZE> param)
{
    RSAnimationTimingCurve curve;
    const float paramFirst = param[0]; // 0: the first parameter of the passed animation curve.
    const float paramSecond = param[1]; // 1: the second parameter of the passed animation curve.
    const float paramThird = param[2]; // 2: the third parameter of the passed animation curve.
    const float paramFourth = param[3]; // 3: the fourth parameter of the passed animation curve.
    switch (configCurve) {
        case WindowAnimationCurve::LINEAR:
            TLOGD(WmsLogTag::WMS_LAYOUT, "Linear params");
            curve = RSAnimationTimingCurve::LINEAR;
            break;
        case WindowAnimationCurve::INTERPOLATION_SPRING:
            TLOGD(WmsLogTag::WMS_LAYOUT, "Spring params: %{public}f, %{public}f, %{public}f, %{public}f",
                paramFirst, paramSecond, paramThird, paramFourth);
            curve = RSAnimationTimingCurve::CreateInterpolatingSpring(paramFirst,
                paramSecond, paramThird, paramFourth);
            break;
        case WindowAnimationCurve::CUBIC_BEZIER:
            TLOGD(WmsLogTag::WMS_LAYOUT, "Bezier params: %{public}f, %{public}f, %{public}f, %{public}f",
                paramFirst, paramSecond, paramThird, paramFourth);
            curve = RSAnimationTimingCurve::CreateCubicCurve(paramFirst, paramSecond,
                paramThird, paramFourth);
            break;
        default:
            TLOGW(WmsLogTag::WMS_LAYOUT, "Unknown curve type: %{public}d, using LINEAR as default",
                static_cast<int>(configCurve));
            curve = RSAnimationTimingCurve::LINEAR;
    }
    return curve;
}

void WindowSessionImpl::UpdateRectForRotation(const Rect& wmRect, const Rect& preRect,
    WindowSizeChangeReason wmReason, const SceneAnimationConfig& config,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    handler_->PostTask([weak = wptr(this), wmReason, wmRect, preRect, config, avoidAreas]() mutable {
        HITRACE_METER_NAME(HITRACE_TAG_WINDOW_MANAGER, "WindowSessionImpl::UpdateRectForRotation");
        auto window = weak.promote();
        if (!window) {
            return;
        }
        auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window->property_->GetDisplayId());
        sptr<DisplayInfo> displayInfo = display ? display->GetDisplayInfo() : nullptr;
        window->UpdateVirtualPixelRatio(display);
        auto rsUIContext = window->GetRSUIContext();
        const std::shared_ptr<RSTransaction>& rsTransaction = config.rsTransaction_;
        window->BeginRSTransaction(rsTransaction);
        window->rotationAnimationCount_++;
        RSAnimationTimingProtocol protocol;
        protocol.SetDuration(config.animationDuration_);
        // animation curve: cubic [0.2, 0.0, 0.2, 1.0]
        auto curve = RSAnimationTimingCurve::CreateCubicCurve(0.2, 0.0, 0.2, 1.0);
        RSNode::OpenImplicitAnimation(rsUIContext, protocol, curve, [weak]() {
            auto window = weak.promote();
            if (!window) {
                return;
            }
            window->rotationAnimationCount_--;
            if (window->rotationAnimationCount_ == 0) {
                window->NotifyRotationAnimationEnd();
            }
        });
        if (wmReason == WindowSizeChangeReason::SNAPSHOT_ROTATION) {
            wmReason = WindowSizeChangeReason::ROTATION;
        }
        if ((wmRect != preRect) || (wmReason != window->lastSizeChangeReason_)) {
            window->NotifySizeChange(wmRect, wmReason);
            window->lastSizeChangeReason_ = wmReason;
        }
        window->UpdateViewportConfig(wmRect, wmReason, rsTransaction, displayInfo, avoidAreas);
        RSNode::CloseImplicitAnimation(rsUIContext);
        if (rsTransaction) {
            rsTransaction->Commit();
        } else {
            RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
        }
        window->postTaskDone_ = true;
    }, "WMS_WindowSessionImpl_UpdateRectForRotation");
}


void WindowSessionImpl::UpdateRectForPageRotation(const Rect& wmRect, const Rect& preRect,
    WindowSizeChangeReason wmReason, const SceneAnimationConfig& config,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    handler_->PostTask(
        [weak = wptr(this), wmReason, wmRect, preRect, config, avoidAreas]() mutable {
            HITRACE_METER_NAME(HITRACE_TAG_WINDOW_MANAGER, "WindowSessionImpl::UpdateRectForPageRotation");
            auto window = weak.promote();
            if (!window) {
                return;
            }
            auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window->property_->GetDisplayId());
            if (display == nullptr) {
                TLOGE(WmsLogTag::WMS_ROTATION, "display is null!");
                return;
            }
            sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
            if (displayInfo == nullptr) {
                TLOGE(WmsLogTag::WMS_ROTATION, "displayInfo is null!");
                return;
            }
            window->UpdateVirtualPixelRatio(display);
            const std::shared_ptr<RSTransaction>& rsTransaction = config.rsTransaction_;
            window->BeginRSTransaction(rsTransaction);
            if ((wmRect != preRect) || (wmReason != window->lastSizeChangeReason_)) {
                window->NotifySizeChange(wmRect, wmReason);
                window->lastSizeChangeReason_ = wmReason;
            }
            window->NotifyClientOrientationChange();
            DisplayOrientation windowOrientation = window->GetCurrentWindowOrientation();
            DisplayOrientation displayOrientation = displayInfo->GetDisplayOrientation();
            TLOGI(WmsLogTag::WMS_ROTATION,
                "windowOrientation: %{public}d, targetdisplayOrientation: %{public}d, "
                "windowRect: %{public}s, preRect: %{public}s, id: %{public}d",
                static_cast<int32_t>(windowOrientation), static_cast<int32_t>(displayOrientation),
                wmRect.ToString().c_str(), preRect.ToString().c_str(), window->GetPersistentId());
            if (wmRect != preRect || windowOrientation != displayOrientation) {
                window->UpdateViewportConfig(wmRect, wmReason, rsTransaction, displayInfo, avoidAreas);
            }
            if (rsTransaction) {
                rsTransaction->Commit();
            } else {
                RSTransactionAdapter::FlushImplicitTransaction(window->GetRSUIContext());
            }
            window->postTaskDone_ = true;
        },
        "WMS_WindowSessionImpl_UpdateRectForPageRotation");
}

void WindowSessionImpl::BeginRSTransaction(const std::shared_ptr<RSTransaction>& rsTransaction) const
{
    if (!rsTransaction) {
        TLOGE(WmsLogTag::WMS_ROTATION, "rsTransaction is null");
        return;
    }
    auto rsUIContext = GetRSUIContext();
    if (rsUIContext) {
        rsTransaction->SetTransactionHandler(rsUIContext->GetRSTransaction());
    }
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
    rsTransaction->Begin();
    TLOGI(WmsLogTag::WMS_ROTATION, "rsTransaction begin");
}

void WindowSessionImpl::UpdateCurrentWindowOrientation(DisplayOrientation displayOrientation)
{
    windowOrientation_ = displayOrientation;
}

DisplayOrientation WindowSessionImpl::GetCurrentWindowOrientation() const
{
    return windowOrientation_;
}

void WindowSessionImpl::UpdateRectForOtherReasonTask(const Rect& wmRect, const Rect& preRect,
    WindowSizeChangeReason wmReason, const std::shared_ptr<RSTransaction>& rsTransaction,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if ((wmRect != preRect) || (wmReason != lastSizeChangeReason_)
        || !postTaskDone_ || notifySizeChangeFlag_) {
        NotifySizeChange(wmRect, wmReason);
        SetNotifySizeChangeFlag(false);
        lastSizeChangeReason_ = wmReason;
        postTaskDone_ = true;
    }
    UpdateViewportConfig(wmRect, wmReason, rsTransaction, nullptr, avoidAreas);
    UpdateFrameLayoutCallbackIfNeeded(wmReason);
}

bool WindowSessionImpl::CheckIfNeedCommitRsTransaction(WindowSizeChangeReason wmReason)
{
    if (wmReason == WindowSizeChangeReason::FULL_TO_SPLIT ||
        wmReason == WindowSizeChangeReason::FULL_TO_FLOATING || wmReason == WindowSizeChangeReason::RECOVER ||
        wmReason == WindowSizeChangeReason::MAXIMIZE) {
        return false;
    }
    return true;
}

void WindowSessionImpl::UpdateRectForOtherReason(const Rect& wmRect, const Rect& preRect,
    WindowSizeChangeReason wmReason, const std::shared_ptr<RSTransaction>& rsTransaction,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if (handler_ == nullptr) {
        UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction, avoidAreas);
        return;
    }

    auto task = [weak = wptr(this), wmReason, wmRect, preRect, rsTransaction, avoidAreas] {
        auto window = weak.promote();
        if (!window) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "window is null, updateViewPortConfig failed");
            return;
        }
        bool ifNeedCommitRsTransaction = window->CheckIfNeedCommitRsTransaction(wmReason);
        if (rsTransaction && ifNeedCommitRsTransaction) {
            RSTransactionAdapter::FlushImplicitTransaction(window->GetRSUIContext());
            rsTransaction->Begin();
        }
        if (wmReason == WindowSizeChangeReason::DRAG) {
            window->UpdateRectForOtherReasonTask(window->GetRect(), preRect, wmReason, rsTransaction, avoidAreas);
            window->isDragTaskPostDone_.store(true);
        } else {
            window->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction, avoidAreas);
        }
        if (rsTransaction && ifNeedCommitRsTransaction) {
            rsTransaction->Commit();
        }
    };
    if (wmReason == WindowSizeChangeReason::DRAG) {
        bool isDragTaskPostDone = true;
        if (isDragTaskPostDone_.compare_exchange_strong(isDragTaskPostDone, false)) {
            handler_->PostTask(task, "WMS_WindowSessionImpl_UpdateRectForOtherReason");
        }
    } else {
        handler_->PostTask(task, "WMS_WindowSessionImpl_UpdateRectForOtherReason");
    }
}

void WindowSessionImpl::NotifyAfterUIContentReady()
{
    auto uiContent = GetUIContentSharedPtr();
    CHECK_UI_CONTENT_RETURN_IF_NULL(uiContent);
    if (IsNeedRenotifyTransform()) {
        auto transform = GetCurrentTransform();
        TLOGI(WmsLogTag::WMS_LAYOUT, "Renotify transform, id:%{public}d, scaleX:%{public}f, scaleY:%{public}f",
            GetPersistentId(), transform.scaleX_, transform.scaleY_);
        uiContent->UpdateTransform(transform);
        SetNeedRenotifyTransform(false);
    }
}

void WindowSessionImpl::NotifyRotationAnimationEnd()
{
    auto task = [weak = wptr(this)] {
        auto window = weak.promote();
        if (!window) {
            TLOGE(WmsLogTag::WMS_DIALOG, "window is null");
            return;
        }
        std::shared_ptr<Ace::UIContent> uiContent = window->GetUIContentSharedPtr();
        if (uiContent == nullptr) {
            WLOGFW("uiContent is null!");
            return;
        }
        uiContent->NotifyRotationAnimationEnd();
    };
    if (handler_ == nullptr) {
        TLOGW(WmsLogTag::WMS_DIALOG, "handler is null!");
        task();
    } else {
        handler_->PostTask(task, "WMS_WindowSessionImpl_NotifyRotationAnimationEnd");
    }
}

void WindowSessionImpl::FlushLayoutSize(int32_t width, int32_t height)
{
    if (!WindowHelper::IsMainWindow(GetType())) {
        return;
    }
    WSRect rect = { 0, 0, width, height };
    bool windowSizeChanged = true;
    bool enableFrameLayoutFinishCb = true;
    if (windowSizeChanged_.compare_exchange_strong(windowSizeChanged, false) ||
        enableFrameLayoutFinishCb_.compare_exchange_strong(enableFrameLayoutFinishCb, false) || layoutRect_ != rect) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
            "NotifyFrameLayoutFinishFromApp, id: %u, rect: %s, notifyListener: %d",
            GetWindowId(), rect.ToString().c_str(), enableFrameLayoutFinishCb_.load());
        TLOGI(WmsLogTag::WMS_PATTERN,
            "NotifyFrameLayoutFinishFromApp, id: %{public}u, rect: %{public}s, notifyListener: %{public}d",
            GetWindowId(), rect.ToString().c_str(), enableFrameLayoutFinishCb_.load());
        if (auto session = GetHostSession()) {
            session->NotifyFrameLayoutFinishFromApp(enableFrameLayoutFinishCb_, rect);
        }
        layoutRect_ = rect;
        enableFrameLayoutFinishCb_ = false;
    }
}

void WindowSessionImpl::FlushVsync()
{
    if (!WindowHelper::IsMainWindow(GetType())) {
        return;
    }

    if (!property_->IsPrelaunch()) {
        TLOGD(WmsLogTag::WMS_PATTERN, "prelaunch is false");
        return;
    }

    if (property_->GetFrameNum() == 0) {
        TLOGD(WmsLogTag::WMS_PATTERN, "prelaunch frameNum is 0");
        return;
    }

    if (hasNotifyPrelaunchStartingwindow_) {
        TLOGD(WmsLogTag::WMS_PATTERN, "prelaunch startingwindow has notify");
        return;
    }

    vsyncCount_++;
    TLOGI(WmsLogTag::WMS_PATTERN, "prelaunch frameNum:%{public}d, vsyncCount:%{public}d",
        property_->GetFrameNum(), vsyncCount_);

    if (vsyncCount_ >= property_->GetFrameNum()) {
        if (auto session = GetHostSession()) {
            session->NotifyRemovePrelaunchStartingWindow();
            hasNotifyPrelaunchStartingwindow_ = true;
        }
    }
}

WMError WindowSessionImpl::NotifySnapshotUpdate()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "NotifySnapshotUpdate, id: %u", GetWindowId());
    TLOGI(WmsLogTag::WMS_PATTERN, "id: %{public}u", GetWindowId());
    if (auto session = GetHostSession()) {
        return session->NotifySnapshotUpdate();
    }
    TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr");
    return WMError::WM_ERROR_INVALID_SESSION;
}

void WindowSessionImpl::GetTitleButtonVisible(bool& hideMaximizeButton, bool& hideMinimizeButton,
    bool& hideSplitButton, bool& hideCloseButton)
{
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_DECOR, "device not support");
        return;
    }
    if (hideMaximizeButton > !windowTitleVisibleFlags_.isMaximizeVisible) {
        TLOGW(WmsLogTag::WMS_DECOR, "isMaximizeVisible param INVALID");
    }
    hideMaximizeButton = hideMaximizeButton || (!windowTitleVisibleFlags_.isMaximizeVisible);
    if (hideMinimizeButton > !windowTitleVisibleFlags_.isMinimizeVisible) {
        TLOGW(WmsLogTag::WMS_DECOR, "isMinimizeVisible param INVALID");
    }
    hideMinimizeButton = hideMinimizeButton || (!windowTitleVisibleFlags_.isMinimizeVisible);
    if (hideSplitButton > !windowTitleVisibleFlags_.isSplitVisible) {
        TLOGW(WmsLogTag::WMS_DECOR, "isSplitVisible param INVALID");
    }
    hideSplitButton = hideSplitButton || (!windowTitleVisibleFlags_.isSplitVisible) || !isSplitButtonVisible_;
    if (hideCloseButton > !windowTitleVisibleFlags_.isCloseVisible) {
        TLOGW(WmsLogTag::WMS_DECOR, "isCloseVisible param INVALID");
    }
    hideCloseButton = hideCloseButton || (!windowTitleVisibleFlags_.isCloseVisible);
}

void WindowSessionImpl::UpdateDensity()
{
    auto preRect = GetRect();
    UpdateViewportConfig(preRect, WindowSizeChangeReason::UNDEFINED);
    WLOGFI("[%{public}d, %{public}d, %{public}u, %{public}u]",
        preRect.posX_, preRect.posY_, preRect.width_, preRect.height_);
}

void WindowSessionImpl::SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "old {useUniqueDensity: %{public}d, virtualPixelRatio: %{public}f}, "
        "new {useUniqueDensity: %{public}d, virtualPixelRatio: %{public}f}",
        useUniqueDensity_, virtualPixelRatio_, useUniqueDensity, virtualPixelRatio);
    bool oldUseUniqueDensity = useUniqueDensity_;
    useUniqueDensity_ = useUniqueDensity;
    if (useUniqueDensity_) {
        float oldVirtualPixelRatio = virtualPixelRatio_;
        virtualPixelRatio_ = virtualPixelRatio;
        if (!MathHelper::NearZero(oldVirtualPixelRatio - virtualPixelRatio)) {
            UpdateDensity();
            SetUniqueVirtualPixelRatioForSub(useUniqueDensity, virtualPixelRatio);
        }
    } else {
        if (oldUseUniqueDensity) {
            UpdateDensity();
            SetUniqueVirtualPixelRatioForSub(useUniqueDensity, virtualPixelRatio);
        }
    }
}

void WindowSessionImpl::UpdateAnimationSpeed(float speed)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), speed, where] {
        auto window = weakThis.promote();
        if (window == nullptr) {
            TLOGW(WmsLogTag::WMS_ANIMATION, "%{public}s: window is nullptr", where);
            return;
        }
        window->UpdateAllWindowSpeed(speed);
        isEnableAnimationSpeed_.store(!FoldScreenStateInternel::FloatEqualAbs(speed, 1.0f));
        animationSpeed_.store(speed);
    };
    handler_->PostTask(task, where, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void WindowSessionImpl::UpdateAllWindowSpeed(float speed)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& [_, pair] : windowSessionMap_) {
        auto& WindowSession = pair.second;
        if (!WindowSession) {
            continue;
        }
        auto rsUIContext = WindowSession->GetRSUIContext();
        auto implicitAnimator = rsUIContext ? rsUIContext->GetRSImplicitAnimator() : nullptr;
        if (implicitAnimator == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to open implicit animation");
            continue;
        }
        implicitAnimator->ApplyAnimationSpeedMultiplier(speed);
    }
}

void WindowSessionImpl::CopyUniqueDensityParameter(sptr<WindowSessionImpl> parentWindow)
{
    if (parentWindow) {
        useUniqueDensity_ = parentWindow->useUniqueDensity_;
        virtualPixelRatio_ = parentWindow->virtualPixelRatio_;
    }
}

sptr<WindowSessionImpl> WindowSessionImpl::FindMainWindowWithContext() const
{
    auto context = GetContext();
    if (context == nullptr) {
        return nullptr;
    }
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& winPair : windowSessionMap_) {
        auto win = winPair.second.second;
        if (win && win->GetType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
            context.get() == win->GetContext().get()) {
            return win;
        }
    }
    TLOGD(WmsLogTag::WMS_MAIN, "Can not find main window, not app type");
    return nullptr;
}

sptr<WindowSessionImpl> WindowSessionImpl::FindExtensionWindowWithContext() const
{
    auto context = GetContext();
    if (context == nullptr) {
        return nullptr;
    }
    std::shared_lock<std::shared_mutex> lock(windowExtensionSessionMutex_);
    for (const auto& window : GetWindowExtensionSessionSet()) {
        if (window && context.get() == window->GetContext().get()) {
            return window;
        }
    }
    return nullptr;
}

sptr<WindowSessionProperty> WindowSessionImpl::GetPropertyByContext() const
{
    if (!WindowHelper::IsSubWindow(GetType())) {
       return property_;
    }
    if (property_->GetIsUIExtensionAbilityProcess()) {
        auto extensionWindow = FindExtensionWindowWithContext();
        return extensionWindow != nullptr ? extensionWindow->GetProperty() : property_;
    }
    auto mainWindow = FindMainWindowWithContext();
    return mainWindow != nullptr ? mainWindow->GetProperty() : property_;
}

void WindowSessionImpl::SetUniqueVirtualPixelRatioForSub(bool useUniqueDensity, float virtualPixelRatio)
{
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(GetPersistentId(), subWindows);
    for (auto& subWindowSession : subWindows) {
        if (subWindowSession != nullptr) {
            subWindowSession->SetUniqueVirtualPixelRatio(useUniqueDensity, virtualPixelRatio);
        }
    }
}

WSError WindowSessionImpl::UpdateOrientation()
{
    TLOGD(WmsLogTag::DMS, "wid: %{public}d", GetPersistentId());
    return WSError::WS_OK;
}

WSError WindowSessionImpl::UpdateDisplayId(uint64_t displayId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "wid: %{public}d, displayId: %{public}" PRIu64, GetPersistentId(), displayId);
    property_->SetDisplayId(displayId);
    return WSError::WS_OK;
}

WSError WindowSessionImpl::UpdateFocus(const sptr<FocusNotifyInfo>& focusNotifyInfo, bool isFocused)
{
    if (focusNotifyInfo == nullptr || !focusNotifyInfo->isSyncNotify_) {
        UpdateFocusState(isFocused);
        return WSError::WS_OK;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "unfocusId:%{public}d, focusId:%{public}d, isFocused:%{public}d,"
        "isSyncNotify:%{public}d, current:%{public}" PRId64 ", new:%{public}" PRId64, focusNotifyInfo->unfocusWindowId_,
        focusNotifyInfo->focusWindowId_, isFocused, focusNotifyInfo->isSyncNotify_, updateFocusTimeStamp_.load(),
        focusNotifyInfo->timeStamp_);
    auto timeStamp = focusNotifyInfo->timeStamp_;
    if (timeStamp <= updateFocusTimeStamp_.load()) {
        return WSError::WS_OK;
    }
    updateFocusTimeStamp_.store(timeStamp);
    auto otherWindowId = isFocused ? focusNotifyInfo->unfocusWindowId_ : focusNotifyInfo->focusWindowId_;
    if (otherWindowId == INVALID_SESSION_ID) {
        UpdateFocusState(isFocused);
        return WSError::WS_OK;
    }
    auto otherWindow = GetWindowWithId(otherWindowId);
    if (isFocused) {
        if (otherWindow != nullptr) {
            otherWindow->UpdateFocusState(!isFocused);
        }
        UpdateFocusState(isFocused);
    } else {
        UpdateFocusState(isFocused);
        if (otherWindow != nullptr) {
            otherWindow->UpdateFocusState(!isFocused);
        }
    }
    return WSError::WS_OK;
}

void WindowSessionImpl::UpdateFocusState(bool isFocused)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "focus: %{public}u, id: %{public}d", isFocused, GetPersistentId());
    isFocused_ = isFocused;
    if (isFocused) {
        std::string bundleName = IsAnco() && property_->GetAncoRealBundleName() != "" ?
            property_->GetAncoRealBundleName() : property_->GetSessionInfo().bundleName_;
        HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "FOCUS_WINDOW",
            OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "PID", getpid(),
            "UID", getuid(),
            "BUNDLE_NAME", bundleName,
            "WINDOW_TYPE", static_cast<uint32_t>(GetType()));
        NotifyAfterFocused();
    } else {
        NotifyAfterUnfocused();
    }
}

bool WindowSessionImpl::IsFocused() const
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Session is invalid");
        return false;
    }

    TLOGD(WmsLogTag::WMS_FOCUS, "window id=%{public}d, isFocused=%{public}d", GetPersistentId(), isFocused_.load());
    return isFocused_;
}

WMError WindowSessionImpl::RequestFocus() const
{
    if (IsWindowSessionInvalid()) {
        WLOGFD("session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().RequestFocusStatus(GetPersistentId(), true);
}

/** @note @window.focus */
WMError WindowSessionImpl::RequestFocusByClient(bool isFocused) const
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (IsWindowSessionInvalid()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto ret = hostSession->RequestFocus(isFocused);
    return static_cast<WMError>(ret);
}

bool WindowSessionImpl::IsNotifyInteractiveDuplicative(bool interactive)
{
    if (interactive == interactive_ && hasFirstNotifyInteractive_) {
        return true;
    }
    hasFirstNotifyInteractive_ = true;
    if (interactive_ != interactive) {
        interactive_ = interactive;
    }
    return false;
}

void WindowSessionImpl::NotifyForegroundInteractiveStatus(bool interactive)
{
    TLOGI(WmsLogTag::WMS_LIFE, "interactive: %{public}d, state: %{public}d", interactive, state_);
    if (IsWindowSessionInvalid() || state_ != WindowState::STATE_SHOWN) {
        return;
    }
    if (IsNotifyInteractiveDuplicative(interactive)) {
        return;
    }
    if (interactive) {
        bool useControlState = property_->GetUseControlState();
        TLOGI(WmsLogTag::WMS_LIFE, "useControlState: %{public}d, persistentId: %{public}d", useControlState,
            GetPersistentId());
        if (!useControlState) {
            NotifyAfterLifecycleResumed();
        }
        NotifyAfterResumed();
    } else {
        NotifyAfterPaused();
        NotifyAfterLifecyclePaused();
    }
}

void WindowSessionImpl::NotifyAppUseControlStatus(bool isUseControl)
{
    {
        std::lock_guard<std::mutex> lock(appUseControlMutex_);
        isAppUseControl_ = isUseControl;
    }
    property_->SetUseControlState(isUseControl);
    TLOGI(WmsLogTag::WMS_LIFE, "isUseControl: %{public}d, state_: %{public}d, persistentId: %{public}d",
        isUseControl, state_, GetPersistentId());
    if (IsWindowSessionInvalid() || state_ != WindowState::STATE_SHOWN) {
        return;
    }
    if (isUseControl) {
        NotifyAfterLifecyclePaused();
    } else {
        NotifyAfterLifecycleResumed();
    }
}

void WindowSessionImpl::NotifyLifecyclePausedStatus()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (IsWindowSessionInvalid() || state_ != WindowState::STATE_SHOWN) {
        return;
    }
    NotifyAfterLifecyclePaused();
}

WSError WindowSessionImpl::UpdateWindowMode(WindowMode mode)
{
    return WSError::WS_OK;
}

/** @note @window.layout */
float WindowSessionImpl::GetVirtualPixelRatio()
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "virtualPixelRatio: %{public}f", virtualPixelRatio_);
    return virtualPixelRatio_;
}

float WindowSessionImpl::GetVirtualPixelRatio(const sptr<DisplayInfo>& displayInfo)
{
    if (useUniqueDensity_) {
        return virtualPixelRatio_;
    }
    return displayInfo->GetVirtualPixelRatio();
}

WMError WindowSessionImpl::GetVirtualPixelRatio(float& vpr)
{
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "get display failed displayId: %{public}" PRIu64, property_->GetDisplayId());
        return WMError::WM_ERROR_NULLPTR;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "get display info failed displayId: %{public}" PRIu64, property_->GetDisplayId());
        return WMError::WM_ERROR_NULLPTR;
    }
    vpr = GetVirtualPixelRatio(displayInfo);
    if (MathHelper::NearZero(vpr)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "get decor height failed, because of wrong vpr: %{public}f", vpr);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return WMError::WM_OK;
}

/** @note @window.layout */
void WindowSessionImpl::UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction, const sptr<DisplayInfo>& info,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    // update avoid areas to listeners
    if (reason != WindowSizeChangeReason::OCCUPIED_AREA_CHANGE) {
        for (const auto& [type, avoidArea] : avoidAreas) {
            TLOGD(WmsLogTag::WMS_IMMS, "avoid type %{public}u area %{public}s",
                type, avoidArea.ToString().c_str());
            if ((lastAvoidAreaMap_.find(type) == lastAvoidAreaMap_.end() && type != AvoidAreaType::TYPE_CUTOUT) ||
                lastAvoidAreaMap_[type] != avoidArea) {
                lastAvoidAreaMap_[type] = avoidArea;
                NotifyAvoidAreaChange(new AvoidArea(avoidArea), type);
            }
        }
    }

    sptr<DisplayInfo> displayInfo;
    if (info == nullptr) {
        auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
        if (display == nullptr) {
            WLOGFE("display is null!");
            return;
        }
        displayInfo = display->GetDisplayInfo();
    } else {
        displayInfo = info;
    }
    if (displayInfo == nullptr) {
        WLOGFE("displayInfo is null!");
        return;
    }
    if (rect.width_ <= 0 || rect.height_ <= 0) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "invalid width: %{public}d, height: %{public}d, id: %{public}d",
              rect.width_, rect.height_, GetPersistentId());
        return;
    }
    auto rotation =  ONE_FOURTH_FULL_CIRCLE_DEGREE * static_cast<uint32_t>(displayInfo->GetOriginRotation());
    auto deviceRotation = static_cast<uint32_t>(displayInfo->GetDefaultDeviceRotationOffset());
    uint32_t transformHint = (rotation + deviceRotation) % FULL_CIRCLE_DEGREE;
    float density = GetVirtualPixelRatio(displayInfo);
    UpdateCurrentWindowOrientation(displayInfo->GetDisplayOrientation());
    int32_t orientation = static_cast<int32_t>(displayInfo->GetDisplayOrientation());
    virtualPixelRatio_ = density;
    auto config = FillViewportConfig(rect, density, orientation, transformHint, GetDisplayId());
    if (reason == WindowSizeChangeReason::DRAG_END && keyFramePolicy_.stopping_) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "key frame stop");
        keyFramePolicy_.stopping_ = false;
        config.SetKeyFrameConfig(true, keyFramePolicy_.animationDuration_, keyFramePolicy_.animationDelay_);
    } else {
        config.SetKeyFrameConfig(keyFramePolicy_.running_, keyFramePolicy_.animationDuration_,
            keyFramePolicy_.animationDelay_);
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        WLOGFW("uiContent is null!");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "WindowSessionimpl::UpdateViewportConfig id:%d [%d, %d, %u, %u] reason:%u orientation:%d", GetPersistentId(),
        rect.posX_, rect.posY_, rect.width_, rect.height_, reason, orientation);
    if (reason == WindowSizeChangeReason::OCCUPIED_AREA_CHANGE && !avoidAreas.empty()) {
        uiContent->UpdateViewportConfig(config, reason, rsTransaction, avoidAreas, occupiedAreaInfo_);
    } else {
        uiContent->UpdateViewportConfig(config, reason, rsTransaction, lastAvoidAreaMap_, occupiedAreaInfo_);
    }

    if (WindowHelper::IsUIExtensionWindow(GetType())) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id: %{public}d, reason: %{public}d, windowRect: %{public}s, "
            "displayOrientation: %{public}d, config[%{public}u, %{public}u, %{public}u, "
            "%{public}f]", GetPersistentId(), reason, rect.ToString().c_str(), orientation,
            rotation, deviceRotation, transformHint, virtualPixelRatio_);
    } else {
        TLOGI_LMT(TEN_SECONDS, RECORD_100_TIMES, WmsLogTag::WMS_LAYOUT,
            "Id: %{public}d, reason: %{public}d, windowRect: %{public}s, displayOrientation: %{public}d, "
            "config[%{public}u, %{public}u, %{public}u, %{public}f]", GetPersistentId(), reason,
            rect.ToString().c_str(), orientation, rotation, deviceRotation, transformHint, virtualPixelRatio_);
    }
}

int32_t WindowSessionImpl::GetFloatingWindowParentId()
{
    auto context = GetContext();
    if (context.get() == nullptr) {
        return INVALID_SESSION_ID;
    }
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    for (const auto& winPair : windowSessionMap_) {
        if (winPair.second.second && WindowHelper::IsMainWindow(winPair.second.second->GetType()) &&
            context.get() == winPair.second.second->GetContext().get()) {
            WLOGFD("Find parent, [parentName: %{public}s, selfPersistentId: %{public}d]",
                winPair.second.second->GetProperty()->GetWindowName().c_str(), GetPersistentId());
            return winPair.second.second->GetProperty()->GetPersistentId();
        }
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(GetContext());
    if (abilityContext != nullptr) {
        int32_t parentId = INVALID_SESSION_ID;
        abilityContext->GetMissionId(parentId);
        TLOGI(WmsLogTag::WMS_LIFE, "parentId: %{public}d", parentId);
        return parentId;
    }
    return INVALID_SESSION_ID;
}

Rect WindowSessionImpl::GetRect() const
{
    return property_->GetWindowRect();
}

void WindowSessionImpl::UpdateTitleButtonVisibility()
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr || !IsDecorEnable()) {
        return;
    }
    WindowType windowType = GetType();
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    if (IsPcOrFreeMultiWindowCapabilityEnabled() && (isSubWindow || isDialogWindow)) {
        uiContent->HideWindowTitleButton(true, !windowOption_->GetSubWindowMaximizeSupported(), true, false);
        return;
    }
    auto windowModeSupportType = property_->GetWindowModeSupportType();
    bool hideSplitButton = !(windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY);
    // not support fullscreen in split and floating mode, or not support float in fullscreen mode
    bool hideMaximizeButton = (!(windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN) &&
        (GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING || WindowHelper::IsSplitWindowMode(GetWindowMode()))) ||
        (!(windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING) &&
        (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN || WindowHelper::IsSplitWindowMode(GetWindowMode())));
    bool hideMinimizeButton = false;
    bool hideCloseButton = false;
    GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    hideMaximizeButton = hideMaximizeButton && !grayOutMaximizeButton_;
    TLOGI(WmsLogTag::WMS_DECOR, "[hideSplit, hideMaximize, hideMinimizeButton, hideCloseButton]:"
        "[%{public}d, %{public}d, %{public}d, %{public}d]",
        hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);
    bool isSuperFoldDisplayDevice = FoldScreenStateInternel::IsSuperFoldDisplayDevice();
    HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);
    if (isSuperFoldDisplayDevice) {
        handler_->PostTask([weakThis = wptr(this), where = __func__] {
            auto window = weakThis.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is null", where);
                return;
            }
            std::shared_ptr<Ace::UIContent> uiContent = window->GetUIContentSharedPtr();
            if (uiContent == nullptr || !window->IsDecorEnable()) {
                TLOGND(WmsLogTag::WMS_LAYOUT_PC, "%{public}s uiContent unavailable", where);
                return;
            }
            uiContent->OnContainerModalEvent(WINDOW_WATERFALL_VISIBILITY_EVENT,
                window->supportEnterWaterfallMode_ ? "true" : "false");
        }, "UIContentOnContainerModalEvent");
    }
}

void WindowSessionImpl::HideTitleButton(bool& hideSplitButton, bool& hideMaximizeButton,
    bool& hideMinimizeButton, bool& hideCloseButton)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr || !IsDecorEnable()) {
        return;
    }
    RealTimeSwitchInfo switchInfo = property_->GetRealTimeSwitchInfo();
    uiContent->OnContainerModalEvent(SCB_COMPATIBLE_MENU_VISIBILITY, switchInfo.isNeedChange_ ? "true" : "false");
    uiContent->OnContainerModalEvent(SCB_GET_COMPATIBLE_PRIMARY_MODE, std::to_string(switchInfo.showTypes_));
    hideMaximizeButton = hideMaximizeButton || property_->IsFullScreenDisabled();
    bool isLayoutFullScreen = property_->IsLayoutFullScreen();
    bool hideSplitBtn = hideSplitButton || property_->IsSplitDisabled();
    if (property_->IsAdaptToImmersive() && !property_->GetIsAtomicService()) {
        uiContent->HideWindowTitleButton(hideSplitBtn, !isLayoutFullScreen && hideMaximizeButton,
            hideMinimizeButton, hideCloseButton);
    } else {
        uiContent->HideWindowTitleButton(hideSplitBtn, hideMaximizeButton, hideMinimizeButton, hideCloseButton);
    }
    // compatible mode adapt to proportional scale, will show its button
    bool showScaleBtn = property_->IsAdaptToProportionalScale() && !property_->GetIsAtomicService();
    uiContent->OnContainerModalEvent(SCB_COMPATIBLE_MAXIMIZE_VISIBILITY,
        !isLayoutFullScreen && showScaleBtn  ? "true" : "false");
    // compatible mode adapt to back, will show its button
    bool isAdaptToBackButton = property_->IsAdaptToBackButton();
    uiContent->OnContainerModalEvent(SCB_BACK_VISIBILITY, isAdaptToBackButton ? "true" : "false");
    bool fullScreenStart = property_->IsFullScreenStart() &&
        (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN);
    uiContent->OnContainerModalEvent(SCB_COMPATIBLE_MAXIMIZE_BTN_RES, fullScreenStart ? "true" : "false");
}

WMError WindowSessionImpl::NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    WindowSetUIContentType setUIContentType;
    if (!navDestinationInfo_.empty()) {
        setUIContentType = WindowSetUIContentType::BY_SHARED;
    } else {
        setUIContentType =
            type == BackupAndRestoreType::NONE ? WindowSetUIContentType::DEFAULT : WindowSetUIContentType::RESTORE;
    }
    return SetUIContentInner(contentInfo, env, storage, setUIContentType, type, ability);
}

WMError WindowSessionImpl::AniSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    WindowSetUIContentType setUIContentType;
    if (!navDestinationInfo_.empty()) {
        setUIContentType = WindowSetUIContentType::BY_SHARED;
    } else {
        setUIContentType =
            type == BackupAndRestoreType::NONE ? WindowSetUIContentType::DEFAULT : WindowSetUIContentType::RESTORE;
    }
    return SetUIContentInner(contentInfo, env, storage, setUIContentType, type, ability, 1u);
}

WMError WindowSessionImpl::NapiSetUIContentByName(const std::string& contentName, napi_env env, napi_value storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentName, env, storage, WindowSetUIContentType::BY_NAME,
        BackupAndRestoreType::NONE, ability);
}
WMError WindowSessionImpl::AniSetUIContentByName(const std::string& contentName, ani_env* env, ani_object storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentName, env, storage, WindowSetUIContentType::BY_NAME,
        BackupAndRestoreType::NONE, ability, 1);
}

WMError WindowSessionImpl::SetUIContentByName(
    const std::string& contentInfo, napi_env env, napi_value storage, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage, WindowSetUIContentType::BY_NAME,
        BackupAndRestoreType::NONE, ability);
}

WMError WindowSessionImpl::SetUIContentByAbc(
    const std::string& contentInfo, napi_env env, napi_value storage, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage, WindowSetUIContentType::BY_ABC,
        BackupAndRestoreType::NONE, ability);
}

void WindowSessionImpl::DestroyExistUIContent()
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->Destroy();
    }
}

std::unique_ptr<Ace::UIContent> WindowSessionImpl::UIContentCreate(AppExecFwk::Ability* ability, void* env, int isAni)
{
    if (isAni) {
        return  ability != nullptr ? Ace::UIContent::Create(ability) :
            Ace::UIContent::CreateWithAniEnv(GetContext().get(), reinterpret_cast<ani_env*>(env));
    } else {
        return  ability != nullptr ? Ace::UIContent::Create(ability) :
            Ace::UIContent::Create(GetContext().get(), reinterpret_cast<NativeEngine*>(env));
    }
}
Ace::UIContentErrorCode WindowSessionImpl::UIContentInitByName(Ace::UIContent* uiContent,
    const std::string& contentInfo, void* storage, int isAni)
{
    if (isAni) {
        return uiContent->InitializeByNameWithAniStorage(this, contentInfo, (ani_object)storage);
    } else {
        return uiContent->InitializeByName(this, contentInfo, (napi_value)storage);
    }
}

template<typename T>
Ace::UIContentErrorCode WindowSessionImpl::UIContentInit(Ace::UIContent* uiContent, T contentInfo,
    void* storage, int isAni)
{
    if (isAni) {
        return uiContent->InitializeWithAniStorage(this, contentInfo, (ani_object)storage);
    } else {
        return uiContent->Initialize(this, contentInfo, (napi_value)storage);
    }
}

Ace::UIContentErrorCode WindowSessionImpl::UIContentRestore(Ace::UIContent* uiContent, const std::string& contentInfo,
    void* storage, Ace::ContentInfoType infoType, int isAni)
{
    if (isAni) {
        return uiContent->Restore(this, contentInfo, (ani_object)storage, infoType);
    } else {
        return uiContent->Restore(this, contentInfo, (napi_value)storage, infoType);
    }
}

WMError WindowSessionImpl::InitUIContent(const std::string& contentInfo, void* env, void* storage,
    WindowSetUIContentType setUIContentType, BackupAndRestoreType restoreType, AppExecFwk::Ability* ability,
    OHOS::Ace::UIContentErrorCode& aceRet, int isAni)
{
    DestroyExistUIContent();
    std::unique_ptr<Ace::UIContent> uiContent = UIContentCreate(ability, (void*)env, isAni);
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "uiContent nullptr id: %{public}d", GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    switch (setUIContentType) {
        default:
        case WindowSetUIContentType::DEFAULT: {
            if (isUIExtensionAbilityProcess_ && property_->GetIsUIExtFirstSubWindow()) {
                // subWindow created by UIExtensionAbility
                uiContent->SetUIExtensionSubWindow(true);
                uiContent->SetUIExtensionAbilityProcess(true);
            } else {
                auto routerStack = GetRestoredRouterStack();
                auto type = GetAceContentInfoType(BackupAndRestoreType::RESOURCESCHEDULE_RECOVERY);
                if (!routerStack.empty() && UIContentRestore(uiContent.get(), routerStack, storage, type,
                    isAni) == Ace::UIContentErrorCode::NO_ERRORS) {
                    TLOGI(WmsLogTag::WMS_LIFE, "Restore router stack succeed.");
                    break;
                }
            }
            if (!intentParam_.empty()) {
                TLOGI(WmsLogTag::WMS_LIFE, "Default SetIntentParam, isColdStart:%{public}u", isIntentColdStart_);
                uiContent->SetIntentParam(intentParam_, std::move(loadPageCallback_), isIntentColdStart_);
                intentParam_ = "";
            }
            aceRet = UIContentInit(uiContent.get(), contentInfo, storage, isAni);
            break;
        }
        case WindowSetUIContentType::RESTORE:
            aceRet = UIContentRestore(uiContent.get(), contentInfo, storage, GetAceContentInfoType(restoreType), isAni);
            break;
        case WindowSetUIContentType::BY_NAME:
            if (!intentParam_.empty()) {
                TLOGI(WmsLogTag::WMS_LIFE, "By name setIntentParam, isColdStart:%{public}u", isIntentColdStart_);
                uiContent->SetIntentParam(intentParam_, std::move(loadPageCallback_), isIntentColdStart_);
                intentParam_ = "";
            }
            aceRet = UIContentInitByName(uiContent.get(), contentInfo, storage, isAni);
            break;
        case WindowSetUIContentType::BY_SHARED:
            TLOGI(WmsLogTag::WMS_LIFE, "By shared, restoreNavDestinationInfo, id:%{public}d", GetPersistentId());
            uiContent->RestoreNavDestinationInfo(navDestinationInfo_, true);
            aceRet = UIContentInit(uiContent.get(), contentInfo, storage, isAni);
            navDestinationInfo_ = "";
            break;
        case WindowSetUIContentType::BY_ABC:
            aceRet = UIContentInit(uiContent.get(), GetAbcContent(contentInfo), storage, isAni);
            break;
    }
    // make uiContent available after Initialize/Restore
    {
        std::unique_lock<std::shared_mutex> lock(uiContentMutex_);
        uiContent_ = std::move(uiContent);
        // compatibleMode app in pc, need change decor title to float title bar
        if (property_->IsAdaptToImmersive()) {
            uiContent_->SetContainerModalTitleVisible(false, true);
            uiContent_->EnableContainerModalCustomGesture(true);
        }
        for (auto& listener : uiContentCreateListeners_) {
            listener->OnUIContentCreate(std::weak_ptr<Ace::UIContent>(uiContent_));
        }
        TLOGI(WmsLogTag::DEFAULT, "[%{public}d, %{public}d]",
            uiContent_->IsUIExtensionSubWindow(), uiContent_->IsUIExtensionAbilityProcess());
    }
    RegisterUIContenCallback();
    return WMError::WM_OK;
}

void WindowSessionImpl::RegisterUIContenCallback()
{
    RegisterWatchFocusActiveChangeCallback();
    RegisterKeyFrameCallback();
}

void WindowSessionImpl::RegisterWatchFocusActiveChangeCallback()
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->AddFocusActiveChangeCallback([this](bool isFocusActive) {
            NotifyWatchFocusActiveChange(isFocusActive);
        });
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "uiContent is nullptr");
    }
}

void WindowSessionImpl::RegisterKeyFrameCallback()
{
    auto uiContent = GetUIContentSharedPtr();
    if (!uiContent) {
        TLOGE(WmsLogTag::WMS_EVENT, "uiContent is nullptr");
        return;
    }
    const char* const where = __func__;
    uiContent->AddKeyFrameNodeCallback([where, weakThis = wptr(this)](
        std::shared_ptr<RSWindowKeyFrameNode>& rsKeyFrameNode, std::shared_ptr<RSTransaction>& rsTransaction) {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s window is null", where);
            return;
        }
        if (auto session = window->GetHostSession()) {
            session->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransaction);
        } else {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is nullptr", where);
        }
    });
    uiContent->AddKeyFrameAnimateEndCallback([where, weakThis = wptr(this)]() {
        auto window = weakThis.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s window is null", where);
            return;
        }
        if (auto session = window->GetHostSession()) {
            session->KeyFrameAnimateEnd();
        } else {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is nullptr", where);
        }
    });
}

WSError WindowSessionImpl::LinkKeyFrameNode()
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    auto uiContent = GetUIContentSharedPtr();
    auto session = GetHostSession();
    if (!uiContent || !session) {
        TLOGE(WmsLogTag::WMS_EVENT, "uiContent or session is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    uiContent->LinkKeyFrameNode();
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SetStageKeyFramePolicy(const KeyFramePolicy& keyFramePolicy)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    keyFramePolicy_ = keyFramePolicy;
    return WSError::WS_OK;
}

WMError WindowSessionImpl::SetDragKeyFramePolicy(const KeyFramePolicy& keyFramePolicy)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    if (!IsPhonePadOrPcWindow()) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "only main window is valid");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);

    if (!IsPcWindow()) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ignore not pc window type");
        return WMError::WM_OK;
    }
    WSError errorCode = hostSession->SetDragKeyFramePolicy(keyFramePolicy);
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "Id: %{public}d, keyFramePolicy: %{public}s, errorCode: %{public}d",
        GetPersistentId(), keyFramePolicy.ToString().c_str(), static_cast<int32_t>(errorCode));
    return static_cast<WMError>(errorCode);
}

WMError WindowSessionImpl::NotifyWatchFocusActiveChange(bool isActive)
{
    TLOGD(WmsLogTag::WMS_EVENT, "isActive:%{public}d", isActive);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().NotifyWatchFocusActiveChange(isActive);
}

void WindowSessionImpl::SetForceSplitConfig(const AppForceLandscapeConfig& config)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "uiContent is null!");
        return;
    }
    AppForceSplitConfig appForceSplitConfig;
    SystemForceSplitConfig systemForceSplitConfig;
    if (config.hasChanged_) {
        if (config.containsAppConfig_) {
            appForceSplitConfig.isRouter = config.isAppRouter_;
            appForceSplitConfig.configJsonStr = config.appConfigJsonStr_;
        } else if (config.containsSysConfig_) {
            systemForceSplitConfig.isRouter = config.isSysRouter_;
            systemForceSplitConfig.homePage = config.sysHomePage_;
            systemForceSplitConfig.configJsonStr = config.sysConfigJsonStr_;
        }
    }
    uiContent->SetForceSplitConfig(
        config.containsSysConfig_ ? std::make_optional(systemForceSplitConfig) : std::nullopt,
        config.containsAppConfig_ ? std::make_optional(appForceSplitConfig) : std::nullopt);
}

void WindowSessionImpl::SetAppHookWindowInfo(const HookWindowInfo& hookWindowInfo)
{
    std::unique_lock<std::shared_mutex> lock(hookWindowInfoMutex_);
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}u, preHookWindowInfo:[%{public}s], newHookWindowInfo:[%{public}s]",
        GetWindowId(), hookWindowInfo_.ToString().c_str(), hookWindowInfo.ToString().c_str());
    hookWindowInfo_ = hookWindowInfo;
}

HookWindowInfo WindowSessionImpl::GetAppHookWindowInfo()
{
    std::shared_lock<std::shared_mutex> lock(hookWindowInfoMutex_);
    return hookWindowInfo_;
}

void WindowSessionImpl::HookWindowSizeByHookWindowInfo(Rect& rect)
{
    auto hookWindowInfo = GetAppHookWindowInfo();
    if (!hookWindowInfo.enableHookWindow || !WindowHelper::IsMainWindow(GetType()) ||
        isFullScreenInForceSplit_.load()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}u, do not need hook window info.", GetWindowId());
        return;
    }
    if (!MathHelper::NearEqual(hookWindowInfo.widthHookRatio, HookWindowInfo::DEFAULT_WINDOW_SIZE_HOOK_RATIO)) {
        rect.width_ = static_cast<uint32_t>(rect.width_ * hookWindowInfo.widthHookRatio);
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}u, hook window width, hooked width:%{public}u, "
            "widthHookRatio:%{public}f.", GetWindowId(), rect.width_, hookWindowInfo.widthHookRatio);
    }
}

WMError WindowSessionImpl::SetUIContentInner(const std::string& contentInfo, void* env, void* storage,
    WindowSetUIContentType setUIContentType, BackupAndRestoreType restoreType, AppExecFwk::Ability* ability,
    int isAni)
{
    TLOGI(WmsLogTag::WMS_LIFE, "%{public}s, state:%{public}u, persistentId: %{public}d",
        contentInfo.c_str(), state_, GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "window is invalid! window state: %{public}d",
            state_);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (GetUIContentSharedPtr() != nullptr) {
        shouldReNotifyHighlight_ = true;
    }
    NotifySetUIContentComplete();
    OHOS::Ace::UIContentErrorCode aceRet = OHOS::Ace::UIContentErrorCode::NO_ERRORS;
    WMError initUIContentRet = InitUIContent(contentInfo, env, storage, setUIContentType, restoreType, ability, aceRet,
        isAni);
    if (initUIContentRet != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "Init UIContent fail, ret:%{public}u", initUIContentRet);
        return initUIContentRet;
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        if (iconCache_ != nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "Use iconCache to set windowIcon");
            uiContent->SetAppWindowIcon(std::move(iconCache_));
        }
        TLOGI(WmsLogTag::WMS_LAYOUT, "single hand, id:%{public}d, posX:%{public}d, posY:%{public}d, "
              "scaleX:%{public}f, scaleY:%{public}f", GetPersistentId(),
              singleHandTransform_.posX, singleHandTransform_.posY,
              singleHandTransform_.scaleX, singleHandTransform_.scaleY);
        uiContent->UpdateSingleHandTransform(singleHandTransform_);
    }
    WindowType winType = GetType();
    bool isSubWindow = WindowHelper::IsSubWindow(winType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(winType);
    if (IsDecorEnable()) {
        if (isSubWindow) {
            SetAPPWindowLabel(subWindowTitle_);
        } else if (isDialogWindow) {
            SetAPPWindowLabel(dialogTitle_);
        }
    }

    AppForceLandscapeConfig config = {};
    if (WindowHelper::IsMainWindow(winType) && GetAppForceLandscapeConfig(config) == WMError::WM_OK &&
        config.supportSplit_ > 0) {
        SetForceSplitConfig(config);
        bool enableForceSplit = false;
        if ((config.mode_ == FORCE_SPLIT_MODE || config.mode_ == NAV_FORCE_SPLIT_MODE) &&
            GetAppForceLandscapeConfigEnable(enableForceSplit) == WMError::WM_OK) {
                SetForceSplitConfigEnable(enableForceSplit);
        }
    }

    uint32_t version = 0;
    auto context = GetContext();
    if ((context != nullptr) && (context->GetApplicationInfo() != nullptr)) {
        version = context->GetApplicationInfo()->apiCompatibleVersion;
    }
    // 10 ArkUI new framework support after API10
    if (version < 10) {
        SetLayoutFullScreenByApiVersion(isIgnoreSafeArea_);
        if (!isSystembarPropertiesSet_) {
            SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, SystemBarProperty());
        }
    } else if (isIgnoreSafeAreaNeedNotify_) {
        SetLayoutFullScreenByApiVersion(isIgnoreSafeArea_);
    } else if (isSubWindow) {
        SetLayoutFullScreenByApiVersion(isIgnoreSafeArea_);
        isIgnoreSafeAreaNeedNotify_ = false;
    }

    // UIContent may be nullptr on setting system bar properties, need to set menubar color on UIContent init.
    if (auto uiContent = GetUIContentSharedPtr()) {
        auto property = GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        uiContent->SetStatusBarItemColor(property.contentColor_);
    }

    UpdateDecorEnable(true);
    if (state_ == WindowState::STATE_SHOWN) {
        // UIContent may be nullptr when show window, need to notify again when window is shown
        if (auto uiContent = GetUIContentSharedPtr()) {
            uiContent->Foreground();
        }
        UpdateTitleButtonVisibility();
    }
    UpdateViewportConfig(GetRect(), WindowSizeChangeReason::UNDEFINED);
    if (shouldReNotifyFocus_) {
        // uiContent may be nullptr when notify focus status, need to notify again when uiContent is not empty.
        NotifyUIContentFocusStatus();
        shouldReNotifyFocus_ = false;
    }
    if (shouldReNotifyHighlight_) {
        // uiContent may be nullptr when notify highlight status, need to notify again when uiContent is not empty.
        NotifyUIContentHighlightStatus(isHighlighted_);
        shouldReNotifyHighlight_ = false;
    }
    if (aceRet != OHOS::Ace::UIContentErrorCode::NO_ERRORS) {
        WLOGFE("failed to init or restore uicontent with file %{public}s. errorCode: %{public}d",
            contentInfo.c_str(), static_cast<uint16_t>(aceRet));
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    NotifyAfterUIContentReady();
    TLOGD(WmsLogTag::WMS_LIFE, "end");
    return WMError::WM_OK;
}

std::shared_ptr<std::vector<uint8_t>> WindowSessionImpl::GetAbcContent(const std::string& abcPath)
{
    std::filesystem::path abcFile { abcPath };
    if (abcFile.empty() || !abcFile.is_absolute() || !std::filesystem::exists(abcFile)) {
        WLOGFE("abc file path is not valid");
        return nullptr;
    }
    int begin, end;
    std::fstream file(abcFile, std::ios::in | std::ios::binary);
    if (!file) {
        WLOGFE("abc file is not valid");
        return nullptr;
    }
    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();
    int len = end - begin;
    WLOGFD("abc file: %{public}s, size: %{public}d", abcPath.c_str(), len);

    if (len <= 0) {
        WLOGFE("abc file size is 0");
        return nullptr;
    }
    std::vector<uint8_t> abcBytes(len);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(abcBytes.data()), len);
    return std::make_shared<std::vector<uint8_t>>(abcBytes);
}

void WindowSessionImpl::UpdateDecorEnableToAce(bool isDecorEnable)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        WindowMode mode = GetWindowMode();
        bool decorVisible = mode == WindowMode::WINDOW_MODE_FLOATING ||
            mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
            (mode == WindowMode::WINDOW_MODE_FULLSCREEN && !property_->IsLayoutFullScreen() && !(IsAnco() &&
            IsPcOrPadFreeMultiWindowMode()));
        TLOGD(WmsLogTag::WMS_DECOR, "decorVisible:%{public}d", decorVisible);
        if (windowSystemConfig_.freeMultiWindowSupport_) {
            auto isSubWindow = WindowHelper::IsSubWindow(GetType());
            decorVisible = decorVisible && (windowSystemConfig_.freeMultiWindowEnable_ ||
                (property_->GetIsPcAppInPad() && isSubWindow)) &&
                property_->GetIsShowDecorInFreeMultiWindow();
        }
        if (mode == WindowMode::WINDOW_MODE_FULLSCREEN && property_->IsDecorFullscreenDisabled()) {
            decorVisible = false;
        }
        uiContent->UpdateDecorVisible(decorVisible, isDecorEnable);
        return;
    }

    std::lock_guard<std::recursive_mutex> lockListener(windowChangeListenerMutex_);
    auto windowChangeListeners = GetListeners<IWindowChangeListener>();
    for (auto& listener : windowChangeListeners) {
        if (listener.GetRefPtr() != nullptr) {
            listener.GetRefPtr()->OnModeChange(GetWindowMode(), isDecorEnable);
        }
    }
}

void WindowSessionImpl::UpdateDecorEnable(bool needNotify, WindowMode mode)
{
    if (mode == WindowMode::WINDOW_MODE_UNDEFINED) {
        mode = GetWindowMode();
    }
    if (needNotify) {
        if (auto uiContent = GetUIContentSharedPtr()) {
            bool decorVisible = mode == WindowMode::WINDOW_MODE_FLOATING ||
                mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
                (mode == WindowMode::WINDOW_MODE_FULLSCREEN && !property_->IsLayoutFullScreen() && !(IsAnco() &&
                IsPcOrPadFreeMultiWindowMode()));
            if (windowSystemConfig_.freeMultiWindowSupport_) {
                auto isSubWindow = WindowHelper::IsSubWindow(GetType());
                decorVisible = decorVisible && (windowSystemConfig_.freeMultiWindowEnable_ ||
                    (property_->GetIsPcAppInPad() && isSubWindow)) &&
                    property_->GetIsShowDecorInFreeMultiWindow();
            }
            if (GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN && property_->IsDecorFullscreenDisabled()) {
                decorVisible = false;
            }
            TLOGI(WmsLogTag::WMS_DECOR, "decorVisible:%{public}d, id: %{public}d", decorVisible, GetPersistentId());
            uiContent->UpdateDecorVisible(decorVisible, IsDecorEnable());
            uiContent->NotifyWindowMode(mode);
        }
        NotifyModeChange(mode, IsDecorEnable());
    }
}

/** @note @window.layout */
void WindowSessionImpl::NotifyModeChange(WindowMode mode, bool hasDeco)
{
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowChangeListenerMutex_);
        auto windowChangeListeners = GetListeners<IWindowChangeListener>();
        for (auto& listener : windowChangeListeners) {
            if (listener.GetRefPtr() != nullptr) {
                listener.GetRefPtr()->OnModeChange(mode, hasDeco);
            }
        }
    }

    if (GetHostSession()) {
        property_->SetWindowMode(mode);
        property_->SetDecorEnable(hasDeco);
    }
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MODE);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE);
}

std::shared_ptr<RSSurfaceNode> WindowSessionImpl::GetSurfaceNode() const
{
    TLOGI(WmsLogTag::DEFAULT, "name:%{public}s, id:%{public}d",
        property_->GetWindowName().c_str(), GetPersistentId());
    return surfaceNode_;
}

const std::shared_ptr<AbilityRuntime::Context> WindowSessionImpl::GetContext() const
{
    TLOGD(WmsLogTag::DEFAULT, "name:%{public}s, id:%{public}d",
        property_->GetWindowName().c_str(), GetPersistentId());
    std::shared_lock<std::shared_mutex> lock(contextMutex_);
    return context_;
}

void WindowSessionImpl::SetContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    std::unique_lock<std::shared_mutex> lock(contextMutex_);
    context_ = context;
}

Rect WindowSessionImpl::GetRequestRect() const
{
    return property_->GetRequestRect();
}

Rect WindowSessionImpl::GetGlobalDisplayRect() const
{
    return property_->GetGlobalDisplayRect();
}

WMError WindowSessionImpl::ClientToGlobalDisplay(const Position& inPosition, Position& outPosition) const
{
    const auto windowId = GetWindowId();
    const auto transform = GetCurrentTransform();
    if (WindowHelper::IsScaled(transform)) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
            "Scaled window is not supported, windowId: %{public}u, scaleX: %{public}f, scaleY: %{public}f",
            windowId, transform.scaleX_, transform.scaleY_);
        return WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS;
    }
    const auto globalDisplayRect = GetGlobalDisplayRect();
    const Position& basePosition = {globalDisplayRect.posX_, globalDisplayRect.posY_};
    if (!inPosition.SafeAdd(basePosition, outPosition)) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
            "Position overflow, windowId: %{public}u, inPosition: %{public}s, basePosition: %{public}s",
            windowId, inPosition.ToString().c_str(), basePosition.ToString().c_str());
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT,
        "windowId: %{public}u, globalDisplayRect: %{public}s, inPosition: %{public}s, outPosition: %{public}s",
        windowId, globalDisplayRect.ToString().c_str(),
        inPosition.ToString().c_str(), outPosition.ToString().c_str());
    return WMError::WM_OK;
}

WMError WindowSessionImpl::GlobalDisplayToClient(const Position& inPosition, Position& outPosition) const
{
    const auto windowId = GetWindowId();
    const auto transform = GetCurrentTransform();
    if (WindowHelper::IsScaled(transform)) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
            "Scaled window is not supported, windowId: %{public}u, scaleX: %{public}f, scaleY: %{public}f",
            windowId, transform.scaleX_, transform.scaleY_);
        return WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS;
    }
    const auto globalDisplayRect = GetGlobalDisplayRect();
    const Position& basePosition = {globalDisplayRect.posX_, globalDisplayRect.posY_};
    if (!inPosition.SafeSub(basePosition, outPosition)) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
            "Position overflow, windowId: %{public}u, inPosition: %{public}s, basePosition: %{public}s",
            windowId, inPosition.ToString().c_str(), basePosition.ToString().c_str());
        return WMError::WM_ERROR_ILLEGAL_PARAM;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT,
        "windowId: %{public}u, globalDisplayRect: %{public}s, inPosition: %{public}s, outPosition: %{public}s",
        windowId, globalDisplayRect.ToString().c_str(),
        inPosition.ToString().c_str(), outPosition.ToString().c_str());
    return WMError::WM_OK;
}

WSError WindowSessionImpl::UpdateGlobalDisplayRectFromServer(const WSRect& rect, SizeChangeReason reason)
{
    const uint32_t windowId = GetWindowId();
    TLOGD(WmsLogTag::WMS_LAYOUT, "windowId: %{public}u, rect: %{public}s, reason: %{public}u",
        windowId, rect.ToString().c_str(), reason);

    // According to the drag specification, in drag-move scenarios, if the final reason is DRAG_END
    // but the last recorded reason is neither DRAG_START nor DRAG, override it to DRAG_MOVE
    // to ensure consistent handling of drag operations (same as JsWindowListener::OnRectChange).
    if (reason == SizeChangeReason::DRAG_END &&
        globalDisplayRectSizeChangeReason_ != SizeChangeReason::DRAG_START &&
        globalDisplayRectSizeChangeReason_ != SizeChangeReason::DRAG) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Override DRAG_END to DRAG_MOVE, windowId: %{public}u", windowId);
        reason = SizeChangeReason::DRAG_MOVE;
    }

    Rect newRect = { rect.posX_, rect.posY_, rect.width_, rect.height_ };
    if (newRect == GetGlobalDisplayRect() && reason == globalDisplayRectSizeChangeReason_) {
        TLOGD(WmsLogTag::WMS_LAYOUT,
            "No change in rect or reason, windowId: %{public}d, rect: %{public}s, reason: %{public}u",
            windowId, rect.ToString().c_str(), reason);
        return WSError::WS_DO_NOTHING;
    }
    property_->SetGlobalDisplayRect(newRect);
    globalDisplayRectSizeChangeReason_ = reason;
    auto windowSizeChangeReason = static_cast<WindowSizeChangeReason>(reason);
    layoutCallback_->OnUpdateGlobalDisplayRect(newRect, windowSizeChangeReason, GetPersistentId());
    NotifyGlobalDisplayRectChange(newRect, windowSizeChangeReason);
    return WSError::WS_OK;
}

WindowType WindowSessionImpl::GetType() const
{
    return property_->GetWindowType();
}

const std::string& WindowSessionImpl::GetWindowName() const
{
    return property_->GetWindowName();
}

WindowState WindowSessionImpl::GetWindowState() const
{
    return state_;
}

WindowState WindowSessionImpl::GetRequestWindowState() const
{
    return requestState_;
}

WSError WindowSessionImpl::NotifyExtensionSecureLimitChange(bool isLimit)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "windowId: %{public}d, isLimite: %{public}u", GetPersistentId(), isLimit);
    std::vector<sptr<IExtensionSecureLimitChangeListener>> secureLimitChangeListeners;
    {
        std::lock_guard<std::mutex> lockListener(secureLimitChangeListenerMutex_);
        secureLimitChangeListeners = GetListeners<IExtensionSecureLimitChangeListener>();
    }
    for (const auto& listener : secureLimitChangeListeners) {
        if (listener != nullptr) {
            listener->OnSecureLimitChange(isLimit);
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        AAFwk::Want want;
        want.SetParam(Extension::EXTENSION_SECURE_LIMIT_CHANGE, isLimit);
        uiContent->SendUIExtProprty(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_EXTENSION_SECURE_LIMIT_CHANGE),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WSError::WS_OK;
}

/** @note @window.focus */
WMError WindowSessionImpl::SetExclusivelyHighlighted(bool isExclusivelyHighlighted)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "windowId: %{public}d, isExclusivelyHighlighted: %{public}d",
        property_->GetPersistentId(), isExclusivelyHighlighted);
    if (WindowHelper::IsMainWindow(GetType()) || WindowHelper::IsDialogWindow(GetType()) ||
        WindowHelper::IsModalWindow(property_->GetWindowFlags())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "unsupport window, type: %{public}u, windowFlags: %{public}u ",
            GetType(), property_->GetWindowFlags());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (property_->GetExclusivelyHighlighted() == isExclusivelyHighlighted) {
        TLOGD(WmsLogTag::WMS_FOCUS, "already exclusivelyHighlighted");
        return WMError::WM_OK;
    }
    property_->SetExclusivelyHighlighted(isExclusivelyHighlighted);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_EXCLUSIVE_HIGHLIGHTED);
}

/** @note @window.focus */
bool WindowSessionImpl::GetExclusivelyHighlighted() const
{
    bool isExclusivelyHighlighted = property_->GetExclusivelyHighlighted();
    TLOGD(WmsLogTag::WMS_FOCUS, "windowId: %{public}d, isExclusivelyHighlighted: %{public}d",
        property_->GetPersistentId(), isExclusivelyHighlighted);
    return isExclusivelyHighlighted;
}

/** @note @window.focus */
WSError WindowSessionImpl::NotifyHighlightChange(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight)
{
    if (highlightNotifyInfo == nullptr || !highlightNotifyInfo->isSyncNotify_) {
        NotifyHighlightChange(isHighlight);
        return WSError::WS_OK;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "timeStamp:%{public}" PRId64 ", highlightId:%{public}d, isHighlight:%{public}d,"
        "isSyncNotify:%{public}d, current:%{public}" PRId64 ", new:%{public}" PRId64, highlightNotifyInfo->timeStamp_,
        highlightNotifyInfo->highlightId_, isHighlight, highlightNotifyInfo->isSyncNotify_,
        updateHighlightTimeStamp_.load(), highlightNotifyInfo->timeStamp_);
    if (highlightNotifyInfo->timeStamp_ <= updateHighlightTimeStamp_.load()) {
        return WSError::WS_OK;
    }
    updateHighlightTimeStamp_.store(highlightNotifyInfo->timeStamp_);
    for (auto unHighlightWindowId : highlightNotifyInfo->notHighlightIds_) {
        if (!isHighlight && static_cast<uint32_t>(unHighlightWindowId) == GetWindowId()) {
            NotifyHighlightChange(isHighlight);
            continue;
        }
        auto unHighlightWindow = GetWindowWithId(unHighlightWindowId);
        if (unHighlightWindow != nullptr) {
            unHighlightWindow->NotifyHighlightChange(false);
        }
    }
    if (isHighlight) {
        NotifyHighlightChange(isHighlight);
    } else {
        auto highlightWindow = GetWindowWithId(highlightNotifyInfo->highlightId_);
        if (highlightWindow != nullptr) {
            highlightWindow->NotifyHighlightChange(true);
        }
    }
    return WSError::WS_OK;
}

/** @note @window.focus */
void WindowSessionImpl::NotifyHighlightChange(bool isHighlight)
{
    if (isHighlighted_ == isHighlight) {
        TLOGI(WmsLogTag::WMS_FOCUS, "update highlight repeated, windowId: %{public}d, isHighlight: %{public}u",
            GetPersistentId(), isHighlight);
        return;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "windowId: %{public}d, isHighlight: %{public}u,", GetPersistentId(), isHighlight);
    isHighlighted_ = isHighlight;
    if (GetUIContentSharedPtr() != nullptr) {
        NotifyUIContentHighlightStatus(isHighlighted_);
    } else {
        shouldReNotifyHighlight_ = true;
    }
    std::lock_guard<std::mutex> lockListener(highlightChangeListenerMutex_);
    auto highlightChangeListeners = GetListeners<IWindowHighlightChangeListener>();
    for (const auto& listener : highlightChangeListeners) {
        if (listener != nullptr) {
            listener->OnWindowHighlightChange(isHighlight);
        }
    }
}

/** @note @window.focus */
WMError WindowSessionImpl::IsWindowHighlighted(bool& highlighted) const
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    hostSession->GetIsHighlighted(highlighted);
    TLOGD(WmsLogTag::WMS_FOCUS, "windowId: %{public}d, isWindowHighlighted: %{public}d",
        GetPersistentId(), highlighted);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetFocusable(bool isFocusable)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "set focusable: windowId=%{public}d, isFocusable=%{public}d",
        property_->GetPersistentId(), isFocusable);
    bool curIsFocusable = GetFocusable();
    property_->SetFocusable(isFocusable);
    WMError ret = UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE);
    if (ret != WMError::WM_OK) {
        property_->SetFocusable(curIsFocusable);
        return ret;
    }
    return WMError::WM_OK;
}

bool WindowSessionImpl::GetFocusable() const
{
    bool isFocusable = property_->GetFocusable();
    TLOGD(WmsLogTag::WMS_FOCUS, "get focusable: windowId=%{public}d, isFocusable=%{public}d",
        property_->GetPersistentId(), isFocusable);
    return isFocusable;
}

WMError WindowSessionImpl::SetTouchable(bool isTouchable)
{
    WLOGFD("%{public}d", isTouchable);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetTouchable(isTouchable);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE);
}

/** @note @window.hierarchy */
WMError WindowSessionImpl::SetTopmost(bool topmost)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "%{public}d", topmost);
    if (!IsPcOrPadFreeMultiWindowMode()) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetTopmost(topmost);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
}

/** @note @window.hierarchy */
bool WindowSessionImpl::IsTopmost() const
{
    return property_->IsTopmost();
}

/** @note @window.hierarchy */
WMError WindowSessionImpl::SetMainWindowTopmost(bool isTopmost)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    property_->SetMainWindowTopmost(isTopmost);
    uint32_t accessTokenId = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
    property_->SetAccessTokenId(accessTokenId);
    TLOGD(WmsLogTag::WMS_HIERARCHY, "tokenId=%{private}u, isTopmost=%{public}d", accessTokenId, isTopmost);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST);
}

/** @note @window.hierarchy */
WMError WindowSessionImpl::RaiseToAppTopOnDrag()
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsAppWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "must be app window");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    if (WindowHelper::IsSubWindow(GetType())) {
        return static_cast<WMError>(hostSession->RaiseToAppTop());
    }
    return static_cast<WMError>(hostSession->RaiseAppMainWindowToTop());
}

bool WindowSessionImpl::IsMainWindowTopmost() const
{
    return property_->IsMainWindowTopmost();
}

WMError WindowSessionImpl::SetWindowDelayRaiseEnabled(bool isEnabled)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "The is PcAppInPad, not supported");
        return WMError::WM_OK;
    }
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsAppWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "window type is not supported");
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    property_->SetWindowDelayRaiseEnabled(isEnabled);
    TLOGI(WmsLogTag::WMS_FOCUS, "isEnabled: %{public}d", isEnabled);
    AAFwk::Want want;
    want.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, isEnabled);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_WINDOW_DELAY_RAISE_STATE),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WMError::WM_OK;
}

bool WindowSessionImpl::IsWindowDelayRaiseEnabled() const
{
    return property_->IsWindowDelayRaiseEnabled();
}

WMError WindowSessionImpl::SetResizeByDragEnabled(bool dragEnabled)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "%{public}d", dragEnabled);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (WindowHelper::IsMainWindow(GetType()) ||
        (WindowHelper::IsSubWindow(GetType()) && windowOption_->GetSubWindowDecorEnable())) {
        property_->SetDragEnabled(dragEnabled);
        hasSetEnableDrag_.store(true);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "This is not main window or decor enabled sub window.");
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
}

/** @note @window.hierarchy */
WMError WindowSessionImpl::SetRaiseByClickEnabled(bool raiseEnabled)
{
    WLOGFD("%{public}d", raiseEnabled);
    auto parentId = GetParentId();
    if (parentId == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Window id: %{public}d Parent id is invalid!",
              GetPersistentId());
        return WMError::WM_ERROR_INVALID_PARENT;
    }

    if (!WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Window id: %{public}d Must be app sub window!",
              GetPersistentId());
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    if (state_ != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Window id: %{public}d The sub window must be shown!",
              GetPersistentId());
        return WMError::WM_DO_NOTHING;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    property_->SetRaiseEnabled(raiseEnabled);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED);
}

/** @note @window.hierarchy */
WMError WindowSessionImpl::SetMainWindowRaiseByClickEnabled(bool raiseEnabled)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "id: %{public}d, enable: %{public}d", GetPersistentId(), raiseEnabled);
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Window id: %{public}d Must be app main window!",
              GetPersistentId());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (state_ != WindowState::STATE_SHOWN) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Window id: %{public}d The main window must be shown!",
              GetPersistentId());
        return WMError::WM_DO_NOTHING;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetRaiseEnabled(raiseEnabled);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED);
}

/** @note @window.immersive */
WMError WindowSessionImpl::SetAvoidAreaOption(uint32_t avoidAreaOption)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetAvoidAreaOption(avoidAreaOption);
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d, set option %{public}d",
        GetPersistentId(), avoidAreaOption);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_AVOID_AREA_OPTION);
}

/** @note @window.immersive */
WMError WindowSessionImpl::GetAvoidAreaOption(uint32_t& avoidAreaOption)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_IMMS, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    avoidAreaOption = property_->GetAvoidAreaOption();
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HideNonSystemFloatingWindows(bool shouldHide)
{
    WLOGFD("%{public}d", shouldHide);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetHideNonSystemFloatingWindows(shouldHide);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);
}

WMError WindowSessionImpl::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "isLandscapeMultiWindow:%{public}d",
        isLandscapeMultiWindow);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    hostSession->SetLandscapeMultiWindow(isLandscapeMultiWindow);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetSingleFrameComposerEnabled(bool enable)
{
    WLOGFD("enable %{public}d", enable);
    if (IsWindowSessionInvalid()) {
        WLOGE("The window state is invalid ");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (surfaceNode_ == nullptr) {
        WLOGE("The surface node is nullptr");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    surfaceNode_->MarkNodeSingleFrameComposer(enable);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);
    return WMError::WM_OK;
}

bool WindowSessionImpl::IsFloatingWindowAppType() const
{
    if (IsWindowSessionInvalid()) {
        return false;
    }
    return property_->IsFloatingWindowAppType();
}

bool WindowSessionImpl::GetTouchable() const
{
    return property_->GetTouchable();
}

WMError WindowSessionImpl::SetWindowType(WindowType type)
{
    TLOGD(WmsLogTag::DEFAULT, "%{public}u type %{public}u", GetWindowId(), static_cast<uint32_t>(type));
    if (type != WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW && !SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "Session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    property_->SetWindowType(type);
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetBrightness(float brightness)
{
    if ((brightness < MINIMUM_BRIGHTNESS &&
         std::fabs(brightness - UNDEFINED_BRIGHTNESS) >= std::numeric_limits<float>::min()) ||
        brightness > MAXIMUM_BRIGHTNESS) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid brightness value: %{public}f", brightness);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!(WindowHelper::IsAppWindow(GetType()) || GetType() == WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "non app window does not support set brightness, type: %{public}u", GetType());
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    property_->SetBrightness(brightness);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);
}

float WindowSessionImpl::GetBrightness() const
{
    return property_->GetBrightness();
}

void WindowSessionImpl::SetRequestedOrientation(Orientation orientation, bool needAnimation)
{
    HITRACE_METER_NAME(HITRACE_TAG_WINDOW_MANAGER, "WindowSessionImpl::SetRequestedOrientation");
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "windowSession is invalid");
        return;
    }
    TLOGI(WmsLogTag::WMS_MAIN, "id:%{public}u lastReqOrientation:%{public}u target:%{public}u state:%{public}u",
        GetPersistentId(), property_->GetRequestedOrientation(), orientation, state_);
    if (!isNeededForciblySetOrientation(orientation) && needAnimation) {
        return;
    }
    if (property_->IsSupportRotateFullScreen()) {
        TLOGI(WmsLogTag::WMS_COMPAT, "compatible request horizontal orientation %{public}u", orientation);
        property_->SetIsLayoutFullScreen(IsHorizontalOrientation(orientation));
    }
    if (needAnimation) {
        NotifyPreferredOrientationChange(orientation);
        SetUserRequestedOrientation(orientation);
    }
    if (orientation == Orientation::INVALID) {
        Orientation requestedOrientation = ConvertInvalidOrientation();
        property_->SetRequestedOrientation(requestedOrientation, needAnimation);
    } else {
        property_->SetRequestedOrientation(orientation, needAnimation);
        property_->SetIsSpecificSessionRequestOrientation(true);
    }
    UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION);
}

Orientation WindowSessionImpl::GetRequestedOrientation()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "windowSession is invalid");
        return Orientation::UNSPECIFIED;
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "userRequestedOrientation:%{public}u", property_->GetUserRequestedOrientation());
    return property_->GetUserRequestedOrientation();
}

WMError WindowSessionImpl::ConvertOrientationAndRotation(
    const RotationInfoType from, const RotationInfoType to, const int32_t value, int32_t& convertedValue)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "windowSession is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!windowSystemConfig_.IsPadWindow() && !windowSystemConfig_.IsPhoneWindow()) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (value < MIN_ROTATION_VALUE || value > MAX_ROTATION_VALUE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "the value to be converted is invalid");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (from == to) {
        convertedValue = value;
        return WMError::WM_OK;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    WSError ret = hostSession->ConvertOrientationAndRotation(from, to, value, convertedValue);
    TLOGI(WmsLogTag::WMS_ROTATION, "convertedValue:%{public}d", convertedValue);
    return static_cast<WMError>(ret);
}

Orientation WindowSessionImpl::ConvertUserOrientationToUserPageOrientation(Orientation Orientation) const
{
    switch (Orientation) {
        case Orientation::USER_ROTATION_LANDSCAPE:
            return Orientation::USER_PAGE_ROTATION_LANDSCAPE;
        case Orientation::USER_ROTATION_LANDSCAPE_INVERTED:
            return Orientation::USER_PAGE_ROTATION_LANDSCAPE_INVERTED;
        case Orientation::USER_ROTATION_PORTRAIT:
            return Orientation::USER_PAGE_ROTATION_PORTRAIT;
        case Orientation::USER_ROTATION_PORTRAIT_INVERTED:
            return Orientation::USER_PAGE_ROTATION_PORTRAIT_INVERTED;
        default:
            break;
    }
    return Orientation::UNSPECIFIED;
}

Orientation WindowSessionImpl::ConvertInvalidOrientation()
{
    Orientation requestedOrientation = GetRequestedOrientation();
    if (IsUserOrientation(requestedOrientation)) {
        requestedOrientation = ConvertUserOrientationToUserPageOrientation(requestedOrientation);
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "convertInvalidOrientation:%{public}u", requestedOrientation);
    return requestedOrientation;
}

void WindowSessionImpl::SetUserRequestedOrientation(Orientation orientation)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "windowSession is invalid");
        return;
    }
    TLOGI(WmsLogTag::WMS_ROTATION,
        "id:%{public}u userRequestedOrientation:%{public}u state:%{public}u",
        GetPersistentId(), orientation, state_);
    property_->SetUserRequestedOrientation(orientation);
}

bool WindowSessionImpl::isNeededForciblySetOrientation(Orientation orientation)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "orientation:%{public}u", orientation);
    if (IsUserOrientation(orientation)) {
        return true;
    }
    Orientation lastOrientation = property_->GetRequestedOrientation();
    TLOGI(WmsLogTag::WMS_ROTATION, "lastOrientation:%{public}u", lastOrientation);
    if (orientation == Orientation::INVALID) {
        orientation = ConvertInvalidOrientation();
        if (IsUserPageOrientation(orientation) && IsUserOrientation(lastOrientation)) {
            lastOrientation = ConvertUserOrientationToUserPageOrientation(lastOrientation);
        }
    }
    return lastOrientation != orientation;
}

std::string WindowSessionImpl::GetContentInfo(BackupAndRestoreType type)
{
    WLOGFD("in");
    if (type == BackupAndRestoreType::NONE) {
        return "";
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        WLOGFE("fail to GetContentInfo id: %{public}d", GetPersistentId());
        return "";
    }
    return uiContent->GetContentInfo(GetAceContentInfoType(type));
}

WMError WindowSessionImpl::SetRestoredRouterStack(const std::string& routerStack)
{
    TLOGD(WmsLogTag::WMS_LIFE, "Set restored router stack.");
    restoredRouterStack_ = routerStack;
    return WMError::WM_OK;
}

std::string WindowSessionImpl::GetRestoredRouterStack()
{
    TLOGD(WmsLogTag::WMS_LIFE, "Get restored router stack.");
    return std::move(restoredRouterStack_);
}

Ace::UIContent* WindowSessionImpl::GetUIContent() const
{
    std::shared_lock<std::shared_mutex> lock(uiContentMutex_);
    return uiContent_.get();
}

std::shared_ptr<Ace::UIContent> WindowSessionImpl::GetUIContentSharedPtr() const
{
    std::shared_lock<std::shared_mutex> lock(uiContentMutex_);
    return uiContent_;
}

Ace::UIContent* WindowSessionImpl::GetUIContentWithId(uint32_t winId) const
{
    sptr<Window> targetWindow = FindWindowById(winId);
    if (targetWindow == nullptr) {
        WLOGE("target window is null");
        return nullptr;
    }
    return targetWindow->GetUIContent();
}

void WindowSessionImpl::OnNewWant(const AAFwk::Want& want)
{
    WLOGFI("[name:%{public}s, id:%{public}d]",
        property_->GetWindowName().c_str(), GetPersistentId());
    navDestinationInfo_ = want.GetStringParam(AAFwk::Want::ATOMIC_SERVICE_SHARE_ROUTER);
    if (!navDestinationInfo_.empty()) {
        auto uiContent = GetUIContentSharedPtr();
        if (uiContent != nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "call uiContent RestoreNavDestinationInfo.");
            uiContent->RestoreNavDestinationInfo(navDestinationInfo_, false);
            navDestinationInfo_ = "";
        } else {
            TLOGE(WmsLogTag::WMS_LIFE, "uiContent is nullptr.");
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->OnNewWant(want);
    }
}

WMError WindowSessionImpl::SetAPPWindowLabel(const std::string& label)
{
    TLOGI(WmsLogTag::WMS_DECOR, "Enter");
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGI(WmsLogTag::WMS_DECOR, "uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent->SetAppWindowTitle(label);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon)
{
    if (icon == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "icon is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uicontent is empty, cache icon");
        // The next time setUIContent is called, use iconCache to set windowIcon
        iconCache_ = icon;
        return WMError::WM_OK;
    }
    uiContent->SetAppWindowIcon(icon);
    TLOGD(WmsLogTag::WMS_DECOR, "end");
    return WMError::WM_OK;
}

WMError WindowSessionImpl::RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    return RegisterListener(lifecycleListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterWindowStageLifeCycleListener(const sptr<IWindowStageLifeCycle>& listener)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
    return RegisterListener(windowStageLifecycleListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWindowStageLifeCycleListener(const sptr<IWindowStageLifeCycle>& listener)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
    return UnregisterListener(windowStageLifecycleListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::mutex> lockListener(displayMoveListenerMutex_);
    return RegisterListener(displayMoveListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::mutex> lockListener(displayMoveListenerMutex_);
    return UnregisterListener(displayMoveListeners_[GetPersistentId()], listener);
}

bool WindowSessionImpl::IsWindowShouldDrag()
{
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow() &&
        !windowSystemConfig_.IsPhoneWindow()) {
        return false;
    }
    return true;
}

bool WindowSessionImpl::CheckCanDragWindowType()
{
    WindowType windowType = GetType();
    if (WindowHelper::IsSystemWindow(windowType) || WindowHelper::IsSubWindow(windowType)) {
        return true;
    }
    return false;
}

/**
 * Currently only supports system windows.
 */
WMError WindowSessionImpl::EnableDrag(bool enableDrag)
{
    if (!IsWindowShouldDrag()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "The device is not supported");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!CheckCanDragWindowType()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, invalid window type:%{public}u",
            GetPersistentId(), GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    property_->SetDragEnabled(enableDrag);
    hasSetEnableDrag_.store(true);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    WMError errorCode = hostSession->SetSystemWindowEnableDrag(enableDrag);
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, enableDrag:%{public}d, errcode:%{public}d",
        GetPersistentId(), enableDrag, static_cast<int>(errorCode));
    return static_cast<WMError>(errorCode);
}

WMError WindowSessionImpl::RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(occupiedAreaChangeListenerMutex_);
    return RegisterListener(occupiedAreaChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(occupiedAreaChangeListenerMutex_);
    return UnregisterListener(occupiedAreaChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterKeyboardWillShowListener(const sptr<IKBWillShowListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    if (!(windowSystemConfig_.supportFunctionType_ & ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Keyboard will animtion notification is not allowed");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    std::lock_guard<std::recursive_mutex> lockListener(keyboardWillShowListenerMutex_);
    WMError ret = RegisterListener(keyboardWillShowListeners_[GetPersistentId()], listener);
    if (ret == WMError::WM_OK && property_->EditSessionInfo().isKeyboardWillShowRegistered_ == false) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardWillShowRegistered(true);
        property_->EditSessionInfo().isKeyboardWillShowRegistered_ = true;
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterKeyboardWillShowListener(const sptr<IKBWillShowListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    std::lock_guard<std::recursive_mutex> lockListener(keyboardWillShowListenerMutex_);
    WMError ret = UnregisterListener(keyboardWillShowListeners_[GetPersistentId()], listener);
    if (keyboardWillShowListeners_[GetPersistentId()].empty()) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardWillShowRegistered(false);
        property_->EditSessionInfo().isKeyboardWillShowRegistered_ = false;
    }
    return ret;
}

WMError WindowSessionImpl::RegisterKeyboardWillHideListener(const sptr<IKBWillHideListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    if (!(windowSystemConfig_.supportFunctionType_ & ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Keyboard will animtion notification is not allowed");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    std::lock_guard<std::recursive_mutex> lockListener(keyboardWillHideListenerMutex_);
    WMError ret = RegisterListener(keyboardWillHideListeners_[GetPersistentId()], listener);
    if (ret == WMError::WM_OK && property_->EditSessionInfo().isKeyboardWillHideRegistered_ == false) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardWillHideRegistered(true);
        property_->EditSessionInfo().isKeyboardWillHideRegistered_ = true;
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterKeyboardWillHideListener(const sptr<IKBWillHideListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    std::lock_guard<std::recursive_mutex> lockListener(keyboardWillHideListenerMutex_);
    WMError ret = UnregisterListener(keyboardWillHideListeners_[GetPersistentId()], listener);
    if (keyboardWillHideListeners_[GetPersistentId()].empty()) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardWillHideRegistered(false);
        property_->EditSessionInfo().isKeyboardWillHideRegistered_ = false;
    }
    return ret;
}

WMError WindowSessionImpl::RegisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    std::lock_guard<std::recursive_mutex> lockListener(keyboardDidShowListenerMutex_);
    WMError ret = RegisterListener(keyboardDidShowListeners_[GetPersistentId()], listener);
    if (ret == WMError::WM_OK && property_->EditSessionInfo().isKeyboardDidShowRegistered_ == false) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardDidShowRegistered(true);
        property_->EditSessionInfo().isKeyboardDidShowRegistered_ = true;
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    std::lock_guard<std::recursive_mutex> lockListener(keyboardDidShowListenerMutex_);
    WMError ret = UnregisterListener(keyboardDidShowListeners_[GetPersistentId()], listener);
    if (keyboardDidShowListeners_[GetPersistentId()].empty()) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardDidShowRegistered(false);
        property_->EditSessionInfo().isKeyboardDidShowRegistered_ = false;
    }
    return ret;
}

WMError WindowSessionImpl::RegisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    std::lock_guard<std::recursive_mutex> lockListener(keyboardDidHideListenerMutex_);
    WMError ret = RegisterListener(keyboardDidHideListeners_[GetPersistentId()], listener);
    if (ret == WMError::WM_OK && property_->EditSessionInfo().isKeyboardDidHideRegistered_ == false) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardDidHideRegistered(true);
        property_->EditSessionInfo().isKeyboardDidHideRegistered_ = true;
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    std::lock_guard<std::recursive_mutex> lockListener(keyboardDidHideListenerMutex_);
    WMError ret = UnregisterListener(keyboardDidHideListeners_[GetPersistentId()], listener);
    if (keyboardDidHideListeners_[GetPersistentId()].empty()) {
        auto hostSession = GetHostSession();
        CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
        hostSession->NotifyKeyboardDidHideRegistered(false);
        property_->EditSessionInfo().isKeyboardDidHideRegistered_ = false;
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    return UnregisterListener(lifecycleListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowChangeListenerMutex_);
    return RegisterListener(windowChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowChangeListenerMutex_);
    return UnregisterListener(windowChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterWindowCrossAxisListener(const sptr<IWindowCrossAxisListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowCrossAxisListenerMutex_);
    return RegisterListener(windowCrossAxisListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWindowCrossAxisListener(const sptr<IWindowCrossAxisListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowCrossAxisListenerMutex_);
    return UnregisterListener(windowCrossAxisListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStatusChangeListenerMutex_);
    return RegisterListener(windowStatusChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStatusChangeListenerMutex_);
    return UnregisterListener(windowStatusChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterWindowStatusDidChangeListener(const sptr<IWindowStatusDidChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStatusDidChangeListenerMutex_);
    return RegisterListener(windowStatusDidChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWindowStatusDidChangeListener(const sptr<IWindowStatusDidChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStatusDidChangeListenerMutex_);
    return UnregisterListener(windowStatusDidChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::SetDecorVisible(bool isVisible)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uicontent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent->SetContainerModalTitleVisible(isVisible, true);
    handler_->PostTask([weakWindow = wptr(this), isVisible, where = __func__] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s: window is null", where);
            return;
        }
        if (auto hostSession = window->GetHostSession()) {
            hostSession->SetDecorVisible(isVisible);
        }
    }, __func__);
    TLOGD(WmsLogTag::WMS_DECOR, "end");
    return WMError::WM_OK;
}

WMError WindowSessionImpl::GetDecorVisible(bool& isVisible)
{
    TLOGD(WmsLogTag::WMS_DECOR, "%{public}u in", GetWindowId());
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uicontent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    isVisible = uiContent->GetContainerModalTitleVisible(GetImmersiveModeEnabledState());
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetWindowTitleMoveEnabled(bool enable)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType()) && !WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "called by invalid window type, type:%{public}d", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uicontent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent->EnableContainerModalGesture(enable);
    TLOGI(WmsLogTag::WMS_DECOR, "enable:%{public}d end", enable);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetSubWindowModal(bool isModal, ModalityType modalityType)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_SUB, "called by invalid window type, type:%{public}d", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (modalityType == ModalityType::APPLICATION_MODALITY && IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "This is PcAppInPad, not support");
        return WMError::WM_OK;
    }
    if (modalityType == ModalityType::APPLICATION_MODALITY && !IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    WMError modalRet = isModal ?
        AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL) :
        RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    if (modalRet != WMError::WM_OK) {
        return modalRet;
    }
    modalRet = isModal && modalityType == ModalityType::APPLICATION_MODALITY ?
        AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL) :
        RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    SubWindowModalType subWindowModalType = SubWindowModalType::TYPE_NORMAL;
    if (isModal) {
        subWindowModalType = modalityType == ModalityType::WINDOW_MODALITY ?
            SubWindowModalType::TYPE_WINDOW_MODALITY :
            SubWindowModalType::TYPE_APPLICATION_MODALITY;
    }
    if (!(property_->GetSubWindowZLevel() <= MAXIMUM_Z_LEVEL && !isModal)) {
        int32_t zLevel = GetSubWindowZLevelByFlags(GetType(), GetWindowFlags(), IsTopmost());
        property_->SetSubWindowZLevel(zLevel);
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL);
    }
    hostSession->NotifySubModalTypeChange(subWindowModalType);
    return modalRet;
}

WMError WindowSessionImpl::SetWindowModal(bool isModal)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_MAIN, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_MAIN, "called by invalid window type, type:%{public}d", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    WMError modalRet = isModal ?
        AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL) :
        RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    if (modalRet != WMError::WM_OK) {
        return modalRet;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    hostSession->NotifyMainModalTypeChange(isModal);
    return modalRet;
}

WMError WindowSessionImpl::SetDecorHeight(int32_t decorHeight)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    float vpr = 0.f;
    auto err = GetVirtualPixelRatio(vpr);
    if (err != WMError::WM_OK) {
        return err;
    }
    int32_t decorHeightWithPx = static_cast<int32_t>(decorHeight * vpr);
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContent->SetContainerModalTitleHeight(decorHeightWithPx);

    if (auto hostSession = GetHostSession()) {
        hostSession->SetCustomDecorHeight(decorHeight);
    }
    decorHeight_ = decorHeight;
    TLOGD(WmsLogTag::WMS_DECOR, "end, height: %{public}d", decorHeight);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::GetDecorHeight(int32_t& height)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uiContent is null, windowId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }
    height = uiContent->GetContainerModalTitleHeight();
    if (height == -1) {
        height = 0;
        TLOGD(WmsLogTag::WMS_DECOR, "Get app window decor height failed");
        return WMError::WM_OK;
    }
    float vpr = 0.f;
    auto err = GetVirtualPixelRatio(vpr);
    if (err != WMError::WM_OK) {
        return err;
    }
    height = static_cast<int32_t>(height / vpr);
    TLOGD(WmsLogTag::WMS_DECOR, "end, height: %{public}d", height);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetDecorButtonStyle(const DecorButtonStyle& decorButtonStyle)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!WindowHelper::CheckButtonStyleValid(decorButtonStyle)) {
        TLOGE(WmsLogTag::WMS_DECOR, "set decor button style param invalid");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (!WindowHelper::IsMainWindow(GetType()) && !WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "called by invalid window type, type:%{public}d", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }

    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uiContent is null, windowId: %{public}u", GetWindowId());
        return WMError::WM_ERROR_NULLPTR;
    }
    nlohmann::json decorJson = WindowSessionImpl::SetContainerButtonStyle(decorButtonStyle);
    auto decorJsonStr =  decorJson.dump();
    TLOGI(WmsLogTag::WMS_DECOR, "decorJsonStr: %{public}s", decorJsonStr.c_str());
    uiContent->OnContainerModalEvent(DECOR_BUTTON_STYLE_CHANGE, decorJsonStr);
    decorButtonStyle_ = decorButtonStyle;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::GetDecorButtonStyle(DecorButtonStyle& decorButtonStyle)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!WindowHelper::IsMainWindow(GetType()) && !WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "called by invalid window type, type:%{public}d", GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    decorButtonStyle = decorButtonStyle_;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::GetTitleButtonArea(TitleButtonRect& titleButtonRect)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    Rect decorRect;
    Rect titleButtonLeftRect;
    bool res = false;
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uicontent is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    res = uiContent->GetContainerModalButtonsRect(decorRect, titleButtonLeftRect);
    if (!res) {
        TLOGE(WmsLogTag::WMS_DECOR, "GetContainerModalButtonsRect failed");
        return WMError::WM_OK;
    }
    float vpr = 0.f;
    auto err = GetVirtualPixelRatio(vpr);
    if (err != WMError::WM_OK) {
        return err;
    }
    titleButtonRect.posX_ = static_cast<int32_t>(decorRect.width_) -
        static_cast<int32_t>(titleButtonLeftRect.width_) - titleButtonLeftRect.posX_;
    titleButtonRect.posX_ = static_cast<int32_t>(titleButtonRect.posX_ / vpr);
    titleButtonRect.posY_ = static_cast<int32_t>(titleButtonLeftRect.posY_ / vpr);
    titleButtonRect.width_ = static_cast<uint32_t>(titleButtonLeftRect.width_ / vpr);
    titleButtonRect.height_ = static_cast<uint32_t>(titleButtonLeftRect.height_ / vpr);
    return WMError::WM_OK;
}

WSError WindowSessionImpl::GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "uiContent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    uiContentRemoteObj = uiContent->GetRemoteObj();
    return WSError::WS_OK;
}

WMError WindowSessionImpl::RegisterWindowTitleButtonRectChangeListener(
    const sptr<IWindowTitleButtonRectChangedListener>& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto persistentId = GetPersistentId();
    TLOGD(WmsLogTag::WMS_DECOR, "Start, id:%{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    {
        std::lock_guard<std::recursive_mutex> lockListener(windowTitleButtonRectChangeListenerMutex_);
        WMError ret = RegisterListener(windowTitleButtonRectChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_DECOR, "register failed");
            return ret;
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        const char* const where = __func__;
        uiContent->SubscribeContainerModalButtonsRectChange(
            [where, weakThis = wptr(this)](Rect& decorRect, Rect& titleButtonLeftRect) {
            auto window = weakThis.promote();
            if (!window) {
                TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s window is null", where);
                return;
            }
            float vpr = 0.f;
            auto err = window->GetVirtualPixelRatio(vpr);
            if (err != WMError::WM_OK) {
                return;
            }
            TitleButtonRect titleButtonRect;
            titleButtonRect.posX_ = static_cast<int32_t>(decorRect.width_) -
                static_cast<int32_t>(titleButtonLeftRect.width_) - titleButtonLeftRect.posX_;
            titleButtonRect.posX_ = static_cast<int32_t>(titleButtonRect.posX_ / vpr);
            titleButtonRect.posY_ = static_cast<int32_t>(titleButtonLeftRect.posY_ / vpr);
            titleButtonRect.width_ = static_cast<uint32_t>(titleButtonLeftRect.width_ / vpr);
            titleButtonRect.height_ = static_cast<uint32_t>(titleButtonLeftRect.height_ / vpr);
            window->NotifyWindowTitleButtonRectChange(titleButtonRect);
        });
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterWindowTitleButtonRectChangeListener(
    const sptr<IWindowTitleButtonRectChangedListener>& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WMError ret = WMError::WM_OK;
    auto persistentId = GetPersistentId();
    TLOGD(WmsLogTag::WMS_DECOR, "Start, id:%{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowTitleButtonRectChangeListenerMutex_);
        ret = UnregisterListener(windowTitleButtonRectChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_DECOR, "failed");
            return ret;
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SubscribeContainerModalButtonsRectChange(nullptr);
    }
    return ret;
}

template<typename T>
EnableIfSame<T, IWindowTitleButtonRectChangedListener,
    std::vector<sptr<IWindowTitleButtonRectChangedListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowTitleButtonRectChangedListener>> windowTitleButtonRectListeners;
        for (auto& listener : windowTitleButtonRectChangeListeners_[GetPersistentId()]) {
            windowTitleButtonRectListeners.push_back(listener);
        }
    return windowTitleButtonRectListeners;
}

void WindowSessionImpl::NotifyWindowTitleButtonRectChange(TitleButtonRect titleButtonRect)
{
    std::lock_guard<std::recursive_mutex> lockListener(windowTitleButtonRectChangeListenerMutex_);
    auto windowTitleButtonRectListeners = GetListeners<IWindowTitleButtonRectChangedListener>();
    for (auto& listener : windowTitleButtonRectListeners) {
        if (listener != nullptr) {
            listener->OnWindowTitleButtonRectChanged(titleButtonRect);
        }
    }
}

void WindowSessionImpl::UpdateRectChangeListenerRegisterStatus()
{
    auto windowId = GetPersistentId();
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "hostSession is null, windowId: %{public}d", windowId);
        return;
    }

    auto isRegisteredIn = [windowId](auto& listeners, auto& mutex) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = listeners.find(windowId);
        return it != listeners.end() && !it->second.empty();
    };
    bool isRegistered = isRegisteredIn(windowRectChangeListeners_, windowRectChangeListenerMutex_) ||
                        isRegisteredIn(rectChangeInGlobalDisplayListeners_, rectChangeInGlobalDisplayListenerMutex_);

    auto ret = hostSession->UpdateRectChangeListenerRegistered(isRegistered);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed, ret: %{public}d, windowId: %{public}d, isRegistered: %{public}d",
              static_cast<int32_t>(ret), windowId, isRegistered);
    }
}

template<typename T>
EnableIfSame<T, IWindowRectChangeListener,
    std::vector<sptr<IWindowRectChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowRectChangeListener>> windowRectChangeListeners;
    for (auto& listener : windowRectChangeListeners_[GetPersistentId()]) {
        windowRectChangeListeners.push_back(listener);
    }
    return windowRectChangeListeners;
}

WMError WindowSessionImpl::RegisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener)
{
    WMError ret = WMError::WM_DO_NOTHING;
    {
        std::lock_guard<std::mutex> lockListener(windowRectChangeListenerMutex_);
        ret = RegisterListener(windowRectChangeListeners_[GetPersistentId()], listener);
    }
    if (ret == WMError::WM_OK) {
        UpdateRectChangeListenerRegisterStatus();
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener)
{
    WMError ret = WMError::WM_DO_NOTHING;
    {
        std::lock_guard<std::mutex> lockListener(windowRectChangeListenerMutex_);
        ret = UnregisterListener(windowRectChangeListeners_[GetPersistentId()], listener);
    }
    if (ret == WMError::WM_OK) {
        UpdateRectChangeListenerRegisterStatus();
    }
    return ret;
}

template<typename T>
EnableIfSame<T, IRectChangeInGlobalDisplayListener,
    std::vector<sptr<IRectChangeInGlobalDisplayListener>>> WindowSessionImpl::GetListeners()
{
    std::lock_guard<std::mutex> lock(rectChangeInGlobalDisplayListenerMutex_);
    auto it = rectChangeInGlobalDisplayListeners_.find(GetPersistentId());
    if (it != rectChangeInGlobalDisplayListeners_.end()) {
        return it->second;
    }
    return {};
}

WMError WindowSessionImpl::RegisterRectChangeInGlobalDisplayListener(
    const sptr<IRectChangeInGlobalDisplayListener>& listener)
{
    WMError ret = WMError::WM_DO_NOTHING;
    {
        std::lock_guard<std::mutex> lock(rectChangeInGlobalDisplayListenerMutex_);
        ret = RegisterListener(rectChangeInGlobalDisplayListeners_[GetPersistentId()], listener);
    }
    if (ret == WMError::WM_OK) {
        UpdateRectChangeListenerRegisterStatus();
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterRectChangeInGlobalDisplayListener(
    const sptr<IRectChangeInGlobalDisplayListener>& listener)
{
    WMError ret = WMError::WM_DO_NOTHING;
    {
        std::lock_guard<std::mutex> lock(rectChangeInGlobalDisplayListenerMutex_);
        ret = UnregisterListener(rectChangeInGlobalDisplayListeners_[GetPersistentId()], listener);
    }
    if (ret == WMError::WM_OK) {
        UpdateRectChangeListenerRegisterStatus();
    }
    return ret;
}

template<typename T>
EnableIfSame<T, IExtensionSecureLimitChangeListener,
    std::vector<sptr<IExtensionSecureLimitChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IExtensionSecureLimitChangeListener>> secureLimitChangeListeners;
    for (auto& listener : secureLimitChangeListeners_[GetPersistentId()]) {
        secureLimitChangeListeners.push_back(listener);
    }
    return secureLimitChangeListeners;
}

WMError WindowSessionImpl::RegisterExtensionSecureLimitChangeListener(
    const sptr<IExtensionSecureLimitChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "name=%{public}s, id=%{public}u", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(secureLimitChangeListenerMutex_);
    return RegisterListener(secureLimitChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterExtensionSecureLimitChangeListener(
    const sptr<IExtensionSecureLimitChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "name=%{public}s, id=%{public}u", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(secureLimitChangeListenerMutex_);
    return UnregisterListener(secureLimitChangeListeners_[GetPersistentId()], listener);
}

template<typename T>
EnableIfSame<T, ISubWindowCloseListener, sptr<ISubWindowCloseListener>> WindowSessionImpl::GetListeners()
{
    sptr<ISubWindowCloseListener> subWindowCloseListeners;
    subWindowCloseListeners = subWindowCloseListeners_[GetPersistentId()];
    return subWindowCloseListeners;
}

WMError WindowSessionImpl::RegisterSubWindowCloseListeners(const sptr<ISubWindowCloseListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsSubWindow(GetType()) && !WindowHelper::IsSystemSubWindow(GetType())) {
        WLOGFE("window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::lock_guard<std::mutex> lockListener(subWindowCloseListenersMutex_);
    subWindowCloseListeners_[GetPersistentId()] = listener;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterSubWindowCloseListeners(const sptr<ISubWindowCloseListener>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsSubWindow(GetType()) && !WindowHelper::IsSystemSubWindow(GetType())) {
        WLOGFE("window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::lock_guard<std::mutex> lockListener(subWindowCloseListenersMutex_);
    subWindowCloseListeners_[GetPersistentId()] = nullptr;
    return WMError::WM_OK;
}

template<typename T>
EnableIfSame<T, IWindowHighlightChangeListener, std::vector<sptr<IWindowHighlightChangeListener>>>
    WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowHighlightChangeListener>> highlightChangeListeners;
    for (auto& listener : highlightChangeListeners_[GetPersistentId()]) {
        highlightChangeListeners.push_back(listener);
    }
    return highlightChangeListeners;
}

WMError WindowSessionImpl::RegisterWindowHighlightChangeListeners(const sptr<IWindowHighlightChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "name=%{public}s, id=%{public}u", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(highlightChangeListenerMutex_);
    return RegisterListener(highlightChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWindowHighlightChangeListeners(
    const sptr<IWindowHighlightChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "name=%{public}s, id=%{public}u", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(highlightChangeListenerMutex_);
    return UnregisterListener(highlightChangeListeners_[GetPersistentId()], listener);
}

template<typename T>
EnableIfSame<T, IMainWindowCloseListener, sptr<IMainWindowCloseListener>> WindowSessionImpl::GetListeners()
{
    return mainWindowCloseListeners_[GetPersistentId()];
}

template<typename T>
EnableIfSame<T, ISystemBarPropertyListener,
    std::vector<sptr<ISystemBarPropertyListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<ISystemBarPropertyListener>> listeners;
    for (auto& listener : systemBarPropertyListeners_[GetPersistentId()]) {
        listeners.push_back(listener);
    }
    return listeners;
}

WMError WindowSessionImpl::RegisterSystemBarPropertyListener(const sptr<ISystemBarPropertyListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(systemBarPropertyListenerMutex_);
    return RegisterListener(systemBarPropertyListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterSystemBarPropertyListener(const sptr<ISystemBarPropertyListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(systemBarPropertyListenerMutex_);
    return UnregisterListener(systemBarPropertyListeners_[GetPersistentId()], listener);
}

void WindowSessionImpl::NotifySystemBarPropertyUpdate(WindowType type, const SystemBarProperty& property)
{
    std::lock_guard<std::mutex> lockListener(systemBarPropertyListenerMutex_);
    auto listeners = GetListeners<ISystemBarPropertyListener>();
    for (auto& listener : listeners) {
        if (listener != nullptr) {
            listener->OnSystemBarPropertyUpdate(type, property);
        }
    }
}

WMError WindowSessionImpl::RegisterMainWindowCloseListeners(const sptr<IMainWindowCloseListener>& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_PC, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::lock_guard<std::mutex> lockListener(mainWindowCloseListenersMutex_);
    mainWindowCloseListeners_[GetPersistentId()] = listener;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterMainWindowCloseListeners(const sptr<IMainWindowCloseListener>& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_PC, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::lock_guard<std::mutex> lockListener(mainWindowCloseListenersMutex_);
    mainWindowCloseListeners_[GetPersistentId()] = nullptr;
    return WMError::WM_OK;
}

template<typename T>
EnableIfSame<T, IWindowWillCloseListener, std::vector<sptr<IWindowWillCloseListener>>> WindowSessionImpl::GetListeners()
{
    return windowWillCloseListeners_[GetPersistentId()];
}

WMError WindowSessionImpl::RegisterWindowWillCloseListeners(const sptr<IWindowWillCloseListener>& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsAppWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::lock_guard<std::recursive_mutex> lockListener(windowWillCloseListenersMutex_);
    return RegisterListener(windowWillCloseListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnRegisterWindowWillCloseListeners(const sptr<IWindowWillCloseListener>& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsAppWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    std::lock_guard<std::recursive_mutex> lockListener(windowWillCloseListenersMutex_);
    return UnregisterListener(windowWillCloseListeners_[GetPersistentId()], listener);
}

template<typename T>
EnableIfSame<T, IWindowTitleChangeListener, std::vector<sptr<IWindowTitleChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowTitleChangeListener>> windowTitleChangeListeners;
    std::lock_guard<std::mutex> lockRectListener(windowTitleChangeListenerMutex_);
    for (auto& listener : windowTitleChangeListeners_[GetPersistentId()]) {
        windowTitleChangeListeners.push_back(listener);
    }
    return windowTitleChangeListeners;
}
 
WMError WindowSessionImpl::RegisterWindowTitleChangeListener(const sptr<IWindowTitleChangeListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(windowTitleChangeListenerMutex_);
    WMError ret = RegisterListener(windowTitleChangeListeners_[GetPersistentId()], listener);
    TLOGI(WmsLogTag::WMS_DECOR, "RegisterWindowTitleChangeListener");
    return ret;
}
 
WMError WindowSessionImpl::UnregisterWindowTitleChangeListener(const sptr<IWindowTitleChangeListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(windowTitleChangeListenerMutex_);
    WMError ret = UnregisterListener(windowTitleChangeListeners_[GetPersistentId()], listener);
    TLOGI(WmsLogTag::WMS_DECOR, "UnregisterWindowTitleChangeListener");
    return ret;
}

template<typename T>
EnableIfSame<T, IWindowTitleOrHotAreasListener, std::vector<sptr<IWindowTitleOrHotAreasListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowTitleOrHotAreasListener>> windowTitleOrHotAreasListeners;
    std::lock_guard<std::mutex> lockListener(windowTitleOrHotAreasListenerMutex_);
    for (auto& listener : windowTitleOrHotAreasListeners_[GetPersistentId()]) {
        windowTitleOrHotAreasListeners.push_back(listener);
    }
    return windowTitleOrHotAreasListeners;
}
 
WMError WindowSessionImpl::RegisterWindowTitleOrHotAreasListener(const sptr<IWindowTitleOrHotAreasListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(windowTitleChangeListenerMutex_);
    WMError ret = RegisterListener(windowTitleOrHotAreasListeners_[GetPersistentId()], listener);
    TLOGI(WmsLogTag::WMS_DECOR, "RegisterWindowTitleOrHotAreasListener");
    return ret;
}
 
WMError WindowSessionImpl::UnregisterWindowTitleOrHotAreasListener(const sptr<IWindowTitleOrHotAreasListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(windowTitleOrHotAreasListenerMutex_);
    WMError ret = UnregisterListener(windowTitleOrHotAreasListeners_[GetPersistentId()], listener);
    TLOGI(WmsLogTag::WMS_DECOR, "UnregisterWindowTitleOrHotAreasListener");
    return ret;
}

template<typename T>
EnableIfSame<T, ISwitchFreeMultiWindowListener,
    std::vector<sptr<ISwitchFreeMultiWindowListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<ISwitchFreeMultiWindowListener>> switchFreeMultiWindowListeners;
    for (auto& listener : switchFreeMultiWindowListeners_[GetPersistentId()]) {
        switchFreeMultiWindowListeners.push_back(listener);
    }
    return switchFreeMultiWindowListeners;
}

WMError WindowSessionImpl::RegisterSwitchFreeMultiWindowListener(const sptr<ISwitchFreeMultiWindowListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsMainWindow(GetType()) && !WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "Start register");
    std::lock_guard<std::mutex> lockListener(switchFreeMultiWindowListenerMutex_);
    return RegisterListener(switchFreeMultiWindowListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterSwitchFreeMultiWindowListener(const sptr<ISwitchFreeMultiWindowListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsMainWindow(GetType()) && !WindowHelper::IsSubWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "window type is not supported");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "Start unregister");
    std::lock_guard<std::mutex> lockListener(switchFreeMultiWindowListenerMutex_);
    return UnregisterListener(switchFreeMultiWindowListeners_[GetPersistentId()], listener);
}

void WindowSessionImpl::RecoverSessionListener()
{
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_RECOVER, "with persistentId=%{public}d", persistentId);
    {
        std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
        if (avoidAreaChangeListeners_.find(persistentId) != avoidAreaChangeListeners_.end() &&
            !avoidAreaChangeListeners_[persistentId].empty()) {
            SingletonContainer::Get<WindowAdapter>().UpdateSessionAvoidAreaListener(persistentId, true);
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(touchOutsideListenerMutex_);
        if (touchOutsideListeners_.find(persistentId) != touchOutsideListeners_.end() &&
            !touchOutsideListeners_[persistentId].empty()) {
            SingletonContainer::Get<WindowAdapter>().UpdateSessionTouchOutsideListener(persistentId, true);
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowVisibilityChangeListenerMutex_);
        if (windowVisibilityChangeListeners_.find(persistentId) != windowVisibilityChangeListeners_.end() &&
            !windowVisibilityChangeListeners_[persistentId].empty()) {
            SingletonContainer::Get<WindowAdapter>().UpdateSessionWindowVisibilityListener(persistentId, true);
        }
    }
    {
        bool hasListener = false;
        {
            std::lock_guard<std::mutex> lockListener(occlusionStateChangeListenerMutex_);
            hasListener = occlusionStateChangeListeners_.count(persistentId) > 0 &&
                !occlusionStateChangeListeners_[persistentId].empty();
        }
        if (hasListener) {
            SingletonContainer::Get<WindowAdapter>().UpdateSessionOcclusionStateListener(persistentId, true);
        }
    }
    UpdateRectChangeListenerRegisterStatus();
    {
        std::lock_guard<std::mutex> lockListener(windowRotationChangeListenerMutex_);
        if (windowRotationChangeListeners_.find(persistentId) != windowRotationChangeListeners_.end() &&
            !windowRotationChangeListeners_[persistentId].empty()) {
            if (auto hostSession = GetHostSession()) {
                hostSession->UpdateRotationChangeRegistered(persistentId, true);
            }
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(screenshotAppEventListenerMutex_);
        if (screenshotAppEventListeners_.find(persistentId) != screenshotAppEventListeners_.end() &&
            !screenshotAppEventListeners_[persistentId].empty()) {
            if (auto hostSession = GetHostSession()) {
                hostSession->UpdateScreenshotAppEventRegistered(persistentId, true);
            }
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(acrossDisplaysChangeListenerMutex_);
        if (acrossDisplaysChangeListeners_.find(persistentId) != acrossDisplaysChangeListeners_.end() &&
            !acrossDisplaysChangeListeners_[persistentId].empty()) {
            if (auto hostSession = GetHostSession()) {
                hostSession->UpdateAcrossDisplaysChangeRegistered(true);
            }
        }
    }
}

template<typename T>
EnableIfSame<T, IWindowLifeCycle, std::vector<sptr<IWindowLifeCycle>>> WindowSessionImpl::GetListeners()
{
    auto iter = lifecycleListeners_.find(GetPersistentId());
    if (iter == lifecycleListeners_.end()) {
        return std::vector<sptr<IWindowLifeCycle>>();
    }
    std::vector<sptr<IWindowLifeCycle>> lifecycleListeners;
    for (auto& listener : iter->second) {
        lifecycleListeners.push_back(listener);
    }
    return lifecycleListeners;
}

template<typename T>
EnableIfSame<T, IWindowStageLifeCycle, std::vector<sptr<IWindowStageLifeCycle>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowStageLifeCycle>> windowStageLifecycleListeners;
    for (auto& listener : windowStageLifecycleListeners_[GetPersistentId()]) {
        windowStageLifecycleListeners.push_back(listener);
    }
    return windowStageLifecycleListeners;
}

template<typename T>
EnableIfSame<T, IWindowChangeListener, std::vector<sptr<IWindowChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowChangeListener>> windowChangeListeners;
    for (auto& listener : windowChangeListeners_[GetPersistentId()]) {
        windowChangeListeners.push_back(listener);
    }
    return windowChangeListeners;
}

template<typename T>
EnableIfSame<T, IWindowCrossAxisListener, std::vector<sptr<IWindowCrossAxisListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowCrossAxisListener>> windowCrossAxisListeners;
    for (const auto& listener : windowCrossAxisListeners_[GetPersistentId()]) {
        windowCrossAxisListeners.push_back(listener);
    }
    return windowCrossAxisListeners;
}

void WindowSessionImpl::NotifyWindowCrossAxisChange(CrossAxisState state)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d, cross axis state %{public}u", GetPersistentId(),
        static_cast<uint32_t>(state));
    crossAxisState_ = state;
    AAFwk::Want want;
    want.SetParam(Extension::CROSS_AXIS_FIELD, static_cast<int32_t>(state));
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_CROSS_AXIS_STATE),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    std::lock_guard<std::recursive_mutex> lockListener(windowCrossAxisListenerMutex_);
    auto windowCrossAxisListeners = GetListeners<IWindowCrossAxisListener>();
    for (const auto& listener : windowCrossAxisListeners) {
        if (listener != nullptr) {
            listener->OnCrossAxisChange(state);
        }
    }
}

bool WindowSessionImpl::IsHitTitleBar(std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
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
    int32_t decorHeight = uiContent->GetContainerModalTitleHeight();
    int32_t statusBarHeight = property_->GetStatusBarHeightInImmersive();
    MMI::PointerEvent::PointerItem pointerItem;
    bool isValidPointItem = pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    bool isHitTitleBarX = pointerItem.GetDisplayX() > windowRect.posX_
        && pointerItem.GetDisplayX() < windowRect.posX_ + windowRect.width_;
    bool isHitTitleBarY = pointerItem.GetDisplayY() > windowRect.posY_ + statusBarHeight
        && pointerItem.GetDisplayY() < windowRect.posY_ + decorHeight + statusBarHeight;
    bool isHitTitleBar = isValidPointItem && isHitTitleBarX && isHitTitleBarY;
    if (isHitTitleBar) {
        TLOGI(WmsLogTag::WMS_DECOR, "hitTitleBar success");
        return true;
    }
    return false;
}

bool WindowSessionImpl::IsWaterfallModeEnabled()
{
    if (!isValidWaterfallMode_.load() && InitWaterfallMode()) {
        isValidWaterfallMode_.store(true);
    }
    bool isWaterfallMode = isFullScreenWaterfallMode_.load();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "waterfall: %{public}d, winId: %{public}u", isWaterfallMode, GetWindowId());
    return isWaterfallMode;
}

bool WindowSessionImpl::InitWaterfallMode()
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is invalid");
        return false;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, false);
    bool isWaterfallMode = false;
    if (hostSession->GetWaterfallMode(isWaterfallMode) != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed winId: %{public}u", GetWindowId());
        return false;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u", GetWindowId());
    isFullScreenWaterfallMode_.store(isWaterfallMode);
    NotifyWaterfallModeChange(isWaterfallMode);
    return true;
}

WMError WindowSessionImpl::RegisterWaterfallModeChangeListener(const sptr<IWaterfallModeChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u", GetWindowId());
    std::lock_guard<std::mutex> lockListener(waterfallModeChangeListenerMutex_);
    return RegisterListener(waterfallModeChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterWaterfallModeChangeListener(const sptr<IWaterfallModeChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u", GetWindowId());
    std::lock_guard<std::mutex> lockListener(waterfallModeChangeListenerMutex_);
    return UnregisterListener(waterfallModeChangeListeners_[GetPersistentId()], listener);
}

std::vector<sptr<IWaterfallModeChangeListener>> WindowSessionImpl::GetWaterfallModeChangeListeners()
{
    std::vector<sptr<IWaterfallModeChangeListener>> listeners;
    std::lock_guard<std::mutex> lockListener(waterfallModeChangeListenerMutex_);
    for (auto& listener : waterfallModeChangeListeners_[GetPersistentId()]) {
        listeners.push_back(listener);
    }
    return listeners;
}

WMError WindowSessionImpl::NotifyAcrossDisplaysChange(bool isAcrossDisplays)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId:%{public}u,isFirst:%{public}u,last:%{public}u,curr:%{public}u",
        GetWindowId(), isFirstNotifyAcrossDisplays_, isAcrossDisplays_, isAcrossDisplays);
    if (!isFirstNotifyAcrossDisplays_ && isAcrossDisplays_ == isAcrossDisplays) {
        return WMError::WM_DO_NOTHING;
    }
    std::lock_guard<std::recursive_mutex> lock(acrossDisplaysChangeListenerMutex_);
    const auto& acrossMultiDisplayChangeListeners = GetListeners<IAcrossDisplaysChangeListener>();
    for (const auto& listener : acrossMultiDisplayChangeListeners) {
        if (listener != nullptr) {
            listener->OnAcrossDisplaysChanged(isAcrossDisplays);
        }
    }
    isFirstNotifyAcrossDisplays_ = false;
    isAcrossDisplays_ = isAcrossDisplays;
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifyWaterfallModeChange(bool isWaterfallMode)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u, waterfall: %{public}d, stat: %{public}u",
        GetWindowId(), isWaterfallMode, static_cast<uint32_t>(state_));
    if (state_ != WindowState::STATE_SHOWN) {
        return;
    }
    AAFwk::Want want;
    want.SetParam(Extension::WATERFALL_MODE_FIELD, isWaterfallMode);
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "send uiext winId: %{public}u", GetWindowId());
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_WATERFALL_MODE),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    auto waterfallModeChangeListeners = GetWaterfallModeChangeListeners();
    for (const auto& listener : waterfallModeChangeListeners) {
        if (listener != nullptr) {
            listener->OnWaterfallModeChange(isWaterfallMode);
        }
    }
}

template<typename T>
EnableIfSame<T, IOccupiedAreaChangeListener,
    std::vector<sptr<IOccupiedAreaChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IOccupiedAreaChangeListener>> occupiedAreaChangeListeners;
    for (auto& listener : occupiedAreaChangeListeners_[GetPersistentId()]) {
        occupiedAreaChangeListeners.push_back(listener);
    }
    return occupiedAreaChangeListeners;
}

template<typename T>
EnableIfSame<T, IKBWillShowListener, std::vector<sptr<IKBWillShowListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IKBWillShowListener>> keyboardWillShowListeners;
    for (auto& listener : keyboardWillShowListeners_[GetPersistentId()]) {
        keyboardWillShowListeners.push_back(listener);
    }
    return keyboardWillShowListeners;
}

template<typename T>
EnableIfSame<T, IKBWillHideListener, std::vector<sptr<IKBWillHideListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IKBWillHideListener>> keyboardWillHideListeners;
    for (auto& listener : keyboardWillHideListeners_[GetPersistentId()]) {
        keyboardWillHideListeners.push_back(listener);
    }
    return keyboardWillHideListeners;
}

template<typename T>
EnableIfSame<T, IKeyboardDidShowListener, std::vector<sptr<IKeyboardDidShowListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IKeyboardDidShowListener>> keyboardDidShowListeners;
    for (auto& listener : keyboardDidShowListeners_[GetPersistentId()]) {
        keyboardDidShowListeners.push_back(listener);
    }
    return keyboardDidShowListeners;
}

template<typename T>
EnableIfSame<T, IKeyboardDidHideListener, std::vector<sptr<IKeyboardDidHideListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IKeyboardDidHideListener>> keyboardDidHideListeners;
    for (auto& listener : keyboardDidHideListeners_[GetPersistentId()]) {
        keyboardDidHideListeners.push_back(listener);
    }
    return keyboardDidHideListeners;
}

template<typename T>
WMError WindowSessionImpl::RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        WLOGFE("already registered");
        return WMError::WM_OK;
    }
    holder.emplace_back(listener);
    return WMError::WM_OK;
}

template<typename T>
WMError WindowSessionImpl::UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        WLOGFE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    holder.erase(std::remove_if(holder.begin(), holder.end(),
        [listener](sptr<T> registeredListener) {
            return registeredListener == listener;
        }), holder.end());
    return WMError::WM_OK;
}

template<typename T>
void WindowSessionImpl::ClearUselessListeners(std::map<int32_t, T>& listeners, int32_t persistentId)
{
    listeners.erase(persistentId);
}

template<typename T>
void WindowSessionImpl::ClearUselessListeners(std::unordered_map<int32_t, T>& listeners, int32_t persistentId)
{
    listeners.erase(persistentId);
}

template<typename T>
EnableIfSame<T, IWindowStatusChangeListener, std::vector<sptr<IWindowStatusChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowStatusChangeListener>> windowStatusChangeListeners;
    for (auto& listener : windowStatusChangeListeners_[GetPersistentId()]) {
        windowStatusChangeListeners.push_back(listener);
    }
    return windowStatusChangeListeners;
}

template<typename T>
EnableIfSame<T, IWindowStatusDidChangeListener, std::vector<sptr<IWindowStatusDidChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowStatusDidChangeListener>> windowStatusDidChangeListeners;
    for (auto& listener : windowStatusDidChangeListeners_[GetPersistentId()]) {
        windowStatusDidChangeListeners.push_back(listener);
    }
    return windowStatusDidChangeListeners;
}

void WindowSessionImpl::ClearListenersById(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "Called id: %{public}d.", GetPersistentId());
    {
        std::lock_guard<std::mutex> lockListener(displayMoveListenerMutex_);
        ClearUselessListeners(displayMoveListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
        ClearUselessListeners(lifecycleListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowChangeListenerMutex_);
        ClearUselessListeners(windowChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
        ClearUselessListeners(avoidAreaChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(dialogDeathRecipientListenerMutex_);
        ClearUselessListeners(dialogDeathRecipientListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(dialogTargetTouchListenerMutex_);
        ClearUselessListeners(dialogTargetTouchListener_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(screenshotListenerMutex_);
        ClearUselessListeners(screenshotListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(screenshotAppEventListenerMutex_);
        ClearUselessListeners(screenshotAppEventListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowStatusChangeListenerMutex_);
        ClearUselessListeners(windowStatusChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowStatusDidChangeListenerMutex_);
        ClearUselessListeners(windowStatusDidChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowTitleButtonRectChangeListenerMutex_);
        ClearUselessListeners(windowTitleButtonRectChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(displayIdChangeListenerMutex_);
        ClearUselessListeners(displayIdChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(systemDensityChangeListenerMutex_);
        ClearUselessListeners(systemDensityChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(acrossDisplaysChangeListenerMutex_);
        ClearUselessListeners(acrossDisplaysChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowNoInteractionListenerMutex_);
        ClearUselessListeners(windowNoInteractionListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(windowRectChangeListenerMutex_);
        ClearUselessListeners(windowRectChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(windowTitleChangeListenerMutex_);
        ClearUselessListeners(windowTitleChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(windowTitleOrHotAreasListenerMutex_);
        ClearUselessListeners(windowTitleOrHotAreasListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(rectChangeInGlobalDisplayListenerMutex_);
        ClearUselessListeners(rectChangeInGlobalDisplayListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(secureLimitChangeListenerMutex_);
        ClearUselessListeners(secureLimitChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(subWindowCloseListenersMutex_);
        ClearUselessListeners(subWindowCloseListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(mainWindowCloseListenersMutex_);
        ClearUselessListeners(mainWindowCloseListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowWillCloseListenersMutex_);
        ClearUselessListeners(windowWillCloseListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(occupiedAreaChangeListenerMutex_);
        ClearUselessListeners(occupiedAreaChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(keyboardWillShowListenerMutex_);
        ClearUselessListeners(keyboardWillShowListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(keyboardWillHideListenerMutex_);
        ClearUselessListeners(keyboardWillHideListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(keyboardDidShowListenerMutex_);
        ClearUselessListeners(keyboardDidShowListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(keyboardDidHideListenerMutex_);
        ClearUselessListeners(keyboardDidHideListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(highlightChangeListenerMutex_);
        ClearUselessListeners(highlightChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowCrossAxisListenerMutex_);
        ClearUselessListeners(windowCrossAxisListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(waterfallModeChangeListenerMutex_);
        ClearUselessListeners(waterfallModeChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(preferredOrientationChangeListenerMutex_);
        ClearUselessListeners(preferredOrientationChangeListener_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(windowOrientationChangeListenerMutex_);
        ClearUselessListeners(windowOrientationChangeListener_, persistentId);
    }
    {
        std::lock_guard<std::mutex> lockListener(windowRotationChangeListenerMutex_);
        ClearUselessListeners(windowRotationChangeListeners_, persistentId);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
        ClearUselessListeners(windowStageLifecycleListeners_, persistentId);
    }
    ClearSwitchFreeMultiWindowListenersById(persistentId);
    TLOGI(WmsLogTag::WMS_LIFE, "Clear success, id: %{public}d.", GetPersistentId());
}

void WindowSessionImpl::ClearSwitchFreeMultiWindowListenersById(int32_t persistentId)
{
    std::lock_guard<std::mutex> lockListener(switchFreeMultiWindowListenerMutex_);
    ClearUselessListeners(switchFreeMultiWindowListeners_, persistentId);
}

void WindowSessionImpl::RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func)
{
    notifyNativeFunc_ = std::move(func);
}

void WindowSessionImpl::ClearVsyncStation()
{
    if (vsyncStation_ != nullptr) {
        vsyncStation_->Destroy();
    }
}

void WindowSessionImpl::SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer)
{
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    inputEventConsumer_ = inputEventConsumer;
}

WMError WindowSessionImpl::SetTitleButtonVisible(bool isMaximizeVisible, bool isMinimizeVisible, bool isSplitVisible,
    bool isCloseVisible)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (GetUIContentSharedPtr() == nullptr || !IsDecorEnable()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    windowTitleVisibleFlags_ = { isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible };
    UpdateTitleButtonVisibility();
    return WMError::WM_OK;
}

WSError WindowSessionImpl::SetSplitButtonVisible(bool isVisible)
{
    TLOGI(WmsLogTag::WMS_DECOR, "isVisible: %{public}d", isVisible);
    auto task = [weakThis = wptr(this), isVisible] {
        auto window = weakThis.promote();
        if (!window) {
            return;
        }
        window->isSplitButtonVisible_ = isVisible;
        window->UpdateTitleButtonVisibility();
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_SetSplitButtonVisible");
    return WSError::WS_OK;
}

WMError WindowSessionImpl::SetFollowScreenChange(bool isFollowScreenChange)
{
    TLOGI(WmsLogTag::DEFAULT, "window %{public}u set follow screen change: %{public}d",
        GetWindowId(), isFollowScreenChange);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!WindowHelper::IsSystemWindow(property_->GetWindowType())) {
        TLOGE(WmsLogTag::DEFAULT, "window %{public}u type not support", GetWindowId());
        return WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE;
    }
    property_->SetFollowScreenChange(isFollowScreenChange);
    return UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE);
}

WMError WindowSessionImpl::GetIsMidScene(bool& isMidScene)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    hostSession->GetIsMidScene(isMidScene);
    return WMError::WM_OK;
}

CrossAxisState WindowSessionImpl::GetCrossAxisState()
{
    if (crossAxisState_ != CrossAxisState::STATE_INVALID) {
        return crossAxisState_;
    }
    if (IsWindowSessionInvalid()) {
        return CrossAxisState::STATE_INVALID;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, CrossAxisState::STATE_INVALID);
    CrossAxisState state = CrossAxisState::STATE_INVALID;
    if (hostSession->GetCrossAxisState(state) != WSError::WS_OK) {
        return CrossAxisState::STATE_INVALID;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "window id is %{public}d, state is %{public}u", GetPersistentId(),
        static_cast<uint32_t>(state));
    return state;
}

WSError WindowSessionImpl::SendContainerModalEvent(const std::string& eventName, const std::string& eventValue)
{
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    auto task = [weakThis = wptr(this), eventName, eventValue] {
        auto window = weakThis.promote();
        if (!window) {
            return;
        }
        std::shared_ptr<Ace::UIContent> uiContent = window->GetUIContentSharedPtr();
        if (uiContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "uiContent is null!");
            return;
        }
        TLOGNI(WmsLogTag::WMS_EVENT, "name: %{public}s, value: %{public}s", eventName.c_str(), eventValue.c_str());
        uiContent->OnContainerModalEvent(eventName, eventValue);
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_SendContainerModalEvent");
    return WSError::WS_OK;
}

WMError WindowSessionImpl::SetWindowContainerColor(const std::string& activeColor, const std::string& inactiveColor)
{
    if (!windowSystemConfig_.IsPcWindow() && !windowSystemConfig_.IsPadWindow()) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_WINDOW_TRANSPARENT) &&
        !SessionPermission::IsSystemCalling() &&
        containerColorList_.count(property_->GetSessionInfo().bundleName_) == 0) {
        TLOGE(WmsLogTag::WMS_DECOR, "winId: %{public}d, permission denied", GetPersistentId());
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        TLOGE(WmsLogTag::WMS_DECOR, "winId: %{public}d, type: %{public}u not supported",
            GetPersistentId(), GetType());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!IsDecorEnable()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t activeColorValue;
    if (!ColorParser::Parse(activeColor, activeColorValue)) {
        TLOGW(WmsLogTag::WMS_DECOR, "window: %{public}s, value: [%{public}s, %{public}u]",
            GetWindowName().c_str(), activeColor.c_str(), activeColorValue);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t inactiveColorValue;
    if (!ColorParser::Parse(inactiveColor, inactiveColorValue)) {
        TLOGW(WmsLogTag::WMS_DECOR, "window: %{public}s, value: [%{public}s, %{public}u]",
            GetWindowName().c_str(), inactiveColor.c_str(), inactiveColorValue);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!SessionPermission::IsSystemCalling() && (inactiveColorValue & OPAQUE) != OPAQUE) {
        TLOGE(WmsLogTag::WMS_DECOR, "winId: %{public}d, inactive alpha value error", GetPersistentId());
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetWindowContainerColor(activeColorValue, inactiveColorValue);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "winId: %{public}d, uicontent is null", GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetWindowContainerModalColor(const std::string& activeColor,
                                                        const std::string& inactiveColor)
{
    if (!windowSystemConfig_.IsPcWindow()) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!SessionPermission::IsSystemCalling()) {
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(GetType())) {
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!IsDecorEnable()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t activeColorValue;
    if (!ColorParser::Parse(activeColor, activeColorValue)) {
        TLOGE(WmsLogTag::WMS_DECOR, "winId: %{public}d, active value: [%{public}s, %{public}u]",
            GetPersistentId(), activeColor.c_str(), activeColorValue);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t inactiveColorValue;
    if (!ColorParser::Parse(inactiveColor, inactiveColorValue)) {
        TLOGE(WmsLogTag::WMS_DECOR, "winId: %{public}d, inactive value: [%{public}s, %{public}u]",
            GetPersistentId(), inactiveColor.c_str(), inactiveColorValue);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetWindowContainerColor(activeColorValue, inactiveColorValue);
    } else {
        TLOGE(WmsLogTag::WMS_DECOR, "uiContent is null!");
        return WMError::WM_ERROR_NULLPTR;
    }
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifyAfterForeground(bool needNotifyListeners, bool needNotifyUiContent, bool waitAttach)
{
    if (needNotifyListeners) {
        NotifyAfterLifecycleForeground();
        {
            std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
            auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
            CALL_LIFECYCLE_LISTENER(AfterForeground, lifecycleListeners);
        }
    }
    GetAttachStateSyncResult(waitAttach, true);
    if (needNotifyUiContent) {
        CALL_UI_CONTENT(Foreground);
    }

    if (vsyncStation_ != nullptr) {
        TLOGD(WmsLogTag::WMS_MAIN, "enable FrameRateLinker, linkerId=%{public}" PRIu64,
            vsyncStation_->GetFrameRateLinkerId());
        vsyncStation_->SetFrameRateLinkerEnable(GetRSUIContext(), true);
        if (WindowHelper::IsMainWindow(GetType())) {
            TLOGD(WmsLogTag::WMS_MAIN, "IsMainWindow: enable soloist linker");
            vsyncStation_->SetDisplaySoloistFrameRateLinkerEnable(true);
        }
    } else {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
    }

    if (!isValidWaterfallMode_.load()) {
        if (InitWaterfallMode()) {
            isValidWaterfallMode_.store(true);
        }
    } else if (isFullScreenWaterfallMode_.load() != waterfallModeWhenEnterBackground_) {
        NotifyWaterfallModeChange(isFullScreenWaterfallMode_.load());
    }
    if (!intentParam_.empty()) {
        std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
        if (uiContent != nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "SetIntentParam, isColdStart:%{public}u", isIntentColdStart_);
            uiContent->SetIntentParam(intentParam_, std::move(loadPageCallback_), isIntentColdStart_);
            intentParam_ = "";
        } else {
            TLOGE(WmsLogTag::WMS_LIFE, "uiContent is nullptr.");
        }
    }
}

void WindowSessionImpl::GetAttachStateSyncResult(bool waitAttachState, bool afterForeground) const
{
    if (!lifecycleCallback_) {
        TLOGW(WmsLogTag::WMS_LIFE, "lifecycleCallback is null");
        return;
    }
    if (waitAttachState && WindowHelper::IsNeedWaitAttachStateWindow(GetType()) &&
        SysCapUtil::GetBundleName() != AppExecFwk::Constants::SCENE_BOARD_BUNDLE_NAME) {
        auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        if (afterForeground) {
            lifecycleCallback_->GetAttachSyncResult(WINDOW_LIFECYCLE_TIMEOUT);
        } else {
            lifecycleCallback_->GetDetachSyncResult(WINDOW_LIFECYCLE_TIMEOUT);
        }
        auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto waitTime = endTime - startTime;
        if (waitTime >= WINDOW_LIFECYCLE_TIMEOUT) {
            TLOGW(WmsLogTag::WMS_LIFE, "window attach state timeout, persistentId:%{public}d, "
                "afterForeground:%{public}d", GetPersistentId(), afterForeground);
        }
        TLOGI(WmsLogTag::WMS_LIFE, "get attach state sync result, id:%{public}d, afterForeground:%{public}d",
            GetPersistentId(), afterForeground);
    }
}

void WindowSessionImpl::NotifyAfterDidForeground(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d", reason);
    if (reason != static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH) &&
        reason != static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL)) {
        TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d no need notify did foreground", reason);
        return;
    }
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "handler is nullptr");
        return;
    }
    const char* const where = __func__;
    handler_->PostTask([weak = wptr(this), where] {
        auto window = weak.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s window is nullptr", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s execute", where);
        auto lifecycleListeners = window->GetListeners<IWindowLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterDidForeground, lifecycleListeners);
    }, where, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void WindowSessionImpl::NotifyAfterBackground(bool needNotifyListeners, bool needNotifyUiContent)
{
    if (needNotifyListeners) {
        {
            std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
            auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
            CALL_LIFECYCLE_LISTENER(AfterBackground, lifecycleListeners);
        }
        NotifyAfterLifecycleBackground();
    }

    if (needNotifyUiContent) {
        CALL_UI_CONTENT(Background);
    }

    if (vsyncStation_ != nullptr) {
        TLOGD(WmsLogTag::WMS_MAIN, "disable FrameRateLinker, linkerId=%{public}" PRIu64,
            vsyncStation_->GetFrameRateLinkerId());
        vsyncStation_->SetFrameRateLinkerEnable(GetRSUIContext(), false);
        if (WindowHelper::IsMainWindow(GetType())) {
            TLOGD(WmsLogTag::WMS_MAIN, "IsMainWindow: disable soloist linker");
            vsyncStation_->SetDisplaySoloistFrameRateLinkerEnable(false);
        }
    } else {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
    }

    waterfallModeWhenEnterBackground_ = isFullScreenWaterfallMode_.load();
}

void WindowSessionImpl::NotifyAfterDidBackground(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d", reason);
    if (reason != static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH) &&
        reason != static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL)) {
        TLOGI(WmsLogTag::WMS_LIFE, "reason: %{public}d no need notify did background", reason);
        return;
    }
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "handler is nullptr");
        return;
    }
    const char* const where = __func__;
    handler_->PostTask([weak = wptr(this), where] {
        auto window = weak.promote();
        if (window == nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s window is nullptr", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s execute", where);
        auto lifecycleListeners = window->GetListeners<IWindowLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterDidBackground, lifecycleListeners);
    }, where, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

static void RequestInputMethodCloseKeyboard(bool isNeedKeyboard, bool keepKeyboardFlag,
    const std::shared_ptr<Ace::UIContent>& uiContent, int32_t windowId)
{
    if (!isNeedKeyboard && !keepKeyboardFlag) {
#ifdef IMF_ENABLE
        auto pid = IPCSkeleton::GetCallingRealPid();
        AppExecFwk::RunningProcessInfo info;
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(pid, info);
        bool isUIEProc = AAFwk::UIExtensionUtils::IsUIExtension(info.extensionType_);
        int32_t hostWindowId = windowId;
        if (isUIEProc && uiContent) {
            hostWindowId = uiContent->GetUIContentWindowID(uiContent->GetInstanceId());
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Notify InputMethod framework close keyboard start. id: %{public}d, "
              "isUIEProc: %{public}d, hostWindowId: %{public}d", windowId, isUIEProc, hostWindowId);
        auto inputMethodController = MiscServices::InputMethodController::GetInstance();
        if (inputMethodController) {
            inputMethodController->RequestHideInput(static_cast<uint32_t>(hostWindowId));
            TLOGD(WmsLogTag::WMS_KEYBOARD, "Notify InputMethod framework close keyboard end.");
        }
#endif
    }
}

void WindowSessionImpl::NotifyUIContentFocusStatus()
{
    if (!isFocused_) {
        CALL_UI_CONTENT(UnFocus);
        return;
    }
    CALL_UI_CONTENT(Focus);
    auto task = [weak = wptr(this)]() {
        auto window = weak.promote();
        if (!window) {
            return;
        }
        bool isNeedKeyboard = false;
        auto uiContent = window->GetUIContentSharedPtr();
        if (uiContent) {
            // isNeedKeyboard is set by arkui and indicates whether the window needs a keyboard or not.
            isNeedKeyboard = uiContent->NeedSoftKeyboard();
        }
        // whether keep the keyboard created by other windows, support system window and app subwindow.
        bool keepKeyboardFlag = window->property_->GetKeepKeyboardFlag();
        int32_t windowId = window->GetPersistentId();
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "id: %{public}d, isNeedKeyboard: %{public}d, keepKeyboardFlag: %{public}d",
            windowId, isNeedKeyboard, keepKeyboardFlag);
        RequestInputMethodCloseKeyboard(isNeedKeyboard, keepKeyboardFlag, uiContent, windowId);
    };
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetOnWindowFocused(task);
    }
}

void WindowSessionImpl::NotifyAfterFocused()
{
    NotifyWindowAfterFocused();
    if (GetUIContentSharedPtr() != nullptr) {
        NotifyUIContentFocusStatus();
    } else {
        shouldReNotifyFocus_ = true;
    }
}

void WindowSessionImpl::NotifyAfterUnfocused(bool needNotifyUiContent)
{
    NotifyWindowAfterUnfocused();
    if (needNotifyUiContent) {
        if (GetUIContentSharedPtr() == nullptr) {
            shouldReNotifyFocus_ = true;
        } else {
            CALL_UI_CONTENT(UnFocus);
        }
    }
}

void WindowSessionImpl::NotifyWindowAfterFocused()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterFocused, lifecycleListeners);
}

void WindowSessionImpl::NotifyWindowAfterUnfocused()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    // use needNotifyUinContent to separate ui content callbacks
    CALL_LIFECYCLE_LISTENER(AfterUnfocused, lifecycleListeners);
}

void WindowSessionImpl::NotifyUIContentHighlightStatus(bool isHighlighted)
{
    if (isHighlighted) {
        CALL_UI_CONTENT(ActiveWindow);
    } else {
        CALL_UI_CONTENT(UnActiveWindow);
    }
}

void WindowSessionImpl::NotifyBeforeDestroy(std::string windowName)
{
    auto task = [this]() {
        if (auto uiContent = GetUIContentSharedPtr()) {
            uiContent->Destroy();
        }
        {
            std::unique_lock<std::shared_mutex> lock(uiContentMutex_);
            if (uiContent_ != nullptr) {
                uiContent_ = nullptr;
                TLOGND(WmsLogTag::WMS_LIFE, "NotifyBeforeDestroy: uiContent destroy success, persistentId:%{public}d",
                    GetPersistentId());
            }
        }
    };
    if (handler_) {
        handler_->PostSyncTask(task, "wms:NotifyBeforeDestroy");
    } else {
        task();
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Release uicontent successfully, id: %{public}d.", GetPersistentId());
    if (notifyNativeFunc_) {
        notifyNativeFunc_(windowName);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "successed with id: %{public}d.", GetPersistentId());
}

void WindowSessionImpl::NotifyAfterDestroy()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterDestroyed, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterActive()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterActive, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterInactive()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterInactive, lifecycleListeners);
}

void WindowSessionImpl::NotifyForegroundFailed(WMError ret)
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER_WITH_PARAM(ForegroundFailed, lifecycleListeners, static_cast<int32_t>(ret));
}

void WindowSessionImpl::NotifyBackgroundFailed(WMError ret)
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER_WITH_PARAM(BackgroundFailed, lifecycleListeners, static_cast<int32_t>(ret));
}

void WindowSessionImpl::NotifyAfterResumed()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterResumed, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterPaused()
{
    std::lock_guard<std::recursive_mutex> lockListener(lifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterPaused, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterLifecycleForeground()
{
    std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowStageLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterLifecycleForeground, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterLifecycleBackground()
{
    std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
    auto lifecycleListeners = GetListeners<IWindowStageLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterLifecycleBackground, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterLifecycleResumed()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
    bool useControlState = property_->GetUseControlState();
    if (useControlState) {
        auto lifecycleListeners = GetListeners<IWindowStageLifeCycle>();
        CALL_LIFECYCLE_LISTENER(AfterLifecyclePaused, lifecycleListeners);
        isInteractiveStateFlag_ = false;
        return;
    }

    if (isInteractiveStateFlag_) {
        TLOGI(WmsLogTag::WMS_LIFE, "window has been in interactive status");
        return;
    }
    if (state_ != WindowState::STATE_SHOWN || !isDidForeground_) {
        TLOGI(WmsLogTag::WMS_LIFE, "state: %{public}d, isDidForeground: %{public}d", state_, isDidForeground_);
        return;
    }
    isInteractiveStateFlag_ = true;
    auto lifecycleListeners = GetListeners<IWindowStageLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterLifecycleResumed, lifecycleListeners);
}

void WindowSessionImpl::NotifyAfterLifecyclePaused()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    std::lock_guard<std::recursive_mutex> lockListener(windowStageLifeCycleListenerMutex_);
    if (!isInteractiveStateFlag_) {
        TLOGI(WmsLogTag::WMS_LIFE, "window has been in noninteractive status");
        return;
    }
    isInteractiveStateFlag_ = false;
    auto lifecycleListeners = GetListeners<IWindowStageLifeCycle>();
    CALL_LIFECYCLE_LISTENER(AfterLifecyclePaused, lifecycleListeners);
}

WSError WindowSessionImpl::MarkProcessed(int32_t eventId)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("session is invalid");
        return WSError::WS_DO_NOTHING;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WSError::WS_DO_NOTHING);
    return hostSession->MarkProcessed(eventId);
}

void WindowSessionImpl::RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "window %{public}s id %{public}d register DialogDeathRecipientListener",
        GetWindowName().c_str(), GetPersistentId());
    if (listener == nullptr) {
        WLOGFE("listener is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lockListener(dialogDeathRecipientListenerMutex_);
    RegisterListener(dialogDeathRecipientListeners_[GetPersistentId()], listener);
}

void WindowSessionImpl::UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "window %{public}s id %{public}d",
        GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::recursive_mutex> lockListener(dialogDeathRecipientListenerMutex_);
    UnregisterListener(dialogDeathRecipientListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "window %{public}s id %{public}d",
        GetWindowName().c_str(), GetPersistentId());
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::recursive_mutex> lockListener(dialogTargetTouchListenerMutex_);
    return RegisterListener(dialogTargetTouchListener_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "window %{public}s id %{public}d",
        GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::recursive_mutex> lockListener(dialogTargetTouchListenerMutex_);
    return UnregisterListener(dialogTargetTouchListener_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterScreenshotListener(const sptr<IScreenshotListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(screenshotListenerMutex_);
    return RegisterListener(screenshotListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(screenshotListenerMutex_);
    return UnregisterListener(screenshotListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = WMError::WM_OK;
    bool isUpdate = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(screenshotAppEventListenerMutex_);
        ret = RegisterListener(screenshotAppEventListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        if (screenshotAppEventListeners_[persistentId].size() == 1) {
            isUpdate = true;
        }
    }
    auto hostSession = GetHostSession();
    if (isUpdate && hostSession != nullptr) {
        ret = hostSession->UpdateScreenshotAppEventRegistered(persistentId, true);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = WMError::WM_OK;
    bool isUpdate = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(screenshotAppEventListenerMutex_);
        ret = UnregisterListener(screenshotAppEventListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        if (screenshotAppEventListeners_[persistentId].empty()) {
            isUpdate = true;
        }
    }
    auto hostSession = GetHostSession();
    if (isUpdate && hostSession != nullptr) {
        ret = hostSession->UpdateScreenshotAppEventRegistered(persistentId, false);
    }
    return ret;
}

template<typename T>
EnableIfSame<T, IDialogDeathRecipientListener, std::vector<sptr<IDialogDeathRecipientListener>>> WindowSessionImpl::
    GetListeners()
{
    std::vector<sptr<IDialogDeathRecipientListener>> dialogDeathRecipientListener;
    for (auto& listener : dialogDeathRecipientListeners_[GetPersistentId()]) {
        dialogDeathRecipientListener.push_back(listener);
    }
    return dialogDeathRecipientListener;
}

template<typename T>
EnableIfSame<T, IDialogTargetTouchListener,
    std::vector<sptr<IDialogTargetTouchListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IDialogTargetTouchListener>> dialogTargetTouchListener;
    for (auto& listener : dialogTargetTouchListener_[GetPersistentId()]) {
        dialogTargetTouchListener.push_back(listener);
    }
    return dialogTargetTouchListener;
}

template<typename T>
EnableIfSame<T, IScreenshotListener, std::vector<sptr<IScreenshotListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IScreenshotListener>> screenshotListeners;
    for (auto& listener : screenshotListeners_[GetPersistentId()]) {
        screenshotListeners.push_back(listener);
    }
    return screenshotListeners;
}

template<typename T>
EnableIfSame<T, IScreenshotAppEventListener, std::vector<IScreenshotAppEventListenerSptr>> WindowSessionImpl::GetListeners()
{
    return screenshotAppEventListeners_[GetPersistentId()];
}


WSError WindowSessionImpl::NotifyDestroy()
{
    if (WindowHelper::IsDialogWindow(property_->GetWindowType())) {
        std::lock_guard<std::recursive_mutex> lockListener(dialogDeathRecipientListenerMutex_);
        auto dialogDeathRecipientListener = GetListeners<IDialogDeathRecipientListener>();
        for (auto& listener : dialogDeathRecipientListener) {
            if (listener != nullptr) {
                listener->OnDialogDeathRecipient();
            }
        }
    } else if (WindowHelper::IsSubWindow(property_->GetWindowType())) {
        if (property_->GetIsUIExtFirstSubWindow() && !isUIExtensionAbilityProcess_) {
            Destroy();
        }
    }
    return WSError::WS_OK;
}

template<typename T>
EnableIfSame<T, IDisplayMoveListener, std::vector<sptr<IDisplayMoveListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IDisplayMoveListener>> displayMoveListeners;
    for (auto& listener : displayMoveListeners_[GetPersistentId()]) {
        displayMoveListeners.push_back(listener);
    }
    return displayMoveListeners;
}

void WindowSessionImpl::NotifyDisplayMove(DisplayId from, DisplayId to)
{
    WLOGFD("from %{public}" PRIu64 " to %{public}" PRIu64, from, to);
    {
        std::lock_guard<std::mutex> lockListener(displayMoveListenerMutex_);
        auto displayMoveListeners = GetListeners<IDisplayMoveListener>();
        for (auto& listener : displayMoveListeners) {
            if (listener != nullptr) {
                listener->OnDisplayMove(from, to);
            }
        }
    }
}
 
void WindowSessionImpl::NotifyDmsDisplayMove(DisplayId to)
{
    auto context = GetContext();
    if (context != nullptr) {
        TLOGI(WmsLogTag::WMS_MAIN, "update display move to dms");
        DisplayManager::GetInstance().UpdateDisplayIdFromAms(to, context->GetToken());
    }
}

WSError WindowSessionImpl::NotifyCloseExistPipWindow()
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    auto task = []() {
        PictureInPictureManager::DoClose(true, true);
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_NotifyCloseExistPipWindow");
    return WSError::WS_OK;
}

void WindowSessionImpl::NotifyTouchDialogTarget(int32_t posX, int32_t posY)
{
    TLOGI(WmsLogTag::WMS_DIALOG, "window %{public}s id %{public}d", GetWindowName().c_str(), GetPersistentId());
    if (auto hostSession = GetHostSession()) {
        hostSession->ProcessPointDownSession(posX, posY);
    }
    std::lock_guard<std::recursive_mutex> lockListener(dialogTargetTouchListenerMutex_);
    auto dialogTargetTouchListener = GetListeners<IDialogTargetTouchListener>();
    for (auto& listener : dialogTargetTouchListener) {
        if (listener != nullptr) {
            listener->OnDialogTargetTouch();
        }
    }
}

void WindowSessionImpl::NotifyScreenshot()
{
    std::lock_guard<std::recursive_mutex> lockListener(screenshotListenerMutex_);
    auto screenshotListeners = GetListeners<IScreenshotListener>();
    for (auto& listener : screenshotListeners) {
        if (listener != nullptr) {
            listener->OnScreenshot();
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        AAFwk::Want want;
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::NOTIFY_SCREENSHOT),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

WSError WindowSessionImpl::NotifyScreenshotAppEvent(ScreenshotEventType type)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, screenshotEvent: %{public}d",
        GetPersistentId(), type);
    std::lock_guard<std::recursive_mutex> lockListener(screenshotAppEventListenerMutex_);
    auto screenshotAppEventListeners = GetListeners<IScreenshotAppEventListener>();
    for (auto& listener : screenshotAppEventListeners) {
        if (listener != nullptr) {
            listener->OnScreenshotAppEvent(type);
        }
    }
    return WSError::WS_OK;
}

/** @note @window.layout */
void WindowSessionImpl::NotifySizeChange(Rect rect, WindowSizeChangeReason reason)
{
    HookWindowSizeByHookWindowInfo(rect);
    if (reason != WindowSizeChangeReason::DRAG && reason != WindowSizeChangeReason::DRAG_MOVE) {
        NotifyTitleChange(true, 0);
    }
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowChangeListenerMutex_);
        auto windowChangeListeners = GetListeners<IWindowChangeListener>();
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, sizeChange listenerSize:%{public}zu",
            GetPersistentId(), windowChangeListeners.size());
        for (auto& listener : windowChangeListeners) {
            if (listener != nullptr) {
                listener->OnSizeChange(rect, reason);
            }
        }
    }
    {
        std::lock_guard<std::mutex> lockRectListener(windowRectChangeListenerMutex_);
        auto windowRectChangeListeners = GetListeners<IWindowRectChangeListener>();
        TLOGD(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, rectChange listenerSize:%{public}zu",
            GetPersistentId(), windowRectChangeListeners.size());
        for (auto& listener : windowRectChangeListeners) {
            if (listener != nullptr) {
                listener->OnRectChange(rect, reason);
            }
        }
    }
    NotifyUIExtHostWindowRectChangeListeners(rect, reason);
}

void WindowSessionImpl::NotifyGlobalDisplayRectChange(const Rect& rect, WindowSizeChangeReason reason)
{
    auto listeners = GetListeners<IRectChangeInGlobalDisplayListener>();
    TLOGD(WmsLogTag::WMS_LAYOUT,
        "windowId: %{public}d, rect: %{public}s, reason: %{public}d, listenerSize: %{public}zu",
        GetPersistentId(), rect.ToString().c_str(), static_cast<int32_t>(reason), listeners.size());
    Rect hookedRect = rect;
    HookWindowSizeByHookWindowInfo(hookedRect);
    for (const auto& listener : listeners) {
        if (listener) {
            listener->OnRectChangeInGlobalDisplay(hookedRect, reason);
        }
    }
    NotifyUIExtHostRectChangeInGlobalDisplayListeners(rect, reason);
}

void WindowSessionImpl::NotifyUIExtHostWindowRectChangeListeners(const Rect rect, const WindowSizeChangeReason reason)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    CHECK_UI_CONTENT_RETURN_IF_NULL(uiContent);
    bool isUIExtensionWindow = WindowHelper::IsUIExtensionWindow(GetType());
    if (!isUIExtensionWindow && !rectChangeUIExtListenerIds_.empty()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "rectChangeUIExtListenerIds_ size: %{public}zu",
            rectChangeUIExtListenerIds_.size());
        AAFwk::Want rectWant;
        rectWant.SetParam(Extension::RECT_X, rect.posX_);
        rectWant.SetParam(Extension::RECT_Y, rect.posY_);
        rectWant.SetParam(Extension::RECT_WIDTH, static_cast<int32_t>(rect.width_));
        rectWant.SetParam(Extension::RECT_HEIGHT, static_cast<int32_t>(rect.height_));
        rectWant.SetParam(Extension::RECT_CHANGE_REASON, static_cast<int32_t>(reason));
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_RECT_CHANGE), rectWant,
            rectChangeUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

void WindowSessionImpl::NotifyUIExtHostRectChangeInGlobalDisplayListeners(const Rect& rect,
    const WindowSizeChangeReason reason)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    CHECK_UI_CONTENT_RETURN_IF_NULL(uiContent);
    bool isUIExtensionWindow = WindowHelper::IsUIExtensionWindow(GetType());
    if (!isUIExtensionWindow && !rectChangeInGlobalDisplayUIExtListenerIds_.empty()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "rectChangeInGlobalDisplayUIExtListenerIds_ size: %{public}zu",
            rectChangeInGlobalDisplayUIExtListenerIds_.size());
        AAFwk::Want rectWant;
        rectWant.SetParam(Extension::RECT_X, rect.posX_);
        rectWant.SetParam(Extension::RECT_Y, rect.posY_);
        rectWant.SetParam(Extension::RECT_WIDTH, static_cast<int32_t>(rect.width_));
        rectWant.SetParam(Extension::RECT_HEIGHT, static_cast<int32_t>(rect.height_));
        rectWant.SetParam(Extension::RECT_CHANGE_REASON, static_cast<int32_t>(reason));
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY), rectWant,
            rectChangeInGlobalDisplayUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

void WindowSessionImpl::NotifySubWindowClose(bool& terminateCloseProcess)
{
    WLOGFD("in");
    std::lock_guard<std::mutex> lockListener(subWindowCloseListenersMutex_);
    auto subWindowCloseListeners = GetListeners<ISubWindowCloseListener>();
    if (subWindowCloseListeners != nullptr) {
        subWindowCloseListeners->OnSubWindowClose(terminateCloseProcess);
    }
}

WMError WindowSessionImpl::NotifyMainWindowClose(bool& terminateCloseProcess)
{
    std::lock_guard<std::mutex> lockListener(mainWindowCloseListenersMutex_);
    auto mainWindowCloseListener = GetListeners<IMainWindowCloseListener>();
    if (mainWindowCloseListener != nullptr) {
        mainWindowCloseListener->OnMainWindowClose(terminateCloseProcess);
        return WMError::WM_OK;
    }
    return WMError::WM_ERROR_NULLPTR;
}

WMError WindowSessionImpl::NotifyWindowWillClose(sptr<Window> window)
{
    std::lock_guard<std::recursive_mutex> lockListener(windowWillCloseListenersMutex_);
    const auto& windowWillCloseListeners = GetListeners<IWindowWillCloseListener>();
    auto res = WMError::WM_ERROR_NULLPTR;
    for (const auto& listener : windowWillCloseListeners) {
        if (listener != nullptr) {
            listener->OnWindowWillClose(window);
            res = WMError::WM_OK;
        }
    }
    return res;
}

void WindowSessionImpl::NotifySwitchFreeMultiWindow(bool enable)
{
    std::lock_guard<std::mutex> lockListener(switchFreeMultiWindowListenerMutex_);
    auto switchFreeMultiWindowListeners = GetListeners<ISwitchFreeMultiWindowListener>();
    for (auto& listener : switchFreeMultiWindowListeners) {
        if (listener != nullptr) {
            listener->OnSwitchFreeMultiWindow(enable);
        }
    }
}

void WindowSessionImpl::NotifyTitleChange(bool isShow, int32_t height)
{
    if (!IsAnco()) {
        return;
    }
    auto windowTitleOrHotAreasListeners = GetListeners<IWindowTitleOrHotAreasListener>();
    property_->SetStatusBarHeightInImmersive(height);
    std::vector<Rect> rectAreas = GetAncoWindowHotAreas();
 
    for (auto& listener : windowTitleOrHotAreasListeners) {
        if (listener != nullptr) {
            TLOGI(WmsLogTag::WMS_IMMS, "NotifyTitleChange, the title bar is show? %{public}d", isShow);
            listener->OnTitleOrHotAreasChange(rectAreas, isShow);
        }
    }
}
 
std::vector<Rect> WindowSessionImpl::GetAncoWindowHotAreas()
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    std::vector<Rect> rectAreas;
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "uiContent is null, windowId: %{public}u", GetWindowId());
        return rectAreas;
    }
    float vpr = GetVirtualPixelRatio();
    float outsideArea = HOTZONE_TOUCH * vpr;
    float insideArea = WINDOW_FRAME_WIDTH * vpr;
    float cornerArea = WINDOW_FRAME_CORNER_WIDTH * vpr;
    int32_t width = static_cast<int32_t>(property_->GetWindowRect().width_);
    int32_t height = static_cast<int32_t>(property_->GetWindowRect().height_);
    int32_t posX = property_->GetWindowRect().posX_;
    int32_t posY = property_->GetWindowRect().posY_;
    int32_t decorHeight = uiContent->GetContainerModalTitleHeight();
    int32_t statusBarHeight = property_->GetStatusBarHeightInImmersive();
 
    Rect titleRect = {posX, posY + statusBarHeight, width * compatScaleX_, decorHeight * compatScaleY_};
    rectAreas.push_back(titleRect);
    Rect rectTop = {posX - outsideArea * compatScaleX_, posY - outsideArea * compatScaleY_,
        (width + outsideArea * 2) * compatScaleX_, (outsideArea + insideArea) * compatScaleY_};
    rectAreas.push_back(rectTop);
    Rect rectLeft = {posX - outsideArea * compatScaleX_, posY - outsideArea * compatScaleY_,
        (outsideArea + insideArea) * compatScaleX_, (height + outsideArea * 2) * compatScaleY_};
    rectAreas.push_back(rectLeft);
    Rect rectRight = {posX + width * compatScaleX_ - insideArea, posY - outsideArea,
        outsideArea + insideArea, height * compatScaleY_ + outsideArea * 2};
    rectAreas.push_back(rectRight);
    Rect rectBottom = {posX - outsideArea * compatScaleX_, posY + (height - insideArea) * compatScaleY_,
        (width + outsideArea * 2) * compatScaleX_, (outsideArea + insideArea) * compatScaleY_};
    rectAreas.push_back(rectBottom);
    Rect rectLeftTop = {posX, posY, cornerArea * compatScaleX_, cornerArea * compatScaleY_};
    rectAreas.push_back(rectLeftTop);
    Rect rectRightTop = {posX + (width - cornerArea) * compatScaleX_, posY,
        cornerArea * compatScaleX_, cornerArea * compatScaleY_};
    rectAreas.push_back(rectRightTop);
    Rect rectLeftBottom = {posX, posY + (height - cornerArea) * compatScaleY_,
        cornerArea * compatScaleX_, cornerArea * compatScaleY_};
    rectAreas.push_back(rectLeftBottom);
    Rect rectRightBottom = {posX + (width - cornerArea) * compatScaleX_, posY + (height - cornerArea) * compatScaleY_,
        cornerArea * compatScaleX_, cornerArea * compatScaleY_};
    rectAreas.push_back(rectRightBottom);
    return rectAreas;
}

WMError WindowSessionImpl::RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = WMError::WM_OK;
    bool isUpdate = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
        ret = RegisterListener(avoidAreaChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        if (avoidAreaChangeListeners_[persistentId].size() == 1) {
            isUpdate = true;
        }
    }
    if (isUpdate) {
        ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionAvoidAreaListener(persistentId, true);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d", persistentId);
    if (listener == nullptr) {
        WLOGFE("listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = WMError::WM_OK;
    bool isUpdate = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
        ret = UnregisterListener(avoidAreaChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        if (avoidAreaChangeListeners_[persistentId].empty()) {
            isUpdate = true;
        }
    }
    if (isUpdate) {
        ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionAvoidAreaListener(persistentId, false);
    }
    return ret;
}

WMError WindowSessionImpl::RegisterExtensionAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    WLOGFI("Start, id:%{public}d", persistentId);
    std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
    return RegisterListener(avoidAreaChangeListeners_[persistentId], listener);
}

WMError WindowSessionImpl::UnregisterExtensionAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    WLOGFI("Start, id:%{public}d", persistentId);
    std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
    return UnregisterListener(avoidAreaChangeListeners_[persistentId], listener);
}

template<typename T>
EnableIfSame<T, IAvoidAreaChangedListener,
    std::vector<sptr<IAvoidAreaChangedListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IAvoidAreaChangedListener>> windowChangeListeners;
    for (auto& listener : avoidAreaChangeListeners_[GetPersistentId()]) {
        windowChangeListeners.push_back(listener);
    }
    return windowChangeListeners;
}

void WindowSessionImpl::NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    std::lock_guard<std::recursive_mutex> lockListener(avoidAreaChangeListenerMutex_);
    auto avoidAreaChangeListeners = GetListeners<IAvoidAreaChangedListener>();
    bool isUIExtensionWithSystemHost =
        WindowHelper::IsUIExtensionWindow(GetType()) && WindowHelper::IsSystemWindow(GetRootHostWindowType());
    bool isSystemWindow = WindowHelper::IsSystemWindow(GetType());
    uint32_t currentApiVersion = GetTargetAPIVersionByApplicationInfo();
    AvoidArea newAvoidArea;
    // api 18 isolation for UEC with system host window
    if ((isUIExtensionWithSystemHost || isSystemWindow) && currentApiVersion < static_cast<uint32_t>(API_VERSION_18)) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d api %{public}u type %{public}d not supported",
            GetPersistentId(), currentApiVersion, type);
    } else {
        newAvoidArea = *avoidArea;
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d api %{public}u type %{public}d area %{public}s",
            GetPersistentId(), currentApiVersion, type, newAvoidArea.ToString().c_str());
    }

    for (auto& listener : avoidAreaChangeListeners) {
        if (listener != nullptr) {
            listener->OnAvoidAreaChanged(newAvoidArea, type);
        }
    }
}

WSError WindowSessionImpl::NotifyTransferComponentData(const AAFwk::WantParams& wantParams)
{
    return WSError::WS_OK;
}

WSErrorCode WindowSessionImpl::NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
    AAFwk::WantParams& reWantParams)
{
    return WSErrorCode::WS_OK;
}

WSError WindowSessionImpl::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    auto task = [weak = wptr(this), avoidArea, type] {
        auto window = weak.promote();
        if (!window) {
            return;
        }
        if ((window->lastAvoidAreaMap_.find(type) == window->lastAvoidAreaMap_.end() &&
             type != AvoidAreaType::TYPE_CUTOUT) ||
            window->lastAvoidAreaMap_[type] != *avoidArea) {
            window->lastAvoidAreaMap_[type] = *avoidArea;
            window->NotifyAvoidAreaChange(avoidArea, type);
            window->UpdateViewportConfig(window->GetRect(), WindowSizeChangeReason::AVOID_AREA_CHANGE);
        }
    };
    handler_->PostTask(std::move(task), __func__);
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SetPipActionEvent(const std::string& action, int32_t status)
{
    TLOGI(WmsLogTag::WMS_PIP, "action: %{public}s, status: %{public}d", action.c_str(), status);
    auto task = [action, status]() {
        PictureInPictureManager::DoActionEvent(action, status);
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_SetPipActionEvent");
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, enabled:%{public}d", controlType, status);
    auto task = [controlType, status]() {
        PictureInPictureManager::DoControlEvent(static_cast<PiPControlType>(controlType),
            static_cast<PiPControlStatus>(status));
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_SetPiPControlEvent");
    return WSError::WS_OK;
}

WSError WindowSessionImpl::NotifyPipWindowSizeChange(double width, double height, double scale)
{
    TLOGI(WmsLogTag::WMS_PIP, "width: %{public}f, height: %{public}f scale: %{public}f", width, height, scale);
    auto task = [width, height, scale]() {
        PictureInPictureManager::PipSizeChange(width, height, scale);
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_NotifyPipWindowSizeChange");
    return WSError::WS_OK;
}

WSError WindowSessionImpl::NotifyPiPActiveStatusChange(bool status)
{
    TLOGI(WmsLogTag::WMS_PIP, "status=%{public}u", status);
    auto task = [status]() {
        PictureInPictureManager::DoActiveStatusChangeEvent(status);
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_NotifyPiPActiveStatusChange");
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SendFbActionEvent(const std::string& action)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "action: %{public}s", action.c_str());
    auto task = [action]() {
        FloatingBallManager::DoFbActionEvent(action);
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_SendFbActionEvent");
    return WSError::WS_OK;
}

WMError WindowSessionImpl::GetFloatingBallWindowId(uint32_t& windowId)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    WMError ret = hostSession->GetFloatingBallWindowId(windowId);
    return ret;
}

WMError WindowSessionImpl::RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    bool isUpdate = false;
    WMError ret = WMError::WM_OK;
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_EVENT, "name=%{public}s, id=%{public}u",
        GetWindowName().c_str(), GetPersistentId());
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    {
        std::lock_guard<std::recursive_mutex> lockListener(touchOutsideListenerMutex_);
        ret = RegisterListener(touchOutsideListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_EVENT, "fail, ret:%{public}u", ret);
            return ret;
        }
        if (touchOutsideListeners_[persistentId].size() == 1) {
            isUpdate = true;
        }
    }
    if (isUpdate) {
        ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionTouchOutsideListener(persistentId, true);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    bool isUpdate = false;
    WMError ret = WMError::WM_OK;
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_EVENT, "name=%{public}s, id=%{public}u",
        GetWindowName().c_str(), GetPersistentId());
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    {
        std::lock_guard<std::recursive_mutex> lockListener(touchOutsideListenerMutex_);
        ret = UnregisterListener(touchOutsideListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_EVENT, "fail, ret:%{public}u", ret);
            return ret;
        }
        if (touchOutsideListeners_[persistentId].empty()) {
            isUpdate = true;
        }
    }
    if (isUpdate) {
        ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionTouchOutsideListener(persistentId, false);
    }
    return ret;
}

template<typename T>
EnableIfSame<T, ITouchOutsideListener, std::vector<sptr<ITouchOutsideListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<ITouchOutsideListener>> windowChangeListeners;
    for (auto& listener : touchOutsideListeners_[GetPersistentId()]) {
        windowChangeListeners.push_back(listener);
    }
    return windowChangeListeners;
}

WSError WindowSessionImpl::NotifyTouchOutside()
{
    TLOGD(WmsLogTag::WMS_EVENT, "window: name=%{public}s, id=%{public}u",
        GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::recursive_mutex> lockListener(touchOutsideListenerMutex_);
    auto touchOutsideListeners = GetListeners<ITouchOutsideListener>();
    for (auto& listener : touchOutsideListeners) {
        if (listener != nullptr) {
            listener->OnTouchOutside();
        }
    }
    return WSError::WS_OK;
}

WMError WindowSessionImpl::RegisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener)
{
    auto persistentId = GetPersistentId();
    WLOGFD("Start, persistentId=%{public}d.", persistentId);
    WMError ret = WMError::WM_OK;
    bool isFirstRegister = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowVisibilityChangeListenerMutex_);
        ret = RegisterListener(windowVisibilityChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        isFirstRegister = windowVisibilityChangeListeners_[persistentId].size() == 1;
    }

    if (isFirstRegister) {
        ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionWindowVisibilityListener(persistentId, true);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener)
{
    auto persistentId = GetPersistentId();
    WLOGFD("Start, persistentId=%{public}d.", persistentId);
    WMError ret = WMError::WM_OK;
    bool isLastUnregister = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowVisibilityChangeListenerMutex_);
        ret = UnregisterListener(windowVisibilityChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        isLastUnregister = windowVisibilityChangeListeners_[persistentId].empty();
    }

    if (isLastUnregister) {
        ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionWindowVisibilityListener(persistentId, false);
    }
    return ret;
}

WMError WindowSessionImpl::RegisterOcclusionStateChangeListener(const sptr<IOcclusionStateChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    bool isFirstRegister = false;
    {
        std::lock_guard<std::mutex> lockListener(occlusionStateChangeListenerMutex_);
        auto ret = RegisterListener(occlusionStateChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed: winId=%{public}d", persistentId);
            return ret;
        }
        isFirstRegister = occlusionStateChangeListeners_[persistentId].size() == 1;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, isFirstRegister=%{public}d", persistentId, isFirstRegister);
    if (!isFirstRegister) {
        return WMError::WM_OK;
    }
    auto ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionOcclusionStateListener(persistentId, true);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "ipc failed: winId=%{public}d, retCode=%{public}d",
            persistentId, static_cast<int32_t>(ret));
        std::lock_guard<std::mutex> lockListener(occlusionStateChangeListenerMutex_);
        ret = UnregisterListener(occlusionStateChangeListeners_[persistentId], listener);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterOcclusionStateChangeListener(const sptr<IOcclusionStateChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    bool isLastUnregister = false;
    {
        std::lock_guard<std::mutex> lockListener(occlusionStateChangeListenerMutex_);
        auto ret = UnregisterListener(occlusionStateChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed: winId=%{public}d", persistentId);
            return ret;
        }
        if (occlusionStateChangeListeners_[persistentId].empty()) {
            occlusionStateChangeListeners_.erase(persistentId);
            isLastUnregister = true;
        }
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, isLastUnregister=%{public}d", persistentId, isLastUnregister);
    if (!isLastUnregister) {
        return WMError::WM_OK;
    }
    auto ret = SingletonContainer::Get<WindowAdapter>().UpdateSessionOcclusionStateListener(persistentId, false);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "ipc failed: winId=%{public}d, retCode=%{public}d",
            persistentId, static_cast<int32_t>(ret));
        std::lock_guard<std::mutex> lockListener(occlusionStateChangeListenerMutex_);
        ret = RegisterListener(occlusionStateChangeListeners_[persistentId], listener);
    }
    return ret;
}

WSError WindowSessionImpl::NotifyWindowOcclusionState(const WindowVisibilityState state)
{
    auto persistentId = GetPersistentId();
    std::vector<sptr<IOcclusionStateChangedListener>> listeners;
    {
        std::lock_guard<std::mutex> lockListener(occlusionStateChangeListenerMutex_);
        for (auto& listener : occlusionStateChangeListeners_[persistentId]) {
            listeners.push_back(listener);
        }
    }
    auto visibilityState = state;
    if (static_cast<uint32_t>(state) > static_cast<uint32_t>(
        WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION)) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, recvVisibilityState=%{public}u",
            persistentId, static_cast<uint32_t>(state));
        visibilityState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
    }
    if (visibilityState == lastVisibilityState_) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, sameVisibilityState=%{public}u",
            persistentId, static_cast<uint32_t>(visibilityState));
        return WSError::WS_OK;
    }
    lastVisibilityState_ = visibilityState;
    uint32_t notifyCounter = 0;
    for (auto& listener : listeners) {
        if (listener != nullptr) {
            listener->OnOcclusionStateChanged(visibilityState);
            notifyCounter++;
        }
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, visibilityState=%{public}u, notifyCounter=%{public}u",
        persistentId, static_cast<uint32_t>(visibilityState), notifyCounter);
    return WSError::WS_OK;
}

WMError WindowSessionImpl::RegisterFrameMetricsChangeListener(const sptr<IFrameMetricsChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    auto uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null: winId=%{public}d", persistentId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    bool isFirstRegister = false;
    {
        std::lock_guard<std::mutex> lockListener(frameMetricsChangeListenerMutex_);
        auto ret = RegisterListener(frameMetricsChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed: winId=%{public}d", persistentId);
            return ret;
        }
        isFirstRegister = frameMetricsChangeListeners_[persistentId].size() == 1;
    }
    if (!isFirstRegister) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "register another: winId=%{public}d", persistentId);
        return WMError::WM_OK;
    }
    uiContent->SetFrameMetricsCallBack([weak = wptr(this), where = __func__](Ace::FrameMetrics info) {
        auto window = weak.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: window is null", where);
            return;
        }
        window->NotifyFrameMetrics(info);
    });
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "register to arkui: winId=%{public}d", persistentId);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterFrameMetricsChangeListener(const sptr<IFrameMetricsChangedListener>& listener)
{
    auto persistentId = GetPersistentId();
    auto uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null: winId=%{public}d", persistentId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    bool isLastUnregister = false;
    {
        std::lock_guard<std::mutex> lockListener(frameMetricsChangeListenerMutex_);
        auto ret = UnregisterListener(frameMetricsChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "failed: winId=%{public}d", persistentId);
            return ret;
        }
        if (frameMetricsChangeListeners_[persistentId].empty()) {
            frameMetricsChangeListeners_.erase(persistentId);
            isLastUnregister = true;
        }
    }
    if (!isLastUnregister) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "unregister another: winId=%{public}d", persistentId);
        return WMError::WM_OK;
    }
    uiContent->SetFrameMetricsCallBack(nullptr);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "unregister to arkui: winId=%{public}d", persistentId);
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifyFrameMetrics(const Ace::FrameMetrics& info)
{
    auto persistentId = GetPersistentId();
    std::vector<sptr<IFrameMetricsChangedListener>> listeners;
    {
        std::lock_guard<std::mutex> lockListener(frameMetricsChangeListenerMutex_);
        for (auto& listener : frameMetricsChangeListeners_[persistentId]) {
            listeners.push_back(listener);
        }
    }
    uint32_t notifyCounter = 0;
    FrameMetrics metrics;
    metrics.firstDrawFrame_ = info.firstDrawFrame;
    metrics.inputHandlingDuration_ = info.inputHandlingDuration;
    metrics.layoutMeasureDuration_ = info.layoutMeasureDuration;
    metrics.vsyncTimestamp_ = info.vsyncTimestamp;
    for (auto& listener : listeners) {
        if (listener != nullptr) {
            listener->OnFrameMetricsChanged(metrics);
            notifyCounter++;
        }
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}d, notifyCounter=%{public}u, firstDrawFrame=%{public}d"
        ", inputHandlingDuration=%{public}" PRIu64 ", layoutMeasureDuration=%{public}" PRIu64
        ", vsyncTimestamp=%{public}" PRIu64, persistentId, notifyCounter, metrics.firstDrawFrame_,
        metrics.inputHandlingDuration_, metrics.layoutMeasureDuration_, metrics.vsyncTimestamp_);
}

WMError WindowSessionImpl::RegisterDisplayIdChangeListener(const IDisplayIdChangeListenerSptr& listener)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "name=%{public}s, id=%{public}u", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(displayIdChangeListenerMutex_);
    return RegisterListener(displayIdChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterDisplayIdChangeListener(const IDisplayIdChangeListenerSptr& listener)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "name=%{public}s, id=%{public}u", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(displayIdChangeListenerMutex_);
    return UnregisterListener(displayIdChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterSystemDensityChangeListener(const ISystemDensityChangeListenerSptr& listener)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "name=%{public}s, id=%{public}d", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(systemDensityChangeListenerMutex_);
    return RegisterListener(systemDensityChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::UnregisterSystemDensityChangeListener(const ISystemDensityChangeListenerSptr& listener)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "name=%{public}s, id=%{public}d", GetWindowName().c_str(), GetPersistentId());
    std::lock_guard<std::mutex> lockListener(systemDensityChangeListenerMutex_);
    return UnregisterListener(systemDensityChangeListeners_[GetPersistentId()], listener);
}

WMError WindowSessionImpl::RegisterAcrossDisplaysChangeListener(
    const IAcrossDisplaysChangeListenerSptr& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = WMError::WM_OK;
    bool isUpdate = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(acrossDisplaysChangeListenerMutex_);
        ret = RegisterListener(acrossDisplaysChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        if (acrossDisplaysChangeListeners_[persistentId].size() == 1) {
            isUpdate = true;
        }
    }
    auto hostSession = GetHostSession();
    if (isUpdate && hostSession != nullptr) {
        ret = hostSession->UpdateAcrossDisplaysChangeRegistered(true);
    }
    if (ret != WMError::WM_OK) {
        std::lock_guard<std::recursive_mutex> lockListener(acrossDisplaysChangeListenerMutex_);
        UnregisterListener(acrossDisplaysChangeListeners_[persistentId], listener);
    }
    return ret;
}

WMError WindowSessionImpl::UnRegisterAcrossDisplaysChangeListener(
    const IAcrossDisplaysChangeListenerSptr& listener)
{
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto persistentId = GetPersistentId();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d", persistentId);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = WMError::WM_OK;
    bool isUpdate = false;
    {
        std::lock_guard<std::recursive_mutex> lockListener(acrossDisplaysChangeListenerMutex_);
        ret = UnregisterListener(acrossDisplaysChangeListeners_[persistentId], listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        if (acrossDisplaysChangeListeners_[persistentId].empty()) {
            isUpdate = true;
        }
    }
    auto hostSession = GetHostSession();
    if (isUpdate && hostSession != nullptr) {
        ret = hostSession->UpdateAcrossDisplaysChangeRegistered(false);
    }
    if (ret != WMError::WM_OK) {
        std::lock_guard<std::recursive_mutex> lockListener(acrossDisplaysChangeListenerMutex_);
        RegisterListener(acrossDisplaysChangeListeners_[persistentId], listener);
    }
    return ret;
}

WMError WindowSessionImpl::RegisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowNoInteractionListenerMutex_);
    WMError ret = RegisterListener(windowNoInteractionListeners_[GetPersistentId()], listener);
    if (ret != WMError::WM_OK) {
        WLOGFE("register failed.");
    } else {
        SubmitNoInteractionMonitorTask(this->lastInteractionEventId_.load(), listener);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
{
    WLOGFD("in");
    std::lock_guard<std::recursive_mutex> lockListener(windowNoInteractionListenerMutex_);
    WMError ret = UnregisterListener(windowNoInteractionListeners_[GetPersistentId()], listener);
    if (windowNoInteractionListeners_[GetPersistentId()].empty()) {
        lastInteractionEventId_.store(-1);
    }
    return ret;
}

template<typename T>
EnableIfSame<T, IWindowRotationChangeListener, std::vector<sptr<IWindowRotationChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IWindowRotationChangeListener>> windowRotationChangeListener;
    for (auto& listener : windowRotationChangeListeners_[GetPersistentId()]) {
        windowRotationChangeListener.push_back(listener);
    }
    return windowRotationChangeListener;
}

WMError WindowSessionImpl::RegisterWindowRotationChangeListener(const sptr<IWindowRotationChangeListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto persistentId = GetPersistentId();
    WMError ret = WMError::WM_OK;
    {
        std::lock_guard<std::mutex> lockListener(windowRotationChangeListenerMutex_);
        ret = RegisterListener(windowRotationChangeListeners_[persistentId], listener);
    }
    auto hostSession = GetHostSession();
    if (hostSession != nullptr && ret == WMError::WM_OK) {
        hostSession->UpdateRotationChangeRegistered(persistentId, true);
    }
    return ret;
}

WMError WindowSessionImpl::UnregisterWindowRotationChangeListener(const sptr<IWindowRotationChangeListener>& listener)
{
    WMError ret = WMError::WM_OK;
    auto persistentId = GetPersistentId();
    bool windowRotationChangeListenerEmpty = false;
    {
        std::lock_guard<std::mutex> lockListener(windowRotationChangeListenerMutex_);
        ret = UnregisterListener(windowRotationChangeListeners_[persistentId], listener);
        windowRotationChangeListenerEmpty = windowRotationChangeListeners_.count(persistentId) == 0 ||
                                            windowRotationChangeListeners_[persistentId].empty();
    }
    auto hostSession = GetHostSession();
    if (hostSession != nullptr && windowRotationChangeListenerEmpty) {
        hostSession->UpdateRotationChangeRegistered(persistentId, false);
    }
    return ret;
}

template<typename T>
EnableIfSame<T, IWindowVisibilityChangedListener, std::vector<IWindowVisibilityListenerSptr>> WindowSessionImpl::GetListeners()
{
    std::vector<IWindowVisibilityListenerSptr> windowVisibilityChangeListeners;
    for (auto& listener : windowVisibilityChangeListeners_[GetPersistentId()]) {
        windowVisibilityChangeListeners.push_back(listener);
    }
    return windowVisibilityChangeListeners;
}

template<typename T>
EnableIfSame<T, IDisplayIdChangeListener,
    std::vector<IDisplayIdChangeListenerSptr>> WindowSessionImpl::GetListeners()
{
    return displayIdChangeListeners_[GetPersistentId()];
}

template<typename T>
EnableIfSame<T, ISystemDensityChangeListener,
    std::vector<ISystemDensityChangeListenerSptr>> WindowSessionImpl::GetListeners()
{
    return systemDensityChangeListeners_[GetPersistentId()];
}

template<typename T>
EnableIfSame<T, IAcrossDisplaysChangeListener, std::vector<IAcrossDisplaysChangeListenerSptr>> WindowSessionImpl::GetListeners()
{
    return acrossDisplaysChangeListeners_[GetPersistentId()];
}

template<typename T>
EnableIfSame<T, IWindowNoInteractionListener, std::vector<IWindowNoInteractionListenerSptr>> WindowSessionImpl::GetListeners()
{
    std::vector<IWindowNoInteractionListenerSptr> noInteractionListeners;
    for (auto& listener : windowNoInteractionListeners_[GetPersistentId()]) {
        noInteractionListeners.push_back(listener);
    }
    return noInteractionListeners;
}

WSError WindowSessionImpl::NotifyDisplayIdChange(DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "id=%{public}u, displayId=%{public}" PRIu64, GetPersistentId(), displayId);
    std::lock_guard<std::mutex> lock(displayIdChangeListenerMutex_);
    auto displayIdChangeListeners = GetListeners<IDisplayIdChangeListener>();
    for (auto& listener : displayIdChangeListeners) {
        if (listener != nullptr) {
            listener->OnDisplayIdChanged(displayId);
        }
    }
    return WSError::WS_OK;
}

WMError WindowSessionImpl::NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed)
{
    TLOGD(WmsLogTag::WMS_EVENT, "isConsumed:%{public}d", isConsumed);
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().NotifyWatchGestureConsumeResult(keyCode, isConsumed);
}

bool WindowSessionImpl::GetWatchGestureConsumed() const
{
    return isWatchGestureConsumed_;
}

void WindowSessionImpl::SetWatchGestureConsumed(bool isWatchGestureConsumed)
{
    TLOGD(WmsLogTag::WMS_EVENT, "wid:%{public}d, isWatchGestureConsumed:%{public}d",
        GetPersistentId(), isWatchGestureConsumed);
    isWatchGestureConsumed_ = isWatchGestureConsumed;
}

WSError WindowSessionImpl::NotifySystemDensityChange(float density)
{
    std::lock_guard<std::mutex> lock(systemDensityChangeListenerMutex_);
    const auto& systemDensityChangeListeners = GetListeners<ISystemDensityChangeListener>();
    for (const auto& listener : systemDensityChangeListeners) {
        if (listener != nullptr) {
            listener->OnSystemDensityChanged(density);
        }
    }
    return WSError::WS_OK;
}

WMError WindowSessionImpl::SetWindowDefaultDensityEnabled(bool enabled)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "WinId: %{public}d, enabled: %{public}u", GetPersistentId(), enabled);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WinId: %{public}d permission denied!", GetPersistentId());
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    SetDefaultDensityEnabledValue(enabled);
    UpdateDensity();
    return WMError::WM_OK;
}

void WindowSessionImpl::SetDefaultDensityEnabledValue(bool enabled)
{
    isDefaultDensityEnabled_.store(enabled);
}

bool WindowSessionImpl::IsStageDefaultDensityEnabled()
{
    if (GetType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        return defaultDensityEnabledStageConfig_.load();
    } else {
        auto mainWindow = FindMainWindowWithContext();
        return mainWindow ? mainWindow->defaultDensityEnabledStageConfig_.load() : false;
    }
}

WSError WindowSessionImpl::NotifyWindowVisibility(bool isVisible)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "window: name=%{public}s, id=%{public}u, isVisible=%{public}d",
        GetWindowName().c_str(), GetPersistentId(), isVisible);
    std::lock_guard<std::recursive_mutex> lockListener(windowVisibilityChangeListenerMutex_);
    auto windowVisibilityListeners = GetListeners<IWindowVisibilityChangedListener>();
    for (auto& listener : windowVisibilityListeners) {
        if (listener != nullptr) {
            listener->OnWindowVisibilityChangedCallback(isVisible);
        }
    }
    return WSError::WS_OK;
}

WSError WindowSessionImpl::NotifyNoInteractionTimeout(const IWindowNoInteractionListenerSptr& listener)
{
    if (listener == nullptr) {
        WLOGFE("invalid listener");
        return WSError::WS_ERROR_NULLPTR;
    }
    WLOGFD("name=%{public}s, id=%{public}u, timeout=%{public}" PRId64,
        GetWindowName().c_str(), GetPersistentId(), listener->GetTimeout());

    listener->OnWindowNoInteractionCallback();
    return WSError::WS_OK;
}

void WindowSessionImpl::NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!pointerEvent) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "Pointer event is nullptr");
        return;
    }

    std::shared_ptr<IInputEventConsumer> inputEventConsumer;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        inputEventConsumer = inputEventConsumer_;
    }
    if (inputEventConsumer != nullptr) {
        WLOGFD("Transfer pointer event to inputEventConsumer");
        if (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
            TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW,
                "Transfer to inputEventConsumer InputTracking id:%{public}d",
                pointerEvent->GetId());
        }
        if (!(inputEventConsumer->OnInputEvent(pointerEvent))) {
            pointerEvent->MarkProcessed();
        }
        return;
    }
    if (FilterPointerEvent(pointerEvent)) {
        return;
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        if (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
            TLOGD(WmsLogTag::WMS_EVENT, "eid:%{public}d", pointerEvent->GetId());
        }
        if (IsWindowDelayRaiseEnabled()) {
            if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_DOWN ||
                pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_UP ||
                pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN ||
                pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
                TLOGI(WmsLogTag::WMS_EVENT, "Delay,id:%{public}d", pointerEvent->GetId());
            }
            pointerEvent->MarkProcessed();
            return;
        }
        TLOGD(WmsLogTag::WMS_EVENT, "Start to process pointerEvent, id: %{public}d", pointerEvent->GetId());
        if (!uiContent->ProcessPointerEvent(pointerEvent)) {
            TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "UI content does not consume");
            pointerEvent->MarkProcessed();
        }
    } else {
        if (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
            TLOGW(WmsLogTag::WMS_INPUT_KEY_FLOW, "pointerEvent not consumed, windowId:%{public}u", GetWindowId());
        }
        pointerEvent->MarkProcessed();
    }
}

WMError WindowSessionImpl::InjectTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!pointerEvent) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "Pointer event is nullptr");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        TLOGI(WmsLogTag::WMS_EVENT, "eid:%{public}d, ac:%{public}d", pointerEvent->GetId(),
            pointerEvent->GetPointerAction());
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGD(WmsLogTag::WMS_EVENT, "Start to process pointerEvent, id: %{public}d", pointerEvent->GetId());
        if (!uiContent->ProcessPointerEvent(pointerEvent)) {
            TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "UI content does not consume");
            pointerEvent->MarkProcessed();
        }
    } else {
        if (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) {
            TLOGW(WmsLogTag::WMS_INPUT_KEY_FLOW, "pointerEvent not consumed, windowId:%{public}u", GetWindowId());
        }
        pointerEvent->MarkProcessed();
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetKeyEventFilter(KeyEventFilterFunc filter)
{
    std::unique_lock<std::mutex> lock(keyEventFilterMutex_);
    keyEventFilter_ = std::move(filter);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::ClearKeyEventFilter()
{
    std::unique_lock<std::mutex> lock(keyEventFilterMutex_);
    keyEventFilter_ = nullptr;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetMouseEventFilter(MouseEventFilterFunc filter)
{
    std::unique_lock<std::mutex> lock(mouseEventFilterMutex_);
    mouseEventFilter_ = std::move(filter);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::ClearMouseEventFilter()
{
    std::unique_lock<std::mutex> lock(mouseEventFilterMutex_);
    mouseEventFilter_ = nullptr;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetTouchEventFilter(TouchEventFilterFunc filter)
{
    std::unique_lock<std::mutex> lock(touchEventFilterMutex_);
    touchEventFilter_ = std::move(filter);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::ClearTouchEventFilter()
{
    std::unique_lock<std::mutex> lock(touchEventFilterMutex_);
    touchEventFilter_ = nullptr;
    return WMError::WM_OK;
}

bool WindowSessionImpl::FilterKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    std::lock_guard<std::mutex> lock(keyEventFilterMutex_);
    if (keyEventFilter_ != nullptr) {
        bool isFilter = keyEventFilter_(*keyEvent.get());
        TLOGE(WmsLogTag::WMS_SYSTEM, "isFilter:%{public}d", isFilter);
        if (isFilter) {
            keyEvent->MarkProcessed();
            return true;
        }
    }
    return false;
}

bool WindowSessionImpl::FilterPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    bool isFiltered = false;
    auto sourceType = pointerEvent->GetSourceType();
    auto action = pointerEvent->GetPointerAction();
    if (sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        TouchEventFilterFunc touchEventFilter = nullptr;
        {
            std::lock_guard<std::mutex> lock(touchEventFilterMutex_);
            if (touchEventFilter_ == nullptr) {
                TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "touch event filter null");
                return false;
            }
            touchEventFilter = touchEventFilter_;
        }
        isFiltered = touchEventFilter(*pointerEvent.get());
    } else if (sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
               (action != OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_BEGIN &&
                action != OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_UPDATE &&
                action != OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_END)) {
        MouseEventFilterFunc mouseEventFilter = nullptr;
        {
            std::lock_guard<std::mutex> lock(mouseEventFilterMutex_);
            if (mouseEventFilter_ == nullptr) {
                TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "mouse event filter null");
                return false;
            }
            mouseEventFilter = mouseEventFilter_;
        }
        isFiltered = mouseEventFilter(*pointerEvent.get());
    }
    if (isFiltered) {
        if (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
            action == MMI::PointerEvent::POINTER_ACTION_UP ||
            action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN ||
            action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP) {
            TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d", pointerEvent->GetId());
        }
        pointerEvent->MarkProcessed();
    }
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "event consumed:%{public}d", isFiltered);
    return isFiltered;
}

void WindowSessionImpl::NotifyConsumeResultToFloatWindow
    (const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool isConsumed)
{
    if ((keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_TAB ||
         keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_ENTER) && !GetWatchGestureConsumed() &&
        keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_DOWN) {
        TLOGD(WmsLogTag::WMS_EVENT, "wid:%{public}u, isConsumed:%{public}d", GetWindowId(), isConsumed);
        NotifyWatchGestureConsumeResult(keyEvent->GetKeyCode(), isConsumed);
    }
}

WMError WindowSessionImpl::HandleEscKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed)
{
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "keyevent is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (keyEvent->GetKeyCode() != MMI::KeyEvent::KEYCODE_ESCAPE) {
        TLOGD(WmsLogTag::WMS_EVENT, "keyevent not esc");
        escKeyHasDown_ = false;
        escKeyEventTriggered_ = false;
        return WMError::WM_DO_NOTHING;
    }

    if (!isConsumed && keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_ESCAPE &&
        IsPcOrPadFreeMultiWindowMode() &&
        property_->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN &&
        GetImmersiveModeEnabledState() &&
        keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_DOWN && !escKeyEventTriggered_) {
        if (Recover() == WMError::WM_OK) {
            isConsumed = true;
            keyEvent->MarkProcessed();
        }
        TLOGI(WmsLogTag::WMS_EVENT, "recover from fullscreen, consumed: %{public}d", isConsumed);
    }

    if (!isConsumed && !escKeyEventTriggered_ && escKeyHasDown_) {
        bool escToBackFlag = keyEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_KEYBOARD_ESCAPE);
        // noflag do not handle; UEC do not handle because it is handled in the host window
        if (!escToBackFlag || property_->GetWindowType() == WindowType::WINDOW_TYPE_UI_EXTENSION) {
            TLOGI(WmsLogTag::WMS_EVENT, "ESC no flag or UIExtension window");
            return WMError::WM_DO_NOTHING;
        }

        TLOGI(WmsLogTag::WMS_EVENT, "normal mode, handlebackevent");
        HandleBackEvent();
    }

    return WMError::WM_OK;
}

void WindowSessionImpl::DispatchKeyEventCallback(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed)
{
    std::shared_ptr<IInputEventConsumer> inputEventConsumer;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        inputEventConsumer = inputEventConsumer_;
    }
    int32_t keyCode = keyEvent->GetKeyCode();
    int32_t keyAction = keyEvent->GetKeyAction();
    if (keyCode == MMI::KeyEvent::KEYCODE_BACK && keyAction == MMI::KeyEvent::KEY_ACTION_UP) {
        TLOGI(WmsLogTag::DEFAULT, "In");
        if (inputEventConsumer != nullptr) {
            WLOGFD("Transfer key event to inputEventConsumer");
            if (inputEventConsumer->OnInputEvent(keyEvent)) {
                return;
            }
            PerformBack();
            keyEvent->MarkProcessed();
            return;
        }
        HandleBackEvent();
        keyEvent->MarkProcessed();
        return;
    }

    if (inputEventConsumer != nullptr) {
        WLOGD("Transfer key event to inputEventConsumer");
        if (!(inputEventConsumer->OnInputEvent(keyEvent))) {
            keyEvent->MarkProcessed();
        }
        return;
    }

    if (auto uiContent = GetUIContentSharedPtr()) {
        if (FilterKeyEvent(keyEvent)) return;
        isConsumed = uiContent->ProcessKeyEvent(keyEvent);
        TLOGI(WmsLogTag::WMS_EVENT, "id: %{public}d, consumed: %{public}d,"
            "escTrigger: %{public}d, escDown: %{public}d",
            keyEvent->GetId(), isConsumed, escKeyEventTriggered_, escKeyHasDown_);
        isConsumed = isConsumed ? isConsumed : WindowInputRedistributeImpl::GetInstance().SendEvent(
            InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, keyEvent);
        HandleEscKeyEvent(keyEvent, isConsumed);
        NotifyConsumeResultToFloatWindow(keyEvent, isConsumed);
        if (!isConsumed) {
            keyEvent->MarkProcessed();
        }
        if (keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_ESCAPE) {
            escKeyHasDown_ = (keyAction == MMI::KeyEvent::KEY_ACTION_DOWN);
            escKeyEventTriggered_ = isConsumed;
        }
    }
}

WSError WindowSessionImpl::HandleBackEvent()
{
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    bool isConsumed = false;
    std::shared_ptr<IInputEventConsumer> inputEventConsumer;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        inputEventConsumer = inputEventConsumer_;
    }
    if (inputEventConsumer != nullptr) {
        WLOGFD("Transfer back event to inputEventConsumer");
        std::shared_ptr<MMI::KeyEvent> backKeyEvent = MMI::KeyEvent::Create();
        if (backKeyEvent == nullptr) {
            WLOGFE("backKeyEvent is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        backKeyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
        backKeyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
        isConsumed = inputEventConsumer->OnInputEvent(backKeyEvent);
    } else {
        if (auto uiContent = GetUIContentSharedPtr()) {
            WLOGFD("Transfer back event to uiContent");
            isConsumed = uiContent->ProcessBackPressed();
        } else {
            WLOGFE("There is no back event consumer");
        }
    }

    if (isConsumed) {
        WLOGD("Back key event is consumed");
        return WSError::WS_OK;
    }
    WLOGFD("report Back");
    SingletonContainer::Get<WindowInfoReporter>().ReportBackButtonInfoImmediately();
    if (handler_ == nullptr) {
        WLOGFE("HandleBackEvent handler_ is nullptr!");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    // notify back event to host session
    wptr<WindowSessionImpl> weak = this;
    auto task = [weak]() {
        auto weakSession = weak.promote();
        if (weakSession == nullptr) {
            WLOGFE("HandleBackEvent session wptr is nullptr");
            return;
        }
        weakSession->PerformBack();
    };
    if (!handler_->PostTask(task, "wms:PerformBack")) {
        WLOGFE("Failed to post PerformBack");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    return WSError::WS_OK;
}

void WindowSessionImpl::NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
    bool notifyInputMethod)
{
    if (keyEvent == nullptr) {
        WLOGFE("keyEvent is nullptr");
        return;
    }

#ifdef IMF_ENABLE
    bool isKeyboardEvent = IsKeyboardEvent(keyEvent);
    if (isKeyboardEvent && notifyInputMethod) {
        WLOGD("Async dispatch keyEvent to input method");
        auto callback = [weakThis = wptr(this)] (std::shared_ptr<MMI::KeyEvent>& keyEvent, bool consumed) {
            if (keyEvent == nullptr) {
                WLOGFW("keyEvent is null, consumed:%{public}" PRId32, consumed);
                return;
            }

            if (consumed) {
                WLOGD("Input method has processed key event, id:%{public}" PRId32, keyEvent->GetId());
                return;
            }

            auto promoteThis = weakThis.promote();
            if (promoteThis == nullptr) {
                WLOGFW("promoteThis is nullptr");
                keyEvent->MarkProcessed();
                return;
            }
            bool isConsumed = false;
            promoteThis->DispatchKeyEventCallback(keyEvent, isConsumed);
        };
        auto ret = MiscServices::InputMethodController::GetInstance()->DispatchKeyEvent(
            const_cast<std::shared_ptr<MMI::KeyEvent>&>(keyEvent), callback);
        if (ret != 0) {
            WLOGFE("DispatchKeyEvent failed, ret:%{public}" PRId32 ", id:%{public}" PRId32, ret, keyEvent->GetId());
            DispatchKeyEventCallback(keyEvent, isConsumed);
        }
        return;
    }
#endif // IMF_ENABLE
    DispatchKeyEventCallback(keyEvent, isConsumed);
}

bool WindowSessionImpl::IsKeyboardEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const
{
    int32_t keyCode = keyEvent->GetKeyCode();
    bool isKeyFN = (keyCode == MMI::KeyEvent::KEYCODE_FN);
    bool isKeyBack = (keyCode == MMI::KeyEvent::KEYCODE_BACK);
    bool isKeyboard = (keyCode >= MMI::KeyEvent::KEYCODE_0 && keyCode <= MMI::KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN);
    bool isKeySound = (keyCode == MMI::KeyEvent::KEYCODE_SOUND);
    TLOGD(WmsLogTag::WMS_EVENT, "isKeyFN:%{public}d, isKeyboard:%{public}d", isKeyFN, isKeyboard);
    return (isKeyFN || isKeyboard || isKeyBack || isKeySound);
}

void WindowSessionImpl::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    if (vsyncStation_ == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
        return;
    }
    vsyncStation_->RequestVsync(vsyncCallback);
}

int64_t WindowSessionImpl::GetVSyncPeriod()
{
    if (vsyncStation_ == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
        return 0;
    }
    return vsyncStation_->GetVSyncPeriod();
}

void WindowSessionImpl::FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType)
{
    if (vsyncStation_ == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
        return;
    }
    vsyncStation_->FlushFrameRate(GetRSUIContext(), rate, animatorExpectedFrameRate, rateType);
}

WMError WindowSessionImpl::UpdateProperty(WSPropertyChangeAction action)
{
    TLOGD(WmsLogTag::DEFAULT, "action:%{public}" PRIu64, action);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::DEFAULT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return hostSession->UpdateSessionPropertyByAction(property_, action);
}

sptr<Window> WindowSessionImpl::Find(const std::string& name)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    auto iter = windowSessionMap_.find(name);
    if (iter == windowSessionMap_.end()) {
        TLOGD(WmsLogTag::DEFAULT, "Can not find window %{public}s", name.c_str());
        return nullptr;
    }
    return iter->second.second;
}

void WindowSessionImpl::SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler)
{
    if (handler == nullptr) {
        WLOGE("ace ability handler is nullptr");
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    aceAbilityHandler_ = handler;
}

WMError WindowSessionImpl::SetBackgroundColor(const std::string& color)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    uint32_t colorValue;
    if (ColorParser::Parse(color, colorValue)) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "window: %{public}s, value: [%{public}s, %{public}u]",
            GetWindowName().c_str(), color.c_str(), colorValue);
        return SetBackgroundColor(colorValue);
    }
    TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid color string: %{public}s", color.c_str());
    return WMError::WM_ERROR_INVALID_PARAM;
}

WMError WindowSessionImpl::SetBackgroundColor(uint32_t color)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "window: %{public}s, value:%{public}u", GetWindowName().c_str(), color);

    // 0xff000000: ARGB style, means Opaque color.
    const bool isAlphaZero = !(color & 0xff000000);
    std::string bundleName;
    std::string abilityName;
    auto context = GetContext();
    if ((context != nullptr) && (context->GetApplicationInfo() != nullptr)) {
        bundleName = context->GetBundleName();
        abilityName = context->GetApplicationInfo()->name;
    }

    if (isAlphaZero && WindowHelper::IsMainWindow(GetType())) {
        auto& reportInstance = SingletonContainer::Get<WindowInfoReporter>();
        reportInstance.ReportZeroOpacityInfoImmediately(bundleName, abilityName);
    }

    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetBackgroundColor(color);
        // 24: Shift right by 24 bits to move the alpha channel to the lowest 8 bits.
        uint8_t alpha = static_cast<uint8_t>((color >> 24) & 0xff);
        property_->SetBackgroundAlpha(alpha);
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_BACKGROUND_ALPHA);
        return WMError::WM_OK;
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null, win=%{public}u, value=%{public}u",
            GetWindowId(), color);
    }

    if (aceAbilityHandler_ != nullptr) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "set ability background color, win=%{public}u, value=%{public}u",
            GetWindowId(), color);
        aceAbilityHandler_->SetBackgroundColor(color);
        return WMError::WM_OK;
    }
    TLOGW(WmsLogTag::WMS_ATTRIBUTE, "FA mode could not set bg color: %{public}u", GetWindowId());
    return WMError::WM_ERROR_INVALID_OPERATION;
}

sptr<Window> WindowSessionImpl::FindWindowById(uint32_t winId)
{
    std::shared_lock<std::shared_mutex> lock(windowSessionMutex_);
    if (windowSessionMap_.empty()) {
        WLOGFE("Please create mainWindow First!");
        return nullptr;
    }
    for (auto iter = windowSessionMap_.begin(); iter != windowSessionMap_.end(); iter++) {
        if (static_cast<int32_t>(winId) == iter->second.first) {
            WLOGD("FindWindow id: %{public}u", winId);
            return iter->second.second;
        }
    }
    WLOGFE("Cannot find Window, id: %{public}d", winId);
    return nullptr;
}

std::vector<sptr<Window>> WindowSessionImpl::GetSubWindow(int parentId)
{
    std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
    auto iter = subWindowSessionMap_.find(parentId);
    if (iter == subWindowSessionMap_.end()) {
        return std::vector<sptr<Window>>();
    }
    return std::vector<sptr<Window>>(subWindowSessionMap_[parentId].begin(), subWindowSessionMap_[parentId].end());
}

bool WindowSessionImpl::IsApplicationModalSubWindowShowing(int32_t parentId)
{
    auto subMap = GetSubWindow(parentId);
    auto applicationModalIndex = std::find_if(subMap.begin(), subMap.end(),
        [](const auto& subWindow) {
            return WindowHelper::IsApplicationModalSubWindow(subWindow->GetType(), subWindow->GetWindowFlags()) &&
                   subWindow->GetWindowState() != WindowState::STATE_SHOWN;
        });
    return applicationModalIndex != subMap.end();
}

uint32_t WindowSessionImpl::GetBackgroundColor() const
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        return uiContent->GetBackgroundColor();
    }
    WLOGD("uiContent is null, windowId: %{public}u, use FA mode", GetWindowId());
    if (aceAbilityHandler_ != nullptr) {
        return aceAbilityHandler_->GetBackgroundColor();
    }
    WLOGFD("FA mode does not get bg color: %{public}u", GetWindowId());
    return 0xffffffff; // means no background color been set, default color is white
}

WMError WindowSessionImpl::SetLayoutFullScreenByApiVersion(bool status)
{
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UpdateSystemBarProperties(
    const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
{
    return WMError::WM_OK;
}

WMError WindowSessionImpl::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    return WMError::WM_OK;
}

void WindowSessionImpl::HookCompatibleModeAvoidAreaNotify()
{
    AvoidArea avoidArea;
    Rect rect = GetRect();
    GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM, avoidArea, rect);
    auto avoidSptr = sptr<AvoidArea>::MakeSptr(avoidArea);
    NotifyAvoidAreaChange(avoidSptr, AvoidAreaType::TYPE_SYSTEM);
}

void WindowSessionImpl::UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
    SystemBarProperty& property)
{
    property.enable_ = systemBarEnable;
    property.enableAnimation_ = systemBarEnableAnimation;
    // isolate on api 18
    if (GetTargetAPIVersion() >= API_VERSION_18) {
        property.settingFlag_ |= SystemBarSettingFlag::ENABLE_SETTING;
    }
}

WMError WindowSessionImpl::SetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
{
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UpdateStatusBarColorByColorMode(uint32_t& contentColor)
{
    SystemBarProperty statusBarProp = property_->GetSystemBarProperty().at(WindowType::WINDOW_TYPE_STATUS_BAR);
    if (static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(statusBarProp.settingFlag_) &
        static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING)) == SystemBarSettingFlag::COLOR_SETTING ||
        systemBarSettingFlag_ == SystemBarSettingFlag::COLOR_SETTING) {
        TLOGD(WmsLogTag::WMS_IMMS, "user has set color");
        return WMError::WM_DO_NOTHING;
    }
    constexpr uint32_t BLACK = 0xFF000000;
    constexpr uint32_t WHITE = 0xFFFFFFFF;
    bool isColorModeSetByApp = !specifiedColorMode_.empty();
    std::string colorMode = specifiedColorMode_;
    if (isColorModeSetByApp) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u type %{public}u colorMode %{public}s",
            GetPersistentId(), GetType(), colorMode.c_str());
        contentColor = colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT ? BLACK : WHITE;
    } else {
        auto appContext = AbilityRuntime::Context::GetApplicationContext();
        if (appContext == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "app context is nullptr");
            return WMError::WM_ERROR_NULLPTR;
        }
        std::shared_ptr<AppExecFwk::Configuration> config = appContext->GetConfiguration();
        if (config == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "config is null, winId: %{public}d", GetPersistentId());
            return WMError::WM_ERROR_NULLPTR;
        }
        colorMode = config->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        isColorModeSetByApp = !config->GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP).empty();
        if (isColorModeSetByApp) {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "win=%{public}u, appColor=%{public}s", GetWindowId(), colorMode.c_str());
            contentColor = colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT ? BLACK : WHITE;
        } else {
            bool hasDarkRes = false;
            appContext->AppHasDarkRes(hasDarkRes);
            TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u type %{public}u hasDarkRes %{public}u colorMode %{public}s",
                GetPersistentId(), GetType(), hasDarkRes, colorMode.c_str());
            contentColor = colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT ? BLACK :
                (hasDarkRes ? WHITE : BLACK);
        }
    }
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifyOccupiedAreaChangeInfoInner(sptr<OccupiedAreaChangeInfo> info)
{
    std::lock_guard<std::recursive_mutex> lockListener(occupiedAreaChangeListenerMutex_);
    auto occupiedAreaChangeListeners = GetListeners<IOccupiedAreaChangeListener>();
    for (auto& listener : occupiedAreaChangeListeners) {
        if (listener != nullptr) {
            listener->OnSizeChange(info);
        }
    }
}

void WindowSessionImpl::NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
    const std::shared_ptr<RSTransaction>& rsTransaction, const Rect& callingWindowRect,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "handler is nullptr");
        return;
    }
    auto task = [weak = wptr(this), info, rsTransaction, callingWindowRect, avoidAreas]() {
        if (info != nullptr) {
            TLOGNI(WmsLogTag::WMS_KEYBOARD, "transaction: %{public}d, safeHeight: %{public}u"
                ", occupied rect: x %{public}d, y %{public}d, w %{public}u, h %{public}u, "
                "callingWindowRect: %{public}s", rsTransaction != nullptr, info->safeHeight_, info->rect_.posX_,
                info->rect_.posY_, info->rect_.width_, info->rect_.height_, callingWindowRect.ToString().c_str());
        }
        auto window = weak.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "window is nullptr, notify occupied area change info failed");
            return;
        }
        if (rsTransaction) {
            RSTransactionAdapter::FlushImplicitTransaction(window->GetRSUIContext());
            rsTransaction->Begin();
        }
        window->NotifyOccupiedAreaChangeInfoInner(info);
        window->occupiedAreaInfo_ = info;
        window->UpdateViewportConfig(callingWindowRect, WindowSizeChangeReason::OCCUPIED_AREA_CHANGE,
            nullptr, nullptr, avoidAreas);
        if (rsTransaction) {
            rsTransaction->Commit();
        }
    };
    handler_->PostTask(task, "WMS_WindowSessionImpl_NotifyOccupiedAreaChangeInfo");
}

void WindowSessionImpl::NotifyKeyboardWillShow(const KeyboardAnimationInfo& keyboardAnimationInfo)
{
    std::lock_guard<std::recursive_mutex> lockListener(keyboardWillShowListenerMutex_);
    auto keyboardWillShowListeners = GetListeners<IKBWillShowListener>();
    for (const auto& listener : keyboardWillShowListeners) {
        if (listener != nullptr) {
            auto config = GetKeyboardAnimationConfig();
            auto animation = keyboardAnimationInfo.isShow ? config.curveIn : config.curveOut;
            listener->OnKeyboardWillShow(keyboardAnimationInfo, animation);
        }
    }
}

void WindowSessionImpl::NotifyKeyboardWillHide(const KeyboardAnimationInfo& keyboardAnimationInfo)
{
    std::lock_guard<std::recursive_mutex> lockListener(keyboardWillHideListenerMutex_);
    auto keyboardWillHideListeners = GetListeners<IKBWillHideListener>();
    for (const auto& listener : keyboardWillHideListeners) {
        if (listener != nullptr) {
            auto config = GetKeyboardAnimationConfig();
            auto animation = keyboardAnimationInfo.isShow ? config.curveIn : config.curveOut;
            listener->OnKeyboardWillHide(keyboardAnimationInfo, animation);
        }
    }
}

void WindowSessionImpl::NotifyKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo)
{
    std::lock_guard<std::recursive_mutex> lockListener(keyboardDidShowListenerMutex_);
    auto keyboardDidShowListeners = GetListeners<IKeyboardDidShowListener>();
    for (const auto& listener : keyboardDidShowListeners) {
        if (listener != nullptr) {
            listener->OnKeyboardDidShow(keyboardPanelInfo);
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        AAFwk::Want want;
        WriteKeyboardInfoToWant(want, keyboardPanelInfo);
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_KEYBOARD_DID_SHOW), want,
            keyboardDidShowUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

void WindowSessionImpl::NotifyKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo)
{
    std::lock_guard<std::recursive_mutex> lockListener(keyboardDidHideListenerMutex_);
    auto keyboardDidHideListeners = GetListeners<IKeyboardDidHideListener>();
    for (const auto& listener : keyboardDidHideListeners) {
        if (listener != nullptr) {
            listener->OnKeyboardDidHide(keyboardPanelInfo);
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        AAFwk::Want want;
        WriteKeyboardInfoToWant(want, keyboardPanelInfo);
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_KEYBOARD_DID_HIDE), want,
            keyboardDidHideUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

void WindowSessionImpl::WriteKeyboardInfoToWant(AAFwk::Want& want, const KeyboardPanelInfo& keyboardPanelInfo) const
{
    want.SetParam(Extension::RECT_X, static_cast<int>(keyboardPanelInfo.rect_.posX_));
    want.SetParam(Extension::RECT_Y, static_cast<int>(keyboardPanelInfo.rect_.posY_));
    want.SetParam(Extension::RECT_WIDTH, static_cast<int>(keyboardPanelInfo.rect_.width_));
    want.SetParam(Extension::RECT_HEIGHT, static_cast<int>(keyboardPanelInfo.rect_.height_));
    want.SetParam(Extension::BEGIN_X, static_cast<int>(keyboardPanelInfo.beginRect_.posX_));
    want.SetParam(Extension::BEGIN_Y, static_cast<int>(keyboardPanelInfo.beginRect_.posY_));
    want.SetParam(Extension::BEGIN_WIDTH, static_cast<int>(keyboardPanelInfo.beginRect_.width_));
    want.SetParam(Extension::BEGIN_HEIGHT, static_cast<int>(keyboardPanelInfo.beginRect_.height_));
    want.SetParam(Extension::END_X, static_cast<int>(keyboardPanelInfo.endRect_.posX_));
    want.SetParam(Extension::END_Y, static_cast<int>(keyboardPanelInfo.endRect_.posY_));
    want.SetParam(Extension::END_WIDTH, static_cast<int>(keyboardPanelInfo.endRect_.width_));
    want.SetParam(Extension::END_HEIGHT, static_cast<int>(keyboardPanelInfo.endRect_.height_));
    want.SetParam(Extension::GRAVITY, static_cast<int>(keyboardPanelInfo.gravity_));
    want.SetParam(Extension::ISSHOWING, keyboardPanelInfo.isShowing_);
}

void WindowSessionImpl::ReadKeyboardInfoFromWant(const AAFwk::Want& want, KeyboardPanelInfo& keyboardPanelInfo) const
{
    keyboardPanelInfo.rect_ = {
        want.GetIntParam(Extension::RECT_X, 0),
        want.GetIntParam(Extension::RECT_Y, 0),
        want.GetIntParam(Extension::RECT_WIDTH, 0),
        want.GetIntParam(Extension::RECT_HEIGHT, 0)
    };
    keyboardPanelInfo.beginRect_ = {
        want.GetIntParam(Extension::BEGIN_X, 0),
        want.GetIntParam(Extension::BEGIN_Y, 0),
        want.GetIntParam(Extension::BEGIN_WIDTH, 0),
        want.GetIntParam(Extension::BEGIN_HEIGHT, 0)
    };
    keyboardPanelInfo.endRect_ = {
        want.GetIntParam(Extension::END_X, 0),
        want.GetIntParam(Extension::END_Y, 0),
        want.GetIntParam(Extension::END_WIDTH, 0),
        want.GetIntParam(Extension::END_HEIGHT, 0)
    };
    keyboardPanelInfo.gravity_ = static_cast<WindowGravity>(want.GetIntParam(Extension::GRAVITY, 0));
    keyboardPanelInfo.isShowing_ = want.GetBoolParam(Extension::ISSHOWING, false);
}

std::set<sptr<WindowSessionImpl>>& WindowSessionImpl::GetWindowExtensionSessionSet()
{
    return g_windowExtensionSessionSet_;
}

void WindowSessionImpl::NotifyKeyboardAnimationCompleted(const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "isShowAnimation: %{public}d, beginRect: %{public}s, endRect: %{public}s",
        keyboardPanelInfo.isShowing_, keyboardPanelInfo.beginRect_.ToString().c_str(),
        keyboardPanelInfo.endRect_.ToString().c_str());
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "handler is nullptr");
        return;
    }
    auto task = [weak = wptr(this), keyboardPanelInfo]() {
        auto window = weak.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "window is nullptr, notify keyboard animation completed failed");
            return;
        }
        if (keyboardPanelInfo.isShowing_) {
            window->NotifyKeyboardDidShow(keyboardPanelInfo);
        } else {
            window->NotifyKeyboardDidHide(keyboardPanelInfo);
        }
    };
    handler_->PostTask(task, __func__);
}

void WindowSessionImpl::NotifyKeyboardAnimationWillBegin(const KeyboardAnimationInfo& keyboardAnimationInfo,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "isShowAnimation: %{public}d, beginRect: %{public}s, endRect: %{public}s \
        rsTransaction is empty:%{public}d",
        keyboardAnimationInfo.isShow, keyboardAnimationInfo.beginRect.ToString().c_str(),
        keyboardAnimationInfo.endRect.ToString().c_str(), (rsTransaction == nullptr));
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "handler is nullptr");
        return;
    }
    auto task = [weak = wptr(this), keyboardAnimationInfo, rsTransaction]() {
        auto window = weak.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "window is nullptr, notify keyboard animation completed failed");
            return;
        }

        if (rsTransaction != nullptr) {
            RSTransaction::FlushImplicitTransaction();
            rsTransaction->Begin();
        }
        if (keyboardAnimationInfo.isShow) {
            window->NotifyKeyboardWillShow(keyboardAnimationInfo);
        } else {
            window->NotifyKeyboardWillHide(keyboardAnimationInfo);
        }
        if (rsTransaction != nullptr) {
            rsTransaction->Commit();
        }
    };
    handler_->PostTask(task, __func__);
}

KeyboardAnimationConfig WindowSessionImpl::GetKeyboardAnimationConfig()
{
    return { windowSystemConfig_.animationIn_, windowSystemConfig_.animationOut_ };
}

void WindowSessionImpl::DumpSessionElementInfo(const std::vector<std::string>& params)
{
    WLOGFD("in");
}

WSError WindowSessionImpl::UpdateMaximizeMode(MaximizeMode mode)
{
    return WSError::WS_OK;
}

WMError WindowSessionImpl::TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
    int64_t uiExtensionIdLevel)
{
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifySessionForeground(uint32_t reason, bool withAnimation)
{
    WLOGFD("in");
}

void WindowSessionImpl::NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    WLOGFD("in");
}

WSError WindowSessionImpl::UpdateTitleInTargetPos(bool isShow, int32_t height)
{
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SwitchFreeMultiWindow(bool enable)
{
    return WSError::WS_OK;
}

WSError WindowSessionImpl::NotifyDialogStateChange(bool isForeground)
{
    return WSError::WS_OK;
}

void WindowSessionImpl::UpdatePiPRect(const Rect& rect, WindowSizeChangeReason reason)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PIP, "session is invalid");
        return;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->UpdatePiPRect(rect, static_cast<SizeChangeReason>(reason));
}

void WindowSessionImpl::UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PIP, "HostSession is invalid");
        return;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->UpdatePiPControlStatus(static_cast<WsPiPControlType>(controlType),
        static_cast<WsPiPControlStatus>(status));
}

void WindowSessionImpl::SetAutoStartPiP(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PIP, "session is invalid");
        return;
    }
    if (auto hostSession = GetHostSession()) {
        hostSession->SetAutoStartPiP(isAutoStart, priority, width, height);
    }
}

void WindowSessionImpl::UpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PIP, "session is invalid");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "UpdatePiPTemplateInfo, pipTemplateType: %{public}u, priority: %{public}d, "
        "defaultWindowSizeType: %{public}d, cornerAdsorptionEnabled: %{public}d", pipTemplateInfo.pipTemplateType,
        pipTemplateInfo.priority, pipTemplateInfo.defaultWindowSizeType,  pipTemplateInfo.cornerAdsorptionEnabled);
    if (auto hostSession = GetHostSession()) {
        hostSession->UpdatePiPTemplateInfo(pipTemplateInfo);
    }
}

WMError WindowSessionImpl::UpdateFloatingBall(const FloatingBallTemplateBaseInfo& fbTemplateBaseInfo,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "session is invalid");
        return WMError::WM_ERROR_FB_STATE_ABNORMALLY;
    }

    if (GetProperty()->GetFbTemplateInfo().template_ == static_cast<uint32_t>(FloatingBallTemplate::STATIC)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Fb static template can't update");
        return WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED;
    }

    if (GetProperty()->GetFbTemplateInfo().template_ != fbTemplateBaseInfo.template_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Fb template type can't update %{public}d, %{public}d",
            GetProperty()->GetFbTemplateInfo().template_, fbTemplateBaseInfo.template_);
        return WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED;
    }
    FloatingBallTemplateInfo fbTemplateInfo = FloatingBallTemplateInfo(fbTemplateBaseInfo, icon);
    GetProperty()->SetFbTemplateInfo(fbTemplateInfo);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_FB_STATE_ABNORMALLY);
    return hostSession->UpdateFloatingBall(fbTemplateInfo);
}

WMError WindowSessionImpl::RestoreFbMainWindow(const std::shared_ptr<AAFwk::Want>& want)
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "session is invalid");
        return WMError::WM_ERROR_FB_STATE_ABNORMALLY;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_FB_STATE_ABNORMALLY);
    return hostSession->RestoreFbMainWindow(want);
}

void WindowSessionImpl::NotifyPrepareCloseFloatingBall()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "NotifyPrepareCloseFloatingBall");
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "session is invalid");
        return;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->NotifyFloatingBallPrepareClose();
}

WindowStatus WindowSessionImpl::GetWindowStatusInner(WindowMode mode)
{
    auto windowStatus = WindowStatus::WINDOW_STATUS_UNDEFINED;
    if (mode == WindowMode::WINDOW_MODE_FLOATING) {
        windowStatus = WindowStatus::WINDOW_STATUS_FLOATING;
        if (property_->GetMaximizeMode() == MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
            windowStatus = WindowStatus::WINDOW_STATUS_MAXIMIZE;
        }
    } else if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        windowStatus = WindowStatus::WINDOW_STATUS_SPLITSCREEN;
    }
    if (mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
        if (IsPcOrPadFreeMultiWindowMode() && GetTargetAPIVersion() >= 14) { // 14: isolated version
            windowStatus = GetImmersiveModeEnabledState() ? WindowStatus::WINDOW_STATUS_FULLSCREEN :
                WindowStatus::WINDOW_STATUS_MAXIMIZE;
        } else {
            windowStatus = WindowStatus::WINDOW_STATUS_FULLSCREEN;
        }
    }
    if (state_ == WindowState::STATE_HIDDEN) {
        windowStatus = WindowStatus::WINDOW_STATUS_MINIMIZE;
    }
    return windowStatus;
}

uint32_t WindowSessionImpl::GetStatusBarHeight() const
{
    uint32_t height = 0;
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, height);
    height = static_cast<uint32_t>(hostSession->GetStatusBarHeight());
    TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u height %{public}u", GetPersistentId(), height);
    return height;
}

/** @note @window.layout */
void WindowSessionImpl::NotifyWindowStatusChange(WindowMode mode)
{
    auto windowStatus = GetWindowStatusInner(mode);
    if (windowSystemConfig_.skipRedundantWindowStatusNotifications_ && lastWindowStatus_.load() == windowStatus) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Duplicate windowStatus:%{public}d, id:%{public}d, WindowMode:%{public}d",
            windowStatus, GetPersistentId(), mode);
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, windowMode:%{public}d, windowStatus:%{public}d, "
        "lastWindowStatus:%{public}d, skipRedundantWindowStatusNotifications:%{public}d", GetPersistentId(), mode,
        windowStatus, lastWindowStatus_.load(), windowSystemConfig_.skipRedundantWindowStatusNotifications_);
    lastWindowStatus_.store(windowStatus);
    std::lock_guard<std::recursive_mutex> lockListener(windowStatusChangeListenerMutex_);
    auto windowStatusChangeListeners = GetListeners<IWindowStatusChangeListener>();
    for (auto& listener : windowStatusChangeListeners) {
        if (listener != nullptr) {
            listener->OnWindowStatusChange(windowStatus);
        }
    }
    if (state_ != WindowState::STATE_HIDDEN) {
        auto ret = HiSysEventWrite(
            HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "WINDOW_STATUS_CHANGE",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "BUNDLE_NAME", property_->GetSessionInfo().bundleName_,
            "WINDOW_MODE", static_cast<int32_t>(mode));
        if (ret) {
            TLOGW(WmsLogTag::WMS_FOCUS, "write event fail, WINDOW_STATUS_CHANGE, ret=%{public}d", ret);
        }
    }
}

/** @note @window.layout */
void WindowSessionImpl::NotifyWindowStatusDidChange(WindowMode mode)
{
    auto windowStatus = GetWindowStatusInner(mode);
    auto lastStatus = lastStatusWhenNotifyWindowStatusDidChange_.load();
    if (lastStatus == windowStatus) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Duplicate windowStatus:%{public}u, id:%{public}d, windowMode:%{public}u",
            windowStatus, GetPersistentId(), mode);
        return;
    }
    lastStatusWhenNotifyWindowStatusDidChange_.store(windowStatus);
    std::vector<sptr<IWindowStatusDidChangeListener>> windowStatusDidChangeListeners;
    {
        std::lock_guard<std::recursive_mutex> lockListener(windowStatusDidChangeListenerMutex_);
        windowStatusDidChangeListeners = GetListeners<IWindowStatusDidChangeListener>();
    }
    const auto& windowRect = GetRect();
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, WindowMode:%{public}u, windowStatus:%{public}u, "
        "lastWindowStatus:%{public}u, listenerSize:%{public}zu, rect:%{public}s",
        GetPersistentId(), mode, windowStatus, lastStatus, windowStatusDidChangeListeners.size(),
        windowRect.ToString().c_str());
    for (auto& listener : windowStatusDidChangeListeners) {
        if (listener != nullptr) {
            listener->OnWindowStatusDidChange(windowStatus);
        }
    }
}

void WindowSessionImpl::NotifyFirstValidLayoutUpdate(const Rect& preRect, const Rect& newRect)
{
    bool isFirstValidLayoutUpdate = true;
    if (!newRect.IsUninitializedSize() &&
        isFirstValidLayoutUpdate_.compare_exchange_strong(isFirstValidLayoutUpdate, false)) {
        updateRectCallback_->OnFirstValidRectUpdate(GetPersistentId());
        TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, rect:%{public}s", GetPersistentId(), newRect.ToString().c_str());
    }
}

template <typename T>
EnableIfSame<T, IPreferredOrientationChangeListener, sptr<IPreferredOrientationChangeListener>> WindowSessionImpl::GetListeners()
{
    sptr<IPreferredOrientationChangeListener> preferredOrientationChangeListener;
    preferredOrientationChangeListener = preferredOrientationChangeListener_[GetPersistentId()];
    return preferredOrientationChangeListener;
}

WMError WindowSessionImpl::RegisterPreferredOrientationChangeListener(
    const sptr<IPreferredOrientationChangeListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "listener is null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::mutex> lockListener(preferredOrientationChangeListenerMutex_);
    preferredOrientationChangeListener_[GetPersistentId()] = listener;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterPreferredOrientationChangeListener(
    const sptr<IPreferredOrientationChangeListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "listener is null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::mutex> lockListener(preferredOrientationChangeListenerMutex_);
    preferredOrientationChangeListener_[GetPersistentId()] = nullptr;
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifyPreferredOrientationChange(Orientation orientation)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    sptr<IPreferredOrientationChangeListener> preferredOrientationChangeListener;
    {
        std::lock_guard<std::mutex> lockListener(preferredOrientationChangeListenerMutex_);
        preferredOrientationChangeListener = GetListeners<IPreferredOrientationChangeListener>();
    }
    if (preferredOrientationChangeListener != nullptr) {
        preferredOrientationChangeListener->OnPreferredOrientationChange(orientation);
        TLOGI(WmsLogTag::WMS_ROTATION, "OnPreferredOrientationChange is success.");
    }
}

void WindowSessionImpl::NotifyClientOrientationChange()
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    sptr<IWindowOrientationChangeListener> windowOrientationChangeListener;
    {
        std::lock_guard<std::mutex> lockListener(windowOrientationChangeListenerMutex_);
        windowOrientationChangeListener = GetListeners<IWindowOrientationChangeListener>();
    }
    if (windowOrientationChangeListener != nullptr) {
        windowOrientationChangeListener->OnOrientationChange();
        TLOGI(WmsLogTag::WMS_ROTATION, "OnOrientationChange is success.");
    }
}

template<typename T>
EnableIfSame<T, IWindowOrientationChangeListener, sptr<IWindowOrientationChangeListener>> WindowSessionImpl::GetListeners()
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    sptr<IWindowOrientationChangeListener> windowOrientationChangeListener;
    windowOrientationChangeListener = windowOrientationChangeListener_[GetPersistentId()];
    return windowOrientationChangeListener;
}

WMError WindowSessionImpl::RegisterOrientationChangeListener(
    const sptr<IWindowOrientationChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "listener is null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::mutex> lockListener(windowOrientationChangeListenerMutex_);
    windowOrientationChangeListener_[GetPersistentId()] = listener;
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterOrientationChangeListener(
    const sptr<IWindowOrientationChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "in");
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "listener is null.");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::mutex> lockListener(windowOrientationChangeListenerMutex_);
    windowOrientationChangeListener_[GetPersistentId()] = nullptr;
    return WMError::WM_OK;
}

void WindowSessionImpl::NotifyTransformChange(const Transform& transform)
{
    WLOGFI("in");
    SetCurrentTransform(transform);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->UpdateTransform(transform);
        SetLayoutTransform(transform);
        SetNeedRenotifyTransform(false);
    } else {
        SetNeedRenotifyTransform(true);
    }
}

void WindowSessionImpl::NotifySingleHandTransformChange(const SingleHandTransform& singleHandTransform)
{
    singleHandTransform_ = singleHandTransform;
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, posX:%{public}d, posY:%{public}d, "
              "scaleX:%{public}f, scaleY:%{public}f", GetPersistentId(),
              singleHandTransform.posX, singleHandTransform.posY,
              singleHandTransform.scaleX, singleHandTransform.scaleY);
        uiContent->UpdateSingleHandTransform(singleHandTransform);
    } else {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, uiContent is nullptr", GetPersistentId());
    }
}

void WindowSessionImpl::SubmitNoInteractionMonitorTask(int32_t eventId,
    const IWindowNoInteractionListenerSptr& listener)
{
    auto task = [sessionWptr = wptr(this), eventId, listenerWptr = wptr(listener)]() {
        auto session = sessionWptr.promote();
        if (session == nullptr) {
            WLOGFE("windowInteractionMonitor task running failed, window session is null");
            return;
        }
        if (eventId != session->lastInteractionEventId_.load()) {
            WLOGFD("event id of windowInteractionMonitor has been changed, need not notify!");
            return;
        }
        if (session->state_ != WindowState::STATE_SHOWN) {
            WLOGFD("window state is not show, need not notify!");
            return;
        }
        session->NotifyNoInteractionTimeout(listenerWptr.promote());
    };
    handler_->PostTask(task, listener->GetTimeout());
}

void WindowSessionImpl::RefreshNoInteractionTimeoutMonitor()
{
    std::lock_guard<std::recursive_mutex> lockListener(windowNoInteractionListenerMutex_);
    if (windowNoInteractionListeners_[GetPersistentId()].empty()) {
        return;
    }
    this->lastInteractionEventId_.fetch_add(1);
    int32_t eventId = lastInteractionEventId_.load();
    auto noInteractionListeners = GetListeners<IWindowNoInteractionListener>();
    for (const auto& listenerItem : noInteractionListeners) {
        SubmitNoInteractionMonitorTask(eventId, listenerItem);
    }
}

bool WindowSessionImpl::IsUserOrientation(Orientation orientation) const
{
    if (orientation == Orientation::USER_ROTATION_PORTRAIT ||
        orientation == Orientation::USER_ROTATION_LANDSCAPE ||
        orientation == Orientation::USER_ROTATION_PORTRAIT_INVERTED ||
        orientation == Orientation::USER_ROTATION_LANDSCAPE_INVERTED) {
        return true;
    }
    return false;
}

bool WindowSessionImpl::IsUserPageOrientation(Orientation orientation) const
{
    if (orientation == Orientation::USER_PAGE_ROTATION_PORTRAIT ||
        orientation == Orientation::USER_PAGE_ROTATION_LANDSCAPE ||
        orientation == Orientation::USER_PAGE_ROTATION_PORTRAIT_INVERTED ||
        orientation == Orientation::USER_PAGE_ROTATION_LANDSCAPE_INVERTED) {
        return true;
    }
    return false;
}

bool WindowSessionImpl::IsVerticalOrientation(Orientation orientation) const
{
    if (orientation == Orientation::VERTICAL ||
        orientation == Orientation::REVERSE_VERTICAL ||
        orientation == Orientation::SENSOR_VERTICAL ||
        orientation == Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED ||
        orientation == Orientation::USER_ROTATION_PORTRAIT ||
        orientation == Orientation::USER_ROTATION_PORTRAIT_INVERTED) {
        return true;
    }
    return false;
}

bool WindowSessionImpl::IsHorizontalOrientation(Orientation orientation) const
{
    return orientation == Orientation::HORIZONTAL ||
           orientation == Orientation::REVERSE_HORIZONTAL ||
           orientation == Orientation::SENSOR_HORIZONTAL ||
           orientation == Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED ||
           orientation == Orientation::USER_ROTATION_LANDSCAPE ||
           orientation == Orientation::USER_ROTATION_LANDSCAPE_INVERTED;
}

WMError WindowSessionImpl::GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus) const
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "callingWindowId: %{public}d", callingWindowId);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().GetCallingWindowWindowStatus(callingWindowId, windowStatus);
}

WMError WindowSessionImpl::GetCallingWindowRect(uint32_t callingWindowId, Rect& rect) const
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().GetCallingWindowRect(callingWindowId, rect);
}

void WindowSessionImpl::SetUiDvsyncSwitch(bool dvsyncSwitch)
{
    if (vsyncStation_ == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
        return;
    }
    vsyncStation_->SetUiDvsyncSwitch(dvsyncSwitch);
}

void WindowSessionImpl::SetTouchEvent(int32_t touchType)
{
    if (vsyncStation_ == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "vsyncStation is null");
        return;
    }
    vsyncStation_->SetTouchEvent(touchType);
}

WSError WindowSessionImpl::NotifyAppForceLandscapeConfigUpdated()
{
    return WSError::WS_DO_NOTHING;
}

WSError WindowSessionImpl::NotifyAppForceLandscapeConfigEnableUpdated()
{
    return WSError::WS_DO_NOTHING;
}

void WindowSessionImpl::SetFrameLayoutCallbackEnable(bool enable)
{
    enableFrameLayoutFinishCb_ = enable;
}

void WindowSessionImpl::UpdateFrameLayoutCallbackIfNeeded(WindowSizeChangeReason wmReason)
{
    bool isDragInPcmode = IsFreeMultiWindowMode() && (wmReason == WindowSizeChangeReason::DRAG_END);
    if (wmReason == WindowSizeChangeReason::FULL_TO_SPLIT || wmReason == WindowSizeChangeReason::SPLIT_TO_FULL ||
        wmReason == WindowSizeChangeReason::FULL_TO_FLOATING || wmReason == WindowSizeChangeReason::FLOATING_TO_FULL ||
        isDragInPcmode) {
        TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "enable framelayoutfinish callback reason:%{public}u", wmReason);
        SetFrameLayoutCallbackEnable(true);
    }
}

WMError WindowSessionImpl::SetContinueState(int32_t continueState)
{
    if (continueState > ContinueState::CONTINUESTATE_MAX || continueState < ContinueState::CONTINUESTATE_UNKNOWN) {
        TLOGE(WmsLogTag::WMS_MAIN, "continueState is invalid: %{public}d", continueState);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    property_->EditSessionInfo().continueState = static_cast<ContinueState>(continueState);
    return WMError::WM_OK;
}

void WindowSessionImpl::SetUIContentComplete()
{
    bool setUIContentCompleted = false;
    if (setUIContentCompleted_.compare_exchange_strong(setUIContentCompleted, true)) {
        TLOGI(WmsLogTag::WMS_LIFE, "persistentId=%{public}d", GetPersistentId());
        handler_->RemoveTask(SET_UICONTENT_TIMEOUT_LISTENER_TASK_NAME + std::to_string(GetPersistentId()));
    } else {
        TLOGI(WmsLogTag::WMS_LIFE, "already SetUIContent, persistentId=%{public}d", GetPersistentId());
    }
}

void WindowSessionImpl::AddSetUIContentTimeoutCheck()
{
    const auto checkBeginTime =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())
            .time_since_epoch().count();
    auto task = [weakThis = wptr(this), checkBeginTime] {
        auto window = weakThis.promote();
        if (window == nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "window is nullptr");
            return;
        }
        if (window->setUIContentCompleted_.load()) {
            TLOGNI(WmsLogTag::WMS_LIFE, "already SetUIContent, persistentId=%{public}d", window->GetPersistentId());
            return;
        }

        const auto checkEndTime =
            std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())
                .time_since_epoch().count();
        if (checkEndTime - checkBeginTime > SET_UICONTENT_TIMEOUT_TIME_AFTER_FREEZE_MS) {
            TLOGNI(WmsLogTag::WMS_LIFE, "will start re-check after freeze, persistentId=%{public}d",
                window->GetPersistentId());
            window->AddSetUIContentTimeoutCheck();
            return;
        }

        TLOGNW(WmsLogTag::WMS_LIFE, "SetUIContent timeout, persistentId=%{public}d", window->GetPersistentId());
        std::ostringstream oss;
        oss << "SetUIContent timeout uid: " << getuid();
        oss << ", windowName: " << window->GetWindowName();
        auto context = window->GetContext();
        if (context) {
            oss << ", bundleName: " << context->GetBundleName();
            if (context->GetApplicationInfo()) {
                oss << ", abilityName: " << context->GetApplicationInfo()->name;
            }
        }
        SingletonContainer::Get<WindowInfoReporter>().ReportWindowException(
            static_cast<int32_t>(WindowDFXHelperType::WINDOW_TRANSPARENT_CHECK), getpid(), oss.str());

        if (WindowHelper::IsUIExtensionWindow(window->GetType())) {
            window->NotifyExtensionTimeout(TimeoutErrorCode::SET_UICONTENT_TIMEOUT);
        }
    };
    handler_->PostTask(task, SET_UICONTENT_TIMEOUT_LISTENER_TASK_NAME + std::to_string(GetPersistentId()),
        SET_UICONTENT_TIMEOUT_TIME_MS, AppExecFwk::EventQueue::Priority::HIGH);
}

void WindowSessionImpl::NotifySetUIContentComplete()
{
    if (WindowHelper::IsSubWindow(GetType()) || WindowHelper::IsSystemWindow(GetType())) {
        // created by UIExtension
        auto extWindow = FindExtensionWindowWithContext();
        if (extWindow != nullptr) {
            extWindow->SetUIContentComplete();
        }
    }
    SetUIContentComplete();
}

void WindowSessionImpl::SetUIExtensionDestroyComplete()
{
    bool setUIExtensionDestroyCompleted = false;
    if (setUIExtensionDestroyCompleted_.compare_exchange_strong(setUIExtensionDestroyCompleted, true)) {
        TLOGI(WmsLogTag::WMS_LIFE, "persistentId=%{public}d", GetPersistentId());
        handler_->RemoveTask(SET_UIEXTENSION_DESTROY_TIMEOUT_LISTENER_TASK_NAME + std::to_string(GetPersistentId()));
    } else {
        TLOGI(WmsLogTag::WMS_LIFE, "already, persistentId=%{public}d", GetPersistentId());
    }
}

void WindowSessionImpl::SetUIExtensionDestroyCompleteInSubWindow()
{
    if (WindowHelper::IsSubWindow(GetType()) || WindowHelper::IsSystemWindow(GetType())) {
        bool startUIExtensionDestroyTimer = true;
        auto extensionWindow = FindExtensionWindowWithContext();
        if (extensionWindow != nullptr && extensionWindow->startUIExtensionDestroyTimer_.compare_exchange_strong(
            startUIExtensionDestroyTimer, false)) {
            TLOGI(WmsLogTag::WMS_LIFE, "called");
            extensionWindow->SetUIExtensionDestroyComplete();
            extensionWindow->setUIExtensionDestroyCompleted_.store(false);
        }
    }
}

void WindowSessionImpl::AddSetUIExtensionDestroyTimeoutCheck()
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), where] {
        auto window = weakThis.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: window is nullptr", where);
            return;
        }
        if (window->setUIExtensionDestroyCompleted_.load()) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: already, persistentId=%{public}d", where,
                window->GetPersistentId());
            return;
        }

        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: timeout, persistentId=%{public}d", where, window->GetPersistentId());
        std::ostringstream oss;
        oss << "SetUIExtDestroy timeout uid: " << getuid();
        oss << ", windowName: " << window->GetWindowName();
        auto context = window->GetContext();
        if (context) {
            oss << ", bundleName: " << context->GetBundleName();
            if (context->GetApplicationInfo()) {
                oss << ", abilityName: " << context->GetApplicationInfo()->name;
            }
        }
        SingletonContainer::Get<WindowInfoReporter>().ReportWindowException(
            static_cast<int32_t>(WindowDFXHelperType::WINDOW_TRANSPARENT_CHECK), getpid(), oss.str());

        if (WindowHelper::IsUIExtensionWindow(window->GetType())) {
            window->NotifyExtensionTimeout(TimeoutErrorCode::SET_UIEXTENSION_DESTROY_TIMEOUT);
        }
    };
    handler_->PostTask(task, SET_UIEXTENSION_DESTROY_TIMEOUT_LISTENER_TASK_NAME + std::to_string(GetPersistentId()),
        SET_UIEXTENSION_DESTROY_TIMEOUT_TIME_MS, AppExecFwk::EventQueue::Priority::HIGH);
    startUIExtensionDestroyTimer_.store(true);
}

WSError WindowSessionImpl::SetEnableDragBySystem(bool enableDrag)
{
    TLOGE(WmsLogTag::WMS_LAYOUT, "enableDrag: %{public}d", enableDrag);
    property_->SetDragEnabled(enableDrag);
    hasSetEnableDrag_.store(true);
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SetDragActivated(bool dragActivated)
{
    dragActivated_ = dragActivated;
    return WSError::WS_OK;
}

bool WindowSessionImpl::IsWindowDraggable()
{
    bool isDragEnabled = GetProperty()->GetDragEnabled();
    TLOGD(WmsLogTag::WMS_LAYOUT, "PersistentId: %{public}d, dragEnabled: %{public}d, dragActivate: %{public}d",
        GetPersistentId(), isDragEnabled, dragActivated_.load());
    return isDragEnabled && dragActivated_.load();
}

void WindowSessionImpl::SetTargetAPIVersion(uint32_t targetAPIVersion)
{
    targetAPIVersion_ = targetAPIVersion;
}

uint32_t WindowSessionImpl::GetTargetAPIVersion() const
{
    return targetAPIVersion_;
}

uint32_t WindowSessionImpl::GetTargetAPIVersionByApplicationInfo() const
{
    return (context_ != nullptr && context_->GetApplicationInfo() != nullptr) ?
        static_cast<uint32_t>(context_->GetApplicationInfo()->apiTargetVersion) % API_VERSION_MOD :
        INVALID_TARGET_API_VERSION;
}

bool WindowSessionImpl::IsDeviceFeatureCapableFor(const std::string& feature) const
{
    if (context_ == nullptr || context_->GetHapModuleInfo() == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window [%{public}d] context is nullptr", GetPersistentId());
        return false;
    }
    std::string deviceType = system::GetParameter("const.product.devicetype", "");
    std::map<std::string, std::vector<std::string>>& requiredDeviceFeatures =
        context_->GetHapModuleInfo()->requiredDeviceFeatures;
    if (requiredDeviceFeatures.find(deviceType) == requiredDeviceFeatures.end()) {
        return false;
    }
    auto& features = requiredDeviceFeatures[deviceType];
    return std::find(features.begin(), features.end(), feature) != features.end();
}

bool WindowSessionImpl::IsDeviceFeatureCapableForFreeMultiWindow() const
{
    static const std::string DEVICE_FEATURE_LARGE_SCREEN = "large_screen";
    return IsDeviceFeatureCapableFor(DEVICE_FEATURE_LARGE_SCREEN)
        && system::GetParameter("const.window.device_feature_support_type", "0") == "1";
}

void WindowSessionImpl::SetLayoutTransform(const Transform& transform)
{
    std::lock_guard<std::recursive_mutex> lock(transformMutex_);
    layoutTransform_ = transform;
}

Transform WindowSessionImpl::GetLayoutTransform() const
{
    std::lock_guard<std::recursive_mutex> lock(transformMutex_);
    return layoutTransform_;
}

WMError WindowSessionImpl::UpdateCompatScaleInfo(const Transform& transform)
{
    if (!WindowHelper::IsMainWindow(GetType()) || !IsInCompatScaleMode()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "id:%{public}d not scale mode", GetPersistentId());
        return WMError::WM_DO_NOTHING;
    }
    compatScaleX_ = transform.scaleX_;
    compatScaleY_ = transform.scaleY_;
    AAFwk::Want want;
    want.SetParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, IsAdaptToSimulationScale());
    want.SetParam(Extension::COMPAT_IS_PROPORTION_SCALE_FIELD, IsAdaptToProportionalScale());
    want.SetParam(Extension::COMPAT_SCALE_X_FIELD, compatScaleX_);
    want.SetParam(Extension::COMPAT_SCALE_Y_FIELD, compatScaleY_);
    NotifyTitleChange(true, 0);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_COMPAT_INFO),
            want, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WMError::WM_OK;
}

void WindowSessionImpl::SetCurrentTransform(const Transform& transform)
{
    {
        std::lock_guard<std::mutex> lock(currentTransformMutex_);
        currentTransform_ = transform;
    }
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "handler is nullptr");
        return;
    }
    handler_->PostTask([weakThis = wptr(this), transform, where = __func__] {
        auto window = weakThis.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_COMPAT, "%{public}s window is null", where);
            return;
        }
        window->UpdateCompatScaleInfo(transform);
    }, __func__);
}

Transform WindowSessionImpl::GetCurrentTransform() const
{
    std::lock_guard<std::mutex> lock(currentTransformMutex_);
    return currentTransform_;
}

void WindowSessionImpl::RegisterWindowInspectorCallback()
{
    auto getWMSWindowListCallback = [weakThis = wptr(this)]() -> std::optional<WindowListInfo> {
        if (auto window = weakThis.promote()) {
            return std::make_optional<WindowListInfo>({
                window->GetWindowName(), window->GetWindowId(),
                static_cast<uint32_t>(window->GetType()), window->GetRect()
            });
        } else {
            return std::nullopt;
        }
    };
    WindowInspector::GetInstance().RegisterGetWMSWindowListCallback(GetWindowId(), std::move(getWMSWindowListCallback));
}

void WindowSessionImpl::SetCompatInfoInExtensionConfig(AAFwk::WantParams& want) const
{
    if (!IsInCompatScaleMode()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "id:%{public}d not scale mode", GetPersistentId());
        return;
    }
    want.SetParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(IsAdaptToSimulationScale())));
    want.SetParam(Extension::COMPAT_IS_PROPORTION_SCALE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(IsAdaptToProportionalScale())));
    want.SetParam(Extension::COMPAT_SCALE_X_FIELD, AAFwk::Float::Box(compatScaleX_));
    want.SetParam(Extension::COMPAT_SCALE_Y_FIELD, AAFwk::Float::Box(compatScaleY_));
}

void WindowSessionImpl::GetExtensionConfig(AAFwk::WantParams& want) const
{
    want.SetParam(Extension::CROSS_AXIS_FIELD, AAFwk::Integer::Box(static_cast<int32_t>(crossAxisState_.load())));
    bool isWaterfallMode = isFullScreenWaterfallMode_.load();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "waterfall: %{public}d, winId: %{public}u", isWaterfallMode, GetWindowId());
    want.SetParam(Extension::WATERFALL_MODE_FIELD, AAFwk::Integer::Box(static_cast<int32_t>(isWaterfallMode)));
    WindowType rootHostWindowType = (GetType() == WindowType::WINDOW_TYPE_UI_EXTENSION) ?
                                    GetRootHostWindowType() : GetType();
    want.SetParam(Extension::ROOT_HOST_WINDOW_TYPE_FIELD,
                  AAFwk::Integer::Box(static_cast<int32_t>(rootHostWindowType)));
    bool isHostWindowDelayRaiseEnabled = IsWindowDelayRaiseEnabled();
    want.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD,
        AAFwk::Integer::Box(static_cast<int32_t>(isHostWindowDelayRaiseEnabled)));
    SetCompatInfoInExtensionConfig(want);
    bool gestureBackEnable = true;
    GetGestureBackEnabled(gestureBackEnable);
    want.SetParam(Extension::GESTURE_BACK_ENABLED, AAFwk::Integer::Box(static_cast<int32_t>(gestureBackEnable)));
}

WMError WindowSessionImpl::OnExtensionMessage(uint32_t code, int32_t persistentId, const AAFwk::Want& data)
{
    switch (code) {
        case static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_TO_RAISE): {
            return HandleHostWindowRaise(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER): {
            return HandleRegisterHostWindowRectChangeListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER): {
            return HandleUnregisterHostWindowRectChangeListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_SHOW_LISTENER): {
            return HandleUIExtRegisterKeyboardDidShowListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_SHOW_LISTENER): {
            return HandleUIExtUnregisterKeyboardDidShowListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_HIDE_LISTENER): {
            return HandleUIExtRegisterKeyboardDidHideListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_HIDE_LISTENER): {
            return HandleUIExtUnregisterKeyboardDidHideListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER): {
            return HandleRegisterHostRectChangeInGlobalDisplayListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER): {
            return HandleUnregisterHostRectChangeInGlobalDisplayListener(code, persistentId, data);
            break;
        }
        default: {
            TLOGI(WmsLogTag::WMS_UIEXT, "Message was not processed, businessCode: %{public}u", code);
            break;
        }
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleHostWindowRaise(uint32_t code, int32_t persistentId, const AAFwk::Want& data)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    auto ret = RaiseToAppTopOnDrag();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Raise host window failed, winId: %{public}u, errCode: %{public}d",
            GetWindowId(), ret);
        return ret;
    }
    TLOGI(WmsLogTag::WMS_HIERARCHY, "Raise host window successfully, winId: %{public}u", GetWindowId());
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleRegisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    rectChangeUIExtListenerIds_.emplace(persistentId);
    TLOGI(WmsLogTag::WMS_UIEXT, "persistentId: %{public}d register rect change listener", persistentId);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleUnregisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    rectChangeUIExtListenerIds_.erase(persistentId);
    TLOGI(WmsLogTag::WMS_UIEXT, "persistentId: %{public}d unregister rect change listener", persistentId);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleRegisterHostRectChangeInGlobalDisplayListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    rectChangeInGlobalDisplayUIExtListenerIds_.emplace(persistentId);
    TLOGI(WmsLogTag::WMS_UIEXT, "persistentId: %{public}d register rect change in global display", persistentId);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleUnregisterHostRectChangeInGlobalDisplayListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    rectChangeInGlobalDisplayUIExtListenerIds_.erase(persistentId);
    TLOGI(WmsLogTag::WMS_UIEXT, "persistentId: %{public}d unregister rect change in global display", persistentId);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleUIExtRegisterKeyboardDidShowListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    if (keyboardDidShowUIExtListeners_.find(persistentId) == keyboardDidShowUIExtListeners_.end()) {
        sptr<IKeyboardDidShowListener> listener = sptr<IKeyboardDidShowListener>::MakeSptr();
        keyboardDidShowUIExtListeners_[persistentId] = listener;
        keyboardDidShowUIExtListenerIds_.emplace(persistentId);
        RegisterKeyboardDidShowListener(listener);
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleUIExtUnregisterKeyboardDidShowListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    if (keyboardDidShowUIExtListeners_.find(persistentId) != keyboardDidShowUIExtListeners_.end()) {
        sptr<IKeyboardDidShowListener> listener = keyboardDidShowUIExtListeners_[persistentId];
        keyboardDidShowUIExtListeners_.erase(persistentId);
        keyboardDidShowUIExtListenerIds_.erase(persistentId);
        UnregisterKeyboardDidShowListener(listener);
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleUIExtRegisterKeyboardDidHideListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    if (keyboardDidHideUIExtListeners_.find(persistentId) == keyboardDidHideUIExtListeners_.end()) {
        sptr<IKeyboardDidHideListener> listener = sptr<IKeyboardDidHideListener>::MakeSptr();
        keyboardDidHideUIExtListeners_[persistentId] = listener;
        keyboardDidHideUIExtListenerIds_.emplace(persistentId);
        RegisterKeyboardDidHideListener(listener);
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::HandleUIExtUnregisterKeyboardDidHideListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    if (keyboardDidHideUIExtListeners_.find(persistentId) != keyboardDidHideUIExtListeners_.end()) {
        sptr<IKeyboardDidHideListener> listener = keyboardDidHideUIExtListeners_[persistentId];
        keyboardDidHideUIExtListeners_.erase(persistentId);
        keyboardDidHideUIExtListenerIds_.erase(persistentId);
        UnregisterKeyboardDidHideListener(listener);
    }
    return WMError::WM_OK;
}

bool WindowSessionImpl::IsValidCrossState(int32_t state) const
{
    return state >= static_cast<int32_t>(CrossAxisState::STATE_INVALID) &&
        state < static_cast<int32_t>(CrossAxisState::STATE_END);
}

void WindowSessionImpl::UpdateSubWindowInfo(uint32_t subWindowLevel,
    const std::shared_ptr<AbilityRuntime::Context>& context)
{
    property_->SetSubWindowLevel(subWindowLevel);
    SetContext(context);
    std::vector<sptr<WindowSessionImpl>> subWindows;
    GetSubWindows(GetPersistentId(), subWindows);
    for (auto& subWindow : subWindows) {
        if (subWindow != nullptr) {
            subWindow->UpdateSubWindowInfo(subWindowLevel + 1, context);
        }
    }
}

bool WindowSessionImpl::IsFullScreenPcAppInPadMode() const
{
    return property_->GetIsPcAppInPad() && GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN
        && !IsFreeMultiWindowMode();
}

void WindowSessionImpl::NotifyClientWindowSize()
{
    if (IsFullScreenPcAppInPadMode()) {
        const auto& windowRect = GetRect();
        NotifySizeChange(windowRect, WindowSizeChangeReason::MOVE);
        auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(property_->GetDisplayId());
        sptr<DisplayInfo> displayInfo = display ? display->GetDisplayInfo() : nullptr;
        UpdateViewportConfig(windowRect, WindowSizeChangeReason::UNDEFINED, nullptr, displayInfo);
    }
}

WSError WindowSessionImpl::NotifyPageRotationIsIgnored()
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    NotifyClientOrientationChange();
    return WSError::WS_OK;
}

WSError WindowSessionImpl::SetCurrentRotation(int32_t currentRotation)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "currentRotation: %{public}d", currentRotation);
    if (currentRotation > FULL_CIRCLE_DEGREE || currentRotation < ZERO_CIRCLE_DEGREE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "currentRotation is invalid: %{public}d", currentRotation);
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    property_->EditSessionInfo().currentRotation_ = currentRotation;
    return WSError::WS_OK;
}

WMError WindowSessionImpl::CheckMultiWindowRect(uint32_t& width, uint32_t& height)
{
    const auto& requestRect = GetRequestRect();

    if (WindowHelper::IsSubWindow(GetType())) {
        auto mainWindow = FindMainWindowWithContext();
        if (mainWindow != nullptr && (mainWindow->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
                                      mainWindow->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY)) {
            if (width == requestRect.width_ && height == requestRect.height_) {
                TLOGW(WmsLogTag::WMS_LAYOUT, "Request same size in multiWindow will not update, return");
                return WMError::WM_ERROR_INVALID_WINDOW;
            }
        }
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::GetRouterStackInfo(std::string& routerStackInfo)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "uiContent is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d", GetPersistentId());
    routerStackInfo = uiContent->GetTopNavDestinationInfo();
    return WMError::WM_OK;
}

RotationChangeResult WindowSessionImpl::NotifyRotationChange(const RotationChangeInfo& rotationChangeInfo)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "info type: %{public}d, orientation: %{public}d, "
        "rect:[%{public}d, %{public}d, %{public}d, %{public}d]", rotationChangeInfo.type_,
        rotationChangeInfo.orientation_, rotationChangeInfo.displayRect_.posX_,
        rotationChangeInfo.displayRect_.posY_, rotationChangeInfo.displayRect_.width_,
        rotationChangeInfo.displayRect_.height_);
    NotifyRotationChangeResultInner(rotationChangeInfo);
    RotationChangeResult rotationChangeResult = { RectType::RELATIVE_TO_SCREEN, { 0, 0, 0, 0 } };
    getRotationResultFuture_->ResetRotationResultLock();
    rotationChangeResult = getRotationResultFuture_->GetRotationResult(WINDOW_ROTATION_CHANGE);
    Rect resultRect = rotationChangeResult.windowRect_;
    if (CheckAndModifyWindowRect(resultRect.width_, resultRect.height_) != WMError::WM_OK ||
        CheckMultiWindowRect(resultRect.width_, resultRect.height_) != WMError::WM_OK) {
        rotationChangeResult.windowRect_.width_ = 0;
        rotationChangeResult.windowRect_.height_ = 0;
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "result rectType: %{public}d, rect:[%{public}d, %{public}d, %{public}d, %{public}d]",
        rotationChangeResult.rectType_, rotationChangeResult.windowRect_.posX_, rotationChangeResult.windowRect_.posY_,
        rotationChangeResult.windowRect_.width_, rotationChangeResult.windowRect_.height_);
    return rotationChangeResult;
}

void WindowSessionImpl::NotifyRotationChangeResult(RotationChangeResult rotationChangeResult)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "release rotation change lock.");
    if (getRotationResultFuture_) {
        getRotationResultFuture_->OnUpdateRotationResult(rotationChangeResult);
    }
}

void WindowSessionImpl::NotifyRotationChangeResultInner(const RotationChangeInfo& rotationChangeInfo)
{
    std::vector<sptr<IWindowRotationChangeListener>> windowRotationChangeListeners;
    {
        std::lock_guard<std::mutex> lockListener(windowRotationChangeListenerMutex_);
        windowRotationChangeListeners = GetListeners<IWindowRotationChangeListener>();
    }
    handler_->PostTask(
        [weakThis = wptr(this), windowRotationChangeListeners, rotationChangeInfo] {
            TLOGI(WmsLogTag::WMS_ROTATION, "post task to notify listener.");
            auto window = weakThis.promote();
            if (window == nullptr) {
                TLOGE(WmsLogTag::WMS_ROTATION, "window is null");
                return;
            }
            RotationChangeResult rotationChangeResult = { RectType::RELATIVE_TO_SCREEN, { 0, 0, 0, 0 } };
            for (auto& listener : windowRotationChangeListeners) {
                if (listener == nullptr) {
                    continue;
                }
                listener->OnRotationChange(rotationChangeInfo, rotationChangeResult);
                if (rotationChangeInfo.type_ == RotationChangeType::WINDOW_DID_ROTATE) {
                    continue;
                }
            }
            window->NotifyRotationChangeResult(rotationChangeResult);
        }, __func__);
}

bool WindowSessionImpl::IsSubWindowMaximizeSupported() const
{
    if (!WindowHelper::IsSubWindow(GetType())) {
        return false;
    }
    if (windowOption_ != nullptr) {
        return windowOption_->GetSubWindowMaximizeSupported();
    }
    return false;
}

WMError WindowSessionImpl::SetIntentParam(const std::string& intentParam,
    const std::function<void()>& loadPageCallback, bool isColdStart)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    intentParam_ = intentParam;
    loadPageCallback_ = loadPageCallback;
    isIntentColdStart_ = isColdStart;
    return WMError::WM_OK;
}

std::shared_ptr<RSUIDirector> WindowSessionImpl::GetRSUIDirector() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, windowId: %{public}u",
          RSAdapterUtil::RSUIDirectorToStr(rsUIDirector_).c_str(), GetWindowId());
    return rsUIDirector_;
}

std::shared_ptr<RSUIContext> WindowSessionImpl::GetRSUIContext() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    auto rsUIContext = rsUIDirector_ ? rsUIDirector_->GetRSUIContext() : nullptr;
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, windowId: %{public}u",
          RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str(), GetWindowId());
    return rsUIContext;
}

nlohmann::json WindowSessionImpl::SetContainerButtonStyle(const DecorButtonStyle& decorButtonStyle)
{
    nlohmann::json decorJson;
    decorJson.emplace(BUTTON_BACKGROUND_CORNER_RADIUS, decorButtonStyle.buttonBackgroundCornerRadius);
    decorJson.emplace(BUTTON_BACKGROUND_SIZE, decorButtonStyle.buttonBackgroundSize);
    decorJson.emplace(BUTTON_ICON_SIZE, decorButtonStyle.buttonIconSize);
    decorJson.emplace(CLOSE_BUTTON_RIGHT_MARGIN, decorButtonStyle.closeButtonRightMargin);
    decorJson.emplace(BUTTON_COLOR_MODE, decorButtonStyle.colorMode);
    decorJson.emplace(BUTTON_SPACING_BETWEEN, decorButtonStyle.spacingBetweenButtons);
    return decorJson;
}

bool WindowSessionImpl::IsAnco() const
{
    return property_->GetCollaboratorType() == static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
}

bool WindowSessionImpl::OnPointDown(int32_t eventId, int32_t posX, int32_t posY)
{
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowId:%{public}u,%{public}d", GetWindowId(), eventId);
        return false;
    }

    auto ret = hostSession->ProcessPointDownSession(posX, posY);
    TLOGD(WmsLogTag::WMS_EVENT, "windowId:%{public}u,%{public}d,%{public}d", GetWindowId(), eventId, ret);
    return ret == WSError::WS_OK;
}

void WindowSessionImpl::SetNavDestinationInfo(const std::string& navDestinationInfo)
{
    navDestinationInfo_ = navDestinationInfo;
}

WMError WindowSessionImpl::GetPiPSettingSwitchStatus(bool& switchStatus) const
{
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PIP, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    return SingletonContainer::Get<WindowAdapter>().GetPiPSettingSwitchStatus(switchStatus);
}

WMError WindowSessionImpl::SetPipParentWindowId(uint32_t windowId) const
{
    TLOGI(WmsLogTag::WMS_PIP, "ParentWindowId:%{public}u", windowId);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_PIP, "session is invalid");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    auto ret = hostSession->SetPipParentWindowId(windowId);
    if (ret != WSError::WS_OK) {
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::IsPiPActive(bool& status)
{
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGE(WmsLogTag::WMS_PIP, "hostSession is null");
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    auto ret = hostSession->IsPiPActive(status);
    if (ret != WMError::WM_OK) {
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    return WMError::WM_OK;
}

void WindowSessionImpl::SwitchSubWindow(bool freeMultiWindowEnable, int32_t parentId)
{
    std::lock_guard<std::recursive_mutex> lock(subWindowSessionMutex_);
    if (subWindowSessionMap_.count(parentId) == 0) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "subWindowSessionMap is empty");
        return;
    }
    for (auto& subWindowSession : subWindowSessionMap_.at(parentId)) {
        if (subWindowSession &&
            subWindowSession->windowSystemConfig_.freeMultiWindowEnable_ != freeMultiWindowEnable) {
            subWindowSession->SetFreeMultiWindowMode(freeMultiWindowEnable);
            subWindowSession->UpdateTitleButtonVisibility();
            subWindowSession->UpdateDecorEnable(true);
            subWindowSession->NotifyFreeWindowModeChange(freeMultiWindowEnable);
            subWindowSession->SwitchSubWindow(freeMultiWindowEnable, subWindowSession->GetPersistentId());
        }
    }
}

void WindowSessionImpl::SwitchSystemWindow(bool freeMultiWindowEnable, int32_t parentId)
{
    std::lock_guard<std::shared_mutex> lock(windowSessionMutex_);
    if (windowSessionMap_.empty()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "windowSessionMap is empty.");
        return;
    }
    for (const auto& winPair : windowSessionMap_) {
        auto systemWindowSession = winPair.second.second;
        if (systemWindowSession && WindowHelper::IsSystemWindow(systemWindowSession->GetType())) {
            systemWindowSession->SetFreeMultiWindowMode(freeMultiWindowEnable);
            systemWindowSession->NotifyFreeWindowModeChange(freeMultiWindowEnable);
        }
    }
}

void WindowSessionImpl::SetNotifySizeChangeFlag(bool flag)
{
    // Support for all system windows
    if (flag) {
        if (GetRect() == GetRequestRect()) {
            return;
        }
        notifySizeChangeFlag_ = true;
        TLOGD(WmsLogTag::WMS_LAYOUT, "Set notify size change flag is true");
        return;
    }
    notifySizeChangeFlag_ = false;
}

WSError WindowSessionImpl::UpdateIsShowDecorInFreeMultiWindow(bool isShow)
{
    TLOGI(WmsLogTag::WMS_DECOR, "id: %{public}d, isShow: %{public}d", GetPersistentId(), isShow);
    property_->SetIsShowDecorInFreeMultiWindow(isShow);
    return WSError::WS_OK;
}

template<typename T>
EnableIfSame<T, IFreeWindowModeChangeListener, std::vector<sptr<IFreeWindowModeChangeListener>>> WindowSessionImpl::GetListeners()
{
    std::vector<sptr<IFreeWindowModeChangeListener>> freeWindowModeChangeListeners;
    for (auto& listener : freeWindowModeChangeListeners_[GetPersistentId()]) {
        freeWindowModeChangeListeners.push_back(listener);
    }
    return freeWindowModeChangeListeners;
}
 
WMError WindowSessionImpl::RegisterFreeWindowModeChangeListener(const sptr<IFreeWindowModeChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "Start register");
    if (listener) {
        std::lock_guard<std::mutex> lockListener(freeWindowModeChangeListenerMutex_);
        return RegisterListener(freeWindowModeChangeListeners_[GetPersistentId()], listener);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d, listener is null", GetPersistentId());
        return WMError::WM_ERROR_NULLPTR;
    }
    return WMError::WM_OK;
}
 
WMError WindowSessionImpl::UnregisterFreeWindowModeChangeListener(const sptr<IFreeWindowModeChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "Start unregister");
    std::lock_guard<std::mutex> lockListener(freeWindowModeChangeListenerMutex_);
    return UnregisterListener(freeWindowModeChangeListeners_[GetPersistentId()], listener);
}
 
void WindowSessionImpl::NotifyFreeWindowModeChange(bool isInFreeWindowMode)
{
    std::lock_guard<std::mutex> lockListener(freeWindowModeChangeListenerMutex_);
    auto freeWindowModeChangeListeners = GetListeners<IFreeWindowModeChangeListener>();
    for (auto& listener : freeWindowModeChangeListeners) {
        if (listener != nullptr) {
            listener->OnFreeWindowModeChange(isInFreeWindowMode);
        }
    }
}

WMError WindowSessionImpl::RegisterUIContentCreateListener(const sptr<IUIContentCreateListener>& listener)
{
    if (!listener) {
        TLOGE(WmsLogTag::WMS_COMPAT, "listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "id: %{public}d", GetParentId());
    std::unique_lock<std::shared_mutex> lock(uiContentMutex_);
    uiContentCreateListeners_.emplace_back(listener);
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UnregisterUIContentCreateListener(const sptr<IUIContentCreateListener>& listener)
{
    if (!listener) {
        TLOGE(WmsLogTag::WMS_COMPAT, "listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "id: %{public}d", GetParentId());
    std::unique_lock<std::shared_mutex> lock(uiContentMutex_);
    auto iter = std::find(uiContentCreateListeners_.begin(), uiContentCreateListeners_.end(), listener);
    if (iter != uiContentCreateListeners_.end()) {
        uiContentCreateListeners_.erase(iter);
    } else {
        TLOGE(WmsLogTag::WMS_COMPAT, "listener not found");
    }
    return WMError::WM_OK;
}

WMError WindowSessionImpl::UpdateCompatibleStyleMode(CompatibleStyleMode mode)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "id:%{public}d mode:%{public}d", GetParentId(), static_cast<int32_t>(mode));
    auto hostSession = GetHostSession();
    if (!hostSession) {
        TLOGNE(WmsLogTag::WMS_COMPAT, "hostSession is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    property_->SetPageCompatibleMode(mode);
    hostSession->NotifyCompatibleModeChange(mode);
    return WMError::WM_OK;
}
} // namespace Rosen
} // namespace OHOS
